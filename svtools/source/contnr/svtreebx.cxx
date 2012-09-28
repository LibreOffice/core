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

#define _SVTREEBX_CXX
#include <vcl/svapp.hxx>

#include <svtools/svlbox.hxx>
#include <svtools/svlbitm.hxx>
#include <svtools/svtreebx.hxx>
#include <comphelper/string.hxx>
#include <svimpbox.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>


using namespace ::com::sun::star::accessibility;

/*
    Bugs/TODO

    - calculate rectangle when editing in-place (bug with some fonts)
    - SetSpaceBetweenEntries: offset is not taken into account in SetEntryHeight
*/

#define TREEFLAG_FIXEDHEIGHT        0x0010


DBG_NAME(SvTreeListBox)

#define SV_LBOX_DEFAULT_INDENT_PIXEL 20

SvTreeListBox::SvTreeListBox( Window* pParent, WinBits nWinStyle )
    : SvLBox( pParent, nWinStyle )
{
    DBG_CTOR(SvTreeListBox,0);
    InitTreeView();

    SetSublistOpenWithLeftRight();
}

SvTreeListBox::SvTreeListBox( Window* pParent , const ResId& rResId )
    : SvLBox( pParent,rResId )
{
    DBG_CTOR(SvTreeListBox,0);

    InitTreeView();
    Resize();

    SetSublistOpenWithLeftRight();
}

void SvTreeListBox::InitTreeView()
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pCheckButtonData = NULL;
    pEdEntry = NULL;
    pEdItem = NULL;
    nEntryHeight = 0;
    pEdCtrl = NULL;
    nFirstSelTab = 0;
    nLastSelTab = 0;
    nFocusWidth = -1;
    mnCheckboxItemWidth = 0;

    Link* pLink = new Link( LINK(this,SvTreeListBox, DefaultCompare) );
    pLBoxImpl->m_pLink = pLink;

    nTreeFlags = TREEFLAG_RECALCTABS;
    nIndent = SV_LBOX_DEFAULT_INDENT_PIXEL;
    nEntryHeightOffs = SV_ENTRYHEIGHTOFFS_PIXEL;
    pImp = new SvImpLBox( this, GetModel(), GetStyle() );

    aContextBmpMode = SVLISTENTRYFLAG_EXPANDED;
    nContextBmpWidthMax = 0;
    SetFont( GetFont() );
    SetSpaceBetweenEntries( 0 );
    SetLineColor();
    InitSettings( sal_True, sal_True, sal_True );
    ImplInitStyle();
    SetTabs();
}


SvTreeListBox::~SvTreeListBox()
{
    DBG_DTOR(SvTreeListBox,0);
    pImp->CallEventListeners( VCLEVENT_OBJECT_DYING );
    delete pImp;
    delete pLBoxImpl->m_pLink;
    ClearTabList();
}

void SvTreeListBox::SetExtendedWinBits( ExtendedWinBits _nBits )
{
    pImp->SetExtendedWindowBits( _nBits );
}

void SvTreeListBox::SetModel( SvLBoxTreeList* pNewModel )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->SetModel( pNewModel );
    SvLBox::SetModel( pNewModel );
}

void SvTreeListBox::DisconnectFromModel()
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SvLBox::DisconnectFromModel();
    pImp->SetModel( GetModel() );
}


sal_uInt16 SvTreeListBox::IsA()
{
    DBG_CHKTHIS(SvTreeListBox,0);
    return SV_LISTBOX_ID_TREEBOX;
}

void SvTreeListBox::SetSublistOpenWithReturn( sal_Bool b )
{
    pImp->bSubLstOpRet = b;
}

void SvTreeListBox::SetSublistOpenWithLeftRight( sal_Bool b )
{
    pImp->bSubLstOpLR = b;
}

void SvTreeListBox::Resize()
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if( IsEditingActive() )
        EndEditing( sal_True );
    SvLBox::Resize();
    pImp->Resize();
    nFocusWidth = -1;
    pImp->ShowCursor( sal_False );
    pImp->ShowCursor( sal_True );
}

/* Cases:

   A) entries have bitmaps
       0. no buttons
       1. node buttons (can optionally also be on root items)
       2. node buttons (can optionally also be on root items) + CheckButton
       3. CheckButton
   B) entries don't have bitmaps  (=>via WindowBits because of D&D!)
       0. no buttons
       1. node buttons (can optionally also be on root items)
       2. node buttons (can optionally also be on root items) + CheckButton
       3. CheckButton
*/

#define NO_BUTTONS              0
#define NODE_BUTTONS            1
#define NODE_AND_CHECK_BUTTONS  2
#define CHECK_BUTTONS           3

#define TABFLAGS_TEXT (SV_LBOXTAB_DYNAMIC |        \
                       SV_LBOXTAB_ADJUST_LEFT |    \
                       SV_LBOXTAB_EDITABLE |       \
                       SV_LBOXTAB_SHOW_SELECTION)

#define TABFLAGS_CONTEXTBMP (SV_LBOXTAB_DYNAMIC | SV_LBOXTAB_ADJUST_CENTER)

#define TABFLAGS_CHECKBTN (SV_LBOXTAB_DYNAMIC |        \
                           SV_LBOXTAB_ADJUST_CENTER |  \
                           SV_LBOXTAB_PUSHABLE)

#define TAB_STARTPOS    2

// take care of GetTextOffset when doing changes
void SvTreeListBox::SetTabs()
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if( IsEditingActive() )
        EndEditing( sal_True );
    nTreeFlags &= (~TREEFLAG_RECALCTABS);
    nFocusWidth = -1;
    const WinBits nStyle( GetStyle() );
    sal_Bool bHasButtons = (nStyle & WB_HASBUTTONS)!=0;
    sal_Bool bHasButtonsAtRoot = (nStyle & (WB_HASLINESATROOT |
                                              WB_HASBUTTONSATROOT))!=0;
    long nStartPos = TAB_STARTPOS;
    long nNodeWidthPixel = GetExpandedNodeBmp().GetSizePixel().Width();

    // pCheckButtonData->Width() knows nothing about the native checkbox width,
    // so we have mnCheckboxItemWidth which becomes valid when something is added.
    long nCheckWidth = 0;
    if( nTreeFlags & TREEFLAG_CHKBTN )
        nCheckWidth = mnCheckboxItemWidth;
    long nCheckWidthDIV2 = nCheckWidth / 2;

    long nContextWidth = nContextBmpWidthMax;
    long nContextWidthDIV2 = nContextWidth / 2;

    ClearTabList();

    int nCase = NO_BUTTONS;
    if( !(nTreeFlags & TREEFLAG_CHKBTN) )
    {
        if( bHasButtons )
            nCase = NODE_BUTTONS;
    }
    else
    {
        if( bHasButtons )
            nCase = NODE_AND_CHECK_BUTTONS;
         else
            nCase = CHECK_BUTTONS;
    }

    switch( nCase )
    {
        case NO_BUTTONS :
            nStartPos += nContextWidthDIV2;  // because of centering
            AddTab( nStartPos, TABFLAGS_CONTEXTBMP );
            nStartPos += nContextWidthDIV2;  // right edge of context bitmap
            // only set a distance if there are bitmaps
            if( nContextBmpWidthMax )
                nStartPos += 5; // distance context bitmap to text
            AddTab( nStartPos, TABFLAGS_TEXT );
            break;

        case NODE_BUTTONS :
            if( bHasButtonsAtRoot )
                nStartPos += ( nIndent + (nNodeWidthPixel/2) );
            else
                nStartPos += nContextWidthDIV2;
            AddTab( nStartPos, TABFLAGS_CONTEXTBMP );
            nStartPos += nContextWidthDIV2;  // right edge of context bitmap
            // only set a distance if there are bitmaps
            if( nContextBmpWidthMax )
                nStartPos += 5; // distance context bitmap to text
            AddTab( nStartPos, TABFLAGS_TEXT );
            break;

        case NODE_AND_CHECK_BUTTONS :
            if( bHasButtonsAtRoot )
                nStartPos += ( nIndent + nNodeWidthPixel );
            else
                nStartPos += nCheckWidthDIV2;
            AddTab( nStartPos, TABFLAGS_CHECKBTN );
            nStartPos += nCheckWidthDIV2;  // right edge of CheckButton
            nStartPos += 3;  // distance CheckButton to context bitmap
            nStartPos += nContextWidthDIV2;  // center of context bitmap
            AddTab( nStartPos, TABFLAGS_CONTEXTBMP );
            nStartPos += nContextWidthDIV2;  // right edge of context bitmap
            // only set a distance if there are bitmaps
            if( nContextBmpWidthMax )
                nStartPos += 5; // distance context bitmap to text
            AddTab( nStartPos, TABFLAGS_TEXT );
            break;

        case CHECK_BUTTONS :
            nStartPos += nCheckWidthDIV2;
            AddTab( nStartPos, TABFLAGS_CHECKBTN );
            nStartPos += nCheckWidthDIV2;  // right edge of CheckButton
            nStartPos += 3;  // distance CheckButton to context bitmap
            nStartPos += nContextWidthDIV2;  // center of context bitmap
            AddTab( nStartPos, TABFLAGS_CONTEXTBMP );
            nStartPos += nContextWidthDIV2;  // right edge of context bitmap
            // only set a distance if there are bitmaps
            if( nContextBmpWidthMax )
                nStartPos += 5; // distance context bitmap to text
            AddTab( nStartPos, TABFLAGS_TEXT );
            break;
    }
    pImp->NotifyTabsChanged();
}

void SvTreeListBox::InitEntry( SvLBoxEntry* pEntry,
  const XubString& aStr, const Image& aCollEntryBmp, const Image& aExpEntryBmp,
  SvLBoxButtonKind eButtonKind)
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SvLBoxButton* pButton;
    SvLBoxString* pString;
    SvLBoxContextBmp* pContextBmp;

    if( nTreeFlags & TREEFLAG_CHKBTN )
    {
        pButton= new SvLBoxButton( pEntry,eButtonKind,0,pCheckButtonData );
        pEntry->AddItem( pButton );
    }

    pContextBmp= new SvLBoxContextBmp( pEntry,0, aCollEntryBmp,aExpEntryBmp,
                                     aContextBmpMode );
    pEntry->AddItem( pContextBmp );

    pString = new SvLBoxString( pEntry, 0, aStr );
    pEntry->AddItem( pString );
}

