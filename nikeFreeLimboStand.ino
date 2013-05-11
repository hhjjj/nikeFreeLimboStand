#include <SPI.h>
#include <Ethernet.h>

#include <ArdOSC.h>

byte myMac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte myIp[]  = { 192, 168, 0, 177 };
int  serverPort  = 8000;
int destPort=10000;
byte destIp[] = {192,168,0,7};

int limbo5Pin = 9;
int limbo4Pin = 8;
int limbo3Pin = 7;
int limboEPin = 6;

OSCServer server;
OSCClient client;


int standHeight = 3; // 5.0 = 3 / 4.0 = 2 / 3.0 = 1 / extreme = 0
int humanCount = 0;
unsigned long time;
unsigned long resetTime = 500; // ms

void setup(){ 
  
 Serial.begin(19200);
 
 Ethernet.begin(myMac ,myIp); 
 server.begin(serverPort);
 
 //set callback function & oscaddress
 server.addCallback("/stand",&msgReceive);
 
 digitalWrite(limbo5Pin, LOW);
 digitalWrite(limbo4Pin, LOW);
 digitalWrite(limbo3Pin, LOW);
 digitalWrite(limboEPin, LOW);
 
 pinMode(limbo5Pin, OUTPUT);
 pinMode(limbo4Pin, OUTPUT);
 pinMode(limbo3Pin, OUTPUT);
 pinMode(limboEPin, OUTPUT);


 pinMode(A0, INPUT);
 humanCount = 0;

 Serial.println("Limbo Stand");
 time = millis();
}
  
void loop(){
  
  if(millis() - time > resetTime)
  {
    // Reset Human Count When no one is detected for certain amount of time
    humanCount = 0;
    time = millis();
    Serial.println("human count reset!");
  }
  
  if(digitalRead(A0) == 1)
  {
    
    humanCount++;
    time = millis();
    if (humanCount > 5000)
    {
      humanCount = 0;
     
    // take Picture
    OSCMessage msg;
    msg.setAddress(destIp,destPort);
    msg.beginMessage("/stand");
    msg.addArgString("shoot");
    client.send(&msg);
     Serial.println("human detected!");
    }
  }
  
  if(server.aviableCheck()>0){
 //    Serial.println("alive! "); 
  }
  
  
}

//int limbo5Pin = 9;
//int limbo4Pin = 8;
//int limbo3Pin = 7;
//int limboEPin = 6;
// 5.0 = 3 / 4.0 = 2 / 3.0 = 1 / extreme = 0

void resetHeight()
{
  int i;
  for(i = 0; i < 4 ; i++)
  {
     digitalWrite(limbo5Pin - i, LOW);
  }
  delay(200);
  
  for(i = 0; i < 4 ; i++)
  {
     digitalWrite(limbo5Pin - i, HIGH);
     delay(600);

  }
    for(i = 0; i < 4 ; i++)
  {
     digitalWrite(limbo5Pin - i, LOW);
  }
  delay(200);
}

void setHeight(int height)
{
  resetHeight();
  digitalWrite(height + 6 , HIGH);
}

void msgReceive(OSCMessage *_mes){
  
  Serial.println("received");
  byte *ip = _mes->getIpAddress();
  destIp[0] = ip[0];
  destIp[1] = ip[1];
  destIp[2] = ip[2];
  destIp[3] = ip[3];
  
  int value = _mes->getArgInt32(0);
  if(value < 4 && value >= 0)
  {
    standHeight = value;
    Serial.print("stand height:");
    Serial.println(standHeight);
    
    setHeight(standHeight);
    
    OSCMessage msg;
    msg.setAddress(_mes->getIpAddress(),destPort);
    msg.beginMessage("/stand");
    msg.addArgString("set");
    client.send(&msg);
  }
  else
  
  {
    int strSize = _mes->getArgStringSize(0);
    char tmpStr[strSize]; //string memory allocation
    _mes->getArgString(0,tmpStr);
    
    String received = String(tmpStr);
    String test = String("test");
    String reset = String("reset");
    if ( received == test)
    {
     OSCMessage msg;
      msg.setAddress(_mes->getIpAddress(),destPort);
      msg.beginMessage("/stand");
      msg.addArgString("ok");
      client.send(&msg);
    }
    else
    if (received == reset)
    {
      resetHeight();
      OSCMessage msg;
      msg.setAddress(_mes->getIpAddress(),destPort);
      msg.beginMessage("/stand");
      msg.addArgString("reset");
      client.send(&msg);
    }
    
  }  
}
