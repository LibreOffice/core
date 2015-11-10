/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <helper/networkdomain.hxx>

#ifdef WNT

//  Windows

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

static OUString GetUserDomain()
{
    sal_Unicode aBuffer[256];
    DWORD   nResult;

    nResult = GetUserDomainW_NT( reinterpret_cast<LPWSTR>(aBuffer), sizeof( aBuffer ) );

    if ( nResult > 0 )
        return OUString( aBuffer );
    else
        return OUString();
}

//  Windows

namespace framework
{

OUString NetworkDomain::GetYPDomainName()
{
    return OUString();
}

OUString NetworkDomain::GetNTDomainName()
{
    return GetUserDomain();
}

}

#elif defined( UNIX )

#include <rtl/ustring.h>
#include <stdlib.h>
#include <errno.h>
#include <osl/thread.h>

//  Unix

#if defined( SOLARIS )

//  Solaris

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

//  Linux

#include <unistd.h>
#include <string.h>

static rtl_uString *getDomainName()
{
    /* Initialize and assume failure */
    rtl_uString *ustrDomainName = nullptr;

    char    buffer[256]; // actually the man page says 65 bytes should be enough

    /* If buffersize is not large enough the name is truncated. */
    int const result = getdomainname( &buffer[0], SAL_N_ELEMENTS(buffer) );

    if ( 0 == result )
    {
        rtl_string2UString(
            &ustrDomainName,
            &buffer[0],
            strlen( &buffer[0] ),
            osl_getThreadTextEncoding(),
            OSTRING_TO_OUSTRING_CVTFLAGS );
    }

    return ustrDomainName;
}

#else /* LINUX */

//  Other Unix

static rtl_uString *getDomainName()
{
    return NULL;
}

#endif

//  Unix

namespace framework
{

OUString NetworkDomain::GetYPDomainName()
{
    rtl_uString* pResult = getDomainName();
    if ( pResult )
        return OUString( pResult );
    else
        return OUString();
}

OUString NetworkDomain::GetNTDomainName()
{
    return OUString();
}

}

#else /* UNIX */

//  Other operating systems (non-Windows and non-Unix)

namespace framework
{

OUString NetworkDomain::GetYPDomainName()
{
    return OUString();
}

OUString NetworkDomain::GetNTDomainName()
{
    return OUString();
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
