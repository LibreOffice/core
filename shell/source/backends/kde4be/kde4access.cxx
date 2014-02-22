/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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
        sClientProgram = (const sal_Unicode *) aClientProgram.utf16();
        return css::beans::Optional< css::uno::Any >(
            true, uno::makeAny( sClientProgram ) );
    } else if (id == "SourceViewFontHeight")
    {
        QFont aFixedFont;
        short nFontHeight;

        aFixedFont = KGlobalSettings::fixedFont();
        nFontHeight = aFixedFont.pointSize();
        return css::beans::Optional< css::uno::Any >(
            true, uno::makeAny( nFontHeight ) );
    } else if (id == "SourceViewFontName")
    {
        QFont aFixedFont;
        QString aFontName;
        :: OUString sFontName;

        aFixedFont = KGlobalSettings::fixedFont();
        aFontName = aFixedFont.family();
        sFontName = (const sal_Unicode *) aFontName.utf16();
        return css::beans::Optional< css::uno::Any >(
            true, uno::makeAny( sFontName ) );
    } else if (id == "EnableATToolSupport")
    {
        /* does not make much sense without an accessibility bridge */
        sal_Bool ATToolSupport = sal_False;
        return css::beans::Optional< css::uno::Any >(
            true, uno::makeAny( OUString::boolean( ATToolSupport ) ) );
    } else if (id == "WorkPathVariable")
    {
        QString aDocumentsDir( KGlobalSettings::documentPath() );
        OUString sDocumentsDir;
        OUString sDocumentsURL;
        if ( aDocumentsDir.endsWith(QChar('/')) )
            aDocumentsDir.truncate ( aDocumentsDir.length() - 1 );
        sDocumentsDir = (const sal_Unicode *) aDocumentsDir.utf16();
        osl_getFileURLFromSystemPath( sDocumentsDir.pData, &sDocumentsURL.pData );
        return css::beans::Optional< css::uno::Any >(
            true, uno::makeAny( sDocumentsURL ) );
    } else if (id == "ooInetFTPProxyName")
    {
        QString aFTPProxy;
        switch ( KProtocolManager::proxyType() )
        {
        case KProtocolManager::ManualProxy: 
            aFTPProxy = KProtocolManager::proxyFor( "FTP" );
            break;
        case KProtocolManager::PACProxy:    
        case KProtocolManager::WPADProxy:   
        case KProtocolManager::EnvVarProxy: 



            aFTPProxy = KProtocolManager::proxyForUrl( KUrl("ftp:
            break;
        default:                            
            break;
        }
        if ( !aFTPProxy.isEmpty() )
        {
            KUrl aProxy(aFTPProxy);
            OUString sProxy = (const sal_Unicode *) aProxy.host().utf16();
            return css::beans::Optional< css::uno::Any >(
                true, uno::makeAny( sProxy ) );
        }
    } else if (id == "ooInetFTPProxyPort")
    {
        QString aFTPProxy;
        switch ( KProtocolManager::proxyType() )
        {
        case KProtocolManager::ManualProxy: 
            aFTPProxy = KProtocolManager::proxyFor( "FTP" );
            break;
        case KProtocolManager::PACProxy:    
        case KProtocolManager::WPADProxy:   
        case KProtocolManager::EnvVarProxy: 



            aFTPProxy = KProtocolManager::proxyForUrl( KUrl("ftp:
            break;
        default:                            
            break;
        }
        if ( !aFTPProxy.isEmpty() )
        {
            KUrl aProxy(aFTPProxy);
            sal_Int32 nPort = aProxy.port();
            return css::beans::Optional< css::uno::Any >(
                true, uno::makeAny( nPort ) );
        }
    } else if (id == "ooInetHTTPProxyName")
    {
        QString aHTTPProxy;
        switch ( KProtocolManager::proxyType() )
        {
        case KProtocolManager::ManualProxy: 
            aHTTPProxy = KProtocolManager::proxyFor( "HTTP" );
            break;
        case KProtocolManager::PACProxy:    
        case KProtocolManager::WPADProxy:   
        case KProtocolManager::EnvVarProxy: 



            aHTTPProxy = KProtocolManager::proxyForUrl( KUrl("http:
            break;
        default:                            
            break;
        }
        if ( !aHTTPProxy.isEmpty() )
        {
            KUrl aProxy(aHTTPProxy);
            OUString sProxy = (const sal_Unicode *) aProxy.host().utf16();
            return css::beans::Optional< css::uno::Any >(
                true, uno::makeAny( sProxy ) );
        }
    } else if (id == "ooInetHTTPProxyPort")
    {
        QString aHTTPProxy;
        switch ( KProtocolManager::proxyType() )
        {
        case KProtocolManager::ManualProxy: 
            aHTTPProxy = KProtocolManager::proxyFor( "HTTP" );
            break;
        case KProtocolManager::PACProxy:    
        case KProtocolManager::WPADProxy:   
        case KProtocolManager::EnvVarProxy: 



            aHTTPProxy = KProtocolManager::proxyForUrl( KUrl("http:
            break;
        default:                            
            break;
        }
        if ( !aHTTPProxy.isEmpty() )
        {
            KUrl aProxy(aHTTPProxy);
            sal_Int32 nPort = aProxy.port();
            return css::beans::Optional< css::uno::Any >(
                true, uno::makeAny( nPort ) );
        }
    } else if (id == "ooInetHTTPSProxyName")
    {
        QString aHTTPSProxy;
        switch ( KProtocolManager::proxyType() )
        {
        case KProtocolManager::ManualProxy: 
            aHTTPSProxy = KProtocolManager::proxyFor( "HTTPS" );
            break;
        case KProtocolManager::PACProxy:    
        case KProtocolManager::WPADProxy:   
        case KProtocolManager::EnvVarProxy: 



            aHTTPSProxy = KProtocolManager::proxyForUrl( KUrl("https:
            break;
        default:                            
            break;
        }
        if ( !aHTTPSProxy.isEmpty() )
        {
            KUrl aProxy(aHTTPSProxy);
            OUString sProxy = (const sal_Unicode *) aProxy.host().utf16();
            return css::beans::Optional< css::uno::Any >(
                true, uno::makeAny( sProxy ) );
        }
    } else if (id == "ooInetHTTPSProxyPort")
    {
        QString aHTTPSProxy;
        switch ( KProtocolManager::proxyType() )
        {
        case KProtocolManager::ManualProxy: 
            aHTTPSProxy = KProtocolManager::proxyFor( "HTTPS" );
            break;
        case KProtocolManager::PACProxy:    
        case KProtocolManager::WPADProxy:   
        case KProtocolManager::EnvVarProxy: 



            aHTTPSProxy = KProtocolManager::proxyForUrl( KUrl("https:
            break;
        default:                            
            break;
        }
        if ( !aHTTPSProxy.isEmpty() )
        {
            KUrl aProxy(aHTTPSProxy);
            sal_Int32 nPort = aProxy.port();
            return css::beans::Optional< css::uno::Any >(
                true, uno::makeAny( nPort ) );
        }
    } else if ( id == "ooInetNoProxy" ) {
        QString aNoProxyFor;
        switch ( KProtocolManager::proxyType() )
        {
        case KProtocolManager::ManualProxy: 
        case KProtocolManager::PACProxy:    
        case KProtocolManager::WPADProxy:   
        case KProtocolManager::EnvVarProxy: 
            aNoProxyFor = KProtocolManager::noProxyFor();
            break;
        default:                            
            break;
        }
        if ( !aNoProxyFor.isEmpty() )
        {
            OUString sNoProxyFor;

            aNoProxyFor = aNoProxyFor.replace( COMMA, SEMI_COLON );
            sNoProxyFor = (const sal_Unicode *) aNoProxyFor.utf16();
            return css::beans::Optional< css::uno::Any >(
                true, uno::makeAny( sNoProxyFor ) );
        }
    } else if ( id == "ooInetProxyType" ) {
        int nProxyType;
        switch ( KProtocolManager::proxyType() )
        {
        case KProtocolManager::ManualProxy: 
        case KProtocolManager::PACProxy:    
        case KProtocolManager::WPADProxy:   
        case KProtocolManager::EnvVarProxy: 
            nProxyType = 1;
            break;
        default:                            
            nProxyType = 0;
        }
        return css::beans::Optional< css::uno::Any >(
            true, uno::makeAny( (sal_Int32) nProxyType ) );
    } else {
        OSL_ASSERT(false); 
    }
    return css::beans::Optional< css::uno::Any >();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
