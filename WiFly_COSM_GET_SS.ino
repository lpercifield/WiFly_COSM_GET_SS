// WiFly Pachube Client
// Send data to a Pachube Feed
// (Based on Ethernet's WebClient Example)
// (based upon Sparkfun WiFly Web Client example)
// Sparkfun WiFly library updated and can be found here
// https://github.com/jcrouchley/WiFly-Shield
// Built using Arduino IDE V0.22

#include <SPI.h>
#include <WiFly.h>

// using NewSoftSerial V11 beta
// downloaded from here http://arduiniana.org/2011/01/newsoftserial-11-beta/
// this will be included as Software Serial in Arduino IDE 1.0
#include <SoftwareSerial.h>

// Wifly RN-XV (XBee shaped) module connected
//  WiFly Tx to pin 2 (SoftSerial Rx)
//  WiFly Rx to pin 3 (SoftSerial Tx)
SoftwareSerial mySerial(10, 11);
#define MAXDATASTREAMS 6
#define BUFFSIZ 90 // plenty big
char buffer[BUFFSIZ];
int buffidx;
//char *buffer;
int numStreams;
//int data[MAXDATASTREAMS];
//char* id[MAXDATASTREAMS];
char streamId[3];
char intHolder[8];
int val;

// Edit credentials.h to provide your own credentials
#include "Credentials.h"

// Using Pachube API V2
WiFlyClient client;

void setup() {

  pinMode(8,OUTPUT);
  digitalWrite(8,HIGH);
  // lots of time for the WiFly to start up and also in case I need to stop the transmit
  delay(10000);

  Serial.begin(9600);  // nice and fast
  mySerial.begin(4800);  // default WiFly baud rate - good enough for this

  WiFly.setUart(&mySerial); // Tell the WiFly library that we are not using the SPIUart

  Serial.println("Wifly begin");

  WiFly.begin();    // startup the WiFly

  Serial.println("Wifly join");

  // Join the WiFi network
  if (!WiFly.join(ssid, passphrase)) {
    Serial.println("Association failed.");
    while (1) {
      // Hang on failure.
    }
  }  

}

uint32_t timeLastUpdated;
int i;
char buff[64];

void loop() {
  if (millis() - timeLastUpdated > TIMETOUPDATE)
  {  // time for the next update
    timeLastUpdated = millis();

    // prepare the data to send
    // format (API V2)
    // multiple lines each with <datastreamID>,<datastreamValue>
    // feedID can be the datastream name of the numberic ID
    Serial.println("connecting...");
    if (client.connect("api.cosm.com", 80)) {
      Serial.println("connected");
      client.print("GET /v2/feeds/");  // APIV2
      client.print(PACHUBEFEED);
      client.println(".csv HTTP/1.1");
      client.print("X-PachubeApiKey: ");
      client.println(APIKEY);
      client.println("Host: api.cosm.com");

      //      client.println("User-Agent: Arduino (WiFly RN-XV)");
      //      client.print("Content-Type: text/csv\nContent-Length: ");
      //      client.println(strlen(buff));
      //      client.println("Connection: close");
      //      client.println();
      //  
      //      client.print(buff);
      client.println();

    } 
    else {
      Serial.println("connection failed");
    }

    //delay(2000);
    int count = 0;
    boolean current_line_is_blank = false;
    boolean header_ended = false;
    numStreams = 0; 

    int counter = 0;
    boolean gotData = false;
    while (client.connected()) {
      if (client.available()) {
        
        counter = 0;
        // TODO verify success (HTTP/1.1 200 OK)
        //Serial.write(client.read());  // display the result
        char c = client.read();
        if (c == '\n' && current_line_is_blank) {
          //Serial.println("end");
          header_ended = true;
          gotData = true;
        }
        if(header_ended){
          //byte myStuff = byte(c);
          //Serial.print(c);
          if(c=='\n'){ //we have a new line but dont know if there are any chars
            numStreams++; // we increase the number of datastreams
            if(buffidx>0){
              parseDataStream();
            }
            buffidx = 0; // start at begninning
            //Serial.println('/');
          }
          else{
            buffer[buffidx++]= c; //keep adding chars to the buffer
          }     
        }
        if (c == '\n') {
          // we're starting a new line
          current_line_is_blank = true;
        } 
        else if (c != '\r') {
          // we've gotten a character on the current line
          current_line_is_blank = false;
        }
      }else{
        if(gotData){
          counter++;
          //Serial.println("NOT AVAILABLE");
        }
        if(counter >50){
          parseDataStream();
          buffidx = 0;
          Serial.println("disconnecting.");
          client.stop();
          Serial.println("disconnected.");
        }
      }
      //Serial.println();
    }
  }
}
void parseDataStream(){
  memset(streamId,0,sizeof(streamId));
  memset(intHolder,0,sizeof(intHolder));
  //Serial.println(buffer);
  //Serial.print(" ");
  //Serial.println(buffidx);
  int commaCounter = 0;
//  int timeStamp;
  int charCounter=0;
  for(int i=0; i<buffidx;i++){
    switch (buffer[i]){
      case ',':
          commaCounter++;
          charCounter = 0;
        break;
      default:
        //Serial.print(buffer[i]);
        if(commaCounter == 0){
          streamId[charCounter] = buffer[i];
        }else if(commaCounter>1){
          intHolder[charCounter]=buffer[i];
        }
        charCounter++;
    }
  }
//  Serial.println();
  Serial.print(streamId);
  Serial.print("\t");
  val = atoi(intHolder);
  Serial.println(val);
//  char* streamId;
//  char* timeStamp;
//  char* streamData;
//  sscanf(buffer,"%s,%s,%s",streamId,timeStamp,streamData);
//  Serial.print(streamId);
//  Serial.print("\t");
//  Serial.println(streamData);
}


