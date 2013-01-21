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



/*************************************************************************
 *
 *    ATTENTION
 *    This file is intended to supply the types and defines for Environment independent
 *    building of the packethandler
 *    Only adaption of this file should be necessary. Else it is a bug!
 *    Ether including apropriate files or defining the types when not available in the environment
 *
 ************************************************************************/

/** defines al least types
    comm_BYTE
    comm_WORD;
    comm_DWORD;
    comm_BOOL
    comm_ULONG
    comm_USHORT
    comm_UINT16
    comm_UINT32
    comm_UniChar
**/

#ifndef _COMMTYPES_HXX
#define _COMMTYPES_HXX

#include <sal/types.h>
typedef sal_Unicode comm_UniChar;
#include "rtl/string.hxx"

//#define DBG_ASSERT( cond, text )
//#define DBG_ERROR( text )
#include <tools/debug.hxx>

#include <tools/solar.h>
typedef sal_uInt8 comm_BYTE;
typedef sal_Bool comm_BOOL;
typedef sal_uInt32 comm_ULONG;
typedef sal_uInt16 comm_USHORT;
typedef sal_uInt16 comm_UINT16;
typedef sal_uInt32 comm_UINT32;

typedef sal_uInt16      comm_WORD;
typedef sal_uInt32      comm_DWORD;

namespace rtl { class OString; }
class String;
#define comm_String String

#endif
