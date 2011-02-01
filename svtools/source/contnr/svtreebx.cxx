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
#include "precompiled_svtools.hxx"

#define _SVTREEBX_CXX
#include <vcl/svapp.hxx>
#ifndef GCC
#endif

class TabBar;

// #102891# -----------------------

#include <svtools/svlbox.hxx>
#include <svtools/svlbitm.hxx>
#include <svtools/svtreebx.hxx>
#include <tools/diagnose_ex.h>
#include <svimpbox.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>


using namespace ::com::sun::star::accessibility;

/*
    Bugs/ToDo

    - Berechnung Rectangle beim Inplace-Editing (Bug bei manchen Fonts)
    - SetSpaceBetweenEntries: Offset wird in SetEntryHeight nicht
      beruecksichtigt
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
    InitSettings( TRUE, TRUE, TRUE );
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

ExtendedWinBits SvTreeListBox::GetExtendedWinBits() const
{
    return pImp->GetExtendedWindowBits();
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


USHORT SvTreeListBox::IsA()
{
    DBG_CHKTHIS(SvTreeListBox,0);
    return SV_LISTBOX_ID_TREEBOX;
}

void SvTreeListBox::SetSublistOpenWithReturn( BOOL b )
{
    pImp->bSubLstOpRet = b;
}

BOOL SvTreeListBox::IsSublistOpenWithReturn() const
{
    return pImp->bSubLstOpRet;
}

void SvTreeListBox::SetSublistOpenWithLeftRight( BOOL b )
{
    pImp->bSubLstOpLR = b;
}

BOOL SvTreeListBox::IsSublistOpenWithLeftRight() const
{
    return pImp->bSubLstOpLR;
}

void SvTreeListBox::Resize()
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if( IsEditingActive() )
        EndEditing( TRUE );
    SvLBox::Resize();
    pImp->Resize();
    nFocusWidth = -1;
    pImp->ShowCursor( FALSE );
    pImp->ShowCursor( TRUE );
}

/* Faelle:

   A) Entries haben Bitmaps
       0. Keine Buttons
       1. Node-Buttons (optional auch an Root-Items)
       2. Node-Buttons (optional auch an Root-Items) + CheckButton
       3. CheckButton
   B) Entries haben keine Bitmaps  (->ueber WindowBits wg. D&D !!!!!!)
       0. Keine Buttons
       1. Node-Buttons (optional auch an Root-Items)
       2. Node-Buttons (optional auch an Root-Items) + CheckButton
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

// bei Aenderungen GetTextOffset beruecksichtigen
void SvTreeListBox::SetTabs()
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if( IsEditingActive() )
        EndEditing( TRUE );
    nTreeFlags &= (~TREEFLAG_RECALCTABS);
    nFocusWidth = -1;
    const WinBits nStyle( GetStyle() );
    BOOL bHasButtons = (nStyle & WB_HASBUTTONS)!=0;
    BOOL bHasButtonsAtRoot = (nStyle & (WB_HASLINESATROOT |
                                              WB_HASBUTTONSATROOT))!=0;
    long nStartPos = TAB_STARTPOS;
    long nNodeWidthPixel = GetExpandedNodeBmp().GetSizePixel().Width();

    long nCheckWidth = 0;
    if( nTreeFlags & TREEFLAG_CHKBTN )
        nCheckWidth = pCheckButtonData->aBmps[0].GetSizePixel().Width();
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
            nStartPos += nContextWidthDIV2;  // wg. Zentrierung
            AddTab( nStartPos, TABFLAGS_CONTEXTBMP );
            nStartPos += nContextWidthDIV2;  // rechter Rand der Context-Bmp
            // Abstand setzen nur wenn Bitmaps da
            if( nContextBmpWidthMax )
                nStartPos += 5; // Abstand Context-Bmp - Text
            AddTab( nStartPos, TABFLAGS_TEXT );
            break;

        case NODE_BUTTONS :
            if( bHasButtonsAtRoot )
                nStartPos += ( nIndent + (nNodeWidthPixel/2) );
            else
                nStartPos += nContextWidthDIV2;
            AddTab( nStartPos, TABFLAGS_CONTEXTBMP );
            nStartPos += nContextWidthDIV2;  // rechter Rand der Context-Bmp
            // Abstand setzen nur wenn Bitmaps da
            if( nContextBmpWidthMax )
                nStartPos += 5; // Abstand Context-Bmp - Text
            AddTab( nStartPos, TABFLAGS_TEXT );
            break;

        case NODE_AND_CHECK_BUTTONS :
            if( bHasButtonsAtRoot )
                nStartPos += ( nIndent + nNodeWidthPixel );
            else
                nStartPos += nCheckWidthDIV2;
            AddTab( nStartPos, TABFLAGS_CHECKBTN );
            nStartPos += nCheckWidthDIV2;  // rechter Rand des CheckButtons
            nStartPos += 3;  // Abstand CheckButton Context-Bmp
            nStartPos += nContextWidthDIV2;  // Mitte der Context-Bmp
            AddTab( nStartPos, TABFLAGS_CONTEXTBMP );
            nStartPos += nContextWidthDIV2;  // rechter Rand der Context-Bmp
            // Abstand setzen nur wenn Bitmaps da
            if( nContextBmpWidthMax )
                nStartPos += 5; // Abstand Context-Bmp - Text
            AddTab( nStartPos, TABFLAGS_TEXT );
            break;

        case CHECK_BUTTONS :
            nStartPos += nCheckWidthDIV2;
            AddTab( nStartPos, TABFLAGS_CHECKBTN );
            nStartPos += nCheckWidthDIV2;  // rechter Rand CheckButton
            nStartPos += 3;  // Abstand CheckButton Context-Bmp
            nStartPos += nContextWidthDIV2;  // Mitte der Context-Bmp
            AddTab( nStartPos, TABFLAGS_CONTEXTBMP );
            nStartPos += nContextWidthDIV2;  // rechter Rand der Context-Bmp
            // Abstand setzen nur wenn Bitmaps da
            if( nContextBmpWidthMax )
                nStartPos += 5; // Abstand Context-Bmp - Text
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
    USHORT nCount = pEntry->ItemCount();
    USHORT nCur = 0;
    SvLBoxItem* pItem;
    while( nCur < nCount )
    {
        pItem = pEntry->GetItem( nCur );
        if ( pItem->IsA() == SV_ITEM_ID_LBOXSTRING &&
             static_cast<SvLBoxString*>( pItem )->GetText().Len() > 0 )
        {
            sRet = static_cast<SvLBoxString*>( pItem )->GetText();
            break;
        }
        nCur++;
    }
    return sRet;
}

const Image& SvTreeListBox::GetExpandedEntryBmp(SvLBoxEntry* pEntry, BmpColorMode _eMode) const
{
    DBG_CHKTHIS(SvTreeListBox,0);
    DBG_ASSERT(pEntry,"Entry?");
    SvLBoxContextBmp* pItem = (SvLBoxContextBmp*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
    DBG_ASSERT(pItem,"GetContextBmp:Item not found");
    return pItem->GetBitmap2( _eMode );
}

const Image& SvTreeListBox::GetCollapsedEntryBmp( SvLBoxEntry* pEntry, BmpColorMode _eMode ) const
{
    DBG_CHKTHIS(SvTreeListBox,0);
    DBG_ASSERT(pEntry,"Entry?");
    SvLBoxContextBmp* pItem = (SvLBoxContextBmp*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
    DBG_ASSERT(pItem,"GetContextBmp:Item not found");
    return pItem->GetBitmap1( _eMode );
}

IMPL_LINK_INLINE_START( SvTreeListBox, CheckButtonClick, SvLBoxButtonData *, pData )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pHdlEntry = pData->GetActEntry();
    CheckButtonHdl();
    return 0;
}
IMPL_LINK_INLINE_END( SvTreeListBox, CheckButtonClick, SvLBoxButtonData *, pData )

SvLBoxEntry* SvTreeListBox::InsertEntry( const XubString& aText,SvLBoxEntry* pParent,
                                     BOOL bChildsOnDemand, ULONG nPos, void* pUser,
                                     SvLBoxButtonKind eButtonKind )
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
    pEntry->EnableChildsOnDemand( bChildsOnDemand );

    // Add the HC versions of the default images
    SvLBoxContextBmp* pBmpItem = static_cast< SvLBoxContextBmp* >( pEntry->GetFirstItem( SV_ITEM_ID_LBOXCONTEXTBMP ) );
    if( pBmpItem )
    {
        pBmpItem->SetBitmap1( pImp->GetDefaultEntryColBmp( BMP_COLOR_HIGHCONTRAST ), BMP_COLOR_HIGHCONTRAST );
        pBmpItem->SetBitmap2( pImp->GetDefaultEntryExpBmp( BMP_COLOR_HIGHCONTRAST ), BMP_COLOR_HIGHCONTRAST );
    }

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
    SvLBoxEntry* pParent, BOOL bChildsOnDemand, ULONG nPos, void* pUser,
    SvLBoxButtonKind eButtonKind )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    nTreeFlags |= TREEFLAG_MANINS;

    aCurInsertedExpBmp = aExpEntryBmp;
    aCurInsertedColBmp = aCollEntryBmp;

    SvLBoxEntry* pEntry = CreateEntry();
    pEntry->SetUserData( pUser );
    InitEntry( pEntry, aText, aCollEntryBmp, aExpEntryBmp, eButtonKind );

    pEntry->EnableChildsOnDemand( bChildsOnDemand );

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
    pItem->SetText( pEntry, aStr );
    pItem->InitViewData( this, pEntry, 0 );
    GetModel()->InvalidateEntry( pEntry );
}

void SvTreeListBox::SetExpandedEntryBmp( SvLBoxEntry* pEntry, const Image& aBmp, BmpColorMode _eMode )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SvLBoxContextBmp* pItem = (SvLBoxContextBmp*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));

    DBG_ASSERT(pItem,"SetExpBmp:Item not found");
    pItem->SetBitmap2( aBmp, _eMode );

    GetModel()->InvalidateEntry( pEntry );
    SetEntryHeight( pEntry );
    Size aSize = aBmp.GetSizePixel();
    // #97680# ---------------
    short nWidth = pImp->UpdateContextBmpWidthVector( pEntry, (short)aSize.Width() );
    if( nWidth > nContextBmpWidthMax )
    {
        nContextBmpWidthMax = nWidth;
        SetTabs();
    }
}

void SvTreeListBox::SetCollapsedEntryBmp(SvLBoxEntry* pEntry,const Image& aBmp, BmpColorMode _eMode )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SvLBoxContextBmp* pItem = (SvLBoxContextBmp*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));

    DBG_ASSERT(pItem,"SetExpBmp:Item not found");
    pItem->SetBitmap1( aBmp, _eMode );

    GetModel()->InvalidateEntry( pEntry );
    SetEntryHeight( pEntry );
    Size aSize = aBmp.GetSizePixel();
    // #97680# -----------
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
        USHORT nFlags = pParent->GetFlags();
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
        USHORT nButtonFlags = pItem->GetButtonFlags();
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

// *********************************************************************
// *********************************************************************

//
//  TODO: Momentan werden die Daten so geklont, dass sie dem
//  Standard-TreeView-Format entsprechen. Hier sollte eigentlich
//  das Model als Referenz dienen. Dies fuehrt dazu, dass
//  SvLBoxEntry::Clone _nicht_ gerufen wird, sondern nur dessen
//  Basisklasse SvListEntry
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
        aCollEntryBmp = pBmpItem->GetBitmap1( BMP_COLOR_NORMAL );
        aExpEntryBmp  = pBmpItem->GetBitmap2( BMP_COLOR_NORMAL );
    }
    SvLBoxButton* pButtonItem = (SvLBoxButton*)(pSource->GetFirstItem(SV_ITEM_ID_LBOXBUTTON));
    if( pButtonItem )
        eButtonKind = pButtonItem->GetKind();
    SvLBoxEntry* pClone = CreateEntry();
    InitEntry( pClone, aStr, aCollEntryBmp, aExpEntryBmp, eButtonKind );
    pClone->SvListEntry::Clone( pSource );
    pClone->EnableChildsOnDemand( pSource->HasChildsOnDemand() );
    pClone->SetUserData( pSource->GetUserData() );

    if ( pBmpItem )
    {
        SvLBoxContextBmp* pCloneBitmap = static_cast< SvLBoxContextBmp* >( pClone->GetFirstItem( SV_ITEM_ID_LBOXCONTEXTBMP ) );
        if ( pCloneBitmap )
        {
            pCloneBitmap->SetBitmap1( pBmpItem->GetBitmap1( BMP_COLOR_HIGHCONTRAST ), BMP_COLOR_HIGHCONTRAST );
            pCloneBitmap->SetBitmap2( pBmpItem->GetBitmap2( BMP_COLOR_HIGHCONTRAST ), BMP_COLOR_HIGHCONTRAST );
        }
    }

    return pClone;
}

// *********************************************************************
// *********************************************************************


void SvTreeListBox::ShowExpandBitmapOnCursor( BOOL bYes )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if( bYes )
        aContextBmpMode = SVLISTENTRYFLAG_FOCUSED;
    else
        aContextBmpMode = SVLISTENTRYFLAG_EXPANDED;
}

void SvTreeListBox::SetIndent( short nNewIndent )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    nIndent = nNewIndent;
    SetTabs();
    if( IsUpdateMode() )
        Invalidate();
}

const Image& SvTreeListBox::GetDefaultExpandedEntryBmp( BmpColorMode _eMode ) const
{
    return pImp->GetDefaultEntryExpBmp( _eMode );
}

const Image& SvTreeListBox::GetDefaultCollapsedEntryBmp( BmpColorMode _eMode ) const
{
    return pImp->GetDefaultEntryColBmp( _eMode );
}

void SvTreeListBox::SetDefaultExpandedEntryBmp( const Image& aBmp, BmpColorMode _eMode )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    Size aSize = aBmp.GetSizePixel();
    if( aSize.Width() > nContextBmpWidthMax )
        nContextBmpWidthMax = (short)aSize.Width();
    SetTabs();

    pImp->SetDefaultEntryExpBmp( aBmp, _eMode );
}

void SvTreeListBox::SetDefaultCollapsedEntryBmp( const Image& aBmp, BmpColorMode _eMode )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    Size aSize = aBmp.GetSizePixel();
    if( aSize.Width() > nContextBmpWidthMax )
        nContextBmpWidthMax = (short)aSize.Width();
    SetTabs();

    pImp->SetDefaultEntryColBmp( aBmp, _eMode );
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

const Image& SvTreeListBox::GetDefaultExpandedNodeImage( BmpColorMode _eMode )
{
    return SvImpLBox::GetDefaultExpandedNodeImage( _eMode );
}

const Image& SvTreeListBox::GetDefaultCollapsedNodeImage( BmpColorMode _eMode )
{
    return SvImpLBox::GetDefaultCollapsedNodeImage( _eMode );
}

void SvTreeListBox::SetNodeBitmaps( const Image& rCollapsedNodeBmp, const Image& rExpandedNodeBmp, BmpColorMode _eMode )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SetExpandedNodeBmp( rExpandedNodeBmp, _eMode );
    SetCollapsedNodeBmp( rCollapsedNodeBmp, _eMode );
    SetTabs();
}

void SvTreeListBox::SetDontKnowNodeBitmap( const Image& rDontKnowBmp, BmpColorMode _eMode )
{
    pImp->SetDontKnowNodeBmp( rDontKnowBmp, _eMode );
}

BOOL SvTreeListBox::EditingEntry( SvLBoxEntry*, Selection& )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    return TRUE;
}

BOOL SvTreeListBox::EditedEntry( SvLBoxEntry* /*pEntry*/,const XubString& /*rNewText*/)
{
    DBG_CHKTHIS(SvTreeListBox,0);
    return TRUE;
}

