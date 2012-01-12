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



#pragma once

#ifdef _UWINAPI_
#   define _KERNEL32_
#   define _USER32_
#   define _SHELL32_
#endif

#ifndef _WINDOWS_
#ifdef _MSC_VER
#   pragma warning(push,1) /* disable warnings within system headers */
#endif
#   include <windows.h>
#ifdef _MSC_VER
#   pragma warning(pop)
#endif
#endif

#ifdef __MINGW32__
#include <basetyps.h>
#ifdef _UWINAPI_
#define WINBASEAPI
#endif
#endif

/** GetUserDomain

The GetUserDomain function retrieves the name of the NT domain the user is
logged in.

Parameters
    @param lpBuffer
        [out] Pointer to a buffer that receives a null-terminated string
        containing the domain name.
    @param nBufferSize
        [in] Specifies the size, in TCHARs, of the buffer pointed to
        by the lpBuffer parameter.


Return Values
    @return
    If the function succeeds, the return value is the number of TCHARs stored
    into the buffer pointed to by lpBuffer, not including the terminating
    null character.

    If the domain name can't be retrieved, the return value is zero.

    If the buffer pointed to by lpBuffer is not large enough, the return value
    is the buffer size, in TCHARs, required to hold the value string and its
    terminating null character.

Remarks
    Windows 95/98/Me: If the user is not logged in onto a NT domain server
    the name of the workgroup is returned.

Requirements
  Windows NT/2000/XP: Included in Windows NT 4 and later.
  Windows 95/98/Me: Included in Windows 95 and later.
  Header: Declared in Uwinapi.h; include Uwinapi.h.
  Library: Use Uwinapi.lib.
  Unicode: Implemented as Unicode and ANSI versions on Windows 95/98/Me/NT/2000/XP.

See Also
@see
*/

EXTERN_C WINBASEAPI DWORD WINAPI GetUserDomainA( LPSTR lpBuffer, DWORD nBuffserSize );
EXTERN_C WINBASEAPI DWORD WINAPI GetUserDomainW( LPWSTR lpBuffer, DWORD nBuffserSize );

#ifdef UNICODE
#define GetUserDomain   GetUserDomainW
#else
#define GetUserDomain   GetUserDomainA
#endif

EXTERN_C WINBASEAPI DWORD WINAPI GetProcessId( HANDLE hProcess );

/* macro that calculates the count of elements of a static array */

#define elementsof(buf) (sizeof(buf) / sizeof((buf)[0]))

#ifdef __cplusplus

inline bool IsValidHandle(HANDLE handle)
{
    return  handle != INVALID_HANDLE_VALUE && handle != NULL;
}

#else   /* __cplusplus */

#define IsValidHandle(Handle)   ((DWORD)(Handle) + 1 > 1)

#endif  /* __cplusplus */

