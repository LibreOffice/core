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


#include <limits.h>
#include <tools/debug.hxx>
#include <vcl/wall.hxx>
#include <vcl/help.hxx>
#include <vcl/decoview.hxx>
#include <vcl/svapp.hxx>
#include <tools/poly.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/controllayout.hxx>

#include <svtools/ivctrl.hxx>
#include "imivctl.hxx"
#include <svtools/svmedit.hxx>

#include <algorithm>
#include <memory>

#define IMPICNVIEW_ACC_RETURN 1
#define IMPICNVIEW_ACC_ESCAPE 2

#define DRAWTEXT_FLAGS_ICON \
    ( TEXT_DRAW_CENTER | TEXT_DRAW_TOP | TEXT_DRAW_ENDELLIPSIS | \
      TEXT_DRAW_CLIP | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK | TEXT_DRAW_MNEMONIC )

#define DRAWTEXT_FLAGS_SMALLICON (TEXT_DRAW_LEFT|TEXT_DRAW_ENDELLIPSIS|TEXT_DRAW_CLIP)

#define EVENTID_SHOW_CURSOR             ((void*)1)
#define EVENTID_ADJUST_SCROLLBARS       ((void*)2)

struct SvxIconChoiceCtrlEntry_Impl
{
    SvxIconChoiceCtrlEntry* _pEntry;
    Point           _aPos;
                    SvxIconChoiceCtrlEntry_Impl( SvxIconChoiceCtrlEntry* pEntry, const Rectangle& rBoundRect )
                    : _pEntry( pEntry), _aPos( rBoundRect.TopLeft()) {}
};

static sal_Bool bEndScrollInvalidate = sal_True;

class IcnViewEdit_Impl : public MultiLineEdit
{
    Link            aCallBackHdl;
    Accelerator     aAccReturn;
    Accelerator     aAccEscape;
    Timer           aTimer;
    sal_Bool            bCanceled;
    sal_Bool            bAlreadyInCallback;
    sal_Bool            bGrabFocus;

    void            CallCallBackHdl_Impl();
                    DECL_LINK(Timeout_Impl, void *);
                    DECL_LINK( ReturnHdl_Impl, Accelerator * );
                    DECL_LINK( EscapeHdl_Impl, Accelerator * );

public:

                    IcnViewEdit_Impl(
                        SvtIconChoiceCtrl* pParent,
                        const Point& rPos,
                        const Size& rSize,
                        const OUString& rData,
                        const Link& rNotifyEditEnd );

                    ~IcnViewEdit_Impl();
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual long    PreNotify( NotifyEvent& rNEvt );
    sal_Bool            EditingCanceled() const { return bCanceled; }
    void            StopEditing( sal_Bool bCancel = sal_False );
    sal_Bool            IsGrabFocus() const { return bGrabFocus; }
};

SvxIconChoiceCtrl_Impl::SvxIconChoiceCtrl_Impl(
    SvtIconChoiceCtrl* pCurView,
    WinBits nWinStyle
) :
    aEntries( this ),
    aVerSBar( pCurView, WB_DRAG | WB_VSCROLL ),
    aHorSBar( pCurView, WB_DRAG | WB_HSCROLL ),
    aScrBarBox( pCurView ),
    aImageSize( 32, 32 ),
    pColumns( 0 )
{
    bChooseWithCursor=sal_False;
    pEntryPaintDev = 0;
    pCurEditedEntry = 0;
    pCurHighlightFrame = 0;
    pEdit = 0;
    pAnchor = 0;
    pPrevDropTarget = 0;
    pHdlEntry = 0;
    pHead = NULL;
    pCursor = NULL;
    bUpdateMode = sal_True;
    bEntryEditingEnabled = sal_False;
    bHighlightFramePressed = sal_False;
    eSelectionMode = MULTIPLE_SELECTION;
    pView = pCurView;
    pZOrderList = new SvxIconChoiceCtrlEntryList_impl();
    ePositionMode = IcnViewPositionModeFree;
    SetStyle( nWinStyle );
    nFlags = 0;
    nUserEventAdjustScrBars = 0;
    nUserEventShowCursor = 0;
    nMaxVirtWidth = DEFAULT_MAX_VIRT_WIDTH;
    nMaxVirtHeight = DEFAULT_MAX_VIRT_HEIGHT;
    pDDRefEntry = 0;
    pDDDev = 0;
    pDDBufDev = 0;
    pDDTempDev = 0;
    eTextMode = IcnShowTextShort;
    pImpCursor = new IcnCursor_Impl( this );
    pGridMap = new IcnGridMap_Impl( this );

    aVerSBar.SetScrollHdl( LINK( this, SvxIconChoiceCtrl_Impl, ScrollUpDownHdl ) );
    aHorSBar.SetScrollHdl( LINK( this, SvxIconChoiceCtrl_Impl, ScrollLeftRightHdl ) );
    Link aEndScrollHdl( LINK( this, SvxIconChoiceCtrl_Impl, EndScrollHdl ) );
    aVerSBar.SetEndScrollHdl( aEndScrollHdl );
    aHorSBar.SetEndScrollHdl( aEndScrollHdl );

    nHorSBarHeight = aHorSBar.GetSizePixel().Height();
    nVerSBarWidth = aVerSBar.GetSizePixel().Width();

    aEditTimer.SetTimeout( 800 );
    aEditTimer.SetTimeoutHdl(LINK(this,SvxIconChoiceCtrl_Impl,EditTimeoutHdl));
    aAutoArrangeTimer.SetTimeout( 100 );
    aAutoArrangeTimer.SetTimeoutHdl(LINK(this,SvxIconChoiceCtrl_Impl,AutoArrangeHdl));
    aCallSelectHdlTimer.SetTimeout( 500 );
    aCallSelectHdlTimer.SetTimeoutHdl( LINK(this,SvxIconChoiceCtrl_Impl,CallSelectHdlHdl));

    aDocRectChangedTimer.SetTimeout( 50 );
    aDocRectChangedTimer.SetTimeoutHdl(LINK(this,SvxIconChoiceCtrl_Impl,DocRectChangedHdl));
    aVisRectChangedTimer.SetTimeout( 50 );
    aVisRectChangedTimer.SetTimeoutHdl(LINK(this,SvxIconChoiceCtrl_Impl,VisRectChangedHdl));

    Clear( sal_True );

    SetGrid( Size(100, 70) );
}

SvxIconChoiceCtrl_Impl::~SvxIconChoiceCtrl_Impl()
{
    pCurEditedEntry = 0;
    DELETEZ(pEdit);
    Clear();
    StopEditTimer();
    CancelUserEvents();
    delete pZOrderList;
    delete pImpCursor;
    delete pGridMap;
    delete pDDDev;
    delete pDDBufDev;
    delete pDDTempDev;
    delete pEntryPaintDev;
    ClearSelectedRectList();
    ClearColumnList();
}

void SvxIconChoiceCtrl_Impl::Clear( sal_Bool bInCtor )
{
    StopEntryEditing( sal_True );
    nSelectionCount = 0;
    pCurHighlightFrame = 0;
    StopEditTimer();
    CancelUserEvents();
    ShowCursor( sal_False );
    bBoundRectsDirty = sal_False;
    nMaxBoundHeight = 0;

    nFlags &= ~(F_PAINTED | F_MOVED_ENTRIES);
    pCursor = 0;
    if( !bInCtor )
    {
        pImpCursor->Clear();
        pGridMap->Clear();
        aVirtOutputSize.Width() = 0;
        aVirtOutputSize.Height() = 0;
        Size aSize( pView->GetOutputSizePixel() );
        nMaxVirtWidth = aSize.Width() - nVerSBarWidth;
        if( nMaxVirtWidth <= 0 )
            nMaxVirtWidth = DEFAULT_MAX_VIRT_WIDTH;
        nMaxVirtHeight = aSize.Height() - nHorSBarHeight;
        if( nMaxVirtHeight <= 0 )
            nMaxVirtHeight = DEFAULT_MAX_VIRT_HEIGHT;
        pZOrderList->clear();
        SetOrigin( Point() );
        if( bUpdateMode )
            pView->Invalidate(INVALIDATE_NOCHILDREN);
    }
    AdjustScrollBars();
    size_t nCount = aEntries.size();
    for( size_t nCur = 0; nCur < nCount; nCur++ )
    {
        SvxIconChoiceCtrlEntry* pCur = aEntries[ nCur ];
        delete pCur;
    }
    aEntries.clear();
    DocRectChanged();
    VisRectChanged();
}

void SvxIconChoiceCtrl_Impl::SetStyle( WinBits nWinStyle )
{
    nWinBits = nWinStyle;
    nCurTextDrawFlags = DRAWTEXT_FLAGS_ICON;
    if( nWinBits & (WB_SMALLICON | WB_DETAILS) )
        nCurTextDrawFlags = DRAWTEXT_FLAGS_SMALLICON;
    if( nWinBits & WB_NOSELECTION )
        eSelectionMode = NO_SELECTION;
    if( !(nWinStyle & (WB_ALIGN_TOP | WB_ALIGN_LEFT)))
        nWinBits |= WB_ALIGN_LEFT;
    if( (nWinStyle & WB_DETAILS))
    {
        if( !pColumns  )
            SetColumn( 0, SvxIconChoiceCtrlColumnInfo( 0, 100, IcnViewAlignLeft ));
    }
}

IMPL_LINK( SvxIconChoiceCtrl_Impl, ScrollUpDownHdl, ScrollBar*, pScrollBar )
{
    StopEntryEditing( sal_True );
    // arrow up: delta=-1; arrow down: delta=+1
    Scroll( 0, pScrollBar->GetDelta(), sal_True );
    bEndScrollInvalidate = sal_True;
    return 0;
}

IMPL_LINK( SvxIconChoiceCtrl_Impl, ScrollLeftRightHdl, ScrollBar*, pScrollBar )
{
    StopEntryEditing( sal_True );
    // arrow left: delta=-1; arrow right: delta=+1
    Scroll( pScrollBar->GetDelta(), 0, sal_True );
    bEndScrollInvalidate = sal_True;
    return 0;
}

IMPL_LINK_NOARG(SvxIconChoiceCtrl_Impl, EndScrollHdl)
{
    if( pView->HasBackground() && !pView->GetBackground().IsScrollable() &&
        bEndScrollInvalidate )
    {
        pView->Invalidate(INVALIDATE_NOCHILDREN);
    }
    return 0;
}

void SvxIconChoiceCtrl_Impl::FontModified()
{
    StopEditTimer();
    DELETEZ(pDDDev);
    DELETEZ(pDDBufDev);
    DELETEZ(pDDTempDev);
    DELETEZ(pEntryPaintDev);
    SetDefaultTextSize();
    ShowCursor( sal_False );
    ShowCursor( sal_True );
}

void SvxIconChoiceCtrl_Impl::InsertEntry( SvxIconChoiceCtrlEntry* pEntry, size_t nPos,
    const Point* pPos )
{
    StopEditTimer();
    aEntries.insert( nPos, pEntry );
    if( (nFlags & F_ENTRYLISTPOS_VALID) && nPos >= aEntries.size() - 1 )
        pEntry->nPos = aEntries.size() - 1;
    else
        nFlags &= ~F_ENTRYLISTPOS_VALID;

    pZOrderList->push_back( pEntry );
    pImpCursor->Clear();
    if( pPos )
    {
        Size aSize( CalcBoundingSize( pEntry ) );
        SetBoundingRect_Impl( pEntry, *pPos, aSize );
        SetEntryPos( pEntry, *pPos, sal_False, sal_True, sal_True /*keep grid map*/ );
        pEntry->nFlags |= ICNVIEW_FLAG_POS_MOVED;
        SetEntriesMoved( sal_True );
    }
    else
    {
        // If the UpdateMode is sal_True, don't set all bounding rectangles to
        // 'to be checked', but only the bounding rectangle of the new entry.
        // Thus, don't call InvalidateBoundingRect!
        pEntry->aRect.Right() = LONG_MAX;
        if( bUpdateMode )
        {
            FindBoundingRect( pEntry );
            Rectangle aOutputArea( GetOutputRect() );
            pGridMap->OccupyGrids( pEntry );
            if( !aOutputArea.IsOver( pEntry->aRect ) )
                return; // is invisible
            pView->Invalidate( pEntry->aRect );
        }
        else
            InvalidateBoundingRect( pEntry->aRect );
    }
}

void SvxIconChoiceCtrl_Impl::CreateAutoMnemonics( MnemonicGenerator* _pGenerator )
{
    ::std::auto_ptr< MnemonicGenerator > pAutoDeleteOwnGenerator;
    if ( !_pGenerator )
    {
        _pGenerator = new MnemonicGenerator;
        pAutoDeleteOwnGenerator.reset( _pGenerator );
    }

    sal_uLong   nEntryCount = GetEntryCount();
    sal_uLong   i;

    // insert texts in generator
    for( i = 0; i < nEntryCount; ++i )
    {
        DBG_ASSERT( GetEntry( i ), "-SvxIconChoiceCtrl_Impl::CreateAutoMnemonics(): more expected than provided!" );

        _pGenerator->RegisterMnemonic( GetEntry( i )->GetText() );
    }

    // exchange texts with generated mnemonics
    for( i = 0; i < nEntryCount; ++i )
    {
        SvxIconChoiceCtrlEntry* pEntry = GetEntry( i );
        OUString                aTxt = pEntry->GetText();

        OUString aNewText = _pGenerator->CreateMnemonic( aTxt );
        if( aNewText != aTxt )
            pEntry->SetText( aNewText );
    }
}

Rectangle SvxIconChoiceCtrl_Impl::GetOutputRect() const
{
    Point aOrigin( pView->GetMapMode().GetOrigin() );
    aOrigin *= -1;
    return Rectangle( aOrigin, aOutputSize );
}

void SvxIconChoiceCtrl_Impl::SetListPositions()
{
    if( nFlags & F_ENTRYLISTPOS_VALID )
        return;

    size_t nCount = aEntries.size();
    for( size_t nCur = 0; nCur < nCount; nCur++ )
    {
        SvxIconChoiceCtrlEntry* pEntry = aEntries[ nCur ];
        pEntry->nPos = nCur;
    }
    nFlags |= F_ENTRYLISTPOS_VALID;
}

void SvxIconChoiceCtrl_Impl::SelectEntry( SvxIconChoiceCtrlEntry* pEntry, sal_Bool bSelect,
    sal_Bool bCallHdl, sal_Bool bAdd, sal_Bool bSyncPaint )
{
    if( eSelectionMode == NO_SELECTION )
        return;

    if( !bAdd )
    {
        if ( 0 == ( nFlags & F_CLEARING_SELECTION ) )
        {
            nFlags |= F_CLEARING_SELECTION;
            DeselectAllBut( pEntry, sal_True );
            nFlags &= ~F_CLEARING_SELECTION;
        }
    }
    if( pEntry->IsSelected() != bSelect )
    {
        pHdlEntry = pEntry;
        sal_uInt16 nEntryFlags = pEntry->GetFlags();
        if( bSelect )
        {
            nEntryFlags |= ICNVIEW_FLAG_SELECTED;
            pEntry->AssignFlags( nEntryFlags );
            nSelectionCount++;
            if( bCallHdl )
                CallSelectHandler( pEntry );
        }
        else
        {
            nEntryFlags &= ~( ICNVIEW_FLAG_SELECTED);
            pEntry->AssignFlags( nEntryFlags );
            nSelectionCount--;
            if( bCallHdl )
                CallSelectHandler( 0 );
        }
        EntrySelected( pEntry, bSelect, bSyncPaint );
    }
}

void SvxIconChoiceCtrl_Impl::EntrySelected( SvxIconChoiceCtrlEntry* pEntry, sal_Bool bSelect,
    sal_Bool bSyncPaint )
{
    // When using SingleSelection, make sure that the cursor is always placed
    // over the (only) selected entry. (But only if a cursor exists.)
    if( bSelect && pCursor &&
        eSelectionMode == SINGLE_SELECTION &&
        pEntry != pCursor )
    {
        SetCursor( pEntry );
        //DBG_ASSERT(pView->GetSelectionCount()==1,"selection count?")
    }

    // Not when dragging though, else the loop in SelectRect doesn't work
    // correctly!
    if( !(nFlags & F_SELECTING_RECT) )
        ToTop( pEntry );
    if( bUpdateMode )
    {
        if( pEntry == pCursor )
            ShowCursor( sal_False );
        if( pView->IsTracking() && (bSelect || !pView->HasBackground()) ) // always synchronous when tracking
            PaintEntry( pEntry );
        else if( bSyncPaint ) // synchronous & with a virtual OutDev!
            PaintEntryVirtOutDev( pEntry );
        else
        {
            pView->Invalidate( CalcFocusRect( pEntry ) );
        }
        if( pEntry == pCursor )
            ShowCursor( sal_True );
    }

    // #i101012# emit vcl event LISTBOX_SELECT only in case that the given entry is selected.
    if ( bSelect )
    {
        CallEventListeners( VCLEVENT_LISTBOX_SELECT, pEntry );
    }
}

