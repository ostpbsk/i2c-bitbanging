#ifndef __I2C_BITBANG_H
#define __I2C_BITBANG_H

#include "stm32f1xx_hal.h"
#include "tim.h"
#include "main.h"
#include <stdbool.h>

#define I2C_DELAY_TIME 5


//Inline delay function using TIM1 prescaled timer
static inline void I2C_Delay() {
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    while (__HAL_TIM_GET_COUNTER(&htim1) < I2C_DELAY_TIME);
}


// I2C Pin Rises and Falls
#define SDA_SET() HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_SET)
#define SDA_RESET() HAL_GPIO_WritePin(SDA_GPIO_Port, SDA_Pin, GPIO_PIN_RESET)
#define SCL_SET() HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_SET)
#define SCL_RESET() HAL_GPIO_WritePin(SCL_GPIO_Port, SCL_Pin, GPIO_PIN_RESET)
#define SDA_READ() HAL_GPIO_ReadPin(SDA_GPIO_Port, SDA_Pin)
#define SCL_READ() HAL_GPIO_ReadPin(SCL_GPIO_Port, SCL_Pin)

// I2C Communication funciton Prototypes
void I2C_Init(); // Call this function before any I2C operation in main.c
void I2C_Start_Cond();
void I2C_Stop_Cond();
bool I2C_Send(uint8_t data);
bool I2C_SendData(uint8_t address, uint8_t *data, uint8_t length);
uint8_t I2C_Read_Byte(bool ACK);
bool I2C_Send_Byte(uint8_t data);
uint8_t* I2C_ReceiveData(uint8_t address, uint8_t *data, uint8_t length);



#endif