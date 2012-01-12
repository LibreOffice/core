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



#ifndef _SV_SALCONST_H
#define _SV_SALCONST_H

// -------------------
// - Constants -
// -------------------

static const unsigned short kByteMask = 0xFF;

static const unsigned short kOneByte  =  8;
static const unsigned short kTwoBytes = 16;

static const unsigned short kOneBit     =  1;
static const unsigned short kFiveBits   =  5;
static const unsigned short kEightBits  =  8;
static const unsigned short kTenBits    = 10;
static const unsigned short kElevenBits = 11;

static const unsigned short kBlackAndWhite  =  1;
static const unsigned short kFourBitColor   =  4;
static const unsigned short kEightBitColor  =  8;
static const unsigned short kThousandsColor = 16;
static const unsigned short kTrueColor      = 32;

static const unsigned long k16BitRedColorMask   = 0x00007c00;
static const unsigned long k16BitGreenColorMask = 0x000003e0;
static const unsigned long k16BitBlueColorMask  = 0x0000001f;

static const unsigned long k32BitRedColorMask   = 0x00ff0000;
static const unsigned long k32BitGreenColorMask = 0x0000ff00;
static const unsigned long k32BitBlueColorMask  = 0x000000ff;

static const unsigned short kPixMapCmpSizeOneBit    =  1;
static const unsigned short kPixMapCmpSizeFourBits  =  4;
static const unsigned short kPixMapCmpSizeFiveBits  =  5;
static const unsigned short kPixMapCmpSizeEightBits =  8;

static const long kPixMapHRes = 72;
static const long kPixMapVRes = 72;

#endif // _SV_SALCONST_H
