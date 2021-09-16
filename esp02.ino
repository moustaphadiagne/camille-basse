#include<ESP8266WiFi.h>
#include<WiFiClient.h>
#include "EmonLib.h"             // Include Emon Library
#include <PZEM004Tv30.h>
#include <InfluxDb.h>

EnergyMonitor emon1;    //instance pour le ZMPT101B
PZEM004Tv30 pzem(D5, D6);	//initialisation objet premier pzem GPIO 14 GPIO 12
PZEM004Tv30 pzemm(D7, D4); 	// deuxieme pzem GPIO 13 GPIO 2
PZEM004Tv30 pzemmm(D1, D2);	//troisieme pzem GPIO 5 GPIO 4
const char* ssid = "RUT955_DEDA";//Replace with your network SSID
const char* password = "1234567890";//Replace with your network password
int s=1;  //compteur
float total=0;
float moy =0;
int debit=0;     //debit en m3/heures
float debits=0;  //debit en m3/secondes
unsigned int timer=0;  //conteur du temps de marche de la pompe
int loopCount = 0;  //compteur qui ne depasse pas 40 secondes sans envoyer les donnees
float volume=0;  //volume en m3

float voltage,current,pf,frequency,power,voltage2,voltage3,current2,current3,pf2,pf3,frequency2,frequency3,power2,power3;
#define VOLT_CAL 535   //calibrage transfo de tension ZMPT101B
#define INFLUXDB_HOST "185.98.128.237"   //adresse ip serveur grafana/influxdb
#define INFLUXDB_PORT 48086    //port du serveur
Influxdb influx(INFLUXDB_HOST,INFLUXDB_PORT);  //requete de connexion
void setup() 
{
Serial.begin(115200);
emon1.voltage(0, VOLT_CAL, 1.7);
WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED)
  {
  delay(500);
  Serial.print(".");
  }
Serial.println(WiFi.localIP());  //affichage de l adresse ip de l esp
influx.setDb("bnsp");  //connexion a la base de donnees 
Serial.println("configuration terminee.");
}
void loop() 
{
voltage = pzem.voltage();
//current = pzem.current();
//power = pzem.power();
frequency = pzem.frequency();
pf = pzem.pf();

voltage2 = pzemm.voltage();
//current2 = pzemm.current();
//power2 = pzemm.power();
frequency2 = pzemm.frequency();
pf2 = pzemm.pf();

voltage3 = pzemmm.voltage();
//current3 = pzemmm.current();
//power3 = pzemmm.power();
frequency3 = pzemmm.frequency();
pf3 = pzemmm.pf();
emon1.calcVI(25,1000);         // Calculate all. No.of half wavelengths (crossings), time-out
float supplyVoltage   = emon1.Vrms;             //la valeur moyenne du signal en analogique cad sortie transfo
total=total+supplyVoltage;
moy=total/s;
if(s ==40)
{Serial.print("moyenne ");  
 Serial.println(moy);

voltage=voltage*sqrt(3);
voltage2=voltage2*sqrt(3);
voltage3=voltage3*sqrt(3);
moy=moy*sqrt(3);
InfluxData row("pompe_2");
  row.addTag("location", "camille_basse");
  row.addValue("voltage", voltage);
  row.addValue("voltage2", voltage2);
  row.addValue("voltage3", voltage3);
  //row.addValue("voltage4", moy);
  row.addValue("frequence", frequency);
  //row.addValue("facteur", pf);
  //row.addValue("timer", timer);
influx.write(row); //ecriture dans la base de donnees
InfluxData row2("pompe_4");
row2.addTag("location", "camille_basse");
row2.addValue("voltage2", moy);
influx.write(row2); 

s=0;
total=0;
}
Serial.println(voltage);
Serial.println(voltage2);
Serial.println(voltage3);
//Serial.println(pf);
//Serial.println(pf2);
//Serial.println(pf3);
Serial.print("zmpt101b : ");  
Serial.println(supplyVoltage);      //tension zmpt101b
Serial.println(moy);
Serial.println(frequency);Serial.println(frequency2);
Serial.println(frequency3);
//Serial.println(pf);//Serial.println(pf2);
//Serial.println(pf3);
Serial.println("-----------------------------");

s=s+1;
delay(1000);
}