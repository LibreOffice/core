/*************************************************************************
 *
 *  $RCSfile: svtreebx.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SVTREEBX_CXX

#ifndef _SV_SVAPP_HXX //autogen wg. Application
#include <vcl/svapp.hxx>
#endif
#pragma hdrstop

class TabBar;

#include <svlbox.hxx>
#include <svlbitm.hxx>
#include <svtreebx.hxx>
#ifndef _SVIMPLBOX_HXX
#include <svimpbox.hxx>
#endif

/*
    Bugs/ToDo

    - Berechnung Rectangle beim Inplace-Editing (Bug bei manchen Fonts)
    - SetSpaceBetweenEntries: Offset wird in SetEntryHeight nicht
      beruecksichtigt
*/

#define TREEFLAG_FIXEDHEIGHT        0x0010


DBG_NAME(SvTreeListBox);

#define SV_LBOX_DEFAULT_INDENT_PIXEL 20

__EXPORT SvTreeListBox::SvTreeListBox( Window* pParent, WinBits nWinStyle )
    : SvLBox(pParent,nWinStyle )
{
    DBG_CTOR(SvTreeListBox,0);
    InitTreeView( nWinStyle );
}

__EXPORT SvTreeListBox::SvTreeListBox( Window* pParent , const ResId& rResId )
    : SvLBox( pParent,rResId )
{
    DBG_CTOR(SvTreeListBox,0);
    InitTreeView( 0 );
    Resize();
}

void SvTreeListBox::InitTreeView( WinBits nWinStyle )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pCheckButtonData = 0;
    nEntryHeight = 0;
    pEdCtrl = 0;
    nFirstSelTab = 0;
    nLastSelTab = 0;
    nFocusWidth = -1;

    Link* pLink = new Link( LINK(this,SvTreeListBox, DefaultCompare) );
    pReserved = pLink;

    nTreeFlags = TREEFLAG_RECALCTABS;
    nIndent = SV_LBOX_DEFAULT_INDENT_PIXEL;
    nEntryHeightOffs = SV_ENTRYHEIGHTOFFS_PIXEL;
    pImp = new SvImpLBox( this, GetModel(), nWinStyle );

    aContextBmpMode = SVLISTENTRYFLAG_EXPANDED;
    nContextBmpWidthMax = 0;
    SetFont( GetFont() );
    SetSpaceBetweenEntries( 0 );
    SetLineColor();
    InitSettings( TRUE, TRUE, TRUE );
    SetWindowBits( nWinStyle );
    SetTabs();
    InitAcc();
}


__EXPORT SvTreeListBox::~SvTreeListBox()
{
    DBG_DTOR(SvTreeListBox,0);
    if( IsInplaceEditingEnabled() )
        Application::RemoveAccel( &aInpEditAcc );
    delete pImp;
    delete (Link*)pReserved;
    ClearTabList();
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

void SvTreeListBox::InitAcc()
{
    DBG_CHKTHIS(SvTreeListBox,0);
#ifdef OS2
    aInpEditAcc.InsertItem( 1, KeyCode(KEY_F9,KEY_SHIFT) );
#else
    aInpEditAcc.InsertItem( 1, KeyCode(KEY_RETURN,KEY_MOD2) );
#endif
    aInpEditAcc.SetActivateHdl( LINK( this, SvTreeListBox, InpEdActivateHdl) );
}

IMPL_LINK_INLINE_START( SvTreeListBox, InpEdActivateHdl, Accelerator *, pAccelerator )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    EditEntry();
    return 1;
}
IMPL_LINK_INLINE_END( SvTreeListBox, InpEdActivateHdl, Accelerator *, pAccelerator )


void __EXPORT SvTreeListBox::Resize()
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
  const XubString& aStr, const Image& aCollEntryBmp, const Image& aExpEntryBmp)
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SvLBoxButton* pButton;
    SvLBoxString* pString;
    SvLBoxContextBmp* pContextBmp;

    if( nTreeFlags & TREEFLAG_CHKBTN )
    {
        pButton= new SvLBoxButton( pEntry,0,pCheckButtonData );
        pEntry->AddItem( pButton );
    }

    pContextBmp= new SvLBoxContextBmp( pEntry,0, aCollEntryBmp,aExpEntryBmp,
                                     aContextBmpMode );
    pEntry->AddItem( pContextBmp );

    pString = new SvLBoxString( pEntry, 0, aStr );
    pEntry->AddItem( pString );
}

