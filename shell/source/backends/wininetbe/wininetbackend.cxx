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

#include <cppuhelper/supportsservice.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

#include "wininetbackend.hxx"

#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <wininet.h>
#include <sal/alloca.h>

#define WININET_DLL_NAME L"wininet.dll"
#define EQUAL_SIGN '='
#define COLON      ':'
#define SPACE      ' '
#define SEMI_COLON ';'

namespace {

struct Library {
    HMODULE module;

    explicit Library(HMODULE theModule): module(theModule) {}

    ~Library() { if (module) FreeLibrary(module); }
};

}

struct ProxyEntry
{
    OUString Server;
    OUString Port;
};


namespace
{
    ProxyEntry ReadProxyEntry(const OUString& aProxy, sal_Int32& i)
    {
        ProxyEntry aProxyEntry;

        aProxyEntry.Server = aProxy.getToken( 0, COLON, i );
        if ( i > -1 )
            aProxyEntry.Port = aProxy.getToken( 0, COLON, i );

        return aProxyEntry;
    }

    ProxyEntry FindProxyEntry(const OUString& aProxyList, const OUString& aType)
    {
        sal_Int32 nIndex = 0;

        do
        {
            // get the next token, e.g. ftp=server:port
            OUString nextToken = aProxyList.getToken( 0, SPACE, nIndex );

            // split the next token again into the parts separated
            // through '=', e.g. ftp=server:port -> ftp and server:port
            sal_Int32 i = 0;
            if( nextToken.indexOf( EQUAL_SIGN ) > -1 )
            {
                if( aType.equals( nextToken.getToken( 0, EQUAL_SIGN, i ) ) )
                    return ReadProxyEntry(nextToken, i);
            }
            else if( aType.isEmpty())
                return ReadProxyEntry(nextToken, i);

        } while ( nIndex >= 0 );

        return ProxyEntry();
    }

} // unnamed namespace