void SvTreeListBox::EnableInplaceEditing( BOOL bOn )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SvLBox::EnableInplaceEditing( bOn );
}

void SvTreeListBox::KeyInput( const KeyEvent& rKEvt )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    // unter OS/2 bekommen wir auch beim Editieren Key-Up/Down
    if( IsEditingActive() )
        return;

    nImpFlags |= SVLBOX_IS_TRAVELSELECT;

#ifdef OVDEBUG
    USHORT nCode = rKEvt.GetKeyCode().GetCode();
    switch ( nCode )
    {
        case KEY_F1:
        {
            SvLBoxEntry* pEntry = First();
            pEntry = NextVisible( pEntry );
            SetEntryText( pEntry, "SetEntryText" );
            Sound::Beep();
        }
        break;
    }
#endif

    if( !pImp->KeyInput( rKEvt ) )
        SvLBox::KeyInput( rKEvt );

    nImpFlags &= ~SVLBOX_IS_TRAVELSELECT;
}

void SvTreeListBox::RequestingChilds( SvLBoxEntry* pParent )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if( !pParent->HasChilds() )
        InsertEntry( String::CreateFromAscii("<dummy>"), pParent, FALSE, LIST_APPEND );
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
    pImp->pCursor = 0; //sonst Absturz beim Inplace-Editieren im GetFocus
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
//  if( IsUpdateMode() )
//      Invalidate();
}

