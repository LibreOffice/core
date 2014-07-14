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


#ifndef _STRCONVERT_H_
#define _STRCONVERT_H_

#include <windows.h>

#ifdef NDEBUG
#define STRCONVERT_H_HAD_NDEBUG
#undef NDEBUG
#endif
#if OSL_DEBUG_LEVEL == 0
#define NDEBUG
#endif
#include <assert.h>

#ifdef __cplusplus
extern "C"{
#endif

int  AllocNecessarySpaceAndCopyWStr2Str( LPCWSTR lpcwstrString, LPSTR* lppStr );
int  AllocSpaceAndCopyWStr2Str( LPCWSTR lpcwstrString, DWORD nWCharsToCopy, LPSTR* lppStr );
int  CalcLenDblNullTerminatedWStr( LPCWSTR lpcwstrString );
int  CalcLenDblNullTerminatedStr( LPCSTR lpcstrString );
void FreeSpaceStr( LPSTR lpszString );

/* WC2MB allocates a sufficient amount of memory on stack and converts
   the wide char parameter to multi byte string using the actual code
   page.

   @Param: wcStr - a wide char string
           mbStr - the corresponding multi byte string

   NOTE: due to the use of _alloca, this must be a macro and no function
*/

#define WC2MB( wcStr, mbStr ) \
if( wcStr ) \
{ \
    int needed = WideCharToMultiByte( CP_ACP, 0, wcStr, -1, NULL, 0, NULL, NULL ); \
    if( needed > 0 ) \
    { \
        int copied; \
        mbStr = _alloca( needed * sizeof( CHAR ) ); \
        copied = WideCharToMultiByte( CP_ACP, 0, wcStr, -1, mbStr, needed, NULL, NULL ); \
        assert( copied == needed ); \
    } \
}


/* WideCharListGetMultiByteLength
   calculates the needed length of a corresponding the multi byte string
   list for a wide char string list.

   @Param: cp - the code page to use for conversion.
           wcList - a double '\0' terminated wide char string list.
*/

int WideCharListGetMultiByteLength( UINT codepage, LPCWSTR wcList );

/* WideCharListToMultiByteList
   converts a double '\0' terminated list of wide char strings to a
   multi byte string list.

   @Param: cp - the code page to use for conversion.
           wcList - a double '\0' terminated wide char string list.
           mbList - a double '\0' terminated multi byte string list.
           dwSize - size of buffer for multi byte string list.
*/

int WideCharListToMultiByteList( UINT codepage, LPCWSTR wcList, LPSTR mbList, DWORD dwSize );


/* WCL2MBL allocates a sufficient amount of memory on stack and converts
   the wide char list parameter to multi byte string list using the actual
   code page.

   @Param: wcList - a wide char string list
           mbList - the corresponding multi byte string list

   NOTE: due to the use of _alloca, this must be a macro and no function
*/

#define WCL2MBL( wcList, mbList ) \
if( wcList ) \
{ \
    int needed = WideCharListGetMultiByteLength( CP_ACP, wcList ); \
    if( needed > 0 ) \
    { \
        int copied; \
        mbList = _alloca( needed * sizeof( CHAR ) ); \
        copied = WideCharListToMultiByteList( CP_ACP, wcList, mbList, needed ); \
        assert( copied == needed ); \
    } \
}

#ifdef __cplusplus
}
#endif

// Restore NDEBUG state
#ifdef STRCONVERT_H_HAD_NDEBUG
#define NDEBUG
#else
#undef NDEBUG
#endif

#endif
