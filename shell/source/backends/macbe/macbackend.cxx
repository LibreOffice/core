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
#include "precompiled_shell.hxx"

// For MAXHOSTNAMELEN constant
#include <sys/param.h>

#include <premac.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <Foundation/NSPathUtilities.h>
#include <postmac.h>

#include "macbackend.hxx"

#include "com/sun/star/beans/Optional.hpp"
#include "rtl/ustrbuf.hxx"
#include "osl/file.h"

#define SPACE      ' '
#define SEMI_COLON ';'

typedef struct
{
    rtl::OUString Server;
    sal_Int32 Port;
} ProxyEntry;

typedef enum {
    sHTTP,
    sHTTPS,
    sFTP
} ServiceType;

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace // private
{

/*
 * Returns current proxy settings for selected service type (HTTP or
 * FTP) as a C string (in the buffer specified by host and hostSize)
 * and a port number.
 */

bool GetProxySetting(ServiceType sType, char *host, size_t hostSize, UInt16 *port)
{
    bool                result;
    CFDictionaryRef     proxyDict;
    CFNumberRef         enableNum;
    int                 enable;
    CFStringRef         hostStr;
    CFNumberRef         portNum;
    int                 portInt;

    proxyDict = SCDynamicStoreCopyProxies(NULL);

    if (!proxyDict)
        return false;

    CFStringRef proxiesEnable;
    CFStringRef proxiesProxy;
    CFStringRef proxiesPort;

    switch ( sType )
    {
        case sHTTP : proxiesEnable =  kSCPropNetProxiesHTTPEnable;
                     proxiesProxy = kSCPropNetProxiesHTTPProxy;
                     proxiesPort = kSCPropNetProxiesHTTPPort;
            break;
        case sHTTPS: proxiesEnable = kSCPropNetProxiesHTTPSEnable;
                     proxiesProxy = kSCPropNetProxiesHTTPSProxy;
                     proxiesPort = kSCPropNetProxiesHTTPSPort;
            break;
        default: proxiesEnable = kSCPropNetProxiesFTPEnable;
                 proxiesProxy = kSCPropNetProxiesFTPProxy;
                 proxiesPort = kSCPropNetProxiesFTPPort;
            break;
    }
    // Proxy enabled?
    enableNum = (CFNumberRef) CFDictionaryGetValue( proxyDict,
                                                   proxiesEnable );

    result = (enableNum != NULL) && (CFGetTypeID(enableNum) == CFNumberGetTypeID());

    if (result)
        result = CFNumberGetValue(enableNum, kCFNumberIntType, &enable) && (enable != 0);

    // Proxy enabled -> get hostname
    if (result)
    {
        hostStr = (CFStringRef) CFDictionaryGetValue( proxyDict,
                                                     proxiesProxy );

        result = (hostStr != NULL) && (CFGetTypeID(hostStr) == CFStringGetTypeID());
    }

    if (result)
        result = CFStringGetCString(hostStr, host, (CFIndex) hostSize, kCFStringEncodingASCII);

    // Get proxy port
    if (result)
    {
        portNum = (CFNumberRef) CFDictionaryGetValue( proxyDict,
                                                     proxiesPort );

        result = (portNum != NULL) && (CFGetTypeID(portNum) == CFNumberGetTypeID());
    }
    else
    {
        CFRelease(proxyDict);
        return false;
    }

    if (result)
        result = CFNumberGetValue(portNum, kCFNumberIntType, &portInt);

    if (result)
        *port = (UInt16) portInt;

    if (proxyDict)
        CFRelease(proxyDict);

    if (!result)
    {
        *host = 0;
        *port = 0;
    }

    return result;
}

} // end private namespace

//------------------------------------------------------------------------------

MacOSXBackend::MacOSXBackend()
{
}

//------------------------------------------------------------------------------

MacOSXBackend::~MacOSXBackend(void)
{
}

//------------------------------------------------------------------------------

MacOSXBackend* MacOSXBackend::createInstance()
{
    return new MacOSXBackend;
}

// ---------------------------------------------------------------------------------------

rtl::OUString CFStringToOUString(const CFStringRef sOrig) {
    CFRetain(sOrig);

    CFIndex nStringLen = CFStringGetLength(sOrig)+1;

    // Allocate a c string buffer
    char sBuffer[nStringLen];

    CFStringGetCString(sOrig, sBuffer, nStringLen, kCFStringEncodingASCII);

    CFRelease(sOrig);

    return rtl::OUString::createFromAscii((sal_Char*)sBuffer);
}

