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

#include <svl/eitem.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>

#include <recfloat.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/sfxresid.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>

SFX_IMPL_FLOATINGWINDOW( SfxRecordingFloatWrapper_Impl, SID_RECORDING_FLOATWINDOW );

SfxRecordingFloatWrapper_Impl::SfxRecordingFloatWrapper_Impl( vcl::Window* pParentWnd ,
                                                sal_uInt16 nId ,
                                                SfxBindings* pBind ,
                                                SfxChildWinInfo const * pInfo )
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
        pBindings->GetDispatcher()->ExecuteList(SID_STOP_RECORDING,
                SfxCallMode::SYNCHRON, { &aItem });
}

bool SfxRecordingFloatWrapper_Impl::QueryClose()
{
    // asking for recorded macro should be replaced if index access is available!
    bool bRet = true;
    css::uno::Reference< css::frame::XDispatchRecorder > xRecorder = pBindings->GetRecorder();
    if ( xRecorder.is() && !xRecorder->getRecordedMacro().isEmpty() )
    {
        vcl::Window* pWin = GetWindow();
        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(pWin ? pWin->GetFrameWeld() : nullptr,
                                                       VclMessageType::Question, VclButtonsType::YesNo,
                                                       SfxResId(STR_MACRO_LOSS)));
        xQueryBox->set_default_response(RET_NO);

        xQueryBox->set_title(SfxResId(STR_CANCEL_RECORDING));
        bRet = (xQueryBox->run() == RET_YES);
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
    // start recording
    SfxBoolItem aItem( SID_RECORDMACRO, true );
    GetBindings().GetDispatcher()->ExecuteList(SID_RECORDMACRO,
            SfxCallMode::SYNCHRON, { &aItem });
}

SfxRecordingFloat_Impl::~SfxRecordingFloat_Impl()
{
    disposeOnce();
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
        aPoint.AdjustX(20 );
        aPoint.AdjustY(10 );
        SetPosPixel( aPoint );
    }

    SfxFloatingWindow::StateChanged( nStateChange );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
