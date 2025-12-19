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

#include <comphelper/dispatchcommand.hxx>
#include <officecfg/Office/Common.hxx>
#include <charmapcontrol.hxx>
#include <charmappopup.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/sfxresid.hxx>

using namespace css;

SfxCharmapCtrl::SfxCharmapCtrl(CharmapPopup* pControl, weld::Widget* pParent)
    : WeldToolbarPopup(pControl->getFrameInterface(), pParent, u"sfx/ui/charmapcontrol.ui"_ustr, u"charmapctrl"_ustr)
    , m_xControl(pControl)
    , m_xVirDev(VclPtr<VirtualDevice>::Create())
    , m_aCharmapContents(*m_xBuilder, m_xVirDev, false)
    , m_xRecentLabel(m_xBuilder->weld_label(u"label2"_ustr))
    , m_xDlgBtn(m_xBuilder->weld_button(u"specialchardlg"_ustr))
    , m_xCharInfoLabel(m_xBuilder->weld_label(u"charinfolabel"_ustr))
{
    m_xCharInfoLabel->set_size_request(-1, m_xCharInfoLabel->get_text_height() * 2);

    m_aCharmapContents.init(false, LINK(this, SfxCharmapCtrl, CharClickHdl),
                            Link<void*,void>(), LINK(this, SfxCharmapCtrl, UpdateRecentHdl),
                            LINK(this, SfxCharmapCtrl, CharFocusInHdl));

    m_xDlgBtn->connect_clicked(LINK(this, SfxCharmapCtrl, OpenDlgHdl));
    m_xDlgBtn->connect_focus_in(LINK(this, SfxCharmapCtrl, DlgBtnFocusInHdl));
}

SfxCharmapCtrl::~SfxCharmapCtrl()
{
}

IMPL_LINK(SfxCharmapCtrl, CharFocusInHdl, const CharAndFont&, rChar, void)
{
    m_xCharInfoLabel->set_label(SfxCharmapContainer::GetCharInfoText(rChar.sChar));
}

IMPL_LINK(SfxCharmapCtrl, CharClickHdl, SvxCharView&, rView, void)
{
    m_xControl->EndPopupMode();

    SfxCharmapContainer::InsertCharToDoc(rView.GetCharAndFont());
}

IMPL_LINK_NOARG(SfxCharmapCtrl, OpenDlgHdl, weld::Button&, void)
{
    m_xControl->EndPopupMode();

    if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
    {
        uno::Reference<frame::XFrame> xFrame = pViewFrm->GetFrame().GetFrameInterface();
        comphelper::dispatchCommand(u".uno:InsertSymbol"_ustr, xFrame, {});
    }
}

IMPL_LINK_NOARG(SfxCharmapCtrl, DlgBtnFocusInHdl, weld::Widget&, void)
{
    m_xCharInfoLabel->set_label(u""_ustr);
}

void SfxCharmapCtrl::GrabFocus()
{
    m_aCharmapContents.GrabFocusToFirstFavorite();
}

IMPL_LINK_NOARG(SfxCharmapCtrl, UpdateRecentHdl, void*, void)
{
    //checking if the characters are recently used or no
    m_xRecentLabel->set_label(m_aCharmapContents.hasRecentChars() ? SfxResId(STR_RECENT) : SfxResId(STR_NORECENT));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
