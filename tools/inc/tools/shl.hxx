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



#ifndef _SHL_HXX
#define _SHL_HXX

#include <tools/solar.h>
#include "tools/toolsdllapi.h"

// ----------------
// - GetAppData() -
// ----------------

#define SHL_SHL1        0
#define SHL_SHL2        1
#define SHL_SHL3        2
#define SHL_APP1        3
#define SHL_APP2        4
#define SHL_APP3        5
#define SHL_TOOLS       6
#define SHL_SV          7
#define SHL_SVT         8
#define SHL_SVDDE       9
#define SHL_ERR         10
#define SHL_IPC         11
#define SHL_SVX         12
#define SHL_ITEM        13
#define SHL_SVD         14
#define SHL_SI          15
#define SHL_SFC         16
#define SHL_SFX         17
//#define SHL_SO2       18
#define SHL_IDL         19
#define SHL_IDE         20
#define SHL_EDIT        21
#define SHL_VCED        22
#define SHL_BASIC       23
#define SHL_HELP        24
#define SHL_LNKCCH      25
#define SHL_CHANNEL     26
#define SHL_SBX         27
#define SHL_SBC         28
#define SHL_SDB         29
#define SHL_SBA         30
#define SHL_SBA2        31
#define SHL_SCH         32
#define SHL_SIM         33
#define SHL_SM          34
#define SHL_SGA         35
#define SHL_DOCMGR      36
// 37 removed
#define SHL_MAIL        38
#define SHL_NEWS        39
#define SHL_OFFAPP      40
#define SHL_INET        41
#define SHL_CHAOS       42
#define SHL_HISTORY     43
#define SHL_SJ          44
#define SHL_CALC        45
#define SHL_DRAW        46
#define SHL_WRITER      47
#define SHL_MONEY       48
#define SHL_HBCI        49
#define SHL_BASE3D      50
#define SHL_BRUSHITEM   51
#define SHL_SFONTITEM   52

// #110743#
// #define SHL_COUNT        53
// the following added for binary filter project
// Sice an array is created for that values, it is necessary to
// put them directly behind the defined ones. Else, some space is
// wasted.
#define BF_SHL_SVX      53
#define BF_SHL_ITEM     54
#define BF_SHL_SVD      55
#define BF_SHL_EDIT     56
#define BF_SHL_SCH      57
#define BF_SHL_SM       58
#define BF_SHL_CALC     59
#define BF_SHL_DRAW     60
#define BF_SHL_WRITER   61
#define BF_SHL_SVT      62
#define BF_SHL_SVDDE    63

#define SHL_COUNT       64

TOOLS_DLLPUBLIC void** GetAppData( sal_uInt16 nSharedLib );

#endif // _SHL_HXX
