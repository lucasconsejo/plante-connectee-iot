/*
 * Main.cpp
 * Connected plant - 2019
 * by Lucas Consejo & Théo Ferreira
 * Ingesup B2B - Ynov Bordeaux
 */
#include "mbed.h"
#include "zest-radio-atzbrf233.h"
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"

NetworkInterface *net;

static I2C i2c(I2C1_SDA, I2C1_SCL);
static AnalogIn analogic(ADC_IN1);

uint8_t lm75_adress = 0x48 << 1;

namespace {
#define PERIOD_MS 500
}

// function that returns the temperature
float temperature(){
	char cmd[2];
	cmd[0]= 0x00;
	i2c.write(lm75_adress, cmd, 1);
	i2c.read(lm75_adress, cmd, 2);

	return ((cmd[0] << 8)| cmd[1] >> 7)*0.5;
}

// function that returns humidity
float humidity(){
	return analogic.read()*3.3*100.0/3.3;
}

// main function
int main()
{
	while(true){
		float temperature_value = temperature();
		printf("The temperature is %f \n", temperature_value);

		float humidity_value = humidity();
		printf("Humidity is %f \n\n", humidity_value);

		ThisThread::sleep_for(PERIOD_MS);
	}
}
