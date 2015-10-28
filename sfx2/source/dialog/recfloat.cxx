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
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/theUICommandDescription.hpp>

#include <svl/eitem.hxx>
#include <svtools/generictoolboxcontroller.hxx>
#include <vcl/msgbox.hxx>
#include <comphelper/processfactory.hxx>

#include "recfloat.hxx"
#include "dialog.hrc"
#include <sfx2/sfxresid.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/imagemgr.hxx>

using namespace ::com::sun::star;

static OUString GetLabelFromCommandURL( const OUString& rCommandURL, const uno::Reference< frame::XFrame >& xFrame )
{
    OUString aLabel;
    OUString aModuleIdentifier;
    uno::Reference< container::XNameAccess > xUICommandLabels;
    uno::Reference< uno::XComponentContext > xContext;
    uno::Reference< container::XNameAccess > xUICommandDescription;
    uno::Reference< css::frame::XModuleManager2 > xModuleManager;

    static uno::WeakReference< uno::XComponentContext > xTmpContext;
    static uno::WeakReference< container::XNameAccess > xTmpNameAccess;
    static uno::WeakReference< css::frame::XModuleManager2 > xTmpModuleMgr;

    xContext = xTmpContext;
    if ( !xContext.is() )
    {
        xContext = ::comphelper::getProcessComponentContext();
        xTmpContext = xContext;
    }

    xUICommandDescription = xTmpNameAccess;
    if ( !xUICommandDescription.is() )
    {
        xUICommandDescription = frame::theUICommandDescription::get(xContext);
        xTmpNameAccess = xUICommandDescription;
    }

    xModuleManager = xTmpModuleMgr;
    if ( !xModuleManager.is() )
    {
        xModuleManager = frame::ModuleManager::create(xContext);
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

        uno::Any a = xUICommandDescription->getByName( aModuleIdentifier );
        uno::Reference< container::XNameAccess > xUICommands;
        a >>= xUICommandLabels;
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

SfxRecordingFloatWrapper_Impl::SfxRecordingFloatWrapper_Impl( vcl::Window* pParentWnd ,
                                                sal_uInt16 nId ,
                                                SfxBindings* pBind ,
                                                SfxChildWinInfo* pInfo )
                    : SfxChildWindow( pParentWnd, nId )
                    , pBindings( pBind )
{
    SetWindow( VclPtr<SfxRecordingFloat_Impl>::Create( pBindings, this, pParentWnd ) );
    SetWantsFocus( false );
    static_cast<SfxFloatingWindow*>(GetWindow())->Initialize( pInfo );
}

SfxRecordingFloatWrapper_Impl::~SfxRecordingFloatWrapper_Impl()
{
    SfxBoolItem aItem( FN_PARAM_1, true );
    css::uno::Reference< css::frame::XDispatchRecorder > xRecorder = pBindings->GetRecorder();
    if ( xRecorder.is() )
        pBindings->GetDispatcher()->Execute( SID_STOP_RECORDING, SfxCallMode::SYNCHRON, &aItem, 0L );
}

bool SfxRecordingFloatWrapper_Impl::QueryClose()
{
    // asking for recorded macro should be replaced if index access is available!
    bool bRet = true;
    css::uno::Reference< css::frame::XDispatchRecorder > xRecorder = pBindings->GetRecorder();
    if ( xRecorder.is() && !xRecorder->getRecordedMacro().isEmpty() )
    {
        ScopedVclPtrInstance< QueryBox > aBox(GetWindow(), WB_YES_NO | WB_DEF_NO , SfxResId(STR_MACRO_LOSS).toString());
        aBox->SetText( SfxResId(STR_CANCEL_RECORDING).toString() );
        bRet = ( aBox->Execute() == RET_YES );
    }

    return bRet;
}

SfxRecordingFloat_Impl::SfxRecordingFloat_Impl(
    SfxBindings* pBind ,
    SfxChildWindow* pChildWin ,
    vcl::Window* pParent )
    : SfxFloatingWindow( pBind,
                         pChildWin,
                         pParent,
                         "FloatingRecord", "sfx/ui/floatingrecord.ui", pBind->GetActiveFrame() )
{
    get(m_pTbx, "toolbar");

    // Retrieve label from helper function
    uno::Reference< frame::XFrame > xFrame = getFrame();
    OUString aCommandStr( ".uno:StopRecording" );
    sal_uInt16 nItemId = m_pTbx->GetItemId(aCommandStr);
    m_pTbx->SetItemText( nItemId, GetLabelFromCommandURL( aCommandStr, xFrame ));

    // create a generic toolbox controller for our internal toolbox
    svt::GenericToolboxController* pController = new svt::GenericToolboxController(
                                                    ::comphelper::getProcessComponentContext(),
                                                    xFrame,
                                                    m_pTbx,
                                                    nItemId,
                                                    aCommandStr );
    xStopRecTbxCtrl = uno::Reference< frame::XToolbarController >(
                            static_cast< cppu::OWeakObject* >( pController ),
                        uno::UNO_QUERY );
    uno::Reference< util::XUpdatable > xUpdate( xStopRecTbxCtrl, uno::UNO_QUERY );
    if ( xUpdate.is() )
        xUpdate->update();

    m_pTbx->SetSelectHdl( LINK( this, SfxRecordingFloat_Impl, Select ) );

    // start recording
    SfxBoolItem aItem( SID_RECORDMACRO, true );
    GetBindings().GetDispatcher()->Execute( SID_RECORDMACRO, SfxCallMode::SYNCHRON, &aItem, 0L );
}

SfxRecordingFloat_Impl::~SfxRecordingFloat_Impl()
{
    disposeOnce();
}

void SfxRecordingFloat_Impl::dispose()
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
    m_pTbx.clear();
    SfxFloatingWindow::dispose();
}

bool SfxRecordingFloat_Impl::Close()
{
    bool bRet = SfxFloatingWindow::Close();
    return bRet;
}

void SfxRecordingFloat_Impl::FillInfo( SfxChildWinInfo& rInfo ) const
{
    SfxFloatingWindow::FillInfo( rInfo );
    rInfo.bVisible = false;
}

void SfxRecordingFloat_Impl::StateChanged( StateChangedType nStateChange )
{
    if ( nStateChange == StateChangedType::InitShow )
    {
        SfxViewFrame *pFrame = GetBindings().GetDispatcher_Impl()->GetFrame();
        vcl::Window* pEditWin = pFrame->GetViewShell()->GetWindow();

        Point aPoint = pEditWin->OutputToScreenPixel( pEditWin->GetPosPixel() );
        aPoint = GetParent()->ScreenToOutputPixel( aPoint );
        aPoint.X() += 20;
        aPoint.Y() += 10;
        SetPosPixel( aPoint );
    }

    SfxFloatingWindow::StateChanged( nStateChange );
}

IMPL_LINK_TYPED( SfxRecordingFloat_Impl, Select, ToolBox*, pToolBar, void )
{
    (void)pToolBar;
    sal_Int16   nKeyModifier( (sal_Int16)m_pTbx->GetModifier() );
    if ( xStopRecTbxCtrl.is() )
        xStopRecTbxCtrl->execute( nKeyModifier );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
