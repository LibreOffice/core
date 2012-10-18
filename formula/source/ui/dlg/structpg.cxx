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

#include "structpg.hxx"
#include "formdlgs.hrc"
#include "formula/formdata.hxx"
#include "formula/formula.hxx"
#include "ModuleHelper.hxx"
#include "formula/IFunctionDescription.hxx"
#include "ForResId.hrc"

//----------------------------------------------------------------------------
namespace formula
{
StructListBox::StructListBox(Window* pParent, const ResId& rResId ):
    SvTreeListBox(pParent,rResId )
{
    bActiveFlag=sal_False;

    Font aFont( GetFont() );
    Size aSize = aFont.GetSize();
    aSize.Height() -= 2;
    aFont.SetSize( aSize );
    SetFont( aFont );
}

SvTreeListEntry* StructListBox::InsertStaticEntry(
        const XubString& rText,
        const Image& rEntryImg,
        SvTreeListEntry* pParent, sal_uLong nPos, IFormulaToken* pToken )
{
    SvTreeListEntry* pEntry = InsertEntry( rText, rEntryImg, rEntryImg, pParent, sal_False, nPos, pToken );
    return pEntry;
}

void StructListBox::SetActiveFlag(sal_Bool bFlag)
{
    bActiveFlag=bFlag;
}

sal_Bool StructListBox::GetActiveFlag()
{
    return bActiveFlag;
}

void StructListBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    bActiveFlag=sal_True;
    SvTreeListBox::MouseButtonDown(rMEvt);
}

void StructListBox::GetFocus()
{
    bActiveFlag=sal_True;
    SvTreeListBox::GetFocus();
}

void StructListBox::LoseFocus()
{
    bActiveFlag=sal_False;
    SvTreeListBox::LoseFocus();
}

//==============================================================================

StructPage::StructPage(Window* pParent):
    TabPage(pParent,ModuleRes(RID_FORMULATAB_STRUCT)),
    aFtStruct       ( this, ModuleRes( FT_STRUCT ) ),
    aTlbStruct      ( this, ModuleRes( TLB_STRUCT ) ),
    maImgEnd        ( ModuleRes( BMP_STR_END ) ),
    maImgError      ( ModuleRes( BMP_STR_ERROR ) ),
    pSelectedToken  ( NULL )
{
    aTlbStruct.SetStyle(aTlbStruct.GetStyle()|WB_HASLINES|WB_CLIPCHILDREN|
                        WB_HASBUTTONS|WB_HSCROLL|WB_NOINITIALSELECTION);

    aTlbStruct.SetNodeDefaultImages();
    aTlbStruct.SetDefaultExpandedEntryBmp(  Image( ModuleRes( BMP_STR_OPEN  ) ) );
    aTlbStruct.SetDefaultCollapsedEntryBmp( Image( ModuleRes( BMP_STR_CLOSE ) ) );

    FreeResource();

    aTlbStruct.SetSelectHdl(LINK( this, StructPage, SelectHdl ) );
}

void StructPage::ClearStruct()
{
    aTlbStruct.SetActiveFlag(sal_False);
    aTlbStruct.Clear();
}

SvTreeListEntry* StructPage::InsertEntry( const XubString& rText, SvTreeListEntry* pParent,
                                       sal_uInt16 nFlag,sal_uLong nPos,IFormulaToken* pIFormulaToken)
{
    aTlbStruct.SetActiveFlag( sal_False );

    SvTreeListEntry* pEntry = NULL;
    switch( nFlag )
    {
        case STRUCT_FOLDER:
            pEntry = aTlbStruct.InsertEntry( rText, pParent, sal_False, nPos, pIFormulaToken );
        break;
        case STRUCT_END:
            pEntry = aTlbStruct.InsertStaticEntry( rText, maImgEnd, pParent, nPos, pIFormulaToken );
        break;
        case STRUCT_ERROR:
            pEntry = aTlbStruct.InsertStaticEntry( rText, maImgError, pParent, nPos, pIFormulaToken );
        break;
    }

    if( pEntry && pParent )
        aTlbStruct.Expand( pParent );
    return pEntry;
}

String StructPage::GetEntryText(SvTreeListEntry* pEntry) const
{
    String aString;
    if(pEntry!=NULL)
        aString=aTlbStruct.GetEntryText(pEntry);
    return  aString;
}

SvTreeListEntry* StructPage::GetParent(SvTreeListEntry* pEntry) const
{
    return aTlbStruct.GetParent(pEntry);
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
                return GetFunctionEntry(aTlbStruct.GetParent(pEntry));
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
    if(aTlbStruct.GetActiveFlag())
    {
        if(pTlb==&aTlbStruct)
        {
            SvTreeListEntry*    pCurEntry=aTlbStruct.GetCurEntry();
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
