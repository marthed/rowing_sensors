#include <WiFi.h>
#include <WiFiUdp.h>

const char* ssid = "[Replace with WiFi network name]";
const char* password = "[Replace with WiFi network password]";

const char* udpServerIP = "[Replace with VR Headset ip adress]";  // Replace with the IP address of the UDP server
const int udpServerPort = "[Replace with VR Headset UDP server port]";             // Replace with the port number of the UDP server

WiFiUDP udp;

int fsrPinLeft = A2;     // the FSR and 10K pulldown are connected to a0
int fsrPinRight = A4;

int fsrPinLeftToe = A9;
int fsrPinRightToe = A7;

int fsrReadingLeft = 0;     // the analog reading from the FSR resistor divider
int fsrReadingRight = 0;

int fsrReadingLeftToe = 0;
int fsrReadingRightToe = 0;

void setup() {
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");

  udp.begin(udpServerPort);
}

void loop() {

  int newFsrReadingLeft = analogRead(fsrPinLeft); 
  Serial.print("Analog reading left heel = ");
  Serial.println(newFsrReadingLeft);  // the raw analog reading

  int newFsrReadingRight = analogRead(fsrPinRight); 
  Serial.print("Analog reading right heel = ");
  Serial.println(newFsrReadingRight);  // the raw analog reading

  int newFsrReadingLeftToe = analogRead(fsrPinLeftToe); 
  Serial.print("Analog reading left toe = ");
  Serial.println(newFsrReadingLeftToe);  // the raw analog reading

  int newFsrReadingRightToe = analogRead(fsrPinRightToe); 
  Serial.print("Analog reading right toe = ");
  Serial.println(newFsrReadingRightToe);  // the raw analog reading

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {

    if (newFsrReadingLeft != fsrReadingLeft || newFsrReadingRight != fsrReadingRight || newFsrReadingLeftToe != fsrReadingLeftToe || newFsrReadingRightToe != fsrReadingRightToe) { 

      fsrReadingLeft = newFsrReadingLeft;
      fsrReadingRight = newFsrReadingRight;
      fsrReadingLeftToe = newFsrReadingLeftToe;
      fsrReadingRightToe = newFsrReadingRightToe;

      Serial.println("Sending message...");
      udp.beginPacket(udpServerIP, udpServerPort);
      udp.println("fs-" + String(newFsrReadingLeft) + "-" + String(newFsrReadingRight) + "-" + String(newFsrReadingLeftToe) + "-" + String(newFsrReadingRightToe));
      udp.endPacket();

      if (udp.parsePacket()) {
        Serial.print("Response from server: ");
        while (udp.available()) {
          Serial.write(udp.read());
        }
        Serial.println();
    } else {
      Serial.println("No response from server");
      }
    }
    
  } else {
    Serial.println("Not connected to Wifi");
  }

  delay(200); // Change to faster later
}