void SvxIconChoiceCtrl_Impl::ResetVirtSize()
{
    StopEditTimer();
    aVirtOutputSize.Width() = 0;
    aVirtOutputSize.Height() = 0;
    const size_t nCount = aEntries.size();
    for( size_t nCur = 0; nCur < nCount; nCur++ )
    {
        SvxIconChoiceCtrlEntry* pCur = aEntries[ nCur ];
        pCur->ClearFlags( ICNVIEW_FLAG_POS_MOVED );
        if( pCur->IsPosLocked() )
        {
            // adapt (among others) VirtSize
            if( !IsBoundingRectValid( pCur->aRect ) )
                FindBoundingRect( pCur );
            else
                AdjustVirtSize( pCur->aRect );
        }
        else
            InvalidateBoundingRect( pCur->aRect );
    }

    if( !(nWinBits & (WB_NOVSCROLL | WB_NOHSCROLL)) )
    {
        Size aRealOutputSize( pView->GetOutputSizePixel() );
        if( aVirtOutputSize.Width() < aRealOutputSize.Width() ||
            aVirtOutputSize.Height() < aRealOutputSize.Height() )
        {
            sal_uLong nGridCount = IcnGridMap_Impl::GetGridCount(
                aRealOutputSize, (sal_uInt16)nGridDX, (sal_uInt16)nGridDY );
            if( nGridCount < nCount )
            {
                if( nWinBits & WB_ALIGN_TOP )
                    nMaxVirtWidth = aRealOutputSize.Width() - nVerSBarWidth;
                else // WB_ALIGN_LEFT
                    nMaxVirtHeight = aRealOutputSize.Height() - nHorSBarHeight;
            }
        }
    }

    pImpCursor->Clear();
    pGridMap->Clear();
    VisRectChanged();
}

void SvxIconChoiceCtrl_Impl::AdjustVirtSize( const Rectangle& rRect )
{
    long nHeightOffs = 0;
    long nWidthOffs = 0;

    if( aVirtOutputSize.Width() < (rRect.Right()+LROFFS_WINBORDER) )
        nWidthOffs = (rRect.Right()+LROFFS_WINBORDER) - aVirtOutputSize.Width();

    if( aVirtOutputSize.Height() < (rRect.Bottom()+TBOFFS_WINBORDER) )
        nHeightOffs = (rRect.Bottom()+TBOFFS_WINBORDER) - aVirtOutputSize.Height();

    if( nWidthOffs || nHeightOffs )
    {
        Range aRange;
        aVirtOutputSize.Width() += nWidthOffs;
        aRange.Max() = aVirtOutputSize.Width();
        aHorSBar.SetRange( aRange );

        aVirtOutputSize.Height() += nHeightOffs;
        aRange.Max() = aVirtOutputSize.Height();
        aVerSBar.SetRange( aRange );

        pImpCursor->Clear();
        pGridMap->OutputSizeChanged();
        AdjustScrollBars();
        DocRectChanged();
    }
}

void SvxIconChoiceCtrl_Impl::InitPredecessors()
{
    DBG_ASSERT(!pHead,"SvxIconChoiceCtrl_Impl::InitPredecessors() >> Already initialized");
    size_t nCount = aEntries.size();
    if( nCount )
    {
        SvxIconChoiceCtrlEntry* pPrev = aEntries[ 0 ];
        for( size_t nCur = 1; nCur <= nCount; nCur++ )
        {
            pPrev->ClearFlags( ICNVIEW_FLAG_POS_LOCKED | ICNVIEW_FLAG_POS_MOVED |
                                ICNVIEW_FLAG_PRED_SET);

            SvxIconChoiceCtrlEntry* pNext;
            if( nCur == nCount )
                pNext = aEntries[ 0 ];
            else
                pNext = aEntries[ nCur ];
            pPrev->pflink = pNext;
            pNext->pblink = pPrev;
            pPrev = pNext;
        }
        pHead = aEntries[ 0 ];
    }
    else
        pHead = 0;
    nFlags &= ~F_MOVED_ENTRIES;
}

void SvxIconChoiceCtrl_Impl::ClearPredecessors()
{
    if( pHead )
    {
        size_t nCount = aEntries.size();
        for( size_t nCur = 0; nCur < nCount; nCur++ )
        {
            SvxIconChoiceCtrlEntry* pCur = aEntries[ nCur ];
            pCur->pflink = 0;
            pCur->pblink = 0;
            pCur->ClearFlags( ICNVIEW_FLAG_PRED_SET );
        }
        pHead = 0;
    }
}

void SvxIconChoiceCtrl_Impl::Arrange( sal_Bool bKeepPredecessors, long nSetMaxVirtWidth, long nSetMaxVirtHeight )
{
    if ( nSetMaxVirtWidth != 0 )
        nMaxVirtWidth = nSetMaxVirtWidth;
    else
        nMaxVirtWidth = aOutputSize.Width();

    if ( nSetMaxVirtHeight != 0 )
        nMaxVirtHeight = nSetMaxVirtHeight;
    else
        nMaxVirtHeight = aOutputSize.Height();

    ImpArrange( bKeepPredecessors );
}

void SvxIconChoiceCtrl_Impl::ImpArrange( sal_Bool bKeepPredecessors )
{
    static Point aEmptyPoint;

    sal_Bool bOldUpdate = bUpdateMode;
    Rectangle aCurOutputArea( GetOutputRect() );
    if( (nWinBits & WB_SMART_ARRANGE) && aCurOutputArea.TopLeft() != aEmptyPoint )
        bUpdateMode = sal_False;
    aAutoArrangeTimer.Stop();
    nFlags &= ~F_MOVED_ENTRIES;
    nFlags |= F_ARRANGING;
    StopEditTimer();
    ShowCursor( sal_False );
    ResetVirtSize();
    if( !bKeepPredecessors )
        ClearPredecessors();
    bBoundRectsDirty = sal_False;
    SetOrigin( Point() );
    VisRectChanged();
    RecalcAllBoundingRectsSmart();
    // TODO: the invalidation in the detail view should be more intelligent
    //if( !(nWinBits & WB_DETAILS ))
        pView->Invalidate( INVALIDATE_NOCHILDREN );
    nFlags &= ~F_ARRANGING;
    if( (nWinBits & WB_SMART_ARRANGE) && aCurOutputArea.TopLeft() != aEmptyPoint )
    {
        MakeVisible( aCurOutputArea );
        SetUpdateMode( bOldUpdate );
    }
    ShowCursor( sal_True );
}

void SvxIconChoiceCtrl_Impl::Paint( const Rectangle& rRect )
{
    bEndScrollInvalidate = sal_False;

#if defined(OV_DRAWGRID)
    Color aOldColor ( pView->GetLineColor() );
    Color aColor( COL_BLACK );
    pView->SetLineColor( aColor );
    Point aOffs( pView->GetMapMode().GetOrigin());
    Size aXSize( pView->GetOutputSizePixel() );

    {
    Point aStart( LROFFS_WINBORDER, 0 );
    Point aEnd( LROFFS_WINBORDER, aXSize.Height());
    aStart -= aOffs;
    aEnd -= aOffs;
    pView->DrawLine( aStart, aEnd );
    }
    {
    Point aStart( 0, TBOFFS_WINBORDER );
    Point aEnd( aXSize.Width(), TBOFFS_WINBORDER );
    aStart -= aOffs;
    aEnd -= aOffs;
    pView->DrawLine( aStart, aEnd );
    }

    for( long nDX = nGridDX; nDX <= aXSize.Width(); nDX += nGridDX )
    {
        Point aStart( nDX+LROFFS_WINBORDER, 0 );
        Point aEnd( nDX+LROFFS_WINBORDER, aXSize.Height());
        aStart -= aOffs;
        aEnd -= aOffs;
        pView->DrawLine( aStart, aEnd );
    }
    for( long nDY = nGridDY; nDY <= aXSize.Height(); nDY += nGridDY )
    {
        Point aStart( 0, nDY+TBOFFS_WINBORDER );
        Point aEnd( aXSize.Width(), nDY+TBOFFS_WINBORDER );
        aStart -= aOffs;
        aEnd -= aOffs;
        pView->DrawLine( aStart, aEnd );
    }
    pView->SetLineColor( aOldColor );
#endif
    nFlags |= F_PAINTED;

    if( !aEntries.size() )
        return;
    if( !pCursor )
    {
        // set cursor to item with focus-flag
        sal_Bool bfound = sal_False;
        for ( sal_uLong i = 0; i < pView->GetEntryCount() && !bfound; i++)
        {
            SvxIconChoiceCtrlEntry* pEntry = pView->GetEntry ( i );
            if( pEntry->IsFocused() )
            {
                pCursor = pEntry;
                bfound=sal_True;
            }
        }

        if( !bfound )
            pCursor = aEntries[ 0 ];
    }

    // Show Focus at Init-Time
    if ( pView->HasFocus() )
        GetFocus();

    size_t nCount = pZOrderList->size();
    if( !nCount )
        return;

    sal_Bool bResetClipRegion = sal_False;
    if( !pView->IsClipRegion() )
    {
        Region const aOutputArea( GetOutputRect() );
        bResetClipRegion = sal_True;
        pView->SetClipRegion( aOutputArea );
    }

    SvxIconChoiceCtrlEntryList_impl* pNewZOrderList = new SvxIconChoiceCtrlEntryList_impl();
    SvxIconChoiceCtrlEntryList_impl* pPaintedEntries = new SvxIconChoiceCtrlEntryList_impl();

    size_t nPos = 0;
    while( nCount )
    {
        SvxIconChoiceCtrlEntry* pEntry = (*pZOrderList)[ nPos ];
        const Rectangle& rBoundRect = GetEntryBoundRect( pEntry );
        if( rRect.IsOver( rBoundRect ) )
        {
            PaintEntry( pEntry, rBoundRect.TopLeft(), pView, sal_True );
            // set entries to Top if they are being repainted
            pPaintedEntries->push_back( pEntry );
        }
        else
            pNewZOrderList->push_back( pEntry );

        nCount--;
        nPos++;
    }
    delete pZOrderList;
    pZOrderList = pNewZOrderList;
    nCount = pPaintedEntries->size();
    if( nCount )
    {
        for( size_t nCur = 0; nCur < nCount; nCur++ )
            pZOrderList->push_back( (*pPaintedEntries)[ nCur ] );
    }
    delete pPaintedEntries;

    if( bResetClipRegion )
        pView->SetClipRegion();
}

void SvxIconChoiceCtrl_Impl::RepaintEntries( sal_uInt16 nEntryFlagsMask )
{
    const size_t nCount = pZOrderList->size();
    if( !nCount )
        return;

    sal_Bool bResetClipRegion = sal_False;
    Rectangle aOutRect( GetOutputRect() );
    if( !pView->IsClipRegion() )
    {
        bResetClipRegion = sal_True;
        pView->SetClipRegion(Region(aOutRect));
    }
    for( size_t nCur = 0; nCur < nCount; nCur++ )
    {
        SvxIconChoiceCtrlEntry* pEntry = (*pZOrderList)[ nCur ];
        if( pEntry->GetFlags() & nEntryFlagsMask )
        {
            const Rectangle& rBoundRect = GetEntryBoundRect( pEntry );
            if( aOutRect.IsOver( rBoundRect ) )
                PaintEntry( pEntry, rBoundRect.TopLeft() );
        }
    }
    if( bResetClipRegion )
        pView->SetClipRegion();
}


void SvxIconChoiceCtrl_Impl::InitScrollBarBox()
{
    aScrBarBox.SetSizePixel( Size(nVerSBarWidth-1, nHorSBarHeight-1) );
    Size aSize( pView->GetOutputSizePixel() );
    aScrBarBox.SetPosPixel( Point(aSize.Width()-nVerSBarWidth+1, aSize.Height()-nHorSBarHeight+1));
}

sal_Bool SvxIconChoiceCtrl_Impl::MouseButtonDown( const MouseEvent& rMEvt)
{
    sal_Bool bHandled = sal_True;
    bHighlightFramePressed = sal_False;
    StopEditTimer();
    sal_Bool bGotFocus = (sal_Bool)(!pView->HasFocus() && !(nWinBits & WB_NOPOINTERFOCUS));
    if( !(nWinBits & WB_NOPOINTERFOCUS) )
        pView->GrabFocus();

    Point aDocPos( rMEvt.GetPosPixel() );
    if(aDocPos.X()>=aOutputSize.Width() || aDocPos.Y()>=aOutputSize.Height())
        return sal_False;
    ToDocPos( aDocPos );
    SvxIconChoiceCtrlEntry* pEntry = GetEntry( aDocPos, sal_True );
    if( pEntry )
        MakeEntryVisible( pEntry, sal_False );

    if( rMEvt.IsShift() && eSelectionMode != SINGLE_SELECTION )
    {
        if( pEntry )
            SetCursor_Impl( pCursor, pEntry, rMEvt.IsMod1(), rMEvt.IsShift(), sal_True);
        return sal_True;
    }

    if( pAnchor && (rMEvt.IsShift() || rMEvt.IsMod1())) // keyboard selection?
    {
        DBG_ASSERT(eSelectionMode != SINGLE_SELECTION,"Invalid selection mode");
        if( rMEvt.IsMod1() )
            nFlags |= F_ADD_MODE;

        if( rMEvt.IsShift() )
        {
            Rectangle aRect( GetEntryBoundRect( pAnchor ));
            if( pEntry )
                aRect.Union( GetEntryBoundRect( pEntry ) );
            else
            {
                Rectangle aTempRect( aDocPos, Size(1,1));
                aRect.Union( aTempRect );
            }
            aCurSelectionRect = aRect;
            SelectRect( aRect, (nFlags & F_ADD_MODE)!=0, &aSelectedRectList );
        }
        else if( rMEvt.IsMod1() )
        {
            AddSelectedRect( aCurSelectionRect );
            pAnchor = 0;
            aCurSelectionRect.SetPos( aDocPos );
        }

        if( !pEntry && !(nWinBits & WB_NODRAGSELECTION))
            pView->StartTracking( STARTTRACK_SCROLLREPEAT );
        return sal_True;
    }
    else
    {
        if( !pEntry )
        {
            if( eSelectionMode == MULTIPLE_SELECTION )
            {
                if( !rMEvt.IsMod1() )  // Ctrl
                {
                    if( !bGotFocus )
                    {
                        SetNoSelection();
                        ClearSelectedRectList();
                    }
                }
                else
                    nFlags |= F_ADD_MODE;
                aCurSelectionRect.SetPos( aDocPos );
                pView->StartTracking( STARTTRACK_SCROLLREPEAT );
            }
            else
                bHandled = sal_False;
            return bHandled;
        }
    }
    sal_Bool bSelected = pEntry->IsSelected();
    sal_Bool bEditingEnabled = IsEntryEditingEnabled();

    if( rMEvt.GetClicks() == 2 )
    {
        DeselectAllBut( pEntry );
        SelectEntry( pEntry, sal_True, sal_True, sal_False, sal_True );
        pHdlEntry = pEntry;
        pView->ClickIcon();
    }
    else
    {
        // Inplace-Editing ?
        if( rMEvt.IsMod2() )  // Alt?
        {
            if( bEntryEditingEnabled && pEntry &&
                pEntry->IsSelected())
            {
                if( pView->EditingEntry( pEntry ))
                    EditEntry( pEntry );
            }
        }
        else if( eSelectionMode == SINGLE_SELECTION )
        {
            DeselectAllBut( pEntry );
            SetCursor( pEntry );
            if( bEditingEnabled && bSelected && !rMEvt.GetModifier() &&
                rMEvt.IsLeft() && IsTextHit( pEntry, aDocPos ) )
            {
                nFlags |= F_START_EDITTIMER_IN_MOUSEUP;
            }
        }
        else if( eSelectionMode == NO_SELECTION )
        {
            if( rMEvt.IsLeft() && (nWinBits & WB_HIGHLIGHTFRAME) )
            {
                pCurHighlightFrame = 0; // force repaint of frame
                bHighlightFramePressed = sal_True;
                SetEntryHighlightFrame( pEntry, sal_True );
            }
        }
        else
        {
            if( !rMEvt.GetModifier() && rMEvt.IsLeft() )
            {
                if( !bSelected )
                {
                    DeselectAllBut( pEntry, sal_True /* paint synchronously */ );
                    SetCursor( pEntry );
                    SelectEntry( pEntry, sal_True, sal_True, sal_False, sal_True );
                }
                else
                {
                    // deselect only in the Up, if the Move happened via D&D!
                    nFlags |= F_DOWN_DESELECT;
                    if( bEditingEnabled && IsTextHit( pEntry, aDocPos ) &&
                        rMEvt.IsLeft())
                    {
                        nFlags |= F_START_EDITTIMER_IN_MOUSEUP;
                    }
                }
            }
            else if( rMEvt.IsMod1() )
                nFlags |= F_DOWN_CTRL;
        }
    }
    return bHandled;
}

