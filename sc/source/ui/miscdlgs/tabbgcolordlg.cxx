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

#include <tabbgcolordlg.hxx>

#include <tools/color.hxx>
#include <vcl/event.hxx>

#include <officecfg/Office/Common.hxx>

#define HDL(hdl) LINK(this,ScTabBgColorDlg,hdl)

ScTabBgColorDlg::ScTabBgColorDlg(weld::Window* pParent, const OUString& rTitle,
    const OUString& rTabBgColorNoColorText, const Color& rDefaultColor)
    : GenericDialogController(pParent, "modules/scalc/ui/tabcolordialog.ui", "TabColorDialog")
    , m_aTabBgColor(rDefaultColor)
    , m_xSelectPalette(m_xBuilder->weld_combo_box("paletteselector"))
    , m_xTabBgColorSet(new ScTabBgColorValueSet(m_xBuilder->weld_scrolled_window("colorsetwin")))
    , m_xTabBgColorSetWin(new weld::CustomWeld(*m_xBuilder, "colorset", *m_xTabBgColorSet))
    , m_xBtnOk(m_xBuilder->weld_button("ok"))
{
    m_xTabBgColorSet->SetDialog(this);
    m_xTabBgColorSet->SetColCount(SvxColorValueSet::getColumnCount());

    m_xDialog->set_title(rTitle);

    const WinBits nBits(m_xTabBgColorSet->GetStyle() | WB_NAMEFIELD | WB_ITEMBORDER | WB_NONEFIELD | WB_3DLOOK | WB_NO_DIRECTSELECT | WB_NOPOINTERFOCUS);
    m_xTabBgColorSet->SetStyle(nBits);
    m_xTabBgColorSet->SetText(rTabBgColorNoColorText);

    const sal_uInt32 nColCount = SvxColorValueSet::getColumnCount();
    const sal_uInt32 nRowCount(10);
    const sal_uInt32 nLength = SvxColorValueSet::getEntryEdgeLength();
    Size aSize(m_xTabBgColorSet->CalcWindowSizePixel(Size(nLength, nLength), nColCount, nRowCount));
    m_xTabBgColorSetWin->set_size_request(aSize.Width() + 8, aSize.Height() + 8);

    FillPaletteLB();

    m_xSelectPalette->connect_changed(LINK(this, ScTabBgColorDlg, SelectPaletteLBHdl));
    m_xTabBgColorSet->SetDoubleClickHdl(HDL(TabBgColorDblClickHdl_Impl));
    m_xBtnOk->connect_clicked(HDL(TabBgColorOKHdl_Impl));
}

ScTabBgColorDlg::~ScTabBgColorDlg()
{
}

void ScTabBgColorDlg::GetSelectedColor( Color& rColor ) const
{
    rColor = m_aTabBgColor;
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
    m_xTabBgColorSet->Clear();
    sal_Int32 nPos = m_xSelectPalette->get_active();
    m_aPaletteManager.SetPalette( nPos );
    m_aPaletteManager.ReloadColorSet(*m_xTabBgColorSet);
    m_xTabBgColorSet->Resize();
    m_xTabBgColorSet->SelectItem(0);
}

//    Handler, called when color selection is changed
IMPL_LINK_NOARG(ScTabBgColorDlg, TabBgColorDblClickHdl_Impl, SvtValueSet*, void)
{
    sal_uInt16 nItemId = m_xTabBgColorSet->GetSelectedItemId();
    Color aColor = nItemId ? ( m_xTabBgColorSet->GetItemColor( nItemId ) ) : COL_AUTO;
    m_aTabBgColor = aColor;
    m_xDialog->response(RET_OK);
}

//    Handler, called when the OK button is pushed
IMPL_LINK_NOARG(ScTabBgColorDlg, TabBgColorOKHdl_Impl, weld::Button&, void)
{
    sal_uInt16 nItemId = m_xTabBgColorSet->GetSelectedItemId();
    Color aColor = nItemId ? ( m_xTabBgColorSet->GetItemColor( nItemId ) ) : COL_AUTO;
    m_aTabBgColor = aColor;
    m_xDialog->response(RET_OK);
}

ScTabBgColorDlg::ScTabBgColorValueSet::ScTabBgColorValueSet(std::unique_ptr<weld::ScrolledWindow> pWindow)
    : ColorValueSet(std::move(pWindow))
    , m_pTabBgColorDlg(nullptr)
{
}

ScTabBgColorDlg::ScTabBgColorValueSet::~ScTabBgColorValueSet()
{
}

bool ScTabBgColorDlg::ScTabBgColorValueSet::KeyInput( const KeyEvent& rKEvt )
{
    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_SPACE:
        case KEY_RETURN:
        {
            sal_uInt16 nItemId = GetSelectedItemId();
            const Color& aColor = nItemId ? ( GetItemColor( nItemId ) ) : COL_AUTO;
            m_pTabBgColorDlg->m_aTabBgColor = aColor;
            m_pTabBgColorDlg->response(RET_OK);
            return true;
        }
        break;
    }
    return ColorValueSet::KeyInput(rKEvt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
