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

#include <config_folders.h>

#include <unx/gensys.h>

#include <vcl/msgbox.hxx>
#include <vcl/button.hxx>

#include <svdata.hxx>

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <osl/process.h>
#include <osl/thread.h>
#include <unotools/configmgr.hxx>

#include <vcl/unohelp.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

using namespace com::sun::star;

namespace {

OUString GetNativeMessageBoxButtonText( StandardButtonType nButtonId, bool bUseResources )
{
    OUString aText;
    if( bUseResources )
    {
        aText = Button::GetStandardText( nButtonId );
    }
    if( aText.isEmpty() )
    {
        switch( nButtonId )
        {
        case StandardButtonType::OK:
            aText = "OK";
            break;
        case StandardButtonType::Cancel:
            aText = "Cancel";
            break;
        case StandardButtonType::Abort:
            aText = "Abort";
            break;
        case StandardButtonType::Retry:
            aText = "Retry";
            break;
        case StandardButtonType::Ignore:
            aText = "Ignore";
            break;
        case StandardButtonType::Yes:
            aText = "Yes";
            break;
        case StandardButtonType::No:
            aText = "No";
            break;
        default: break;
        }
    }
    return aText;
}

}

SalGenericSystem::SalGenericSystem()
{
}

SalGenericSystem::~SalGenericSystem()
{
}

int SalGenericSystem::ShowNativeMessageBox( const OUString& rTitle, const OUString& rMessage )
{
    int nDefButton = 0;
    std::list< OUString > aButtons;
    int nButtonIds[5] = {0}, nBut = 0;

    ImplHideSplash();

    aButtons.push_back( GetNativeMessageBoxButtonText( StandardButtonType::OK, false/*bUseResources*/ ) );
    nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK;
    int nResult = ShowNativeDialog( rTitle, rMessage, aButtons, nDefButton );

    return nResult != -1 ? nButtonIds[ nResult ] : 0;
}

#if !defined(ANDROID) && !defined(IOS)

// X11-specific

const char* SalGenericSystem::getFrameResName()
{
    /*  according to ICCCM:
     *  first search command line for -name parameter
     *  then try RESOURCE_NAME environment variable
     *  then use argv[0] stripped by directories
     */
    static OStringBuffer aResName;
    if( aResName.isEmpty() )
    {
        int nArgs = osl_getCommandArgCount();
        for( int n = 0; n < nArgs-1; n++ )
        {
            OUString aArg;
            osl_getCommandArg( n, &aArg.pData );
            if( aArg.equalsIgnoreAsciiCase("-name") )
            {
                osl_getCommandArg( n+1, &aArg.pData );
                aResName.append( OUStringToOString( aArg, osl_getThreadTextEncoding() ) );
                break;
            }
        }
        if( aResName.isEmpty() )
        {
            const char* pEnv = getenv( "RESOURCE_NAME" );
            if( pEnv && *pEnv )
                aResName.append( pEnv );
        }
        if( aResName.isEmpty() )
            aResName.append( OUStringToOString( utl::ConfigManager::getProductName().toAsciiLowerCase(),
                osl_getThreadTextEncoding()));
    }
    return aResName.getStr();
}

const char* SalGenericSystem::getFrameClassName()
{
    static OStringBuffer aClassName;
    if( aClassName.isEmpty() )
    {
        OUString aIni, aProduct;
        rtl::Bootstrap::get( "BRAND_BASE_DIR", aIni );
        aIni += "/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE( "bootstrap" );
        rtl::Bootstrap aBootstrap( aIni );
        aBootstrap.getFrom( "ProductKey", aProduct );

        if( !aProduct.isEmpty() )
            aClassName.append( OUStringToOString( aProduct, osl_getThreadTextEncoding() ) );
        else
            aClassName.append( OUStringToOString( utl::ConfigManager::getProductName(), osl_getThreadTextEncoding()));
    }
    return aClassName.getStr();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
