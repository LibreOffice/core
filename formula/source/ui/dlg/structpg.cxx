/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
        const Image& rEntryImg, const Image& rEntryImgHC,
        SvLBoxEntry* pParent, sal_uLong nPos, IFormulaToken* pToken )
{
    SvLBoxEntry* pEntry = InsertEntry( rText, rEntryImg, rEntryImg, pParent, sal_False, nPos, pToken );
    SvLBoxContextBmp* pBmpItem = static_cast< SvLBoxContextBmp* >( pEntry->GetFirstItem( SV_ITEM_ID_LBOXCONTEXTBMP ) );
    DBG_ASSERT( pBmpItem, "StructListBox::InsertStaticEntry - missing item" );
    pBmpItem->SetBitmap1( rEntryImgHC, BMP_COLOR_HIGHCONTRAST );
    pBmpItem->SetBitmap2( rEntryImgHC, BMP_COLOR_HIGHCONTRAST );
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
    maImgEndHC      ( ModuleRes( BMP_STR_END_H ) ),
    maImgErrorHC    ( ModuleRes( BMP_STR_ERROR_H ) ),
    pSelectedToken  ( NULL )
{
    aTlbStruct.SetStyle(aTlbStruct.GetStyle()|WB_HASLINES|WB_CLIPCHILDREN|
                        WB_HASBUTTONS|WB_HSCROLL|WB_NOINITIALSELECTION);

    aTlbStruct.SetNodeDefaultImages();
    aTlbStruct.SetDefaultExpandedEntryBmp( Image( ModuleRes( BMP_STR_OPEN ) ) );
    aTlbStruct.SetDefaultCollapsedEntryBmp( Image( ModuleRes( BMP_STR_CLOSE ) ) );
    aTlbStruct.SetDefaultExpandedEntryBmp( Image( ModuleRes( BMP_STR_OPEN_H ) ), BMP_COLOR_HIGHCONTRAST );
    aTlbStruct.SetDefaultCollapsedEntryBmp( Image( ModuleRes( BMP_STR_CLOSE_H ) ), BMP_COLOR_HIGHCONTRAST );

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
            pEntry = aTlbStruct.InsertStaticEntry( rText, maImgEnd, maImgEndHC, pParent, nPos, pIFormulaToken );
        break;
        case STRUCT_ERROR:
            pEntry = aTlbStruct.InsertStaticEntry( rText, maImgError, maImgErrorHC, pParent, nPos, pIFormulaToken );
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

