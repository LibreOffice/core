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

#undef SC_DLLIMPLEMENTATION

#include <scresid.hxx>
#include <strings.hrc>
#include <tabbgcolordlg.hxx>

#include <tools/color.hxx>
#include <vcl/event.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/weld/ScrolledWindow.hxx>
#include <vcl/weld/Dialog.hxx>
#include <vcl/weld/IconView.hxx>

#include <officecfg/Office/Common.hxx>

ScTabBgColorDlg::ScTabBgColorDlg(weld::Window* pParent, const OUString& rTitle)
    : GenericDialogController(pParent, u"modules/scalc/ui/tabcolordialog.ui"_ustr,
                              u"TabColorDialog"_ustr)
    , m_xSelectPalette(m_xBuilder->weld_combo_box(u"paletteselector"_ustr))
    , m_xDefaultButton(m_xBuilder->weld_toggle_button(u"defaultbutton"_ustr))
    , m_aTabBgColorIconView(m_xBuilder->weld_icon_view(u"coloriconview"_ustr))
    , m_xBtnOk(m_xBuilder->weld_button(u"ok"_ustr))
{
    m_xDialog->set_title(rTitle);

    FillPaletteLB();

    m_xSelectPalette->connect_changed(LINK(this, ScTabBgColorDlg, SelectPaletteLBHdl));
    m_xDefaultButton->connect_toggled(LINK(this, ScTabBgColorDlg, DefaultButtonToggled));
    m_aTabBgColorIconView.setSelectionChangedHdl(LINK(this, ScTabBgColorDlg, TabBgColorSelectHdl));
    m_aTabBgColorIconView.setColorActivatedHdl(LINK(this, ScTabBgColorDlg, TabBgColorActivatedHdl));
    m_xBtnOk->connect_clicked(LINK(this, ScTabBgColorDlg, TabBgColorOKHdl_Impl));
}

ScTabBgColorDlg::~ScTabBgColorDlg()
{
}

Color ScTabBgColorDlg::GetSelectedColor() const
{
    if (m_xDefaultButton->get_active())
        return COL_AUTO;

    const int nIndex = m_aTabBgColorIconView.get_selected_index();
    Color aColor = nIndex >= 0 ? (m_aTabBgColorIconView.getColor(nIndex)) : COL_AUTO;
    return aColor;
}

void ScTabBgColorDlg::FillPaletteLB()
{
    m_xSelectPalette->clear();
    std::vector<OUString> aPaletteList = m_aPaletteManager.GetPaletteList();
    for (auto const& palette : aPaletteList)
    {
        m_xSelectPalette->append_text(palette);
    }
    OUString aPaletteName( officecfg::Office::Common::UserColors::PaletteName::get() );
    m_xSelectPalette->set_active_text(aPaletteName);
    if (m_xSelectPalette->get_active() != -1)
    {
        SelectPaletteLBHdl(*m_xSelectPalette);
    }
}

IMPL_LINK_NOARG(ScTabBgColorDlg, SelectPaletteLBHdl, weld::ComboBox&, void)
{
    m_aTabBgColorIconView.clear();
    sal_Int32 nPos = m_xSelectPalette->get_active();
    m_aPaletteManager.SetPalette( nPos );
    m_aPaletteManager.ReloadColorSet(m_aTabBgColorIconView);
    m_xDefaultButton->set_active(true);
    m_aTabBgColorIconView.unselect_all();
}

IMPL_LINK_NOARG(ScTabBgColorDlg, DefaultButtonToggled, weld::Toggleable&, void)
{
    // Only allow toggling default color on, not off by clicking the button.
    // Disabling instead happens by selecting another color.
    if (!m_xDefaultButton->get_active())
    {
        m_xDefaultButton->set_active(true);
        return;
    }

    m_aTabBgColorIconView.unselect_all();
}

//    Handler, called when color selection is changed
IMPL_LINK_NOARG(ScTabBgColorDlg, TabBgColorSelectHdl, const ColorIconView&, void)
{
    m_xDefaultButton->set_active(false);
}

IMPL_LINK_NOARG(ScTabBgColorDlg, TabBgColorActivatedHdl, const Color&, void)
{
    m_xDialog->response(RET_OK);
}

//    Handler, called when the OK button is pushed
IMPL_LINK_NOARG(ScTabBgColorDlg, TabBgColorOKHdl_Impl, weld::Button&, void)
{
    m_xDialog->response(RET_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