void SvTreeListBox::ShowTargetEmphasis( SvLBoxEntry* pEntry, BOOL /* bShow  */ )
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
        // das Fenster nach oben verschieben
        nDeltaEntries *= -1;
        long nVis = pImp->aVerSBar.GetVisibleSize();
        long nTemp = nThumb + nVis;
        if( nDeltaEntries > (nMax - nTemp) )
            nDeltaEntries = (short)(nMax - nTemp);
        pImp->PageDown( (USHORT)nDeltaEntries );
    }
    else
    {
        if( nDeltaEntries > nThumb )
            nDeltaEntries = (short)nThumb;
        pImp->PageUp( (USHORT)nDeltaEntries );
    }
    pImp->SyncVerThumb();
    NotifyEndScroll();
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
    aSizeLogic = Size(GetTextWidth('X'), GetTextHeight());
    if( GetEntryHeight() > aSizeLogic.Height() )
        nOffset = ( GetEntryHeight() - (short)aSizeLogic.Height()) / 2;
    return nOffset;
}

void SvTreeListBox::SetEntryHeight( SvLBoxEntry* pEntry )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    short nHeight, nHeightMax=0;
    USHORT nCount = pEntry->ItemCount();
    USHORT nCur = 0;
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

void SvTreeListBox::SetEntryHeight( short nHeight, BOOL bAlways )
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

BOOL SvTreeListBox::Expand( SvLBoxEntry* pParent )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pHdlEntry = pParent;
    BOOL bExpanded = FALSE;
    USHORT nFlags;

    if( pParent->HasChildsOnDemand() )
        RequestingChilds( pParent );
    if( pParent->HasChilds() )
    {
        nImpFlags |= SVLBOX_IS_EXPANDING;
        if( ExpandingHdl() )
        {
            bExpanded = TRUE;
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
        GetModel()->InvalidateEntry( pParent ); // neu zeichnen
    }

    // --> OD 2009-04-01 #i92103#
    if ( bExpanded )
    {
        pImp->CallEventListeners( VCLEVENT_ITEM_EXPANDED, pParent );
    }
    // <--

    return bExpanded;
}

BOOL SvTreeListBox::Collapse( SvLBoxEntry* pParent )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    nImpFlags &= ~SVLBOX_IS_EXPANDING;
    pHdlEntry = pParent;
    BOOL bCollapsed = FALSE;

    if( ExpandingHdl() )
    {
        bCollapsed = TRUE;
        pImp->CollapsingEntry( pParent );
        SvListView::Collapse( pParent );
        pImp->EntryCollapsed( pParent );
        pHdlEntry = pParent;
        ExpandedHdl();
    }

    // --> OD 2009-04-01 #i92103#
    if ( bCollapsed )
    {
        pImp->CallEventListeners( VCLEVENT_ITEM_COLLAPSED, pParent );
    }
    // <--

    return bCollapsed;
}

BOOL SvTreeListBox::Select( SvLBoxEntry* pEntry, BOOL bSelect )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    DBG_ASSERT(pEntry,"Select: Null-Ptr");
    BOOL bRetVal = SvListView::Select( pEntry, bSelect );
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

ULONG SvTreeListBox::SelectChilds( SvLBoxEntry* pParent, BOOL bSelect )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->DestroyAnchor();
    ULONG nRet = 0;
    if( !pParent->HasChilds() )
        return 0;
    USHORT nRefDepth = pModel->GetDepth( pParent );
    SvLBoxEntry* pChild = FirstChild( pParent );
    do {
        nRet++;
        Select( pChild, bSelect );
        pChild = Next( pChild );
    } while( pChild && pModel->GetDepth( pChild ) > nRefDepth );
    return nRet;
}

