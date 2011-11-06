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



#ifndef SC_FLTTYPES_HXX
#define SC_FLTTYPES_HXX

enum BiffTyp
{
    BiffX = 0x0000,
    Biff2 = 0x2000, Biff2M = 0x2002, Biff2C = 0x2004,
    Biff3 = 0x3000, Biff3W = 0x3001, Biff3M = 0x3002, Biff3C = 0x3004,
    Biff4 = 0x4000, Biff4W = 0x4001, Biff4M = 0x4002, Biff4C = 0x4004,
    Biff5 = 0x5000, Biff5W = 0x5001, Biff5V = 0x5002, Biff5C = 0x5004, Biff5M4 = 0x5008,
    Biff8 = 0x8000, Biff8W = 0x8001, Biff8V = 0x8002, Biff8C = 0x8004, Biff8M4 = 0x8008
};

enum Lotus123Typ
{
    Lotus_X,
    Lotus_WK1,
    Lotus_WK3,
    Lotus_WK4,
    Lotus_FM3
};

#endif

