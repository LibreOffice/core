/*************************************************************************
 *
 *  $RCSfile: svicnvw.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:56 $
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

#include <svlbox.hxx>
#include <svicnvw.hxx>
#include <svimpicn.hxx>
#include <svlbitm.hxx>

#pragma hdrstop

#define ICNVW_BLOCK_ENTRYINS    0x0001

SvIcnVwDataEntry::SvIcnVwDataEntry()
    : nIcnVwFlags(0),eTextMode(ShowTextDontKnow)
{
}

SvIcnVwDataEntry::~SvIcnVwDataEntry()
{
}

SvIconView::SvIconView( Window* pParent, WinBits nWinStyle ) :
    SvLBox( pParent, nWinStyle | WB_BORDER )
{
    nWinBits = nWinStyle;
    nIcnVwFlags = 0;
    pImp = new SvImpIconView( this, GetModel(), nWinStyle | WB_ICON );
    pImp->pViewData = 0;
    SetSelectionMode( SINGLE_SELECTION );
    SetLineColor();
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    SetBackground( Wallpaper( rStyleSettings.GetFieldColor() ) );
    SetDefaultFont();
}

SvIconView::SvIconView( Window* pParent , const ResId& rResId ) :
    SvLBox( pParent, rResId )
{
    pImp = new SvImpIconView( this, GetModel(), WB_BORDER | WB_ICON );
    nIcnVwFlags = 0;
    pImp->pViewData = 0;
    SetLineColor();
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    SetBackground( Wallpaper( rStyleSettings.GetFieldColor() ) );
    SetDefaultFont();
    pImp->SetSelectionMode( GetSelectionMode() );
    pImp->SetWindowBits( nWindowStyle );
    nWinBits = nWindowStyle;
}

SvIconView::~SvIconView()
{
    delete pImp;
}

void SvIconView::SetDefaultFont()
{
    SetFont( GetFont() );
}

SvLBoxEntry* SvIconView::CreateEntry( const XubString& rStr,
    const Image& rCollEntryBmp, const Image& rExpEntryBmp )
{
    SvLBoxEntry* pEntry = new SvLBoxEntry;

    SvLBoxContextBmp* pContextBmp =
      new SvLBoxContextBmp( pEntry,0, rCollEntryBmp,rExpEntryBmp, 0xffff );
    pEntry->AddItem( pContextBmp );

    SvLBoxString* pString = new SvLBoxString( pEntry, 0, rStr );
    pEntry->AddItem( pString );

    return pEntry;
}

void SvIconView::DisconnectFromModel()
{
    SvLBox::DisconnectFromModel();
    pImp->SetModel( GetModel(), 0 );
}


SvLBoxEntry* SvIconView::InsertEntry( const XubString& rText,
    SvLBoxEntry* pParent, BOOL bChildsOnDemand, ULONG nPos )
{
    SvLBoxEntry* pEntry = CreateEntry(
        rText, aCollapsedEntryBmp, aExpandedEntryBmp );
    pEntry->EnableChildsOnDemand( bChildsOnDemand );

    if ( !pParent )
        SvLBox::Insert( pEntry, nPos );
    else
        SvLBox::Insert( pEntry, pParent, nPos );
    return pEntry;
}

SvLBoxEntry* SvIconView::InsertEntry( const XubString& rText,
    const Image& rExpEntryBmp,
    const Image& rCollEntryBmp,
    SvLBoxEntry* pParent, BOOL bChildsOnDemand, ULONG nPos)
{
    SvLBoxEntry* pEntry = CreateEntry(
        rText, rCollEntryBmp, rExpEntryBmp );

    pEntry->EnableChildsOnDemand( bChildsOnDemand );
    if ( !pParent )
        SvLBox::Insert( pEntry, nPos );
    else
        SvLBox::Insert( pEntry, pParent, nPos );
    return pEntry;
}


void SvIconView::SetEntryText(SvLBoxEntry* pEntry, const XubString& rStr)
{
    SvLBoxString* pItem = (SvLBoxString*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
    if ( pItem )
    {
        pItem->SetText( pEntry, rStr );
        GetModel()->InvalidateEntry( pEntry );
    }
}

void SvIconView::SetExpandedEntryBmp(SvLBoxEntry* pEntry, const Image& rBmp)
{
    SvLBoxContextBmp* pItem = (SvLBoxContextBmp*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
    if ( pItem )
    {
        pItem->SetBitmap2( pEntry, rBmp );
        GetModel()->InvalidateEntry( pEntry );
    }
}

void SvIconView::SetCollapsedEntryBmp(SvLBoxEntry* pEntry,
    const Image& rBmp )
{
    SvLBoxContextBmp* pItem = (SvLBoxContextBmp*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
    if ( pItem )
    {
        pItem->SetBitmap1( pEntry, rBmp );
        GetModel()->InvalidateEntry( pEntry );
    }
}

XubString SvIconView::GetEntryText(SvLBoxEntry* pEntry ) const
{
    XubString aStr;
    SvLBoxString* pItem = (SvLBoxString*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
    if ( pItem )
        aStr = pItem->GetText();
    return aStr;
}

Image SvIconView::GetExpandedEntryBmp(SvLBoxEntry* pEntry) const
{
    Image aBmp;
    SvLBoxContextBmp* pItem = (SvLBoxContextBmp*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
    if ( pItem )
        aBmp = pItem->GetBitmap2();
    return aBmp;
}

Image SvIconView::GetCollapsedEntryBmp(SvLBoxEntry* pEntry) const
{
    Image aBmp;
    SvLBoxContextBmp* pItem = (SvLBoxContextBmp*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
    if ( pItem )
        aBmp = pItem->GetBitmap1();
    return aBmp;
}


SvLBoxEntry* SvIconView::CloneEntry( SvLBoxEntry* pSource )
{
    XubString aStr;
    Image aCollEntryBmp;
    Image aExpEntryBmp;

    SvLBoxString* pStringItem = (SvLBoxString*)(pSource->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
    if ( pStringItem )
        aStr = pStringItem->GetText();
    SvLBoxContextBmp* pBmpItem =(SvLBoxContextBmp*)(pSource->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
    if ( pBmpItem )
    {
        aCollEntryBmp = pBmpItem->GetBitmap1();
        aExpEntryBmp  = pBmpItem->GetBitmap2();
    }
    SvLBoxEntry* pEntry = CreateEntry( aStr, aCollEntryBmp, aExpEntryBmp );
    pEntry->SvListEntry::Clone( pSource );
    pEntry->EnableChildsOnDemand( pSource->HasChildsOnDemand() );
    pEntry->SetUserData( pSource->GetUserData() );
    return pEntry;
}


USHORT SvIconView::IsA()
{
    return SV_LISTBOX_ID_ICONVIEW;
}

void SvIconView::RequestingChilds( SvLBoxEntry* pParent )
{
    if ( !pParent->HasChilds() )
        InsertEntry( String::CreateFromAscii("<dummy>"), pParent, FALSE, LIST_APPEND );
}

void __EXPORT SvIconView::Paint( const Rectangle& rRect )
{
    pImp->Paint( rRect );
}

void __EXPORT SvIconView::MouseButtonDown( const MouseEvent& rMEvt )
{
    pImp->MouseButtonDown( rMEvt );
}

void __EXPORT SvIconView::MouseButtonUp( const MouseEvent& rMEvt )
{
    pImp->MouseButtonUp( rMEvt );
}

void __EXPORT SvIconView::MouseMove( const MouseEvent& rMEvt )
{
    pImp->MouseMove( rMEvt );
}

void __EXPORT SvIconView::KeyInput( const KeyEvent& rKEvt )
{
    // unter OS/2 bekommen wir auch beim Editieren Key-Up/Down
    if( IsEditingActive() )
        return;

    nImpFlags |= SVLBOX_IS_TRAVELSELECT;
    BOOL bKeyUsed = pImp->KeyInput( rKEvt );
    if ( !bKeyUsed )
        SvLBox::KeyInput( rKEvt );
    nImpFlags &= ~SVLBOX_IS_TRAVELSELECT;
}

void __EXPORT SvIconView::Resize()
{
    pImp->Resize();
    SvLBox::Resize();
}

void __EXPORT SvIconView::GetFocus()
{
    pImp->GetFocus();
    SvLBox::GetFocus();
}

void __EXPORT SvIconView::LoseFocus()
{
    pImp->LoseFocus();
    SvLBox::LoseFocus();
}

void SvIconView::SetUpdateMode( BOOL bUpdate )
{
    Control::SetUpdateMode( bUpdate );
    if ( bUpdate )
        pImp->UpdateAll();
}

void SvIconView::SetModel( SvLBoxTreeList* pNewModel )
{
}

void SvIconView::SetModel( SvLBoxTreeList* pNewModel, SvLBoxEntry* pParent )
{
    nIcnVwFlags |= ICNVW_BLOCK_ENTRYINS;
    SvLBox::SetModel( pNewModel );
    nIcnVwFlags &= (~ICNVW_BLOCK_ENTRYINS);
    if ( pParent && pParent->HasChildsOnDemand() )
        RequestingChilds( pParent );
    pImp->SetModel( pNewModel, pParent );
}

void __EXPORT SvIconView::ModelHasCleared()
{
    SvLBox::ModelHasCleared();
    pImp->Clear();
}

void __EXPORT SvIconView::ModelHasInserted( SvListEntry* pEntry )
{
    if( !(nIcnVwFlags & ICNVW_BLOCK_ENTRYINS ) )
        pImp->EntryInserted( (SvLBoxEntry*)pEntry );
}

void __EXPORT SvIconView::ModelHasInsertedTree( SvListEntry* pEntry )
{
    pImp->TreeInserted( (SvLBoxEntry*)pEntry );
}

void __EXPORT SvIconView::ModelIsMoving(SvListEntry* pSource,
        SvListEntry* /* pTargetParent */ , ULONG /* nChildPos */ )
{
    pImp->MovingEntry( (SvLBoxEntry*)pSource );
}