String SvTreeListBox::GetEntryText(SvLBoxEntry* pEntry) const
{
    DBG_CHKTHIS(SvTreeListBox,0);
    DBG_ASSERT( pEntry, "SvTreeListBox::GetEntryText(): no entry" );
    SvLBoxString* pItem = (SvLBoxString*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
    DBG_ASSERT( pEntry, "SvTreeListBox::GetEntryText(): item not found" );
    return pItem->GetText();
}

String SvTreeListBox::SearchEntryText( SvLBoxEntry* pEntry ) const
{
    DBG_CHKTHIS(SvTreeListBox,0);
    DBG_ASSERT( pEntry, "SvTreeListBox::SearchEntryText(): no entry" );
    String sRet;
    sal_uInt16 nCount = pEntry->ItemCount();
    sal_uInt16 nCur = 0;
    SvLBoxItem* pItem;
    while( nCur < nCount )
    {
        pItem = pEntry->GetItem( nCur );
        if ( pItem->IsA() == SV_ITEM_ID_LBOXSTRING && !static_cast<SvLBoxString*>( pItem )->GetText().isEmpty() )
        {
            sRet = static_cast<SvLBoxString*>( pItem )->GetText();
            break;
        }
        nCur++;
    }
    return sRet;
}

const Image& SvTreeListBox::GetExpandedEntryBmp(SvLBoxEntry* pEntry) const
{
    DBG_CHKTHIS(SvTreeListBox,0);
    DBG_ASSERT(pEntry,"Entry?");
    SvLBoxContextBmp* pItem = (SvLBoxContextBmp*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
    DBG_ASSERT(pItem,"GetContextBmp:Item not found");
    return pItem->GetBitmap2( );
}

const Image& SvTreeListBox::GetCollapsedEntryBmp( SvLBoxEntry* pEntry ) const
{
    DBG_CHKTHIS(SvTreeListBox,0);
    DBG_ASSERT(pEntry,"Entry?");
    SvLBoxContextBmp* pItem = (SvLBoxContextBmp*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
    DBG_ASSERT(pItem,"GetContextBmp:Item not found");
    return pItem->GetBitmap1( );
}

IMPL_LINK_INLINE_START( SvTreeListBox, CheckButtonClick, SvLBoxButtonData *, pData )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pHdlEntry = pData->GetActEntry();
    CheckButtonHdl();
    return 0;
}
IMPL_LINK_INLINE_END( SvTreeListBox, CheckButtonClick, SvLBoxButtonData *, pData )

SvLBoxEntry* SvTreeListBox::InsertEntry(
    const XubString& aText,
    SvLBoxEntry* pParent,
    sal_Bool bChildrenOnDemand, sal_uLong nPos,
    void* pUser,
    SvLBoxButtonKind eButtonKind
)
{
    DBG_CHKTHIS(SvTreeListBox,0);
    nTreeFlags |= TREEFLAG_MANINS;

    const Image& rDefExpBmp = pImp->GetDefaultEntryExpBmp( );
    const Image& rDefColBmp = pImp->GetDefaultEntryColBmp( );

    aCurInsertedExpBmp = rDefExpBmp;
    aCurInsertedColBmp = rDefColBmp;

    SvLBoxEntry* pEntry = CreateEntry();
    pEntry->SetUserData( pUser );
    InitEntry( pEntry, aText, rDefColBmp, rDefExpBmp, eButtonKind );
    pEntry->EnableChildrenOnDemand( bChildrenOnDemand );

    if( !pParent )
        SvLBox::Insert( pEntry, nPos );
    else
        SvLBox::Insert( pEntry, pParent, nPos );

    aPrevInsertedExpBmp = rDefExpBmp;
    aPrevInsertedColBmp = rDefColBmp;

    nTreeFlags &= (~TREEFLAG_MANINS);

    return pEntry;
}

SvLBoxEntry* SvTreeListBox::InsertEntry( const XubString& aText,
    const Image& aExpEntryBmp, const Image& aCollEntryBmp,
    SvLBoxEntry* pParent, sal_Bool bChildrenOnDemand, sal_uLong nPos, void* pUser,
    SvLBoxButtonKind eButtonKind )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    nTreeFlags |= TREEFLAG_MANINS;

    aCurInsertedExpBmp = aExpEntryBmp;
    aCurInsertedColBmp = aCollEntryBmp;

    SvLBoxEntry* pEntry = CreateEntry();
    pEntry->SetUserData( pUser );
    InitEntry( pEntry, aText, aCollEntryBmp, aExpEntryBmp, eButtonKind );

    pEntry->EnableChildrenOnDemand( bChildrenOnDemand );

    if( !pParent )
        SvLBox::Insert( pEntry, nPos );
    else
        SvLBox::Insert( pEntry, pParent, nPos );

    aPrevInsertedExpBmp = aExpEntryBmp;
    aPrevInsertedColBmp = aCollEntryBmp;

    nTreeFlags &= (~TREEFLAG_MANINS);

    return pEntry;
}

void SvTreeListBox::SetEntryText( SvLBoxEntry* pEntry, const XubString& aStr)
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SvLBoxString* pItem = (SvLBoxString*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
    DBG_ASSERT(pItem,"SetText:Item not found");
    pItem->SetText( aStr );
    pItem->InitViewData( this, pEntry, 0 );
    GetModel()->InvalidateEntry( pEntry );
}

void SvTreeListBox::SetExpandedEntryBmp( SvLBoxEntry* pEntry, const Image& aBmp )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SvLBoxContextBmp* pItem = (SvLBoxContextBmp*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));

    DBG_ASSERT(pItem,"SetExpBmp:Item not found");
    pItem->SetBitmap2( aBmp );

    GetModel()->InvalidateEntry( pEntry );
    SetEntryHeight( pEntry );
    Size aSize = aBmp.GetSizePixel();
    short nWidth = pImp->UpdateContextBmpWidthVector( pEntry, (short)aSize.Width() );
    if( nWidth > nContextBmpWidthMax )
    {
        nContextBmpWidthMax = nWidth;
        SetTabs();
    }
}

void SvTreeListBox::SetCollapsedEntryBmp(SvLBoxEntry* pEntry,const Image& aBmp )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SvLBoxContextBmp* pItem = (SvLBoxContextBmp*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));

    DBG_ASSERT(pItem,"SetExpBmp:Item not found");
    pItem->SetBitmap1( aBmp );

    GetModel()->InvalidateEntry( pEntry );
    SetEntryHeight( pEntry );
    Size aSize = aBmp.GetSizePixel();
    short nWidth = pImp->UpdateContextBmpWidthVector( pEntry, (short)aSize.Width() );
    if( nWidth > nContextBmpWidthMax )
    {
        nContextBmpWidthMax = nWidth;
        SetTabs();
    }
}

void SvTreeListBox::ImpEntryInserted( SvLBoxEntry* pEntry )
{
    DBG_CHKTHIS(SvTreeListBox,0);

    SvLBoxEntry* pParent = (SvLBoxEntry*)pModel->GetParent( pEntry );
    if( pParent )
    {
        sal_uInt16 nFlags = pParent->GetFlags();
        nFlags &= ~SV_ENTRYFLAG_NO_NODEBMP;
        pParent->SetFlags( nFlags );
    }

    if(!((nTreeFlags & TREEFLAG_MANINS) &&
         (aPrevInsertedExpBmp == aCurInsertedExpBmp)  &&
         (aPrevInsertedColBmp == aCurInsertedColBmp) ))
    {
        Size aSize = GetCollapsedEntryBmp( pEntry ).GetSizePixel();
        if( aSize.Width() > nContextBmpWidthMax )
        {
            nContextBmpWidthMax = (short)aSize.Width();
            nTreeFlags |= TREEFLAG_RECALCTABS;
        }
        aSize = GetExpandedEntryBmp( pEntry ).GetSizePixel();
        if( aSize.Width() > nContextBmpWidthMax )
        {
            nContextBmpWidthMax = (short)aSize.Width();
            nTreeFlags |= TREEFLAG_RECALCTABS;
        }
    }
    SetEntryHeight( (SvLBoxEntry*)pEntry );

    if( nTreeFlags & TREEFLAG_CHKBTN )
    {
        SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXBUTTON));
        if( pItem )
        {
            long nWidth = pItem->GetSize(this, pEntry).Width();
            if( mnCheckboxItemWidth < nWidth )
            {
                mnCheckboxItemWidth = nWidth;
                nTreeFlags |= TREEFLAG_RECALCTABS;
            }
        }
    }
}



void SvTreeListBox::SetCheckButtonState( SvLBoxEntry* pEntry, SvButtonState eState)
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if( nTreeFlags & TREEFLAG_CHKBTN )
    {
        SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXBUTTON));
        if(!(pItem && pItem->CheckModification()))
            return ;
        switch( eState )
        {
            case SV_BUTTON_CHECKED:
                pItem->SetStateChecked();
                break;

            case SV_BUTTON_UNCHECKED:
                pItem->SetStateUnchecked();
                break;

            case SV_BUTTON_TRISTATE:
                pItem->SetStateTristate();
                break;
        }
        InvalidateEntry( pEntry );
    }
}

SvButtonState SvTreeListBox::GetCheckButtonState( SvLBoxEntry* pEntry ) const
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SvButtonState eState = SV_BUTTON_UNCHECKED;
    if( nTreeFlags & TREEFLAG_CHKBTN )
    {
        SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXBUTTON));
        if(!pItem)
            return SV_BUTTON_TRISTATE;
        sal_uInt16 nButtonFlags = pItem->GetButtonFlags();
        eState = pCheckButtonData->ConvertToButtonState( nButtonFlags );
    }
    return eState;
}

void SvTreeListBox::CheckButtonHdl()
{
    DBG_CHKTHIS(SvTreeListBox,0);
    aCheckButtonHdl.Call( this );
    if ( pCheckButtonData )
        pImp->CallEventListeners( VCLEVENT_CHECKBOX_TOGGLE, (void*)pCheckButtonData->GetActEntry() );
}

//
// TODO: Currently all data is cloned so that they conform to the default tree
// view format. Actually, the model should be used as a reference here. This
// leads to us _not_ calling SvLBoxEntry::Clone, but only its base class
// SvListEntry.
//

