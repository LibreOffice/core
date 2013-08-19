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

#include "sal/config.h"

#include <cstring>

#include "rtl/process.h"
#include "rtl/bootstrap.hxx"
#include "rtl/random.h"
#include "rtl/string.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/uri.hxx"
#include "osl/diagnose.h"
#include "osl/file.hxx"
#include "osl/security.hxx"
#include "osl/thread.hxx"

#include "cppuhelper/bootstrap.hxx"
#include "cppuhelper/findsofficepath.h"

#include "com/sun/star/uno/XComponentContext.hpp"

#include "com/sun/star/bridge/UnoUrlResolver.hpp"
#include "com/sun/star/bridge/XUnoUrlResolver.hpp"

#include "macro_expander.hxx"

#define ARLEN(x) sizeof (x) / sizeof *(x)

using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

using rtl::Bootstrap;
using rtl::OUString;

namespace cppu
{

BootstrapException::BootstrapException()
{
}

BootstrapException::BootstrapException( const ::rtl::OUString & rMessage )
    :m_aMessage( rMessage )
{
}

BootstrapException::BootstrapException( const BootstrapException & e )
{
    m_aMessage = e.m_aMessage;
}

BootstrapException::~BootstrapException()
{
}

BootstrapException & BootstrapException::operator=( const BootstrapException & e )
{
    m_aMessage = e.m_aMessage;
    return *this;
}

const ::rtl::OUString & BootstrapException::getMessage() const
{
    return m_aMessage;
}

Reference< XComponentContext > SAL_CALL bootstrap()
{
    Reference< XComponentContext > xRemoteContext;

    try
    {
        char const * p1 = cppuhelper_detail_findSofficePath();
        if (p1 == NULL) {
            throw BootstrapException(
                "no soffice installation found!");
        }
        rtl::OUString p2;
        if (!rtl_convertStringToUString(
                &p2.pData, p1, std::strlen(p1), osl_getThreadTextEncoding(),
                (RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR |
                 RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_ERROR |
                 RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR)))
        {
            throw BootstrapException(
                "bad characters in soffice installation path!");
        }
        OUString path;
        if (osl::FileBase::getFileURLFromSystemPath(p2, path) !=
            osl::FileBase::E_None)
        {
            throw BootstrapException(
                "cannot convert soffice installation path to URL!");
        }
        if (!path.isEmpty() && path[path.getLength() - 1] != '/') {
            path += "/";
        }

        OUString uri;
        if (!Bootstrap::get("URE_BOOTSTRAP", uri)) {
            Bootstrap::set(
                "URE_BOOTSTRAP",
                Bootstrap::encode(path + SAL_CONFIGFILE("fundamental")));
        }

        // create default local component context
        Reference< XComponentContext > xLocalContext(
            defaultBootstrap_InitialComponentContext() );
        if ( !xLocalContext.is() )
            throw BootstrapException( "no local component context!" );

        // create a random pipe name
        rtlRandomPool hPool = rtl_random_createPool();
        if ( hPool == 0 )
            throw BootstrapException( "cannot create random pool!" );
        sal_uInt8 bytes[ 16 ];
        if ( rtl_random_getBytes( hPool, bytes, ARLEN( bytes ) )
            != rtl_Random_E_None )
            throw BootstrapException( "random pool error!" );
        rtl_random_destroyPool( hPool );
        ::rtl::OUStringBuffer buf;
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "uno" ) );
        for ( sal_uInt32 i = 0; i < ARLEN( bytes ); ++i )
            buf.append( static_cast< sal_Int32 >( bytes[ i ] ) );
        OUString sPipeName( buf.makeStringAndClear() );

        // accept string
        OSL_ASSERT( buf.isEmpty() );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "--accept=pipe,name=" ) );
        buf.append( sPipeName );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( ";urp;" ) );

        // arguments
        OUString args [] = {
            OUString("--nologo"),
            OUString("--nodefault"),
            OUString("--norestore"),
            OUString("--nocrashreport"),
            OUString("--nolockcheck"),
            buf.makeStringAndClear()
        };
        rtl_uString * ar_args [] = {
            args[ 0 ].pData,
            args[ 1 ].pData,
            args[ 2 ].pData,
            args[ 3 ].pData,
            args[ 4 ].pData,
            args[ 5 ].pData
        };
        ::osl::Security sec;

        // start office process
        oslProcess hProcess = 0;
        oslProcessError rc = osl_executeProcess(
            OUString(path + "soffice").pData, ar_args, ARLEN( ar_args ),
            osl_Process_DETACHED,
            sec.getHandle(),
            0, // => current working dir
            0, 0, // => no env vars
            &hProcess );
        switch ( rc )
        {
            case osl_Process_E_None:
                osl_freeProcessHandle( hProcess );
                break;
            case osl_Process_E_NotFound:
                throw BootstrapException( "image not found!" );
            case osl_Process_E_TimedOut:
                throw BootstrapException( "timout occurred!" );
            case osl_Process_E_NoPermission:
                throw BootstrapException( "permission denied!" );
            case osl_Process_E_Unknown:
                throw BootstrapException( "unknown error!" );
            case osl_Process_E_InvalidError:
            default:
                throw BootstrapException( "unmapped error!" );
        }

        // create a URL resolver
        Reference< bridge::XUnoUrlResolver > xUrlResolver(
            bridge::UnoUrlResolver::create( xLocalContext ) );

        // connection string
        OSL_ASSERT( buf.isEmpty() );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM( "uno:pipe,name=" ) );
        buf.append( sPipeName );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
            ";urp;StarOffice.ComponentContext" ) );
        OUString sConnectString( buf.makeStringAndClear() );

        // wait until office is started
        for ( ; ; )
        {
            try
            {
                // try to connect to office
                xRemoteContext.set(
                    xUrlResolver->resolve( sConnectString ), UNO_QUERY_THROW );
                break;
            }
            catch ( connection::NoConnectException & )
            {
                // wait 500 ms, then try to connect again
                TimeValue tv = { 0 /* secs */, 500000000 /* nanosecs */ };
                ::osl::Thread::wait( tv );
            }
        }
    }
    catch ( Exception & e )
    {
        throw BootstrapException(
            "unexpected UNO exception caught: " + e.Message );
    }

    return xRemoteContext;
}

OUString bootstrap_expandUri(OUString const & uri) {
    static char const PREFIX[] = "vnd.sun.star.expand:";
    return uri.matchAsciiL(RTL_CONSTASCII_STRINGPARAM(PREFIX))
        ? cppuhelper::detail::expandMacros(
            rtl::Uri::decode(
                uri.copy(RTL_CONSTASCII_LENGTH(PREFIX)),
                rtl_UriDecodeWithCharset, RTL_TEXTENCODING_UTF8))
        : uri;
}

} // namespace cppu

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
