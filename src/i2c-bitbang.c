#include "i2c-bitbang.h"

/***
 * @brief I2C Initialisation
 * @details Initialises the I2C pins to high state, as the I2C bus is idle when both SDA and SCL are high
 * @note Call this function before any I2C operation
 */ 
void I2C_Init() {
    SDA_SET();
    SCL_SET();
}

/***
 * @brief I2C Start Condition
 * @details Generates a start condition on the I2C bus by pulling SDA low while SCL is high, Initiating the I2C communication
 */

void I2C_Start_Cond() {
    SDA_RESET();
    I2C_Delay();
    SCL_RESET();
    I2C_Delay();
}

/***
 * @brief I2C Stop Condition
 * @details Generates a stop condition on the I2C bus by pulling SDA high while SCL is high, Ending the I2C communication
 */

void I2C_Stop_Cond() {
    SCL_SET();
    I2C_Delay();
    SDA_SET();
    I2C_Delay();
}

/***
 * @brief I2C Send Byte
 * @details Sends a byte of data over the I2C bus, effectively shifting out the bits one by one and comparing them with 0x80 mask
 *      to then set the SDA line accordingly (1 = high, 0 = low), and then checking for an ACK from the slave device
 * @param data The byte to be sent
 * @return ACK (true if ACK received, false otherwise)
 */

bool I2C_Send_Byte(uint8_t data) {
    for (uint8_t bit = 0; bit < 8; bit++) {
        ((data & 0x80) ? SDA_SET() : SDA_RESET()); // Set SDA high or low based on the most significant bit
        data <<= 1; // Shift left to prepare for the next bit
        // Generate clock pulse
        I2C_Delay();
        SCL_SET();
        I2C_Delay();
        SCL_RESET();
        I2C_Delay();     
    }
    // Release SDA after sending all the data for ACK/NACK
    SDA_SET();
    I2C_Delay();
    SCL_SET();
    I2C_Delay();
    bool ACK = !HAL_GPIO_ReadPin(SDA_GPIO_Port, SDA_Pin); // SDA can check for ACK/NACK because it can be pulled low by the slave due to OD config
    SCL_RESET();
    I2C_Delay();

    return ACK;
}

/***
 * @brief I2C Read Byte
 * @details Reads a byte of data from the I2C bus, utilizing the OD configuration of the SDA pin to allow the slave device to pull the line low
 *     after recieving the data, device can also send ACK/NACK to resume or stop the communication
 * @param ACK true if ACK is to be sent, false if NACK is to be sent
 * @return The byte read from the I2C bus
 */
uint8_t I2C_Read_Byte(bool ACK) {
    uint8_t data = 0;
    SDA_SET(); // Release SDA for input
    for (uint8_t bit = 0; bit < 8; bit++) {
        data <<= 1; // Shift left to read the next bit
        // Generate clock pulse with clock stretching
        SCL_SET();
        // Wait for clock stretching
        volatile uint16_t timeout = 1000;
        while (!SCL_READ()) {
            if (--timeout == 0) return 0;
        }
        I2C_Delay();
        // Read the SDA line and write it to the data variable
        if (SDA_READ()) data |= 0x01;
        I2C_Delay();
        SCL_RESET();
    }
    // Send ACK/NACK and generate clock pulse
    if (ACK) SDA_RESET();
    else SDA_SET();
    SCL_SET();
    I2C_Delay();
    SCL_RESET();
    SDA_SET(); // Release SDA for the next byte
    return data;
}


/***
 * @brief I2C Send Data
 * @details Sends a data stream over the I2C bus byte by byte, then checking for ACK after each byte
 * if address is not acknowledged, the function will return false, if slave doesnt acknowledge the data byte, it will return false
 * after sending all the data, the function will return true, indicating success
 * @param address I2C address of the slave device
 * @param data Pointer to the data buffer to be sent
 * @param length The length of the data buffer
 * @return true - transaction successful, false - transaction failed
 */

bool I2C_SendData(uint8_t address, uint8_t *data, uint8_t length) {
    I2C_Start_Cond();
    if (!I2C_Send_Byte((address << 1) | 0x00)) { // Check ACK for address
        I2C_Stop_Cond();
        return false; // Address not acknowledged
    }
    for (uint8_t i = 0; i < length; i++) {
        if (!I2C_Send_Byte(data[i])) { // Check ACK for each byte
            I2C_Stop_Cond();
            return false; // Data byte not acknowledged
        }
    }
    I2C_Stop_Cond();
    return true; // Success
}

/***
 * @brief I2C Receive Data
 * @details Recieves a data stream over the I2C bus byte by byte after sending the address
 * if address is not acknowledged, the function will return false, if slave doesnt acknowledge the data byte, it will return false
 * after receiving all the data, the function will return true, indicating success
 * @param address I2C address of the slave device
 * @param data Pointer to the data buffer to be received
 * @param length The length of the data buffer
 * @return Pointer to the data buffer
 */

uint8_t* I2C_ReceiveData(uint8_t address, uint8_t *data, uint8_t length) {
    I2C_Start_Cond();
    if (!I2C_Send_Byte((address << 1) | 0x01)) { // Check ACK for read address
        I2C_Stop_Cond();
        return 0; // Address not acknowledged
    }
    for (uint8_t i = 0; i < length; i++) {
        data[i] = I2C_Read_Byte(i < length - 1);
    }
    I2C_Stop_Cond();
    return data;
}