SvLBoxEntry* SvTreeListBox::CloneEntry( SvLBoxEntry* pSource )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    XubString aStr;
    Image aCollEntryBmp;
    Image aExpEntryBmp;
    SvLBoxButtonKind eButtonKind = SvLBoxButtonKind_enabledCheckbox;

    SvLBoxString* pStringItem = (SvLBoxString*)(pSource->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
    if( pStringItem )
        aStr = pStringItem->GetText();
    SvLBoxContextBmp* pBmpItem = (SvLBoxContextBmp*)(pSource->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
    if( pBmpItem )
    {
        aCollEntryBmp = pBmpItem->GetBitmap1( );
        aExpEntryBmp  = pBmpItem->GetBitmap2( );
    }
    SvLBoxButton* pButtonItem = (SvLBoxButton*)(pSource->GetFirstItem(SV_ITEM_ID_LBOXBUTTON));
    if( pButtonItem )
        eButtonKind = pButtonItem->GetKind();
    SvLBoxEntry* pClone = CreateEntry();
    InitEntry( pClone, aStr, aCollEntryBmp, aExpEntryBmp, eButtonKind );
    pClone->SvListEntry::Clone( pSource );
    pClone->EnableChildrenOnDemand( pSource->HasChildrenOnDemand() );
    pClone->SetUserData( pSource->GetUserData() );

    return pClone;
}

void SvTreeListBox::SetIndent( short nNewIndent )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    nIndent = nNewIndent;
    SetTabs();
    if( IsUpdateMode() )
        Invalidate();
}

const Image& SvTreeListBox::GetDefaultExpandedEntryBmp( ) const
{
    return pImp->GetDefaultEntryExpBmp( );
}

const Image& SvTreeListBox::GetDefaultCollapsedEntryBmp( ) const
{
    return pImp->GetDefaultEntryColBmp( );
}

void SvTreeListBox::SetDefaultExpandedEntryBmp( const Image& aBmp )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    Size aSize = aBmp.GetSizePixel();
    if( aSize.Width() > nContextBmpWidthMax )
        nContextBmpWidthMax = (short)aSize.Width();
    SetTabs();

    pImp->SetDefaultEntryExpBmp( aBmp );
}

void SvTreeListBox::SetDefaultCollapsedEntryBmp( const Image& aBmp )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    Size aSize = aBmp.GetSizePixel();
    if( aSize.Width() > nContextBmpWidthMax )
        nContextBmpWidthMax = (short)aSize.Width();
    SetTabs();

    pImp->SetDefaultEntryColBmp( aBmp );
}

void SvTreeListBox::EnableCheckButton( SvLBoxButtonData* pData )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    DBG_ASSERT(!GetEntryCount(),"EnableCheckButton: Entry count != 0");
    if( !pData )
        nTreeFlags &= (~TREEFLAG_CHKBTN);
    else
    {
        SetCheckButtonData( pData );
        nTreeFlags |= TREEFLAG_CHKBTN;
        pData->SetLink( LINK(this, SvTreeListBox, CheckButtonClick));
    }

    SetTabs();
    if( IsUpdateMode() )
        Invalidate();
}

void SvTreeListBox::SetCheckButtonData( SvLBoxButtonData* pData )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if ( pData )
        pCheckButtonData = pData;
}

const Image& SvTreeListBox::GetDefaultExpandedNodeImage( )
{
    return SvImpLBox::GetDefaultExpandedNodeImage( );
}

const Image& SvTreeListBox::GetDefaultCollapsedNodeImage( )
{
    return SvImpLBox::GetDefaultCollapsedNodeImage( );
}

void SvTreeListBox::SetNodeBitmaps( const Image& rCollapsedNodeBmp, const Image& rExpandedNodeBmp )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SetExpandedNodeBmp( rExpandedNodeBmp );
    SetCollapsedNodeBmp( rCollapsedNodeBmp );
    SetTabs();
}

sal_Bool SvTreeListBox::EditingEntry( SvLBoxEntry*, Selection& )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    return sal_True;
}

sal_Bool SvTreeListBox::EditedEntry( SvLBoxEntry* /*pEntry*/,const rtl::OUString& /*rNewText*/)
{
    DBG_CHKTHIS(SvTreeListBox,0);
    return sal_True;
}

void SvTreeListBox::EnableInplaceEditing( sal_Bool bOn )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SvLBox::EnableInplaceEditing( bOn );
}

void SvTreeListBox::KeyInput( const KeyEvent& rKEvt )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    // under OS/2, we get key up/down even while editing
    if( IsEditingActive() )
        return;

    nImpFlags |= SVLBOX_IS_TRAVELSELECT;

#ifdef OVDEBUG
    sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();
    switch ( nCode )
    {
        case KEY_F1:
        {
            SvLBoxEntry* pEntry = First();
            pEntry = NextVisible( pEntry );
            SetEntryText( pEntry, "SetEntryText" );
        }
        break;
    }
#endif

    if( !pImp->KeyInput( rKEvt ) )
        SvLBox::KeyInput( rKEvt );

    nImpFlags &= ~SVLBOX_IS_TRAVELSELECT;
}

void SvTreeListBox::RequestingChildren( SvLBoxEntry* pParent )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if( !pParent->HasChildren() )
        InsertEntry( rtl::OUString("<dummy>"), pParent, sal_False, LIST_APPEND );
}

void SvTreeListBox::GetFocus()
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->GetFocus();
    SvLBox::GetFocus();

    SvLBoxEntry* pEntry = FirstSelected();
    if ( pEntry )
        pImp->CallEventListeners( VCLEVENT_LISTBOX_SELECT, pEntry );

}

void SvTreeListBox::LoseFocus()
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->LoseFocus();
    SvLBox::LoseFocus();
}

void SvTreeListBox::ModelHasCleared()
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->pCursor = 0; // else we crash in GetFocus when editing in-place
    delete pEdCtrl;
    pEdCtrl = NULL;
    pImp->Clear();
    nFocusWidth = -1;

    nContextBmpWidthMax = 0;
    SetDefaultExpandedEntryBmp( GetDefaultExpandedEntryBmp() );
    SetDefaultCollapsedEntryBmp( GetDefaultCollapsedEntryBmp() );

    if( !(nTreeFlags & TREEFLAG_FIXEDHEIGHT ))
        nEntryHeight = 0;
    AdjustEntryHeight( GetFont() );
    AdjustEntryHeight( GetDefaultExpandedEntryBmp() );
    AdjustEntryHeight( GetDefaultCollapsedEntryBmp() );

    SvLBox::ModelHasCleared();
}

void SvTreeListBox::ShowTargetEmphasis( SvLBoxEntry* pEntry, sal_Bool /* bShow  */ )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->PaintDDCursor( pEntry );
}

void SvTreeListBox::ScrollOutputArea( short nDeltaEntries )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if( !nDeltaEntries || !pImp->aVerSBar.IsVisible() )
        return;

    long nThumb = pImp->aVerSBar.GetThumbPos();
    long nMax = pImp->aVerSBar.GetRange().Max();

    NotifyBeginScroll();
    if( nDeltaEntries < 0 )
    {
        // move window up
        nDeltaEntries *= -1;
        long nVis = pImp->aVerSBar.GetVisibleSize();
        long nTemp = nThumb + nVis;
        if( nDeltaEntries > (nMax - nTemp) )
            nDeltaEntries = (short)(nMax - nTemp);
        pImp->PageDown( (sal_uInt16)nDeltaEntries );
    }
    else
    {
        if( nDeltaEntries > nThumb )
            nDeltaEntries = (short)nThumb;
        pImp->PageUp( (sal_uInt16)nDeltaEntries );
    }
    pImp->SyncVerThumb();
    NotifyEndScroll();
}

void SvTreeListBox::ScrollToAbsPos( long nPos )
{
    pImp->ScrollToAbsPos( nPos );
}

void SvTreeListBox::SetSelectionMode( SelectionMode eSelectMode )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SvLBox::SetSelectionMode( eSelectMode );
    pImp->SetSelectionMode( eSelectMode );
}

void SvTreeListBox::SetDragDropMode( DragDropMode nDDMode )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SvLBox::SetDragDropMode( nDDMode );
    pImp->SetDragDropMode( nDDMode );
}

short SvTreeListBox::GetHeightOffset(const Image& rBmp, Size& aSizeLogic )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    short nOffset = 0;
    aSizeLogic = rBmp.GetSizePixel();
    if( GetEntryHeight() > aSizeLogic.Height() )
        nOffset = ( GetEntryHeight() - (short)aSizeLogic.Height()) / 2;
    return nOffset;
}

short SvTreeListBox::GetHeightOffset(const Font& /* rFont */, Size& aSizeLogic )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    short nOffset = 0;
    aSizeLogic = Size(GetTextWidth(rtl::OUString('X')), GetTextHeight());
    if( GetEntryHeight() > aSizeLogic.Height() )
        nOffset = ( GetEntryHeight() - (short)aSizeLogic.Height()) / 2;
    return nOffset;
}

void SvTreeListBox::SetEntryHeight( SvLBoxEntry* pEntry )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    short nHeight, nHeightMax=0;
    sal_uInt16 nCount = pEntry->ItemCount();
    sal_uInt16 nCur = 0;
    SvViewDataEntry* pViewData = GetViewDataEntry( pEntry );
    while( nCur < nCount )
    {
        SvLBoxItem* pItem = pEntry->GetItem( nCur );
        nHeight = (short)(pItem->GetSize( pViewData, nCur ).Height());
        if( nHeight > nHeightMax )
            nHeightMax = nHeight;
        nCur++;
    }

    if( nHeightMax > nEntryHeight )
    {
        nEntryHeight = nHeightMax;
        SvLBox::SetFont( GetFont() );
        pImp->SetEntryHeight( nHeightMax );
    }
}

void SvTreeListBox::SetEntryHeight( short nHeight, sal_Bool bAlways )
{
    DBG_CHKTHIS(SvTreeListBox,0);

    if( bAlways || nHeight > nEntryHeight )
    {
        nEntryHeight = nHeight;
        if( nEntryHeight )
            nTreeFlags |= TREEFLAG_FIXEDHEIGHT;
        else
            nTreeFlags &= ~TREEFLAG_FIXEDHEIGHT;
        SvLBox::SetFont( GetFont() );
        pImp->SetEntryHeight( nHeight );
    }
}


void SvTreeListBox::AdjustEntryHeight( const Image& rBmp )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    Size aSize;
    GetHeightOffset( rBmp, aSize );
    if( aSize.Height() > nEntryHeight )
    {
        nEntryHeight = (short)aSize.Height() + nEntryHeightOffs;
        pImp->SetEntryHeight( nEntryHeight );
    }
}

void SvTreeListBox::AdjustEntryHeight( const Font& rFont )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    Size aSize;
    GetHeightOffset( rFont, aSize );
    if( aSize.Height()  >  nEntryHeight )
    {
        nEntryHeight = (short)aSize.Height() + nEntryHeightOffs;
        pImp->SetEntryHeight( nEntryHeight );
    }
}

