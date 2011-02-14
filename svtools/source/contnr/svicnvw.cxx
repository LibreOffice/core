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

#include <svtools/svlbox.hxx>
#include <svtools/svicnvw.hxx>
#include <svimpicn.hxx>
#include <svtools/svlbitm.hxx>

#ifndef GCC
#endif

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
    nIcnVwFlags = 0;
    pImp = new SvImpIconView( this, GetModel(), nWinStyle | WB_ICON );
    pImp->mpViewData = 0;
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
    pImp->mpViewData = 0;
    SetLineColor();
    const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();
    SetBackground( Wallpaper( rStyleSettings.GetFieldColor() ) );
    SetDefaultFont();
    pImp->SetSelectionMode( GetSelectionMode() );
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
    SvLBoxEntry* pParent, sal_Bool bChildsOnDemand, sal_uLong nPos )
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
    SvLBoxEntry* pParent, sal_Bool bChildsOnDemand, sal_uLong nPos)
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
        pItem->SetBitmap2( rBmp );
        GetModel()->InvalidateEntry( pEntry );
    }
}

void SvIconView::SetCollapsedEntryBmp(SvLBoxEntry* pEntry,
    const Image& rBmp )
{
    SvLBoxContextBmp* pItem = (SvLBoxContextBmp*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
    if ( pItem )
    {
        pItem->SetBitmap1( rBmp );
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


sal_uInt16 SvIconView::IsA()
{
    return SV_LISTBOX_ID_ICONVIEW;
}

void SvIconView::RequestingChilds( SvLBoxEntry* pParent )
{
    if ( !pParent->HasChilds() )
        InsertEntry( String::CreateFromAscii("<dummy>"), pParent, sal_False, LIST_APPEND );
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
    sal_Bool bKeyUsed = pImp->KeyInput( rKEvt );
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

void SvIconView::SetUpdateMode( sal_Bool bUpdate )
{
    Control::SetUpdateMode( bUpdate );
    if ( bUpdate )
        pImp->UpdateAll();
}

void SvIconView::SetModel( SvLBoxTreeList* )
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
        SvListEntry* /* pTargetParent */ , sal_uLong /* nChildPos */ )
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

void SvIconView::ShowTargetEmphasis( SvLBoxEntry* pEntry, sal_Bool bShow )
{
    pImp->ShowTargetEmphasis( pEntry, bShow );
}

Point SvIconView::GetEntryPosition( SvLBoxEntry* pEntry ) const
{
    return ((SvIconView*)this)->pImp->GetEntryPosition( pEntry );
}

void SvIconView::SetEntryPosition( SvLBoxEntry* pEntry, const Point& rPos)
{
    pImp->SetEntryPosition( pEntry, rPos, sal_False, sal_True );
}

void SvIconView::SetEntryPosition( SvLBoxEntry* pEntry, const Point& rPos, sal_Bool bAdjustAtGrid )
{
    pImp->SetEntryPosition( pEntry, rPos, bAdjustAtGrid );
}

void SvIconView::SetFont( const Font& rFont )
{
    Font aTempFont( rFont );
    aTempFont.SetTransparent( sal_True );
    SvLBox::SetFont( aTempFont );
    RecalcViewData();
    pImp->ChangedFont();
}

void SvIconView::ViewDataInitialized( SvLBoxEntry* pEntry )
{
    pImp->ViewDataInitialized( pEntry );
}

SvLBoxEntry* SvIconView::GetDropTarget( const Point& rPos )
{
    return pImp->GetDropTarget( rPos );
}

SvLBoxEntry* SvIconView::GetEntry( const Point& rPixPos, sal_Bool ) const
{
    Point aPos( rPixPos );
    aPos -= GetMapMode().GetOrigin();
    return ((SvIconView*)this)->pImp->GetEntry( aPos );
}

SvLBoxEntry* SvIconView::GetEntryFromLogicPos( const Point& rDocPos ) const
{
    return ((SvIconView*)this)->pImp->GetEntry( rDocPos );
}


void SvIconView::StateChanged( StateChangedType i_nStateChange )
{
    SvLBox::StateChanged( i_nStateChange );
    if ( i_nStateChange == STATE_CHANGE_STYLE )
        pImp->SetStyle( GetStyle() );
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

void SvIconView::SetSelectionMode( SelectionMode eSelectMode )
{
    SvLBox::SetSelectionMode( eSelectMode );
    pImp->SetSelectionMode( eSelectMode );
}

sal_Bool SvIconView::Select( SvLBoxEntry* pEntry, sal_Bool bSelect )
{
    EndEditing();
    sal_Bool bRetVal = SvListView::Select( pEntry, bSelect );
    if( bRetVal )
    {
        pImp->EntrySelected( pEntry, bSelect );
        pHdlEntry = pEntry;
        SelectHdl();
    }
    return bRetVal;
}

void SvIconView::SelectAll( sal_Bool bSelect, sal_Bool )
{
    SvLBoxEntry* pEntry = pImp->GetCurParent();
    pEntry = FirstChild( pEntry );
    while( pEntry )
    {
        Select( pEntry, bSelect );
        pEntry = NextSibling( pEntry );
    }
}

void SvIconView::SetCurEntry( SvLBoxEntry* _pEntry )
{
    pImp->SetCursor( _pEntry );
    OnCurrentEntryChanged();
}

SvLBoxEntry* SvIconView::GetCurEntry() const
{
    return pImp->GetCurEntry();
}

void SvIconView::Arrange()
{
#ifdef DBG_UTIL
    sal_uInt16 n=1;
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

sal_Bool SvIconView::NotifyMoving( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
    SvLBoxEntry*& rpNewParent, sal_uLong& rNewChildPos )
{
    return pImp->NotifyMoving(pTarget,pEntry,rpNewParent,rNewChildPos);
}

sal_Bool SvIconView::NotifyCopying( SvLBoxEntry* pTarget, SvLBoxEntry* pEntry,
    SvLBoxEntry*& rpNewParent, sal_uLong& rNewChildPos )
{
    return pImp->NotifyCopying(pTarget,pEntry,rpNewParent,rNewChildPos);
}


void SvIconView::EnableInplaceEditing( sal_Bool bEnable )
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
            SelectAll( sal_False );
            EditItemText( pEntry, (SvLBoxString*)pItem, aSel );
        }
    }
}


void SvIconView::EditItemText( SvLBoxEntry* pEntry, SvLBoxItem* pItem,
    const Selection& rSel )
{
    DBG_ASSERT(pEntry&&pItem,"EditItemText:Params?");
    pCurEdEntry = pEntry;
    pCurEdItem = pItem;
    Rectangle aRect( pImp->CalcTextRect( pEntry, (SvLBoxString*)pItem,0,sal_True ));

    aRect.Bottom() += 4;
    pImp->MakeVisible( aRect ); // vor der Umrechnung in Pixel-Koord. rufen!
    aRect.Bottom() -= 4;

    Point aPos( aRect.TopLeft() );
    aPos += GetMapMode().GetOrigin(); // Dok-Koord. -> Window-Koord.
    aRect.SetPos( aPos );

    aRect.Bottom() += 2; // sieht huebscher aus

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
    EditText( ((SvLBoxString*)pItem)->GetText(), aRect, rSel, sal_True );
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
                SelectAll( sal_False );
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


sal_Bool SvIconView::EditingEntry( SvLBoxEntry*, Selection& )
{
    return sal_True;
}

sal_Bool SvIconView::EditedEntry( SvLBoxEntry*, const XubString& )
{
    return sal_True;
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
    pImp->PrepareCommandEvent( rCEvt.GetMousePosPixel() );
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

SvViewData* SvIconView::CreateViewData( SvListEntry* )
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

sal_uLong SvIconView::GetSelectionCount() const
{
    return (sal_uLong)(pImp->GetSelectionCount());
}

void SvIconView::SetGrid( long nDX, long nDY )
{
    pImp->SetGrid( nDX, nDY );
}

void SvIconView::ModelNotification( sal_uInt16 nActionId, SvListEntry* pEntry1,
    SvListEntry* pEntry2, sal_uLong nPos )
{
    SvLBox::ModelNotification( nActionId, pEntry1, pEntry2, nPos );
    switch( nActionId )
    {
        case LISTACTION_RESORTING:
            SetUpdateMode( sal_False );
            break;

        case LISTACTION_RESORTED:
            SetUpdateMode( sal_True );
            Arrange();
            break;

        case LISTACTION_CLEARED:
            if( IsUpdateMode() )
                Update();
            break;
    }
}


void SvIconView::Scroll( long nDeltaX, long nDeltaY, sal_uInt16 )
{
    pImp->Scroll( nDeltaX, nDeltaY, sal_False );
}

void SvIconView::PrepareCommandEvent( const CommandEvent& rCEvt )
{
    pImp->PrepareCommandEvent( rCEvt.GetMousePosPixel() );
}

void SvIconView::StartDrag( sal_Int8 nAction, const Point& rPos )
{
    pImp->SttDrag( rPos );
    SvLBoxEntry* pEntry = GetEntry( rPos, sal_True );
    pImp->mpViewData = pEntry;
    SvLBox::StartDrag( nAction, rPos );
}

void SvIconView::DragFinished( sal_Int8 )
{
    pImp->EndDrag();
}

sal_Int8 SvIconView::AcceptDrop( const AcceptDropEvent& rEvt )
{
    if( pImp->mpViewData )
        pImp->HideDDIcon();
    sal_Int8 nRet = SvLBox::AcceptDrop( rEvt );
    if( DND_ACTION_NONE != nRet )
        pImp->ShowDDIcon( pImp->mpViewData, rEvt.maPosPixel );

    return nRet;
}

sal_Int8 SvIconView::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    if( pImp->mpViewData )
    {
        pImp->HideDDIcon();
        pImp->mpViewData = 0;
    }
    return SvLBox::ExecuteDrop( rEvt );
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

void SvIconView::SelectRect( const Rectangle& rRect, sal_Bool bAdd,
    SvPtrarr* pRects, short nOffs )
{
    pImp->SelectRect( rRect, bAdd, pRects, nOffs );
}

void SvIconView::CalcScrollOffsets( const Point& rRefPosPixel, long& rX, long& rY,
    sal_Bool b, sal_uInt16 nBorderWidth )
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

void SvIconView::LockEntryPos( SvLBoxEntry* pEntry, sal_Bool bLock )
{
    SvIcnVwDataEntry* pViewData = (SvIcnVwDataEntry*)GetViewData( pEntry );
    if( bLock )
        pViewData->SetVwFlags( ICNVW_FLAG_POS_LOCKED );
    else
        pViewData->ClearVwFlags( ICNVW_FLAG_POS_LOCKED );
}

sal_Bool SvIconView::IsEntryPosLocked( const SvLBoxEntry* pEntry ) const
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

SvLBoxEntry* SvIconView::GetNextEntry( const Point& rPixPos, SvLBoxEntry* pCurEntry, sal_Bool  ) const
{
    Point aPos( rPixPos );
    aPos -= GetMapMode().GetOrigin();
    return ((SvIconView*)this)->pImp->GetNextEntry( aPos, pCurEntry );
}

SvLBoxEntry* SvIconView::GetPrevEntry( const Point& rPixPos, SvLBoxEntry* pCurEntry, sal_Bool  ) const
{
    Point aPos( rPixPos );
    aPos -= GetMapMode().GetOrigin();
    return ((SvIconView*)this)->pImp->GetPrevEntry( aPos, pCurEntry );
}

void SvIconView::ShowFocusRect( const SvLBoxEntry* pEntry )
{
    pImp->ShowFocusRect( pEntry );
}


