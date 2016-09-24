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

#include <svl/eitem.hxx>
#include <vcl/msgbox.hxx>

#include "recfloat.hxx"
#include "dialog.hrc"
#include <sfx2/sfxresid.hxx>
#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/imagemgr.hxx>

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
    // start recording
    SfxBoolItem aItem( SID_RECORDMACRO, true );
    GetBindings().GetDispatcher()->ExecuteList(SID_RECORDMACRO,
            SfxCallMode::SYNCHRON, { &aItem });
}

SfxRecordingFloat_Impl::~SfxRecordingFloat_Impl()
{
    disposeOnce();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