sal_Bool SvTreeListBox::Expand( SvLBoxEntry* pParent )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pHdlEntry = pParent;
    sal_Bool bExpanded = sal_False;
    sal_uInt16 nFlags;

    if( pParent->HasChildrenOnDemand() )
        RequestingChildren( pParent );
    if( pParent->HasChildren() )
    {
        nImpFlags |= SVLBOX_IS_EXPANDING;
        if( ExpandingHdl() )
        {
            bExpanded = sal_True;
            SvListView::Expand( pParent );
            pImp->EntryExpanded( pParent );
            pHdlEntry = pParent;
            ExpandedHdl();
        }
        nFlags = pParent->GetFlags();
        nFlags &= ~SV_ENTRYFLAG_NO_NODEBMP;
        nFlags |= SV_ENTRYFLAG_HAD_CHILDREN;
        pParent->SetFlags( nFlags );
    }
    else
    {
        nFlags = pParent->GetFlags();
        nFlags |= SV_ENTRYFLAG_NO_NODEBMP;
        pParent->SetFlags( nFlags );
        GetModel()->InvalidateEntry( pParent ); // repaint
    }

    // #i92103#
    if ( bExpanded )
    {
        pImp->CallEventListeners( VCLEVENT_ITEM_EXPANDED, pParent );
    }

    return bExpanded;
}

sal_Bool SvTreeListBox::Collapse( SvLBoxEntry* pParent )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    nImpFlags &= ~SVLBOX_IS_EXPANDING;
    pHdlEntry = pParent;
    sal_Bool bCollapsed = sal_False;

    if( ExpandingHdl() )
    {
        bCollapsed = sal_True;
        pImp->CollapsingEntry( pParent );
        SvListView::Collapse( pParent );
        pImp->EntryCollapsed( pParent );
        pHdlEntry = pParent;
        ExpandedHdl();
    }

    // #i92103#
    if ( bCollapsed )
    {
        pImp->CallEventListeners( VCLEVENT_ITEM_COLLAPSED, pParent );
    }

    return bCollapsed;
}

sal_Bool SvTreeListBox::Select( SvLBoxEntry* pEntry, sal_Bool bSelect )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    DBG_ASSERT(pEntry,"Select: Null-Ptr");
    sal_Bool bRetVal = SvListView::Select( pEntry, bSelect );
    DBG_ASSERT(IsSelected(pEntry)==bSelect,"Select failed");
    if( bRetVal )
    {
        pImp->EntrySelected( pEntry, bSelect );
        pHdlEntry = pEntry;
        if( bSelect )
        {
            SelectHdl();
            pImp->CallEventListeners( VCLEVENT_LISTBOX_SELECT, pEntry );
        }
        else
            DeselectHdl();
    }
    return bRetVal;
}

sal_uLong SvTreeListBox::SelectChildren( SvLBoxEntry* pParent, sal_Bool bSelect )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->DestroyAnchor();
    sal_uLong nRet = 0;
    if( !pParent->HasChildren() )
        return 0;
    sal_uInt16 nRefDepth = pModel->GetDepth( pParent );
    SvLBoxEntry* pChild = FirstChild( pParent );
    do {
        nRet++;
        Select( pChild, bSelect );
        pChild = Next( pChild );
    } while( pChild && pModel->GetDepth( pChild ) > nRefDepth );
    return nRet;
}

void SvTreeListBox::SelectAll( sal_Bool bSelect, sal_Bool )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->SelAllDestrAnch(
        bSelect,
        sal_True,       // delete anchor,
        sal_True );     // even when using SINGLE_SELECTION, deselect the cursor
}

void SvTreeListBox::ModelHasInsertedTree( SvListEntry* pEntry )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    sal_uInt16 nRefDepth = pModel->GetDepth( (SvLBoxEntry*)pEntry );
    SvLBoxEntry* pTmp = (SvLBoxEntry*)pEntry;
    do
    {
        ImpEntryInserted( pTmp );
        pTmp = Next( pTmp );
    } while( pTmp && nRefDepth < pModel->GetDepth( pTmp ) );
    pImp->TreeInserted( (SvLBoxEntry*)pEntry );
}

void SvTreeListBox::ModelHasInserted( SvListEntry* pEntry )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    ImpEntryInserted( (SvLBoxEntry*)pEntry );
    pImp->EntryInserted( (SvLBoxEntry*)pEntry );
}

void SvTreeListBox::ModelIsMoving(SvListEntry* pSource,
                                        SvListEntry* /* pTargetParent */,
                                        sal_uLong /* nChildPos */ )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->MovingEntry( (SvLBoxEntry*)pSource );
}

void SvTreeListBox::ModelHasMoved( SvListEntry* pSource )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->EntryMoved( (SvLBoxEntry*)pSource );
}

void SvTreeListBox::ModelIsRemoving( SvListEntry* pEntry )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if(pEdEntry == pEntry)
        pEdEntry = NULL;

    pImp->RemovingEntry( (SvLBoxEntry*)pEntry );
    NotifyRemoving( (SvLBoxEntry*)pEntry );
}

void SvTreeListBox::ModelHasRemoved( SvListEntry* pEntry  )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if ( pEntry == pHdlEntry)
        pHdlEntry = NULL;
    pImp->EntryRemoved();
}

void SvTreeListBox::SetCollapsedNodeBmp( const Image& rBmp)
{
    DBG_CHKTHIS(SvTreeListBox,0);
    AdjustEntryHeight( rBmp );
    pImp->SetCollapsedNodeBmp( rBmp );
}

void SvTreeListBox::SetExpandedNodeBmp( const Image& rBmp )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    AdjustEntryHeight( rBmp );
    pImp->SetExpandedNodeBmp( rBmp );
}


void SvTreeListBox::SetFont( const Font& rFont )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    Font aTempFont( rFont );
    aTempFont.SetTransparent( sal_True );
    Control::SetFont( aTempFont );
    AdjustEntryHeight( aTempFont );
    // always invalidate, else things go wrong in SetEntryHeight
    RecalcViewData();
}


void SvTreeListBox::Paint( const Rectangle& rRect )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SvLBox::Paint( rRect );
    if( nTreeFlags & TREEFLAG_RECALCTABS )
        SetTabs();
    pImp->Paint( rRect );
}

void SvTreeListBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->MouseButtonDown( rMEvt );
}

void SvTreeListBox::MouseButtonUp( const MouseEvent& rMEvt )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->MouseButtonUp( rMEvt );
}

void SvTreeListBox::MouseMove( const MouseEvent& rMEvt )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->MouseMove( rMEvt );
}


void SvTreeListBox::SetUpdateMode( sal_Bool bUpdate )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->SetUpdateMode( bUpdate );
}

void SvTreeListBox::SetSpaceBetweenEntries( short nOffsLogic )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if( nOffsLogic != nEntryHeightOffs )
    {
        nEntryHeight = nEntryHeight - nEntryHeightOffs;
        nEntryHeightOffs = (short)nOffsLogic;
        nEntryHeight = nEntryHeight + nOffsLogic;
        AdjustEntryHeight( GetFont() );
        RecalcViewData();
        pImp->SetEntryHeight( nEntryHeight );
    }
}

void SvTreeListBox::SetCursor( SvLBoxEntry* pEntry, sal_Bool bForceNoSelect )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->SetCursor(pEntry, bForceNoSelect);
}

void SvTreeListBox::SetCurEntry( SvLBoxEntry* pEntry )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->SetCurEntry( pEntry );
}

Image SvTreeListBox::GetExpandedNodeBmp( ) const
{
    return pImp->GetExpandedNodeBmp( );
}

Point SvTreeListBox::GetEntryPosition( SvLBoxEntry* pEntry ) const
{
    return pImp->GetEntryPosition( pEntry );
}

void SvTreeListBox::ShowEntry( SvLBoxEntry* pEntry )
{
    MakeVisible( pEntry );
}

void SvTreeListBox::MakeVisible( SvLBoxEntry* pEntry )
{
    pImp->MakeVisible(pEntry);
}

void SvTreeListBox::MakeVisible( SvLBoxEntry* pEntry, sal_Bool bMoveToTop )
{
    pImp->MakeVisible( pEntry, bMoveToTop );
}

void SvTreeListBox::ModelHasEntryInvalidated( SvListEntry* pEntry )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    // reinitialize the separate items of the entries
    SvLBox::ModelHasEntryInvalidated( pEntry );
    // repaint
    pImp->InvalidateEntry( (SvLBoxEntry*)pEntry );
}

void SvTreeListBox::EditItemText( SvLBoxEntry* pEntry, SvLBoxString* pItem,
    const Selection& rSelection )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    DBG_ASSERT(pEntry&&pItem,"EditItemText: Bad params");
    if( IsSelected( pEntry ))
    {
        pImp->ShowCursor( sal_False );
        SvListView::Select( pEntry, sal_False );
        PaintEntry( pEntry );
        SvListView::Select( pEntry, sal_True );
        pImp->ShowCursor( sal_True );
    }
    pEdEntry = pEntry;
    pEdItem = pItem;
    SvLBoxTab* pTab = GetTab( pEntry, pItem );
    DBG_ASSERT(pTab,"EditItemText:Tab not found");

    Size aItemSize( pItem->GetSize(this, pEntry) );
    Point aPos = GetEntryPosition( pEntry );
    aPos.Y() += ( nEntryHeight - aItemSize.Height() ) / 2;
    aPos.X() = GetTabPos( pEntry, pTab );
    long nOutputWidth = pImp->GetOutputSize().Width();
    Size aSize( nOutputWidth - aPos.X(), aItemSize.Height() );
    sal_uInt16 nPos = std::find( aTabs.begin(), aTabs.end(), pTab ) - aTabs.begin();
    if( nPos+1 < (sal_uInt16)aTabs.size() )
    {
        SvLBoxTab* pRightTab = aTabs[ nPos + 1 ];
        long nRight = GetTabPos( pEntry, pRightTab );
        if( nRight <= nOutputWidth )
            aSize.Width() = nRight - aPos.X();
    }
    Point aOrigin( GetMapMode().GetOrigin() );
    aPos += aOrigin; // convert to win coordinates
    aSize.Width() -= aOrigin.X();
    Rectangle aRect( aPos, aSize );
    EditText( pItem->GetText(), aRect, rSelection );
}

void SvTreeListBox::EditEntry( SvLBoxEntry* pEntry )
{
    pImp->aEditClickPos = Point( -1, -1 );
    ImplEditEntry( pEntry );
}