sal_Bool SvxIconChoiceCtrl_Impl::MouseButtonUp( const MouseEvent& rMEvt )
{
    sal_Bool bHandled = sal_False;
    if( rMEvt.IsRight() && (nFlags & (F_DOWN_CTRL | F_DOWN_DESELECT) ))
    {
        nFlags &= ~(F_DOWN_CTRL | F_DOWN_DESELECT);
        bHandled = sal_True;
    }

    Point aDocPos( rMEvt.GetPosPixel() );
    ToDocPos( aDocPos );
    SvxIconChoiceCtrlEntry* pDocEntry = GetEntry( aDocPos );
    if( pDocEntry )
    {
        if( nFlags & F_DOWN_CTRL )
        {
            // Ctrl & MultiSelection
            ToggleSelection( pDocEntry );
            SetCursor( pDocEntry );
            bHandled = sal_True;
        }
        else if( nFlags & F_DOWN_DESELECT )
        {
            DeselectAllBut( pDocEntry );
            SetCursor( pDocEntry );
            SelectEntry( pDocEntry, sal_True, sal_True, sal_False, sal_True );
            bHandled = sal_True;
        }
    }

    nFlags &= ~(F_DOWN_CTRL | F_DOWN_DESELECT);
    if( nFlags & F_START_EDITTIMER_IN_MOUSEUP )
    {
        bHandled = sal_True;
        StartEditTimer();
        nFlags &= ~F_START_EDITTIMER_IN_MOUSEUP;
    }

    if((nWinBits & WB_HIGHLIGHTFRAME) && bHighlightFramePressed && pCurHighlightFrame)
    {
        bHandled = sal_True;
        SvxIconChoiceCtrlEntry* pEntry = pCurHighlightFrame;
        pCurHighlightFrame = 0; // force repaint of frame
        bHighlightFramePressed = sal_False;
        SetEntryHighlightFrame( pEntry, sal_True );

        pHdlEntry = pCurHighlightFrame;
        pView->ClickIcon();

        // set focus on Icon
        SvxIconChoiceCtrlEntry* pOldCursor = pCursor;
        SetCursor_Impl( pOldCursor, pHdlEntry, sal_False, sal_False, sal_True );

        pHdlEntry = 0;
    }
    return bHandled;
}

sal_Bool SvxIconChoiceCtrl_Impl::MouseMove( const MouseEvent& rMEvt )
{
    const Point aDocPos( pView->PixelToLogic(rMEvt.GetPosPixel()) );

    if( pView->IsTracking() )
        return sal_False;
    else if( nWinBits & WB_HIGHLIGHTFRAME )
    {
        SvxIconChoiceCtrlEntry* pEntry = GetEntry( aDocPos, sal_True );
        SetEntryHighlightFrame( pEntry );
    }
    else
        return sal_False;
    return sal_True;
}

void SvxIconChoiceCtrl_Impl::SetCursor_Impl( SvxIconChoiceCtrlEntry* pOldCursor,
    SvxIconChoiceCtrlEntry* pNewCursor, sal_Bool bMod1, sal_Bool bShift, sal_Bool bPaintSync )
{
    if( pNewCursor )
    {
        SvxIconChoiceCtrlEntry* pFilterEntry = 0;
        sal_Bool bDeselectAll = sal_False;
        if( eSelectionMode != SINGLE_SELECTION )
        {
            if( !bMod1 && !bShift )
                bDeselectAll = sal_True;
            else if( bShift && !bMod1 && !pAnchor )
            {
                bDeselectAll = sal_True;
                pFilterEntry = pOldCursor;
            }
        }
        if( bDeselectAll )
            DeselectAllBut( pFilterEntry, bPaintSync );
        ShowCursor( sal_False );
        MakeEntryVisible( pNewCursor );
        SetCursor( pNewCursor );
        if( bMod1 && !bShift )
        {
            if( pAnchor )
            {
                AddSelectedRect( pAnchor, pOldCursor );
                pAnchor = 0;
            }
        }
        else if( bShift )
        {
            if( !pAnchor )
                pAnchor = pOldCursor;
            if ( nWinBits & WB_ALIGN_LEFT )
                SelectRange( pAnchor, pNewCursor, (nFlags & F_ADD_MODE)!=0 );
            else
                SelectRect(pAnchor,pNewCursor,(nFlags & F_ADD_MODE)!=0,&aSelectedRectList);
        }
        else
        {
            SelectEntry( pCursor, sal_True, sal_True,  sal_False, bPaintSync );
            aCurSelectionRect = GetEntryBoundRect( pCursor );
        }
    }
}

sal_Bool SvxIconChoiceCtrl_Impl::KeyInput( const KeyEvent& rKEvt )
{
    StopEditTimer();

    sal_Bool bMod2 = rKEvt.GetKeyCode().IsMod2();
    sal_Unicode cChar = rKEvt.GetCharCode();
    sal_uLong nPos = (sal_uLong)-1;
    if ( bMod2 && cChar && IsMnemonicChar( cChar, nPos ) )
    {
        // shortcut is clicked
        SvxIconChoiceCtrlEntry* pNewCursor = GetEntry( nPos );
        SvxIconChoiceCtrlEntry* pOldCursor = pCursor;
        if ( pNewCursor != pOldCursor )
            SetCursor_Impl( pOldCursor, pNewCursor, sal_False, sal_False, sal_False );
        return sal_True;
    }

    if ( bMod2 )
        // no actions with <ALT>
        return sal_False;

    sal_Bool bKeyUsed = sal_True;
    sal_Bool bMod1 = rKEvt.GetKeyCode().IsMod1();
    sal_Bool bShift = rKEvt.GetKeyCode().IsShift();

    if( eSelectionMode == SINGLE_SELECTION || eSelectionMode == NO_SELECTION)
    {
        bShift = sal_False;
        bMod1 = sal_False;
    }

    if( bMod1 )
        nFlags |= F_ADD_MODE;

    SvxIconChoiceCtrlEntry* pNewCursor;
    SvxIconChoiceCtrlEntry* pOldCursor = pCursor;

    sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();
    switch( nCode )
    {
        case KEY_UP:
        case KEY_PAGEUP:
            if( pCursor )
            {
                MakeEntryVisible( pCursor );
                if( nCode == KEY_UP )
                    pNewCursor = pImpCursor->GoUpDown(pCursor,sal_False);
                else
                    pNewCursor = pImpCursor->GoPageUpDown(pCursor,sal_False);
                SetCursor_Impl( pOldCursor, pNewCursor, bMod1, bShift, sal_True );
                if( !pNewCursor )
                {
                    Rectangle aRect( GetEntryBoundRect( pCursor ) );
                    if( aRect.Top())
                    {
                        aRect.Bottom() -= aRect.Top();
                        aRect.Top() = 0;
                        MakeVisible( aRect );
                    }
                }

                if ( bChooseWithCursor && pNewCursor != NULL )
                {
                    pHdlEntry = pNewCursor;//GetCurEntry();
                    pCurHighlightFrame = pHdlEntry;
                    pView->ClickIcon();
                    pCurHighlightFrame = NULL;
                }
            }
            break;

        case KEY_DOWN:
        case KEY_PAGEDOWN:
            if( pCursor )
            {
                if( nCode == KEY_DOWN )
                    pNewCursor=pImpCursor->GoUpDown( pCursor,sal_True );
                else
                    pNewCursor=pImpCursor->GoPageUpDown( pCursor,sal_True );
                SetCursor_Impl( pOldCursor, pNewCursor, bMod1, bShift, sal_True );

                if ( bChooseWithCursor && pNewCursor != NULL)
                {
                    pHdlEntry = pNewCursor;//GetCurEntry();
                    pCurHighlightFrame = pHdlEntry;
                    pView->ClickIcon();
                    pCurHighlightFrame = NULL;
                }
            }
            break;

        case KEY_RIGHT:
            if( pCursor )
            {
                pNewCursor=pImpCursor->GoLeftRight(pCursor,sal_True );
                SetCursor_Impl( pOldCursor, pNewCursor, bMod1, bShift, sal_True );
            }
            break;

        case KEY_LEFT:
            if( pCursor )
            {
                MakeEntryVisible( pCursor );
                pNewCursor = pImpCursor->GoLeftRight(pCursor,sal_False );
                SetCursor_Impl( pOldCursor, pNewCursor, bMod1, bShift, sal_True );
                if( !pNewCursor )
                {
                    Rectangle aRect( GetEntryBoundRect(pCursor));
                    if( aRect.Left() )
                    {
                        aRect.Right() -= aRect.Left();
                        aRect.Left() = 0;
                        MakeVisible( aRect );
                    }
                }
            }
            break;

        case KEY_F2:
            if( !bMod1 && !bShift )
                EditTimeoutHdl( 0 );
            else
                bKeyUsed = sal_False;
            break;

        case KEY_F8:
            if( rKEvt.GetKeyCode().IsShift() )
            {
                if( nFlags & F_ADD_MODE )
                    nFlags &= (~F_ADD_MODE);
                else
                    nFlags |= F_ADD_MODE;
            }
            else
                bKeyUsed = sal_False;
            break;

        case KEY_SPACE:
            if( pCursor && eSelectionMode != SINGLE_SELECTION )
            {
                if( !bMod1 )
                {
                    //SelectAll( sal_False );
                    SetNoSelection();
                    ClearSelectedRectList();

                    // click Icon with spacebar
                    SetEntryHighlightFrame( GetCurEntry(), sal_True );
                    pView->ClickIcon();
                    pHdlEntry = pCurHighlightFrame;
                    pCurHighlightFrame=0;
                }
                else
                    ToggleSelection( pCursor );
            }
            break;

#ifdef DBG_UTIL
        case KEY_F10:
            if( rKEvt.GetKeyCode().IsShift() )
            {
                if( pCursor )
                    pView->SetEntryTextMode( IcnShowTextFull, pCursor );
            }
            if( rKEvt.GetKeyCode().IsMod1() )
            {
                if( pCursor )
                    pView->SetEntryTextMode( IcnShowTextShort, pCursor );
            }
            break;
#endif

        case KEY_ADD:
        case KEY_DIVIDE :
        case KEY_A:
            if( bMod1 && (eSelectionMode != SINGLE_SELECTION))
                SelectAll( sal_True );
            else
                bKeyUsed = sal_False;
            break;

        case KEY_SUBTRACT:
        case KEY_COMMA :
            if( bMod1 )
                SetNoSelection();
            else
                bKeyUsed = sal_False;
            break;

        case KEY_RETURN:
            if( bMod1 )
            {
                if( pCursor && bEntryEditingEnabled )
                    /*pView->*/EditEntry( pCursor );
            }
            else
                bKeyUsed = sal_False;
            break;

        case KEY_END:
            if( pCursor )
            {
                pNewCursor = aEntries[ aEntries.size() - 1 ];
                SetCursor_Impl( pOldCursor, pNewCursor, bMod1, bShift, sal_True );
            }
            break;

        case KEY_HOME:
            if( pCursor )
            {
                pNewCursor = aEntries[ 0 ];
                SetCursor_Impl( pOldCursor, pNewCursor, bMod1, bShift, sal_True );
            }
            break;

        default:
            bKeyUsed = sal_False;

    }
    return bKeyUsed;
}

// recalculate TopLeft of scrollbars (but not their sizes!)
void SvxIconChoiceCtrl_Impl::PositionScrollBars( long nRealWidth, long nRealHeight )
{
    // horizontal scrollbar
    Point aPos( 0, nRealHeight );
    aPos.Y() -= nHorSBarHeight;

    if( aHorSBar.GetPosPixel() != aPos )
        aHorSBar.SetPosPixel( aPos );

    // vertical scrollbar
    aPos.X() = nRealWidth; aPos.Y() = 0;
    aPos.X() -= nVerSBarWidth;
    aPos.X()++;
    aPos.Y()--;

    if( aVerSBar.GetPosPixel() != aPos )
        aVerSBar.SetPosPixel( aPos );
}

void SvxIconChoiceCtrl_Impl::AdjustScrollBars( sal_Bool )
{
    Rectangle aOldOutRect( GetOutputRect() );
    long nVirtHeight = aVirtOutputSize.Height();
    long nVirtWidth = aVirtOutputSize.Width();

    Size aOSize( pView->Control::GetOutputSizePixel() );
    long nRealHeight = aOSize.Height();
    long nRealWidth = aOSize.Width();

    PositionScrollBars( nRealWidth, nRealHeight );

    const MapMode& rMapMode = pView->GetMapMode();
    Point aOrigin( rMapMode.GetOrigin() );

    long nVisibleWidth;
    if( nRealWidth > nVirtWidth )
        nVisibleWidth = nVirtWidth + aOrigin.X();
    else
        nVisibleWidth = nRealWidth;

    long nVisibleHeight;
    if( nRealHeight > nVirtHeight )
        nVisibleHeight = nVirtHeight + aOrigin.Y();
    else
        nVisibleHeight = nRealHeight;

    sal_Bool bVerSBar = ( nWinBits & WB_VSCROLL ) != 0;
    sal_Bool bHorSBar = ( nWinBits & WB_HSCROLL ) != 0;
    sal_Bool bNoVerSBar = ( nWinBits & WB_NOVSCROLL ) != 0;
    sal_Bool bNoHorSBar = ( nWinBits & WB_NOHSCROLL ) != 0;

    sal_uInt16 nResult = 0;
    if( nVirtHeight )
    {
        // activate vertical scrollbar?
        if( !bNoVerSBar && (bVerSBar || ( nVirtHeight > nVisibleHeight)) )
        {
            nResult = 0x0001;
            nRealWidth -= nVerSBarWidth;

            if( nRealWidth > nVirtWidth )
                nVisibleWidth = nVirtWidth + aOrigin.X();
            else
                nVisibleWidth = nRealWidth;

            nFlags |= F_HOR_SBARSIZE_WITH_VBAR;
        }
        // activate horizontal scrollbar?
        if( !bNoHorSBar && (bHorSBar || (nVirtWidth > nVisibleWidth)) )
        {
            nResult |= 0x0002;
            nRealHeight -= nHorSBarHeight;

            if( nRealHeight > nVirtHeight )
                nVisibleHeight = nVirtHeight + aOrigin.Y();
            else
                nVisibleHeight = nRealHeight;

            // do we need a vertical scrollbar after all?
            if( !(nResult & 0x0001) &&  // only if not already there
                ( !bNoVerSBar && ((nVirtHeight > nVisibleHeight) || bVerSBar)) )
            {
                nResult = 3; // both turned on
                nRealWidth -= nVerSBarWidth;

                if( nRealWidth > nVirtWidth )
                    nVisibleWidth = nVirtWidth + aOrigin.X();
                else
                    nVisibleWidth = nRealWidth;

                nFlags |= F_VER_SBARSIZE_WITH_HBAR;
            }
        }
    }

    // size vertical scrollbar
    long nThumb = aVerSBar.GetThumbPos();
    Size aSize( nVerSBarWidth, nRealHeight );
    aSize.Height() += 2;
    if( aSize != aVerSBar.GetSizePixel() )
        aVerSBar.SetSizePixel( aSize );
    aVerSBar.SetVisibleSize( nVisibleHeight );
    aVerSBar.SetPageSize( GetScrollBarPageSize( nVisibleHeight ));

    if( nResult & 0x0001 )
    {
        aVerSBar.SetThumbPos( nThumb );
        aVerSBar.Show();
    }
    else
    {
        aVerSBar.SetThumbPos( 0 );
        aVerSBar.Hide();
    }

    // size horizontal scrollbar
    nThumb = aHorSBar.GetThumbPos();
    aSize.Width() = nRealWidth;
    aSize.Height() = nHorSBarHeight;
    aSize.Width()++;
    if( nResult & 0x0001 ) // vertical scrollbar?
    {
        aSize.Width()++;
        nRealWidth++;
    }
    if( aSize != aHorSBar.GetSizePixel() )
        aHorSBar.SetSizePixel( aSize );
    aHorSBar.SetVisibleSize( nVisibleWidth );
    aHorSBar.SetPageSize( GetScrollBarPageSize(nVisibleWidth ));
    if( nResult & 0x0002 )
    {
        aHorSBar.SetThumbPos( nThumb );
        aHorSBar.Show();
    }
    else
    {
        aHorSBar.SetThumbPos( 0 );
        aHorSBar.Hide();
    }

    aOutputSize.Width() = nRealWidth;
    if( nResult & 0x0002 ) // horizontal scrollbar ?
        nRealHeight++; // because lower border is clipped
    aOutputSize.Height() = nRealHeight;

    Rectangle aNewOutRect( GetOutputRect() );
    if( aNewOutRect != aOldOutRect && pView->HasBackground() )
    {
        Wallpaper aPaper( pView->GetBackground() );
        aPaper.SetRect( aNewOutRect );
        pView->SetBackground( aPaper );
    }

    if( (nResult & (0x0001|0x0002)) == (0x0001|0x0002) )
        aScrBarBox.Show();
    else
        aScrBarBox.Hide();
}

void SvxIconChoiceCtrl_Impl::Resize()
{
    StopEditTimer();
    InitScrollBarBox();
    aOutputSize = pView->GetOutputSizePixel();
    pImpCursor->Clear();
    pGridMap->OutputSizeChanged();

    const Size& rSize = pView->Control::GetOutputSizePixel();
    PositionScrollBars( rSize.Width(), rSize.Height() );
    // The scrollbars are shown/hidden asynchronously, so derived classes can
    // do an Arrange during Resize, without the scrollbars suddenly turning
    // on and off again.
    // If an event is already underway, we don't need to send a new one, at least
    // as long as there is only one event type.
    if ( ! nUserEventAdjustScrBars )
        nUserEventAdjustScrBars =
            Application::PostUserEvent( LINK( this, SvxIconChoiceCtrl_Impl, UserEventHdl),
                EVENTID_ADJUST_SCROLLBARS);

    if( pView->HasBackground() && !pView->GetBackground().IsScrollable() )
    {
        Rectangle aRect( GetOutputRect());
        Wallpaper aPaper( pView->GetBackground() );
        aPaper.SetRect( aRect );
        pView->SetBackground( aPaper );
    }
    VisRectChanged();
}

