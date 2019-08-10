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


// For MAXHOSTNAMELEN constant
#include <sys/param.h>

#include <premac.h>
#include <SystemConfiguration/SystemConfiguration.h>
#include <Foundation/NSPathUtilities.h>
#include <postmac.h>

#include "macbackend.hxx"

#include <com/sun/star/beans/Optional.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <osl/file.h>

#define SPACE      ' '
#define SEMI_COLON ';'

typedef enum {
    sHTTP,
    sHTTPS,
    sFTP
} ServiceType;


namespace
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

    proxyDict = SCDynamicStoreCopyProxies(nullptr);

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
    enableNum = static_cast<CFNumberRef>(CFDictionaryGetValue( proxyDict,
                                                   proxiesEnable ));

    result = (enableNum != nullptr) && (CFGetTypeID(enableNum) == CFNumberGetTypeID());

    if (result)
        result = CFNumberGetValue(enableNum, kCFNumberIntType, &enable) && (enable != 0);

    // Proxy enabled -> get hostname
    if (result)
    {
        hostStr = static_cast<CFStringRef>(CFDictionaryGetValue( proxyDict,
                                                     proxiesProxy ));

        result = (hostStr != nullptr) && (CFGetTypeID(hostStr) == CFStringGetTypeID());
    }

    if (result)
        result = CFStringGetCString(hostStr, host, static_cast<CFIndex>(hostSize), kCFStringEncodingASCII);

    // Get proxy port
    if (result)
    {
        portNum = static_cast<CFNumberRef>(CFDictionaryGetValue( proxyDict,
                                                     proxiesPort ));

        result = (portNum != nullptr) && (CFGetTypeID(portNum) == CFNumberGetTypeID());
    }
    else
    {
        CFRelease(proxyDict);
        return false;
    }

    if (result)
        result = CFNumberGetValue(portNum, kCFNumberIntType, &portInt);

    if (result)
        *port = static_cast<UInt16>(portInt);

    if (proxyDict)
        CFRelease(proxyDict);

    if (!result)
    {
        *host = 0;
        *port = 0;
    }

    return result;
}

} // unnamed namespace

MacOSXBackend::MacOSXBackend()
{
}

MacOSXBackend::~MacOSXBackend(void)
{
}

MacOSXBackend* MacOSXBackend::createInstance()
{
    return new MacOSXBackend;
}

static OUString CFStringToOUString(const CFStringRef sOrig) {
    CFRetain(sOrig);

    CFIndex nStringLen = CFStringGetLength(sOrig)+1;

    // Allocate a c string buffer
    char sBuffer[nStringLen];

    CFStringGetCString(sOrig, sBuffer, nStringLen, kCFStringEncodingASCII);

    CFRelease(sOrig);

    return OUString::createFromAscii(sBuffer);
}

static OUString GetOUString( NSString* pStr )
{
    if( ! pStr )
        return OUString();
    int nLen = [pStr length];
    if( nLen == 0 )
        return OUString();

    OUStringBuffer aBuf( nLen+1 );
    aBuf.setLength( nLen );
    [pStr getCharacters:
     reinterpret_cast<unichar *>(const_cast<sal_Unicode*>(aBuf.getStr()))];
    return aBuf.makeStringAndClear();
}

void MacOSXBackend::setPropertyValue(
    OUString const &, css::uno::Any const &)
{
    throw css::lang::IllegalArgumentException(
        "setPropertyValue not supported",
        static_cast< cppu::OWeakObject * >(this), -1);
}