XubString SvTreeListBox::GetEntryText(SvLBoxEntry* pEntry) const
{
    DBG_CHKTHIS(SvTreeListBox,0);
    DBG_ASSERT(pEntry,"Entry?")
    SvLBoxString* pItem = (SvLBoxString*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
    DBG_ASSERT(pItem,"GetEntryText:Item not found")
    return pItem->GetText();
}

const Image& SvTreeListBox::GetExpandedEntryBmp(SvLBoxEntry* pEntry) const
{
    DBG_CHKTHIS(SvTreeListBox,0);
    DBG_ASSERT(pEntry,"Entry?")
    SvLBoxContextBmp* pItem = (SvLBoxContextBmp*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
    DBG_ASSERT(pItem,"GetContextBmp:Item not found")
    return pItem->GetBitmap2();
}

const Image& SvTreeListBox::GetCollapsedEntryBmp(SvLBoxEntry* pEntry ) const
{
    DBG_CHKTHIS(SvTreeListBox,0);
    DBG_ASSERT(pEntry,"Entry?")
    SvLBoxContextBmp* pItem = (SvLBoxContextBmp*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
    DBG_ASSERT(pItem,"GetContextBmp:Item not found")
    return pItem->GetBitmap1();
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
                                     BOOL bChildsOnDemand, ULONG nPos, void* pUser )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    nTreeFlags |= TREEFLAG_MANINS;

    aCurInsertedExpBmp = aExpandedEntryBmp;
    aCurInsertedColBmp = aCollapsedEntryBmp;

    SvLBoxEntry* pEntry = CreateEntry();
    pEntry->SetUserData( pUser );
    InitEntry( pEntry, aText, aCollapsedEntryBmp, aExpandedEntryBmp );
    pEntry->EnableChildsOnDemand( bChildsOnDemand );

    if( !pParent )
        SvLBox::Insert( pEntry, nPos );
    else
        SvLBox::Insert( pEntry, pParent, nPos );

    aPrevInsertedExpBmp = aExpandedEntryBmp;
    aPrevInsertedColBmp = aCollapsedEntryBmp;

    nTreeFlags &= (~TREEFLAG_MANINS);

    return pEntry;
}

SvLBoxEntry* SvTreeListBox::InsertEntry( const XubString& aText,
    const Image& aExpEntryBmp, const Image& aCollEntryBmp,
    SvLBoxEntry* pParent, BOOL bChildsOnDemand, ULONG nPos, void* pUser )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    nTreeFlags |= TREEFLAG_MANINS;

    aCurInsertedExpBmp = aExpEntryBmp;
    aCurInsertedColBmp = aCollEntryBmp;

    short nExpWidth = (short)aExpEntryBmp.GetSizePixel().Width();
    short nColWidth = (short)aCollEntryBmp.GetSizePixel().Width();
    short nMax = Max(nExpWidth, nColWidth);
    if( nMax > nContextBmpWidthMax )
    {
        nContextBmpWidthMax = nMax;
        SetTabs();
    }

    SvLBoxEntry* pEntry = CreateEntry();
    pEntry->SetUserData( pUser );
    InitEntry( pEntry, aText, aCollEntryBmp, aExpEntryBmp );

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
    DBG_ASSERT(pItem,"SetText:Item not found")
    pItem->SetText( pEntry, aStr );
    pItem->InitViewData( this, pEntry, 0 );
    GetModel()->InvalidateEntry( pEntry );
}

void SvTreeListBox::SetExpandedEntryBmp(SvLBoxEntry* pEntry, const Image& aBmp)
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SvLBoxContextBmp* pItem = (SvLBoxContextBmp*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
    DBG_ASSERT(pItem,"SetExpBmp:Item not found")
    pItem->SetBitmap2( pEntry, aBmp );
    GetModel()->InvalidateEntry( pEntry );
    SetEntryHeight( pEntry );
    Size aSize = aBmp.GetSizePixel();
    if( aSize.Width() > nContextBmpWidthMax )
    {
        nContextBmpWidthMax = (short)aSize.Width();
        SetTabs();
    }
}

void SvTreeListBox::SetCollapsedEntryBmp(SvLBoxEntry* pEntry,const Image& aBmp )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SvLBoxContextBmp* pItem = (SvLBoxContextBmp*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
    DBG_ASSERT(pItem,"SetExpBmp:Item not found")
    pItem->SetBitmap1( pEntry, aBmp );
    GetModel()->InvalidateEntry( pEntry );
    SetEntryHeight( pEntry );
    Size aSize = aBmp.GetSizePixel();
    if( aSize.Width() > nContextBmpWidthMax )
    {
        nContextBmpWidthMax = (short)aSize.Width();
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
        DBG_ASSERT(pItem,"SetCheckButton:Item not found")
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
        DBG_ASSERT(pItem,"GetChButnState:Item not found")
        USHORT nButtonFlags = pItem->GetButtonFlags();
        eState = pCheckButtonData->ConvertToButtonState( nButtonFlags );
    }
    return eState;
}


void __EXPORT SvTreeListBox::CheckButtonHdl()
{
    DBG_CHKTHIS(SvTreeListBox,0);
    aCheckButtonHdl.Call( this );
}

BOOL __EXPORT SvTreeListBox::QueryDrop( DropEvent& rDEvt )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    return SvLBox::QueryDrop( rDEvt ); // Basisklasse rufen
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

SvLBoxEntry* __EXPORT SvTreeListBox::CloneEntry( SvLBoxEntry* pSource )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    XubString aStr;
    Image aCollEntryBmp;
    Image aExpEntryBmp;

    SvLBoxString* pStringItem = (SvLBoxString*)(pSource->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
    if( pStringItem )
        aStr = pStringItem->GetText();
    SvLBoxContextBmp* pBmpItem = (SvLBoxContextBmp*)(pSource->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
    if( pBmpItem )
    {
        aCollEntryBmp = pBmpItem->GetBitmap1();
        aExpEntryBmp  = pBmpItem->GetBitmap2();
    }
    SvLBoxEntry* pEntry = CreateEntry();
    InitEntry( pEntry, aStr, aCollEntryBmp, aExpEntryBmp );
    pEntry->SvListEntry::Clone( pSource );
    pEntry->EnableChildsOnDemand( pSource->HasChildsOnDemand() );
    pEntry->SetUserData( pSource->GetUserData() );
    return pEntry;
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

void SvTreeListBox::SetDefaultExpandedEntryBmp( const Image& aBmp)
{
    DBG_CHKTHIS(SvTreeListBox,0);
    Size aSize = aBmp.GetSizePixel();
    if( aSize.Width() > nContextBmpWidthMax )
        nContextBmpWidthMax = (short)aSize.Width();
    SetTabs();
    aExpandedEntryBmp = aBmp;
}

void SvTreeListBox::SetDefaultCollapsedEntryBmp( const Image& aBmp)
{
    DBG_CHKTHIS(SvTreeListBox,0);
    Size aSize = aBmp.GetSizePixel();
    if( aSize.Width() > nContextBmpWidthMax )
        nContextBmpWidthMax = (short)aSize.Width();
    SetTabs();
    aCollapsedEntryBmp = aBmp;
}

void SvTreeListBox::EnableCheckButton( SvLBoxButtonData* pData )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    DBG_ASSERT(!GetEntryCount(),"EnableCheckButton: Entry count != 0");
    if( !pData )
        nTreeFlags &= (~TREEFLAG_CHKBTN);
    else
    {
        pCheckButtonData = pData;
        nTreeFlags |= TREEFLAG_CHKBTN;
        pData->SetLink( LINK(this, SvTreeListBox, CheckButtonClick));
    }

    SetTabs();
    if( IsUpdateMode() )
        Invalidate();
}

void SvTreeListBox::SetNodeBitmaps( const Image& rCollapsedNodeBmp,
    const Image& rExpandedNodeBmp)
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SetExpandedNodeBmp( rExpandedNodeBmp );
    SetCollapsedNodeBmp( rCollapsedNodeBmp );
    SetTabs();
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
    if( Control::HasFocus() )
        Application::InsertAccel( &aInpEditAcc );
}

void __EXPORT SvTreeListBox::KeyInput( const KeyEvent& rKEvt )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    // unter OS/2 bekommen wir auch beim Editieren Key-Up/Down
    if( IsEditingActive() )
        return;

    nImpFlags |= SVLBOX_IS_TRAVELSELECT;
    USHORT nCode = rKEvt.GetKeyCode().GetCode();

#ifdef OVDEBUG
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

    if( IsInplaceEditingEnabled() &&
         nCode == KEY_RETURN &&
         rKEvt.GetKeyCode().IsMod2() )
    {
        SvLBoxEntry* pEntry = GetCurEntry();
        if( pEntry )
            EditEntry( pEntry );
        nImpFlags &= ~SVLBOX_IS_TRAVELSELECT;
    }
    else
    {
        BOOL bKeyUsed = pImp->KeyInput( rKEvt );
        nImpFlags &= ~SVLBOX_IS_TRAVELSELECT;
        if( !bKeyUsed )
            SvLBox::KeyInput( rKEvt );
    }
}

