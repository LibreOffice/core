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

#include "tabbgcolordlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"

#include <tools/color.hxx>
#include <sfx2/objsh.hxx>
#include <svx/xtable.hxx>
#include <svx/drawitem.hxx>
#include <unotools/pathoptions.hxx>
#include <tools/resid.hxx>
#include <editeng/editrids.hrc>
#include <editeng/eerdll.hxx>
#include <vcl/builderfactory.hxx>

#include <boost/scoped_ptr.hpp>

#define HDL(hdl) LINK(this,ScTabBgColorDlg,hdl)

ScTabBgColorDlg::ScTabBgColorDlg(vcl::Window* pParent, const OUString& rTitle,
    const OUString& rTabBgColorNoColorText, const Color& rDefaultColor,
    const OString& sHelpId)
    : ModalDialog(pParent, "TabColorDialog", "modules/scalc/ui/tabcolordialog.ui")
    , m_aTabBgColor(rDefaultColor)
    , m_aTabBgColorNoColorText(rTabBgColorNoColorText)

{
    get(m_pTabBgColorSet, "colorset");
    m_pTabBgColorSet->SetDialog(this);
    m_pTabBgColorSet->SetColCount(SvxColorValueSet::getColumnCount());
    get(m_pBtnOk, "ok");

    SetHelpId( sHelpId );
    this->SetText( rTitle );
    this->SetStyle(GetStyle() | WB_BORDER | WB_STDFLOATWIN | WB_3DLOOK | WB_DIALOGCONTROL | WB_SYSTEMWINDOW | WB_STANDALONE | WB_HIDE);

    FillColorValueSets_Impl();
    m_pTabBgColorSet->SetDoubleClickHdl( HDL(TabBgColorDblClickHdl_Impl) );
    m_pBtnOk->SetClickHdl( HDL(TabBgColorOKHdl_Impl) );
}

ScTabBgColorDlg::~ScTabBgColorDlg()
{
    disposeOnce();
}

void ScTabBgColorDlg::dispose()
{
    m_pTabBgColorSet.clear();
    m_pBtnOk.clear();
    ModalDialog::dispose();
}

void ScTabBgColorDlg::GetSelectedColor( Color& rColor ) const
{
    rColor = this->m_aTabBgColor;
}

void ScTabBgColorDlg::FillColorValueSets_Impl()
{
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    const SfxPoolItem* pItem = NULL;
    XColorListRef pColorList;

    sal_uInt16 nSelectedItem = 0;

    OSL_ENSURE( pDocSh, "DocShell not found!" );

    if ( pDocSh && ( 0 != ( pItem = pDocSh->GetItem(SID_COLOR_TABLE) ) ) )
        pColorList = static_cast<const SvxColorListItem*>(pItem)->GetColorList();
    if ( !pColorList.is() )
        pColorList = XColorList::CreateStdColorList();

    long nColorCount(0);

    if ( pColorList.is() )
    {
        nColorCount = pColorList->Count();
        m_pTabBgColorSet->addEntriesForXColorList(*pColorList);
    }

    if (nColorCount)
    {
        const WinBits nBits(m_pTabBgColorSet->GetStyle() | WB_NAMEFIELD | WB_ITEMBORDER | WB_NONEFIELD | WB_3DLOOK | WB_NO_DIRECTSELECT | WB_NOPOINTERFOCUS);
        m_pTabBgColorSet->SetText( m_aTabBgColorNoColorText );
        m_pTabBgColorSet->SetStyle( nBits );
    }

    //lock down a preferred size
    const sal_uInt32 nColCount = SvxColorValueSet::getColumnCount();
    const sal_uInt32 nRowCount(ceil(double(nColorCount)/nColCount));
    const sal_uInt32 nLength = SvxColorValueSet::getEntryEdgeLength();
    Size aSize(m_pTabBgColorSet->CalcWindowSizePixel(Size(nLength, nLength), nColCount, nRowCount));
    m_pTabBgColorSet->set_width_request(aSize.Width()+8);
    m_pTabBgColorSet->set_height_request(aSize.Height()+8);

    m_pTabBgColorSet->SelectItem(nSelectedItem);
}

///    Handler, called when color selection is changed
IMPL_LINK_NOARG_TYPED(ScTabBgColorDlg, TabBgColorDblClickHdl_Impl, ValueSet*, void)
{
    sal_uInt16 nItemId = m_pTabBgColorSet->GetSelectItemId();
    Color aColor = nItemId ? ( m_pTabBgColorSet->GetItemColor( nItemId ) ) : Color( COL_AUTO );
    m_aTabBgColor = aColor;
    EndDialog( RET_OK );
}

//    Handler, called when the OK button is pushed
IMPL_LINK_NOARG_TYPED(ScTabBgColorDlg, TabBgColorOKHdl_Impl, Button*, void)
{
    sal_uInt16 nItemId = m_pTabBgColorSet->GetSelectItemId();
    Color aColor = nItemId ? ( m_pTabBgColorSet->GetItemColor( nItemId ) ) : Color( COL_AUTO );
    m_aTabBgColor = aColor;
    EndDialog( RET_OK );
}

ScTabBgColorDlg::ScTabBgColorValueSet::ScTabBgColorValueSet(vcl::Window* pParent, WinBits nStyle)
    : SvxColorValueSet(pParent, nStyle)
    , m_pTabBgColorDlg(NULL)
{
}

ScTabBgColorDlg::ScTabBgColorValueSet::~ScTabBgColorValueSet()
{
    disposeOnce();
}

void ScTabBgColorDlg::ScTabBgColorValueSet::dispose()
{
    m_pTabBgColorDlg.clear();
    SvxColorValueSet::dispose();
}

VCL_BUILDER_DECL_FACTORY(ScTabBgColorValueSet)
{
    WinBits nWinBits = WB_TABSTOP;

    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
       nWinBits |= WB_BORDER;

    rRet = VclPtr<ScTabBgColorDlg::ScTabBgColorValueSet>::Create(pParent, nWinBits);
}

void ScTabBgColorDlg::ScTabBgColorValueSet::KeyInput( const KeyEvent& rKEvt )
{
    switch ( rKEvt.GetKeyCode().GetCode() )
    {
        case KEY_SPACE:
        case KEY_RETURN:
        {
            sal_uInt16 nItemId = GetSelectItemId();
            const Color& aColor = nItemId ? ( GetItemColor( nItemId ) ) : Color( COL_AUTO );
            m_pTabBgColorDlg->m_aTabBgColor = aColor;
            m_pTabBgColorDlg->EndDialog(RET_OK);
        }
        break;
    }
    SvxColorValueSet::KeyInput(rKEvt);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
