/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x 
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "board_horus.h"

void i2cInit(void)
{
  I2C_DeInit(I2C);

  I2C_InitTypeDef I2C_InitStructure;
  I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_OwnAddress1 = 0x00;
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_Init(I2C, &I2C_InitStructure);
  I2C_Cmd(I2C, ENABLE);

  GPIO_PinAFConfig(I2C_GPIO, I2C_GPIO_PinSource_SCL, I2C_GPIO_AF);
  GPIO_PinAFConfig(I2C_GPIO, I2C_GPIO_PinSource_SDA, I2C_GPIO_AF);

  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitStructure.GPIO_Pin = I2C_GPIO_PIN_SCL | I2C_GPIO_PIN_SDA;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(I2C_GPIO, &GPIO_InitStructure);
}

#define I2C_TIMEOUT_MAX 1000
bool I2C_WaitEvent(uint32_t event)
{
  uint32_t timeout = I2C_TIMEOUT_MAX;
  while (!I2C_CheckEvent(I2C, event)) {
    if ((timeout--) == 0) return false;
  }
  return true;
}

bool I2C_WaitEventCleared(uint32_t event)
{
  uint32_t timeout = I2C_TIMEOUT_MAX;
  while (I2C_CheckEvent(I2C, event)) {
    if ((timeout--) == 0) return false;
  }
  return true;
}

int16_t i2cReadRegister(uint8_t address, uint8_t index)
{
  if (!I2C_WaitEventCleared(I2C_FLAG_BUSY))
    return -1;

  I2C_GenerateSTART(I2C, ENABLE);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    return -2;

  I2C_Send7bitAddress(I2C, address, I2C_Direction_Transmitter);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    return -3;

  I2C_SendData(I2C, index);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    return -4;

  I2C_GenerateSTART(I2C, ENABLE);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    return -5;

  I2C_Send7bitAddress(I2C, address, I2C_Direction_Receiver);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    return -6;

  I2C_AcknowledgeConfig(I2C, DISABLE);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))
    return -7;

  uint8_t result = I2C_ReceiveData(I2C);

  I2C_GenerateSTOP(I2C, ENABLE);

  return result;
}

void i2cWriteRegister(uint8_t address, uint8_t index, uint8_t data)
{
  if (!I2C_WaitEventCleared(I2C_FLAG_BUSY))
    return;

  I2C_GenerateSTART(I2C, ENABLE);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    return;

  I2C_Send7bitAddress(I2C, address, I2C_Direction_Transmitter);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    return;

  I2C_SendData(I2C, index);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    return;

  I2C_SendData(I2C, data);
  if (!I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    return;

  I2C_GenerateSTOP(I2C, ENABLE);
}

void setScaledVolume(uint8_t volume)
{
  if (volume > VOLUME_LEVEL_MAX) {
    volume = VOLUME_LEVEL_MAX;
  }

  setVolume(volumeScale[volume]);
}

void setVolume(uint8_t volume)
{
  i2cWriteRegister(I2C_ADDRESS_VOLUME, 0, volume);
}

int32_t getVolume()
{
  return i2cReadRegister(I2C_ADDRESS_VOLUME, 0);
}