void SvTreeListBox::RequestingChilds( SvLBoxEntry* pParent )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if( !pParent->HasChilds() )
        InsertEntry( String::CreateFromAscii("<dummy>"), pParent, FALSE, LIST_APPEND );
}

void __EXPORT SvTreeListBox::GetFocus()
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if( IsInplaceEditingEnabled() )
        Application::InsertAccel( &aInpEditAcc );
    pImp->GetFocus();
    SvLBox::GetFocus();
}

void __EXPORT SvTreeListBox::LoseFocus()
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if( IsInplaceEditingEnabled() )
        Application::RemoveAccel( &aInpEditAcc );
    pImp->LoseFocus();
    SvLBox::LoseFocus();
}

void __EXPORT SvTreeListBox::ModelHasCleared()
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->pCursor = 0; //sonst Absturz beim Inplace-Editieren im GetFocus
    delete pEdCtrl;
    pEdCtrl = 0;
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
        ExpandedHdl();
    }
    return bCollapsed;
}

BOOL SvTreeListBox::Select( SvLBoxEntry* pEntry, BOOL bSelect )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    DBG_ASSERT(pEntry,"Select: Null-Ptr")
    BOOL bRetVal = SvListView::Select( pEntry, bSelect );
    DBG_ASSERT(IsSelected(pEntry)==bSelect,"Select failed");
    if( bRetVal )
    {
        pImp->EntrySelected( pEntry, bSelect );
        pHdlEntry = pEntry;
        if( bSelect )
            SelectHdl();
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

void SvTreeListBox::SelectAll( BOOL bSelect, BOOL bPaint )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->SelAllDestrAnch(
        bSelect,
        TRUE,       // Anker loeschen,
        TRUE );     // auch bei SINGLE_SELECTION den Cursor deselektieren
}

