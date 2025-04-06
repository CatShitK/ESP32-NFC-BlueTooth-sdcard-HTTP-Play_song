#include <Arduino.h>
#include "Audio.h"
#include <WiFi.h>
#include <HTTPClient.h>
// //测试SD卡
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <FSImpl.h>
#include <vfs_api.h>
//#include "WiFiMulti.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>

#include <Wire.h>
#include <Adafruit_PN532.h>
#include <Adafruit_NeoPixel.h>
// //测试NFC，串口会打印MAC地址，需要记录
#include <Wire.h>
#include <Adafruit_PN532.h>
#include <WiFi.h>
#include <Arduino.h>
#include "Audio.h"
#include "SD.h"
#include "FS.h"
#include <Wire.h>
// #define SDA_PIN 21->18// #define I2S_DOUT      0-->47
// #define I2S_BCLK      2-->48
// #define SCL_PIN 22->17// #define SPI_MOSI      23->9
// #define SPI_MISO      19->8
// #define SPI_SCK       18->7

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "driver/gpio.h"


//PN532
#define SDA_PIN 18  
#define SCL_PIN 17
//MAx98357
#define I2S_DOUT      47
#define I2S_BCLK      41
// #define I2S_BCLK      48
#define I2S_LRC       15
//SD卡
#define SD_CS          5
#define SPI_MOSI      9
#define SPI_MISO     8
#define SPI_SCK      7
//点灯
#define Led 2

Audio audio;
Adafruit_PN532 nfc(SDA_PIN, SCL_PIN);
File configFile;
String pre_uidString;
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 }; // 存储读取到的UID
uint8_t uidLength; // UID长度
// 在全局变量中添加一个标志位
bool isReadingNFC = false;
// 全局变量
bool isPlayingAudio = false;
bool audiopaly = true;
//WiFiMulti wifiMulti;
// put function declarations here:
const int dataPin = 1;
HTTPClient http;
Cookie cookie;
#define DHTTYPE DHT11
// const char* ssid = "败家之眼";
// const char* password = "Tgs200410";
const char* ssid = "new";
const char* password = "kkkkkkkk";
// const char* ssid = "ChinaNet-r9fK";   //你的wifi和密码
// const char* password = "vckbtpuy";
//#define HttpGet "http://192.168.31.225:3000/download"
// #define HttpGet "http://139.196.44.63:30003/media/get-music?nfcKey=pingju.mp3"
#define HttpGet "http://139.196.44.63:30003/media/get-music?nfcKey="
File myFile;
u8_t wifiFlag = 0;
bool hasWrittenConfig = true;  // 新增标志变量
bool Wifisound = false;
// 全局变量，用于存储WiFi和密码
String globalWifi, globalPassword;
String globalHttpGet;
void appendToConfigFile(const String& uidString, const String& fileName);
void appendWiToConfigFile(const String& wifi,const String& password);


// 清除WiFi配置
// 清空指定文件内容的函数
// bool clearFileContent(const char* filePath) {
//   Serial.printf("Clearing file: %s\n", filePath);
//   if (!SD.begin(SD_CS)) {
//     Serial.println("SD Initfail！");
//     return false;
//   }

//   // 打开文件用于写入
//   File file = SD.open(filePath, FILE_WRITE);
//   if (!file) {
//     Serial.println("Can not open");
//     return false;
//   }

//   // 清空文件内容
//   file.truncate(0);

//   // 关闭文件
//   file.close();
//   Serial.println("Wificonfig is clear");
//   return true;
// }
void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);
    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

