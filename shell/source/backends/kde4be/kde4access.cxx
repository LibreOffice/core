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

#include "precompiled_shell.hxx"
#include "sal/config.h"

#include "QFont"
#include "QString"
#include "kemailsettings.h"
#include "kglobalsettings.h"
#include "kprotocolmanager.h"

#include "com/sun/star/uno/Any.hxx"
#include "cppu/unotype.hxx"
#include "osl/diagnose.h"
#include "osl/file.h"
#include "rtl/string.h"
#include "rtl/ustring.hxx"

#include "kde4access.hxx"

#define SPACE      ' '
#define COMMA      ','
#define SEMI_COLON ';'

namespace kde4access {

namespace {

namespace css = com::sun::star ;
namespace uno = css::uno ;

}

css::beans::Optional< css::uno::Any > getValue(rtl::OUString const & id) {
    if (id.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("ExternalMailer"))) {
        KEMailSettings aEmailSettings;
        QString aClientProgram;
        ::rtl::OUString sClientProgram;

        aClientProgram = aEmailSettings.getSetting( KEMailSettings::ClientProgram );
        if ( aClientProgram.isEmpty() )
            aClientProgram = "kmail";
        else
            aClientProgram = aClientProgram.section(SPACE, 0, 0);
        sClientProgram = (const sal_Unicode *) aClientProgram.utf16();
        return css::beans::Optional< css::uno::Any >(
            true, uno::makeAny( sClientProgram ) );
    } else if (id.equalsAsciiL(
                   RTL_CONSTASCII_STRINGPARAM("SourceViewFontHeight")))
    {
        QFont aFixedFont;
        short nFontHeight;

        aFixedFont = KGlobalSettings::fixedFont();
        nFontHeight = aFixedFont.pointSize();
        return css::beans::Optional< css::uno::Any >(
            true, uno::makeAny( nFontHeight ) );
    } else if (id.equalsAsciiL(
                   RTL_CONSTASCII_STRINGPARAM("SourceViewFontName")))
    {
        QFont aFixedFont;
        QString aFontName;
        :: rtl::OUString sFontName;

        aFixedFont = KGlobalSettings::fixedFont();
        aFontName = aFixedFont.family();
        sFontName = (const sal_Unicode *) aFontName.utf16();
        return css::beans::Optional< css::uno::Any >(
            true, uno::makeAny( sFontName ) );
    } else if (id.equalsAsciiL(
                   RTL_CONSTASCII_STRINGPARAM("EnableATToolSupport")))
    {
        /* does not make much sense without an accessibility bridge */
        sal_Bool ATToolSupport = sal_False;
        return css::beans::Optional< css::uno::Any >(
            true, uno::makeAny( rtl::OUString::valueOf( ATToolSupport ) ) );
    } else if (id.equalsAsciiL(
                   RTL_CONSTASCII_STRINGPARAM("WorkPathVariable")))
    {
        QString aDocumentsDir( KGlobalSettings::documentPath() );
        rtl::OUString sDocumentsDir;
        rtl::OUString sDocumentsURL;
        if ( aDocumentsDir.endsWith(QChar('/')) )
            aDocumentsDir.truncate ( aDocumentsDir.length() - 1 );
        sDocumentsDir = (const sal_Unicode *) aDocumentsDir.utf16();
        osl_getFileURLFromSystemPath( sDocumentsDir.pData, &sDocumentsURL.pData );
        return css::beans::Optional< css::uno::Any >(
            true, uno::makeAny( sDocumentsURL ) );
    } else if (id.equalsAsciiL(
                   RTL_CONSTASCII_STRINGPARAM("ooInetFTPProxyName")))
    {
        QString aFTPProxy;
        switch ( KProtocolManager::proxyType() )
        {
        case KProtocolManager::ManualProxy: // Proxies are manually configured
            aFTPProxy = KProtocolManager::proxyFor( "FTP" );
            break;
        case KProtocolManager::PACProxy:    // A proxy configuration URL has been given
        case KProtocolManager::WPADProxy:   // A proxy should be automatically discovered
        case KProtocolManager::EnvVarProxy: // Use the proxy values set through environment variables
// In such cases, the proxy address is not stored in KDE, but determined dynamically.
// The proxy address may depend on the requested address, on the time of the day, on the speed of the wind...
// The best we can do here is to ask the current value for a given address.
            aFTPProxy = KProtocolManager::proxyForUrl( KUrl("ftp://ftp.libreoffice.org") );
            break;
        default:                            // No proxy is used
            break;
        }
        if ( !aFTPProxy.isEmpty() )
        {
            KUrl aProxy(aFTPProxy);
            ::rtl::OUString sProxy = (const sal_Unicode *) aProxy.host().utf16();
            return css::beans::Optional< css::uno::Any >(
                true, uno::makeAny( sProxy ) );
        }
    } else if (id.equalsAsciiL(
                   RTL_CONSTASCII_STRINGPARAM("ooInetFTPProxyPort")))
    {
        QString aFTPProxy;
        switch ( KProtocolManager::proxyType() )
        {
        case KProtocolManager::ManualProxy: // Proxies are manually configured
            aFTPProxy = KProtocolManager::proxyFor( "FTP" );
            break;
        case KProtocolManager::PACProxy:    // A proxy configuration URL has been given
        case KProtocolManager::WPADProxy:   // A proxy should be automatically discovered
        case KProtocolManager::EnvVarProxy: // Use the proxy values set through environment variables
// In such cases, the proxy address is not stored in KDE, but determined dynamically.
// The proxy address may depend on the requested address, on the time of the day, on the speed of the wind...
// The best we can do here is to ask the current value for a given address.
            aFTPProxy = KProtocolManager::proxyForUrl( KUrl("ftp://ftp.libreoffice.org") );
            break;
        default:                            // No proxy is used
            break;
        }
        if ( !aFTPProxy.isEmpty() )
        {
            KUrl aProxy(aFTPProxy);
            sal_Int32 nPort = aProxy.port();
            return css::beans::Optional< css::uno::Any >(
                true, uno::makeAny( nPort ) );
        }
    } else if (id.equalsAsciiL(
                   RTL_CONSTASCII_STRINGPARAM("ooInetHTTPProxyName")))
    {
        QString aHTTPProxy;
        switch ( KProtocolManager::proxyType() )
        {
        case KProtocolManager::ManualProxy: // Proxies are manually configured
            aHTTPProxy = KProtocolManager::proxyFor( "HTTP" );
            break;
        case KProtocolManager::PACProxy:    // A proxy configuration URL has been given
        case KProtocolManager::WPADProxy:   // A proxy should be automatically discovered
        case KProtocolManager::EnvVarProxy: // Use the proxy values set through environment variables
// In such cases, the proxy address is not stored in KDE, but determined dynamically.
// The proxy address may depend on the requested address, on the time of the day, on the speed of the wind...
// The best we can do here is to ask the current value for a given address.
            aHTTPProxy = KProtocolManager::proxyForUrl( KUrl("http://http.libreoffice.org") );
            break;
        default:                            // No proxy is used
            break;
        }
        if ( !aHTTPProxy.isEmpty() )
        {
            KUrl aProxy(aHTTPProxy);
            ::rtl::OUString sProxy = (const sal_Unicode *) aProxy.host().utf16();
            return css::beans::Optional< css::uno::Any >(
                true, uno::makeAny( sProxy ) );
        }
    } else if (id.equalsAsciiL(
                   RTL_CONSTASCII_STRINGPARAM("ooInetHTTPProxyPort")))
    {
        QString aHTTPProxy;
        switch ( KProtocolManager::proxyType() )
        {
        case KProtocolManager::ManualProxy: // Proxies are manually configured
            aHTTPProxy = KProtocolManager::proxyFor( "HTTP" );
            break;
        case KProtocolManager::PACProxy:    // A proxy configuration URL has been given
        case KProtocolManager::WPADProxy:   // A proxy should be automatically discovered
        case KProtocolManager::EnvVarProxy: // Use the proxy values set through environment variables
// In such cases, the proxy address is not stored in KDE, but determined dynamically.
// The proxy address may depend on the requested address, on the time of the day, on the speed of the wind...
// The best we can do here is to ask the current value for a given address.
            aHTTPProxy = KProtocolManager::proxyForUrl( KUrl("http://http.libreoffice.org") );
            break;
        default:                            // No proxy is used
            break;
        }
        if ( !aHTTPProxy.isEmpty() )
        {
            KUrl aProxy(aHTTPProxy);
            sal_Int32 nPort = aProxy.port();
            return css::beans::Optional< css::uno::Any >(
                true, uno::makeAny( nPort ) );
        }
    } else if (id.equalsAsciiL(
                   RTL_CONSTASCII_STRINGPARAM("ooInetHTTPSProxyName")))
    {
        QString aHTTPSProxy;
        switch ( KProtocolManager::proxyType() )
        {
        case KProtocolManager::ManualProxy: // Proxies are manually configured
            aHTTPSProxy = KProtocolManager::proxyFor( "HTTPS" );
            break;
        case KProtocolManager::PACProxy:    // A proxy configuration URL has been given
        case KProtocolManager::WPADProxy:   // A proxy should be automatically discovered
        case KProtocolManager::EnvVarProxy: // Use the proxy values set through environment variables
// In such cases, the proxy address is not stored in KDE, but determined dynamically.
// The proxy address may depend on the requested address, on the time of the day, on the speed of the wind...
// The best we can do here is to ask the current value for a given address.
            aHTTPSProxy = KProtocolManager::proxyForUrl( KUrl("https://https.libreoffice.org") );
            break;
        default:                            // No proxy is used
            break;
        }
        if ( !aHTTPSProxy.isEmpty() )
        {
            KUrl aProxy(aHTTPSProxy);
            ::rtl::OUString sProxy = (const sal_Unicode *) aProxy.host().utf16();
            return css::beans::Optional< css::uno::Any >(
                true, uno::makeAny( sProxy ) );
        }
    } else if (id.equalsAsciiL(
                   RTL_CONSTASCII_STRINGPARAM("ooInetHTTPSProxyPort")))
    {
        QString aHTTPSProxy;
        switch ( KProtocolManager::proxyType() )
        {
        case KProtocolManager::ManualProxy: // Proxies are manually configured
            aHTTPSProxy = KProtocolManager::proxyFor( "HTTPS" );
            break;
        case KProtocolManager::PACProxy:    // A proxy configuration URL has been given
        case KProtocolManager::WPADProxy:   // A proxy should be automatically discovered
        case KProtocolManager::EnvVarProxy: // Use the proxy values set through environment variables
// In such cases, the proxy address is not stored in KDE, but determined dynamically.
// The proxy address may depend on the requested address, on the time of the day, on the speed of the wind...
// The best we can do here is to ask the current value for a given address.
            aHTTPSProxy = KProtocolManager::proxyForUrl( KUrl("https://https.libreoffice.org") );
            break;
        default:                            // No proxy is used
            break;
        }
        if ( !aHTTPSProxy.isEmpty() )
        {
            KUrl aProxy(aHTTPSProxy);
            sal_Int32 nPort = aProxy.port();
            return css::beans::Optional< css::uno::Any >(
                true, uno::makeAny( nPort ) );
        }
    } else if (id.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("ooInetNoProxy"))) {
        QString aNoProxyFor;
        switch ( KProtocolManager::proxyType() )
        {
        case KProtocolManager::ManualProxy: // Proxies are manually configured
        case KProtocolManager::PACProxy:    // A proxy configuration URL has been given
        case KProtocolManager::WPADProxy:   // A proxy should be automatically discovered
        case KProtocolManager::EnvVarProxy: // Use the proxy values set through environment variables
            aNoProxyFor = KProtocolManager::noProxyFor();
            break;
        default:                            // No proxy is used
            break;
        }
        if ( !aNoProxyFor.isEmpty() )
        {
            ::rtl::OUString sNoProxyFor;

            aNoProxyFor = aNoProxyFor.replace( COMMA, SEMI_COLON );
            sNoProxyFor = (const sal_Unicode *) aNoProxyFor.utf16();
            return css::beans::Optional< css::uno::Any >(
                true, uno::makeAny( sNoProxyFor ) );
        }
    } else if (id.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("ooInetProxyType"))) {
        int nProxyType;
        switch ( KProtocolManager::proxyType() )
        {
        case KProtocolManager::ManualProxy: // Proxies are manually configured
        case KProtocolManager::PACProxy:    // A proxy configuration URL has been given
        case KProtocolManager::WPADProxy:   // A proxy should be automatically discovered
        case KProtocolManager::EnvVarProxy: // Use the proxy values set through environment variables
            nProxyType = 1;
            break;
        default:                            // No proxy is used
            nProxyType = 0;
        }
        return css::beans::Optional< css::uno::Any >(
            true, uno::makeAny( (sal_Int32) nProxyType ) );
    } else {
        OSL_ASSERT(false); // this cannot happen
    }
    return css::beans::Optional< css::uno::Any >();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