void __EXPORT SvTreeListBox::ModelHasInsertedTree( SvListEntry* pEntry )
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

void __EXPORT SvTreeListBox::ModelHasInserted( SvListEntry* pEntry )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    ImpEntryInserted( (SvLBoxEntry*)pEntry );
    pImp->EntryInserted( (SvLBoxEntry*)pEntry );
}

void __EXPORT SvTreeListBox::ModelIsMoving(SvListEntry* pSource,
                                        SvListEntry* /* pTargetParent */,
                                        ULONG /* nChildPos */ )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->MovingEntry( (SvLBoxEntry*)pSource );
}

void __EXPORT SvTreeListBox::ModelHasMoved( SvListEntry* pSource )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->EntryMoved( (SvLBoxEntry*)pSource );
}

void __EXPORT SvTreeListBox::ModelIsRemoving( SvListEntry* pEntry )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->RemovingEntry( (SvLBoxEntry*)pEntry );
    NotifyRemoving( (SvLBoxEntry*)pEntry );
}

void __EXPORT SvTreeListBox::ModelHasRemoved( SvListEntry* /* pEntry */ )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->EntryRemoved();
}

void SvTreeListBox::SetCollapsedNodeBmp( const Image& rBmp)
{
    DBG_CHKTHIS(SvTreeListBox,0);
    AdjustEntryHeight( rBmp );
    pImp->SetCollapsedNodeBmp( rBmp );
}