void SvTreeListBox::ImplEditEntry( SvLBoxEntry* pEntry )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if( IsEditingActive() )
        EndEditing();
    if( !pEntry )
        pEntry = GetCurEntry();
    if( pEntry )
    {
        long nClickX = pImp->aEditClickPos.X();
        bool bIsMouseTriggered = nClickX >= 0;

        SvLBoxString* pItem = NULL;
        sal_uInt16 nCount = pEntry->ItemCount();
        long nTabPos, nNextTabPos = 0;
        for( sal_uInt16 i = 0 ; i < nCount ; i++ )
        {
            SvLBoxItem* pTmpItem = pEntry->GetItem( i );
            if( pTmpItem->IsA() != SV_ITEM_ID_LBOXSTRING )
                continue;

            SvLBoxTab* pTab = GetTab( pEntry, pTmpItem );
            nNextTabPos = -1;
            if( i < nCount - 1 )
            {
                SvLBoxItem* pNextItem = pEntry->GetItem( i + 1 );
                SvLBoxTab* pNextTab = GetTab( pEntry, pNextItem );
                nNextTabPos = pNextTab->GetPos();
            }

            if( pTab && pTab->IsEditable() )
            {
                nTabPos = pTab->GetPos();
                if( !bIsMouseTriggered || (nClickX > nTabPos && (nNextTabPos == -1 || nClickX < nNextTabPos ) ) )
                {
                    pItem = static_cast<SvLBoxString*>( pTmpItem );
                    break;
                }
            }
        }

        Selection aSel( SELECTION_MIN, SELECTION_MAX );
        if( pItem && EditingEntry( pEntry, aSel ) )
        {
            SelectAll( sal_False );
            MakeVisible( pEntry );
            EditItemText( pEntry, pItem, aSel );
        }
    }
}

sal_Bool SvTreeListBox::AreChildrenTransient() const
{
    return pImp->AreChildrenTransient();
}

void SvTreeListBox::SetChildrenNotTransient()
{
    pImp->SetChildrenNotTransient();
}

void SvTreeListBox::EditedText( const XubString& rStr )

{
    DBG_CHKTHIS(SvTreeListBox,0);
    if(pEdEntry) // we have to check if this entry is null that means that it is removed while editing
    {
        if( EditedEntry( pEdEntry, rStr ) )
        {
            ((SvLBoxString*)pEdItem)->SetText( rStr );
            pModel->InvalidateEntry( pEdEntry );
        }
        if( GetSelectionCount() == 0 )
            Select( pEdEntry );
        if( GetSelectionMode() == MULTIPLE_SELECTION && !GetCurEntry() )
            SetCurEntry( pEdEntry );
    }
}

void SvTreeListBox::EditingRequest( SvLBoxEntry* pEntry, SvLBoxItem* pItem,
                                    const Point& )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if( IsEditingActive() )
        EndEditing();
    if( pItem->IsA() == SV_ITEM_ID_LBOXSTRING )
    {
        Selection aSel( SELECTION_MIN, SELECTION_MAX );
        if( EditingEntry( pEntry, aSel ) )
        {
            SelectAll( sal_False );
            EditItemText( pEntry, (SvLBoxString*)pItem, aSel );
        }
    }
}



SvLBoxEntry* SvTreeListBox::GetDropTarget( const Point& rPos )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    // scroll
    if( rPos.Y() < 12 )
    {
        SvLBox::ImplShowTargetEmphasis( SvLBox::pTargetEntry, sal_False );
        ScrollOutputArea( +1 );
    }
    else
    {
        Size aSize( pImp->GetOutputSize() );
        if( rPos.Y() > aSize.Height() - 12 )
        {
            SvLBox::ImplShowTargetEmphasis( SvLBox::pTargetEntry, sal_False );
            ScrollOutputArea( -1 );
        }
    }

    SvLBoxEntry* pTarget = pImp->GetEntry( rPos );
    // when dropping in a vacant space, use the last entry
    if( !pTarget )
        return (SvLBoxEntry*)LastVisible();
    else if( (GetDragDropMode() & SV_DRAGDROP_ENABLE_TOP) &&
             pTarget == First() && rPos.Y() < 6 )
        return 0;

    return pTarget;
}


SvLBoxEntry* SvTreeListBox::GetEntry( const Point& rPos, sal_Bool bHit ) const
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SvLBoxEntry* pEntry = pImp->GetEntry( rPos );
    if( pEntry && bHit )
    {
        long nLine = pImp->GetEntryLine( pEntry );
        if( !(pImp->EntryReallyHit( pEntry, rPos, nLine)) )
            return 0;
    }
    return pEntry;
}

SvLBoxEntry* SvTreeListBox::GetCurEntry() const
{
    DBG_CHKTHIS(SvTreeListBox,0);
    return pImp->GetCurEntry();
}

void SvTreeListBox::ImplInitStyle()
{
    DBG_CHKTHIS(SvTreeListBox,0);

    const WinBits nWindowStyle = GetStyle();

    nTreeFlags |= TREEFLAG_RECALCTABS;
    if( nWindowStyle & WB_SORT )
    {
        GetModel()->SetSortMode( SortAscending );
        GetModel()->SetCompareHdl( LINK(this,SvTreeListBox,DefaultCompare));
    }
    else
    {
        GetModel()->SetSortMode( SortNone );
        GetModel()->SetCompareHdl( Link() );
    }
    pImp->SetStyle( nWindowStyle );
    pImp->Resize();
    Invalidate();
}

void SvTreeListBox::PaintEntry( SvLBoxEntry* pEntry )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    DBG_ASSERT(pEntry,"PaintEntry:No Entry");
    if( pEntry )
        pImp->PaintEntry( pEntry );
}

void SvTreeListBox::InvalidateEntry( SvLBoxEntry* pEntry )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    DBG_ASSERT(pEntry,"InvalidateEntry:No Entry");
    if( pEntry )
    {
        GetModel()->InvalidateEntry( pEntry );
    }
}


long SvTreeListBox::PaintEntry(SvLBoxEntry* pEntry,long nLine,sal_uInt16 nTabFlags)
{
    return PaintEntry1(pEntry,nLine,nTabFlags);
}

#define SV_TAB_BORDER 8

