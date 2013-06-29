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

#include "newerverwarn.hxx"
#include "newerverwarn.hrc"
#include "ids.hrc"

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>
#include <com/sun/star/setup/UpdateCheckConfig.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/container/XNameReplace.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/configurationhelper.hxx>
#include <rtl/bootstrap.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/msgbox.hxx>
#include <osl/process.h>

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
    // detect execute path
    OUString sProgramPath;
    osl_getExecutableFile( &sProgramPath.pData );
    sal_uInt32 nLastIndex = sProgramPath.lastIndexOf( '/' );
    if ( nLastIndex > 0 )
        sProgramPath = sProgramPath.copy( 0, nLastIndex + 1 );

    // read keys from soffice.ini (sofficerc)
    OUString sIniFileName = sProgramPath;
    sIniFileName += OUString( SAL_CONFIGFILE( "version" ) );
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
            uno::Reference < container::XNameReplace > xUpdateConfig =
                setup::UpdateCheckConfig::create(xContext);

            sal_Bool bUpdateCheckEnabled = sal_False;
            OSL_VERIFY( xUpdateConfig->getByName("AutoCheckEnabled") >>= bUpdateCheckEnabled );

            // TODO: do we need to respect the bUpdateCheckEnabled flag? Finally, its meaning is "are automatic
            // updates enabled", but this here is not an automatic update, but one triggered explicitly by the user.

            uno::Any aVal = ::comphelper::ConfigurationHelper::readDirectKey(
                                    xContext,
                                    "org.openoffice.Office.Addons/",
                                    "AddonUI/OfficeHelp/UpdateCheckJob",
                                    "URL",
                                    ::comphelper::ConfigurationHelper::E_READONLY );
            util::URL aURL;
            if ( aVal >>= aURL.Complete )
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
    // one button too small for its text?
    long nBtnTextWidth = m_aUpdateBtn.GetCtrlTextWidth( m_aUpdateBtn.GetText() );
    long nTemp = m_aLaterBtn.GetCtrlTextWidth( m_aLaterBtn.GetText() );
    if ( nTemp > nBtnTextWidth )
        nBtnTextWidth = nTemp;
    nBtnTextWidth = nBtnTextWidth * 115 / 100; // a little offset
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

} // end of namespace uui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
