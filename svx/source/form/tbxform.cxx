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

#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/stritem.hxx>
#include <vcl/event.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/settings.hxx>

#include <svx/dialmgr.hxx>
#include <svx/labelitemwindow.hxx>
#include <svx/svxids.hrc>
#include <svx/strings.hrc>
#include <tbxform.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

SvxFmAbsRecWin::SvxFmAbsRecWin(vcl::Window* pParent, SfxToolBoxControl* pController)
    : InterimItemWindow(pParent, "svx/ui/absrecbox.ui", "AbsRecBox")
    , m_xWidget(m_xBuilder->weld_entry("entry"))
    , m_pController(pController)
{
    m_xWidget->connect_key_press(LINK(this, SvxFmAbsRecWin, KeyInputHdl));
    m_xWidget->connect_activate(LINK(this, SvxFmAbsRecWin, ActivatedHdl));
    m_xWidget->connect_focus_out(LINK(this, SvxFmAbsRecWin, FocusOutHdl));

    SetSizePixel(m_xWidget->get_preferred_size());
}

void SvxFmAbsRecWin::dispose()
{
    m_xWidget.reset();
    InterimItemWindow::dispose();
}

void SvxFmAbsRecWin::GetFocus()
{
    if (m_xWidget)
        m_xWidget->grab_focus();
    InterimItemWindow::GetFocus();
}

SvxFmAbsRecWin::~SvxFmAbsRecWin()
{
    disposeOnce();
}

void SvxFmAbsRecWin::FirePosition( bool _bForce )
{
    if (!_bForce && !m_xWidget->get_value_changed_from_saved())
        return;

    sal_Int64 nRecord = m_xWidget->get_text().toInt64();
    if (nRecord < 1)
        nRecord = 1;

    SfxInt32Item aPositionParam( FN_PARAM_1, static_cast<sal_Int32>(nRecord) );

    Any a;
    Sequence< PropertyValue > aArgs( 1 );
    aArgs[0].Name = "Position";
    aPositionParam.QueryValue( a );
    aArgs[0].Value = a;
    m_pController->Dispatch( ".uno:AbsoluteRecord",
                             aArgs );
    m_pController->updateStatus();

    m_xWidget->save_value();
}

IMPL_LINK_NOARG(SvxFmAbsRecWin, FocusOutHdl, weld::Widget&, void)
{
    FirePosition( false );
}

IMPL_LINK(SvxFmAbsRecWin, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    return ChildKeyInput(rKEvt);
}

IMPL_LINK_NOARG(SvxFmAbsRecWin, ActivatedHdl, weld::Entry&, bool)
{
    if (!m_xWidget->get_text().isEmpty())
        FirePosition( true );
    return true;
}

SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxCtlAbsRec, SfxInt32Item );

SvxFmTbxCtlAbsRec::SvxFmTbxCtlAbsRec( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx )
    :SfxToolBoxControl( nSlotId, nId, rTbx )
{
}


SvxFmTbxCtlAbsRec::~SvxFmTbxCtlAbsRec()
{
}

void SvxFmTbxCtlAbsRec::StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    sal_uInt16              nId = GetId();
    ToolBox*            pToolBox = &GetToolBox();
    SvxFmAbsRecWin*     pWin = static_cast<SvxFmAbsRecWin*>( pToolBox->GetItemWindow(nId) );

    assert(pWin && "Control not found!");

    if (pState)
    {
        const SfxInt32Item* pItem = dynamic_cast< const SfxInt32Item* >( pState );
        DBG_ASSERT( pItem, "SvxFmTbxCtlAbsRec::StateChanged: invalid item!" );
        pWin->set_text(OUString::number(pItem ? pItem->GetValue() : -1));
    }

    bool bEnable = SfxItemState::DISABLED != eState && pState;
    if (!bEnable)
        pWin->set_text(OUString());


    // enabling/disabling of the window
    pToolBox->EnableItem(nId, bEnable);
    SfxToolBoxControl::StateChanged( nSID, eState,pState );
}

VclPtr<InterimItemWindow> SvxFmTbxCtlAbsRec::CreateItemWindow( vcl::Window* pParent )
{
    return VclPtrInstance<SvxFmAbsRecWin>(pParent, this);
}

SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxCtlRecText, SfxBoolItem );

SvxFmTbxCtlRecText::SvxFmTbxCtlRecText( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx )
    :SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemWindowNonInteractive(nId, true);
}

SvxFmTbxCtlRecText::~SvxFmTbxCtlRecText()
{
}

VclPtr<InterimItemWindow> SvxFmTbxCtlRecText::CreateItemWindow( vcl::Window* pParent )
{
    OUString aText(SvxResId(RID_STR_REC_TEXT));
    VclPtrInstance<LabelItemWindow> xFixedText(pParent, aText);

    xFixedText->Show();

    return xFixedText;
}

SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxCtlRecFromText, SfxBoolItem );

SvxFmTbxCtlRecFromText::SvxFmTbxCtlRecFromText( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx )
    :SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemWindowNonInteractive(nId, true);
}

SvxFmTbxCtlRecFromText::~SvxFmTbxCtlRecFromText()
{
}

VclPtr<InterimItemWindow> SvxFmTbxCtlRecFromText::CreateItemWindow( vcl::Window* pParent )
{
    OUString aText(SvxResId(RID_STR_REC_FROM_TEXT));
    VclPtrInstance<LabelItemWindow> xFixedText(pParent, aText);

    xFixedText->Show();

    return xFixedText;
}

SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxCtlRecTotal, SfxStringItem );

SvxFmTbxCtlRecTotal::SvxFmTbxCtlRecTotal( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx )
    : SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemWindowNonInteractive(nId, true);
}

SvxFmTbxCtlRecTotal::~SvxFmTbxCtlRecTotal()
{
}

VclPtr<InterimItemWindow> SvxFmTbxCtlRecTotal::CreateItemWindow( vcl::Window* pParent )
{
    OUString const aSample("123456");
    m_xFixedText.reset(VclPtr<LabelItemWindow>::Create(pParent, aSample));
    m_xFixedText->set_label("");

    m_xFixedText->Show();

    return m_xFixedText;
}

void SvxFmTbxCtlRecTotal::StateChanged( sal_uInt16 nSID, SfxItemState eState, const SfxPoolItem* pState )
{
    // setting the FixedText
    if (GetSlotId() != SID_FM_RECORD_TOTAL)
        return;

    OUString aText;
    if (pState)
        aText = static_cast<const SfxStringItem*>(pState)->GetValue();
    else
        aText = "?";

    m_xFixedText->set_label(aText);

    SfxToolBoxControl::StateChanged( nSID, eState,pState );
}

SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxNextRec, SfxBoolItem );


SvxFmTbxNextRec::SvxFmTbxNextRec( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx )
    :SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits(nId, rTbx.GetItemBits(nId) | ToolBoxItemBits::REPEAT);

    AllSettings aSettings = rTbx.GetSettings();
    MouseSettings aMouseSettings = aSettings.GetMouseSettings();
    aMouseSettings.SetButtonRepeat(aMouseSettings.GetButtonRepeat() / 4);
    aSettings.SetMouseSettings(aMouseSettings);
    rTbx.SetSettings(aSettings, true);
}

SFX_IMPL_TOOLBOX_CONTROL( SvxFmTbxPrevRec, SfxBoolItem );


SvxFmTbxPrevRec::SvxFmTbxPrevRec( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx )
    :SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits(nId, rTbx.GetItemBits(nId) | ToolBoxItemBits::REPEAT);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
