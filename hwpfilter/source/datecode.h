/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef __DATECODE_H__
#define __DATACODE_H__

static hchar defaultform[] =
{
    '1', 0x9165, 32, '2', 0xB6A9, 32, '3', 0xB7A9, 0
};
#ifdef _DATECODE_WEEK_DEFINES_
static hchar kor_week[] =
{
    0xB7A9, 0xB6A9, 0xD1C1, 0xAE81, 0xA1A2, 0x8B71, 0xC9A1
};
static hchar china_week[] =
{
    0x4CC8, 0x4BE4, 0x525A, 0x48D8, 0x45AB, 0x4270, 0x50B4
};
static char eng_week[] = { "SunMonTueWedThuFriSat" };
static char eng_mon[] = { "JanFebMarAprMayJunJulAugSepOctNovDec" };
static const char *en_mon[] =
{
    "January", "February", "March", "April", "May", "June", "July",
    "August", "September", "October", "November", "December"
};
static const char *en_week[] =
{
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"
};
#endif //_DATECODE_WEEK_DEFINES_
#endif