void __EXPORT SvIconView::ModelHasMoved(SvListEntry* pSource )
{
    pImp->EntryMoved( (SvLBoxEntry*)pSource );
}

void __EXPORT SvIconView::ModelIsRemoving( SvListEntry* pEntry )
{
    pImp->RemovingEntry( (SvLBoxEntry*)pEntry );
    NotifyRemoving( (SvLBoxEntry*)pEntry );
}

void __EXPORT SvIconView::ModelHasRemoved( SvListEntry* /* pEntry */ )
{
    pImp->EntryRemoved();
}

void __EXPORT SvIconView::ModelHasEntryInvalidated( SvListEntry* pEntry )
{
    // die einzelnen Items des Entries reinitialisieren
    SvLBox::ModelHasEntryInvalidated( pEntry );
    // painten
    pImp->ModelHasEntryInvalidated( pEntry );
}

void SvIconView::ShowTargetEmphasis( SvLBoxEntry* pEntry, BOOL bShow )
{
    pImp->ShowTargetEmphasis( pEntry, bShow );
}

Point SvIconView::GetEntryPos( SvLBoxEntry* pEntry ) const
{
    return ((SvIconView*)this)->pImp->GetEntryPos( pEntry );
}

void SvIconView::SetEntryPos( SvLBoxEntry* pEntry, const Point& rPos)
{
    pImp->SetEntryPos( pEntry, rPos, FALSE, TRUE );
}

