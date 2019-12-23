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

#include <com/sun/star/frame/XDispatchRecorder.hpp>

#include <svl/eitem.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/weldutils.hxx>
#include <vcl/windowstate.hxx>

#include <recfloat.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/sfxresid.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>

SFX_IMPL_MODELESSDIALOGCONTOLLER(SfxRecordingFloatWrapper_Impl, SID_RECORDING_FLOATWINDOW);

SfxRecordingFloatWrapper_Impl::SfxRecordingFloatWrapper_Impl(vcl::Window* pParentWnd,
                                                             sal_uInt16 nId,
                                                             SfxBindings* pBind,
                                                             SfxChildWinInfo const * pInfo)
    : SfxChildWindow(pParentWnd, nId)
    , pBindings(pBind)
{
    SetController(std::make_shared<SfxRecordingFloat_Impl>(pBindings, this, pParentWnd->GetFrameWeld()));
    SetWantsFocus(false);
    SfxRecordingFloat_Impl* pFloatDlg = static_cast<SfxRecordingFloat_Impl*>(GetController().get());

    weld::Dialog* pDlg = pFloatDlg->getDialog();

    SfxViewFrame *pFrame = pBind->GetDispatcher_Impl()->GetFrame();
    vcl::Window* pEditWin = pFrame->GetViewShell()->GetWindow();

    Point aPos = pEditWin->OutputToScreenPixel( pEditWin->GetPosPixel() );
    aPos.AdjustX(20);
    aPos.AdjustY(10);

    WindowStateData aState;
    aState.SetMask(WindowStateMask::Pos);
    aState.SetX(aPos.X());
    aState.SetY(aPos.Y());
    pDlg->set_window_state(aState.ToStr());

    pFloatDlg->Initialize(pInfo);
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
        SfxRecordingFloat_Impl* pFloatDlg = static_cast<SfxRecordingFloat_Impl*>(GetController().get());
        weld::Dialog* pDlg = pFloatDlg->getDialog();

        std::unique_ptr<weld::MessageDialog> xQueryBox(Application::CreateMessageDialog(pDlg,
                                                       VclMessageType::Question, VclButtonsType::YesNo,
                                                       SfxResId(STR_MACRO_LOSS)));
        xQueryBox->set_default_response(RET_NO);

        xQueryBox->set_title(SfxResId(STR_CANCEL_RECORDING));
        bRet = (xQueryBox->run() == RET_YES);
    }

    return bRet;
}

SfxRecordingFloat_Impl::SfxRecordingFloat_Impl(SfxBindings* pBind, SfxChildWindow* pChildWin,
                                               weld::Window* pParent)
    : SfxModelessDialogController(pBind, pChildWin, pParent, "sfx/ui/floatingrecord.ui",
                                  "FloatingRecord")
    , m_xToolbar(m_xBuilder->weld_toolbar("toolbar"))
    , m_aDispatcher(*m_xToolbar, pBind->GetActiveFrame())
{
    // start recording
    SfxBoolItem aItem( SID_RECORDMACRO, true );
    GetBindings().GetDispatcher()->ExecuteList(SID_RECORDMACRO,
            SfxCallMode::SYNCHRON, { &aItem });
}

SfxRecordingFloat_Impl::~SfxRecordingFloat_Impl()
{
}

void SfxRecordingFloat_Impl::FillInfo( SfxChildWinInfo& rInfo ) const
{
    SfxModelessDialogController::FillInfo( rInfo );
    rInfo.bVisible = false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