void SvTreeListBox::SetExpandedNodeBmp( const Image& rBmp)
{
    DBG_CHKTHIS(SvTreeListBox,0);
    AdjustEntryHeight( rBmp );
    pImp->SetExpandedNodeBmp( rBmp );
}


void SvTreeListBox::SetFont( const Font& rFont )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    Font aTempFont( rFont );
    aTempFont.SetTransparent( TRUE );
#ifndef VCL
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    aTempFont.SetColor( rStyleSettings.GetFieldTextColor() );
#endif
    Control::SetFont( aTempFont );
    AdjustEntryHeight( aTempFont );
    // immer Invalidieren, sonst fallen wir
    // bei SetEntryHeight auf die Nase
    RecalcViewData();
}


void __EXPORT SvTreeListBox::Paint( const Rectangle& rRect )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    SvLBox::Paint( rRect );
    if( nTreeFlags & TREEFLAG_RECALCTABS )
        SetTabs();
    pImp->Paint( rRect );
}

void __EXPORT SvTreeListBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->MouseButtonDown( rMEvt );
}

void __EXPORT SvTreeListBox::MouseButtonUp( const MouseEvent& rMEvt )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->MouseButtonUp( rMEvt );
}

void __EXPORT SvTreeListBox::MouseMove( const MouseEvent& rMEvt )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    pImp->MouseMove( rMEvt );
}


void __EXPORT SvTreeListBox::SetUpdateMode( BOOL bUpdate )
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
        nEntryHeight -= nEntryHeightOffs;
        nEntryHeightOffs = (short)nOffsLogic;
        nEntryHeight += nOffsLogic;
        AdjustEntryHeight( GetFont() );
        RecalcViewData();
        pImp->SetEntryHeight( nEntryHeight );
    }
}

void SvTreeListBox::SetCursor( SvLBoxEntry* pEntry, BOOL bForceNoSelect )
{
    pImp->SetCursor(pEntry, bForceNoSelect);
}

void SvTreeListBox::SetCurEntry( SvLBoxEntry* pEntry )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if( pEntry )
        pImp->SetCurEntry( pEntry );
}

Image SvTreeListBox::GetCollapsedNodeBmp() const
{
    return pImp->GetCollapsedNodeBmp();
}

Image SvTreeListBox::GetExpandedNodeBmp() const
{
    return pImp->GetExpandedNodeBmp();
}

Point SvTreeListBox::GetEntryPos( SvLBoxEntry* pEntry ) const
{
    return pImp->GetEntryPos( pEntry );
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

void __EXPORT SvTreeListBox::ModelHasEntryInvalidated( SvListEntry* pEntry )
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
    Point aPos = GetEntryPos( pEntry );
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
    DBG_CHKTHIS(SvTreeListBox,0);
    if( IsEditingActive() )
        EndEditing();
    if( !pEntry )
        pEntry = GetCurEntry();
    if( pEntry )
    {
        SvLBoxString* pItem = (SvLBoxString* )pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING);
        Selection aSel( SELECTION_MIN, SELECTION_MAX );
        if( pItem && EditingEntry( pEntry, aSel ) )
        {
            SelectAll( FALSE );
            MakeVisible( pEntry );
            EditItemText( pEntry, pItem, aSel );
        }
    }
}

void SvTreeListBox::EditedText( const XubString& rStr )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    Point aPos = GetEntryPos( pEdEntry );
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

void SvTreeListBox::SetWindowBits( WinBits nWinStyle )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    nWindowStyle = nWinStyle;
    nTreeFlags |= TREEFLAG_RECALCTABS;
    if( nWinStyle & WB_SORT )
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
    nWinStyle |= WB_VSCROLL;
#endif
    pImp->SetWindowBits( nWinStyle );
    pImp->Resize();
    Invalidate();
}

