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

static DWORD WINAPI GetUserDomainW_NT( LPWSTR lpBuffer, DWORD nSize )
{
    return GetEnvironmentVariable( TEXT("USERDOMAIN"), lpBuffer, nSize );
}

static rtl::OUString GetUserDomain()
{
    sal_Unicode aBuffer[256];
    DWORD   nResult;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