void SvTreeListBox::SelectAll( BOOL bSelect, BOOL )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->SelAllDestrAnch(
        bSelect,
        TRUE,       // Anker loeschen,
        TRUE );     // auch bei SINGLE_SELECTION den Cursor deselektieren
}

void SvTreeListBox::ModelHasInsertedTree( SvListEntry* pEntry )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    USHORT nRefDepth = pModel->GetDepth( (SvLBoxEntry*)pEntry );
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
                                        ULONG /* nChildPos */ )
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

void SvTreeListBox::SetCollapsedNodeBmp( const Image& rBmp, BmpColorMode _eMode )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    AdjustEntryHeight( rBmp );
    pImp->SetCollapsedNodeBmp( rBmp, _eMode );
}

void SvTreeListBox::SetExpandedNodeBmp( const Image& rBmp, BmpColorMode _eMode )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    AdjustEntryHeight( rBmp );
    pImp->SetExpandedNodeBmp( rBmp, _eMode );
}


void SvTreeListBox::SetFont( const Font& rFont )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    Font aTempFont( rFont );
    aTempFont.SetTransparent( TRUE );
    Control::SetFont( aTempFont );
    AdjustEntryHeight( aTempFont );
    // immer Invalidieren, sonst fallen wir
    // bei SetEntryHeight auf die Nase
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


void SvTreeListBox::SetUpdateMode( BOOL bUpdate )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->SetUpdateMode( bUpdate );
}

void SvTreeListBox::SetUpdateModeFast( BOOL bUpdate )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->SetUpdateModeFast( bUpdate );
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

void SvTreeListBox::SetCursor( SvLBoxEntry* pEntry, BOOL bForceNoSelect )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->SetCursor(pEntry, bForceNoSelect);
}

void SvTreeListBox::SetCurEntry( SvLBoxEntry* pEntry )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->SetCurEntry( pEntry );
}

Image SvTreeListBox::GetCollapsedNodeBmp( BmpColorMode _eMode ) const
{
    return pImp->GetCollapsedNodeBmp( _eMode );
}

Image SvTreeListBox::GetExpandedNodeBmp( BmpColorMode _eMode ) const
{
    return pImp->GetExpandedNodeBmp( _eMode );
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

void SvTreeListBox::MakeVisible( SvLBoxEntry* pEntry, BOOL bMoveToTop )
{
    pImp->MakeVisible( pEntry, bMoveToTop );
}

void SvTreeListBox::ModelHasEntryInvalidated( SvListEntry* pEntry )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    // die einzelnen Items des Entries reinitialisieren
    SvLBox::ModelHasEntryInvalidated( pEntry );
    // repainten
    pImp->InvalidateEntry( (SvLBoxEntry*)pEntry );
}

void SvTreeListBox::EditItemText( SvLBoxEntry* pEntry, SvLBoxString* pItem,
    const Selection& rSelection )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    DBG_ASSERT(pEntry&&pItem,"EditItemText: Bad params");
    if( IsSelected( pEntry ))
    {
        pImp->ShowCursor( FALSE );
        SvListView::Select( pEntry, FALSE );
        PaintEntry( pEntry );
        SvListView::Select( pEntry, TRUE );
        pImp->ShowCursor( TRUE );
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
    USHORT nPos = aTabs.GetPos( pTab );
    if( nPos+1 < aTabs.Count() )
    {
        SvLBoxTab* pRightTab = (SvLBoxTab*)aTabs.GetObject( nPos + 1 );
        long nRight = GetTabPos( pEntry, pRightTab );
        if( nRight <= nOutputWidth )
            aSize.Width() = nRight - aPos.X();
    }
    Point aOrigin( GetMapMode().GetOrigin() );
    aPos += aOrigin; // in Win-Koord umrechnen
    aSize.Width() -= aOrigin.X();
    Rectangle aRect( aPos, aSize );
#ifdef OS2
    // Platz lassen fuer WB_BORDER
    aRect.Left() -= 2;
    aRect.Top() -= 3;
    aRect.Bottom() += 3;
#endif
    EditText( pItem->GetText(), aRect, rSelection );
}

void SvTreeListBox::CancelEditing()
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SvLBox::CancelTextEditing();
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
        USHORT nCount = pEntry->ItemCount();
        for( USHORT i = 0 ; i < nCount ; i++ )
        {
            SvLBoxItem* pTmpItem = pEntry->GetItem( i );
            if( pTmpItem->IsA() != SV_ITEM_ID_LBOXSTRING )
                continue;

            SvLBoxTab* pTab = GetTab( pEntry, pTmpItem );
            long nTabPos = pTab->GetPos();
            long nNextTabPos = -1;
            if( i < nCount - 1 )
            {
                SvLBoxItem* pNextItem = pEntry->GetItem( i + 1 );
                SvLBoxTab* pNextTab = GetTab( pEntry, pNextItem );
                nNextTabPos = pNextTab->GetPos();
            }

            if( pTab && pTab->IsEditable() )
            {
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
            SelectAll( FALSE );
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
        Point aPos = GetEntryPosition( pEdEntry );
        if( EditedEntry( pEdEntry, rStr ) )
        {
            ((SvLBoxString*)pEdItem)->SetText( pEdEntry, rStr );
            pModel->InvalidateEntry( pEdEntry );
        }
        //if( GetSelectionMode() == SINGLE_SELECTION )
        //{
        if( GetSelectionCount() == 0 )
            Select( pEdEntry );
        if( GetSelectionMode() == MULTIPLE_SELECTION && !GetCurEntry() )
            SetCurEntry( pEdEntry );
        //}
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
            SelectAll( FALSE );
            EditItemText( pEntry, (SvLBoxString*)pItem, aSel );
        }
    }
}



SvLBoxEntry* SvTreeListBox::GetDropTarget( const Point& rPos )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    // Scrollen
    if( rPos.Y() < 12 )
    {
        SvLBox::ImplShowTargetEmphasis( SvLBox::pTargetEntry, FALSE );
        ScrollOutputArea( +1 );
    }
    else
    {
        Size aSize( pImp->GetOutputSize() );
        if( rPos.Y() > aSize.Height() - 12 )
        {
            SvLBox::ImplShowTargetEmphasis( SvLBox::pTargetEntry, FALSE );
            ScrollOutputArea( -1 );
        }
    }

    SvLBoxEntry* pTarget = pImp->GetEntry( rPos );
    // bei Droppen in leere Flaeche -> den letzten Eintrag nehmen
    if( !pTarget )
        return (SvLBoxEntry*)LastVisible();
    else if( (GetDragDropMode() & SV_DRAGDROP_ENABLE_TOP) &&
             pTarget == First() && rPos.Y() < 6 )
        return 0;

    return pTarget;
}