sal_Bool SvxIconChoiceCtrl_Impl::CheckHorScrollBar()
{
    if( !pZOrderList || !aHorSBar.IsVisible() )
        return sal_False;
    const MapMode& rMapMode = pView->GetMapMode();
    Point aOrigin( rMapMode.GetOrigin() );
    if(!( nWinBits & WB_HSCROLL) && !aOrigin.X() )
    {
        long nWidth = aOutputSize.Width();
        const size_t nCount = pZOrderList->size();
        long nMostRight = 0;
        for( size_t nCur = 0; nCur < nCount; nCur++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = (*pZOrderList)[ nCur ];
            long nRight = GetEntryBoundRect(pEntry).Right();
            if( nRight > nWidth )
                return sal_False;
            if( nRight > nMostRight )
                nMostRight = nRight;
        }
        aHorSBar.Hide();
        aOutputSize.Height() += nHorSBarHeight;
        aVirtOutputSize.Width() = nMostRight;
        aHorSBar.SetThumbPos( 0 );
        Range aRange;
        aRange.Max() = nMostRight - 1;
        aHorSBar.SetRange( aRange  );
        if( aVerSBar.IsVisible() )
        {
            Size aSize( aVerSBar.GetSizePixel());
            aSize.Height() += nHorSBarHeight;
            aVerSBar.SetSizePixel( aSize );
        }
        return sal_True;
    }
    return sal_False;
}

sal_Bool SvxIconChoiceCtrl_Impl::CheckVerScrollBar()
{
    if( !pZOrderList || !aVerSBar.IsVisible() )
        return sal_False;
    const MapMode& rMapMode = pView->GetMapMode();
    Point aOrigin( rMapMode.GetOrigin() );
    if(!( nWinBits & WB_VSCROLL) && !aOrigin.Y() )
    {
        long nDeepest = 0;
        long nHeight = aOutputSize.Height();
        const size_t nCount = pZOrderList->size();
        for( size_t nCur = 0; nCur < nCount; nCur++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = (*pZOrderList)[ nCur ];
            long nBottom = GetEntryBoundRect(pEntry).Bottom();
            if( nBottom > nHeight )
                return sal_False;
            if( nBottom > nDeepest )
                nDeepest = nBottom;
        }
        aVerSBar.Hide();
        aOutputSize.Width() += nVerSBarWidth;
        aVirtOutputSize.Height() = nDeepest;
        aVerSBar.SetThumbPos( 0 );
        Range aRange;
        aRange.Max() = nDeepest - 1;
        aVerSBar.SetRange( aRange  );
        if( aHorSBar.IsVisible() )
        {
            Size aSize( aHorSBar.GetSizePixel());
            aSize.Width() += nVerSBarWidth;
            aHorSBar.SetSizePixel( aSize );
        }
        return sal_True;
    }
    return sal_False;
}


// hides scrollbars if they're unnecessary
void SvxIconChoiceCtrl_Impl::CheckScrollBars()
{
    CheckVerScrollBar();
    if( CheckHorScrollBar() )
        CheckVerScrollBar();
    if( aVerSBar.IsVisible() && aHorSBar.IsVisible() )
        aScrBarBox.Show();
    else
        aScrBarBox.Hide();
}


void SvxIconChoiceCtrl_Impl::GetFocus()
{
    RepaintEntries( ICNVIEW_FLAG_SELECTED );
    if( pCursor )
    {
        pCursor->SetFlags( ICNVIEW_FLAG_FOCUSED );
        ShowCursor( sal_True );
    }
}

void SvxIconChoiceCtrl_Impl::LoseFocus()
{
    StopEditTimer();
    if( pCursor )
        pCursor->ClearFlags( ICNVIEW_FLAG_FOCUSED );
    ShowCursor( sal_False );

//  HideFocus ();
//  pView->Invalidate ( aFocus.aRect );

    RepaintEntries( ICNVIEW_FLAG_SELECTED );
}

void SvxIconChoiceCtrl_Impl::SetUpdateMode( sal_Bool bUpdate )
{
    if( bUpdate != bUpdateMode )
    {
        bUpdateMode = bUpdate;
        if( bUpdate )
        {
            AdjustScrollBars();
            pImpCursor->Clear();
            pGridMap->Clear();
            pView->Invalidate(INVALIDATE_NOCHILDREN);
        }
    }
}

void SvxIconChoiceCtrl_Impl::PaintEntry( SvxIconChoiceCtrlEntry* pEntry, sal_Bool bIsBackgroundPainted )
{
    Point aPos( GetEntryPos( pEntry ) );
    PaintEntry( pEntry, aPos, 0, bIsBackgroundPainted );
}

// priorities of the emphasis:  bDropTarget => bCursored => bSelected
void SvxIconChoiceCtrl_Impl::PaintEmphasis(
    const Rectangle& rTextRect, const Rectangle& rImageRect,
    sal_Bool bSelected, sal_Bool bDropTarget, sal_Bool bCursored, OutputDevice* pOut,
    sal_Bool bIsBackgroundPainted )
{
    static Color aTransparent( COL_TRANSPARENT );

    if( !pOut )
        pOut = pView;

#ifdef OV_CHECK_EMPH_RECTS
    {
        Color aXOld( pOut->GetFillColor() );
        pOut->SetFillColor( Color( COL_GREEN ));
        pOut->DrawRect( rTextRect );
        pOut->DrawRect( rImageRect );
        pOut->SetFillColor( aXOld );
    }
#endif

    const StyleSettings& rSettings = pOut->GetSettings().GetStyleSettings();
    Color aOldFillColor( pOut->GetFillColor() );

    sal_Bool bSolidTextRect = sal_False;
    sal_Bool bSolidImageRect = sal_False;

    if( bDropTarget && ( eSelectionMode != NO_SELECTION ) )
    {
        pOut->SetFillColor( rSettings.GetHighlightColor() );
        bSolidTextRect = sal_True;
        bSolidImageRect = sal_True;
    }
    else
    {
        if ( !bSelected || bCursored )
        {
            if( !pView->HasFontFillColor() )
                pOut->SetFillColor( pOut->GetBackground().GetColor() );
            else
            {
                const Color& rFillColor = pView->GetFont().GetFillColor();
                pOut->SetFillColor( rFillColor );
                if( rFillColor != aTransparent )
                    bSolidTextRect = sal_True;
            }
        }
    }

    // draw text rectangle
    if( !bSolidTextRect )
    {
        if( !bIsBackgroundPainted )
            pOut->Erase( rTextRect );
    }
    else
    {
        Color aOldLineColor;
        if( bCursored )
        {
            aOldLineColor = pOut->GetLineColor();
            pOut->SetLineColor( Color( COL_GRAY ) );
        }
        pOut->DrawRect( rTextRect );
        if( bCursored )
            pOut->SetLineColor( aOldLineColor );
    }

    // draw image rectangle
    if( !bSolidImageRect )
    {
        if( !bIsBackgroundPainted )
            pOut->Erase( rImageRect );
    }
// the emphasis of the images has to be drawn by the derived class (in the
// virtual function DrawEntryImage)
//  else
//      pOut->DrawRect( rImageRect );

    pOut->SetFillColor( aOldFillColor );
}


void SvxIconChoiceCtrl_Impl::PaintItem( const Rectangle& rRect,
    IcnViewFieldType eItem, SvxIconChoiceCtrlEntry* pEntry, sal_uInt16 nPaintFlags,
    OutputDevice* pOut, const String* pStr, ::vcl::ControlLayoutData* _pLayoutData )
{
    if( eItem == IcnViewFieldTypeText )
    {
        String aText;
        if( !pStr )
            aText = pView->GetEntryText( pEntry, sal_False );
        else
            aText = *pStr;

        if ( _pLayoutData )
        {
            pOut->DrawText( rRect, aText, nCurTextDrawFlags,
                &_pLayoutData->m_aUnicodeBoundRects, &_pLayoutData->m_aDisplayText );
        }
        else
        {
            Color aOldFontColor = pOut->GetTextColor();
            if ( pView->AutoFontColor() )
            {
                Color aBkgColor( pOut->GetBackground().GetColor() );
                Color aFontColor;
                sal_uInt16 nColor = ( aBkgColor.GetRed() + aBkgColor.GetGreen() + aBkgColor.GetBlue() ) / 3;
                if ( nColor > 127 )
                    aFontColor.SetColor ( COL_BLACK );
                else
                    aFontColor.SetColor( COL_WHITE );
                pOut->SetTextColor( aFontColor );
            }

            pOut->DrawText( rRect, aText, nCurTextDrawFlags );

            if ( pView->AutoFontColor() )
                pOut->SetTextColor( aOldFontColor );

            if( pEntry->IsFocused() )
            {
                Rectangle aRect ( CalcFocusRect( (SvxIconChoiceCtrlEntry*)pEntry ) );
                /*pView->*/ShowFocus( aRect );
                DrawFocusRect( pOut );
            }
        }
    }
    else
    {
        Point aPos( rRect.TopLeft() );
        if( nPaintFlags & PAINTFLAG_HOR_CENTERED )
            aPos.X() += (rRect.GetWidth() - aImageSize.Width() ) / 2;
        if( nPaintFlags & PAINTFLAG_VER_CENTERED )
            aPos.Y() += (rRect.GetHeight() - aImageSize.Height() ) / 2;
        pView->DrawEntryImage( pEntry, aPos, *pOut );
    }
}

void SvxIconChoiceCtrl_Impl::PaintEntryVirtOutDev( SvxIconChoiceCtrlEntry* pEntry )
{
#ifdef OV_NO_VIRT_OUTDEV
    PaintEntry( pEntry );
#else
    if( !pEntryPaintDev )
    {
        pEntryPaintDev = new VirtualDevice( *pView );
        pEntryPaintDev->SetFont( pView->GetFont() );
        pEntryPaintDev->SetLineColor();
        //pEntryPaintDev->SetBackground( pView->GetBackground() );
    }
    const Rectangle& rRect = GetEntryBoundRect( pEntry );
    Rectangle aOutRect( GetOutputRect() );
    if( !rRect.IsOver( aOutRect ) )
        return;
    Wallpaper aPaper( pView->GetBackground() );
    Rectangle aRect( aPaper.GetRect() );

    // move rectangle, so the bounding rectangle of the entry lies in
    // VirtOut-Dev at 0,0
    aRect.Move( -rRect.Left(), -rRect.Top() );
    aPaper.SetRect( aRect );
    pEntryPaintDev->SetBackground( aPaper );
    pEntryPaintDev->SetFont( pView->GetFont() );

    Size aSize( rRect.GetSize() );
    pEntryPaintDev->SetOutputSizePixel( aSize );
    pEntryPaintDev->DrawOutDev(
        Point(), aSize, rRect.TopLeft(), aSize, *pView );

    PaintEntry( pEntry, Point(), pEntryPaintDev );

    pView->DrawOutDev(
        rRect.TopLeft(),
        aSize,
        Point(),
        aSize,
        *pEntryPaintDev );
#endif
}


void SvxIconChoiceCtrl_Impl::PaintEntry( SvxIconChoiceCtrlEntry* pEntry, const Point& rPos,
    OutputDevice* pOut, sal_Bool bIsBackgroundPainted )
{
    if( !pOut )
        pOut = pView;

    sal_Bool bSelected = sal_False;

    if( eSelectionMode != NO_SELECTION )
        bSelected = pEntry->IsSelected();

    sal_Bool bCursored = pEntry->IsCursored();
    sal_Bool bDropTarget = pEntry->IsDropTarget();
    sal_Bool bNoEmphasis = pEntry->IsBlockingEmphasis();

    Font aTempFont( pOut->GetFont() );

    // AutoFontColor
    /*
    if ( pView->AutoFontColor() )
    {
        aTempFont.SetColor ( aFontColor );
    }
    */

    String aEntryText( pView->GetEntryText( pEntry, sal_False ) );
    Rectangle aTextRect( CalcTextRect(pEntry,&rPos,sal_False,&aEntryText));
    Rectangle aBmpRect( CalcBmpRect(pEntry, &rPos ) );

    sal_Bool    bShowSelection =
        (   (   ( bSelected && !bCursored )
            ||  bDropTarget
            )
        &&  !bNoEmphasis
        &&  ( eSelectionMode != NO_SELECTION )
        );
    sal_Bool bActiveSelection = ( 0 != ( nWinBits & WB_NOHIDESELECTION ) ) || pView->HasFocus();

    if ( bShowSelection )
    {
        const StyleSettings& rSettings = pOut->GetSettings().GetStyleSettings();
        Font aNewFont( aTempFont );

        // font fill colors that are attributed "hard" need corresponding "hard"
        // attributed highlight colors
        if( pView->HasFontFillColor() )
        {
            if( (nWinBits & WB_NOHIDESELECTION) || pView->HasFocus() )
                aNewFont.SetFillColor( rSettings.GetHighlightColor() );
            else
                aNewFont.SetFillColor( rSettings.GetDeactiveColor() );
        }

        Color aWinCol = rSettings.GetWindowTextColor();
        if ( !bActiveSelection && rSettings.GetFaceColor().IsBright() == aWinCol.IsBright() )
            aNewFont.SetColor( rSettings.GetWindowTextColor() );
        else
            aNewFont.SetColor( rSettings.GetHighlightTextColor() );

        pOut->SetFont( aNewFont );

        pOut->SetFillColor( pOut->GetBackground().GetColor() );
        pOut->DrawRect( CalcFocusRect( pEntry ) );
        pOut->SetFillColor( );
    }

    sal_Bool bResetClipRegion = sal_False;
    if( !pView->IsClipRegion() && (aVerSBar.IsVisible() || aHorSBar.IsVisible()) )
    {
        Rectangle aOutputArea( GetOutputRect() );
        if( aOutputArea.IsOver(aTextRect) || aOutputArea.IsOver(aBmpRect) )
        {
            pView->SetClipRegion(Region(aOutputArea));
            bResetClipRegion = sal_True;
        }
    }

#ifdef OV_DRAWBOUNDRECT
    {
        Color aXOldColor = pOut->GetLineColor();
        pOut->SetLineColor( Color( COL_LIGHTRED ) );
        Rectangle aXRect( pEntry->aRect );
        aXRect.SetPos( rPos );
        pOut->DrawRect( aXRect );
        pOut->SetLineColor( aXOldColor );
    }
#endif

    sal_Bool bLargeIconMode = WB_ICON == ( nWinBits & (VIEWMODE_MASK) );
    sal_uInt16 nBmpPaintFlags = PAINTFLAG_VER_CENTERED;
    if ( bLargeIconMode )
        nBmpPaintFlags |= PAINTFLAG_HOR_CENTERED;
    sal_uInt16 nTextPaintFlags = bLargeIconMode ? PAINTFLAG_HOR_CENTERED : PAINTFLAG_VER_CENTERED;

    if( !bNoEmphasis )
        PaintEmphasis(aTextRect,aBmpRect,bSelected,bDropTarget,bCursored,pOut,bIsBackgroundPainted);

    if ( bShowSelection )
        pView->DrawSelectionBackground( CalcFocusRect( pEntry ),
        bActiveSelection ? 1 : 2 /* highlight */, sal_False /* check */, sal_True /* border */, sal_False /* ext border only */ );

    PaintItem( aBmpRect, IcnViewFieldTypeImage, pEntry, nBmpPaintFlags, pOut );

    PaintItem( aTextRect, IcnViewFieldTypeText, pEntry,
        nTextPaintFlags, pOut );

    // draw highlight frame
    if( pEntry == pCurHighlightFrame && !bNoEmphasis )
        DrawHighlightFrame( pOut, CalcFocusRect( pEntry ), sal_False );

    pOut->SetFont( aTempFont );
    if( bResetClipRegion )
        pView->SetClipRegion();
}

void SvxIconChoiceCtrl_Impl::SetEntryPos( SvxIconChoiceCtrlEntry* pEntry, const Point& rPos,
    sal_Bool bAdjustAtGrid, sal_Bool bCheckScrollBars, sal_Bool bKeepGridMap )
{
    ShowCursor( sal_False );
    Rectangle aBoundRect( GetEntryBoundRect( pEntry ));
    pView->Invalidate( aBoundRect );
    ToTop( pEntry );
    if( !IsAutoArrange() )
    {
        sal_Bool bAdjustVirtSize = sal_False;
        if( rPos != aBoundRect.TopLeft() )
        {
            Point aGridOffs(
                pEntry->aGridRect.TopLeft() - pEntry->aRect.TopLeft() );
            pImpCursor->Clear();
            if( !bKeepGridMap )
                pGridMap->Clear();
            aBoundRect.SetPos( rPos );
            pEntry->aRect = aBoundRect;
            pEntry->aGridRect.SetPos( rPos + aGridOffs );
            bAdjustVirtSize = sal_True;
        }
        if( bAdjustAtGrid )
        {
            if( bAdjustVirtSize )
            {
                // By aligning the (in some cases newly positioned) entry, it
                // can become completely visible again, so that maybe we don't
                // need a scrollbar after all. To avoid suddenly turning the
                // scrollbar(s) on and then off again, we use the aligned
                // bounding rectangle of the entry to enlarge the virtual
                // output size. The virtual size has to be adapted, because
                // AdjustEntryAtGrid depends on it.
                const Rectangle& rBoundRect = GetEntryBoundRect( pEntry );
                Rectangle aCenterRect( CalcBmpRect( pEntry, 0 ));
                Point aNewPos( AdjustAtGrid( aCenterRect, rBoundRect ) );
                Rectangle aNewBoundRect( aNewPos, pEntry->aRect.GetSize());
                AdjustVirtSize( aNewBoundRect );
                bAdjustVirtSize = sal_False;
            }
            AdjustEntryAtGrid( pEntry );
            ToTop( pEntry );
        }
        if( bAdjustVirtSize )
            AdjustVirtSize( pEntry->aRect );

        if( bCheckScrollBars && bUpdateMode )
            CheckScrollBars();

        pView->Invalidate( pEntry->aRect );
        pGridMap->OccupyGrids( pEntry );
    }
    else
    {
        SvxIconChoiceCtrlEntry* pPrev = FindEntryPredecessor( pEntry, rPos );
        SetEntryPredecessor( pEntry, pPrev );
        aAutoArrangeTimer.Start();
    }
    ShowCursor( sal_True );
}