void SvIconView::SetEntryPos( SvLBoxEntry* pEntry, const Point& rPos, BOOL bAdjustAtGrid )
{
    pImp->SetEntryPos( pEntry, rPos, bAdjustAtGrid );
}

void SvIconView::SetFont( const Font& rFont )
{
    Font aTempFont( rFont );
    aTempFont.SetTransparent( TRUE );
    SvLBox::SetFont( aTempFont );
    RecalcViewData();
    pImp->ChangedFont();
}

void SvIconView::ViewDataInitialized( SvLBoxEntry* pEntry )
{
    pImp->ViewDataInitialized( pEntry );
}

SvLBoxEntry* SvIconView::GetCurEntry() const
{
    return pImp->GetCurEntry();
}

SvLBoxEntry* SvIconView::GetDropTarget( const Point& rPos )
{
    return pImp->GetDropTarget( rPos );
}

SvLBoxEntry* SvIconView::GetEntry( const Point& rPixPos, BOOL ) const
{
    Point aPos( rPixPos );
    aPos -= GetMapMode().GetOrigin();
    return ((SvIconView*)this)->pImp->GetEntry( aPos );
}

SvLBoxEntry* SvIconView::GetEntryFromLogicPos( const Point& rDocPos ) const
{
    return ((SvIconView*)this)->pImp->GetEntry( rDocPos );
}