SvLBoxEntry* SvTreeListBox::GetEntry( const Point& rPos, BOOL bHit ) const
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
#ifdef OS2
    nWindowStyle |= WB_VSCROLL;
#endif
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
    //  pImp->InvalidateEntry( pEntry );
    }
}


long SvTreeListBox::PaintEntry(SvLBoxEntry* pEntry,long nLine,USHORT nTabFlags)
{
    return PaintEntry1(pEntry,nLine,nTabFlags);
}

#define SV_TAB_BORDER 8

long SvTreeListBox::PaintEntry1(SvLBoxEntry* pEntry,long nLine,USHORT nTabFlags,
    BOOL bHasClipRegion )
{
    DBG_CHKTHIS(SvTreeListBox,0);

    Rectangle aRect; // multi purpose

    BOOL bHorSBar = pImp->HasHorScrollBar();
    PreparePaint( pEntry );

    // #97680# ------------------
    pImp->UpdateContextBmpWidthMax( pEntry );

    if( nTreeFlags & TREEFLAG_RECALCTABS )
        SetTabs();

    short nTempEntryHeight = GetEntryHeight();
    long nWidth = pImp->GetOutputSize().Width();

    // wurde innerhalb des PreparePaints die horizontale ScrollBar
    // angeschaltet? Wenn ja, muss die ClipRegion neu gesetzt werden
    if( !bHorSBar && pImp->HasHorScrollBar() )
        SetClipRegion( Region(pImp->GetClipRegionRect()) );

    Point aEntryPos( GetMapMode().GetOrigin() );
    aEntryPos.X() *= -1; // Umrechnung Dokumentkoord.
    long nMaxRight = nWidth + aEntryPos.X() - 1;

    Color aBackupTextColor( GetTextColor() );
    Font aBackupFont( GetFont() );
    Color aBackupColor = GetFillColor();

    bool bCurFontIsSel = false;
    BOOL bInUse = pEntry->HasInUseEmphasis();
    // wenn eine ClipRegion von aussen gesetzt wird, dann
    // diese nicht zuruecksetzen
    const WinBits nWindowStyle = GetStyle();
    const BOOL bResetClipRegion = !bHasClipRegion;
    const BOOL bHideSelection = ((nWindowStyle & WB_HIDESELECTION) && !HasFocus())!=0;
    const StyleSettings& rSettings = GetSettings().GetStyleSettings();

    Font aHighlightFont( GetFont() );
    const Color aHighlightTextColor( rSettings.GetHighlightTextColor() );
    aHighlightFont.SetColor( aHighlightTextColor );

    Size aRectSize( 0, nTempEntryHeight );

    if( !bHasClipRegion && nWindowStyle & WB_HSCROLL )
    {
        SetClipRegion( Region(pImp->GetClipRegionRect()) );
        bHasClipRegion = TRUE;
    }

    SvViewDataEntry* pViewDataEntry = GetViewDataEntry( pEntry );

    USHORT nTabCount = aTabs.Count();
    USHORT nItemCount = pEntry->ItemCount();
    USHORT nCurTab = 0;
    USHORT nCurItem = 0;

    while( nCurTab < nTabCount && nCurItem < nItemCount )
    {
        SvLBoxTab* pTab = (SvLBoxTab*)aTabs.GetObject( nCurTab );
        USHORT nNextTab = nCurTab + 1;
        SvLBoxTab* pNextTab = nNextTab < nTabCount ? (SvLBoxTab*)aTabs.GetObject(nNextTab) : 0;
        SvLBoxItem* pItem = nCurItem < nItemCount ? pEntry->GetItem(nCurItem) : 0;

        USHORT nFlags = pTab->nFlags;
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
            //verhindern, das rechter Rand von der Tabtrennung abgeschnitten wird
            nX = nTabPos + pTab->CalcOffset(aSize.Width(), (nNextTabPos-SV_TAB_BORDER-1) -nTabPos);
        else
            nX = nTabPos + pTab->CalcOffset(aSize.Width(), nNextTabPos-nTabPos);

        if( nFlags & nTabFlags )
        {
            if( !bHasClipRegion && nX + aSize.Width() >= nMaxRight )
            {
                SetClipRegion( Region(pImp->GetClipRegionRect()) );
                bHasClipRegion = TRUE;
            }
            aEntryPos.X() = nX;
            aEntryPos.Y() = nLine;

            // Hintergrund-Muster & Farbe bestimmen

            Wallpaper aWallpaper = GetBackground();

            int bSelTab = nFlags & SV_LBOXTAB_SHOW_SELECTION;
            USHORT nItemType = pItem->IsA();

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
                        SetFont( aHighlightFont );
                        bCurFontIsSel = true;
                    }
                }
                aWallpaper.SetColor( aNewWallColor );
            }
            else  // keine Selektion
            {
                if( bInUse && nItemType == SV_ITEM_ID_LBOXCONTEXTBMP )
                    aWallpaper.SetColor( rSettings.GetFieldColor() );
                else if( bCurFontIsSel )
                {
                    bCurFontIsSel = false;
                    SetTextColor( aBackupTextColor );
                    SetFont( aBackupFont );
                }
            }

            // Hintergrund zeichnen
            if( !(nTreeFlags & TREEFLAG_USESEL))
            {
                // nur den Bereich zeichnen, den das Item einnimmt
                aRectSize.Width() = aSize.Width();
                aRect.SetPos( aEntryPos );
                aRect.SetSize( aRectSize );
            }
            else
            {
                // vom aktuellen bis zum naechsten Tab zeichnen
                if( nCurTab != 0 )
                    aRect.Left() = nTabPos;
                else
                    // beim nullten Tab immer ab Spalte 0 zeichnen
                    // (sonst Probleme bei Tabs mit Zentrierung)
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
            // bei anwenderdefinierter Selektion, die bei einer Tabposition
            // groesser 0 beginnt den Hintergrund des 0.ten Items nicht
            // fuellen, da sonst z.B. TablistBoxen mit Linien nicht
            // realisiert werden koennen.
            if( !(nCurTab==0 && (nTreeFlags & TREEFLAG_USESEL) && nFirstSelTab) )
            {
                SetFillColor( aWallpaper.GetColor() );
                // Bei kleinen hor. Resizes tritt dieser Fall auf
                if( aRect.Left() < aRect.Right() )
                    DrawRect( aRect );
            }
            // Item zeichnen
            // vertikal zentrieren
            aEntryPos.Y() += ( nTempEntryHeight - aSize.Height() ) / 2;
            pItem->Paint( aEntryPos, *this, pViewDataEntry->GetFlags(), pEntry );

            // Trennungslinie zwischen Tabs
            if( pNextTab && pItem->IsA() == SV_ITEM_ID_LBOXSTRING &&
                // nicht am rechten Fensterrand!
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
        // Cursor-Emphasis
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
        SetFont( aBackupFont );
    }

    USHORT nFirstDynTabPos;
    SvLBoxTab* pFirstDynamicTab = GetFirstDynamicTab( nFirstDynTabPos );
    long nDynTabPos = GetTabPos( pEntry, pFirstDynamicTab );
    nDynTabPos += pImp->nNodeBmpTabDistance;
    nDynTabPos += pImp->nNodeBmpWidth / 2;
    nDynTabPos += 4; // 4 Pixel Reserve, damit die Node-Bitmap
                     // nicht zu nah am naechsten Tab steht

    if( (!(pEntry->GetFlags() & SV_ENTRYFLAG_NO_NODEBMP)) &&
        (nWindowStyle & WB_HASBUTTONS) && pFirstDynamicTab &&
        ( pEntry->HasChilds() || pEntry->HasChildsOnDemand() ) )
    {
        // ersten festen Tab suchen, und pruefen ob die Node-Bitmap
        // in ihn hineinragt
        USHORT nNextTab = nFirstDynTabPos;
        SvLBoxTab* pNextTab;
        do
        {
            nNextTab++;
            pNextTab = nNextTab < nTabCount ? (SvLBoxTab*)aTabs.GetObject(nNextTab) : 0;
        } while( pNextTab && pNextTab->IsDynamic() );

        if( !pNextTab || (GetTabPos( pEntry, pNextTab ) > nDynTabPos) )
        {
            if((nWindowStyle & WB_HASBUTTONSATROOT) || pModel->GetDepth(pEntry) > 0)
            {
                Point aPos( GetTabPos(pEntry,pFirstDynamicTab), nLine );
                aPos.X() += pImp->nNodeBmpTabDistance;

                const Image* pImg = 0;
                BmpColorMode eBitmapMode = BMP_COLOR_NORMAL;
                if ( GetSettings().GetStyleSettings().GetHighContrastMode() )
                    eBitmapMode = BMP_COLOR_HIGHCONTRAST;

                if( IsExpanded(pEntry) )
                    pImg = &pImp->GetExpandedNodeBmp( eBitmapMode );
                else
                {
                    if( (!pEntry->HasChilds()) && pEntry->HasChildsOnDemand() &&
                        (!(pEntry->GetFlags() & SV_ENTRYFLAG_HAD_CHILDREN)) &&
                        pImp->GetDontKnowNodeBmp().GetSizePixel().Width() )
                        pImg = &pImp->GetDontKnowNodeBmp( eBitmapMode );
                    else
                        pImg = &pImp->GetCollapsedNodeBmp( eBitmapMode );
                }
                aPos.Y() += (nTempEntryHeight - pImg->GetSizePixel().Height()) / 2;

                USHORT nStyle = 0;
                if ( !IsEnabled() )
                    nStyle |= IMAGE_DRAW_DISABLE;

                //native
                BOOL bNativeOK = FALSE;
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
                        if( (!pEntry->HasChilds()) && pEntry->HasChildsOnDemand() &&
                            (!(pEntry->GetFlags() & SV_ENTRYFLAG_HAD_CHILDREN)) &&
                            pImp->GetDontKnowNodeBmp().GetSizePixel().Width() )
                            aControlValue.setTristateVal( BUTTONVALUE_DONTKNOW );//dont know
                        else
                            aControlValue.setTristateVal( BUTTONVALUE_OFF );//collapsed node
                    }

                    bNativeOK = DrawNativeControl( CTRL_LISTNODE, PART_ENTIRE_CONTROL,
                                            aCtrlRegion, nState, aControlValue, rtl::OUString() );
                }

                if( !bNativeOK) {
                //non native
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

    USHORT nCurTab;
    SvLBoxTab* pTab = GetFirstTab( SV_LBOXTAB_SHOW_SELECTION, nCurTab );
    long nTabPos = 0;
    if( pTab )
        nTabPos = GetTabPos( pEntry, pTab );
    long nNextTabPos;
    if( pTab && nCurTab < aTabs.Count() - 1 )
    {
        SvLBoxTab* pNextTab = (SvLBoxTab*)aTabs.GetObject( nCurTab + 1 );
        nNextTabPos = GetTabPos( pEntry, pNextTab );
    }
    else
    {
        nNextTabPos = nRealWidth;
        if( nTabPos > nRealWidth )
            nNextTabPos += 50;
    }

    BOOL bUserSelection = (BOOL)( nTreeFlags & TREEFLAG_USESEL ) != 0;
    if( !bUserSelection )
    {
        if( pTab && nCurTab < pEntry->ItemCount() )
        {
            SvLBoxItem* pItem = pEntry->GetItem( nCurTab );
            aSize.Width() = pItem->GetSize( this, pEntry ).Width();
            if( !aSize.Width() )
                aSize.Width() = 15;
            long nX = nTabPos; //GetTabPos( pEntry, pTab );
            // Ausrichtung
            nX += pTab->CalcOffset( aSize.Width(), nNextTabPos - nTabPos );
            aRect.Left() = nX;
            // damit erster & letzter Buchstabe nicht angeknabbert werden
            aRect.SetSize( aSize );
            if( aRect.Left() > 0 )
                aRect.Left()--;
            aRect.Right()++;
        }
    }
    else
    {
        // wenn erster SelTab != 0, dann muessen wir auch rechnen
        if( nFocusWidth == -1 || nFirstSelTab )
        {
            USHORT nLastTab;
            SvLBoxTab* pLastTab = GetLastTab(SV_LBOXTAB_SHOW_SELECTION,nLastTab);
            nLastTab++;
            if( nLastTab < aTabs.Count() ) // gibts noch einen ?
                pLastTab = (SvLBoxTab*)aTabs.GetObject( nLastTab );
            else
                pLastTab = 0;  // ueber gesamte Breite selektieren
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
                    aSize.Width() += pTab->GetPos(); // Tab0 immer ab ganz links
            }
        }
        // wenn Sel. beim nullten Tab anfaengt, dann ab Spalte 0 sel. zeichnen
        if( nCurTab != 0 )
        {
            aRect.Left() = nTabPos;
            aSize.Width() -= nTabPos;
        }
        aRect.SetSize( aSize );
    }
    // rechten Rand anpassen wg. Clipping
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
        USHORT nDepth = pModel->GetDepth( pEntry );
        nDepth = nDepth * (USHORT)nIndent;
        nPos += (long)nDepth;
    }
    return nPos;
}