void SvxIconChoiceCtrl_Impl::SetNoSelection()
{
    // block recursive calls via SelectEntry
    if( !(nFlags & F_CLEARING_SELECTION ))
    {
        nFlags |= F_CLEARING_SELECTION;
        DeselectAllBut( 0, sal_True );
        nFlags &= ~F_CLEARING_SELECTION;
    }
}

SvxIconChoiceCtrlEntry* SvxIconChoiceCtrl_Impl::GetEntry( const Point& rDocPos, sal_Bool bHit )
{
    CheckBoundingRects();
    // search through z-order list from the end
    size_t nCount = pZOrderList->size();
    while( nCount )
    {
        nCount--;
        SvxIconChoiceCtrlEntry* pEntry = (*pZOrderList)[ nCount ];
        if( pEntry->aRect.IsInside( rDocPos ) )
        {
            if( bHit )
            {
                Rectangle aRect = CalcBmpRect( pEntry );
                aRect.Top() -= 3;
                aRect.Bottom() += 3;
                aRect.Left() -= 3;
                aRect.Right() += 3;
                if( aRect.IsInside( rDocPos ) )
                    return pEntry;
                aRect = CalcTextRect( pEntry );
                if( aRect.IsInside( rDocPos ) )
                    return pEntry;
            }
            else
                return pEntry;
        }
    }
    return 0;
}

Point SvxIconChoiceCtrl_Impl::GetEntryPos( SvxIconChoiceCtrlEntry* pEntry )
{
    return pEntry->aRect.TopLeft();
}

void SvxIconChoiceCtrl_Impl::MakeEntryVisible( SvxIconChoiceCtrlEntry* pEntry, sal_Bool bBound )
{
    if ( bBound )
    {
        const Rectangle& rRect = GetEntryBoundRect( pEntry );
        MakeVisible( rRect );
    }
    else
    {
        Rectangle aRect = CalcBmpRect( pEntry );
        aRect.Union( CalcTextRect( pEntry ) );
        aRect.Top() += TBOFFS_BOUND;
        aRect.Bottom() += TBOFFS_BOUND;
        aRect.Left() += LROFFS_BOUND;
        aRect.Right() += LROFFS_BOUND;
        MakeVisible( aRect );
    }
}

const Rectangle& SvxIconChoiceCtrl_Impl::GetEntryBoundRect( SvxIconChoiceCtrlEntry* pEntry )
{
    if( !IsBoundingRectValid( pEntry->aRect ))
        FindBoundingRect( pEntry );
    return pEntry->aRect;
}

Rectangle SvxIconChoiceCtrl_Impl::CalcBmpRect( SvxIconChoiceCtrlEntry* pEntry, const Point* pPos )
{
    Rectangle aBound = GetEntryBoundRect( pEntry );
    if( pPos )
        aBound.SetPos( *pPos );
    Point aPos( aBound.TopLeft() );

    switch( nWinBits & (VIEWMODE_MASK) )
    {
        case WB_ICON:
        {
            aPos.X() += ( aBound.GetWidth() - aImageSize.Width() ) / 2;
            return Rectangle( aPos, aImageSize );
        }

        case WB_SMALLICON:
        case WB_DETAILS:
            aPos.Y() += ( aBound.GetHeight() - aImageSize.Height() ) / 2;
            //TODO: determine horizontal distance to bounding rectangle
            return Rectangle( aPos, aImageSize );

        default:
            OSL_FAIL("IconView: Viewmode not set");
            return aBound;
    }
}

Rectangle SvxIconChoiceCtrl_Impl::CalcTextRect( SvxIconChoiceCtrlEntry* pEntry,
    const Point* pEntryPos, sal_Bool bEdit, const String* pStr )
{
    String aEntryText;
    if( !pStr )
        aEntryText = pView->GetEntryText( pEntry, bEdit );
    else
        aEntryText = *pStr;

    const Rectangle aMaxTextRect( CalcMaxTextRect( pEntry ) );
    Rectangle aBound( GetEntryBoundRect( pEntry ) );
    if( pEntryPos )
        aBound.SetPos( *pEntryPos );

    Rectangle aTextRect( aMaxTextRect );
    if( !bEdit )
        aTextRect = pView->GetTextRect( aTextRect, aEntryText, nCurTextDrawFlags );

    Size aTextSize( aTextRect.GetSize() );

    Point aPos( aBound.TopLeft() );
    long nBoundWidth = aBound.GetWidth();
    long nBoundHeight = aBound.GetHeight();

    switch( nWinBits & (VIEWMODE_MASK) )
    {
        case WB_ICON:
            aPos.Y() += aImageSize.Height();
            aPos.Y() += VER_DIST_BMP_STRING;
            // at little more space when editing
            if( bEdit )
            {
                // +20%
                long nMinWidth = (( (aImageSize.Width()*10) / 100 ) * 2 ) +
                                 aImageSize.Width();
                if( nMinWidth > nBoundWidth )
                    nMinWidth = nBoundWidth;

                if( aTextSize.Width() < nMinWidth )
                    aTextSize.Width() = nMinWidth;

                // when editing, overlap with the area below is allowed
                Size aOptSize = aMaxTextRect.GetSize();
                if( aOptSize.Height() > aTextSize.Height() )
                    aTextSize.Height() = aOptSize.Height();
            }
            aPos.X() += (nBoundWidth - aTextSize.Width()) / 2;
            break;

        case WB_SMALLICON:
        case WB_DETAILS:
            aPos.X() += aImageSize.Width();
            aPos.X() += HOR_DIST_BMP_STRING;
            aPos.Y() += (nBoundHeight - aTextSize.Height()) / 2;
            break;
    }
    return Rectangle( aPos, aTextSize );
}


long SvxIconChoiceCtrl_Impl::CalcBoundingWidth( SvxIconChoiceCtrlEntry* pEntry ) const
{
    long nStringWidth = GetItemSize( pEntry, IcnViewFieldTypeText ).Width();
//  nStringWidth += 2*LROFFS_TEXT;
    long nWidth = 0;

    switch( nWinBits & (VIEWMODE_MASK) )
    {
        case WB_ICON:
            nWidth = std::max( nStringWidth, aImageSize.Width() );
            break;

        case WB_SMALLICON:
        case WB_DETAILS:
            nWidth = aImageSize.Width();
            nWidth += HOR_DIST_BMP_STRING;
            nWidth += nStringWidth;
            break;
    }
    return nWidth;
}

long SvxIconChoiceCtrl_Impl::CalcBoundingHeight( SvxIconChoiceCtrlEntry* pEntry ) const
{
    long nStringHeight = GetItemSize( pEntry, IcnViewFieldTypeText).Height();
    long nHeight = 0;

    switch( nWinBits & (VIEWMODE_MASK) )
    {
        case WB_ICON:
            nHeight = aImageSize.Height();
            nHeight += VER_DIST_BMP_STRING;
            nHeight += nStringHeight;
            break;

        case WB_SMALLICON:
        case WB_DETAILS:
            nHeight = std::max( aImageSize.Height(), nStringHeight );
            break;
    }
    if( nHeight > nMaxBoundHeight )
    {
        ((SvxIconChoiceCtrl_Impl*)this)->nMaxBoundHeight = nHeight;
        ((SvxIconChoiceCtrl_Impl*)this)->aHorSBar.SetLineSize( GetScrollBarLineSize() );
        ((SvxIconChoiceCtrl_Impl*)this)->aVerSBar.SetLineSize( GetScrollBarLineSize() );
    }
    return nHeight;
}

Size SvxIconChoiceCtrl_Impl::CalcBoundingSize( SvxIconChoiceCtrlEntry* pEntry ) const
{
    return Size( CalcBoundingWidth( pEntry ),
                 CalcBoundingHeight( pEntry ) );
}

void SvxIconChoiceCtrl_Impl::RecalcAllBoundingRectsSmart()
{
    nMaxBoundHeight = 0;
    pZOrderList->clear();
    size_t nCur;
    SvxIconChoiceCtrlEntry* pEntry;
    const size_t nCount = aEntries.size();

    if( !IsAutoArrange() || !pHead )
    {
        for( nCur = 0; nCur < nCount; nCur++ )
        {
            pEntry = aEntries[ nCur ];
            if( IsBoundingRectValid( pEntry->aRect ))
            {
                Size aBoundSize( pEntry->aRect.GetSize() );
                if( aBoundSize.Height() > nMaxBoundHeight )
                    nMaxBoundHeight = aBoundSize.Height();
            }
            else
                FindBoundingRect( pEntry );
            pZOrderList->push_back( pEntry );
        }
    }
    else
    {
        nCur = 0;
        pEntry = pHead;
        while( nCur != nCount )
        {
            DBG_ASSERT(pEntry->pflink&&pEntry->pblink,"SvxIconChoiceCtrl_Impl::RecalcAllBoundingRect > Bad link(s)");
            if( IsBoundingRectValid( pEntry->aRect ))
            {
                Size aBoundSize( pEntry->aRect.GetSize() );
                if( aBoundSize.Height() > nMaxBoundHeight )
                    nMaxBoundHeight = aBoundSize.Height();
            }
            else
                FindBoundingRect( pEntry );
            pZOrderList->push_back( pEntry );
            pEntry = pEntry->pflink;
            nCur++;
        }
    }
    AdjustScrollBars();
}

void SvxIconChoiceCtrl_Impl::FindBoundingRect( SvxIconChoiceCtrlEntry* pEntry )
{
    DBG_ASSERT(!pEntry->IsPosLocked(),"Locked entry pos in FindBoundingRect");
    if( pEntry->IsPosLocked() && IsBoundingRectValid( pEntry->aRect) )
    {
        AdjustVirtSize( pEntry->aRect );
        return;
    }
    Size aSize( CalcBoundingSize( pEntry ) );
    Point aPos(pGridMap->GetGridRect(pGridMap->GetUnoccupiedGrid(sal_True)).TopLeft());
    SetBoundingRect_Impl( pEntry, aPos, aSize );
}

void SvxIconChoiceCtrl_Impl::SetBoundingRect_Impl( SvxIconChoiceCtrlEntry* pEntry, const Point& rPos,
    const Size& /*rBoundingSize*/ )
{
    Rectangle aGridRect( rPos, Size(nGridDX, nGridDY) );
    pEntry->aGridRect = aGridRect;
    Center( pEntry );
    AdjustVirtSize( pEntry->aRect );
    pGridMap->OccupyGrids( pEntry );
}


void SvxIconChoiceCtrl_Impl::SetCursor( SvxIconChoiceCtrlEntry* pEntry, sal_Bool bSyncSingleSelection,
    sal_Bool bShowFocusAsync )
{
    if( pEntry == pCursor )
    {
        if( pCursor && eSelectionMode == SINGLE_SELECTION && bSyncSingleSelection &&
                !pCursor->IsSelected() )
            SelectEntry( pCursor, sal_True, sal_True );
        return;
    }
    ShowCursor( sal_False );
    SvxIconChoiceCtrlEntry* pOldCursor = pCursor;
    pCursor = pEntry;
    if( pOldCursor )
    {
        pOldCursor->ClearFlags( ICNVIEW_FLAG_FOCUSED );
        if( eSelectionMode == SINGLE_SELECTION && bSyncSingleSelection )
            SelectEntry( pOldCursor, sal_False, sal_True ); // deselect old cursor
    }
    if( pCursor )
    {
        ToTop( pCursor );
        pCursor->SetFlags( ICNVIEW_FLAG_FOCUSED );
        if( eSelectionMode == SINGLE_SELECTION && bSyncSingleSelection )
            SelectEntry( pCursor, sal_True, sal_True );
        if( !bShowFocusAsync )
            ShowCursor( sal_True );
        else
        {
            if( !nUserEventShowCursor )
                nUserEventShowCursor =
                    Application::PostUserEvent( LINK( this, SvxIconChoiceCtrl_Impl, UserEventHdl),
                        EVENTID_SHOW_CURSOR );
        }
    }
}


void SvxIconChoiceCtrl_Impl::ShowCursor( sal_Bool bShow )
{
    if( !pCursor || !bShow || !pView->HasFocus() )
    {
        pView->HideFocus();
        return;
    }
    Rectangle aRect ( CalcFocusRect( pCursor ) );
    /*pView->*/ShowFocus( aRect );
}


void SvxIconChoiceCtrl_Impl::HideDDIcon()
{
    pView->Update();
    ImpHideDDIcon();
    pDDBufDev = pDDDev;
    pDDDev = 0;
}

void SvxIconChoiceCtrl_Impl::ImpHideDDIcon()
{
    if( pDDDev )
    {
        Size aSize( pDDDev->GetOutputSizePixel() );
        // restore pView
        pView->DrawOutDev( aDDLastRectPos, aSize, Point(), aSize, *pDDDev );
    }
}

sal_Bool SvxIconChoiceCtrl_Impl::HandleScrollCommand( const CommandEvent& rCmd )
{
    Rectangle aDocRect( GetDocumentRect() );
    Rectangle aVisRect( GetVisibleRect() );
    if( aVisRect.IsInside( aDocRect ))
        return sal_False;
    Size aDocSize( aDocRect.GetSize() );
    Size aVisSize( aVisRect.GetSize() );
    sal_Bool bHor = aDocSize.Width() > aVisSize.Width();
    sal_Bool bVer = aDocSize.Height() > aVisSize.Height();

    long nScrollDX = 0, nScrollDY = 0;

    switch( rCmd.GetCommand() )
    {
        case COMMAND_STARTAUTOSCROLL:
        {
            pView->EndTracking();
            sal_uInt16 nScrollFlags = 0;
            if( bHor )
                nScrollFlags |= AUTOSCROLL_HORZ;
            if( bVer )
                nScrollFlags |= AUTOSCROLL_VERT;
            if( nScrollFlags )
            {
                pView->StartAutoScroll( nScrollFlags );
                return sal_True;
            }
        }
        break;

        case COMMAND_WHEEL:
        {
            const CommandWheelData* pData = rCmd.GetWheelData();
            if( pData && (COMMAND_WHEEL_SCROLL == pData->GetMode()) && !pData->IsHorz() )
            {
                sal_uLong nScrollLines = pData->GetScrollLines();
                if( nScrollLines == COMMAND_WHEEL_PAGESCROLL )
                {
                    nScrollDY = GetScrollBarPageSize( aVisSize.Width() );
                    if( pData->GetDelta() < 0 )
                        nScrollDY *= -1;
                }
                else
                {
                    nScrollDY = pData->GetNotchDelta() * (long)nScrollLines;
                    nScrollDY *= GetScrollBarLineSize();
                }
            }
        }
        break;

        case COMMAND_AUTOSCROLL:
        {
            const CommandScrollData* pData = rCmd.GetAutoScrollData();
            if( pData )
            {
                nScrollDX = pData->GetDeltaX() * GetScrollBarLineSize();
                nScrollDY = pData->GetDeltaY() * GetScrollBarLineSize();
            }
        }
        break;
    }

    if( nScrollDX || nScrollDY )
    {
        aVisRect.Top() -= nScrollDY;
        aVisRect.Bottom() -= nScrollDY;
        aVisRect.Left() -= nScrollDX;
        aVisRect.Right() -= nScrollDX;
        MakeVisible( aVisRect );
        return sal_True;
    }
    return sal_False;
}


void SvxIconChoiceCtrl_Impl::Command( const CommandEvent& rCEvt )
{
    // scroll mouse event?
    if( (rCEvt.GetCommand() == COMMAND_WHEEL) ||
        (rCEvt.GetCommand() == COMMAND_STARTAUTOSCROLL) ||
        (rCEvt.GetCommand() == COMMAND_AUTOSCROLL) )
    {
        if( HandleScrollCommand( rCEvt ) )
            return;
    }
}

void SvxIconChoiceCtrl_Impl::ToTop( SvxIconChoiceCtrlEntry* pEntry )
{
    if( !pZOrderList->empty()
    &&  pEntry != pZOrderList->back()
    ) {
        for(
            SvxIconChoiceCtrlEntryList_impl::iterator it = pZOrderList->begin();
            it != pZOrderList->end();
            ++it
        ) {
            if ( *it == pEntry )
            {
                pZOrderList->erase( it );
                pZOrderList->push_back( pEntry );
                break;
            }
        }
    }
}

void SvxIconChoiceCtrl_Impl::ClipAtVirtOutRect( Rectangle& rRect ) const
{
    if( rRect.Bottom() >= aVirtOutputSize.Height() )
        rRect.Bottom() = aVirtOutputSize.Height() - 1;
    if( rRect.Right() >= aVirtOutputSize.Width() )
        rRect.Right() = aVirtOutputSize.Width() - 1;
    if( rRect.Top() < 0 )
        rRect.Top() = 0;
    if( rRect.Left() < 0 )
        rRect.Left() = 0;
}