void SvIconView::SetWindowBits( WinBits nWinStyle )
{
    nWinBits = nWinStyle;
    pImp->SetWindowBits( nWinStyle );
}

void SvIconView::PaintEntry( SvLBoxEntry* pEntry )
{
    pImp->PaintEntry( pEntry );
}


void SvIconView::PaintEntry( SvLBoxEntry* pEntry, const Point& rPos )
{
    pImp->PaintEntry( pEntry, rPos );
}

Rectangle SvIconView::GetFocusRect( SvLBoxEntry* pEntry )
{
    return pImp->CalcFocusRect( pEntry );
}

void SvIconView::InvalidateEntry( SvLBoxEntry* pEntry )
{
    pImp->InvalidateEntry( pEntry );
}

void SvIconView::SetDragDropMode( DragDropMode nDDMode )
{
    SvLBox::SetDragDropMode( nDDMode );
    pImp->SetDragDropMode( nDDMode );
}

void SvIconView::SetSelectionMode( SelectionMode eSelMode )
{
    SvLBox::SetSelectionMode( eSelMode );
    pImp->SetSelectionMode( eSelMode );
}

BOOL SvIconView::Select( SvLBoxEntry* pEntry, BOOL bSelect )
{
    EndEditing();
    BOOL bRetVal = SvListView::Select( pEntry, bSelect );
    if( bRetVal )
    {
        pImp->EntrySelected( pEntry, bSelect );
        pHdlEntry = pEntry;
        SelectHdl();
    }
    return bRetVal;
}

void SvIconView::SelectAll( BOOL bSelect, BOOL bPaint )
{
    SvLBoxEntry* pEntry = pImp->GetCurParent();
    pEntry = FirstChild( pEntry );
    while( pEntry )
    {
        Select( pEntry, bSelect );
        pEntry = NextSibling( pEntry );
    }
}

void SvIconView::Arrange()
{
#ifdef DBG_UTIL
    USHORT n=1;
    if( n == 1 && n-1 == 0 )
    {
        pImp->Arrange();
    }
    else
    {
        pImp->AdjustAtGrid();
    }
#else
    pImp->Arrange();
#endif
}


void SvIconView::SetSpaceBetweenEntries( long nX, long nY )
{
    pImp->SetSpaceBetweenEntries( nX, nY );
}

BOOL SvIconView::NotifyMoving( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
    SvLBoxEntry*& rpNewParent, ULONG& rNewChildPos )
{
    return pImp->NotifyMoving(pTarget,pEntry,rpNewParent,rNewChildPos);
}

BOOL SvIconView::NotifyCopying( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
    SvLBoxEntry*& rpNewParent, ULONG& rNewChildPos )
{
    return pImp->NotifyCopying(pTarget,pEntry,rpNewParent,rNewChildPos);
}


void SvIconView::EnableInplaceEditing( BOOL bEnable )
{
    SvLBox::EnableInplaceEditing( bEnable );
}

void SvIconView::EditingRequest( SvLBoxEntry* pEntry, SvLBoxItem* pItem,
                                 const Point& )
{
    if ( pItem->IsA() == SV_ITEM_ID_LBOXSTRING )
    {
        Selection aSel( SELECTION_MIN, SELECTION_MAX );
        if ( EditingEntry( pEntry, aSel ) )
        {
            SelectAll( FALSE );
            EditItemText( pEntry, (SvLBoxString*)pItem, aSel );
        }
    }
}


void SvIconView::EditItemText( SvLBoxEntry* pEntry, SvLBoxItem* pItem,
    const Selection& rSel )
{
    DBG_ASSERT(pEntry&&pItem,"EditItemText:Params?")
    pCurEdEntry = pEntry;
    pCurEdItem = pItem;
    Rectangle aRect( pImp->CalcTextRect( pEntry, (SvLBoxString*)pItem,0,TRUE ));

    aRect.Bottom() += 4;
    pImp->MakeVisible( aRect ); // vor der Umrechnung in Pixel-Koord. rufen!
    aRect.Bottom() -= 4;

    Point aPos( aRect.TopLeft() );
    aPos += GetMapMode().GetOrigin(); // Dok-Koord. -> Window-Koord.
    aRect.SetPos( aPos );

    aRect.Bottom() += 2; // sieht huebscher aus

#ifdef WIN
    aRect.Bottom() += 4;
#endif
#ifdef OS2

#if OS2_SINGLE_LINE_EDIT
    aRect.Left() -= 3;
    aRect.Right() += 3;
    aRect.Top() -= 3;
    aRect.Bottom() += 3;
#else
    aRect.Left() -= 10;
    aRect.Right() += 10;
    aRect.Top() -= 5;
    aRect.Bottom() += 5;
#endif

#endif // OS2
    EditText( ((SvLBoxString*)pItem)->GetText(), aRect, rSel, TRUE );
}

