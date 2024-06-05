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

#include <pagenumberdlg.hxx>
#include <svx/SvxNumOptionsTabPageHelper.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/graph.hxx>
#include <vcl/BitmapTools.hxx>
#include <vcl/virdev.hxx>
#include <tools/gen.hxx>

SwPageNumberDlg::SwPageNumberDlg(weld::Window* pParent)
    : SfxDialogController(pParent, u"modules/swriter/ui/pagenumberdlg.ui"_ustr,
                          u"PageNumberDialog"_ustr)
    , m_xOk(m_xBuilder->weld_button(u"ok"_ustr))
    , m_xCancel(m_xBuilder->weld_button(u"cancel"_ustr))
    , m_xPageNumberPosition(m_xBuilder->weld_combo_box(u"positionCombo"_ustr))
    , m_xPageNumberAlignment(m_xBuilder->weld_combo_box(u"alignmentCombo"_ustr))
    , m_xMirrorOnEvenPages(m_xBuilder->weld_check_button(u"mirrorCheckbox"_ustr))
    , m_xIncludePageTotal(m_xBuilder->weld_check_button(u"pagetotalCheckbox"_ustr))
    , m_xPageNumberTypeLB(new SvxPageNumberListBox(m_xBuilder->weld_combo_box(u"numfmtlb"_ustr)))
    , m_xPreviewImage(m_xBuilder->weld_image(u"previewImage"_ustr))
    , m_aPageNumberPosition(1) // bottom
    , m_aPageNumberAlignment(1) // center
    , m_nPageNumberType(SVX_NUM_CHARS_UPPER_LETTER)
{
    m_xOk->connect_clicked(LINK(this, SwPageNumberDlg, OkHdl));
    m_xPageNumberPosition->connect_changed(LINK(this, SwPageNumberDlg, PositionSelectHdl));
    m_xPageNumberAlignment->connect_changed(LINK(this, SwPageNumberDlg, AlignmentSelectHdl));
    m_xPageNumberPosition->set_active(m_aPageNumberPosition);
    m_xPageNumberAlignment->set_active(m_aPageNumberAlignment);
    m_xMirrorOnEvenPages->set_sensitive(false);
    m_xMirrorOnEvenPages->set_state(TRISTATE_TRUE);
    m_xIncludePageTotal->set_state(TRISTATE_FALSE);
    SvxNumOptionsTabPageHelper::GetI18nNumbering(m_xPageNumberTypeLB->get_widget(),
                                                 ::std::numeric_limits<sal_uInt16>::max());
    m_xPageNumberTypeLB->connect_changed(LINK(this, SwPageNumberDlg, NumberTypeSelectHdl));
    m_xIncludePageTotal->connect_toggled(LINK(this, SwPageNumberDlg, IncludePageTotalChangeHdl));
    updateImage();
}

IMPL_LINK_NOARG(SwPageNumberDlg, OkHdl, weld::Button&, void) { m_xDialog->response(RET_OK); }

IMPL_LINK_NOARG(SwPageNumberDlg, PositionSelectHdl, weld::ComboBox&, void)
{
    m_aPageNumberPosition = m_xPageNumberPosition->get_active();
    updateImage();
}

IMPL_LINK_NOARG(SwPageNumberDlg, AlignmentSelectHdl, weld::ComboBox&, void)
{
    m_aPageNumberAlignment = m_xPageNumberAlignment->get_active();
    updateImage();

    if (m_aPageNumberAlignment == 1) // centered
        m_xMirrorOnEvenPages->set_sensitive(false);
    else
        m_xMirrorOnEvenPages->set_sensitive(true);
}

IMPL_LINK_NOARG(SwPageNumberDlg, NumberTypeSelectHdl, weld::ComboBox&, void)
{
    m_nPageNumberType = m_xPageNumberTypeLB->get_active_id();
}

IMPL_LINK_NOARG(SwPageNumberDlg, IncludePageTotalChangeHdl, weld::Toggleable&, void)
{
    updateImage();
}

bool SwPageNumberDlg::GetMirrorOnEvenPages()
{
    return m_xMirrorOnEvenPages->get_sensitive()
           && m_xMirrorOnEvenPages->get_state() == TRISTATE_TRUE;
}

bool SwPageNumberDlg::GetIncludePageTotal()
{
    return m_xIncludePageTotal->get_state() == TRISTATE_TRUE;
}

void SwPageNumberDlg::SetPageNumberType(SvxNumType nSet)
{
    m_nPageNumberType = nSet;
    m_xPageNumberTypeLB->set_active_id(nSet);
}

void SwPageNumberDlg::updateImage()
{
    int nBackgroundWidth = 75;
    int nBackgroundHeight = 105;

    int nMargin = 7;

    ScopedVclPtrInstance<VirtualDevice> pVirtualDev;
    Size aVDSize(nBackgroundWidth, nBackgroundHeight);
    pVirtualDev->SetOutputSizePixel(aVDSize);
    pVirtualDev->SetBackground(Color(0xF0, 0xF0, 0xF0));
    pVirtualDev->Erase();

    OUString sText = u"#"_ustr;

    if (m_xIncludePageTotal->get_state() == TRISTATE_TRUE)
    {
        sText += " / #";
    }

    DrawTextFlags eFlags = DrawTextFlags::Left;

    if (m_aPageNumberAlignment == 1)
    {
        eFlags = DrawTextFlags::Center;
    }
    else if (m_aPageNumberAlignment == 2)
    {
        eFlags = DrawTextFlags::Right;
    }

    eFlags |= m_aPageNumberPosition ? DrawTextFlags::Bottom : DrawTextFlags::Top;

    pVirtualDev->DrawText(
        tools::Rectangle(nMargin, nMargin, nBackgroundWidth - nMargin, nBackgroundHeight - nMargin),
        sText, eFlags);

    m_xPreviewImage->set_image(pVirtualDev);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