SvLBoxItem* SvTreeListBox::GetItem_Impl( SvLBoxEntry* pEntry, long nX,
    SvLBoxTab** ppTab, USHORT nEmptyWidth )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SvLBoxItem* pItemClicked = 0;
    USHORT nTabCount = aTabs.Count();
    USHORT nItemCount = pEntry->ItemCount();
    SvLBoxTab* pTab = (SvLBoxTab*)aTabs.GetObject(0);
    SvLBoxItem* pItem = pEntry->GetItem(0);
    USHORT nNextItem = 1;
    nX -= GetMapMode().GetOrigin().X();
    long nRealWidth = pImp->GetOutputSize().Width();
    nRealWidth -= GetMapMode().GetOrigin().X();

    while( 1 )
    {
        SvLBoxTab* pNextTab=nNextItem<nTabCount ? (SvLBoxTab*)aTabs.GetObject(nNextItem) : 0;
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
        pTab = (SvLBoxTab*)aTabs.GetObject( nNextItem );
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

SvLBoxItem* SvTreeListBox::GetFirstDynamicItem( SvLBoxEntry* pEntry )
{
    DBG_CHKTHIS(SvTreeListBox,0);

    SvLBoxTab* pTab = (SvLBoxTab*)aTabs.GetObject(0);
    SvLBoxItem* pItem = pEntry->GetItem(0);
    USHORT nTabCount = aTabs.Count();

    USHORT nNext = 1;
    while ( !pTab->IsDynamic() && nNext < nTabCount )
    {
        pItem = pEntry->GetItem( nNext );
        pTab = (SvLBoxTab*)aTabs.GetObject( nNext );
        nNext++;
    }
    return pItem;
}

void SvTreeListBox::AddTab(long nTabPos,USHORT nFlags,void* pUserData )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    nFocusWidth = -1;
    SvLBoxTab* pTab = new SvLBoxTab( nTabPos, nFlags );
    pTab->SetUserData( pUserData );
    aTabs.Insert( pTab, aTabs.Count() );
    if( nTreeFlags & TREEFLAG_USESEL )
    {
        USHORT nPos = aTabs.Count() - 1;
        if( nPos >= nFirstSelTab && nPos <= nLastSelTab )
            pTab->nFlags |= SV_LBOXTAB_SHOW_SELECTION;
        else
            // String-Items werden normalerweise immer selektiert
            // deshalb explizit ausschalten
            pTab->nFlags &= ~SV_LBOXTAB_SHOW_SELECTION;
    }
}