void SvIconView::EditEntry( SvLBoxEntry* pEntry )
{
    if( !pEntry )
        pEntry = pImp->GetCurEntry();
    if( pEntry )
    {
        SvLBoxString* pItem = (SvLBoxString*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXSTRING));
        if( pItem )
        {
            Selection aSel( SELECTION_MIN, SELECTION_MAX );
            if( EditingEntry( pEntry, aSel ) )
            {
                SelectAll( FALSE );
                EditItemText( pEntry, pItem, aSel );
            }
        }
    }
}

void SvIconView::EditedText( const XubString& rStr )
{
    XubString aRefStr( ((SvLBoxString*)pCurEdItem)->GetText() );
    if ( EditedEntry( pCurEdEntry, rStr ) )
    {
        ((SvLBoxString*)pCurEdItem)->SetText( pCurEdEntry, rStr );
        pModel->InvalidateEntry( pCurEdEntry );
    }
    if( GetSelectionMode()==SINGLE_SELECTION && !GetSelectionCount())
        Select( pCurEdEntry );
}


BOOL SvIconView::EditingEntry( SvLBoxEntry*, Selection& )
{
    return TRUE;
}

BOOL SvIconView::EditedEntry( SvLBoxEntry*, const XubString& )
{
    return TRUE;
}


void SvIconView::WriteDragServerInfo( const Point& rPos, SvLBoxDDInfo* pInfo)
{
    pImp->WriteDragServerInfo( rPos, pInfo );
}

void SvIconView::ReadDragServerInfo( const Point& rPos, SvLBoxDDInfo* pInfo )
{
    pImp->ReadDragServerInfo( rPos, pInfo );
}

void SvIconView::Command( const CommandEvent& rCEvt )
{
    pImp->Command( rCEvt );
}

void SvIconView::SetCurParent( SvLBoxEntry* pNewParent )
{
    if ( pNewParent && pNewParent->HasChildsOnDemand() )
        RequestingChilds( pNewParent );
    pImp->SetCurParent( pNewParent );
}

SvLBoxEntry* SvIconView::GetCurParent() const
{
    return pImp->GetCurParent();
}

SvViewData* SvIconView::CreateViewData( SvListEntry* pEntry )
{
    SvIcnVwDataEntry* pEntryData = new SvIcnVwDataEntry;
    return (SvViewData*)pEntryData;
}

void SvIconView::InitViewData( SvViewData* pData, SvListEntry* pEntry )
{
    SvLBox::InitViewData( pData, pEntry );
    pImp->InvalidateBoundingRect( ((SvIcnVwDataEntry*)pData)->aRect );
}

Region SvIconView::GetDragRegion() const
{
    Rectangle aRect;
    SvLBoxEntry* pEntry = GetCurEntry();
    if( pEntry )
        aRect = pImp->GetBoundingRect( pEntry );
    Region aRegion( aRect );
    return aRegion;
}

ULONG SvIconView::GetSelectionCount() const
{
    return (ULONG)(pImp->GetSelectionCount());
}

void SvIconView::SetGrid( long nDX, long nDY )
{
    pImp->SetGrid( nDX, nDY );
}

void SvIconView::ModelNotification( USHORT nActionId, SvListEntry* pEntry1,
    SvListEntry* pEntry2, ULONG nPos )
{
    SvLBox::ModelNotification( nActionId, pEntry1, pEntry2, nPos );
    switch( nActionId )
    {
        case LISTACTION_RESORTING:
            SetUpdateMode( FALSE );
            break;

        case LISTACTION_RESORTED:
            SetUpdateMode( TRUE );
            Arrange();
            break;

        case LISTACTION_CLEARED:
            if( IsUpdateMode() )
                Update();
            break;
    }
}


void SvIconView::Scroll( long nDeltaX, long nDeltaY )
{
    pImp->Scroll( nDeltaX, nDeltaY, FALSE );
}

