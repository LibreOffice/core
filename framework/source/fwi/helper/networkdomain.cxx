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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"
#include <helper/networkdomain.hxx>

namespace framework
{

#ifdef WNT
//_________________________________________________________________________________________________________________
//  Windows
//_________________________________________________________________________________________________________________

#define UNICODE
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

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
        RegQueryValueEx( hkeyLogon, TEXT("LMLogon"), 0, NULL, (LPBYTE)&dwLogon, &dwLogonSize );
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
        nResult = GetUserDomainW_WINDOWS( reinterpret_cast<LPWSTR>(aBuffer), sizeof( aBuffer ) );
    else
        nResult = GetUserDomainW_NT( reinterpret_cast<LPWSTR>(aBuffer), sizeof( aBuffer ) );

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
#include <sal/alloca.h>

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