SvLBoxTab* SvTreeListBox::GetFirstDynamicTab( USHORT& rPos ) const
{
    DBG_CHKTHIS(SvTreeListBox,0);
    USHORT nCurTab = 0;
    USHORT nTabCount = aTabs.Count();
    while( nCurTab < nTabCount )
    {
        SvLBoxTab* pTab = (SvLBoxTab*)aTabs.GetObject(nCurTab);
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
    USHORT nDummy;
    return GetFirstDynamicTab( nDummy );
}

SvLBoxTab* SvTreeListBox::GetTab( SvLBoxEntry* pEntry, SvLBoxItem* pItem) const
{
    DBG_CHKTHIS(SvTreeListBox,0);
    USHORT nPos = pEntry->GetPos( pItem );
    return (SvLBoxTab*)aTabs.GetObject( nPos );
}

void SvTreeListBox::ClearTabList()
{
    DBG_CHKTHIS(SvTreeListBox,0);
    USHORT nTabCount = aTabs.Count();
    while( nTabCount )
    {
        nTabCount--;
        SvLBoxTab* pDelTab = (SvLBoxTab*)aTabs.GetObject( nTabCount );
        delete pDelTab;
    }
    aTabs.Remove(0,aTabs.Count());
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

void SvTreeListBox::Invalidate( USHORT nInvalidateFlags )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if( nFocusWidth == -1 )
        // damit Control nicht nach dem Paint ein falsches FocusRect anzeigt
        pImp->RecalcFocusRect();
    NotifyInvalidating();
    SvLBox::Invalidate( nInvalidateFlags );
    pImp->Invalidate();
}

void SvTreeListBox::Invalidate( const Rectangle& rRect, USHORT nInvalidateFlags )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if( nFocusWidth == -1 )
        // damit Control nicht nach dem Paint ein falsches FocusRect anzeigt
        pImp->RecalcFocusRect();
    NotifyInvalidating();
    SvLBox::Invalidate( rRect, nInvalidateFlags );
}


void SvTreeListBox::SetHighlightRange( USHORT nStart, USHORT nEnd)
{
    DBG_CHKTHIS(SvTreeListBox,0);

    USHORT nTemp;
    nTreeFlags |= TREEFLAG_USESEL;
    if( nStart > nEnd )
    {
        nTemp = nStart;
        nStart = nEnd;
        nEnd = nTemp;
    }
    // alle Tabs markieren, die im Bereich liegen
    nTreeFlags |= TREEFLAG_RECALCTABS;
    nFirstSelTab = nStart;
    nLastSelTab = nEnd;
    pImp->RecalcFocusRect();
}

void SvTreeListBox::RemoveHighlightRange()
{
    DBG_CHKTHIS(SvTreeListBox,0);
    nTreeFlags &= (~TREEFLAG_USESEL);
    if( IsUpdateMode() )
        Invalidate();
}

ULONG SvTreeListBox::GetAscInsertionPos(SvLBoxEntry*,SvLBoxEntry*)
{
    return LIST_APPEND;
}

ULONG SvTreeListBox::GetDescInsertionPos(SvLBoxEntry*,SvLBoxEntry*)
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


void SvTreeListBox::RemoveParentKeepChilds( SvLBoxEntry* pParent )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    DBG_ASSERT(pParent,"RemoveParentKeepChilds:No Parent");
    SvLBoxEntry* pNewParent = GetParent( pParent );
    if( pParent->HasChilds())
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

SvLBoxTab* SvTreeListBox::GetFirstTab( USHORT nFlagMask, USHORT& rPos )
{
    USHORT nTabCount = aTabs.Count();
    for( USHORT nPos = 0; nPos < nTabCount; nPos++ )
    {
        SvLBoxTab* pTab = (SvLBoxTab*)aTabs.GetObject( nPos );
        if( (pTab->nFlags & nFlagMask) )
        {
            rPos = nPos;
            return pTab;
        }
    }
    rPos = 0xffff;
    return 0;
}

SvLBoxTab* SvTreeListBox::GetLastTab( USHORT nFlagMask, USHORT& rTabPos )
{
    short nTabCount = (short)aTabs.Count();
    if( nTabCount )
    {
        for( short nPos = nTabCount-1; nPos >= 0; nPos-- )
        {
            SvLBoxTab* pTab = (SvLBoxTab*)aTabs.GetObject( (USHORT)nPos );
            if( (pTab->nFlags & nFlagMask) )
            {
                rTabPos = (USHORT)nPos;
                return pTab;
            }
        }
    }
    rTabPos = 0xffff;
    return 0;
}

void SvTreeListBox::SetAddMode( BOOL bAdd )
{
    pImp->SetAddMode( bAdd );
}

BOOL SvTreeListBox::IsAddMode() const
{
    return pImp->IsAddMode();
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
    // #102891# ----------------
    pImp->UpdateIntlWrapper();
    return pImp->pIntlWrapper->getCaseCollator()->compareString( aLeft, aRight );
}