// 封装的函数，用于写入WiFi配置到SD卡
// 写入WiFi配置到SD卡
void writeWifiConfigToSD() {
    deleteFile(SD, "/WifiConfig.txt");
    if (globalWifi.isEmpty() || globalPassword.isEmpty()) 
    {
      Serial.println("Wifi and Password is Empty");
      return;
    }
    String Wifipackage = globalWifi + " " + globalPassword;
    writeFile(SD, "/WifiConfig.txt",Wifipackage.c_str());
    Serial.println("Wifi config written to config file");
    //当配置文件有值，才进行连接
    hasWrittenConfig = false;
    // File file = SD.open("/WifiConfig.txt", FILE_APPEND);
    // if (file) {
    //     file.println(globalWifi + " " + globalPassword);
    //     file.close();
    //     Serial.println("Wifi config written to config file");
    //     //当配置文件有值，才进行连接
    //     hasWrittenConfig = false;
    // } else {
    //     Serial.println("Failed to open config file for writing");
    // }
}
//实现将 SD 卡中的 config.txt 文件内容打印到串口的代码:
void printWifiFileContents() {
  File file = SD.open("/WifiConfig.txt", FILE_READ);
  if (file) {
    while (file.available()) {
      Serial.println(file.readStringUntil('\n'));
    }
    file.close();
  } else {
    Serial.println("Failed to open config file");
  }
}
//实现将 SD 卡中的 config.txt 文件内容打印到串口的代码:
void printConfigFileContents() {
  File file = SD.open("/config.txt", FILE_READ);
  if (file) {
    while (file.available()) {
      Serial.println(file.readStringUntil('\n'));
    }
    file.close();
  } else {
    Serial.println("Failed to open config file");
  }
}
  /******查看SD卡文件列表*******/  
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);
  
    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }
  
    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

//使用使用压缩传输:
const char *headerKeys[] = {"Content-Disposition", "Content-Type"};

 
BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;
String isWifi = "false";  
int BeginWificonnect = 0;
int ComposeHttp = 0;
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
void connectToWiFi(const std::string& wifi, const std::string& password);
// //调用接口
// void send_Message()
// {
//   http.begin(HttpGet);
//   int httpCode = http.GET();
//   Serial.print(httpCode);
//   if (httpCode == 200)
//   {
//     Serial.print("请求成功");
//   }
// }

//生成随机UUID
void generateUUID(char* uuid) {
  char hexChars[] = "0123456789abcdef";
  for (int i = 0; i < 36; i++) {
    if (i == 8 || i == 13 || i == 18 || i == 23) {
      uuid[i] = '-';
    } else {
      uuid[i] = hexChars[random(16)];
    }
  }
  uuid[36] = '\0'; // 添加字符串结尾标志
}

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      // 只会在第一次调用
      deviceConnected = true;
      Serial.println("onConnect!!!");
      //连接上蓝牙，返回wifi状态
          //初始化防止报错
    std::string dataToSend = "NoYet";

    // 将字符串转换为字节数组
    uint8_t sendData[dataToSend.length()];
    for (size_t i = 0; i < dataToSend.length(); i++) {
        sendData[i] = dataToSend[i];
    }
    // 将字节数组设置为特征值并发送
    pTxCharacteristic->setValue(sendData, dataToSend.length());
    pTxCharacteristic->notify();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("设备未连接");
    }
};
String resStr;
String chipId;
//Call back函数
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();
    //接收消息
    //uint16_t rxValue = (uint16_t*)pCharacteristic->getValue();

    std::string wifi, password;
    //如果wifi收到消息则对消息进行分割
    if (rxValue.length() > 0) {
      //接收原本消息打印在串口
      Serial.print("Wx Received Value: ");
        for (int i = 0; i < rxValue.length(); i++){
          Serial.print(rxValue[i]);
          resStr += rxValue[i];
          //对接收到字符串进行判断，如果有分号，则代表是连接wifi，则开始连接wifi
          if(rxValue[i]=='a')
          {
            BeginWificonnect = 1;
            //digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
            Serial.println("LED on");
          }
          //对接收到字符串进行判断，如果有等号，则代表是拼接http地址，则开始拼接
          if(rxValue[i]=='b')
          {
            ComposeHttp = 1;
            //digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
            Serial.println("LED off");
          }          
        }
        Serial.println();
        //Serial.println("*********");
        Serial.println(resStr);
      
      std::string received_data = rxValue;
      //微信小程序接收链接，误解需求， //如果收到账号密码，才开始连接wifi
        if(BeginWificonnect==10)
      {
        // 分割字符串
      std::vector<std::string> parts;
      std::size_t pos = 0;
      std::string delimiter = ";";
      while ((pos = received_data.find(delimiter)) != std::string::npos) {
          parts.push_back(received_data.substr(0, pos));
          received_data.erase(0, pos + delimiter.length());
      }
      parts.push_back(received_data);

      // 提取 Wifi 和 Password
      if (parts.size() >= 2) {
          wifi = parts[0];
          password = parts[1];
          
      }
      // 输出结果
      std::cout << "Wifi: " << wifi << std::endl;
      std::cout << "Password: " << password << std::endl;
            //       removeDir(SD, "/WifiConfig.txt");
            // createDir(SD, "/WifiConfig.txt");
      //将获取到的wifi和密码传入wifi连接函数
      connectToWiFi(wifi, password);
      // 将WiFi配置存储到全局变量中
      globalWifi = (wifi.c_str());
      globalPassword = (password.c_str());
      //重新置零，用于下次接收
      BeginWificonnect==0;
      }

      // //开始链接字符串产生可访问的链接
      //   if(ComposeHttp==1)
      // {
      // // 链接字符串
      // globalHttpGet = "http://139.196.44.63:30003/media/get-music?"+resStr;
      // Serial.println("组合后的HTTP地址为:");
      // Serial.println(globalHttpGet);
      // //将获取到的HTTP下载

      
      // //重新置零，用于下次接收
      // ComposeHttp==0;
      // }

}
    //初始化防止报错
    std::string dataToSend = "NoYet";
    // 检查 WiFi 连接状态并发送相应的字符串
    if (WiFi.status() == WL_CONNECTED) {
        dataToSend = "WiFiconnected";
        Serial.println("WiFi connected");
        hasWrittenConfig = false;
    } else {
        dataToSend = "NoWificonnected";
        Serial.println("NoWificonnected");
    }

    // 将字符串转换为字节数组
    uint8_t sendData[dataToSend.length()];
    for (size_t i = 0; i < dataToSend.length(); i++) {
        sendData[i] = dataToSend[i];
    }
    // 将字节数组设置为特征值并发送
    pTxCharacteristic->setValue(sendData, dataToSend.length());
    pTxCharacteristic->notify();
    }
};

  const char* password1 = "kkkkkkkk";
