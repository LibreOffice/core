/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#pragma once
#include <sal/macros.h>
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

#ifdef __cplusplus

inline bool IsValidHandle(HANDLE handle)
{
    return  handle != INVALID_HANDLE_VALUE && handle != NULL;
}

#else   /* __cplusplus */

#define IsValidHandle(Handle)   ((DWORD)(Handle) + 1 > 1)

#endif  /* __cplusplus */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
