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
#include <sc.hrc>

#include <tools/color.hxx>
#include <sfx2/objsh.hxx>
#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <unotools/pathoptions.hxx>
#include <vcl/builderfactory.hxx>
#include <osl/diagnose.h>

#define HDL(hdl) LINK(this,ScTabBgColorDlg,hdl)

ScTabBgColorDlg::ScTabBgColorDlg(weld::Window* pParent, const OUString& rTitle,
    const OUString& rTabBgColorNoColorText, const Color& rDefaultColor)
    : GenericDialogController(pParent, "modules/scalc/ui/tabcolordialog.ui", "TabColorDialog")
    , m_aTabBgColor(rDefaultColor)
    , m_aTabBgColorNoColorText(rTabBgColorNoColorText)
    , m_xTabBgColorSet(new weld::CustomWeld(*m_xBuilder, "colorset", m_aTabBgColorSet))
    , m_xBtnOk(m_xBuilder->weld_button("ok"))
{
    m_aTabBgColorSet.SetDialog(this);
    m_aTabBgColorSet.SetColCount(SvxColorValueSet::getColumnCount());

    m_xDialog->set_title(rTitle);

    FillColorValueSets_Impl();
    m_aTabBgColorSet.SetDoubleClickHdl(HDL(TabBgColorDblClickHdl_Impl));
    m_xBtnOk->connect_clicked(HDL(TabBgColorOKHdl_Impl));
}

ScTabBgColorDlg::~ScTabBgColorDlg()
{
}

void ScTabBgColorDlg::GetSelectedColor( Color& rColor ) const
{
    rColor = m_aTabBgColor;
}

void ScTabBgColorDlg::FillColorValueSets_Impl()
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SfxPoolItem* pItem = nullptr;
    XColorListRef pColorList;

    OSL_ENSURE( pDocSh, "DocShell not found!" );

    if ( pDocSh && ( nullptr != ( pItem = pDocSh->GetItem(SID_COLOR_TABLE) ) ) )
        pColorList = static_cast<const SvxColorListItem*>(pItem)->GetColorList();
    if ( !pColorList.is() )
        pColorList = XColorList::CreateStdColorList();

    long nColorCount(0);

    if ( pColorList.is() )
    {
        nColorCount = pColorList->Count();
        m_aTabBgColorSet.addEntriesForXColorList(*pColorList);
    }

    if (nColorCount)
    {
        const WinBits nBits(m_aTabBgColorSet.GetStyle() | WB_NAMEFIELD | WB_ITEMBORDER | WB_NONEFIELD | WB_3DLOOK | WB_NO_DIRECTSELECT | WB_NOPOINTERFOCUS);
        m_aTabBgColorSet.SetText(m_aTabBgColorNoColorText);
        m_aTabBgColorSet.SetStyle(nBits);
    }

    //lock down a preferred size
    const sal_uInt32 nColCount = SvxColorValueSet::getColumnCount();
    const sal_uInt32 nRowCount(ceil(double(nColorCount)/nColCount));
    const sal_uInt32 nLength = SvxColorValueSet::getEntryEdgeLength();
    Size aSize(m_aTabBgColorSet.CalcWindowSizePixel(Size(nLength, nLength), nColCount, nRowCount));
    m_xTabBgColorSet->set_size_request(aSize.Width() + 8, aSize.Height() + 8);
    m_aTabBgColorSet.SelectItem(0);
}

///    Handler, called when color selection is changed
IMPL_LINK_NOARG(ScTabBgColorDlg, TabBgColorDblClickHdl_Impl, SvtValueSet*, void)
{
    sal_uInt16 nItemId = m_aTabBgColorSet.GetSelectedItemId();
    Color aColor = nItemId ? ( m_aTabBgColorSet.GetItemColor( nItemId ) ) : COL_AUTO;
    m_aTabBgColor = aColor;
    m_xDialog->response(RET_OK);
}

//    Handler, called when the OK button is pushed
IMPL_LINK_NOARG(ScTabBgColorDlg, TabBgColorOKHdl_Impl, weld::Button&, void)
{
    sal_uInt16 nItemId = m_aTabBgColorSet.GetSelectedItemId();
    Color aColor = nItemId ? ( m_aTabBgColorSet.GetItemColor( nItemId ) ) : COL_AUTO;
    m_aTabBgColor = aColor;
    m_xDialog->response(RET_OK);
}

ScTabBgColorDlg::ScTabBgColorValueSet::ScTabBgColorValueSet()
    : ColorValueSet(nullptr)
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