// rRect: area of the document (in document coordinates) that we want to make
// visible
// bScrBar == sal_True: rectangle was calculated because of a scrollbar event

void SvxIconChoiceCtrl_Impl::MakeVisible( const Rectangle& rRect, sal_Bool bScrBar,
    sal_Bool bCallRectChangedHdl )
{
    Rectangle aVirtRect( rRect );
    ClipAtVirtOutRect( aVirtRect );
    Point aOrigin( pView->GetMapMode().GetOrigin() );
    // convert to document coordinate
    aOrigin *= -1;
    Rectangle aOutputArea( GetOutputRect() );
    if( aOutputArea.IsInside( aVirtRect ) )
        return; // is already visible

    long nDy;
    if( aVirtRect.Top() < aOutputArea.Top() )
    {
        // scroll up (nDy < 0)
        nDy = aVirtRect.Top() - aOutputArea.Top();
    }
    else if( aVirtRect.Bottom() > aOutputArea.Bottom() )
    {
        // scroll down (nDy > 0)
        nDy = aVirtRect.Bottom() - aOutputArea.Bottom();
    }
    else
        nDy = 0;

    long nDx;
    if( aVirtRect.Left() < aOutputArea.Left() )
    {
        // scroll to the left (nDx < 0)
        nDx = aVirtRect.Left() - aOutputArea.Left();
    }
    else if( aVirtRect.Right() > aOutputArea.Right() )
    {
        // scroll to the right (nDx > 0)
        nDx = aVirtRect.Right() - aOutputArea.Right();
    }
    else
        nDx = 0;

    aOrigin.X() += nDx;
    aOrigin.Y() += nDy;
    aOutputArea.SetPos( aOrigin );
    if( GetUpdateMode() )
    {
        HideDDIcon();
        pView->Update();
        ShowCursor( sal_False );
    }

    // invert origin for SV (so we can scroll/paint using document coordinates)
    aOrigin *= -1;
    SetOrigin( aOrigin );

    sal_Bool bScrollable = pView->GetBackground().IsScrollable();
    if( pView->HasBackground() && !bScrollable )
    {
        Rectangle aRect( GetOutputRect());
        Wallpaper aPaper( pView->GetBackground() );
        aPaper.SetRect( aRect );
        pView->SetBackground( aPaper );
    }

    if( bScrollable && GetUpdateMode() )
    {
        // scroll in reverse direction!
        pView->Control::Scroll( -nDx, -nDy, aOutputArea,
            SCROLL_NOCHILDREN | SCROLL_USECLIPREGION | SCROLL_CLIP );
    }
    else
        pView->Invalidate(INVALIDATE_NOCHILDREN);

    if( aHorSBar.IsVisible() || aVerSBar.IsVisible() )
    {
        if( !bScrBar )
        {
            aOrigin *= -1;
            // correct thumbs
            if(aHorSBar.IsVisible() && aHorSBar.GetThumbPos() != aOrigin.X())
                aHorSBar.SetThumbPos( aOrigin.X() );
            if(aVerSBar.IsVisible() && aVerSBar.GetThumbPos() != aOrigin.Y())
                aVerSBar.SetThumbPos( aOrigin.Y() );
        }
    }

    if( GetUpdateMode() )
        ShowCursor( sal_True );

    // check if we still need scrollbars
    CheckScrollBars();
    if( bScrollable && GetUpdateMode() )
        pView->Update();

    // If the requested area can not be made completely visible, the
    // Vis-Rect-Changed handler is called in any case. This case may occur e.g.
    // if only few pixels of the lower border are invisible, but a scrollbar has
    // a larger line size.
    if( bCallRectChangedHdl || GetOutputRect() != rRect )
        VisRectChanged();
}

sal_uLong SvxIconChoiceCtrl_Impl::GetSelectionCount() const
{
    if( (nWinBits & WB_HIGHLIGHTFRAME) && pCurHighlightFrame )
        return 1;
    return nSelectionCount;
}

void SvxIconChoiceCtrl_Impl::ToggleSelection( SvxIconChoiceCtrlEntry* pEntry )
{
    sal_Bool bSel;
    if( pEntry->IsSelected() )
        bSel = sal_False;
    else
        bSel = sal_True;
    SelectEntry( pEntry, bSel, sal_True, sal_True );
}

void SvxIconChoiceCtrl_Impl::DeselectAllBut( SvxIconChoiceCtrlEntry* pThisEntryNot,
    sal_Bool bPaintSync )
{
    ClearSelectedRectList();
    //
    // TODO: work through z-order list, if necessary!
    //
    size_t nCount = aEntries.size();
    for( size_t nCur = 0; nCur < nCount; nCur++ )
    {
        SvxIconChoiceCtrlEntry* pEntry = aEntries[ nCur ];
        if( pEntry != pThisEntryNot && pEntry->IsSelected() )
            SelectEntry( pEntry, sal_False, sal_True, sal_True, bPaintSync );
    }
    pAnchor = 0;
    nFlags &= (~F_ADD_MODE);
}

Size SvxIconChoiceCtrl_Impl::GetMinGrid() const
{
    Size aMinSize( aImageSize );
    aMinSize.Width() += 2 * LROFFS_BOUND;
    aMinSize.Height() += TBOFFS_BOUND;  // single offset is enough (FileDlg)
    String aStrDummy( RTL_CONSTASCII_USTRINGPARAM( "XXX" ) );
    Size aTextSize( pView->GetTextWidth( aStrDummy ), pView->GetTextHeight() );
    if( nWinBits & WB_ICON )
    {
        aMinSize.Height() += VER_DIST_BMP_STRING;
        aMinSize.Height() += aTextSize.Height();
    }
    else
    {
        aMinSize.Width() += HOR_DIST_BMP_STRING;
        aMinSize.Width() += aTextSize.Width();
    }
    return aMinSize;
}

void SvxIconChoiceCtrl_Impl::SetGrid( const Size& rSize )
{
    Size aSize( rSize );
    Size aMinSize( GetMinGrid() );
    if( aSize.Width() < aMinSize.Width() )
        aSize.Width() = aMinSize.Width();
    if( aSize.Height() < aMinSize.Height() )
        aSize.Height() = aMinSize.Height();

    nGridDX = aSize.Width();
    // HACK: Detail mode is not yet fully implemented, this workaround makes it
    // fly with a single column
    if( nWinBits & WB_DETAILS )
    {
        const SvxIconChoiceCtrlColumnInfo* pCol = GetColumn( 0 );
        if( pCol )
            ((SvxIconChoiceCtrlColumnInfo*)pCol)->SetWidth( nGridDX );
    }
    nGridDY = aSize.Height();
    SetDefaultTextSize();
}

// Calculates the maximum size that the text rectangle may use within its
// bounding rectangle. In WB_ICON mode with IcnShowTextFull, Bottom is set to
// LONG_MAX.

Rectangle SvxIconChoiceCtrl_Impl::CalcMaxTextRect( const SvxIconChoiceCtrlEntry* pEntry ) const
{
    Rectangle aBoundRect;
    // avoid infinite recursion: don't calculate the bounding rectangle here
    if( IsBoundingRectValid( pEntry->aRect ) )
        aBoundRect = pEntry->aRect;
    else
        aBoundRect = pEntry->aGridRect;

    Rectangle aBmpRect( ((SvxIconChoiceCtrl_Impl*)this)->CalcBmpRect(
        (SvxIconChoiceCtrlEntry*)pEntry ) );
    if( nWinBits & WB_ICON )
    {
        aBoundRect.Top() = aBmpRect.Bottom();
        aBoundRect.Top() += VER_DIST_BMP_STRING;
        if( aBoundRect.Top() > aBoundRect.Bottom())
            aBoundRect.Top() = aBoundRect.Bottom();
        aBoundRect.Left() += LROFFS_BOUND;
        aBoundRect.Left()++;
        aBoundRect.Right() -= LROFFS_BOUND;
        aBoundRect.Right()--;
        if( aBoundRect.Left() > aBoundRect.Right())
            aBoundRect.Left() = aBoundRect.Right();
        if( GetEntryTextModeSmart( pEntry ) == IcnShowTextFull )
            aBoundRect.Bottom() = LONG_MAX;
    }
    else
    {
        aBoundRect.Left() = aBmpRect.Right();
        aBoundRect.Left() += HOR_DIST_BMP_STRING;
        aBoundRect.Right() -= LROFFS_BOUND;
        if( aBoundRect.Left() > aBoundRect.Right() )
            aBoundRect.Left() = aBoundRect.Right();
        long nHeight = aBoundRect.GetSize().Height();
        nHeight = nHeight - aDefaultTextSize.Height();
        nHeight /= 2;
        aBoundRect.Top() += nHeight;
        aBoundRect.Bottom() -= nHeight;
    }
    return aBoundRect;
}

void SvxIconChoiceCtrl_Impl::SetDefaultTextSize()
{
    long nDY = nGridDY;
    nDY -= aImageSize.Height();
    nDY -= VER_DIST_BMP_STRING;
    nDY -= 2*TBOFFS_BOUND;
    if( nDY <= 0 )
        nDY = 2;

    long nDX = nGridDX;
    nDX -= 2*LROFFS_BOUND;
    nDX -= 2;
    if( nDX <= 0 )
        nDX = 2;

    long nHeight = pView->GetTextHeight();
    if( nDY < nHeight )
        nDY = nHeight;
    aDefaultTextSize = Size( nDX, nDY );
}


void SvxIconChoiceCtrl_Impl::Center( SvxIconChoiceCtrlEntry* pEntry ) const
{
    pEntry->aRect = pEntry->aGridRect;
    Size aSize( CalcBoundingSize( pEntry ) );
    if( nWinBits & WB_ICON )
    {
        // center horizontally
        long nBorder = pEntry->aGridRect.GetWidth() - aSize.Width();
        pEntry->aRect.Left() += nBorder / 2;
        pEntry->aRect.Right() -= nBorder / 2;
    }
    // center vertically
    pEntry->aRect.Bottom() = pEntry->aRect.Top() + aSize.Height();
}


// The deltas are the offsets by which the view is moved on the document.
// left, up: offsets < 0
// right, down: offsets > 0
void SvxIconChoiceCtrl_Impl::Scroll( long nDeltaX, long nDeltaY, sal_Bool bScrollBar )
{
    const MapMode& rMapMode = pView->GetMapMode();
    Point aOrigin( rMapMode.GetOrigin() );
    // convert to document coordinate
    aOrigin *= -1;
    aOrigin.Y() += nDeltaY;
    aOrigin.X() += nDeltaX;
    Rectangle aRect( aOrigin, aOutputSize );
    MakeVisible( aRect, bScrollBar );
}


const Size& SvxIconChoiceCtrl_Impl::GetItemSize( SvxIconChoiceCtrlEntry*,
    IcnViewFieldType eItem ) const
{
    if( eItem == IcnViewFieldTypeText )
        return aDefaultTextSize;
    return aImageSize;
}

Rectangle SvxIconChoiceCtrl_Impl::CalcFocusRect( SvxIconChoiceCtrlEntry* pEntry )
{
    Rectangle aBmpRect( CalcBmpRect( pEntry ) );
    Rectangle aTextRect( CalcTextRect( pEntry ) );
    Rectangle aBoundRect( GetEntryBoundRect( pEntry ) );
    Rectangle aFocusRect( aBoundRect.Left(), aBmpRect.Top() - 1,
                          aBoundRect.Right() - 4, aTextRect.Bottom() + 1 );
    // the focus rectangle should not touch the text
    if( aFocusRect.Left() - 1 >= pEntry->aRect.Left() )
        aFocusRect.Left()--;
    if( aFocusRect.Right() + 1 <= pEntry->aRect.Right() )
        aFocusRect.Right()++;

    return aFocusRect;
}

// the hot spot is the inner 50 % of the rectangle
static Rectangle GetHotSpot( const Rectangle& rRect )
{
    Rectangle aResult( rRect );
    aResult.Justify();
    Size aSize( rRect.GetSize() );
    long nDelta = aSize.Width() / 4;
    aResult.Left() += nDelta;
    aResult.Right() -= nDelta;
    nDelta = aSize.Height() / 4;
    aResult.Top() += nDelta;
    aResult.Bottom() -= nDelta;
    return aResult;
}

void SvxIconChoiceCtrl_Impl::SelectRect( SvxIconChoiceCtrlEntry* pEntry1, SvxIconChoiceCtrlEntry* pEntry2,
    sal_Bool bAdd, std::vector<Rectangle*>* pOtherRects )
{
    DBG_ASSERT(pEntry1 && pEntry2,"SelectEntry: Invalid Entry-Ptr");
    Rectangle aRect( GetEntryBoundRect( pEntry1 ) );
    aRect.Union( GetEntryBoundRect( pEntry2 ) );
    SelectRect( aRect, bAdd, pOtherRects );
}

void SvxIconChoiceCtrl_Impl::SelectRect( const Rectangle& rRect, bool bAdd,
    std::vector<Rectangle*>* pOtherRects )
{
    aCurSelectionRect = rRect;
    if( !pZOrderList || !pZOrderList->size() )
        return;

    // set flag, so ToTop won't be called in Select
    sal_Bool bAlreadySelectingRect = nFlags & F_SELECTING_RECT ? sal_True : sal_False;
    nFlags |= F_SELECTING_RECT;

    CheckBoundingRects();
    pView->Update();
    const size_t nCount = pZOrderList->size();

    Rectangle aRect( rRect );
    aRect.Justify();
    bool bCalcOverlap = (bAdd && pOtherRects && !pOtherRects->empty()) ? true : false;

    sal_Bool bResetClipRegion = sal_False;
    if( !pView->IsClipRegion() )
    {
        bResetClipRegion = sal_True;
        pView->SetClipRegion(Region(GetOutputRect()));
    }

    for( size_t nPos = 0; nPos < nCount; nPos++ )
    {
        SvxIconChoiceCtrlEntry* pEntry = (*pZOrderList)[ nPos ];

        if( !IsBoundingRectValid( pEntry->aRect ))
            FindBoundingRect( pEntry );
        Rectangle aBoundRect( GetHotSpot( pEntry->aRect ) );
        sal_Bool bSelected = pEntry->IsSelected();

        sal_Bool bOverlaps;
        if( bCalcOverlap )
            bOverlaps = IsOver( pOtherRects, aBoundRect );
        else
            bOverlaps = sal_False;
        sal_Bool bOver = aRect.IsOver( aBoundRect );

        if( bOver && !bOverlaps )
        {
            // is inside the new selection rectangle and outside of any old one
            // => select
            if( !bSelected )
                SelectEntry( pEntry, sal_True, sal_True, sal_True );
        }
        else if( !bAdd )
        {
            // is outside of the selection rectangle
            // => deselect
            if( bSelected )
                SelectEntry( pEntry, sal_False, sal_True, sal_True );
        }
        else if( bAdd && bOverlaps )
        {
            // The entry is inside an old (=>span multiple rectangles with Ctrl)
            // selection rectangle.

            // There is still a bug here! The selection status of an entry in a
            // previous rectangle has to be restored, if it was touched by the
            // current selection rectangle but is not inside it any more.
            // For simplicity's sake, let's assume that all entries in the old
            // rectangles were correctly selected. It is wrong to just deselect
            // the intersection.
            // Possible solution: remember a snapshot of the selection before
            // spanning the rectangle.
            if( aBoundRect.IsOver( rRect))
            {
                // deselect intersection between old rectangles and current rectangle
                if( bSelected )
                    SelectEntry( pEntry, sal_False, sal_True, sal_True );
            }
            else
            {
                // select entry of an old rectangle
                if( !bSelected )
                    SelectEntry( pEntry, sal_True, sal_True, sal_True );
            }
        }
        else if( !bOver && bSelected )
        {
            // this entry is completely outside the rectangle => deselect it
            SelectEntry( pEntry, sal_False, sal_True, sal_True );
        }
    }

    if( !bAlreadySelectingRect )
        nFlags &= ~F_SELECTING_RECT;

    pView->Update();
    if( bResetClipRegion )
        pView->SetClipRegion();
}

void SvxIconChoiceCtrl_Impl::SelectRange(
                        SvxIconChoiceCtrlEntry* pStart,
                        SvxIconChoiceCtrlEntry* pEnd,
                        sal_Bool bAdd )
{
    sal_uLong nFront = GetEntryListPos( pStart );
    sal_uLong nBack  = GetEntryListPos( pEnd );
    sal_uLong nFirst = std::min( nFront, nBack );
    sal_uLong nLast  = std::max( nFront, nBack );
    sal_uLong i;
    SvxIconChoiceCtrlEntry* pEntry;

    if ( ! bAdd )
    {
        // deselect everything before the first entry if not in
        // adding mode
        for ( i=0; i<nFirst; i++ )
        {
            pEntry = GetEntry( i );
            if( pEntry->IsSelected() )
                SelectEntry( pEntry, sal_False, sal_True, sal_True, sal_True );
        }
    }

    // select everything between nFirst and nLast
    for ( i=nFirst; i<=nLast; i++ )
    {
        pEntry = GetEntry( i );
        if( ! pEntry->IsSelected() )
            SelectEntry( pEntry, sal_True, sal_True,  sal_True, sal_True );
    }

    if ( ! bAdd )
    {
        // deselect everything behind the last entry if not in
        // adding mode
        sal_uLong nEnd = GetEntryCount();
        for ( ; i<nEnd; i++ )
        {
            pEntry = GetEntry( i );
            if( pEntry->IsSelected() )
                SelectEntry( pEntry, sal_False, sal_True, sal_True, sal_True );
        }
    }
}