WinInetBackend::WinInetBackend()
{
    Library hWinInetDll( LoadLibraryW( WININET_DLL_NAME ) );
    if( hWinInetDll.module )
    {
        typedef BOOL ( WINAPI *InternetQueryOption_Proc_T )( HINTERNET, DWORD, LPVOID, LPDWORD );

        InternetQueryOption_Proc_T lpfnInternetQueryOption =
            reinterpret_cast< InternetQueryOption_Proc_T >(
                GetProcAddress( hWinInetDll.module, "InternetQueryOptionW" ) );
        if (lpfnInternetQueryOption)
        {
            // Some Windows versions would fail the InternetQueryOption call
            // with ERROR_OUTOFMEMORY when the initial dwLength were zero (and
            // are apparently fine with the initial sizeof (INTERNET_PROXY_INFO)
            // and need no reallocation), while other versions fail with
            // ERROR_INSUFFICIENT_BUFFER upon that initial dwLength and need a
            // reallocation:
            INTERNET_PROXY_INFO pi;
            LPINTERNET_PROXY_INFO lpi = &pi;
            DWORD dwLength = sizeof (pi);
            BOOL ok = lpfnInternetQueryOption(
                nullptr,
                INTERNET_OPTION_PROXY,
                lpi,
                &dwLength );
            if (!ok)
            {
                DWORD err = GetLastError();
                if (err == ERROR_INSUFFICIENT_BUFFER)
                {
                    // allocate sufficient space on the stack
                    // insufficient space on the stack results
                    // in a stack overflow exception, we assume
                    // this never happens, because of the relatively
                    // small amount of memory we need
                    // alloca is nice because it is fast and we don't
                    // have to free the allocated memory, it will be
                    // automatically done
                    lpi = static_cast< LPINTERNET_PROXY_INFO >(
                        alloca( dwLength ) );
                    ok = lpfnInternetQueryOption(
                        nullptr,
                        INTERNET_OPTION_PROXY,
                        lpi,
                        &dwLength );
                    if (!ok)
                    {
                        err = GetLastError();
                    }
                }
                if (!ok)
                {
                    SAL_WARN(
                        "shell",
                        "InternetQueryOption INTERNET_OPTION_PROXY"
                            " GetLastError=" << err);
                    return;
                }
            }

            // if a proxy is disabled, InternetQueryOption returns
            // an empty proxy list, so we don't have to check if
            // proxy is enabled or not

            // We use InternetQueryOptionW (see https://msdn.microsoft.com/en-us/library/aa385101);
            // it fills INTERNET_PROXY_INFO struct which is defined in WinInet.h to have LPCTSTR
            // (i.e., the UNICODE-dependent generic string type expanding to const wchar_t* when
            // UNICODE is defined, and InternetQueryOption macro expands to InternetQueryOptionW).
            // Thus, it's natural to expect that W version would return wide strings. But it's not
            // true. The W version still returns const char* in INTERNET_PROXY_INFO.
            OUString aProxyList       = OUString::createFromAscii( lpi->lpszProxy );
            OUString aProxyBypassList = OUString::createFromAscii( lpi->lpszProxyBypass );

            // override default for ProxyType, which is "0" meaning "No proxies".
            valueProxyType_.IsPresent = true;
            valueProxyType_.Value <<= sal_Int32(1);

            // fill proxy bypass list
            if( aProxyBypassList.getLength() > 0 )
            {
                OUStringBuffer aReverseList;
                sal_Int32 nIndex = 0;
                do
                {
                    OUString aToken = aProxyBypassList.getToken( 0, SPACE, nIndex );
                    if ( aProxyList.indexOf( aToken ) == -1 )
                    {
                        if ( aReverseList.getLength() )
                        {
                            aReverseList.insert( 0, sal_Unicode( SEMI_COLON ) );
                            aReverseList.insert( 0, aToken );
                        }
                        else
                            aReverseList = aToken;
                    }
                }
                while ( nIndex >= 0 );

                aProxyBypassList = aReverseList.makeStringAndClear();

                valueNoProxy_.IsPresent = true;
                valueNoProxy_.Value <<= aProxyBypassList.replace( SPACE, SEMI_COLON );
            }

            if( aProxyList.getLength() > 0 )
            {

                // this implementation follows the algorithm
                // of the internet explorer
                // if there are type-dependent proxy settings
                // and type independent proxy settings in the
                // registry the internet explorer chooses the
                // type independent proxy for all settings
                // e.g. imagine the following registry entry
                // ftp=server:port;http=server:port;server:port
                // the last token server:port is type independent
                // so the ie chooses this proxy server

                // if there is no port specified for a type independent
                // server the ie uses the port of an http server if
                // there is one and it has a port


                ProxyEntry aTypeIndepProxy = FindProxyEntry( aProxyList, OUString());
                ProxyEntry aHttpProxy = FindProxyEntry( aProxyList, "http" );
                ProxyEntry aHttpsProxy  = FindProxyEntry( aProxyList, "https" );

                ProxyEntry aFtpProxy  = FindProxyEntry( aProxyList, "ftp" );

                if( aTypeIndepProxy.Server.getLength() )
                {
                    aHttpProxy.Server = aTypeIndepProxy.Server;
                    aHttpsProxy.Server  = aTypeIndepProxy.Server;
                    aFtpProxy.Server  = aTypeIndepProxy.Server;

                    if( aTypeIndepProxy.Port.getLength() )
                    {
                        aHttpProxy.Port = aTypeIndepProxy.Port;
                        aHttpsProxy.Port  = aTypeIndepProxy.Port;
                        aFtpProxy.Port  = aTypeIndepProxy.Port;
                    }
                    else
                    {
                        aFtpProxy.Port  = aHttpProxy.Port;
                        aHttpsProxy.Port  = aHttpProxy.Port;
                    }
                }

                // http proxy name
                if( aHttpProxy.Server.getLength() > 0 )
                {
                    valueHttpProxyName_.IsPresent = true;
                    valueHttpProxyName_.Value <<= aHttpProxy.Server;
                }

                // http proxy port
                if( aHttpProxy.Port.getLength() > 0 )
                {
                    valueHttpProxyPort_.IsPresent = true;
                    valueHttpProxyPort_.Value <<= aHttpProxy.Port.toInt32();
                }

                // https proxy name
                if( aHttpsProxy.Server.getLength() > 0 )
                {
                    valueHttpsProxyName_.IsPresent = true;
                    valueHttpsProxyName_.Value <<= aHttpsProxy.Server;
                }

                // https proxy port
                if( aHttpsProxy.Port.getLength() > 0 )
                {
                    valueHttpsProxyPort_.IsPresent = true;
                    valueHttpsProxyPort_.Value <<= aHttpsProxy.Port.toInt32();
                }

                // ftp proxy name
                if( aFtpProxy.Server.getLength() > 0 )
                {
                    valueFtpProxyName_.IsPresent = true;
                    valueFtpProxyName_.Value <<= aFtpProxy.Server;
                }

                // ftp proxy port
                if( aFtpProxy.Port.getLength() > 0 )
                {
                    valueFtpProxyPort_.IsPresent = true;
                    valueFtpProxyPort_.Value <<= aFtpProxy.Port.toInt32();
                }
            }
        }
    }
}

