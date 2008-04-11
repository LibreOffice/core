/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: macbelayer.cxx,v $
 * $Revision: 1.6 $
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

#include "macbelayer.hxx"

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

MacOSXLayer::MacOSXLayer( const uno::Reference<uno::XComponentContext>& xContext)
{
    //Create instance of LayerContentDescriber Service
    rtl::OUString const k_sLayerDescriberService(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.configuration.backend.LayerDescriber"));

    typedef uno::Reference<backend::XLayerContentDescriber> LayerDescriber;
    uno::Reference< lang::XMultiComponentFactory > xServiceManager = xContext->getServiceManager();
    if( xServiceManager.is() )
    {
        m_xLayerContentDescriber = LayerDescriber::query(
            xServiceManager->createInstanceWithContext(k_sLayerDescriberService, xContext));
    }
    else
    {
        OSL_TRACE("Could not retrieve ServiceManager");
    }

}

//------------------------------------------------------------------------------

rtl::OUString CFStringToOUString(const CFStringRef sOrig) {
    CFRetain(sOrig);

    CFIndex nStringLen = CFStringGetLength(sOrig)+1;

    // Allocate a c string buffer
    char sBuffer[nStringLen];

    CFStringGetCString(sOrig, sBuffer, nStringLen, kCFStringEncodingASCII);

    CFRelease(sOrig);

    return rtl::OUString::createFromAscii((sal_Char*)sBuffer);
}

void SAL_CALL MacOSXLayer::readData(
    const uno::Reference<backend::XLayerHandler>& xHandler)
    throw ( backend::MalformedDataException,
            lang::NullPointerException,
            lang::WrappedTargetException,
            uno::RuntimeException)
{

    if (m_xLayerContentDescriber.is())
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
                    aProxyBypassList += rtl::OUString::createFromAscii( ";" );

                aProxyBypassList += CFStringToOUString(rException);
            }
        }

        if (rProxyDict)
            CFRelease(rProxyDict);

        // override default for ProxyType, which is "0" meaning "No proxies".
        // CAUTION: if you add properties, please increase the sequence size here !
        uno::Sequence<backend::PropertyInfo> aPropInfoList(8);
        sal_Int32 nProperties = 1;

        aPropInfoList[0].Name = rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Inet/Settings/ooInetProxyType") );
        aPropInfoList[0].Type = rtl::OUString(
            RTL_CONSTASCII_USTRINGPARAM( "int" ) );
        aPropInfoList[0].Protected = sal_False;
        aPropInfoList[0].Value = uno::makeAny( nProperties );

        // fill proxy bypass list
        if( aProxyBypassList.getLength() > 0 )
        {
            aPropInfoList[nProperties].Name = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Inet/Settings/ooInetNoProxy") );
            aPropInfoList[nProperties].Type = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "string" ) );
            aPropInfoList[nProperties].Protected = sal_False;
            aPropInfoList[nProperties++].Value = uno::makeAny( aProxyBypassList.replace( SPACE, SEMI_COLON ) );
        }

        ProxyEntry aHttpProxy;
        ProxyEntry aHttpsProxy;
        ProxyEntry aFtpProxy;

        char host[MAXHOSTNAMELEN];
        UInt16 port;
        bool retVal;

        retVal = GetProxySetting(sHTTP, host, 100, &port);

        if (retVal)
        {
            aHttpProxy.Server = rtl::OUString::createFromAscii( host );
            aHttpProxy.Port = port;
        }

        retVal = GetProxySetting(sHTTPS, host, 100, &port);

        if (retVal)
        {
            aHttpsProxy.Server = rtl::OUString::createFromAscii( host );
            aHttpsProxy.Port = port;
        }

        retVal = GetProxySetting(sFTP, host, 100, &port);

        if (retVal)
        {
            aFtpProxy.Server = rtl::OUString::createFromAscii( host );
            aFtpProxy.Port = port;
        }

        // http proxy name
        if( aHttpProxy.Server.getLength() > 0 )
        {
            aPropInfoList[nProperties].Name = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Inet/Settings/ooInetHTTPProxyName") );
            aPropInfoList[nProperties].Type = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "string" ) );
            aPropInfoList[nProperties].Protected = sal_False;
            aPropInfoList[nProperties++].Value = uno::makeAny( aHttpProxy.Server );
        }

        // http proxy port
        if( aHttpProxy.Port > 0 )
        {
            aPropInfoList[nProperties].Name = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Inet/Settings/ooInetHTTPProxyPort") );
            aPropInfoList[nProperties].Type = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "int" ) );
            aPropInfoList[nProperties].Protected = sal_False;
            aPropInfoList[nProperties++].Value = uno::makeAny( aHttpProxy.Port );
        }

        // https proxy name
        if( aHttpsProxy.Server.getLength() > 0 )
        {
            aPropInfoList[nProperties].Name = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Inet/Settings/ooInetHTTPSProxyName") );
            aPropInfoList[nProperties].Type = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "string" ) );
            aPropInfoList[nProperties].Protected = sal_False;
            aPropInfoList[nProperties++].Value = uno::makeAny( aHttpsProxy.Server );
        }

        // https proxy port
        if( aHttpsProxy.Port > 0 )
        {
            aPropInfoList[nProperties].Name = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Inet/Settings/ooInetHTTPSProxyPort") );
            aPropInfoList[nProperties].Type = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "int" ) );
            aPropInfoList[nProperties].Protected = sal_False;
            aPropInfoList[nProperties++].Value = uno::makeAny( aHttpsProxy.Port );
        }

        // ftp proxy name
        if( aFtpProxy.Server.getLength() > 0 )
        {
            aPropInfoList[nProperties].Name = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Inet/Settings/ooInetFTPProxyName") );
            aPropInfoList[nProperties].Type = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "string" ) );
            aPropInfoList[nProperties].Protected = sal_False;
            aPropInfoList[nProperties++].Value = uno::makeAny( aFtpProxy.Server );
        }

        // ftp proxy port
        if( aFtpProxy.Port > 0 )
        {
            aPropInfoList[nProperties].Name = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Inet/Settings/ooInetFTPProxyPort") );
            aPropInfoList[nProperties].Type = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "int" ) );
            aPropInfoList[nProperties].Protected = sal_False;
            aPropInfoList[nProperties++].Value = uno::makeAny( aFtpProxy.Port );
        }

        // resize the property info list appropriately
        aPropInfoList.realloc(nProperties);

        m_xLayerContentDescriber->describeLayer(xHandler, aPropInfoList);
    }
    else
    {
        OSL_TRACE("Could not create com.sun.star.configuration.backend.LayerContentDescriber Service");
    }
}

//------------------------------------------------------------------------------

rtl::OUString SAL_CALL MacOSXLayer::getTimestamp(void)
    throw (uno::RuntimeException)
{
    // FIXME: Always new timestamp!?
    rtl::OUString aTimestamp = rtl::OUString::valueOf( time(NULL) );

    return aTimestamp;
}

//------------------------------------------------------------------------------

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

void SAL_CALL MacOSXPathLayer::readData(
    const uno::Reference<backend::XLayerHandler>& i_xHandler)
    throw ( backend::MalformedDataException,
            lang::NullPointerException,
            lang::WrappedTargetException,
            uno::RuntimeException)
{
    if (m_xLayerContentDescriber.is())
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
                uno::Sequence<backend::PropertyInfo> aPropInfoList(1);

                aPropInfoList[0].Name = rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Office.Paths/Variables/Work" ) );
                aPropInfoList[0].Type = rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "string" ) );
                aPropInfoList[0].Protected = sal_False;
                aPropInfoList[0].Value <<= aDocURL;

                m_xLayerContentDescriber->describeLayer(i_xHandler, aPropInfoList);
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
    }
    else
    {
        OSL_TRACE("Could not create com.sun.star.configuration.backend.LayerContentDescriber Service");
    }
}

