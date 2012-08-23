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

#include <com/sun/star/frame/XDispatchRecorderSupplier.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/UICommandDescription.hpp>

#include <svl/eitem.hxx>
#include <svtools/generictoolboxcontroller.hxx>
#include <vcl/msgbox.hxx>
#include <comphelper/componentcontext.hxx>
#include <comphelper/processfactory.hxx>

#include "recfloat.hxx"
#include "dialog.hrc"
#include "sfx2/sfxresid.hxx"
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include "sfx2/imagemgr.hxx"

using namespace ::com::sun::star;

static rtl::OUString GetLabelFromCommandURL( const rtl::OUString& rCommandURL, const uno::Reference< frame::XFrame >& xFrame )
{
    rtl::OUString aLabel;
    rtl::OUString aModuleIdentifier;
    uno::Reference< container::XNameAccess > xUICommandLabels;
    uno::Reference< lang::XMultiServiceFactory > xServiceManager;
    uno::Reference< container::XNameAccess > xUICommandDescription;
    uno::Reference< ::com::sun::star::frame::XModuleManager > xModuleManager;

    static uno::WeakReference< lang::XMultiServiceFactory > xTmpServiceManager;
    static uno::WeakReference< container::XNameAccess >     xTmpNameAccess;
    static uno::WeakReference< ::com::sun::star::frame::XModuleManager > xTmpModuleMgr;

    xServiceManager = xTmpServiceManager;
    if ( !xServiceManager.is() )
    {
        xServiceManager = ::comphelper::getProcessServiceFactory();
        xTmpServiceManager = xServiceManager;
    }

    xUICommandDescription = xTmpNameAccess;
    if ( !xUICommandDescription.is() )
    {
        xUICommandDescription = uno::Reference< container::XNameAccess >(
                                    ui::UICommandDescription::create(comphelper::ComponentContext(xServiceManager).getUNOContext()) );
        xTmpNameAccess = xUICommandDescription;
    }

    xModuleManager = xTmpModuleMgr;
    if ( !xModuleManager.is() )
    {
        xModuleManager = uno::Reference< ::com::sun::star::frame::XModuleManager >(
            xServiceManager->createInstance(
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.frame.ModuleManager" ))),
            uno::UNO_QUERY_THROW );
        xTmpModuleMgr = xModuleManager;
    }

    // Retrieve label from UI command description service
    try
    {
        try
        {
            aModuleIdentifier = xModuleManager->identify( xFrame );
        }
        catch( uno::Exception& )
        {
        }

        if ( xUICommandDescription.is() )
        {
            uno::Any a = xUICommandDescription->getByName( aModuleIdentifier );
            uno::Reference< container::XNameAccess > xUICommands;
            a >>= xUICommandLabels;
        }
    }
    catch ( uno::Exception& )
    {
    }

    if ( xUICommandLabels.is() )
    {
        try
        {
            if ( !rCommandURL.isEmpty() )
            {
                uno::Sequence< beans::PropertyValue > aPropSeq;
                uno::Any a( xUICommandLabels->getByName( rCommandURL ));
                if ( a >>= aPropSeq )
                {
                    for ( sal_Int32 i = 0; i < aPropSeq.getLength(); i++ )
                    {
                        if ( aPropSeq[i].Name == "Label" )
                        {
                            aPropSeq[i].Value >>= aLabel;
                            break;
                        }
                    }
                }
            }
        }
        catch (uno::Exception& )
        {
        }
    }

    return aLabel;
}

SFX_IMPL_FLOATINGWINDOW( SfxRecordingFloatWrapper_Impl, SID_RECORDING_FLOATWINDOW );

SfxRecordingFloatWrapper_Impl::SfxRecordingFloatWrapper_Impl( Window* pParentWnd ,
                                                sal_uInt16 nId ,
                                                SfxBindings* pBind ,
                                                SfxChildWinInfo* pInfo )
                    : SfxChildWindow( pParentWnd, nId )
                    , pBindings( pBind )
{
    pWindow = new SfxRecordingFloat_Impl( pBindings, this, pParentWnd );
    SetWantsFocus( sal_False );
    eChildAlignment = SFX_ALIGN_NOALIGNMENT;
    ( ( SfxFloatingWindow* ) pWindow )->Initialize( pInfo );
}

SfxRecordingFloatWrapper_Impl::~SfxRecordingFloatWrapper_Impl()
{
    SfxBoolItem aItem( FN_PARAM_1, sal_True );
    com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder = pBindings->GetRecorder();
    if ( xRecorder.is() )
        pBindings->GetDispatcher()->Execute( SID_STOP_RECORDING, SFX_CALLMODE_SYNCHRON, &aItem, 0L );
}

sal_Bool SfxRecordingFloatWrapper_Impl::QueryClose()
{
    // asking for recorded macro should be replaced if index access is available!
    sal_Bool bRet = sal_True;
    com::sun::star::uno::Reference< com::sun::star::frame::XDispatchRecorder > xRecorder = pBindings->GetRecorder();
    if ( xRecorder.is() && !xRecorder->getRecordedMacro().isEmpty() )
    {
        QueryBox aBox( GetWindow(), WB_YES_NO | WB_DEF_NO , SfxResId(STR_MACRO_LOSS).toString() );
        aBox.SetText( SfxResId(STR_CANCEL_RECORDING).toString() );
        bRet = ( aBox.Execute() == RET_YES );
    }

    return bRet;
}

SfxRecordingFloat_Impl::SfxRecordingFloat_Impl(
    SfxBindings* pBind ,
    SfxChildWindow* pChildWin ,
    Window* pParent )
    : SfxFloatingWindow( pBind,
                         pChildWin,
                         pParent,
                         SfxResId( SID_RECORDING_FLOATWINDOW ) )
    , aTbx( this, SfxResId(SID_RECORDING_FLOATWINDOW) )
{
    // Retrieve label from helper function
    uno::Reference< frame::XFrame > xFrame = GetBindings().GetActiveFrame();
    rtl::OUString aCommandStr( RTL_CONSTASCII_USTRINGPARAM( ".uno:StopRecording" ));
    aTbx.SetItemText( SID_STOP_RECORDING, GetLabelFromCommandURL( aCommandStr, xFrame ));

    // Determine size of toolbar
    Size aTbxSize = aTbx.CalcWindowSizePixel();
    aTbx.SetPosSizePixel( Point(), aTbxSize );
    SetOutputSizePixel( aTbxSize );

    // create a generic toolbox controller for our internal toolbox
    svt::GenericToolboxController* pController = new svt::GenericToolboxController(
                                                    ::comphelper::getProcessServiceFactory(),
                                                    xFrame,
                                                    &aTbx,
                                                    SID_STOP_RECORDING,
                                                    aCommandStr );
    xStopRecTbxCtrl = uno::Reference< frame::XToolbarController >(
                            static_cast< cppu::OWeakObject* >( pController ),
                        uno::UNO_QUERY );
    uno::Reference< util::XUpdatable > xUpdate( xStopRecTbxCtrl, uno::UNO_QUERY );
    if ( xUpdate.is() )
        xUpdate->update();

    aTbx.SetSelectHdl( LINK( this, SfxRecordingFloat_Impl, Select ) );

    // start recording
    SfxBoolItem aItem( SID_RECORDMACRO, sal_True );
    GetBindings().GetDispatcher()->Execute( SID_RECORDMACRO, SFX_CALLMODE_SYNCHRON, &aItem, 0L );
}

SfxRecordingFloat_Impl::~SfxRecordingFloat_Impl()
{
    try
    {
        if ( xStopRecTbxCtrl.is() )
        {
            uno::Reference< lang::XComponent > xComp( xStopRecTbxCtrl, uno::UNO_QUERY );
            xComp->dispose();
        }
    }
    catch ( uno::Exception& )
    {
    }
}

sal_Bool SfxRecordingFloat_Impl::Close()
{
    sal_Bool bRet = SfxFloatingWindow::Close();
    return bRet;
}

void SfxRecordingFloat_Impl::FillInfo( SfxChildWinInfo& rInfo ) const
{
    SfxFloatingWindow::FillInfo( rInfo );
    rInfo.bVisible = sal_False;
}

void SfxRecordingFloat_Impl::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == STATE_CHANGE_INITSHOW )
    {
        SfxViewFrame *pFrame = GetBindings().GetDispatcher_Impl()->GetFrame();
        Window* pEditWin = pFrame->GetViewShell()->GetWindow();

        Point aPoint = pEditWin->OutputToScreenPixel( pEditWin->GetPosPixel() );
        aPoint = GetParent()->ScreenToOutputPixel( aPoint );
        aPoint.X() += 20;
        aPoint.Y() += 10;
        SetPosPixel( aPoint );
    }

    SfxFloatingWindow::StateChanged( nStateChange );
}

IMPL_LINK( SfxRecordingFloat_Impl, Select, ToolBox*, pToolBar )
{
    (void)pToolBar;
    sal_Int16   nKeyModifier( (sal_Int16)aTbx.GetModifier() );
    if ( xStopRecTbxCtrl.is() )
        xStopRecTbxCtrl->execute( nKeyModifier );

    return 1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
