#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Wire.h>
#include "RTClib.h"

#define buzzerPin D1

// NTP/UDP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Konfigurasi WiFi
const char* ssid = "12";
const char* password = "12348765";

// Konfigurasi dot matrix
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define DATA_PIN D5
#define CS_PIN D6
#define CLK_PIN D7

MD_Parola Matrix = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
String displayText = "";

ESP8266WebServer server(80);

String subuh = "03:49";
String dzuhur = "11:31";
String ashar = "14:58";
String maghrib = "17:49";
String isya = "19:05";

void setup() {
  Serial.begin(115200);
  pinMode(buzzerPin, OUTPUT);
  timeClient.begin();
  timeClient.setTimeOffset(25200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Matrix.begin();
  Matrix.setIntensity(15);

  server.on("/", HTTP_GET, []() {
    String html = "<html><body><h1>MATRIX-8266</h1>";
    html += "<form method='post' action='/message'><input type='text' name='text'><input type='submit' value='Submit'></form>";
    html += "<button onclick='subuh()'>Subuh</button>";
    html += "<button onclick='dzuhur()'>Dzuhur</button>";
    html += "<button onclick='ashar()'>Ashar</button>";
    html += "<button onclick='maghrib()'>Maghrib</button>";
    html += "<button onclick='isya()'>Isya</button>";
    html += "<button onclick='showTime()'>Show Time</button><br>";
    html += "<button onclick='cls()'>Clear</button><br>";
    html += "<script>function showTime() {fetch('/time').then(response => response.text()).then(text => alert(text));}";
    html += "function subuh() {fetch('/subuh').then(response => response.text()).then(text => alert(text));}";
    html += "function dzuhur() {fetch('/dzuhur').then(response => response.text()).then(text => alert(text));}";
    html += "function ashar() {fetch('/ashar').then(response => response.text()).then(text => alert(text));}";
    html += "function maghrib() {fetch('/maghrib').then(response => response.text()).then(text => alert(text));}";
    html += "function isya() {fetch('/isya').then(response => response.text()).then(text => alert(text));}";
    html += "function cls() {fetch('/clear')}</script>";
    html += "</body></html>";
    server.send(200, "text/html", html);
  });

  server.on("/message", HTTP_POST, []() {
    displayText = server.arg("text");
    server.send(200, "text/plain", displayText);
  });

  server.on("/time", HTTP_GET, []() {
    displayTimeOnMatrix();
    server.send(200, "text/plain", "Time");
    displayTimeOnMatrix();
  });

  server.on("/subuh", HTTP_GET, []() {
    showSubuh();
    server.send(200, "text/plain", "Subuh");
    showSubuh();
  });

  server.on("/dzuhur", HTTP_GET, []() {
    showDzuhur();
    server.send(200, "text/plain", "Dzuhur");
    showDzuhur();
  });

  server.on("/ashar", HTTP_GET, []() {
    showAshar();
    server.send(200, "text/plain", "Ashar");
    showAshar();
  });

  server.on("/maghrib", HTTP_GET, []() {
    showMaghrib();
    server.send(200, "text/plain", "Maghrib");
    showMaghrib();
  });

  server.on("/isya", HTTP_GET, []() {
    showIsya();
    server.send(200, "text/plain", "Isya");
    showIsya();
  });

  server.on("/clear", HTTP_GET, []() {
    cls();
    server.send(200, "text/plain", "Clear Matrix.");
    cls();
  });

  server.begin();
}

void loop() {
  server.handleClient();
  timeClient.update();

  if (!displayText.isEmpty()) {
    displayOnMatrix(displayText);
    displayText = "";
  }
}

void displayTimeOnMatrix() {
  timeClient.update();

  int hour = timeClient.getHours();
  int minute = timeClient.getMinutes();

  String displayTime = (hour < 10 ? "0" + String(hour) : String(hour)) + ":" + (minute < 10 ? "0" + String(minute) : String(minute));
  Matrix.setTextAlignment(PA_CENTER);
  Matrix.print(displayTime);
  timeClient.update();

  if (displayTime == subuh || displayTime == dzuhur || displayTime == ashar || displayTime == maghrib || displayTime == isya) {
    tone(buzzerPin, 1000); 
  } else {
    noTone(buzzerPin);
  }
}

void showSubuh() {
  Matrix.setTextAlignment(PA_CENTER);
  Matrix.print(String(subuh));
}

void showDzuhur() {
  Matrix.setTextAlignment(PA_CENTER);
  Matrix.print(String(dzuhur));
}

void showAshar() {
  Matrix.setTextAlignment(PA_CENTER);
  Matrix.print(String(ashar));
}

void showMaghrib() {
  Matrix.setTextAlignment(PA_CENTER);
  Matrix.print(String(maghrib));
}

void showIsya() {
  Matrix.setTextAlignment(PA_CENTER);
  Matrix.print(String(isya));
}

void cls() {
  Matrix.displayClear();
}

void displaySolatOnMatrix() {
  if (Matrix.displayAnimate()) {
    Matrix.displayReset();
  }
}

void displayOnMatrix(String text) {
  Matrix.setTextAlignment(PA_CENTER);
  Matrix.print(displayText);
}
