/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_formula.hxx"



//----------------------------------------------------------------------------

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

SvLBoxEntry* StructListBox::InsertStaticEntry(
        const XubString& rText,
        const Image& rEntryImg,
        SvLBoxEntry* pParent, sal_uLong nPos, IFormulaToken* pToken )
{
    SvLBoxEntry* pEntry = InsertEntry( rText, rEntryImg, rEntryImg, pParent, sal_False, nPos, pToken );
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
    //
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

SvLBoxEntry* StructPage::InsertEntry( const XubString& rText, SvLBoxEntry* pParent,
                                       sal_uInt16 nFlag,sal_uLong nPos,IFormulaToken* pIFormulaToken)
{
    aTlbStruct.SetActiveFlag( sal_False );

    SvLBoxEntry* pEntry = NULL;
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

String StructPage::GetEntryText(SvLBoxEntry* pEntry) const
{
    String aString;
    if(pEntry!=NULL)
        aString=aTlbStruct.GetEntryText(pEntry);
    return  aString;
}

SvLBoxEntry* StructPage::GetParent(SvLBoxEntry* pEntry) const
{
    return aTlbStruct.GetParent(pEntry);
}
IFormulaToken* StructPage::GetFunctionEntry(SvLBoxEntry* pEntry)
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
            SvLBoxEntry*    pCurEntry=aTlbStruct.GetCurEntry();
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

IFormulaToken* StructPage::GetSelectedToken()
{
    return pSelectedToken;
}

String StructPage::GetSelectedEntryText()
{
    return aTlbStruct.GetEntryText(aTlbStruct.GetCurEntry());
}

} // formula

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