long SvTreeListBox::PaintEntry1(SvLBoxEntry* pEntry,long nLine,sal_uInt16 nTabFlags,
    sal_Bool bHasClipRegion )
{
    DBG_CHKTHIS(SvTreeListBox,0);

    Rectangle aRect; // multi purpose

    sal_Bool bHorSBar = pImp->HasHorScrollBar();
    PreparePaint( pEntry );

    pImp->UpdateContextBmpWidthMax( pEntry );

    if( nTreeFlags & TREEFLAG_RECALCTABS )
        SetTabs();

    short nTempEntryHeight = GetEntryHeight();
    long nWidth = pImp->GetOutputSize().Width();

    // Did we turn on the scrollbar within PreparePaints? If yes, we have to set
    // the ClipRegion anew.
    if( !bHorSBar && pImp->HasHorScrollBar() )
        SetClipRegion( Region(pImp->GetClipRegionRect()) );

    Point aEntryPos( GetMapMode().GetOrigin() );
    aEntryPos.X() *= -1; // conversion document coordinates
    long nMaxRight = nWidth + aEntryPos.X() - 1;

    Color aBackupTextColor( GetTextColor() );
    Font aBackupFont( GetFont() );
    Color aBackupColor = GetFillColor();

    bool bCurFontIsSel = false;
    sal_Bool bInUse = pEntry->HasInUseEmphasis();
    // if a ClipRegion was set from outside, we don't have to reset it
    const WinBits nWindowStyle = GetStyle();
    const sal_Bool bResetClipRegion = !bHasClipRegion;
    const sal_Bool bHideSelection = ((nWindowStyle & WB_HIDESELECTION) && !HasFocus())!=0;
    const StyleSettings& rSettings = GetSettings().GetStyleSettings();

    Font aHighlightFont( GetFont() );
    const Color aHighlightTextColor( rSettings.GetHighlightTextColor() );
    aHighlightFont.SetColor( aHighlightTextColor );

    Size aRectSize( 0, nTempEntryHeight );

    if( !bHasClipRegion && nWindowStyle & WB_HSCROLL )
    {
        SetClipRegion( Region(pImp->GetClipRegionRect()) );
        bHasClipRegion = sal_True;
    }

    SvViewDataEntry* pViewDataEntry = GetViewDataEntry( pEntry );

    sal_uInt16 nTabCount = aTabs.size();
    sal_uInt16 nItemCount = pEntry->ItemCount();
    sal_uInt16 nCurTab = 0;
    sal_uInt16 nCurItem = 0;

    while( nCurTab < nTabCount && nCurItem < nItemCount )
    {
        SvLBoxTab* pTab = aTabs[ nCurTab ];
        sal_uInt16 nNextTab = nCurTab + 1;
        SvLBoxTab* pNextTab = nNextTab < nTabCount ? aTabs[nNextTab] : 0;
        SvLBoxItem* pItem = nCurItem < nItemCount ? pEntry->GetItem(nCurItem) : 0;

        sal_uInt16 nFlags = pTab->nFlags;
        Size aSize( pItem->GetSize( pViewDataEntry, nCurItem ));
        long nTabPos = GetTabPos( pEntry, pTab );

        long nNextTabPos;
        if( pNextTab )
            nNextTabPos = GetTabPos( pEntry, pNextTab );
        else
        {
            nNextTabPos = nMaxRight;
            if( nTabPos > nMaxRight )
                nNextTabPos += 50;
        }

        long nX;
        if( pTab->nFlags & SV_LBOXTAB_ADJUST_RIGHT )
            // avoid cutting the right edge off the tab separation
            nX = nTabPos + pTab->CalcOffset(aSize.Width(), (nNextTabPos-SV_TAB_BORDER-1) -nTabPos);
        else
            nX = nTabPos + pTab->CalcOffset(aSize.Width(), nNextTabPos-nTabPos);

        if( nFlags & nTabFlags )
        {
            if( !bHasClipRegion && nX + aSize.Width() >= nMaxRight )
            {
                SetClipRegion( Region(pImp->GetClipRegionRect()) );
                bHasClipRegion = sal_True;
            }
            aEntryPos.X() = nX;
            aEntryPos.Y() = nLine;

            // set background pattern/color

            Wallpaper aWallpaper = GetBackground();

            int bSelTab = nFlags & SV_LBOXTAB_SHOW_SELECTION;
            sal_uInt16 nItemType = pItem->IsA();

            if ( pViewDataEntry->IsSelected() && bSelTab && !pViewDataEntry->IsCursored() )
            {
                Color aNewWallColor = rSettings.GetHighlightColor();
                if ( !bInUse || nItemType != SV_ITEM_ID_LBOXCONTEXTBMP )
                {
                    // if the face color is bright then the deactive color is also bright
                    // -> so you can't see any deactive selection
                    if ( bHideSelection && !rSettings.GetFaceColor().IsBright() &&
                         aWallpaper.GetColor().IsBright() != rSettings.GetDeactiveColor().IsBright() )
                        aNewWallColor = rSettings.GetDeactiveColor();
                    // set font color to highlight
                    if ( !bCurFontIsSel )
                    {
                        SetTextColor( aHighlightTextColor );
                        Control::SetFont( aHighlightFont );
                        bCurFontIsSel = true;
                    }
                }
                aWallpaper.SetColor( aNewWallColor );
            }
            else  // no selection
            {
                if( bInUse && nItemType == SV_ITEM_ID_LBOXCONTEXTBMP )
                    aWallpaper.SetColor( rSettings.GetFieldColor() );
                else if( bCurFontIsSel )
                {
                    bCurFontIsSel = false;
                    SetTextColor( aBackupTextColor );
                    Control::SetFont( aBackupFont );
                }
            }

            // draw background
            if( !(nTreeFlags & TREEFLAG_USESEL))
            {
                // only draw the area that is used by the item
                aRectSize.Width() = aSize.Width();
                aRect.SetPos( aEntryPos );
                aRect.SetSize( aRectSize );
            }
            else
            {
                // draw from the current to the next tab
                if( nCurTab != 0 )
                    aRect.Left() = nTabPos;
                else
                    // if we're in the 0th tab, always draw from column 0 --
                    // else we get problems with centered tabs
                    aRect.Left() = 0;
                aRect.Top() = nLine;
                aRect.Bottom() = nLine + nTempEntryHeight - 1;
                if( pNextTab )
                {
                    long nRight;
                    nRight = GetTabPos(pEntry,pNextTab)-1;
                    if( nRight > nMaxRight )
                        nRight = nMaxRight;
                    aRect.Right() = nRight;
                }
                else
                    aRect.Right() = nMaxRight;
            }
            // A custom selection that starts at a tab position > 0, do not fill
            // the background of the 0th item, else e.g. we might not be able to
            // realize tab listboxes with lines.
            if( !(nCurTab==0 && (nTreeFlags & TREEFLAG_USESEL) && nFirstSelTab) )
            {
                SetFillColor( aWallpaper.GetColor() );
                // this case may occur for smaller horizontal resizes
                if( aRect.Left() < aRect.Right() )
                    DrawRect( aRect );
            }
            // draw item
            // center vertically
            aEntryPos.Y() += ( nTempEntryHeight - aSize.Height() ) / 2;
            pItem->Paint( aEntryPos, *this, pViewDataEntry->GetFlags(), pEntry );

            // division line between tabs
            if( pNextTab && pItem->IsA() == SV_ITEM_ID_LBOXSTRING &&
                // not at the right edge of the window!
                aRect.Right() < nMaxRight )
            {
                aRect.Left() = aRect.Right() - SV_TAB_BORDER;
                DrawRect( aRect );
            }

            SetFillColor( aBackupColor );
        }
        nCurItem++;
        nCurTab++;
    }
    if( pViewDataEntry->IsCursored() && !HasFocus() )
    {
        // cursor emphasis
        SetFillColor();
        Color aOldLineColor = GetLineColor();
        SetLineColor( Color( COL_BLACK ) );
        aRect = GetFocusRect( pEntry, nLine );
        aRect.Top()++;
        aRect.Bottom()--;
        DrawRect( aRect );
        SetLineColor( aOldLineColor );
        SetFillColor( aBackupColor );
    }

    if( bCurFontIsSel )
    {
        SetTextColor( aBackupTextColor );
        Control::SetFont( aBackupFont );
    }

    sal_uInt16 nFirstDynTabPos;
    SvLBoxTab* pFirstDynamicTab = GetFirstDynamicTab( nFirstDynTabPos );
    long nDynTabPos = GetTabPos( pEntry, pFirstDynamicTab );
    nDynTabPos += pImp->nNodeBmpTabDistance;
    nDynTabPos += pImp->nNodeBmpWidth / 2;
    nDynTabPos += 4; // 4 pixels of buffer, so the node bitmap is not too close
                     // to the next tab

    if( (!(pEntry->GetFlags() & SV_ENTRYFLAG_NO_NODEBMP)) &&
        (nWindowStyle & WB_HASBUTTONS) && pFirstDynamicTab &&
        ( pEntry->HasChildren() || pEntry->HasChildrenOnDemand() ) )
    {
        // find first tab and check if the node bitmap extends into it
        sal_uInt16 nNextTab = nFirstDynTabPos;
        SvLBoxTab* pNextTab;
        do
        {
            nNextTab++;
            pNextTab = nNextTab < nTabCount ? aTabs[nNextTab] : 0;
        } while( pNextTab && pNextTab->IsDynamic() );

        if( !pNextTab || (GetTabPos( pEntry, pNextTab ) > nDynTabPos) )
        {
            if((nWindowStyle & WB_HASBUTTONSATROOT) || pModel->GetDepth(pEntry) > 0)
            {
                Point aPos( GetTabPos(pEntry,pFirstDynamicTab), nLine );
                aPos.X() += pImp->nNodeBmpTabDistance;

                const Image* pImg = 0;

                if( IsExpanded(pEntry) )
                    pImg = &pImp->GetExpandedNodeBmp( );
                else
                {
                    if( (!pEntry->HasChildren()) && pEntry->HasChildrenOnDemand() &&
                        (!(pEntry->GetFlags() & SV_ENTRYFLAG_HAD_CHILDREN)) &&
                        pImp->GetDontKnowNodeBmp().GetSizePixel().Width() )
                        pImg = &pImp->GetDontKnowNodeBmp( );
                    else
                        pImg = &pImp->GetCollapsedNodeBmp( );
                }
                aPos.Y() += (nTempEntryHeight - pImg->GetSizePixel().Height()) / 2;

                sal_uInt16 nStyle = 0;
                if ( !IsEnabled() )
                    nStyle |= IMAGE_DRAW_DISABLE;

                //native
                sal_Bool bNativeOK = sal_False;
                if ( IsNativeControlSupported( CTRL_LISTNODE, PART_ENTIRE_CONTROL) )
                {
                    ImplControlValue    aControlValue;
                    Rectangle           aCtrlRegion( aPos,  pImg->GetSizePixel() );
                    ControlState        nState = 0;

                    if ( IsEnabled() )  nState |= CTRL_STATE_ENABLED;

                    if ( IsExpanded(pEntry) )
                        aControlValue.setTristateVal( BUTTONVALUE_ON );//expanded node
                    else
                    {
                        if( (!pEntry->HasChildren() )                              &&
                              pEntry->HasChildrenOnDemand()                        &&
                             (!(pEntry->GetFlags() & SV_ENTRYFLAG_HAD_CHILDREN)) &&
                            pImp->GetDontKnowNodeBmp().GetSizePixel().Width()
                        )
                            aControlValue.setTristateVal( BUTTONVALUE_DONTKNOW ); //dont know
                        else
                            aControlValue.setTristateVal( BUTTONVALUE_OFF ); //collapsed node
                    }

                    bNativeOK = DrawNativeControl( CTRL_LISTNODE, PART_ENTIRE_CONTROL,
                                            aCtrlRegion, nState, aControlValue, rtl::OUString() );
                }

                if( !bNativeOK) {
                    DrawImage( aPos, *pImg ,nStyle);
                }
            }
        }
    }


    if( bHasClipRegion && bResetClipRegion )
        SetClipRegion();
    return 0; // nRowLen;
}

void SvTreeListBox::PreparePaint( SvLBoxEntry* )
{
}

Rectangle SvTreeListBox::GetFocusRect( SvLBoxEntry* pEntry, long nLine )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    Size aSize;
    Rectangle aRect;
    aRect.Top() = nLine;
    aSize.Height() = GetEntryHeight();

    long nRealWidth = pImp->GetOutputSize().Width();
    nRealWidth -= GetMapMode().GetOrigin().X();

    sal_uInt16 nCurTab;
    SvLBoxTab* pTab = GetFirstTab( SV_LBOXTAB_SHOW_SELECTION, nCurTab );
    long nTabPos = 0;
    if( pTab )
        nTabPos = GetTabPos( pEntry, pTab );
    long nNextTabPos;
    if( pTab && nCurTab < aTabs.size() - 1 )
    {
        SvLBoxTab* pNextTab = aTabs[ nCurTab + 1 ];
        nNextTabPos = GetTabPos( pEntry, pNextTab );
    }
    else
    {
        nNextTabPos = nRealWidth;
        if( nTabPos > nRealWidth )
            nNextTabPos += 50;
    }

    sal_Bool bUserSelection = (sal_Bool)( nTreeFlags & TREEFLAG_USESEL ) != 0;
    if( !bUserSelection )
    {
        if( pTab && nCurTab < pEntry->ItemCount() )
        {
            SvLBoxItem* pItem = pEntry->GetItem( nCurTab );
            aSize.Width() = pItem->GetSize( this, pEntry ).Width();
            if( !aSize.Width() )
                aSize.Width() = 15;
            long nX = nTabPos; //GetTabPos( pEntry, pTab );
            // alignment
            nX += pTab->CalcOffset( aSize.Width(), nNextTabPos - nTabPos );
            aRect.Left() = nX;
            // make sure that first and last letter aren't cut off slightly
            aRect.SetSize( aSize );
            if( aRect.Left() > 0 )
                aRect.Left()--;
            aRect.Right()++;
        }
    }
    else
    {
        // if SelTab != 0, we have to calculate also
        if( nFocusWidth == -1 || nFirstSelTab )
        {
            sal_uInt16 nLastTab;
            SvLBoxTab* pLastTab = GetLastTab(SV_LBOXTAB_SHOW_SELECTION,nLastTab);
            nLastTab++;
            if( nLastTab < aTabs.size() ) // is there another one?
                pLastTab = aTabs[ nLastTab ];
            else
                pLastTab = 0;  // select whole width
            aSize.Width() = pLastTab ? pLastTab->GetPos() : 0x0fffffff;
            nFocusWidth = (short)aSize.Width();
            if( pTab )
                nFocusWidth = nFocusWidth - (short)nTabPos; //pTab->GetPos();
        }
        else
        {
            aSize.Width() = nFocusWidth;
            if( pTab )
            {
                if( nCurTab )
                    aSize.Width() += nTabPos;
                else
                    aSize.Width() += pTab->GetPos(); // Tab0 always from the leftmost position
            }
        }
        // if selection starts with 0th tab, draw from column 0 on
        if( nCurTab != 0 )
        {
            aRect.Left() = nTabPos;
            aSize.Width() -= nTabPos;
        }
        aRect.SetSize( aSize );
    }
    // adjust right edge because of clipping
    if( aRect.Right() >= nRealWidth )
    {
        aRect.Right() = nRealWidth-1;
        nFocusWidth = (short)aRect.GetWidth();
    }
    return aRect;
}