void SvIconView::PrepareCommandEvent( const CommandEvent& rCEvt )
{
    pImp->PrepareCommandEvent( rCEvt );
}

void SvIconView::BeginDrag( const Point& rPos )
{
    SvLBoxEntry* pEntry = GetEntry( rPos, TRUE );
    pImp->pViewData = pEntry;
    SvLBox::BeginDrag( rPos );
}

BOOL __EXPORT SvIconView::QueryDrop( DropEvent& rDEvt )
{
    if( pImp->pViewData )
    {
        pImp->HideDDIcon();
    }
    BOOL bResult = SvLBox::QueryDrop( rDEvt );
    if( bResult )
    {
        pImp->ShowDDIcon( pImp->pViewData, rDEvt.GetPosPixel() );
    }
    return bResult;
}

BOOL SvIconView::Drop( const DropEvent& rDEvt )
{
    if( pImp->pViewData )
    {
        pImp->HideDDIcon();
        pImp->pViewData = 0;
    }
    return SvLBox::Drop( rDEvt );
}

void SvIconView::ShowDDIcon( SvLBoxEntry* pRefEntry, const Point& rPos )
{
    pImp->ShowDDIcon( pRefEntry, rPos );
}

void SvIconView::HideDDIcon()
{
    pImp->HideDDIcon();
}

void SvIconView::HideShowDDIcon( SvLBoxEntry* pRefEntry, const Point& rPos )
{
    pImp->HideShowDDIcon( pRefEntry, rPos );
}

void SvIconView::SelectRect( const Rectangle& rRect, BOOL bAdd,
    SvPtrarr* pRects, short nOffs )
{
    pImp->SelectRect( rRect, bAdd, pRects, nOffs );
}

void SvIconView::CalcScrollOffsets( const Point& rRefPosPixel, long& rX, long& rY,
    BOOL b, USHORT nBorderWidth )
{
    pImp->CalcScrollOffsets( rRefPosPixel, rX, rY, b, nBorderWidth );
}

void SvIconView::EndTracking()
{
    pImp->EndTracking();
}

void SvIconView::MakeVisible( SvLBoxEntry* pEntry )
{
    pImp->MakeVisible( pEntry );
}

void SvIconView::PreparePaint( SvLBoxEntry* )
{
}

void SvIconView::AdjustAtGrid( SvLBoxEntry* pEntry )
{
    pImp->AdjustAtGrid( pEntry );
}

void SvIconView::LockEntryPos( SvLBoxEntry* pEntry, BOOL bLock )
{
    SvIcnVwDataEntry* pViewData = (SvIcnVwDataEntry*)GetViewData( pEntry );
    if( bLock )
        pViewData->SetVwFlags( ICNVW_FLAG_POS_LOCKED );
    else
        pViewData->ClearVwFlags( ICNVW_FLAG_POS_LOCKED );
}

BOOL SvIconView::IsEntryPosLocked( const SvLBoxEntry* pEntry ) const
{
    const SvIcnVwDataEntry* pViewData = (const SvIcnVwDataEntry*)GetViewData( (SvListEntry*)pEntry );
    return pViewData->IsEntryPosLocked();
}

void SvIconView::SetTextMode( SvIconViewTextMode eMode, SvLBoxEntry* pEntry )
{
    pImp->SetTextMode( eMode, pEntry );
}

SvIconViewTextMode SvIconView::GetTextMode( const SvLBoxEntry* pEntry ) const
{
    return pImp->GetTextMode( pEntry );
}

SvLBoxEntry* SvIconView::GetNextEntry( const Point& rPixPos, SvLBoxEntry* pCurEntry, BOOL  ) const
{
    Point aPos( rPixPos );
    aPos -= GetMapMode().GetOrigin();
    return ((SvIconView*)this)->pImp->GetNextEntry( aPos, pCurEntry );
}

SvLBoxEntry* SvIconView::GetPrevEntry( const Point& rPixPos, SvLBoxEntry* pCurEntry, BOOL  ) const
{
    Point aPos( rPixPos );
    aPos -= GetMapMode().GetOrigin();
    return ((SvIconView*)this)->pImp->GetPrevEntry( aPos, pCurEntry );
}

void SvIconView::ShowFocusRect( const SvLBoxEntry* pEntry )
{
    pImp->ShowFocusRect( pEntry );
}


