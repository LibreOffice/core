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

#include <svdata.hxx>

#include <rtl/strbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <osl/process.h>
#include <osl/thread.h>
#include <unotools/configmgr.hxx>

SalGenericSystem::SalGenericSystem()
{
}

SalGenericSystem::~SalGenericSystem()
{
}

int SalGenericSystem::ShowNativeMessageBox( const OUString& rTitle, const OUString& rMessage )
{
    std::vector< OUString > aButtons;
    int nButtonIds[5] = {0}, nBut = 0;

    ImplHideSplash();

    aButtons.push_back( u"OK"_ustr );
    nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK;
    int nResult = ShowNativeDialog( rTitle, rMessage, aButtons );

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
        rtl::Bootstrap::get( u"BRAND_BASE_DIR"_ustr, aIni );
        aIni += "/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE( "bootstrap" );
        rtl::Bootstrap aBootstrap( aIni );
        aBootstrap.getFrom( u"ProductKey"_ustr, aProduct );

        if( !aProduct.isEmpty() )
            aClassName.append( OUStringToOString( aProduct, osl_getThreadTextEncoding() ) );
        else
            aClassName.append( OUStringToOString( utl::ConfigManager::getProductName(), osl_getThreadTextEncoding()));
    }
    return aClassName.getStr();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