//连接wifi函数,传参版
void connectToWiFi(const std::string& wifi, const std::string& password) {
  uint8_t Wifi_Time = 0;
  //  unsigned long startTime = millis(); //时间记录
  //  Serial.print("确认一下Wifi的账号和密码...");
  //      Serial.print("Wifi: ");
  //      Serial.println(wifi.c_str());
  //      Serial.print("Password: ");
  //      Serial.println(password.c_str());
   Serial.print("Connecting to WiFi: ");
   Serial.println(wifi.c_str());
   Serial.print("Using Password: ");
   Serial.println(password.c_str());
   WiFi.begin(wifi.c_str(), password.c_str());

    // 等待连接成功
    while (WiFi.status() != WL_CONNECTED) {
      if(Wifi_Time>=10)
      {
        Serial.println("WiFi connected Fail");  //5s没连接成功返回错误
        break;
      }
      delay(500);
      Serial.print(".");
      Wifi_Time++;
    }
    if(WiFi.status() == WL_CONNECTED)
    {   
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      //WriteFile("/WifiConfig.txt", "ElectronicWings.com");
      // String arduinoString(stdString.c_str());
      // writeWifiConfigToSD(wifi,password);
    }

}

void Init_uid()
{
   // 生成UUID
  char uuid[37];
  generateUUID(uuid);
  char uuid_RX[37];   //由于微信小程序bug，uuid接收和发送uuid不能相同，相同则会出bug，所以再生成一次
  generateUUID(uuid_RX);
  chipId = String((uint32_t)ESP.getEfuseMac(), HEX);
  chipId.toUpperCase();
//  chipid =ESP.getEfuseMac();
//  Serial.printf("Chip id: %s\n", chipid.c_str());
  Serial.println("chipId:"+chipId);
  Serial.println();
  // Create the BLE Device
  BLEDevice::init("wsc_Secure");
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  //随机生成的uuid放入
  BLEService *pService = pServer->createService(uuid);        //随机生成的uuid放入
  //BLEService *pService = pServer->createService(SERVICE_UUID);
  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
                        // CHARACTERISTIC_UUID_TX,  
                        uuid,                                 //随机生成的uuid放入
                        BLECharacteristic::PROPERTY_NOTIFY
                      );

  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
      // CHARACTERISTIC_UUID_RX,                                  //由于微信小程序bug，uuid接收和发送uuid不能相同，相同则会出bug，所以再生成一次
      uuid_RX,                                                    //随机生成的uuid放入
      BLECharacteristic::PROPERTY_WRITE
                                          );

  pRxCharacteristic->setCallbacks(new MyCallbacks());
  // Start the service
  pService->start();
  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

