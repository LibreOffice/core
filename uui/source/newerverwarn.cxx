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

#include <config_features.h>
#include <config_folders.h>

#include "newerverwarn.hxx"
#include "newerverwarn.hrc"
#include "ids.hrc"

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>

#include <comphelper/processfactory.hxx>
#include <rtl/bootstrap.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/msgbox.hxx>
#include <osl/process.h>
#include <officecfg/Office/Addons.hxx>

using namespace com::sun::star;

namespace uui
{

NewerVersionWarningDialog::NewerVersionWarningDialog(
    Window* pParent, const OUString& rVersion, ResMgr& rResMgr ) :

    ModalDialog( pParent, ResId( RID_DLG_NEWER_VERSION_WARNING, rResMgr ) ),

    m_aImage        ( this, ResId( FI_IMAGE, rResMgr ) ),
    m_aInfoText     ( this, ResId( FT_INFO, rResMgr ) ),
    m_aButtonLine   ( this, ResId( FL_BUTTON, rResMgr ) ),
    m_aUpdateBtn    ( this, ResId( PB_UPDATE, rResMgr ) ),
    m_aLaterBtn     ( this, ResId( PB_LATER, rResMgr ) ),
    m_sVersion      ( rVersion )
{
    FreeResource();

    m_aUpdateBtn.SetClickHdl( LINK( this, NewerVersionWarningDialog, UpdateHdl ) );
    m_aLaterBtn.SetClickHdl( LINK( this, NewerVersionWarningDialog, LaterHdl ) );

    InitButtonWidth();
}

NewerVersionWarningDialog::~NewerVersionWarningDialog()
{
}

IMPL_LINK_NOARG(NewerVersionWarningDialog, UpdateHdl)
{
    
    OUString sProgramPath;
    osl_getExecutableFile( &sProgramPath.pData );
    sal_uInt32 nLastIndex = sProgramPath.lastIndexOf( '/' );
    if ( nLastIndex > 0 )
        sProgramPath = sProgramPath.copy( 0, nLastIndex + 1 );

    
    OUString sIniFileName = sProgramPath;
#if HAVE_FEATURE_MACOSX_MACLIKE_APP_STRUCTURE
    sIniFileName += "../" LIBO_ETC_FOLDER "/";
#endif
    sIniFileName += SAL_CONFIGFILE( "version" );
    ::rtl::Bootstrap aIniFile( sIniFileName );
    OUString sNotifyURL;
    aIniFile.getFrom( OUString(  "ODFNotifyURL" ), sNotifyURL );

    try
    {
        uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
        if (  !sNotifyURL.isEmpty()  &&  !m_sVersion.isEmpty() )
        {
            uno::Reference< com::sun::star::system::XSystemShellExecute > xSystemShell( com::sun::star::system::SystemShellExecute::create(xContext) );
            sNotifyURL += m_sVersion;
            if ( !sNotifyURL.isEmpty() )
            {
                xSystemShell->execute(
                    sNotifyURL, OUString(), com::sun::star::system::SystemShellExecuteFlags::URIS_ONLY );
            }
        }
        else
        {
            
            
            
            
            
            css::uno::Reference< css::container::XHierarchicalNameAccess > xOfficeHelp(officecfg::Office::Addons::AddonUI::OfficeHelp::get(xContext), css::uno::UNO_QUERY_THROW);

            util::URL aURL;
            if ( xOfficeHelp->getByHierarchicalName("['UpdateCheckJob']/URL") >>= aURL.Complete )
            {
                uno::Reference< util::XURLTransformer > xTransformer( util::URLTransformer::create(xContext) );
                xTransformer->parseStrict( aURL );

                uno::Reference < frame::XDesktop2 > xDesktop = frame::Desktop::create(xContext);

                uno::Reference< frame::XDispatchProvider > xDispatchProvider(
                    xDesktop->getCurrentFrame(), uno::UNO_QUERY );
                if ( !xDispatchProvider.is() )
                    xDispatchProvider = uno::Reference < frame::XDispatchProvider > ( xDesktop, uno::UNO_QUERY );

                uno::Reference< frame::XDispatch > xDispatch =
                    xDispatchProvider->queryDispatch( aURL, OUString(), 0 );
                if ( xDispatch.is() )
                    xDispatch->dispatch( aURL, uno::Sequence< beans::PropertyValue >() );
            }
        }
    }
    catch( const uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    EndDialog( RET_OK );
    return 0;
}

IMPL_LINK_NOARG(NewerVersionWarningDialog, LaterHdl)
{
    EndDialog( RET_ASK_LATER );
    return 0;
}

void NewerVersionWarningDialog::InitButtonWidth()
{
    
    long nBtnTextWidth = m_aUpdateBtn.GetCtrlTextWidth( m_aUpdateBtn.GetText() );
    long nTemp = m_aLaterBtn.GetCtrlTextWidth( m_aLaterBtn.GetText() );
    if ( nTemp > nBtnTextWidth )
        nBtnTextWidth = nTemp;
    nBtnTextWidth = nBtnTextWidth * 115 / 100; 
    long nMaxBtnWidth = LogicToPixel( Size( MAX_BUTTON_WIDTH, 0 ), MAP_APPFONT ).Width();
    nBtnTextWidth = std::min( nBtnTextWidth, nMaxBtnWidth );
    long nButtonWidth = m_aUpdateBtn .GetSizePixel().Width();

    if ( nBtnTextWidth > nButtonWidth )
    {
        long nDelta = nBtnTextWidth - nButtonWidth;
        Point aNewPos = m_aUpdateBtn.GetPosPixel();
        aNewPos.X() -= 2*nDelta;
        Size aNewSize = m_aUpdateBtn.GetSizePixel();
        aNewSize.Width() += nDelta;
        m_aUpdateBtn.SetPosSizePixel( aNewPos, aNewSize );
        aNewPos = m_aLaterBtn.GetPosPixel();
        aNewPos.X() -= nDelta;
        m_aLaterBtn.SetPosSizePixel( aNewPos, aNewSize );
    }
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