long SvTreeListBox::GetTabPos( SvLBoxEntry* pEntry, SvLBoxTab* pTab)
{
    DBG_CHKTHIS(SvTreeListBox,0);
    DBG_ASSERT(pTab,"No Tab");
    long nPos = pTab->GetPos();
    if( pTab->IsDynamic() )
    {
        sal_uInt16 nDepth = pModel->GetDepth( pEntry );
        nDepth = nDepth * (sal_uInt16)nIndent;
        nPos += (long)nDepth;
    }
    return nPos;
}

SvLBoxItem* SvTreeListBox::GetItem_Impl( SvLBoxEntry* pEntry, long nX,
    SvLBoxTab** ppTab, sal_uInt16 nEmptyWidth )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SvLBoxItem* pItemClicked = 0;
    sal_uInt16 nTabCount = aTabs.size();
    sal_uInt16 nItemCount = pEntry->ItemCount();
    SvLBoxTab* pTab = aTabs.front();
    SvLBoxItem* pItem = pEntry->GetItem(0);
    sal_uInt16 nNextItem = 1;
    nX -= GetMapMode().GetOrigin().X();
    long nRealWidth = pImp->GetOutputSize().Width();
    nRealWidth -= GetMapMode().GetOrigin().X();

    while( 1 )
    {
        SvLBoxTab* pNextTab=nNextItem<nTabCount ? aTabs[nNextItem] : 0;
        long nStart = GetTabPos( pEntry, pTab );

        long nNextTabPos;
        if( pNextTab )
            nNextTabPos = GetTabPos( pEntry, pNextTab );
        else
        {
            nNextTabPos = nRealWidth;
            if( nStart > nRealWidth )
                nNextTabPos += 50;
        }

        Size aItemSize( pItem->GetSize(this, pEntry));
        nStart += pTab->CalcOffset( aItemSize.Width(), nNextTabPos - nStart );
        long nLen = aItemSize.Width();
        if( pNextTab )
        {
            long nTabWidth = GetTabPos( pEntry, pNextTab ) - nStart;
            if( nTabWidth < nLen )
                nLen = nTabWidth;
        }

        if( !nLen )
            nLen = nEmptyWidth;

        if( nX >= nStart && nX < (nStart+nLen ) )
        {
            pItemClicked = pItem;
            if( ppTab )
            {
                *ppTab = pTab;
                break;
            }
        }
        if( nNextItem >= nItemCount || nNextItem >= nTabCount)
            break;
        pTab = aTabs[ nNextItem ];
        pItem = pEntry->GetItem( nNextItem );
        nNextItem++;
    }
    return pItemClicked;
}

SvLBoxItem* SvTreeListBox::GetItem(SvLBoxEntry* pEntry,long nX,SvLBoxTab** ppTab)
{
    return GetItem_Impl( pEntry, nX, ppTab, 0 );
}

SvLBoxItem* SvTreeListBox::GetItem(SvLBoxEntry* pEntry,long nX )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SvLBoxTab* pDummyTab;
    return GetItem_Impl( pEntry, nX, &pDummyTab, 0 );
}

void SvTreeListBox::AddTab(long nTabPos,sal_uInt16 nFlags,void* pUserData )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    nFocusWidth = -1;
    SvLBoxTab* pTab = new SvLBoxTab( nTabPos, nFlags );
    pTab->SetUserData( pUserData );
    aTabs.push_back( pTab );
    if( nTreeFlags & TREEFLAG_USESEL )
    {
        sal_uInt16 nPos = aTabs.size() - 1;
        if( nPos >= nFirstSelTab && nPos <= nLastSelTab )
            pTab->nFlags |= SV_LBOXTAB_SHOW_SELECTION;
        else
            // string items usually have to be selected -- turn this off
            // explicitly
            pTab->nFlags &= ~SV_LBOXTAB_SHOW_SELECTION;
    }
}



SvLBoxTab* SvTreeListBox::GetFirstDynamicTab( sal_uInt16& rPos ) const
{
    DBG_CHKTHIS(SvTreeListBox,0);
    sal_uInt16 nCurTab = 0;
    sal_uInt16 nTabCount = aTabs.size();
    while( nCurTab < nTabCount )
    {
        SvLBoxTab* pTab = aTabs[nCurTab];
        if( pTab->nFlags & SV_LBOXTAB_DYNAMIC )
        {
            rPos = nCurTab;
            return pTab;
        }
        nCurTab++;
    }
    return 0;
}

SvLBoxTab* SvTreeListBox::GetFirstDynamicTab() const
{
    sal_uInt16 nDummy;
    return GetFirstDynamicTab( nDummy );
}

SvLBoxTab* SvTreeListBox::GetTab( SvLBoxEntry* pEntry, SvLBoxItem* pItem) const
{
    DBG_CHKTHIS(SvTreeListBox,0);
    sal_uInt16 nPos = pEntry->GetPos( pItem );
    return aTabs[ nPos ];
}

void SvTreeListBox::ClearTabList()
{
    DBG_CHKTHIS(SvTreeListBox,0);
    sal_uInt16 nTabCount = aTabs.size();
    while( nTabCount )
    {
        nTabCount--;
        SvLBoxTab* pDelTab = aTabs[ nTabCount ];
        delete pDelTab;
    }
    aTabs.clear();
}


Size SvTreeListBox::GetOutputSizePixel() const
{
    DBG_CHKTHIS(SvTreeListBox,0);
    Size aSize = pImp->GetOutputSize();
    return aSize;
}

void SvTreeListBox::NotifyBeginScroll()
{
    DBG_CHKTHIS(SvTreeListBox,0);
}

void SvTreeListBox::NotifyEndScroll()
{
    DBG_CHKTHIS(SvTreeListBox,0);
}

void SvTreeListBox::NotifyScrolling( long )
{
    DBG_CHKTHIS(SvTreeListBox,0);
}

void SvTreeListBox::NotifyScrolled()
{
    DBG_CHKTHIS(SvTreeListBox,0);
    aScrolledHdl.Call( this );
}

void SvTreeListBox::NotifyInvalidating()
{
    DBG_CHKTHIS(SvTreeListBox,0);
}

void SvTreeListBox::Invalidate( sal_uInt16 nInvalidateFlags )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if( nFocusWidth == -1 )
        // to make sure that the control doesn't show the wrong focus rectangle
        // after painting
        pImp->RecalcFocusRect();
    NotifyInvalidating();
    SvLBox::Invalidate( nInvalidateFlags );
    pImp->Invalidate();
}

void SvTreeListBox::Invalidate( const Rectangle& rRect, sal_uInt16 nInvalidateFlags )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if( nFocusWidth == -1 )
        // to make sure that the control doesn't show the wrong focus rectangle
        // after painting
        pImp->RecalcFocusRect();
    NotifyInvalidating();
    SvLBox::Invalidate( rRect, nInvalidateFlags );
}


void SvTreeListBox::SetHighlightRange( sal_uInt16 nStart, sal_uInt16 nEnd)
{
    DBG_CHKTHIS(SvTreeListBox,0);

    sal_uInt16 nTemp;
    nTreeFlags |= TREEFLAG_USESEL;
    if( nStart > nEnd )
    {
        nTemp = nStart;
        nStart = nEnd;
        nEnd = nTemp;
    }
    // select all tabs that lie within the area
    nTreeFlags |= TREEFLAG_RECALCTABS;
    nFirstSelTab = nStart;
    nLastSelTab = nEnd;
    pImp->RecalcFocusRect();
}

sal_uLong SvTreeListBox::GetAscInsertionPos(SvLBoxEntry*,SvLBoxEntry*)
{
    return LIST_APPEND;
}

sal_uLong SvTreeListBox::GetDescInsertionPos(SvLBoxEntry*,SvLBoxEntry*)
{
    DBG_CHKTHIS(SvTreeListBox,0);
    return LIST_APPEND;
}

Region SvTreeListBox::GetDragRegion() const
{
    DBG_CHKTHIS(SvTreeListBox,0);
    Rectangle aRect;
    SvLBoxEntry* pEntry = GetCurEntry();
    if( pEntry )
    {
        Point aPos = GetEntryPosition( pEntry );
        aRect = ((SvTreeListBox*)this)->GetFocusRect( pEntry, aPos.Y() );
    }
    Region aRegion( aRect );
    return aRegion;
}


void SvTreeListBox::Command( const CommandEvent& rCEvt )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    // FIXME gnumake2 resync to DEV300_m84
    pImp->Command( rCEvt );
}


void SvTreeListBox::RemoveParentKeepChildren( SvLBoxEntry* pParent )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    DBG_ASSERT(pParent,"RemoveParentKeepChildren:No Parent");
    SvLBoxEntry* pNewParent = GetParent( pParent );
    if( pParent->HasChildren())
    {
        SvLBoxEntry* pChild = FirstChild( pParent );
        while( pChild )
        {
            pModel->Move( pChild, pNewParent, LIST_APPEND );
            pChild = FirstChild( pParent );
        }
    }
    pModel->Remove( pParent );
}

SvLBoxTab* SvTreeListBox::GetFirstTab( sal_uInt16 nFlagMask, sal_uInt16& rPos )
{
    sal_uInt16 nTabCount = aTabs.size();
    for( sal_uInt16 nPos = 0; nPos < nTabCount; nPos++ )
    {
        SvLBoxTab* pTab = aTabs[ nPos ];
        if( (pTab->nFlags & nFlagMask) )
        {
            rPos = nPos;
            return pTab;
        }
    }
    rPos = 0xffff;
    return 0;
}

SvLBoxTab* SvTreeListBox::GetLastTab( sal_uInt16 nFlagMask, sal_uInt16& rTabPos )
{
    sal_uInt16 nPos = (sal_uInt16)aTabs.size();
    while( nPos )
    {
        --nPos;
        SvLBoxTab* pTab = aTabs[ nPos ];
        if( (pTab->nFlags & nFlagMask) )
        {
            rTabPos = nPos;
            return pTab;
        }
    }
    rTabPos = 0xffff;
    return 0;
}