rtl::OUString GetOUString( NSString* pStr )
{
    if( ! pStr )
        return rtl::OUString();
    int nLen = [pStr length];
    if( nLen == 0 )
        return rtl::OUString();

    rtl::OUStringBuffer aBuf( nLen+1 );
    aBuf.setLength( nLen );
    [pStr getCharacters: const_cast<sal_Unicode*>(aBuf.getStr())];
    return aBuf.makeStringAndClear();
}

void MacOSXBackend::setPropertyValue(
    rtl::OUString const &, css::uno::Any const &)
    throw (
        css::beans::UnknownPropertyException, css::beans::PropertyVetoException,
        css::lang::IllegalArgumentException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    throw css::lang::IllegalArgumentException(
        rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM("setPropertyValue not supported")),
        static_cast< cppu::OWeakObject * >(this), -1);
}

css::uno::Any MacOSXBackend::getPropertyValue(
    rtl::OUString const & PropertyName)
    throw (
        css::beans::UnknownPropertyException, css::lang::WrappedTargetException,
        css::uno::RuntimeException)
{
    if (PropertyName.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM("WorkPathVariable")))
    {
        rtl::OUString aDocDir;
        NSArray* pPaths = NSSearchPathForDirectoriesInDomains( NSDocumentDirectory, NSUserDomainMask, true );
        if( pPaths && [pPaths count] > 0 )
        {
            aDocDir = GetOUString( [pPaths objectAtIndex: 0] );

            rtl::OUString aDocURL;
            if( aDocDir.getLength() > 0 &&
                osl_getFileURLFromSystemPath( aDocDir.pData, &aDocURL.pData ) == osl_File_E_None )
            {
                return css::uno::makeAny(
                    css::beans::Optional< css::uno::Any >(
                        true, css::uno::makeAny( aDocURL ) ) );
            }
            else
            {
                OSL_TRACE( "user documents list contains empty file path or conversion failed" );
            }
        }
        else
        {
            OSL_TRACE( "Got nil or empty list of user document directories" );
        }
        return css::uno::makeAny(css::beans::Optional< css::uno::Any >());
    } else if (PropertyName.equalsAsciiL(
                   RTL_CONSTASCII_STRINGPARAM("ooInetFTPProxyName")))
    {
        ProxyEntry aFtpProxy;

        char host[MAXHOSTNAMELEN];
        UInt16 port;
        bool retVal;

        retVal = GetProxySetting(sFTP, host, 100, &port);

        if (retVal)
        {
            aFtpProxy.Server = rtl::OUString::createFromAscii( host );
        }

        // ftp proxy name
        if( aFtpProxy.Server.getLength() > 0 )
        {
            return css::uno::makeAny(
                css::beans::Optional< css::uno::Any >(
                    true, uno::makeAny( aFtpProxy.Server ) ) );
        }
        return css::uno::makeAny(css::beans::Optional< css::uno::Any >());
    } else if (PropertyName.equalsAsciiL(
                   RTL_CONSTASCII_STRINGPARAM("ooInetFTPProxyPort")))
    {
        ProxyEntry aFtpProxy;

        char host[MAXHOSTNAMELEN];
        UInt16 port;
        bool retVal;

        retVal = GetProxySetting(sFTP, host, 100, &port);

        if (retVal)
        {
            aFtpProxy.Port = port;
        }

        // ftp proxy port
        if( aFtpProxy.Port > 0 )
        {
            return css::uno::makeAny(
                css::beans::Optional< css::uno::Any >(
                    true, uno::makeAny( aFtpProxy.Port ) ) );
        }
        return css::uno::makeAny(css::beans::Optional< css::uno::Any >());
    } else if (PropertyName.equalsAsciiL(
                   RTL_CONSTASCII_STRINGPARAM("ooInetHTTPProxyName")))
    {
        ProxyEntry aHttpProxy;

        char host[MAXHOSTNAMELEN];
        UInt16 port;
        bool retVal;

        retVal = GetProxySetting(sHTTP, host, 100, &port);

        if (retVal)
        {
            aHttpProxy.Server = rtl::OUString::createFromAscii( host );
        }

        // http proxy name
        if( aHttpProxy.Server.getLength() > 0 )
        {
            return css::uno::makeAny(
                css::beans::Optional< css::uno::Any >(
                    true, uno::makeAny( aHttpProxy.Server ) ) );
        }
        return css::uno::makeAny(css::beans::Optional< css::uno::Any >());
    } else if (PropertyName.equalsAsciiL(
                   RTL_CONSTASCII_STRINGPARAM("ooInetHTTPProxyPort")))
    {
        ProxyEntry aHttpProxy;

        char host[MAXHOSTNAMELEN];
        UInt16 port;
        bool retVal;

        retVal = GetProxySetting(sHTTP, host, 100, &port);

        if (retVal)
        {
            aHttpProxy.Port = port;
        }

        // http proxy port
        if( aHttpProxy.Port > 0 )
        {
            return css::uno::makeAny(
                css::beans::Optional< css::uno::Any >(
                    true, uno::makeAny( aHttpProxy.Port ) ) );
        }
        return css::uno::makeAny(css::beans::Optional< css::uno::Any >());
    } else if (PropertyName.equalsAsciiL(
                   RTL_CONSTASCII_STRINGPARAM("ooInetHTTPSProxyName")))
    {
        ProxyEntry aHttpsProxy;

        char host[MAXHOSTNAMELEN];
        UInt16 port;
        bool retVal;

        retVal = GetProxySetting(sHTTPS, host, 100, &port);

        if (retVal)
        {
            aHttpsProxy.Server = rtl::OUString::createFromAscii( host );
        }

        // https proxy name
        if( aHttpsProxy.Server.getLength() > 0 )
        {
            return css::uno::makeAny(
                css::beans::Optional< css::uno::Any >(
                    true, uno::makeAny( aHttpsProxy.Server ) ) );
        }
        return css::uno::makeAny(css::beans::Optional< css::uno::Any >());
    } else if (PropertyName.equalsAsciiL(
                   RTL_CONSTASCII_STRINGPARAM("ooInetHTTPSProxyPort")))
    {
        ProxyEntry aHttpsProxy;

        char host[MAXHOSTNAMELEN];
        UInt16 port;
        bool retVal;

        retVal = GetProxySetting(sHTTPS, host, 100, &port);

        if (retVal)
        {
            aHttpsProxy.Port = port;
        }

        // https proxy port
        if( aHttpsProxy.Port > 0 )
        {
            return css::uno::makeAny(
                css::beans::Optional< css::uno::Any >(
                    true, uno::makeAny( aHttpsProxy.Port ) ) );
        }
        return css::uno::makeAny(css::beans::Optional< css::uno::Any >());
    } else if (PropertyName.equalsAsciiL(
                   RTL_CONSTASCII_STRINGPARAM("ooInetProxyType")))
    {
        // override default for ProxyType, which is "0" meaning "No proxies".
        sal_Int32 nProperties = 1;
        return css::uno::makeAny(
            css::beans::Optional< css::uno::Any >(
                true, uno::makeAny( nProperties ) ) );
    } else if (PropertyName.equalsAsciiL(
                   RTL_CONSTASCII_STRINGPARAM("ooInetNoProxy")))
    {
        rtl::OUString aProxyBypassList;

        CFArrayRef rExceptionsList;
        CFDictionaryRef rProxyDict = SCDynamicStoreCopyProxies(NULL);

        if (!rProxyDict)
            rExceptionsList = false;
        else
            rExceptionsList = (CFArrayRef) CFDictionaryGetValue(rProxyDict, kSCPropNetProxiesExceptionsList);

        if (rExceptionsList)
        {
            for (CFIndex idx = 0; idx < CFArrayGetCount(rExceptionsList); idx++)
            {
                CFStringRef rException = (CFStringRef) CFArrayGetValueAtIndex(rExceptionsList, idx);

                if (idx>0)
                    aProxyBypassList += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(";"));

                aProxyBypassList += CFStringToOUString(rException);
            }
        }

        if (rProxyDict)
            CFRelease(rProxyDict);

        // fill proxy bypass list
        if( aProxyBypassList.getLength() > 0 )
        {
            return css::uno::makeAny(
                css::beans::Optional< css::uno::Any >(
                    true,
                    uno::makeAny( aProxyBypassList.replace( SPACE, SEMI_COLON ) ) ) );
        }
        return css::uno::makeAny(css::beans::Optional< css::uno::Any >());
    } else {
        throw css::beans::UnknownPropertyException(
            PropertyName, static_cast< cppu::OWeakObject * >(this));
    }
}

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL MacOSXBackend::getBackendName(void)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.configuration.backend.MacOSXBackend"));
}

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL MacOSXBackend::getImplementationName(void)
    throw (uno::RuntimeException)
{
    return getBackendName();
}

//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL MacOSXBackend::getBackendServiceNames(void)
{
    uno::Sequence<rtl::OUString> aServiceNameList(1);
    aServiceNameList[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.backend.MacOSXBackend"));

    return aServiceNameList;
}

//------------------------------------------------------------------------------

sal_Bool SAL_CALL MacOSXBackend::supportsService(const rtl::OUString& aServiceName)
    throw (uno::RuntimeException)
{
    uno::Sequence< rtl::OUString > const svc = getBackendServiceNames();

    for(sal_Int32 i = 0; i < svc.getLength(); ++i )
        if(svc[i] == aServiceName)
            return true;

    return false;
}

//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL MacOSXBackend::getSupportedServiceNames(void)
    throw (uno::RuntimeException)
{
    return getBackendServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
