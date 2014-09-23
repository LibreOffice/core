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
#include "svtools/treelistentry.hxx"

#include "structpg.hxx"
#include "formdlgs.hrc"
#include "formula/formdata.hxx"
#include "formula/formula.hxx"
#include "ModuleHelper.hxx"
#include "formula/IFunctionDescription.hxx"
#include "ForResId.hrc"


namespace formula
{
StructListBox::StructListBox(vcl::Window* pParent, WinBits nBits ):
    SvTreeListBox(pParent, nBits)
{
    bActiveFlag=false;

    vcl::Font aFont( GetFont() );
    Size aSize = aFont.GetSize();
    aSize.Height() -= 2;
    aFont.SetSize( aSize );
    SetFont( aFont );
}

SvTreeListEntry* StructListBox::InsertStaticEntry(
        const OUString& rText,
        const Image& rEntryImg,
        SvTreeListEntry* pParent, sal_uLong nPos, IFormulaToken* pToken )
{
    SvTreeListEntry* pEntry = InsertEntry( rText, rEntryImg, rEntryImg, pParent, false, nPos, pToken );
    return pEntry;
}

void StructListBox::SetActiveFlag(bool bFlag)
{
    bActiveFlag=bFlag;
}


void StructListBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    bActiveFlag=true;
    SvTreeListBox::MouseButtonDown(rMEvt);
}

void StructListBox::GetFocus()
{
    bActiveFlag=true;
    SvTreeListBox::GetFocus();
}

void StructListBox::LoseFocus()
{
    bActiveFlag=false;
    SvTreeListBox::LoseFocus();
}

extern "C" SAL_DLLPUBLIC_EXPORT vcl::Window* SAL_CALL makeStructListBox(vcl::Window *pParent, VclBuilder::stringmap &)
{
    return new StructListBox(pParent, WB_BORDER);
}

StructPage::StructPage(vcl::Window* pParent):
    TabPage(pParent, "StructPage", "formula/ui/structpage.ui"),
    maImgEnd        ( ModuleRes( BMP_STR_END ) ),
    maImgError      ( ModuleRes( BMP_STR_ERROR ) ),
    pSelectedToken  ( NULL )
{
    get(m_pTlbStruct, "struct");
    Size aSize(LogicToPixel(Size(86, 162), MAP_APPFONT));
    m_pTlbStruct->set_height_request(aSize.Height());
    m_pTlbStruct->set_width_request(aSize.Width());
    m_pTlbStruct->SetStyle(m_pTlbStruct->GetStyle()|WB_HASLINES|WB_CLIPCHILDREN|
                        WB_HASBUTTONS|WB_HSCROLL|WB_NOINITIALSELECTION);

    m_pTlbStruct->SetNodeDefaultImages();
    m_pTlbStruct->SetDefaultExpandedEntryBmp(  Image( ModuleRes( BMP_STR_OPEN  ) ) );
    m_pTlbStruct->SetDefaultCollapsedEntryBmp( Image( ModuleRes( BMP_STR_CLOSE ) ) );


    m_pTlbStruct->SetSelectHdl(LINK( this, StructPage, SelectHdl ) );
}

void StructPage::ClearStruct()
{
    m_pTlbStruct->SetActiveFlag(false);
    m_pTlbStruct->Clear();
}

SvTreeListEntry* StructPage::InsertEntry( const OUString& rText, SvTreeListEntry* pParent,
                                       sal_uInt16 nFlag,sal_uLong nPos,IFormulaToken* pIFormulaToken)
{
    m_pTlbStruct->SetActiveFlag( false );

    SvTreeListEntry* pEntry = NULL;
    switch( nFlag )
    {
        case STRUCT_FOLDER:
            pEntry = m_pTlbStruct->InsertEntry( rText, pParent, false, nPos, pIFormulaToken );
        break;
        case STRUCT_END:
            pEntry = m_pTlbStruct->InsertStaticEntry( rText, maImgEnd, pParent, nPos, pIFormulaToken );
        break;
        case STRUCT_ERROR:
            pEntry = m_pTlbStruct->InsertStaticEntry( rText, maImgError, pParent, nPos, pIFormulaToken );
        break;
    }

    if( pEntry && pParent )
        m_pTlbStruct->Expand( pParent );
    return pEntry;
}

OUString StructPage::GetEntryText(SvTreeListEntry* pEntry) const
{
    OUString aString;
    if(pEntry!=NULL)
        aString = m_pTlbStruct->GetEntryText(pEntry);
    return  aString;
}

SvTreeListEntry* StructPage::GetParent(SvTreeListEntry* pEntry) const
{
    return m_pTlbStruct->GetParent(pEntry);
}
IFormulaToken* StructPage::GetFunctionEntry(SvTreeListEntry* pEntry)
{
    if(pEntry!=NULL)
    {
        IFormulaToken * pToken=(IFormulaToken *)pEntry->GetUserData();
        if(pToken!=NULL)
        {
            if ( !(pToken->isFunction() || pToken->getArgumentCount() > 1 ) )
            {
                return GetFunctionEntry(m_pTlbStruct->GetParent(pEntry));
            }
            else
            {
                return pToken;
            }
        }
    }
    return NULL;
}

IMPL_LINK( StructPage, SelectHdl, SvTreeListBox*, pTlb )
{
    if(m_pTlbStruct->GetActiveFlag())
    {
        if(pTlb==m_pTlbStruct)
        {
            SvTreeListEntry*    pCurEntry=m_pTlbStruct->GetCurEntry();
            if(pCurEntry!=NULL)
            {
                pSelectedToken=(IFormulaToken *)pCurEntry->GetUserData();
                if(pSelectedToken!=NULL)
                {
                    if ( !(pSelectedToken->isFunction() || pSelectedToken->getArgumentCount() > 1) )
                    {
                        pSelectedToken = GetFunctionEntry(pCurEntry);
                    }
                }
            }
        }

        aSelLink.Call(this);
    }
    return 0;
}

} // formula

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
