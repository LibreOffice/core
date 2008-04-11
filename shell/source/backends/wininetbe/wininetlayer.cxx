/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: wininetlayer.cxx,v $
 * $Revision: 1.7 $
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

#ifndef _WININETLAYER_HXX_
#include "wininetlayer.hxx"
#endif

#include <malloc.h>

#include <rtl/ustrbuf.hxx>

#define EQUAL_SIGN '='
#define COLON      ':'
#define SPACE      ' '
#define SEMI_COLON ';'

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

WinInetLayer::WinInetLayer( InternetQueryOption_Proc_T lpfnInternetQueryOption,
    const uno::Reference<uno::XComponentContext>& xContext)
  : m_lpfnInternetQueryOption(lpfnInternetQueryOption)
{
    //Create instance of LayerContentDescriber Service
    rtl::OUString const k_sLayerDescriberService(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.comp.configuration.backend.LayerDescriber"));

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

void SAL_CALL WinInetLayer::readData(
    const uno::Reference<backend::XLayerHandler>& xHandler)
    throw ( backend::MalformedDataException,
            lang::NullPointerException,
            lang::WrappedTargetException,
            uno::RuntimeException)
{

    if (m_xLayerContentDescriber.is() && m_lpfnInternetQueryOption)
    {
        LPINTERNET_PROXY_INFO lpi = NULL;

        // query for the neccessary space
        DWORD dwLength = 0;
        BOOL bRet = m_lpfnInternetQueryOption(
            NULL,
            INTERNET_OPTION_PROXY,
            (LPVOID)lpi,
            &dwLength );

        // allocate sufficient space on the heap
        // insufficient space on the heap results
        // in a stack overflow exception, we assume
        // this never happens, because of the relatively
        // small amount of memory we need
        // _alloca is nice because it is fast and we don't
        // have to free the allocated memory, it will be
        // automatically done
        lpi = reinterpret_cast< LPINTERNET_PROXY_INFO >(
            _alloca( dwLength ) );

        bRet = m_lpfnInternetQueryOption(
            NULL,
            INTERNET_OPTION_PROXY,
            (LPVOID)lpi,
            &dwLength );

        // if a proxy is disabled, InternetQueryOption returns
        // an empty proxy list, so we don't have to check if
        // proxy is enabled or not

        rtl::OUString aProxyList       = rtl::OUString::createFromAscii( lpi->lpszProxy );
        rtl::OUString aProxyBypassList = rtl::OUString::createFromAscii( lpi->lpszProxyBypass );

        // override default for ProxyType, which is "0" meaning "No proxies".
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

            aPropInfoList[nProperties].Name = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Inet/Settings/ooInetNoProxy") );
            aPropInfoList[nProperties].Type = rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "string" ) );
            aPropInfoList[nProperties].Protected = sal_False;
            aPropInfoList[nProperties++].Value = uno::makeAny( aProxyBypassList.replace( SPACE, SEMI_COLON ) );
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
                RTL_CONSTASCII_USTRINGPARAM( "http" ) ) );
            ProxyEntry aHttpsProxy  = FindProxyEntry( aProxyList, rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "https" ) ) );

            ProxyEntry aFtpProxy  = FindProxyEntry( aProxyList, rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM( "ftp" ) ) );

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
                aPropInfoList[nProperties].Name = rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Inet/Settings/ooInetHTTPProxyName") );
                aPropInfoList[nProperties].Type = rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "string" ) );
                aPropInfoList[nProperties].Protected = sal_False;
                aPropInfoList[nProperties++].Value = uno::makeAny( aHttpProxy.Server );
            }

            // http proxy port
            if( aHttpProxy.Port.getLength() > 0 )
            {
                aPropInfoList[nProperties].Name = rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Inet/Settings/ooInetHTTPProxyPort") );
                aPropInfoList[nProperties].Type = rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "int" ) );
                aPropInfoList[nProperties].Protected = sal_False;
                aPropInfoList[nProperties++].Value = uno::makeAny( aHttpProxy.Port.toInt32() );
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
            if( aHttpsProxy.Port.getLength() > 0 )
            {
                aPropInfoList[nProperties].Name = rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Inet/Settings/ooInetHTTPSProxyPort") );
                aPropInfoList[nProperties].Type = rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "int" ) );
                aPropInfoList[nProperties].Protected = sal_False;
                aPropInfoList[nProperties++].Value = uno::makeAny( aHttpsProxy.Port.toInt32() );
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
            if( aFtpProxy.Port.getLength() > 0 )
            {
                aPropInfoList[nProperties].Name = rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "org.openoffice.Inet/Settings/ooInetFTPProxyPort") );
                aPropInfoList[nProperties].Type = rtl::OUString(
                    RTL_CONSTASCII_USTRINGPARAM( "int" ) );
                aPropInfoList[nProperties].Protected = sal_False;
                aPropInfoList[nProperties++].Value = uno::makeAny( aFtpProxy.Port.toInt32() );
            }
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

rtl::OUString SAL_CALL WinInetLayer::getTimestamp(void)
    throw (uno::RuntimeException)
{
    rtl::OUString aTimestamp;

    if (m_lpfnInternetQueryOption)
    {
        LPINTERNET_PROXY_INFO lpi = NULL;

        // query for the neccessary space
        DWORD dwLength = 0;
        BOOL bRet = m_lpfnInternetQueryOption(
            NULL,
            INTERNET_OPTION_PROXY,
            (LPVOID)lpi,
            &dwLength );

        // allocate sufficient space on the heap
        // insufficient space on the heap results
        // in a stack overflow exception, we assume
        // this never happens, because of the relatively
        // small amount of memory we need
        // _alloca is nice because it is fast and we don't
        // have to free the allocated memory, it will be
        // automatically done
        lpi = reinterpret_cast< LPINTERNET_PROXY_INFO >(
            _alloca( dwLength ) );

        bRet = m_lpfnInternetQueryOption(
            NULL,
            INTERNET_OPTION_PROXY,
            (LPVOID)lpi,
            &dwLength );

        aTimestamp = rtl::OUString::createFromAscii( lpi->lpszProxy );
        aTimestamp += rtl::OUString::createFromAscii( lpi->lpszProxyBypass );
    }

    return aTimestamp;
}

//------------------------------------------------------------------------------