void SvTreeListBox::RequestHelp( const HelpEvent& rHEvt )
{
    if( !pImp->RequestHelp( rHEvt ) )
        SvLBox::RequestHelp( rHEvt );
}

void SvTreeListBox::CursorMoved( SvLBoxEntry* )
{
}

IMPL_LINK( SvTreeListBox, DefaultCompare, SvSortData*, pData )
{
    SvLBoxEntry* pLeft = (SvLBoxEntry*)(pData->pLeft );
    SvLBoxEntry* pRight = (SvLBoxEntry*)(pData->pRight );
    String aLeft( ((SvLBoxString*)(pLeft->GetFirstItem(SV_ITEM_ID_LBOXSTRING)))->GetText());
    String aRight( ((SvLBoxString*)(pRight->GetFirstItem(SV_ITEM_ID_LBOXSTRING)))->GetText());
    pImp->UpdateStringSorter();
    return pImp->m_pStringSorter->compare(aLeft, aRight);
}

void SvTreeListBox::ModelNotification( sal_uInt16 nActionId, SvListEntry* pEntry1,
                        SvListEntry* pEntry2, sal_uLong nPos )
{
    if( nActionId == LISTACTION_CLEARING )
        CancelTextEditing();

    SvLBox::ModelNotification( nActionId, pEntry1, pEntry2, nPos );
    switch( nActionId )
    {
        case LISTACTION_INSERTED:
        {
            SvLBoxEntry* pEntry( dynamic_cast< SvLBoxEntry* >( pEntry1 ) );
            if ( !pEntry )
            {
                SAL_WARN( "svtools.contnr", "SvTreeListBox::ModelNotification: invalid entry!" );
                break;
            }

            SvLBoxContextBmp* pBmpItem = static_cast< SvLBoxContextBmp* >( pEntry->GetFirstItem( SV_ITEM_ID_LBOXCONTEXTBMP ) );
            if ( !pBmpItem )
                break;
            const Image& rBitmap1( pBmpItem->GetBitmap1() );
            const Image& rBitmap2( pBmpItem->GetBitmap2() );
            short nMaxWidth = short( Max( rBitmap1.GetSizePixel().Width(), rBitmap2.GetSizePixel().Width() ) );
            nMaxWidth = pImp->UpdateContextBmpWidthVector( pEntry, nMaxWidth );
            if( nMaxWidth > nContextBmpWidthMax )
            {
                nContextBmpWidthMax = nMaxWidth;
                SetTabs();
            }
        }
        break;

        case LISTACTION_RESORTING:
            SetUpdateMode( sal_False );
            break;

        case LISTACTION_RESORTED:
            // after a selection: show first entry and also keep the selection
            MakeVisible( (SvLBoxEntry*)pModel->First(), sal_True );
            SetUpdateMode( sal_True );
            break;

        case LISTACTION_CLEARED:
            if( IsUpdateMode() )
                Update();
            break;
    }
}

void SvTreeListBox::EndSelection()
{
    pImp->EndSelection();
}

void SvTreeListBox::RepaintScrollBars() const
{
    ((SvTreeListBox*)this)->pImp->RepaintScrollBars();
}

ScrollBar *SvTreeListBox::GetVScroll()
{
    return &((SvTreeListBox*)this)->pImp->aVerSBar;
}

ScrollBar *SvTreeListBox::GetHScroll()
{
    return &((SvTreeListBox*)this)->pImp->aHorSBar;
}

void SvTreeListBox::EnableAsyncDrag( sal_Bool b )
{
    pImp->EnableAsyncDrag( b );
}

SvLBoxEntry* SvTreeListBox::GetFirstEntryInView() const
{
    Point aPos;
    return GetEntry( aPos );
}

SvLBoxEntry* SvTreeListBox::GetNextEntryInView(SvLBoxEntry* pEntry ) const
{
    SvLBoxEntry* pNext = (SvLBoxEntry*)NextVisible( pEntry );
    if( pNext )
    {
        Point aPos( GetEntryPosition(pNext) );
        const Size& rSize = pImp->GetOutputSize();
        if( aPos.Y() < 0 || aPos.Y() >= rSize.Height() )
            return 0;
    }
    return pNext;
}

SvLBoxEntry* SvTreeListBox::GetLastEntryInView() const
{
    SvLBoxEntry* pEntry = GetFirstEntryInView();
    SvLBoxEntry* pNext = 0;
    while( pEntry )
    {
        pNext = (SvLBoxEntry*)NextVisible( pEntry );
        if( pNext )
        {
          Point aPos( GetEntryPosition(pNext) );
          const Size& rSize = pImp->GetOutputSize();
          if( aPos.Y() < 0 || aPos.Y() + GetEntryHeight() >= rSize.Height() )
              break;
          else
              pEntry = pNext;
        }
        else
            break;
    }
    return pEntry;
}

void SvTreeListBox::ShowFocusRect( const SvLBoxEntry* pEntry )
{
    pImp->ShowFocusRect( pEntry );
}

void SvTreeListBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    if( (rDCEvt.GetType()==DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        nEntryHeight = 0;   // _together_ with sal_True of 1. par (bFont) of InitSettings() a zero-height
                            //  forces complete recalc of heights!
        InitSettings( sal_True, sal_True, sal_True );
        Invalidate();
    }
    else
        Control::DataChanged( rDCEvt );
}

void SvTreeListBox::StateChanged( StateChangedType i_nStateChange )
{
    SvLBox::StateChanged( i_nStateChange );
    if ( i_nStateChange == STATE_CHANGE_STYLE )
        ImplInitStyle();
}

void SvTreeListBox::InitSettings(sal_Bool bFont,sal_Bool bForeground,sal_Bool bBackground)
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    if( bFont )
    {
        Font aFont;
        aFont = rStyleSettings.GetFieldFont();
        aFont.SetColor( rStyleSettings.GetWindowTextColor() );
        SetPointFont( aFont );
        AdjustEntryHeight( aFont );
        RecalcViewData();
    }

    if( bForeground || bFont )
    {
        SetTextColor( rStyleSettings.GetFieldTextColor() );
        SetTextFillColor();
    }

    if( bBackground )
        SetBackground( rStyleSettings.GetFieldColor() );

    // always try to re-create default-SvLBoxButtonData
    if( pCheckButtonData && pCheckButtonData->HasDefaultImages() )
        pCheckButtonData->SetDefaultImages( this );
}

sal_Bool SvTreeListBox::IsCellFocusEnabled() const
{
    return pImp->IsCellFocusEnabled();
}

bool SvTreeListBox::SetCurrentTabPos( sal_uInt16 _nNewPos )
{
    return pImp->SetCurrentTabPos( _nNewPos );
}

sal_uInt16 SvTreeListBox::GetCurrentTabPos() const
{
    return pImp->GetCurrentTabPos();
}

void SvTreeListBox::InitStartEntry()
{
    if( !pImp->pStartEntry )
        pImp->pStartEntry = GetModel()->First();
}

PopupMenu* SvTreeListBox::CreateContextMenu( void )
{
    return NULL;
}

void SvTreeListBox::ExcecuteContextMenuAction( sal_uInt16 )
{
    DBG_WARNING( "SvTreeListBox::ExcecuteContextMenuAction(): now there's happening nothing!" );
}

void SvTreeListBox::EnableContextMenuHandling( void )
{
    DBG_ASSERT( pImp, "-SvTreeListBox::EnableContextMenuHandling(): No implementation!" );

    pImp->bContextMenuHandling = sal_True;
}

void SvTreeListBox::EnableContextMenuHandling( sal_Bool b )
{
    DBG_ASSERT( pImp, "-SvTreeListBox::EnableContextMenuHandling(): No implementation!" );

    pImp->bContextMenuHandling = b;
}

sal_Bool SvTreeListBox::IsContextMenuHandlingEnabled( void ) const
{
    DBG_ASSERT( pImp, "-SvTreeListBox::IsContextMenuHandlingEnabled(): No implementation!" );

    return pImp->bContextMenuHandling;
}

void SvTreeListBox::EnableList( bool _bEnable )
{
    // call base class method
    Window::Enable( _bEnable != false );
    // then paint immediately
    Paint( Rectangle( Point(), GetSizePixel() ) );
}

::com::sun::star::uno::Reference< XAccessible > SvTreeListBox::CreateAccessible()
{
    Window* pParent = GetAccessibleParentWindow();
    DBG_ASSERT( pParent, "SvTreeListBox::CreateAccessible - accessible parent not found" );

    ::com::sun::star::uno::Reference< XAccessible > xAccessible;
    if ( pParent )
    {
        ::com::sun::star::uno::Reference< XAccessible > xAccParent = pParent->GetAccessible();
        if ( xAccParent.is() )
        {
            // need to be done here to get the vclxwindow later on in the accessbile
            ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xTemp(GetComponentInterface());
            xAccessible = pImp->m_aFactoryAccess.getFactory().createAccessibleTreeListBox( *this, xAccParent );
        }
    }
    return xAccessible;
}

void SvTreeListBox::FillAccessibleEntryStateSet( SvLBoxEntry* pEntry, ::utl::AccessibleStateSetHelper& rStateSet ) const
{
    DBG_ASSERT( pEntry, "SvTreeListBox::FillAccessibleEntryStateSet: invalid entry" );

    if ( pEntry->HasChildrenOnDemand() || pEntry->HasChildren() )
    {
        rStateSet.AddState( AccessibleStateType::EXPANDABLE );
        if ( IsExpanded( pEntry ) )
            rStateSet.AddState( (sal_Int16)AccessibleStateType::EXPANDED );
    }

    if ( GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED )
        rStateSet.AddState( AccessibleStateType::CHECKED );
    if ( IsEntryVisible( pEntry ) )
        rStateSet.AddState( AccessibleStateType::VISIBLE );
    if ( IsSelected( pEntry ) )
        rStateSet.AddState( AccessibleStateType::SELECTED );
}

Rectangle SvTreeListBox::GetBoundingRect( SvLBoxEntry* pEntry )
{
    Point aPos = GetEntryPosition( pEntry );
    Rectangle aRect = GetFocusRect( pEntry, aPos.Y() );
    return aRect;
}

void SvTreeListBox::EnableCellFocus()
{
    pImp->EnableCellFocus();
}

void SvTreeListBox::CallImplEventListeners(sal_uLong nEvent, void* pData)
{
    CallEventListeners(nEvent, pData);
}

void SvTreeListBox::FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& rStateSet ) const
{
      SvLBox::FillAccessibleStateSet( rStateSet );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