css::uno::Any MacOSXBackend::getPropertyValue(
    OUString const & PropertyName)
{
    if ( PropertyName == "WorkPathVariable" )
    {
        OUString aDocDir;
        NSArray* pPaths = NSSearchPathForDirectoriesInDomains( NSDocumentDirectory, NSUserDomainMask, true );
        if( pPaths && [pPaths count] > 0 )
        {
            aDocDir = GetOUString( [pPaths objectAtIndex: 0] );

            OUString aDocURL;
            if( aDocDir.getLength() > 0 &&
                osl_getFileURLFromSystemPath( aDocDir.pData, &aDocURL.pData ) == osl_File_E_None )
            {
                return css::uno::makeAny(
                    css::beans::Optional< css::uno::Any >(
                        true, css::uno::makeAny( aDocURL ) ) );
            }
            else
            {
                SAL_WARN("shell", "user documents list contains empty file path or conversion failed" );
            }
        }
        else
        {
            SAL_WARN("shell", "Got nil or empty list of user document directories" );
        }
        return css::uno::makeAny(css::beans::Optional< css::uno::Any >());
    } else if ( PropertyName == "ooInetFTPProxyName" )
    {
        char host[MAXHOSTNAMELEN];
        UInt16 port;
        bool retVal;

        retVal = GetProxySetting(sFTP, host, 100, &port);

        if (retVal)
        {
            auto const Server = OUString::createFromAscii( host );
            if( Server.getLength() > 0 )
            {
                return css::uno::makeAny(
                    css::beans::Optional< css::uno::Any >(
                        true, uno::makeAny( Server ) ) );
            }
        }
        return css::uno::makeAny(css::beans::Optional< css::uno::Any >());
    } else if ( PropertyName == "ooInetFTPProxyPort" )
    {
        char host[MAXHOSTNAMELEN];
        UInt16 port;
        bool retVal;

        retVal = GetProxySetting(sFTP, host, 100, &port);

        if (retVal && port > 0)
        {
            return css::uno::makeAny(
                css::beans::Optional< css::uno::Any >(
                    true, uno::makeAny( sal_Int32(port) ) ) );
        }
        return css::uno::makeAny(css::beans::Optional< css::uno::Any >());
    } else if ( PropertyName == "ooInetHTTPProxyName" )
    {
        char host[MAXHOSTNAMELEN];
        UInt16 port;
        bool retVal;

        retVal = GetProxySetting(sHTTP, host, 100, &port);

        if (retVal)
        {
            auto const Server = OUString::createFromAscii( host );
            if( Server.getLength() > 0 )
            {
                return css::uno::makeAny(
                    css::beans::Optional< css::uno::Any >(
                        true, uno::makeAny( Server ) ) );
            }
        }
        return css::uno::makeAny(css::beans::Optional< css::uno::Any >());
    } else if ( PropertyName == "ooInetHTTPProxyPort" )
    {
        char host[MAXHOSTNAMELEN];
        UInt16 port;
        bool retVal;

        retVal = GetProxySetting(sHTTP, host, 100, &port);

        if (retVal && port > 0)
        {
            return css::uno::makeAny(
                css::beans::Optional< css::uno::Any >(
                    true, uno::makeAny( sal_Int32(port) ) ) );
        }
        return css::uno::makeAny(css::beans::Optional< css::uno::Any >());
    } else if ( PropertyName == "ooInetHTTPSProxyName" )
    {
        char host[MAXHOSTNAMELEN];
        UInt16 port;
        bool retVal;

        retVal = GetProxySetting(sHTTPS, host, 100, &port);

        if (retVal)
        {
            auto const Server = OUString::createFromAscii( host );
            if( Server.getLength() > 0 )
            {
                return css::uno::makeAny(
                    css::beans::Optional< css::uno::Any >(
                        true, uno::makeAny( Server ) ) );
            }
        }
        return css::uno::makeAny(css::beans::Optional< css::uno::Any >());
    } else if ( PropertyName == "ooInetHTTPSProxyPort" )
    {
        char host[MAXHOSTNAMELEN];
        UInt16 port;
        bool retVal;

        retVal = GetProxySetting(sHTTPS, host, 100, &port);

        if (retVal && port > 0)
        {
            return css::uno::makeAny(
                css::beans::Optional< css::uno::Any >(
                    true, uno::makeAny( sal_Int32(port) ) ) );
        }
        return css::uno::makeAny(css::beans::Optional< css::uno::Any >());
    } else if ( PropertyName == "ooInetProxyType" )
    {
        // override default for ProxyType, which is "0" meaning "No proxies".
        return css::uno::makeAny(
            css::beans::Optional< css::uno::Any >(
                true, uno::makeAny( sal_Int32(1) ) ) );
    } else if ( PropertyName == "ooInetNoProxy" )
    {
        OUString aProxyBypassList;

        CFArrayRef rExceptionsList;
        CFDictionaryRef rProxyDict = SCDynamicStoreCopyProxies(nullptr);

        if (!rProxyDict)
            rExceptionsList = nullptr;
        else
            rExceptionsList = static_cast<CFArrayRef>(CFDictionaryGetValue(rProxyDict, kSCPropNetProxiesExceptionsList));

        if (rExceptionsList)
        {
            for (CFIndex idx = 0; idx < CFArrayGetCount(rExceptionsList); idx++)
            {
                CFStringRef rException = static_cast<CFStringRef>(CFArrayGetValueAtIndex(rExceptionsList, idx));

                if (idx>0)
                    aProxyBypassList += ";";

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

OUString MacOSXBackend::getBackendName(void)
{
    return "com.sun.star.comp.configuration.backend.MacOSXBackend";
}

OUString SAL_CALL MacOSXBackend::getImplementationName(void)
{
    return getBackendName();
}

uno::Sequence<OUString> MacOSXBackend::getBackendServiceNames(void)
{
    uno::Sequence<OUString> aServiceNameList { "com.sun.star.configuration.backend.MacOSXBackend" };

    return aServiceNameList;
}

sal_Bool SAL_CALL MacOSXBackend::supportsService(const OUString& aServiceName)
{
    return cppu::supportsService(this, aServiceName);
}

uno::Sequence<OUString> SAL_CALL MacOSXBackend::getSupportedServiceNames(void)
{
    return getBackendServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