bool SvxIconChoiceCtrl_Impl::IsOver( std::vector<Rectangle*>* pRectList, const Rectangle& rBoundRect ) const
{
    const sal_uInt16 nCount = pRectList->size();
    for( sal_uInt16 nCur = 0; nCur < nCount; nCur++ )
    {
        Rectangle* pRect = (*pRectList)[ nCur ];
        if( rBoundRect.IsOver( *pRect ))
            return true;
    }
    return false;
}

void SvxIconChoiceCtrl_Impl::AddSelectedRect( SvxIconChoiceCtrlEntry* pEntry1,
    SvxIconChoiceCtrlEntry* pEntry2 )
{
    DBG_ASSERT(pEntry1 && pEntry2,"SelectEntry: Invalid Entry-Ptr");
    Rectangle aRect( GetEntryBoundRect( pEntry1 ) );
    aRect.Union( GetEntryBoundRect( pEntry2 ) );
    AddSelectedRect( aRect );
}

void SvxIconChoiceCtrl_Impl::AddSelectedRect( const Rectangle& rRect )
{
    Rectangle* pRect = new Rectangle( rRect );
    pRect->Justify();
    aSelectedRectList.push_back( pRect );
}

void SvxIconChoiceCtrl_Impl::ClearSelectedRectList()
{
    const sal_uInt16 nCount = aSelectedRectList.size();
    for( sal_uInt16 nCur = 0; nCur < nCount; nCur++ )
    {
        Rectangle* pRect = aSelectedRectList[ nCur ];
        delete pRect;
    }
    aSelectedRectList.clear();
}

IMPL_LINK_NOARG(SvxIconChoiceCtrl_Impl, AutoArrangeHdl)
{
    aAutoArrangeTimer.Stop();
    Arrange( IsAutoArrange() );
    return 0;
}

IMPL_LINK_NOARG(SvxIconChoiceCtrl_Impl, VisRectChangedHdl)
{
    aVisRectChangedTimer.Stop();
    pView->VisibleRectChanged();
    return 0;
}

IMPL_LINK_NOARG(SvxIconChoiceCtrl_Impl, DocRectChangedHdl)
{
    aDocRectChangedTimer.Stop();
    pView->DocumentRectChanged();
    return 0;
}

sal_Bool SvxIconChoiceCtrl_Impl::IsTextHit( SvxIconChoiceCtrlEntry* pEntry, const Point& rDocPos )
{
    Rectangle aRect( CalcTextRect( pEntry ));
    if( aRect.IsInside( rDocPos ) )
        return sal_True;
    return sal_False;
}

IMPL_LINK_NOARG(SvxIconChoiceCtrl_Impl, EditTimeoutHdl)
{
    SvxIconChoiceCtrlEntry* pEntry = GetCurEntry();
    if( bEntryEditingEnabled && pEntry &&
        pEntry->IsSelected())
    {
        if( pView->EditingEntry( pEntry ))
            EditEntry( pEntry );
    }
    return 0;
}


//
// Function to align entries to the grid
//

// pStart == 0: align all entries
// else: align all entries of the row from pStart on (including pStart)
void SvxIconChoiceCtrl_Impl::AdjustEntryAtGrid( SvxIconChoiceCtrlEntry* pStart )
{
    IconChoiceMap aLists;
    pImpCursor->CreateGridAjustData( aLists, pStart );
    for (IconChoiceMap::const_iterator iter = aLists.begin();
            iter != aLists.end(); ++iter)
    {
        AdjustAtGrid(iter->second, pStart);
    }
    IcnCursor_Impl::DestroyGridAdjustData( aLists );
    CheckScrollBars();
}

// align a row, might expand width, doesn't break the line
void SvxIconChoiceCtrl_Impl::AdjustAtGrid( const SvxIconChoiceCtrlEntryPtrVec& rRow, SvxIconChoiceCtrlEntry* pStart )
{
    if( rRow.empty() )
        return;

    sal_Bool bGo;
    if( !pStart )
        bGo = sal_True;
    else
        bGo = sal_False;

    long nCurRight = 0;
    for( sal_uInt16 nCur = 0; nCur < rRow.size(); nCur++ )
    {
        SvxIconChoiceCtrlEntry* pCur = rRow[ nCur ];
        if( !bGo && pCur == pStart )
            bGo = sal_True;

        // SvIcnVwDataEntry* pViewData = ICNVIEWDATA(pCur);
        // Decisive (for our eye) is the bitmap, else, the entry might jump too
        // much within long texts.
        const Rectangle& rBoundRect = GetEntryBoundRect( pCur );
        Rectangle aCenterRect( CalcBmpRect( pCur, 0 ));
        if( bGo && !pCur->IsPosLocked() )
        {
            long nWidth = aCenterRect.GetSize().Width();
            Point aNewPos( AdjustAtGrid( aCenterRect, rBoundRect ) );
            while( aNewPos.X() < nCurRight )
                aNewPos.X() += nGridDX;
            if( aNewPos != rBoundRect.TopLeft() )
            {
                SetEntryPos( pCur, aNewPos );
                pCur->SetFlags( ICNVIEW_FLAG_POS_MOVED );
                nFlags |= F_MOVED_ENTRIES;
            }
            nCurRight = aNewPos.X() + nWidth;
        }
        else
        {
            nCurRight = rBoundRect.Right();
        }
    }
}

// Aligns a rectangle to the grid, but doesn't guarantee that the new position
// is vacant. The position can be used for SetEntryPos. The CenterRect describes
// a part of the bounding rectangle that is used for calculating the target
// rectangle.
Point SvxIconChoiceCtrl_Impl::AdjustAtGrid( const Rectangle& rCenterRect,
    const Rectangle& rBoundRect ) const
{
    Point aPos( rCenterRect.TopLeft() );
    Size aSize( rCenterRect.GetSize() );

    aPos.X() -= LROFFS_WINBORDER;
    aPos.Y() -= TBOFFS_WINBORDER;

    // align (the center of the rectangle is the reference)
    short nGridX = (short)((aPos.X()+(aSize.Width()/2)) / nGridDX);
    short nGridY = (short)((aPos.Y()+(aSize.Height()/2)) / nGridDY);
    aPos.X() = nGridX * nGridDX;
    aPos.Y() = nGridY * nGridDY;
    // horizontal center
    aPos.X() += (nGridDX - rBoundRect.GetSize().Width() ) / 2;

    aPos.X() += LROFFS_WINBORDER;
    aPos.Y() += TBOFFS_WINBORDER;

    return aPos;
}

void SvxIconChoiceCtrl_Impl::SetEntryTextMode( SvxIconChoiceCtrlTextMode eMode, SvxIconChoiceCtrlEntry* pEntry )
{
    if( !pEntry )
    {
        if( eTextMode != eMode )
        {
            if( eTextMode == IcnShowTextDontKnow )
                eTextMode = IcnShowTextShort;
            eTextMode = eMode;
            Arrange( sal_True );
        }
    }
    else
    {
        if( pEntry->eTextMode != eMode )
        {
            pEntry->eTextMode = eMode;
            InvalidateEntry( pEntry );
            pView->Invalidate( GetEntryBoundRect( pEntry ) );
            AdjustVirtSize( pEntry->aRect );
        }
    }
}

SvxIconChoiceCtrlTextMode SvxIconChoiceCtrl_Impl::GetEntryTextModeSmart( const SvxIconChoiceCtrlEntry* pEntry ) const
{
    DBG_ASSERT(pEntry,"GetEntryTextModeSmart: Entry not set");
    SvxIconChoiceCtrlTextMode eMode = pEntry->GetTextMode();
    if( eMode == IcnShowTextDontKnow )
        return eTextMode;
    return eMode;
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Draw my own focusrect, because the focusrect of the outputdevice has got the inverted color
// of the background. But what will we see, if the backgroundcolor is gray ? - We will see
// a gray focusrect on a gray background !!!
//
void SvxIconChoiceCtrl_Impl::ShowFocus ( Rectangle& rRect )
{
    Color aBkgColor ( pView->GetBackground().GetColor() );
    Color aPenColor;
    sal_uInt16 nColor = ( aBkgColor.GetRed() + aBkgColor.GetGreen() + aBkgColor.GetBlue() ) / 3;
    if ( nColor > 128 )
        aPenColor.SetColor ( COL_BLACK );
    else
        aPenColor.SetColor( COL_WHITE );

    aFocus.bOn = sal_True;
    aFocus.aPenColor = aPenColor;
    aFocus.aRect = rRect;
}

void SvxIconChoiceCtrl_Impl::DrawFocusRect ( OutputDevice* pOut )
{
    pOut->SetLineColor( aFocus.aPenColor );
    pOut->SetFillColor();
    Polygon aPolygon ( aFocus.aRect );

    LineInfo aLineInfo ( LINE_DASH );

    aLineInfo.SetDashLen ( 1 );

    aLineInfo.SetDotLen ( 1L );
    aLineInfo.SetDistance ( 1L );
    aLineInfo.SetDotCount ( 1 );

    pOut->DrawPolyLine ( aPolygon, aLineInfo );
}

sal_Bool SvxIconChoiceCtrl_Impl::IsMnemonicChar( sal_Unicode cChar, sal_uLong& rPos ) const
{
    sal_Bool bRet = sal_False;
    const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetUILocaleI18nHelper();
    size_t nEntryCount = GetEntryCount();
    for ( size_t i = 0; i < nEntryCount; ++i )
    {
        if ( rI18nHelper.MatchMnemonic( GetEntry( i )->GetText(), cChar ) )
        {
            bRet = sal_True;
            rPos = i;
            break;
        }
    }

    return bRet;
}

//
////////////////////////////////////////////////////////////////////////////////////////////////

IMPL_LINK(SvxIconChoiceCtrl_Impl, UserEventHdl, void*, nId )
{
    if( nId == EVENTID_ADJUST_SCROLLBARS )
    {
        nUserEventAdjustScrBars = 0;
        AdjustScrollBars();
    }
    else if( nId == EVENTID_SHOW_CURSOR )
    {
        nUserEventShowCursor = 0;
        ShowCursor( sal_True );
    }
    return 0;
}

void SvxIconChoiceCtrl_Impl::CancelUserEvents()
{
    if( nUserEventAdjustScrBars )
    {
        Application::RemoveUserEvent( nUserEventAdjustScrBars );
        nUserEventAdjustScrBars = 0;
    }
    if( nUserEventShowCursor )
    {
        Application::RemoveUserEvent( nUserEventShowCursor );
        nUserEventShowCursor = 0;
    }
}

void SvxIconChoiceCtrl_Impl::InvalidateEntry( SvxIconChoiceCtrlEntry* pEntry )
{
    if( pEntry == pCursor )
        ShowCursor( sal_False );
    pView->Invalidate( pEntry->aRect );
    Center( pEntry );
    pView->Invalidate( pEntry->aRect );
    if( pEntry == pCursor )
        ShowCursor( sal_True );
}

void SvxIconChoiceCtrl_Impl::EditEntry( SvxIconChoiceCtrlEntry* pEntry )
{
    DBG_ASSERT(pEntry,"EditEntry: Entry not set");
    if( !pEntry )
        return;

    StopEntryEditing( sal_True );
    DELETEZ(pEdit);
    SetNoSelection();

    pCurEditedEntry = pEntry;
    String aEntryText( pView->GetEntryText( pEntry, sal_True ) );
    Rectangle aRect( CalcTextRect( pEntry, 0, sal_True, &aEntryText ) );
    MakeVisible( aRect );
    Point aPos( aRect.TopLeft() );
    aPos = pView->GetPixelPos( aPos );
    aRect.SetPos( aPos );
    pView->HideFocus();
    pEdit = new IcnViewEdit_Impl(
        pView,
        aRect.TopLeft(),
        aRect.GetSize(),
        aEntryText,
        LINK( this, SvxIconChoiceCtrl_Impl, TextEditEndedHdl ) );
}

IMPL_LINK_NOARG(SvxIconChoiceCtrl_Impl, TextEditEndedHdl)
{
    DBG_ASSERT(pEdit,"TextEditEnded: pEdit not set");
    if( !pEdit )
    {
        pCurEditedEntry = 0;
        return 0;
    }
    DBG_ASSERT(pCurEditedEntry,"TextEditEnded: pCurEditedEntry not set");

    if( !pCurEditedEntry )
    {
        pEdit->Hide();
        if( pEdit->IsGrabFocus() )
            pView->GrabFocus();
        return 0;
    }

    String aText;
    if ( !pEdit->EditingCanceled() )
        aText = pEdit->GetText();
    else
        aText = pEdit->GetSavedValue();

    if( pView->EditedEntry( pCurEditedEntry, aText, pEdit->EditingCanceled() ) )
        InvalidateEntry( pCurEditedEntry );
    if( !GetSelectionCount() )
        SelectEntry( pCurEditedEntry, sal_True );

    pEdit->Hide();
    if( pEdit->IsGrabFocus() )
        pView->GrabFocus();
    // The edit can not be deleted here, because it is not within a handler. It
    // will be deleted in the dtor or in the next EditEntry.
    pCurEditedEntry = 0;
    return 0;
}

void SvxIconChoiceCtrl_Impl::StopEntryEditing( sal_Bool bCancel )
{
    if( pEdit )
        pEdit->StopEditing( bCancel );
}

SvxIconChoiceCtrlEntry* SvxIconChoiceCtrl_Impl::GetFirstSelectedEntry( sal_uLong& rPos ) const
{
    if( !GetSelectionCount() )
        return 0;

    if( (nWinBits & WB_HIGHLIGHTFRAME) && (eSelectionMode == NO_SELECTION) )
    {
        rPos = pView->GetEntryListPos( pCurHighlightFrame );
        return pCurHighlightFrame;
    }

    size_t nCount = aEntries.size();
    if( !pHead )
    {
        for( size_t nCur = 0; nCur < nCount; nCur++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = aEntries[ nCur ];
            if( pEntry->IsSelected() )
            {
                rPos = nCur;
                return pEntry;
            }
        }
    }
    else
    {
        SvxIconChoiceCtrlEntry* pEntry = pHead;
        while( nCount-- )
        {
            if( pEntry->IsSelected() )
            {
                rPos = GetEntryListPos( pEntry );
                return pEntry;
            }
            pEntry = pEntry->pflink;
            if( nCount && pEntry == pHead )
            {
                OSL_FAIL("SvxIconChoiceCtrl_Impl::GetFirstSelectedEntry > infinite loop!");
                return 0;
            }
        }
    }
    return 0;
}

void SvxIconChoiceCtrl_Impl::SelectAll( sal_Bool bSelect, sal_Bool bPaint )
{
    bPaint = sal_True;

    size_t nCount = aEntries.size();
    for( size_t nCur = 0; nCur < nCount && (bSelect || GetSelectionCount() ); nCur++ )
    {
        SvxIconChoiceCtrlEntry* pEntry = aEntries[ nCur ];
        SelectEntry( pEntry, bSelect, sal_True, sal_True, bPaint );
    }
    nFlags &= (~F_ADD_MODE);
    pAnchor = 0;
}

IcnViewEdit_Impl::IcnViewEdit_Impl( SvtIconChoiceCtrl* pParent, const Point& rPos,
    const Size& rSize, const OUString& rData, const Link& rNotifyEditEnd ) :
    MultiLineEdit( pParent, (pParent->GetStyle() & WB_ICON) ? WB_CENTER : WB_LEFT),
    aCallBackHdl( rNotifyEditEnd ),
    bCanceled( sal_False ),
    bAlreadyInCallback( sal_False ),
    bGrabFocus( sal_False )
{
    Font aFont( pParent->GetPointFont() );
    aFont.SetTransparent( sal_False );
    SetControlFont( aFont );
    if( !pParent->HasFontFillColor() )
    {
        Color aColor( pParent->GetBackground().GetColor() );
        SetControlBackground( aColor );
    }
    else
        SetControlBackground( aFont.GetFillColor() );
    SetControlForeground( aFont.GetColor() );
    SetPosPixel( rPos );
    SetSizePixel( CalcAdjustedSize(rSize) );
    SetText( rData );
    SaveValue();

    aAccReturn.InsertItem( IMPICNVIEW_ACC_RETURN, KeyCode(KEY_RETURN) );
    aAccEscape.InsertItem( IMPICNVIEW_ACC_ESCAPE, KeyCode(KEY_ESCAPE) );

    aAccReturn.SetActivateHdl( LINK( this, IcnViewEdit_Impl, ReturnHdl_Impl) );
    aAccEscape.SetActivateHdl( LINK( this, IcnViewEdit_Impl, EscapeHdl_Impl) );
    GetpApp()->InsertAccel( &aAccReturn);//, ACCEL_ALWAYS );
    GetpApp()->InsertAccel( &aAccEscape);//, ACCEL_ALWAYS );
    Show();
    GrabFocus();
}

IcnViewEdit_Impl::~IcnViewEdit_Impl()
{
    if( !bAlreadyInCallback )
    {
        GetpApp()->RemoveAccel( &aAccReturn );
        GetpApp()->RemoveAccel( &aAccEscape );
    }
}

void IcnViewEdit_Impl::CallCallBackHdl_Impl()
{
    aTimer.Stop();
    if ( !bAlreadyInCallback )
    {
        bAlreadyInCallback = sal_True;
        GetpApp()->RemoveAccel( &aAccReturn );
        GetpApp()->RemoveAccel( &aAccEscape );
        Hide();
        aCallBackHdl.Call( this );
    }
}

IMPL_LINK_NOARG(IcnViewEdit_Impl, Timeout_Impl)
{
    CallCallBackHdl_Impl();
    return 0;
}

IMPL_LINK( IcnViewEdit_Impl, ReturnHdl_Impl, Accelerator*, EMPTYARG  )
{
    bCanceled = sal_False;
    bGrabFocus = sal_True;
    CallCallBackHdl_Impl();
    return 1;
}

IMPL_LINK( IcnViewEdit_Impl, EscapeHdl_Impl, Accelerator*, EMPTYARG  )
{
    bCanceled = sal_True;
    bGrabFocus = sal_True;
    CallCallBackHdl_Impl();
    return 1;
}

void IcnViewEdit_Impl::KeyInput( const KeyEvent& rKEvt )
{
    KeyCode aCode = rKEvt.GetKeyCode();
    sal_uInt16 nCode = aCode.GetCode();

    switch ( nCode )
    {
        case KEY_ESCAPE:
            bCanceled = sal_True;
            bGrabFocus = sal_True;
            CallCallBackHdl_Impl();
            break;

        case KEY_RETURN:
            bCanceled = sal_False;
            bGrabFocus = sal_True;
            CallCallBackHdl_Impl();
            break;

        default:
            MultiLineEdit::KeyInput( rKEvt );
    }
}

long IcnViewEdit_Impl::PreNotify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == EVENT_LOSEFOCUS )
    {
        if ( !bAlreadyInCallback &&
            ((!Application::GetFocusWindow()) || !IsChild(Application::GetFocusWindow())))
        {
            bCanceled = sal_False;
            aTimer.SetTimeout(10);
            aTimer.SetTimeoutHdl(LINK(this,IcnViewEdit_Impl,Timeout_Impl));
            aTimer.Start();
        }
    }
    return 0;
}

