#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include "arduino_secrets.h"

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDR 0x3C
#define FONT_SIZE 2
#define CHAR_SIZE 6
#define MSG_SIZE 17

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT);
WiFiClientSecure wifi;
HTTPClient httpClient;
int x = display.width();
int minX = -FONT_SIZE * CHAR_SIZE * MSG_SIZE;
int timesScrolled = 0;

char trainTime1[MSG_SIZE] = {0};
char trainTime2[MSG_SIZE] = {0};
char trainTime3[MSG_SIZE] = {0};

void setTrainTimes(DynamicJsonDocument document) {
    JsonArray timesArray = document.as<JsonArray>();
    char journeyTemplate[] = "%s -> %s %s\0";
    snprintf(trainTime1, sizeof(trainTime1), journeyTemplate, timesArray[0]["dep"].as<String>(), timesArray[0]["arr"].as<String>(), timesArray[0]["dep_time"].as<String>());
    snprintf(trainTime2, sizeof(trainTime2), journeyTemplate, timesArray[1]["dep"].as<String>(), timesArray[1]["arr"].as<String>(), timesArray[1]["dep_time"].as<String>());
    snprintf(trainTime3, sizeof(trainTime3), journeyTemplate, timesArray[2]["dep"].as<String>(), timesArray[2]["arr"].as<String>(), timesArray[2]["dep_time"].as<String>());
}

void connectWiFi()
{
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print("Trying to connect to ");
        Serial.println(SSID);
        WiFi.begin(SSID, SSID_PASS);
        delay(500);
    }

    wifi.setInsecure();
}

void getTrainTimes()
{
    connectWiFi();
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("fetching");
    display.display();
    DynamicJsonDocument doc(1024);
    Serial.println("Sending request");
    httpClient.addHeader("x-functions-key", FUNCTIONS_KEY);
    httpClient.begin(wifi, FUNCTIONS_URL);
    int code = httpClient.GET();
    Serial.println(code);
    deserializeJson(doc, httpClient.getString());
    httpClient.end();
    wifi.stop();
    setTrainTimes(doc);
}

void showTimesScrolledDebug()
{
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(timesScrolled);
}

void updateDisplay()
{
    display.clearDisplay();
    // showTimesScrolledDebug();
    display.setTextSize(FONT_SIZE);
    display.println();
    display.setCursor(x, 8);
    display.print(trainTime1);
    display.setCursor(x, 28);
    display.print(trainTime2);
    display.setCursor(x, 48);
    display.print(trainTime3);
    display.display();
}

void setup()
{
    Serial.begin(115200);
    connectWiFi();
    Serial.print("Connected and assigned ");
    Serial.println(WiFi.localIP());
    display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    display.clearDisplay();
    display.setTextSize(FONT_SIZE);
    display.setTextColor(WHITE);
    display.setTextWrap(false);
    getTrainTimes();
}

void loop()
{
    updateDisplay();
    x -= 2.5;
    if (x < minX)
    {
        x = display.width();
        timesScrolled++;
    }

    // refresh times rougly every 30 mins
    if (timesScrolled == 360) {
        getTrainTimes();
        timesScrolled = 0;
    }
}