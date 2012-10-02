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


#include "rtl/ustrbuf.hxx"

#include "wininetbackend.hxx"

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#include <wininet.h>
#include <sal/alloca.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#define WININET_DLL_NAME "wininet.dll"
#define EQUAL_SIGN '='
#define COLON      ':'
#define SPACE      ' '
#define SEMI_COLON ';'

namespace {

struct Library {
    HMODULE module;

    Library(HMODULE theModule): module(theModule) {}

    ~Library() { if (module) FreeLibrary(module); }
};

}

typedef struct
{
    rtl::OUString Server;
    rtl::OUString Port;
} ProxyEntry;

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace // private
{
    ProxyEntry ReadProxyEntry(const rtl::OUString& aProxy, sal_Int32& i)
    {
        ProxyEntry aProxyEntry;

        aProxyEntry.Server = aProxy.getToken( 0, COLON, i );
        if ( i > -1 )
            aProxyEntry.Port = aProxy.getToken( 0, COLON, i );

        return aProxyEntry;
    }

    ProxyEntry FindProxyEntry(const rtl::OUString& aProxyList, const rtl::OUString& aType)
    {
        sal_Int32 nIndex = 0;

        do
        {
            // get the next token, e.g. ftp=server:port
            rtl::OUString nextToken = aProxyList.getToken( 0, SPACE, nIndex );

            // split the next token again into the parts separated
            // through '=', e.g. ftp=server:port -> ftp and server:port
            sal_Int32 i = 0;
            if( nextToken.indexOf( EQUAL_SIGN ) > -1 )
            {
                if( aType.equals( nextToken.getToken( 0, EQUAL_SIGN, i ) ) )
                    return ReadProxyEntry(nextToken, i);
            }
            else if( aType.getLength() == 0)
                return ReadProxyEntry(nextToken, i);

        } while ( nIndex >= 0 );

        return ProxyEntry();
    }

} // end private namespace

//------------------------------------------------------------------------------

WinInetBackend::WinInetBackend()
{
    Library hWinInetDll( LoadLibrary( WININET_DLL_NAME ) );
    if( hWinInetDll.module )
    {
        typedef BOOL ( WINAPI *InternetQueryOption_Proc_T )( HINTERNET, DWORD, LPVOID, LPDWORD );

        InternetQueryOption_Proc_T lpfnInternetQueryOption =
            reinterpret_cast< InternetQueryOption_Proc_T >(
                GetProcAddress( hWinInetDll.module, "InternetQueryOptionA" ) );
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
            DWORD dwLength = sizeof (INTERNET_PROXY_INFO);
            BOOL ok = lpfnInternetQueryOption(
                NULL,
                INTERNET_OPTION_PROXY,
                (LPVOID)lpi,
                &dwLength );
            if (!ok)
            {
                DWORD err = GetLastError();
                if (err == ERROR_INSUFFICIENT_BUFFER)
                {
                    // allocate sufficient space on the heap
                    // insufficient space on the heap results
                    // in a stack overflow exception, we assume
                    // this never happens, because of the relatively
                    // small amount of memory we need
                    // alloca is nice because it is fast and we don't
                    // have to free the allocated memory, it will be
                    // automatically done
                    lpi = reinterpret_cast< LPINTERNET_PROXY_INFO >(
                        alloca( dwLength ) );
                    ok = lpfnInternetQueryOption(
                        NULL,
                        INTERNET_OPTION_PROXY,
                        (LPVOID)lpi,
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

            rtl::OUString aProxyList       = rtl::OUString::createFromAscii( lpi->lpszProxy );
            rtl::OUString aProxyBypassList = rtl::OUString::createFromAscii( lpi->lpszProxyBypass );

            // override default for ProxyType, which is "0" meaning "No proxies".
            sal_Int32 nProperties = 1;

            valueProxyType_.IsPresent = true;
            valueProxyType_.Value <<= nProperties;

            // fill proxy bypass list
            if( aProxyBypassList.getLength() > 0 )
            {
                rtl::OUStringBuffer aReverseList;
                sal_Int32 nIndex = 0;
                do
                {
                    rtl::OUString aToken = aProxyBypassList.getToken( 0, SPACE, nIndex );
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
                //-------------------------------------------------
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
                //-------------------------------------------------

                ProxyEntry aTypeIndepProxy = FindProxyEntry( aProxyList, rtl::OUString());
                ProxyEntry aHttpProxy = FindProxyEntry( aProxyList, rtl::OUString(
                    "http"  ) );
                ProxyEntry aHttpsProxy  = FindProxyEntry( aProxyList, rtl::OUString(
                    "https"  ) );

                ProxyEntry aFtpProxy  = FindProxyEntry( aProxyList, rtl::OUString(
                    "ftp"  ) );

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

//------------------------------------------------------------------------------

WinInetBackend::~WinInetBackend(void)
{
}

//------------------------------------------------------------------------------

WinInetBackend* WinInetBackend::createInstance()
{
    return new WinInetBackend;
}

// ---------------------------------------------------------------------------------------

void WinInetBackend::setPropertyValue(
    rtl::OUString const &, css::uno::Any const &)
    throw (
        css::beans::UnknownPropertyException, css::beans::PropertyVetoException,
        css::lang::IllegalArgumentException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    throw css::lang::IllegalArgumentException(
        rtl::OUString(
            "setPropertyValue not supported"),
        static_cast< cppu::OWeakObject * >(this), -1);
}

css::uno::Any WinInetBackend::getPropertyValue(
    rtl::OUString const & PropertyName)
    throw (
        css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
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

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL WinInetBackend::getBackendName(void) {
    return rtl::OUString("com.sun.star.comp.configuration.backend.WinInetBackend") ;
}

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL WinInetBackend::getImplementationName(void)
    throw (uno::RuntimeException)
{
    return getBackendName() ;
}

//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL WinInetBackend::getBackendServiceNames(void)
{
    uno::Sequence<rtl::OUString> aServiceNameList(1);
    aServiceNameList[0] = rtl::OUString( "com.sun.star.configuration.backend.WinInetBackend") ;

    return aServiceNameList ;
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL WinInetBackend::supportsService(const rtl::OUString& aServiceName)
    throw (uno::RuntimeException)
{
    uno::Sequence< rtl::OUString > const svc = getBackendServiceNames();

    for(sal_Int32 i = 0; i < svc.getLength(); ++i )
        if(svc[i] == aServiceName)
            return true;

    return false;
}

//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL WinInetBackend::getSupportedServiceNames(void)
    throw (uno::RuntimeException)
{
    return getBackendServiceNames() ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