void IcnViewEdit_Impl::StopEditing( sal_Bool bCancel )
{
    if ( !bAlreadyInCallback )
    {
        bCanceled = bCancel;
        CallCallBackHdl_Impl();
    }
}

sal_uLong SvxIconChoiceCtrl_Impl::GetEntryListPos( SvxIconChoiceCtrlEntry* pEntry ) const
{
    if( !(nFlags & F_ENTRYLISTPOS_VALID ))
        ((SvxIconChoiceCtrl_Impl*)this)->SetListPositions();
    return pEntry->nPos;
}

void SvxIconChoiceCtrl_Impl::InitSettings()
{
    const StyleSettings& rStyleSettings = pView->GetSettings().GetStyleSettings();

    if( !pView->HasFont() )
    {
        // unit (from settings) is Point
        Font aFont( rStyleSettings.GetFieldFont() );
        //const Font& rFont = pView->GetFont();
        //if( pView->HasFontTextColor() )
            aFont.SetColor( rStyleSettings.GetWindowTextColor() );
        //if( pView->HasFontFillColor() )
            //aFont.SetFillColor( rFont.GetFillColor() );
        pView->SetPointFont( aFont );
        SetDefaultTextSize();
    }

    //if( !pView->HasFontTextColor() )
        pView->SetTextColor( rStyleSettings.GetFieldTextColor() );
    //if( !pView->HasFontFillColor() )
        pView->SetTextFillColor();

    //if( !pView->HasBackground() )
        pView->SetBackground( rStyleSettings.GetFieldColor());

    long nScrBarSize = rStyleSettings.GetScrollBarSize();
    if( nScrBarSize != nHorSBarHeight || nScrBarSize != nVerSBarWidth )
    {
        nHorSBarHeight = nScrBarSize;
        Size aSize( aHorSBar.GetSizePixel() );
        aSize.Height() = nScrBarSize;
        aHorSBar.Hide();
        aHorSBar.SetSizePixel( aSize );

        nVerSBarWidth = nScrBarSize;
        aSize = aVerSBar.GetSizePixel();
        aSize.Width() = nScrBarSize;
        aVerSBar.Hide();
        aVerSBar.SetSizePixel( aSize );

        Size aOSize( pView->Control::GetOutputSizePixel() );
        PositionScrollBars( aOSize.Width(), aOSize.Height() );
        AdjustScrollBars();
    }
}

EntryList_Impl::EntryList_Impl( SvxIconChoiceCtrl_Impl* pOwner ) :
    _pOwner( pOwner )
{
    _pOwner->pHead = 0;
}

EntryList_Impl::~EntryList_Impl()
{
    _pOwner->pHead = 0;
}

void EntryList_Impl::clear()
{
    _pOwner->pHead = 0;
    maIconChoiceCtrlEntryList.clear();
}

void EntryList_Impl::insert( size_t nPos, SvxIconChoiceCtrlEntry* pEntry )
{
    if ( nPos < maIconChoiceCtrlEntryList.size() ) {
        maIconChoiceCtrlEntryList.insert( maIconChoiceCtrlEntryList.begin() + nPos, pEntry );
    } else {
        maIconChoiceCtrlEntryList.push_back( pEntry );
    }
    if( _pOwner->pHead )
        pEntry->SetBacklink( _pOwner->pHead->pblink );
}

void SvxIconChoiceCtrl_Impl::SetPositionMode( SvxIconChoiceCtrlPositionMode eMode )
{
    if( eMode == ePositionMode )
        return;

    SvxIconChoiceCtrlPositionMode eOldMode = ePositionMode;
    ePositionMode = eMode;
    size_t nCount = aEntries.size();

    if( eOldMode == IcnViewPositionModeAutoArrange )
    {
        // when positioning moved entries "hard", there are problems with
        // unwanted overlaps, as these entries aren't taken into account in
        // Arrange.
        if( aEntries.size() )
            aAutoArrangeTimer.Start();
        return;
    }

    if( ePositionMode == IcnViewPositionModeAutoArrange )
    {
        for( size_t nCur = 0; nCur < nCount; nCur++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = aEntries[ nCur ];
            if( pEntry->GetFlags() & (ICNVIEW_FLAG_POS_LOCKED | ICNVIEW_FLAG_POS_MOVED))
                SetEntryPos(pEntry, GetEntryBoundRect( pEntry ).TopLeft());
        }

        if( aEntries.size() )
            aAutoArrangeTimer.Start();
    }
    else if( ePositionMode == IcnViewPositionModeAutoAdjust )
    {
        AdjustEntryAtGrid( 0 );
    }
}

void SvxIconChoiceCtrl_Impl::SetEntryPredecessor( SvxIconChoiceCtrlEntry* pEntry,
    SvxIconChoiceCtrlEntry* pPredecessor )
{
    if( !IsAutoArrange() )
        return;

    if( pEntry == pPredecessor )
        return;

    sal_uLong nPos1 = GetEntryListPos( pEntry );
    if( !pHead )
    {
        if( pPredecessor )
        {
            sal_uLong nPos2 = GetEntryListPos( pPredecessor );
            if( nPos1 == (nPos2 + 1) )
                return; // is already the predecessor
        }
        else if( !nPos1 )
            return;
    }

    if( !pHead )
        InitPredecessors();

    if( !pPredecessor && pHead == pEntry )
        return; // is already the first one

    sal_Bool bSetHead = sal_False;
    if( !pPredecessor )
    {
        bSetHead = sal_True;
        pPredecessor = pHead->pblink;
    }
    if( pEntry == pHead )
    {
        pHead = pHead->pflink;
        bSetHead = sal_False;
    }
    if( pEntry != pPredecessor )
    {
        pEntry->Unlink();
        pEntry->SetBacklink( pPredecessor );
    }
    if( bSetHead )
        pHead = pEntry;
    pEntry->SetFlags( ICNVIEW_FLAG_PRED_SET );
    aAutoArrangeTimer.Start();
}

SvxIconChoiceCtrlEntry* SvxIconChoiceCtrl_Impl::FindEntryPredecessor( SvxIconChoiceCtrlEntry* pEntry,
    const Point& rPosTopLeft )
{
    Point aPos( rPosTopLeft ); //TopLeft
    Rectangle aCenterRect( CalcBmpRect( pEntry, &aPos ));
    Point aNewPos( aCenterRect.Center() );
    sal_uLong nGrid = GetPredecessorGrid( aNewPos );
    size_t nCount = aEntries.size();
    if( nGrid == ULONG_MAX )
        return 0;
    if( nGrid >= nCount )
        nGrid = nCount - 1;
    if( !pHead )
        return aEntries[ nGrid ];

    SvxIconChoiceCtrlEntry* pCur = pHead; // Grid 0
    // TODO: go through list from the end if nGrid > nCount/2
    for( sal_uLong nCur = 0; nCur < nGrid; nCur++ )
        pCur = pCur->pflink;

    return pCur;
}

sal_uLong SvxIconChoiceCtrl_Impl::GetPredecessorGrid( const Point& rPos) const
{
    Point aPos( rPos );
    aPos.X() -= LROFFS_WINBORDER;
    aPos.Y() -= TBOFFS_WINBORDER;
    sal_uInt16 nMaxCol = (sal_uInt16)(aVirtOutputSize.Width() / nGridDX);
    if( nMaxCol )
        nMaxCol--;
    sal_uInt16 nGridX = (sal_uInt16)(aPos.X() / nGridDX);
    if( nGridX > nMaxCol )
        nGridX = nMaxCol;
    sal_uInt16 nGridY = (sal_uInt16)(aPos.Y() / nGridDY);
    sal_uInt16 nGridsX = (sal_uInt16)(aOutputSize.Width() / nGridDX);
    sal_uLong nGrid = (nGridY * nGridsX) + nGridX;
    long nMiddle = (nGridX * nGridDX) + (nGridDX / 2);
    if( rPos.X() < nMiddle )
    {
        if( !nGrid )
            nGrid = ULONG_MAX;
        else
            nGrid--;
    }
    return nGrid;
}

sal_Bool SvxIconChoiceCtrl_Impl::RequestHelp( const HelpEvent& rHEvt )
{
    if ( !(rHEvt.GetMode() & HELPMODE_QUICK ) )
        return sal_False;

    Point aPos( pView->ScreenToOutputPixel(rHEvt.GetMousePosPixel() ) );
    aPos -= pView->GetMapMode().GetOrigin();
    SvxIconChoiceCtrlEntry* pEntry = GetEntry( aPos, sal_True );

    if ( !pEntry )
        return sal_False;

    String sQuickHelpText = pEntry->GetQuickHelpText();
    String aEntryText( pView->GetEntryText( pEntry, sal_False ) );
    Rectangle aTextRect( CalcTextRect( pEntry, 0, sal_False, &aEntryText ) );
    if ( ( !aTextRect.IsInside( aPos ) || !aEntryText.Len() ) && !sQuickHelpText.Len() )
        return sal_False;

    Rectangle aOptTextRect( aTextRect );
    aOptTextRect.Bottom() = LONG_MAX;
    sal_uInt16 nNewFlags = nCurTextDrawFlags;
    nNewFlags &= ~( TEXT_DRAW_CLIP | TEXT_DRAW_ENDELLIPSIS );
    aOptTextRect = pView->GetTextRect( aOptTextRect, aEntryText, nNewFlags );
    if ( aOptTextRect != aTextRect || sQuickHelpText.Len() > 0 )
    {
        //aTextRect.Right() = aTextRect.Left() + aRealSize.Width() + 4;
        Point aPt( aOptTextRect.TopLeft() );
        aPt += pView->GetMapMode().GetOrigin();
        aPt = pView->OutputToScreenPixel( aPt );
        // subtract border of tooltip help
        aPt.Y() -= 1;
        aPt.X() -= 3;
        aOptTextRect.SetPos( aPt );
        String sHelpText;
        if ( sQuickHelpText.Len() > 0 )
            sHelpText = sQuickHelpText;
        else
            sHelpText = aEntryText;
        Help::ShowQuickHelp( (Window*)pView, aOptTextRect, sHelpText, QUICKHELP_LEFT | QUICKHELP_VCENTER );
    }

    return sal_True;
}

void SvxIconChoiceCtrl_Impl::ClearColumnList()
{
    if( !pColumns )
        return;

    pColumns->clear();
    DELETEZ(pColumns);
}

void SvxIconChoiceCtrl_Impl::SetColumn( sal_uInt16 nIndex, const SvxIconChoiceCtrlColumnInfo& rInfo)
{
    if( !pColumns )
        pColumns = new SvxIconChoiceCtrlColumnInfoMap;

    SvxIconChoiceCtrlColumnInfo* pInfo = new SvxIconChoiceCtrlColumnInfo( rInfo );
    pColumns->insert( nIndex,  pInfo );

    // HACK: Detail mode is not yet fully implemented, this workaround makes it
    // fly with a single column
    if( !nIndex && (nWinBits & WB_DETAILS) )
        nGridDX = pInfo->GetWidth();

    if( GetUpdateMode() )
        Arrange( IsAutoArrange() );
}

const SvxIconChoiceCtrlColumnInfo* SvxIconChoiceCtrl_Impl::GetColumn( sal_uInt16 nIndex ) const
{
    if (!pColumns)
        return 0;
    SvxIconChoiceCtrlColumnInfoMap::const_iterator it = pColumns->find( nIndex );
    if( it == pColumns->end() )
        return 0;
    return it->second;
}

void SvxIconChoiceCtrl_Impl::DrawHighlightFrame(
    OutputDevice* pOut, const Rectangle& rBmpRect, sal_Bool bHide )
{
    Rectangle aBmpRect( rBmpRect );
    long nBorder = 2;
    if( aImageSize.Width() < 32 )
        nBorder = 1;
    aBmpRect.Right() += nBorder;
    aBmpRect.Left() -= nBorder;
    aBmpRect.Bottom() += nBorder;
    aBmpRect.Top() -= nBorder;

    if ( bHide )
        pView->Invalidate( aBmpRect );
    else
    {
        DecorationView aDecoView( pOut );
        sal_uInt16 nDecoFlags;
        if ( bHighlightFramePressed )
            nDecoFlags = FRAME_HIGHLIGHT_TESTBACKGROUND | FRAME_HIGHLIGHT_IN;
        else
            nDecoFlags = FRAME_HIGHLIGHT_TESTBACKGROUND | FRAME_HIGHLIGHT_OUT;
        aDecoView.DrawHighlightFrame( aBmpRect, nDecoFlags );
    }
}

void SvxIconChoiceCtrl_Impl::SetEntryHighlightFrame( SvxIconChoiceCtrlEntry* pEntry,
    sal_Bool bKeepHighlightFlags )
{
    if( pEntry == pCurHighlightFrame )
        return;

    if( !bKeepHighlightFlags )
        bHighlightFramePressed = sal_False;

    HideEntryHighlightFrame();
    pCurHighlightFrame = pEntry;
    if( pEntry )
    {
        Rectangle aBmpRect( CalcFocusRect(pEntry) );
        DrawHighlightFrame( pView, aBmpRect, sal_False );
    }
}

void SvxIconChoiceCtrl_Impl::HideEntryHighlightFrame()
{
    if( !pCurHighlightFrame )
        return;

    SvxIconChoiceCtrlEntry* pEntry = pCurHighlightFrame;
    pCurHighlightFrame = 0;
    Rectangle aBmpRect( CalcFocusRect(pEntry) );
    DrawHighlightFrame( pView, aBmpRect, sal_True );
}

void SvxIconChoiceCtrl_Impl::CallSelectHandler( SvxIconChoiceCtrlEntry* )
{
    // When single-click mode is active, the selection handler should be called
    // synchronously, as the selection is automatically taken away once the
    // mouse cursor doesn't touch the object any more. Else, we might run into
    // missing calls to Select if the object is selected from a mouse movement,
    // because when starting the timer, the mouse cursor might have already left
    // the object.
    // In special cases (=>SfxFileDialog!), synchronous calls can be forced via
    // WB_NOASYNCSELECTHDL.
    if( nWinBits & (WB_NOASYNCSELECTHDL | WB_HIGHLIGHTFRAME) )
    {
        pHdlEntry = 0;
        pView->ClickIcon();
        //pView->Select();
    }
    else
        aCallSelectHdlTimer.Start();
}

IMPL_LINK_NOARG(SvxIconChoiceCtrl_Impl, CallSelectHdlHdl)
{
    pHdlEntry = 0;
    pView->ClickIcon();
    //pView->Select();
    return 0;
}

void SvxIconChoiceCtrl_Impl::SetOrigin( const Point& rPos, sal_Bool bDoNotUpdateWallpaper )
{
    MapMode aMapMode( pView->GetMapMode() );
    aMapMode.SetOrigin( rPos );
    pView->SetMapMode( aMapMode );
    if( !bDoNotUpdateWallpaper )
    {
        sal_Bool bScrollable = pView->GetBackground().IsScrollable();
        if( pView->HasBackground() && !bScrollable )
        {
            Rectangle aRect( GetOutputRect());
            Wallpaper aPaper( pView->GetBackground() );
            aPaper.SetRect( aRect );
            pView->SetBackground( aPaper );
        }
    }
}

void SvxIconChoiceCtrl_Impl::CallEventListeners( sal_uLong nEvent, void* pData )
{
    pView->CallImplEventListeners( nEvent, pData );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
