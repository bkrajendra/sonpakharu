#include <FS.h> //this needs to be first, or it all crashes and burns...
#include <Arduino.h>
#include "MATRIX7219.h"

#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#ifdef ESP32
#include <SPIFFS.h>
#endif

#ifdef ESP32
#include <WiFi.h>
#include <WebServer.h>
#endif
#include <ESPmDNS.h>

#include <uri/UriBraces.h>
#include <uri/UriRegex.h>
//  ESP32
// uint8_t dataPin   = 12; //16;
// uint8_t pulseValve = 13; //17;
// uint8_t clockPin  = 14; //18;
// uint8_t count     = 1;

// default custom static IP
// char static_ip[16] = "10.0.1.59";
// char static_gw[16] = "10.0.1.1";
// char static_sn[16] = "255.255.255.0";
//  select which pin will trigger the configuration portal when set to LOW
#define TRIGGER_PIN 12   // 16;
uint8_t pulseValve1 = 13; // 17;
uint8_t pulseValve2 = 14; // 18;
WiFiManager wifiManager;
WebServer server(80);
bool portalRunning = false;

// Declarations
void checkButton();
void handleRoot();
void generatePulseON();
void generatePulseOFF();


void setup()
{
  // initialize LED digital pin as an output.
  // pinMode(LED_BUILTIN, OUTPUT);
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  pinMode(pulseValve1, OUTPUT);
  pinMode(pulseValve2, OUTPUT);
  Serial.begin(115200);
  Serial.println("############### Welcome to Sonpakhru ###############");

  // start-block2
  IPAddress _ip = IPAddress(192, 168, 1, 31);
  IPAddress _gw = IPAddress(192, 168, 1, 1);
  IPAddress _sn = IPAddress(255, 255, 255, 0);
  // end-block2
  wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);
  if (!wifiManager.autoConnect("Sonpakhru", "123456789"))
  {
    Serial.println("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.restart();
    delay(5000);
  }

  // if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");

  Serial.println("local ip");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("sonpakhru"))
  {
    Serial.println("sonpakhru MDNS responder started");
  }

  server.on(F("/"), handleRoot);

  server.on("/on", []() {
    generatePulseON();
    // server.send(200, "text/plain", "Its ON");
    server.sendHeader("Location", String("/"));
    server.send(302);
  });

  server.on("/off", []() {
    generatePulseOFF();
    //server.send(200, "text/plain", "Its OFF");
    server.sendHeader("Location", String("/"));
    server.send(302);
  });

  server.onNotFound([]()
                    { server.send(404, "text/plain", "Page Not Found"); });
  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
  checkButton();
  server.handleClient();
  delay(2);//allow the cpu to switch to other tasks
}

void checkButton()
{
  // is auto timeout portal running
  if (portalRunning)
  {
    wifiManager.process();
  }

  // is configuration portal requested?
  if (digitalRead(TRIGGER_PIN) == LOW)
  {
    delay(50);
    if (digitalRead(TRIGGER_PIN) == LOW)
    {
      if (!portalRunning)
      {
        Serial.println("Button Pressed, Starting Portal");
        wifiManager.startWebPortal();
        portalRunning = true;
      }
      else
      {
        Serial.println("Button Pressed, Stopping Portal");
        wifiManager.stopWebPortal();
        portalRunning = false;
      }
    }
  }
}

void handleRoot()
{
  char temp[1000];
  snprintf(temp, 1000,
  "<!DOCTYPE html><html>\
    <head><meta name='viewport' content='width=device-width, initial-scale=1'>\
    <title>Sonpakharu Control</title>\
    <link rel='icon' href='data:,'>\
    <style>\
      html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\
      .button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;\
      text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}\
      .button2 {background-color: #555555;}\
    </style>\
  </head>\
  <body>\
    <h1>Sonpakharu Control</h1>\
    <h6>Control Valve</h6>\
    <p><a href=\"/on\"><button class=\"button\">ON</button></a></p><p><a href=\"/off\"><button class=\"button button2\">OFF</button></a></p>\
  </body>\
</html>");
  server.send(200, "text/html", temp);
}

void generatePulseON()
{
  digitalWrite(pulseValve1, HIGH);
  digitalWrite(pulseValve2, LOW);
  delay(30);
  digitalWrite(pulseValve1, LOW);
  digitalWrite(pulseValve2, LOW);
}
void generatePulseOFF()
{
  digitalWrite(pulseValve1, LOW);
  digitalWrite(pulseValve2, HIGH);
  delay(30);
  digitalWrite(pulseValve1, LOW);
  digitalWrite(pulseValve2, LOW);
}