void SvTreeListBox::PaintEntry( SvLBoxEntry* pEntry )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    DBG_ASSERT(pEntry,"PaintEntry:No Entry")
    if( pEntry )
        pImp->PaintEntry( pEntry );
}

void SvTreeListBox::InvalidateEntry( SvLBoxEntry* pEntry )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    DBG_ASSERT(pEntry,"InvalidateEntry:No Entry")
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

    if( nTreeFlags & TREEFLAG_RECALCTABS )
        SetTabs();

    short nTempEntryHeight = GetEntryHeight();
    long nWidth = pImp->GetOutputSize().Width();

    // wurde innerhalb des PreparePaints die horizontale ScrollBar
    // angeschaltet? Wenn ja, muss die ClipRegion neu gesetzt werden
    if( !bHorSBar && pImp->HasHorScrollBar() )
        SetClipRegion( Region(pImp->GetClipRegionRect()) );

    Point aPos( GetMapMode().GetOrigin() );
    aPos.X() *= -1; // Umrechnung Dokumentkoord.
    long nMaxRight = nWidth + aPos.X() - 1;

    Font aBackupFont( GetFont() );
    Color aBackupColor = GetFillColor();

    int bCurFontIsSel = FALSE;
    BOOL bInUse = pEntry->HasInUseEmphasis();
    // wenn eine ClipRegion von aussen gesetzt wird, dann
    // diese nicht zuruecksetzen
    BOOL bResetClipRegion = !bHasClipRegion;
    BOOL bHideSelection = ((nWindowStyle & WB_HIDESELECTION) && !HasFocus())!=0;
    const StyleSettings& rSettings = GetSettings().GetStyleSettings();
    Font aHiliteFont( GetFont() );
    aHiliteFont.SetColor( rSettings.GetHighlightTextColor() );

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
            aPos.X() = nX;
            aPos.Y() = nLine;

            // Hintergrund-Muster & Farbe bestimmen

            Wallpaper aWallpaper = GetBackground();

            int bSelTab = nFlags & SV_LBOXTAB_SHOW_SELECTION;
            USHORT nItemType = pItem->IsA();

            if( pViewDataEntry->IsSelected() && bSelTab &&
                !pViewDataEntry->IsCursored() )
            {
                if( !bInUse || nItemType != SV_ITEM_ID_LBOXCONTEXTBMP )
                {
                    if( bHideSelection )
                        aWallpaper.SetColor( rSettings.GetDeactiveColor() );
                    else
                        aWallpaper.SetColor( rSettings.GetHighlightColor() );
                    // Font auf Hilite-Farbe setzen ?
                    if( !bCurFontIsSel && nItemType == SV_ITEM_ID_LBOXSTRING )
                    {
                        Control::SetFont( aHiliteFont );
                        bCurFontIsSel = TRUE;
                    }
                }
                else // ContextBitmap + InUse-Emphasis + Selektiert
                    aWallpaper.SetColor( rSettings.GetHighlightColor() );
            }
            else  // keine Selektion
            {
                if( bInUse && nItemType == SV_ITEM_ID_LBOXCONTEXTBMP )
                    aWallpaper.SetColor( rSettings.GetFieldColor() );
                else if( bCurFontIsSel && nItemType == SV_ITEM_ID_LBOXSTRING )
                {
                    bCurFontIsSel = FALSE;
                    Control::SetFont( aBackupFont );
                }
            }

            // Hintergrund zeichnen
            if( !(nTreeFlags & TREEFLAG_USESEL))
            {
                // nur den Bereich zeichnen, den das Item einnimmt
                aRectSize.Width() = aSize.Width();
                aRect.SetPos( aPos );
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
            aPos.Y() += ( nTempEntryHeight - aSize.Height() ) / 2;
            pItem->Paint( aPos, *this, pViewDataEntry->GetFlags(), pEntry );

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
        Control::SetFont( aBackupFont );

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
                if( IsExpanded(pEntry) )
                    pImg = &pImp->GetExpandedNodeBmp();
                else
                {
                    if( (!pEntry->HasChilds()) && pEntry->HasChildsOnDemand() &&
                        (!(pEntry->GetFlags() & SV_ENTRYFLAG_HAD_CHILDREN)) &&
                        pImp->GetDontKnowNodeBmp().GetSizePixel().Width() )
                        pImg = &pImp->GetDontKnowNodeBmp();
                    else
                        pImg = &pImp->GetCollapsedNodeBmp();
                }
                aPos.Y() += (nTempEntryHeight - pImg->GetSizePixel().Height()) / 2;
                DrawImage( aPos, *pImg );
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
                nFocusWidth -= (short)nTabPos; //pTab->GetPos();
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
    DBG_ASSERT(pTab,"No Tab")
    long nPos = pTab->GetPos();
    if( pTab->IsDynamic() )
    {
        USHORT nDepth = pModel->GetDepth( pEntry );
        nDepth *= (USHORT)nIndent;
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

void SvTreeListBox::AddTab(long nPos,USHORT nFlags,void* pUserData )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    nFocusWidth = -1;
    SvLBoxTab* pTab = new SvLBoxTab( nPos, nFlags );
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

void SvTreeListBox::NotifyScrolling( long nCount )
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

void SvTreeListBox::Invalidate()
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if( nFocusWidth == -1 )
        // damit Control nicht nach dem Paint ein falsches FocusRect anzeigt
        pImp->RecalcFocusRect();
    NotifyInvalidating();
    SvLBox::Invalidate();
    pImp->Invalidate();
}

void SvTreeListBox::Invalidate( const Rectangle& rRect )
{
    DBG_CHKTHIS(SvTreeListBox,0);
    if( nFocusWidth == -1 )
        // damit Control nicht nach dem Paint ein falsches FocusRect anzeigt
        pImp->RecalcFocusRect();
    NotifyInvalidating();
    SvLBox::Invalidate( rRect );
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

ULONG SvTreeListBox::GetAscInsertionPos(SvLBoxEntry* pEntry,SvLBoxEntry* pParent)
{
    return LIST_APPEND;
}

ULONG SvTreeListBox::GetDescInsertionPos(SvLBoxEntry* pEntry,SvLBoxEntry* pParent)
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
        Point aPos = GetEntryPos( pEntry );
        aRect = ((SvTreeListBox*)this)->GetFocusRect( pEntry, aPos.Y() );
    }
    Region aRegion( aRect );
    return aRegion;
}


