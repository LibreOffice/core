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


#include <generic/gensys.h>

#include <vcl/msgbox.hxx>
#include <vcl/button.hxx>

#include <svdata.hxx>

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <osl/process.h>
#include <osl/thread.h>

#include "vcl/unohelp.hxx"
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

using namespace com::sun::star;

namespace {

OUString GetNativeMessageBoxButtonText( int nButtonId, bool bUseResources )
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
        case BUTTON_OK:
            aText = "OK";
            break;
        case BUTTON_CANCEL:
            aText = "Cancel";
            break;
        case BUTTON_ABORT:
            aText = "Abort";
            break;
        case BUTTON_RETRY:
            aText = "Retry";
            break;
        case BUTTON_IGNORE:
            aText = "Ignore";
            break;
        case BUTTON_YES:
            aText = "Yes";
            break;
        case BUTTON_NO:
            aText = "No";
            break;
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

int SalGenericSystem::ShowNativeMessageBox( const rtl::OUString& rTitle, const rtl::OUString& rMessage,
                                            int nButtonCombination, int nDefaultButton,
                                            bool bUseResources )
{
    int nDefButton = 0;
    std::list< rtl::OUString > aButtons;
    int nButtonIds[5], nBut = 0;

    ImplHideSplash();

    if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK ||
        nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL )
    {
        aButtons.push_back( GetNativeMessageBoxButtonText( BUTTON_OK, bUseResources ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK;
    }
    if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO_CANCEL ||
        nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO )
    {
        aButtons.push_back( GetNativeMessageBoxButtonText( BUTTON_YES, bUseResources ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_YES;
        aButtons.push_back( GetNativeMessageBoxButtonText( BUTTON_NO, bUseResources ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO;
        if( nDefaultButton == SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO )
            nDefButton = 1;
    }
    if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL ||
        nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO_CANCEL ||
        nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_RETRY_CANCEL )
    {
        if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_RETRY_CANCEL )
        {
            aButtons.push_back( GetNativeMessageBoxButtonText( BUTTON_RETRY, bUseResources ) );
            nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY;
        }
        aButtons.push_back( GetNativeMessageBoxButtonText( BUTTON_CANCEL, bUseResources ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL;
        if( nDefaultButton == SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL )
            nDefButton = aButtons.size()-1;
    }
    if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_ABORT_RETRY_IGNORE )
    {
        aButtons.push_back( GetNativeMessageBoxButtonText( BUTTON_ABORT, bUseResources ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_ABORT;
        aButtons.push_back( GetNativeMessageBoxButtonText( BUTTON_RETRY, bUseResources ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY;
        aButtons.push_back( GetNativeMessageBoxButtonText( BUTTON_IGNORE, bUseResources ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_IGNORE;
        switch( nDefaultButton )
        {
            case SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY: nDefButton = 1;break;
            case SALSYSTEM_SHOWNATIVEMSGBOX_BTN_IGNORE: nDefButton = 2;break;
        }
    }

    int nResult = ShowNativeDialog( rTitle, rMessage, aButtons, nDefButton );

    return nResult != -1 ? nButtonIds[ nResult ] : 0;
}

// ------------------------------------------------------------------------
//           Helpers primarily for X Windowing derivatives
// ------------------------------------------------------------------------

const char* SalGenericSystem::getFrameResName()
{
    /*  according to ICCCM:
     *  first search command line for -name parameter
     *  then try RESOURCE_NAME environment variable
     *  then use argv[0] stripped by directories
     */
    static rtl::OStringBuffer aResName;
    if( !aResName.getLength() )
    {
        int nArgs = osl_getCommandArgCount();
        for( int n = 0; n < nArgs-1; n++ )
        {
            rtl::OUString aArg;
            if( ! osl_getCommandArg( n, &aArg.pData ) &&
                aArg.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("-name")) &&
                ! osl_getCommandArg( n+1, &aArg.pData ) )
            {
                aResName.append( rtl::OUStringToOString( aArg, osl_getThreadTextEncoding() ) );
                break;
            }
        }
        if( !aResName.getLength() )
        {
            const char* pEnv = getenv( "RESOURCE_NAME" );
            if( pEnv && *pEnv )
                aResName.append( pEnv );
        }
        if( !aResName.getLength() )
            aResName.append( "VCLSalFrame" );
    }
    return aResName.getStr();
}

const char* SalGenericSystem::getFrameClassName()
{
    static rtl::OStringBuffer aClassName;
    if( !aClassName.getLength() )
    {
        rtl::OUString aIni, aProduct;
        rtl::Bootstrap::get( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "BRAND_BASE_DIR" ) ), aIni );
        aIni += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/program/" SAL_CONFIGFILE( "bootstrap" ) ) );
        rtl::Bootstrap aBootstrap( aIni );
        aBootstrap.getFrom( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ProductKey" ) ), aProduct );

        if( !aProduct.isEmpty() )
            aClassName.append( rtl::OUStringToOString( aProduct, osl_getThreadTextEncoding() ) );
        else
            aClassName.append( "VCLSalFrame" );
    }
    return aClassName.getStr();
}

rtl::OString SalGenericSystem::getFrameResName( SalExtStyle nStyle )
{
    rtl::OStringBuffer aBuf( 64 );
    aBuf.append( getFrameResName() );
    if( (nStyle & SAL_FRAME_EXT_STYLE_DOCUMENT) )
        aBuf.append( ".DocumentWindow" );

    return aBuf.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
