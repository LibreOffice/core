/*************************************************************************
 *
 *  $RCSfile: networkdomain.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2002-08-27 13:56:32 $
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

#ifndef __FRAMEWORK_HELPER_NETWORKDOMAIN_HXX_
#include <helper/networkdomain.hxx>
#endif

namespace framework
{

#ifdef WNT
//_________________________________________________________________________________________________________________
//  Windows
//_________________________________________________________________________________________________________________

#define UNICODE
#include <windows.h>

//_________________________________________________________________________________________________________________
//  Win NT, Win 2000, Win XP
//_________________________________________________________________________________________________________________

static DWORD WINAPI GetUserDomainW_NT( LPWSTR lpBuffer, DWORD nSize )
{
    return GetEnvironmentVariable( TEXT("USERDOMAIN"), lpBuffer, nSize );
}

//_________________________________________________________________________________________________________________
//  Win 9x,Win ME
//_________________________________________________________________________________________________________________

static DWORD WINAPI GetUserDomainW_WINDOWS( LPWSTR lpBuffer, DWORD nSize )
{
    HKEY    hkeyLogon;
    HKEY    hkeyWorkgroup;
    DWORD   dwResult = 0;


    if ( ERROR_SUCCESS  == RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        TEXT("Network\\Logon"),
        0, KEY_READ, &hkeyLogon ) )
    {
        DWORD   dwLogon = 0;
        DWORD   dwLogonSize = sizeof(dwLogon);
        LONG    lResult = RegQueryValueEx( hkeyLogon, TEXT("LMLogon"), 0, NULL, (LPBYTE)&dwLogon, &dwLogonSize );
        RegCloseKey( hkeyLogon );

        if ( dwLogon )
        {
            HKEY    hkeyNetworkProvider;

            if ( ERROR_SUCCESS  == RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                TEXT("SYSTEM\\CurrentControlSet\\Services\\MSNP32\\NetworkProvider"),
                0, KEY_READ, &hkeyNetworkProvider ) )
            {
                DWORD   dwBufferSize = nSize;
                LONG    lResult = RegQueryValueEx( hkeyNetworkProvider, TEXT("AuthenticatingAgent"), 0, NULL, (LPBYTE)lpBuffer, &dwBufferSize );

                if ( ERROR_SUCCESS == lResult || ERROR_MORE_DATA == lResult )
                    dwResult = dwBufferSize / sizeof(TCHAR);

                RegCloseKey( hkeyNetworkProvider );
            }
        }
    }
    else if ( ERROR_SUCCESS == RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        TEXT("SYSTEM\\CurrentControlSet\\Services\\VxD\\VNETSUP"),
        0, KEY_READ, &hkeyWorkgroup ) )
    {
        DWORD   dwBufferSize = nSize;
        LONG    lResult = RegQueryValueEx( hkeyWorkgroup, TEXT("Workgroup"), 0, NULL, (LPBYTE)lpBuffer, &dwBufferSize );

        if ( ERROR_SUCCESS == lResult || ERROR_MORE_DATA == lResult )
            dwResult = dwBufferSize / sizeof(TCHAR);

        RegCloseKey( hkeyWorkgroup );
    }


    return dwResult;
}

static rtl::OUString GetUserDomain()
{
    sal_Unicode aBuffer[256];

    long    nVersion = GetVersion();
    DWORD   nResult;

    if ( nVersion < 0 )
        nResult = GetUserDomainW_WINDOWS( aBuffer, sizeof( aBuffer ) );
    else
        nResult = GetUserDomainW_NT( aBuffer, sizeof( aBuffer ) );

    if ( nResult > 0 )
        return rtl::OUString( aBuffer );
    else
        return rtl::OUString();
}

//_________________________________________________________________________________________________________________
//  Windows
//_________________________________________________________________________________________________________________

rtl::OUString NetworkDomain::GetYPDomainName()
{
    return ::rtl::OUString();
}

rtl::OUString NetworkDomain::GetNTDomainName()
{
    return GetUserDomain();
}

#elif defined( UNIX )

#include <rtl/ustring.h>
#include <stdlib.h>
#include <errno.h>
#include <osl/thread.h>

//_________________________________________________________________________________________________________________
//  Unix
//_________________________________________________________________________________________________________________

#if defined( SOLARIS )

//_________________________________________________________________________________________________________________
//  Solaris
//_________________________________________________________________________________________________________________

#include <sys/systeminfo.h>
#include <alloca.h>

static rtl_uString *getDomainName()
{
    /* Initialize and assume failure */
    rtl_uString *ustrDomainName = NULL;

    char    szBuffer[256];

    long    nCopied = sizeof(szBuffer);
    char    *pBuffer = szBuffer;
    long    nBufSize;

    do
    {
        nBufSize = nCopied;
        nCopied = sysinfo( SI_SRPC_DOMAIN, pBuffer, nBufSize );

        /*  If nCopied is greater than buffersize we need to allocate
            a buffer with suitable size */

        if ( nCopied > nBufSize )
            pBuffer = (char *)alloca( nCopied );

    } while ( nCopied > nBufSize );

    if ( -1 != nCopied  )
    {
        rtl_string2UString(
            &ustrDomainName,
            pBuffer,
            nCopied - 1,
            osl_getThreadTextEncoding(),
            OSTRING_TO_OUSTRING_CVTFLAGS );
    }

    return ustrDomainName;
}

#elif defined( LINUX ) /* endif SOLARIS */

//_________________________________________________________________________________________________________________
//  Linux
//_________________________________________________________________________________________________________________

#include <unistd.h>
#include <string.h>

static rtl_uString *getDomainName()
{
    /* Initialize and assume failure */
    rtl_uString *ustrDomainName = NULL;

    char    *pBuffer;
    int     result;
    size_t  nBufSize = 0;

    do
    {
        nBufSize += 256; /* Increase buffer size by steps of 256 bytes */
        pBuffer = (char *)alloca( nBufSize );
        result = getdomainname( pBuffer, nBufSize );
        /* If buffersize in not large enough -1 is returned and errno
        is set to EINVAL. This only applies to libc. With glibc the name
        is truncated. */
    } while ( -1 == result && EINVAL == errno );

    if ( 0 == result )
    {
        rtl_string2UString(
            &ustrDomainName,
            pBuffer,
            strlen( pBuffer ),
            osl_getThreadTextEncoding(),
            OSTRING_TO_OUSTRING_CVTFLAGS );
    }

    return ustrDomainName;
}

#else /* LINUX */

//_________________________________________________________________________________________________________________
//  Other Unix
//_________________________________________________________________________________________________________________

static rtl_uString *getDomainName()
{
    return NULL;
}

#endif

//_________________________________________________________________________________________________________________
//  Unix
//_________________________________________________________________________________________________________________

rtl::OUString NetworkDomain::GetYPDomainName()
{
    rtl_uString* pResult = getDomainName();
    if ( pResult )
        return rtl::OUString( pResult );
    else
        return rtl::OUString();
}

rtl::OUString NetworkDomain::GetNTDomainName()
{
    return ::rtl::OUString();
}

#else /* UNIX */

//_________________________________________________________________________________________________________________
//  Other operating systems (non-Windows and non-Unix)
//_________________________________________________________________________________________________________________

rtl::OUString NetworkDomain::GetYPDomainName()
{
    return rtl::OUString();
}

rtl::OUString NetworkDomain::GetNTDomainName()
{
    return rtl::OUString();
}

#endif

} // namespace framework