void SvTreeListBox::Command( const CommandEvent& rCEvt )
{
    DBG_CHKTHIS(SvTreeListBox,0);
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
    // Intntl: Compare(a,b) ==> b.Compare(a) ==> strcmp(a,b)
    const International& rInter = Application::GetAppInternational();
    StringCompare eCompare = rInter.Compare( aLeft, aRight );
    return (long)eCompare;
}

void SvTreeListBox::ModelNotification( USHORT nActionId, SvListEntry* pEntry1,
                        SvListEntry* pEntry2, ULONG nPos )
{
    if( nActionId == LISTACTION_CLEARING )
        CancelTextEditing();

    SvLBox::ModelNotification( nActionId, pEntry1, pEntry2, nPos );
    switch( nActionId )
    {
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

void SvTreeListBox::SetDontKnowNodeBitmap( const Image& rCollapsedNodeBmp )
{
    pImp->SetDontKnowNodeBmp( rCollapsedNodeBmp );
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
        Point aPos( GetEntryPos(pNext) );
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
        InitSettings( TRUE, TRUE, TRUE );
        Invalidate();
    }
    else
        Control::DataChanged( rDCEvt );
}

void SvTreeListBox::InitSettings(BOOL bFont,BOOL bForeground,BOOL bBackground)
{
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    if( bFont )
    {
        Font aFont;
        aFont = rStyleSettings.GetFieldFont();
        SetPointFont( aFont );
    }

    if( bForeground || bFont )
    {
        SetTextColor( rStyleSettings.GetFieldTextColor() );
        SetTextFillColor();
    }

    if( bBackground )
        SetBackground( rStyleSettings.GetFieldColor() );
}

void SvTreeListBox::InitStartEntry()
{
    if( !pImp->pStartEntry )
        pImp->pStartEntry = GetModel()->First();
}