TaskHandle_t downloadTaskHandle = NULL;
TaskHandle_t ReadNfcMatchMP3Handle = NULL; // 定义 ReadNfcMatchMP3 任务的句柄
bool downloadComplete = false;

// void downloadMp3Task(void *pvParameters) {
//   while (1) {
//     // 等待主任务的通知
//     ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
//     //downloadMp3FileToSD(HttpGet);
//     Serial.println("Downloade compete：");
//     listDir(SD, "/", 0);
//     // 通知主任务下载完成
//     // 设置下载完成标志位
//     downloadComplete = true;
// // 检查 ReadNfcMatchMP3Handle 是否有效
//     if (ReadNfcMatchMP3Handle != NULL) {
//       // 通知主任务下载完成
//       xTaskNotify(ReadNfcMatchMP3Handle, 1, eSetValueWithOverwrite);
//     } else {
//       Serial.println("Error: ReadNfcMatchMP3Handle is NULL");
//     }
//   }
// }

void ReadNfcMatchMP3(void *pvParameters) 
{
  while (1) 
  {
    Serial.println("Task 1 is running...");
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 }; // 存储读取到的UID
    uint8_t uidLength; // UID长度
   
    Serial.println("Waiting for NFC card....");
    // 检查是否有nfc卡片
    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) 
    {
      // 打印 UID
      Serial.print("UID: ");
      for (uint8_t i = 0; i < uidLength; i++) 
      {
        Serial.print(uid[i] < 0x10 ? " 0" : " ");
        Serial.print(uid[i], HEX);
      }
      Serial.println();
      String uidString = "";
      for (int i = 0; i < sizeof(uid); i++) 
      {
        if (uid[i] < 0x10) 
        {
          uidString += "0"; // 如果元素小于0x10，则补0
        }
        uidString += String(uid[i], HEX); // 将元素转换为十六进制字符串并拼接
      }

      Serial.println(uidString); // 打印拼接后的字符串

      if(pre_uidString == uidString)
      {
        //Serial.println("相同的UID,暂停");
        Serial.println("Stop or Play song"); 
        audio.pauseResume();
        delay(300);
        continue;
      }
      else
      {
        //Serial.println("不同的UID,操作切换歌曲");
        pre_uidString = uidString;
        //
        // audiopaly = false;
      }
      bool found = false;
      // 在配置文件中查找UID对应的MP3文件名
      configFile.seek(0);
      while (configFile.available()) 
      {
        String line = configFile.readStringUntil('\n');
        line.trim();
        if (line.startsWith(uidString)) 
        {
          // 找到匹配的UID，播放对应的MP3文件
          String mp3FileName = "/" + line.substring(15); // 假设UID和MP3文件名之间有一个空格
          Serial.println(mp3FileName); // 打印拼接后的字符串
          // audiopaly = true;
          // 如果当前没有在播放，则开始播放，若在播放，则暂停
          audio.stopSong();
          delay(30);
          Serial.println("MP3 music shop!!");
          audio.connecttoFS(SD, "pass.mp3"); 
          printConfigFileContents();
          found = true;
          break;
        }
      }
      if (!found)
    {

    Serial.println("NFC card not found, playing Alarm...");
    audio.connecttoFS(SD, "Alarm.mp3"); 

    // //当本地没有对应文件，则向小程序发送请求获取NFC-KEY值
    // Serial.println("MP3 file not found, Connectting to WX...");

    // //小程序传输http，已弃用
    // // 有一个需要发送的数据字符串
    // String dataToSend = uidString;
    // // 将字符串转换为字节数组
    // size_t length = dataToSend.length();
    // uint8_t *sendData = (uint8_t *)malloc(length);
    // for (size_t i = 0; i < length; ++i) {
    //   sendData[i] = dataToSend[i];
    // }
    // // 设置特征的值并发送通知
    // pTxCharacteristic->setValue(sendData, length);
    // pTxCharacteristic->notify();
    // // 释放之前分配的内存
    // free(sendData);
    // sendData = NULL;
    // //小程序传输http，已弃用
    

    // 在配置文件中找不到对应的MP3文件,改为通过HTTP在线播放
    // //若找不到就在线播放
    // Serial.println("MP3 file not found, playing online...");
    //audio.connecttohost(HttpGet); //  128k mp
    //downloadMp3FileToSD(HttpGet,"/file11.mp3");
    //audio.connecttoFS(SD, "/file11.mp3");
    // Serial.println("下载完毕的SD卡文件列表：");
    // listDir(SD, "/", 0);
    // 创建子任务执行下载操作
      // if (downloadTaskHandle == NULL) {
    //  xTaskCreate(downloadMp3Task, "DownloadMP3", 4096, NULL, 2, &downloadTaskHandle);
    // Serial.println("Downloade compete：");
    // listDir(SD, "/", 0);
     
  //    }

      // // 通知子任务开始下载
      // xTaskNotify(downloadTaskHandle, 1, eSetValueWithOverwrite);

      // // 等待下载完成
      // ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    }
    }  
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// // 写入 UID 到配置文件的方法
// void appendUidToConfigFile(const String& uidString) {
//   File file = SD.open("/config.txt", FILE_APPEND);
//   if (file) {
//     // 先写入一个空行
//     file.println();
//     // 写入 UID
//     file.println(uidString);
//     file.close();
//     Serial.println("UID appended to config file");
//   } else {
//     Serial.println("Failed to open config file for writing UID");
//   }
// }

// // 写入文件名到配置文件的方法（带空格）
// void appendFileNameToConfigFile(const String& fileName) {
//   File file = SD.open("/config.txt", FILE_APPEND);
//   if (file) {
//     // 写入空格和文件名
//     file.print(" ");
//     file.println(fileName);
//     file.close();
//     Serial.println("File name appended to config file");
//   } else {
//     Serial.println("Failed to open config file for writing file name");
//   }
// }

// //写入uid和filename
void appendToConfigFile(const String& uidString, const String& fileName) {
  File file = SD.open("/config.txt", FILE_APPEND);
  if (file) {
    file.println(uidString+ " " + fileName);
    file.close();
    Serial.println("Data appended to config file");
  } else {
    Serial.println("Failed to open config file for writing");
  }
}

void setup() {
  //SPI初始化
  Wire.setPins(SDA_PIN,SCL_PIN);
  Wire.begin();
  Serial.begin(115200);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  //SD卡初始化
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  //图片放大器初始化
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(3); // 0...21    控制音量

  // digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW

 
  Init_uid();  //随机生成uuid
  if (!SD.begin(SD_CS)) 
  {
  Serial.println("SD卡初始化失败！");
    return;
  }
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) 
  {
    Serial.print("Didn't find PN53x board");
    while (1);
  }
  nfc.SAMConfig();
  Serial.println("Waiting for NFC card...");
  configFile = SD.open("/config.txt", FILE_READ);
  if (!configFile) {
    Serial.println("无法打开配置文件！");
    return;
  }
  else
  {
    Serial.println("打开配置文件成功！");
  }
  // //查看wifi配置文件
  // // 打开WifiConfig.txt文件
  // File configFile = SD.open("/WifiConfig.txt", FILE_READ);
  // if (configFile) {
  //   String configContent = configFile.readString();  // 读取全部内容
  //   configFile.close();
  //   if (configContent.length() > 0)
  //   {
  //   // 查找并分割字符串以获取WiFi SSID和密码
  //   int delimiterPos = configContent.indexOf(" ");
  //   if (delimiterPos != -1) {
  //     // 用于存储WiFi SSID和密码的变量
  //     String wifiSSID;
  //     String wifiPassword;
  //     wifiSSID = configContent.substring(0, delimiterPos);
  //     wifiPassword = configContent.substring(delimiterPos + 1);
  //     wifiSSID.trim();  // 去掉首尾空格
  //     wifiPassword.trim(); 
  //     Serial.println("WiFi SSID: " + wifiSSID);
  //     Serial.println("WiFi Password: " + wifiPassword);
  //     connectToWiFi(wifiSSID.c_str(), wifiPassword.c_str());
  //     } 
  //   }
  //   else {
  //     Serial.println("WifiConfig.txt is empty.");
  //   }
  // } else {
  //   Serial.println("Failed to open WifiConfig.txt.");
  // }     

  //  // 打开WifiConfig.txt文件
  // File configFile = SD.open("/WifiConfig.txt", FILE_READ);
  // if (configFile) {
  //   String wifiSSID, wifiPassword;
  //   // 读取WiFi SSID
  //   wifiSSID = configFile.readStringUntil('\r');
  //   wifiSSID.trim();  // 去掉首尾空格
  //   // 读取WiFi密码，跳过换行符
  //   configFile.seek(0);  // 重置文件指针到文件开始位置
  //   wifiPassword = configFile.readStringUntil('\r');
  //   wifiPassword.trim();  // 去掉首尾空格
  //   configFile.close();
  //   if (wifiSSID.length() > 0 && wifiPassword.length() > 0) {
  //     Serial.println("WiFi SSID: " + wifiSSID);
  //     Serial.println("WiFi Password: " + wifiPassword);
  //     connectToWiFi(wifiSSID.c_str(), wifiPassword.c_str());
  //   } else {
  //     Serial.println("WifiConfig.txt is empty or not properly formatted.");
  //   }
  // } else {
  //   Serial.println("Failed to open WifiConfig.txt.");
  // }

//                           测试http连接直接连接wifi
  // WiFi.begin(ssid, password);
  // while(WiFi.status() != WL_CONNECTED)
  // {
  //   delay(500);
  //   Serial.print(".");
  // }
  // Serial.print("Wificonnect!!!!");

  //downloadMp3FileToSD(HttpGet);
  //xTaskCreate(ReadNfcMatchMP3Task, "NfcTask", 4096, NULL, 2, NULL);
  //audio.connecttohost(HttpGet); //  128k mp在线播放
  xTaskCreate(ReadNfcMatchMP3, "ReadNfcMatchMP3", 1024*6, NULL, 2, &ReadNfcMatchMP3Handle);
  // if (WiFi.status() == WL_CONNECTED) {
  //       Serial.println("WiFi connected");
  //       audio.connecttoFS(SD,"/WiFisuccess.mp3");

  //   } else {
  //       Serial.println("NoWificonnected");
  //       audio.connecttoFS(SD,"/WiFifail.mp3");
  //   }
}

String readString;

void loop()
{


    audio.loop();
    if(Serial.available()){ // put streamURL in serial monitor
        audio.stopSong();
        String r=Serial.readString(); r.trim();
        if(r.length()>5) audio.connecttohost(r.c_str());
        log_i("free heap=%i", ESP.getFreeHeap());
    }
  while (Serial.available() > 0) {
    if (deviceConnected) {
      delay(3);
      readString += Serial.read();
      pTxCharacteristic->setValue(chipId.c_str());
//      pTxCharacteristic->setValue((uint32_t)ESP.getEfuseMac());
      pTxCharacteristic->notify();
      Serial.println(chipId);
      //writeWifiConfigToSD();
    }
  }
  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
    Serial.println("BLUETOOTH");
  }
  // Writewifi_flag == 0 &&
  if (!hasWrittenConfig)
  { 
    Serial.println("Writing Wificonfig new!");
    writeWifiConfigToSD();
    printWifiFileContents();
    //除非重新写入配置文件，不然不会重复执行
    hasWrittenConfig=true;
  }
}