WinInetBackend::~WinInetBackend()
{
}

WinInetBackend* WinInetBackend::createInstance()
{
    return new WinInetBackend;
}

void WinInetBackend::setPropertyValue(
    OUString const &, css::uno::Any const &)
{
    throw css::lang::IllegalArgumentException(
        "setPropertyValue not supported",
        static_cast< cppu::OWeakObject * >(this), -1);
}

css::uno::Any WinInetBackend::getPropertyValue(
    OUString const & PropertyName)
{
    if ( PropertyName == "ooInetFTPProxyName" )
    {
        return css::uno::makeAny(valueFtpProxyName_);
    } else if ( PropertyName == "ooInetFTPProxyPort" )
    {
        return css::uno::makeAny(valueFtpProxyPort_);
    } else if ( PropertyName == "ooInetHTTPProxyName" )
    {
        return css::uno::makeAny(valueHttpProxyName_);
    } else if ( PropertyName == "ooInetHTTPProxyPort" )
    {
        return css::uno::makeAny(valueHttpProxyPort_);
    } else if ( PropertyName == "ooInetHTTPSProxyName" )
    {
        return css::uno::makeAny(valueHttpsProxyName_);
    } else if ( PropertyName == "ooInetHTTPSProxyPort" )
    {
        return css::uno::makeAny(valueHttpsProxyPort_);
    } else if ( PropertyName == "ooInetNoProxy" )
    {
        return css::uno::makeAny(valueNoProxy_);
    } else if ( PropertyName == "ooInetProxyType" )
    {
        return css::uno::makeAny(valueProxyType_);
    } else {
        throw css::beans::UnknownPropertyException(
            PropertyName, static_cast< cppu::OWeakObject * >(this));
    }
}

OUString WinInetBackend::getBackendName() {
    return OUString("com.sun.star.comp.configuration.backend.WinInetBackend") ;
}

OUString SAL_CALL WinInetBackend::getImplementationName()
{
    return getBackendName() ;
}

uno::Sequence<OUString> WinInetBackend::getBackendServiceNames()
{
    uno::Sequence<OUString> aServiceNameList { "com.sun.star.configuration.backend.WinInetBackend" };

    return aServiceNameList ;
}

sal_Bool SAL_CALL WinInetBackend::supportsService(const OUString& aServiceName)
{
    return cppu::supportsService(this, aServiceName);
}

uno::Sequence<OUString> SAL_CALL WinInetBackend::getSupportedServiceNames()
{
    return getBackendServiceNames() ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