void SvTreeListBox::ModelNotification( USHORT nActionId, SvListEntry* pEntry1,
                        SvListEntry* pEntry2, ULONG nPos )
{
    if( nActionId == LISTACTION_CLEARING )
        CancelTextEditing();

    SvLBox::ModelNotification( nActionId, pEntry1, pEntry2, nPos );
    switch( nActionId )
    {
        case LISTACTION_INSERTED:
        {
            SvLBoxEntry* pEntry( dynamic_cast< SvLBoxEntry* >( pEntry1 ) );
            ENSURE_OR_BREAK( pEntry, "SvTreeListBox::ModelNotification: invalid entry!" );
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
            SetUpdateMode( FALSE );
            break;

        case LISTACTION_RESORTED:
            // nach Sortierung den ersten Eintrag anzeigen, dabei die
            // Selektion erhalten.
            MakeVisible( (SvLBoxEntry*)pModel->First(), TRUE );
            SetUpdateMode( TRUE );
            break;

        case LISTACTION_CLEARED:
            if( IsUpdateMode() )
                Update();
            break;
    }
}

// bei Aenderungen SetTabs beruecksichtigen
long SvTreeListBox::GetTextOffset() const
{
    DBG_CHKTHIS(SvTreeListBox,0);
    const WinBits nWindowStyle = GetStyle();
    BOOL bHasButtons = (nWindowStyle & WB_HASBUTTONS)!=0;
    BOOL bHasButtonsAtRoot = (nWindowStyle & (WB_HASLINESATROOT |
                                              WB_HASBUTTONSATROOT))!=0;
    long nStartPos = TAB_STARTPOS;
    long nNodeWidthPixel = GetExpandedNodeBmp().GetSizePixel().Width();

    long nCheckWidth = 0;
    if( nTreeFlags & TREEFLAG_CHKBTN )
        nCheckWidth = pCheckButtonData->aBmps[0].GetSizePixel().Width();
    long nCheckWidthDIV2 = nCheckWidth / 2;

    long nContextWidth = nContextBmpWidthMax;
    long nContextWidthDIV2 = nContextWidth / 2;

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
            nStartPos += nContextWidthDIV2;  // wg. Zentrierung
            nStartPos += nContextWidthDIV2;  // rechter Rand der Context-Bmp
            if( nContextBmpWidthMax )
                nStartPos += 5; // Abstand Context-Bmp - Text
            break;

        case NODE_BUTTONS :
            if( bHasButtonsAtRoot )
                nStartPos += ( nIndent + (nNodeWidthPixel/2) );
            else
                nStartPos += nContextWidthDIV2;
            nStartPos += nContextWidthDIV2;  // rechter Rand der Context-Bmp
            if( nContextBmpWidthMax )
                nStartPos += 5; // Abstand Context-Bmp - Text
            break;

        case NODE_AND_CHECK_BUTTONS :
            if( bHasButtonsAtRoot )
                nStartPos += ( nIndent + nNodeWidthPixel );
            else
                nStartPos += nCheckWidthDIV2;
            nStartPos += nCheckWidthDIV2;  // rechter Rand des CheckButtons
            nStartPos += 3;  // Abstand CheckButton Context-Bmp
            nStartPos += nContextWidthDIV2;  // Mitte der Context-Bmp
            nStartPos += nContextWidthDIV2;  // rechter Rand der Context-Bmp
            // Abstand setzen nur wenn Bitmaps da
            if( nContextBmpWidthMax )
                nStartPos += 5; // Abstand Context-Bmp - Text
            break;

        case CHECK_BUTTONS :
            nStartPos += nCheckWidthDIV2;
            nStartPos += nCheckWidthDIV2;  // rechter Rand CheckButton
            nStartPos += 3;  // Abstand CheckButton Context-Bmp
            nStartPos += nContextWidthDIV2;  // Mitte der Context-Bmp
            nStartPos += nContextWidthDIV2;  // rechter Rand der Context-Bmp
            if( nContextBmpWidthMax )
                nStartPos += 5; // Abstand Context-Bmp - Text
            break;
    }
    return nStartPos;
}

void SvTreeListBox::EndSelection()
{
    pImp->EndSelection();
}

BOOL SvTreeListBox::IsNodeButton( const Point& rPos ) const
{
    SvLBoxEntry* pEntry = GetEntry( rPos );
    if( pEntry )
        return pImp->IsNodeButton( rPos, pEntry );
    return FALSE;
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

void SvTreeListBox::EnableAsyncDrag( BOOL b )
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

void SvTreeListBox::ShowFocusRect( const SvLBoxEntry* pEntry )
{
    pImp->ShowFocusRect( pEntry );
}

void SvTreeListBox::SetTabBar( TabBar* pTabBar )
{
    pImp->SetTabBar( pTabBar );
}

void SvTreeListBox::DataChanged( const DataChangedEvent& rDCEvt )
{
    if( (rDCEvt.GetType()==DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        nEntryHeight = 0;   // _together_ with TRUE of 1. par (bFont) of InitSettings() a zero-height
                            //  forces complete recalc of heights!
        InitSettings( TRUE, TRUE, TRUE );
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

void SvTreeListBox::InitSettings(BOOL bFont,BOOL bForeground,BOOL bBackground)
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

BOOL SvTreeListBox::IsCellFocusEnabled() const
{
    return pImp->IsCellFocusEnabled();
}

bool SvTreeListBox::SetCurrentTabPos( USHORT _nNewPos )
{
    return pImp->SetCurrentTabPos( _nNewPos );
}

USHORT SvTreeListBox::GetCurrentTabPos() const
{
    return pImp->GetCurrentTabPos();
}

void SvTreeListBox::InitStartEntry()
{
    if( !pImp->pStartEntry )
        pImp->pStartEntry = GetModel()->First();
}

void SvTreeListBox::CancelPendingEdit()
{
    if( pImp )
        pImp->CancelPendingEdit();
}

PopupMenu* SvTreeListBox::CreateContextMenu( void )
{
    return NULL;
}

void SvTreeListBox::ExcecuteContextMenuAction( USHORT )
{
    DBG_WARNING( "SvTreeListBox::ExcecuteContextMenuAction(): now there's happening nothing!" );
}

void SvTreeListBox::EnableContextMenuHandling( void )
{
    DBG_ASSERT( pImp, "-SvTreeListBox::EnableContextMenuHandling(): No implementation!" );

    pImp->bContextMenuHandling = TRUE;
}

void SvTreeListBox::EnableContextMenuHandling( BOOL b )
{
    DBG_ASSERT( pImp, "-SvTreeListBox::EnableContextMenuHandling(): No implementation!" );

    pImp->bContextMenuHandling = b;
}

BOOL SvTreeListBox::IsContextMenuHandlingEnabled( void ) const
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

    if ( pEntry->HasChildsOnDemand() || pEntry->HasChilds() )
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

void SvTreeListBox::CallImplEventListeners(ULONG nEvent, void* pData)
{
    CallEventListeners(nEvent, pData);
}

void SvTreeListBox::FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& rStateSet ) const
{
      SvLBox::FillAccessibleStateSet( rStateSet );
}
