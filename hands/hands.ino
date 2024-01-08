/***************************************************************************
  This is an example for the Adafruit SensorLab library
  It will look for a supported gyroscope and collect
  rad/s data for a few seconds to calcualte the zero rate
  calibration offsets
  
  Written by Limor Fried for Adafruit Industries.
 ***************************************************************************/
#include <WiFi.h>
#include <WiFiUdp.h>

#include <Adafruit_SensorLab.h>
Adafruit_SensorLab lab;

#define NUMBER_SAMPLES 500

const char* ssid = "[Replace with WiFi network name]";
const char* password = "[Replace with WiFi network password]";

const char* udpServerIP = "[Replace with VR Headset ip adress]";  // Replace with the IP address of the UDP server
const int udpServerPort = "[Replace with VR Headset UDP server port]";             // Replace with the port number of the UDP server

// Button 
const int BUTTON_PIN_12 = 12;
const int BUTTON_PIN_14 = 14;

WiFiUDP udp;

Adafruit_Sensor *gyro;
sensors_event_t event;

Adafruit_Sensor *temp;
sensors_event_t temp_event;

float min_x, max_x, mid_x;
float min_y, max_y, mid_y;
float min_z, max_z, mid_z;

float min_temp, max_temp, mid_temp;

float xNoise, yNoise, zNoise;
float xOffset, yOffset, zOffset;
float gx, gy, gz;
float X, Y, Z;

float udpCounter = 0;

void setup(void) {
  Serial.begin(115200);

  pinMode(BUTTON_PIN_12, INPUT_PULLUP);
  pinMode(BUTTON_PIN_14, INPUT_PULLUP);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi!");

  while (!Serial) delay(10);     // will pause Zero, Leonardo, etc until serial console opens
  
  Serial.println(F("Sensor Lab - Gyroscope Calibration!"));
  lab.begin();
  
  Serial.println("Looking for a gyro");
  gyro = lab.getGyroscope();
  temp = lab.getTemperatureSensor();

  if (!gyro) {
    Serial.println(F("Could not find a gyro, check wiring!"));
    while(1) delay(10);
  }

  if (!temp) {
    Serial.println(F("Could not find temperature sensor, check wiring!"));
    while(1) delay(10);
  }

  gyro->printSensorDetails();
  delay(100);

  
  gyro->getEvent(&event);
  min_x = max_x = event.gyro.x;
  min_y = max_y = event.gyro.y;
  min_z = max_z = event.gyro.z;
  
  temp->getEvent(&temp_event);
  min_temp = max_temp = temp_event.temperature;
    
  delay(10);

  Serial.println(F("Place gyro on flat, stable surface!"));

  Serial.print(F("Fetching samples in 3..."));
  delay(1000);
  Serial.print("2...");
  delay(1000);
  Serial.print("1...");
  delay(1000);
  Serial.println("NOW!");
  
  float x, y, z, t;
  for (uint16_t sample = 0; sample < NUMBER_SAMPLES; sample++) {
    gyro->getEvent(&event);
    x = event.gyro.x;
    y = event.gyro.y;
    z = event.gyro.z;

    temp->getEvent(&temp_event);
    t = temp_event.temperature;

    Serial.print(F("Gyro: ("));
    Serial.print(x); Serial.print(", ");
    Serial.print(y); Serial.print(", ");
    Serial.print(z); Serial.print(")");
    Serial.print("Temp: ");Serial.print(t); 

    min_x = min(min_x, x);
    min_y = min(min_y, y);
    min_z = min(min_z, z);

    min_temp = min(min_temp, t);
  
    max_x = max(max_x, x);
    max_y = max(max_y, y);
    max_z = max(max_z, z);

    max_temp = max(max_temp, t);
  
    mid_x = (max_x + min_x) / 2;
    mid_y = (max_y + min_y) / 2;
    mid_z = (max_z + min_z) / 2;

    mid_temp = (max_temp + min_temp) / 2;

    Serial.print(F(" Zero rate offset: ("));
    Serial.print(mid_x, 4); Serial.print(", ");
    Serial.print(mid_y, 4); Serial.print(", ");
    Serial.print(mid_z, 4); Serial.print(")");  
  
    Serial.print(F(" rad/s noise: ("));
    Serial.print(max_x - min_x, 3); Serial.print(", ");
    Serial.print(max_y - min_y, 3); Serial.print(", ");
    Serial.print(max_z - min_z, 3); Serial.println(")");
    
    delay(10);
  }
  Serial.println(F("\n\nFinal zero rate offset in radians/s: "));
  Serial.print(mid_x, 4); Serial.print(", ");
  Serial.print(mid_y, 4); Serial.print(", ");
  Serial.println(mid_z, 4);

  Serial.println("Avg Temp: " + String(mid_temp));

  xOffset = mid_x;
  yOffset = mid_y;
  zOffset = mid_z;

  xNoise = max_x - min_x;
  yNoise = max_y - min_y;
  zNoise = max_z - min_z;

}



void loop() {
  gyro->getEvent(&event);
    gx = event.gyro.x - xOffset;
    gy = event.gyro.y - yOffset;
    gz = event.gyro.z - zOffset;

    if (abs(gx) > xNoise) {
      X += gx;
    }
    if (abs(gy) > yNoise) {
      Y += gy;
    }
    if (abs(gz) > zNoise) {
      Z += gz;
    }


  if (udpCounter > 200) {
      // BUTTONS
      int readingLeft = digitalRead(BUTTON_PIN_14);
      int readingRight = digitalRead(BUTTON_PIN_12);

      Serial.print("Gyroscope (dps): ");
      Serial.print("X: "); Serial.print(gx); Serial.print(" ");
      Serial.print("Y: "); Serial.print(gy); Serial.print(" ");
      Serial.print("Z: "); Serial.println(gz);

      Serial.print("Gyroscope Acc: ");
      Serial.print("X: "); Serial.print(X); Serial.print(" ");
      Serial.print("Y: "); Serial.print(Y); Serial.print(" ");
      Serial.print("Z: "); Serial.println(Z);


      Serial.println();
      Serial.print(readingLeft); Serial.print(" "); Serial.print(readingRight);
      Serial.println();


      Serial.println("Sending message...");

      udp.beginPacket(udpServerIP, udpServerPort);
      udp.println("bg;" + String(readingLeft) + ";" + String(readingRight) + ";" + String(X) + ";" + String(Y) + ";" + String(Z));
      udp.endPacket();
      udpCounter = 0;
    }
    else {
      udpCounter += 10;
    }

  delay(10); 
}