/*************************************************************************
 *
 *  $RCSfile: uwinapi.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 16:45:59 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma once

#ifdef _UWINAPI_
#   define _KERNEL32_
#   define _USER32_
#   define _SHELL32_
#endif

#ifndef _WINDOWS_
#   include <windows.h>
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

