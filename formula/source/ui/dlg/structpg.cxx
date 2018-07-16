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

#include <svl/zforlist.hxx>
#include <svl/stritem.hxx>
#include <vcl/builderfactory.hxx>
#include <svtools/treelistentry.hxx>

#include "structpg.hxx"
#include <formula/formdata.hxx>
#include <formula/formula.hxx>
#include <formula/IFunctionDescription.hxx>
#include <formula/token.hxx>
#include <bitmaps.hlst>

namespace formula
{
StructListBox::StructListBox(vcl::Window* pParent, WinBits nBits ):
    SvTreeListBox(pParent, nBits)
{
    bActiveFlag = false;

    vcl::Font aFont( GetFont() );
    Size aSize = aFont.GetFontSize();
    aSize.AdjustHeight(-2);
    aFont.SetFontSize( aSize );
    SetFont( aFont );
}

SvTreeListEntry* StructListBox::InsertStaticEntry(
        const OUString& rText,
        const Image& rEntryImg,
        SvTreeListEntry* pParent, sal_uLong nPos, const FormulaToken* pToken )
{
    SvTreeListEntry* pEntry = InsertEntry( rText, rEntryImg, rEntryImg, pParent, false, nPos,
            const_cast<FormulaToken*>(pToken) );
    return pEntry;
}

void StructListBox::SetActiveFlag(bool bFlag)
{
    bActiveFlag = bFlag;
}


void StructListBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    bActiveFlag = true;
    SvTreeListBox::MouseButtonDown(rMEvt);
}

void StructListBox::GetFocus()
{
    bActiveFlag = true;
    SvTreeListBox::GetFocus();
}

void StructListBox::LoseFocus()
{
    bActiveFlag = false;
    SvTreeListBox::LoseFocus();
}

VCL_BUILDER_FACTORY_ARGS(StructListBox, WB_BORDER)

StructPage::StructPage(vcl::Window* pParent):
    TabPage(pParent, "StructPage", "formula/ui/structpage.ui"),
    maImgEnd(BitmapEx(BMP_STR_END)),
    maImgError(BitmapEx(BMP_STR_ERROR)),
    pSelectedToken  ( nullptr )
{
    get(m_pTlbStruct, "struct");
    Size aSize(LogicToPixel(Size(86, 162), MapMode(MapUnit::MapAppFont)));
    m_pTlbStruct->set_height_request(aSize.Height());
    m_pTlbStruct->set_width_request(aSize.Width());
    m_pTlbStruct->SetStyle(m_pTlbStruct->GetStyle()|WB_HASLINES|WB_CLIPCHILDREN|
                        WB_HASBUTTONS|WB_HSCROLL|WB_NOINITIALSELECTION);

    m_pTlbStruct->SetNodeDefaultImages();
    m_pTlbStruct->SetDefaultExpandedEntryBmp(Image(BitmapEx(BMP_STR_OPEN)));
    m_pTlbStruct->SetDefaultCollapsedEntryBmp(Image(BitmapEx(BMP_STR_CLOSE)));


    m_pTlbStruct->SetSelectHdl(LINK( this, StructPage, SelectHdl ) );
}

StructPage::~StructPage()
{
    disposeOnce();
}

void StructPage::dispose()
{
    m_pTlbStruct.clear();
    TabPage::dispose();
}

void StructPage::ClearStruct()
{
    m_pTlbStruct->SetActiveFlag(false);
    m_pTlbStruct->Clear();
}

SvTreeListEntry* StructPage::InsertEntry( const OUString& rText, SvTreeListEntry* pParent,
                                       sal_uInt16 nFlag, sal_uLong nPos, const FormulaToken* pIFormulaToken )
{
    m_pTlbStruct->SetActiveFlag( false );

    SvTreeListEntry* pEntry = nullptr;
    switch( nFlag )
    {
        case STRUCT_FOLDER:
            pEntry = m_pTlbStruct->InsertEntry( rText, pParent, false, nPos,
                    const_cast<FormulaToken*>(pIFormulaToken));
        break;
        case STRUCT_END:
            pEntry = m_pTlbStruct->InsertStaticEntry( rText, maImgEnd, pParent, nPos, pIFormulaToken );
        break;
        case STRUCT_ERROR:
            pEntry = m_pTlbStruct->InsertStaticEntry( rText, maImgError, pParent, nPos, pIFormulaToken );
        break;
    }

    if ( pEntry && pParent )
        m_pTlbStruct->Expand( pParent );
    return pEntry;
}

OUString StructPage::GetEntryText(SvTreeListEntry* pEntry) const
{
    OUString aString;
    if (pEntry != nullptr)
        aString = m_pTlbStruct->GetEntryText(pEntry);
    return  aString;
}

const FormulaToken* StructPage::GetFunctionEntry(SvTreeListEntry* pEntry)
{
    if (pEntry != nullptr)
    {
        const FormulaToken * pToken = static_cast<const FormulaToken *>(pEntry->GetUserData());
        if (pToken != nullptr)
        {
            if ( !(pToken->IsFunction() || pToken->GetParamCount() > 1 ) )
            {
                return GetFunctionEntry(m_pTlbStruct->GetParent(pEntry));
            }
            else
            {
                return pToken;
            }
        }
    }
    return nullptr;
}

IMPL_LINK( StructPage, SelectHdl, SvTreeListBox*, pTlb, void )
{
    if (m_pTlbStruct->GetActiveFlag())
    {
        if (pTlb == m_pTlbStruct)
        {
            SvTreeListEntry*    pCurEntry = m_pTlbStruct->GetCurEntry();
            if (pCurEntry != nullptr)
            {
                pSelectedToken = static_cast<const FormulaToken *>(pCurEntry->GetUserData());
                if (pSelectedToken != nullptr)
                {
                    if ( !(pSelectedToken->IsFunction() || pSelectedToken->GetParamCount() > 1) )
                    {
                        pSelectedToken = GetFunctionEntry(pCurEntry);
                    }
                }
            }
        }

        aSelLink.Call(*this);
    }
}

} // formula

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
