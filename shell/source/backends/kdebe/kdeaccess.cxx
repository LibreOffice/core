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

#include "com/sun/star/uno/Any.hxx"
#include "cppu/unotype.hxx"
#include "osl/diagnose.h"
#include "osl/file.h"
#include "rtl/string.h"
#include "rtl/ustring.hxx"

#include "shell/kde_headers.h"

#include "kdeaccess.hxx"

#define SPACE      ' '
#define COMMA      ','
#define SEMI_COLON ';'

namespace kdeaccess {

namespace {

namespace uno = css::uno ;

}

css::beans::Optional< css::uno::Any > getValue(OUString const & id) {
    if ( id == "ExternalMailer" ) {
        KEMailSettings aEmailSettings;
        QString aClientProgram;
        OUString sClientProgram;

        aClientProgram = aEmailSettings.getSetting( KEMailSettings::ClientProgram );
        if ( aClientProgram.isEmpty() )
            aClientProgram = "kmail";
        else
            aClientProgram = aClientProgram.section(SPACE, 0, 0);
        sClientProgram = (const sal_Unicode *) aClientProgram.ucs2();
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
        :: OUString sFontName;

        aFixedFont = KGlobalSettings::fixedFont();
        aFontName = aFixedFont.family();
        sFontName = (const sal_Unicode *) aFontName.ucs2();
        return css::beans::Optional< css::uno::Any >(
            true, uno::makeAny( sFontName ) );
    } else if (id.equalsAsciiL(
                   RTL_CONSTASCII_STRINGPARAM("EnableATToolSupport")))
    {
        /* does not make much sense without an accessibility bridge */
        sal_Bool ATToolSupport = sal_False;
        return css::beans::Optional< css::uno::Any >(
            true, uno::makeAny( OUString::boolean( ATToolSupport ) ) );
    } else if (id.equalsAsciiL(
                   RTL_CONSTASCII_STRINGPARAM("WorkPathVariable")))
    {
        QString aDocumentsDir( KGlobalSettings::documentPath() );
        OUString sDocumentsDir;
        OUString sDocumentsURL;
        if ( aDocumentsDir.endsWith(QChar('/')) )
            aDocumentsDir.truncate ( aDocumentsDir.length() - 1 );
        sDocumentsDir = (const sal_Unicode *) aDocumentsDir.ucs2();
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
            aFTPProxy = KProtocolManager::proxyForURL( "ftp://ftp.openoffice.org" );
            break;
        default:                            // No proxy is used
            break;
        }
        if ( !aFTPProxy.isEmpty() )
        {
            KURL aProxy(aFTPProxy);
            OUString sProxy = (const sal_Unicode *) aProxy.host().ucs2();
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
            aFTPProxy = KProtocolManager::proxyForURL( "ftp://ftp.openoffice.org" );
            break;
        default:                            // No proxy is used
            break;
        }
        if ( !aFTPProxy.isEmpty() )
        {
            KURL aProxy(aFTPProxy);
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
            aHTTPProxy = KProtocolManager::proxyForURL( "http://http.openoffice.org" );
            break;
        default:                            // No proxy is used
            break;
        }
        if ( !aHTTPProxy.isEmpty() )
        {
            KURL aProxy(aHTTPProxy);
            OUString sProxy = (const sal_Unicode *) aProxy.host().ucs2();
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
            aHTTPProxy = KProtocolManager::proxyForURL( "http://http.openoffice.org" );
            break;
        default:                            // No proxy is used
            break;
        }
        if ( !aHTTPProxy.isEmpty() )
        {
            KURL aProxy(aHTTPProxy);
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
            aHTTPSProxy = KProtocolManager::proxyForURL( "https://https.openoffice.org" );
            break;
        default:                            // No proxy is used
            break;
        }
        if ( !aHTTPSProxy.isEmpty() )
        {
            KURL aProxy(aHTTPSProxy);
            OUString sProxy = (const sal_Unicode *) aProxy.host().ucs2();
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
            aHTTPSProxy = KProtocolManager::proxyForURL( "https://https.openoffice.org" );
            break;
        default:                            // No proxy is used
            break;
        }
        if ( !aHTTPSProxy.isEmpty() )
        {
            KURL aProxy(aHTTPSProxy);
            sal_Int32 nPort = aProxy.port();
            return css::beans::Optional< css::uno::Any >(
                true, uno::makeAny( nPort ) );
        }
    } else if ( id == "ooInetNoProxy" ) {
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
            OUString sNoProxyFor;

            aNoProxyFor = aNoProxyFor.replace( COMMA, SEMI_COLON );
            sNoProxyFor = (const sal_Unicode *) aNoProxyFor.ucs2();
            return css::beans::Optional< css::uno::Any >(
                true, uno::makeAny( sNoProxyFor ) );
        }
    } else if ( id == "ooInetProxyType" ) {
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
