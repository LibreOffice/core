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

#include <vcl/svapp.hxx>
#include <vcl/salnativewidgets.hxx>
#include <vcl/help.hxx>

#include <stack>

#define _SVTREEBX_CXX
#include <svtools/svtreebx.hxx>
#include <svtools/svlbox.hxx>
#include <svimpbox.hxx>
#include <rtl/instance.hxx>
#include <svtools/svtresid.hxx>
#include <tools/wintypes.hxx>
#include <svtools/svtools.hrc>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>

#define NODE_BMP_TABDIST_NOTVALID   -2000000
#define FIRST_ENTRY_TAB             1

// #i27063# (pl), #i32300# (pb) never access VCL after DeInitVCL - also no destructors
Image*  SvImpLBox::s_pDefCollapsed      = NULL;
Image*  SvImpLBox::s_pDefExpanded       = NULL;
sal_Int32 SvImpLBox::s_nImageRefCount   = 0;

SvImpLBox::SvImpLBox( SvTreeListBox* pLBView, SvLBoxTreeList* pLBTree, WinBits nWinStyle) :

    aVerSBar( pLBView, WB_DRAG | WB_VSCROLL ),
    aHorSBar( pLBView, WB_DRAG | WB_HSCROLL ),
    aScrBarBox( pLBView ),
    aOutputSize( 0, 0 ),
    aSelEng( pLBView, (FunctionSet*)0 ),
    aFctSet( this, &aSelEng, pLBView ),
    nExtendedWinBits( 0 ),
    bAreChildrenTransient( sal_True ),
    m_pStringSorter(NULL)
{
    osl_atomic_increment(&s_nImageRefCount);
    pView = pLBView;
    pTree = pLBTree;
    aSelEng.SetFunctionSet( (FunctionSet*)&aFctSet );
    aSelEng.ExpandSelectionOnMouseMove( sal_False );
    SetStyle( nWinStyle );
    SetSelectionMode( SINGLE_SELECTION );
    SetDragDropMode( 0 );

    aVerSBar.SetScrollHdl( LINK( this, SvImpLBox, ScrollUpDownHdl ) );
    aHorSBar.SetScrollHdl( LINK( this, SvImpLBox, ScrollLeftRightHdl ) );
    aHorSBar.SetEndScrollHdl( LINK( this, SvImpLBox, EndScrollHdl ) );
    aVerSBar.SetEndScrollHdl( LINK( this, SvImpLBox, EndScrollHdl ) );
    aVerSBar.SetRange( Range(0,0) );
    aVerSBar.Hide();
    aHorSBar.SetRange( Range(0,0) );
    aHorSBar.SetPageSize( 24 ); // pixels
    aHorSBar.SetLineSize( 8 ); // pixels

    nHorSBarHeight = (short)aHorSBar.GetSizePixel().Height();
    nVerSBarWidth = (short)aVerSBar.GetSizePixel().Width();

    pStartEntry = 0;
    pCursor             = 0;
    pAnchor             = 0;
    nVisibleCount       = 0;    // number of rows of data in control
    nNodeBmpTabDistance = NODE_BMP_TABDIST_NOTVALID;
    nYoffsNodeBmp       = 0;
    nNodeBmpWidth       = 0;

    bAsyncBeginDrag     = sal_False;
    aAsyncBeginDragTimer.SetTimeout( 0 );
    aAsyncBeginDragTimer.SetTimeoutHdl( LINK(this,SvImpLBox,BeginDragHdl));
    // button animation in listbox
    pActiveButton = 0;
    pActiveEntry = 0;
    pActiveTab = 0;

    nFlags = 0;
    nCurTabPos = FIRST_ENTRY_TAB;

    aEditTimer.SetTimeout( 800 );
    aEditTimer.SetTimeoutHdl( LINK(this,SvImpLBox,EditTimerCall) );

    nMostRight = -1;
    pMostRightEntry = 0;
    nCurUserEvent = 0xffffffff;

    bUpdateMode = sal_True;
    bInVScrollHdl = sal_False;
    nFlags |= F_FILLING;

    bSubLstOpRet = bSubLstOpLR = bContextMenuHandling = bIsCellFocusEnabled = sal_False;
}

SvImpLBox::~SvImpLBox()
{
    aEditTimer.Stop();
    StopUserEvent();

    delete m_pStringSorter;
    if ( osl_atomic_decrement(&s_nImageRefCount) == 0 )
    {
        DELETEZ(s_pDefCollapsed);
        DELETEZ(s_pDefExpanded);
    }
}

void SvImpLBox::UpdateStringSorter()
{
    const ::com::sun::star::lang::Locale& rNewLocale = Application::GetSettings().GetLocale();

    if( m_pStringSorter )
    {
        // different Locale from the older one, drop it and force recreate
        const ::com::sun::star::lang::Locale &aLocale = m_pStringSorter->getLocale();
        if( aLocale.Language != rNewLocale.Language ||
            aLocale.Country != rNewLocale.Country ||
            aLocale.Variant != rNewLocale.Variant )
        {
            delete m_pStringSorter;
            m_pStringSorter = NULL;
        }
    }

    if( !m_pStringSorter )
    {
        m_pStringSorter = new comphelper::string::NaturalStringSorter(
                              ::comphelper::getProcessComponentContext(),
                              rNewLocale);
    }
}

// #97680# ----------------------
short SvImpLBox::UpdateContextBmpWidthVector( SvLBoxEntry* pEntry, short nWidth )
{
    DBG_ASSERT( pView->pModel, "View and Model aren't valid!" );

    sal_uInt16 nDepth = pView->pModel->GetDepth( pEntry );
    // initialize vector if necessary
    std::vector< short >::size_type nSize = aContextBmpWidthVector.size();
    while ( nDepth > nSize )
    {
        aContextBmpWidthVector.resize( nSize + 1 );
        aContextBmpWidthVector.at( nSize ) = nWidth;
        ++nSize;
    }
    if( aContextBmpWidthVector.size() == nDepth )
    {
        aContextBmpWidthVector.resize( nDepth + 1 );
        aContextBmpWidthVector.at( nDepth ) = 0;
    }
    short nContextBmpWidth = aContextBmpWidthVector[ nDepth ];
    if( nContextBmpWidth < nWidth )
    {
        aContextBmpWidthVector.at( nDepth ) = nWidth;
        return nWidth;
    }
    else
        return nContextBmpWidth;
}

void SvImpLBox::UpdateContextBmpWidthVectorFromMovedEntry( SvLBoxEntry* pEntry )
{
    DBG_ASSERT( pEntry, "Moved Entry is invalid!" );

    SvLBoxContextBmp* pBmpItem = static_cast< SvLBoxContextBmp* >( pEntry->GetFirstItem( SV_ITEM_ID_LBOXCONTEXTBMP ) );
    short nExpWidth = (short)pBmpItem->GetBitmap1().GetSizePixel().Width();
    short nColWidth = (short)pBmpItem->GetBitmap2().GetSizePixel().Width();
    short nMax = Max(nExpWidth, nColWidth);
    UpdateContextBmpWidthVector( pEntry, nMax );

    if( pEntry->HasChildren() ) // recursive call, whether expanded or not
    {
        SvLBoxEntry* pChild = pView->FirstChild( pEntry );
        DBG_ASSERT( pChild, "The first child is invalid!" );
        do
        {
            UpdateContextBmpWidthVectorFromMovedEntry( pChild );
            pChild = pView->Next( pChild );
        } while ( pChild );
    }
}

void SvImpLBox::UpdateContextBmpWidthMax( SvLBoxEntry* pEntry )
{
    sal_uInt16 nDepth = pView->pModel->GetDepth( pEntry );
    if( aContextBmpWidthVector.size() < 1 )
        return;
    short nWidth = aContextBmpWidthVector[ nDepth ];
    if( nWidth != pView->nContextBmpWidthMax ) {
        pView->nContextBmpWidthMax = nWidth;
        nFlags |= F_IGNORE_CHANGED_TABS;
        pView->SetTabs();
        nFlags &= ~F_IGNORE_CHANGED_TABS;
    }
}

void SvImpLBox::CalcCellFocusRect( SvLBoxEntry* pEntry, Rectangle& rRect )
{
    if ( pEntry && bIsCellFocusEnabled )
    {
        if ( nCurTabPos > FIRST_ENTRY_TAB )
        {
            SvLBoxItem* pItem = pCursor->GetItem( nCurTabPos );
            rRect.Left() = pView->GetTab( pCursor, pItem )->GetPos();
        }
        if ( pCursor->ItemCount() > ( nCurTabPos + 1 ) )
        {
            SvLBoxItem* pNextItem = pCursor->GetItem( nCurTabPos + 1 );
            long nRight = pView->GetTab( pCursor, pNextItem )->GetPos() - 1;
            if ( nRight < rRect.Right() )
                rRect.Right() = nRight;
        }
    }
}

void SvImpLBox::SetStyle( WinBits i_nWinStyle )
{
    m_nStyle = i_nWinStyle;
    if ( ( m_nStyle & WB_SIMPLEMODE) && ( aSelEng.GetSelectionMode() == MULTIPLE_SELECTION ) )
        aSelEng.AddAlways( sal_True );
}

void SvImpLBox::SetExtendedWindowBits( ExtendedWinBits _nBits )
{
    nExtendedWinBits = _nBits;
}

// don't touch the model any more
void SvImpLBox::Clear()
{
    StopUserEvent();
    pStartEntry = 0;
    pAnchor = 0;

    pActiveButton = 0;
    pActiveEntry = 0;
    pActiveTab = 0;

    nMostRight = -1;
    pMostRightEntry = 0;

    // don't touch the cursor any more
    if( pCursor )
    {
        if( pView->HasFocus() )
            pView->HideFocus();
        pCursor = 0;
    }
    aVerSBar.Hide();
    aVerSBar.SetThumbPos( 0 );
    Range aRange( 0, 0 );
    aVerSBar.SetRange( aRange );
    aOutputSize = pView->Control::GetOutputSizePixel();
    nFlags &= ~(F_VER_SBARSIZE_WITH_HBAR | F_HOR_SBARSIZE_WITH_VBAR );
    aHorSBar.Hide();
    aHorSBar.SetThumbPos( 0 );
    MapMode aMapMode( pView->GetMapMode());
    aMapMode.SetOrigin( Point(0,0) );
    pView->Control::SetMapMode( aMapMode );
    aHorSBar.SetRange( aRange );
    aHorSBar.SetSizePixel(Size(aOutputSize.Width(),nHorSBarHeight));
    pView->SetClipRegion();
    if( GetUpdateMode() )
        pView->Invalidate( GetVisibleArea() );
    nFlags |= F_FILLING;
    if( !aHorSBar.IsVisible() && !aVerSBar.IsVisible() )
        aScrBarBox.Hide();

    aContextBmpWidthVector.clear();
}

// *********************************************************************
// Paint, navigate, scroll
// *********************************************************************

IMPL_LINK_NOARG_INLINE_START(SvImpLBox, EndScrollHdl)
{
    if( nFlags & F_ENDSCROLL_SET_VIS_SIZE )
    {
        aVerSBar.SetVisibleSize( nNextVerVisSize );
        nFlags &= ~F_ENDSCROLL_SET_VIS_SIZE;
    }
    EndScroll();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SvImpLBox, EndScrollHdl)


// handler for vertical scrollbar

IMPL_LINK( SvImpLBox, ScrollUpDownHdl, ScrollBar *, pScrollBar )
{
    DBG_ASSERT(!bInVScrollHdl,"Scroll handler out-paces itself!");
    long nDelta = pScrollBar->GetDelta();
    if( !nDelta )
        return 0;

    nFlags &= (~F_FILLING);

    bInVScrollHdl = sal_True;

    if( pView->IsEditingActive() )
    {
        pView->EndEditing( sal_True ); // Cancel
        pView->Update();
    }
    BeginScroll();

    if( nDelta > 0 )
    {
        if( nDelta == 1 )
            CursorDown();
        else
            PageDown( (sal_uInt16) nDelta );
    }
    else
    {
        nDelta *= (-1);
        if( nDelta == 1 )
            CursorUp();
        else
            PageUp( (sal_uInt16) nDelta );
    }
    bInVScrollHdl = sal_False;
    return 0;
}


void SvImpLBox::CursorDown()
{
    SvLBoxEntry* pNextFirstToDraw = (SvLBoxEntry*)(pView->NextVisible( pStartEntry));
    if( pNextFirstToDraw )
    {
        nFlags &= (~F_FILLING);
        pView->NotifyScrolling( -1 );
        ShowCursor( sal_False );
        pView->Update();
        pStartEntry = pNextFirstToDraw;
        Rectangle aArea( GetVisibleArea() );
        pView->Scroll( 0, -(pView->GetEntryHeight()), aArea, SCROLL_NOCHILDREN );
        pView->Update();
        ShowCursor( sal_True );
        pView->NotifyScrolled();
    }
}

void SvImpLBox::CursorUp()
{
    SvLBoxEntry* pPrevFirstToDraw = (SvLBoxEntry*)(pView->PrevVisible( pStartEntry));
    if( pPrevFirstToDraw )
    {
        nFlags &= (~F_FILLING);
        long nEntryHeight = pView->GetEntryHeight();
        pView->NotifyScrolling( 1 );
        ShowCursor( sal_False );
        pView->Update();
        pStartEntry = pPrevFirstToDraw;
        Rectangle aArea( GetVisibleArea() );
        aArea.Bottom() -= nEntryHeight;
        pView->Scroll( 0, nEntryHeight, aArea, SCROLL_NOCHILDREN );
        pView->Update();
        ShowCursor( sal_True );
        pView->NotifyScrolled();
    }
}

void SvImpLBox::PageDown( sal_uInt16 nDelta )
{
    sal_uInt16 nRealDelta = nDelta;

    if( !nDelta )
        return;

    SvLBoxEntry* pNext;
    pNext = (SvLBoxEntry*)(pView->NextVisible( pStartEntry, nRealDelta ));
    if( (sal_uLong)pNext == (sal_uLong)pStartEntry )
        return;

    ShowCursor( sal_False );

    nFlags &= (~F_FILLING);
    pView->Update();
    pStartEntry = pNext;

    if( nRealDelta >= nVisibleCount )
    {
        pView->Invalidate( GetVisibleArea() );
        pView->Update();
    }
    else
    {
        long nScroll = nRealDelta * (-1);
        pView->NotifyScrolling( nScroll );
        Rectangle aArea( GetVisibleArea() );
        nScroll = pView->GetEntryHeight()*nRealDelta;
        nScroll = -nScroll;
        pView->Update();
        pView->Scroll( 0, nScroll, aArea, SCROLL_NOCHILDREN );
        pView->Update();
        pView->NotifyScrolled();
    }

    ShowCursor( sal_True );
}

void SvImpLBox::PageUp( sal_uInt16 nDelta )
{
    sal_uInt16 nRealDelta = nDelta;
    if( !nDelta )
        return;

    SvLBoxEntry* pPrev = (SvLBoxEntry*)(pView->PrevVisible( pStartEntry, nRealDelta ));
    if( (sal_uLong)pPrev == (sal_uLong)pStartEntry )
        return;

    nFlags &= (~F_FILLING);
    ShowCursor( sal_False );

    pView->Update();
    pStartEntry = pPrev;
    if( nRealDelta >= nVisibleCount )
    {
        pView->Invalidate( GetVisibleArea() );
        pView->Update();
    }
    else
    {
        long nEntryHeight = pView->GetEntryHeight();
        pView->NotifyScrolling( (long)nRealDelta );
        Rectangle aArea( GetVisibleArea() );
        pView->Update();
        pView->Scroll( 0, nEntryHeight*nRealDelta, aArea, SCROLL_NOCHILDREN );
        pView->Update();
        pView->NotifyScrolled();
    }

    ShowCursor( sal_True );
}

void SvImpLBox::KeyUp( sal_Bool bPageUp, sal_Bool bNotifyScroll )
{
    if( !aVerSBar.IsVisible() )
        return;

    long nDelta;
    if( bPageUp )
        nDelta = aVerSBar.GetPageSize();
    else
        nDelta = 1;

    long nThumbPos = aVerSBar.GetThumbPos();

    if( nThumbPos < nDelta )
        nDelta = nThumbPos;

    if( nDelta <= 0 )
        return;

    nFlags &= (~F_FILLING);
    if( bNotifyScroll )
        BeginScroll();

    aVerSBar.SetThumbPos( nThumbPos - nDelta );
    if( bPageUp )
        PageUp( (short)nDelta );
    else
        CursorUp();

    if( bNotifyScroll )
        EndScroll();
}


void SvImpLBox::KeyDown( sal_Bool bPageDown, sal_Bool bNotifyScroll )
{
    if( !aVerSBar.IsVisible() )
        return;

    long nDelta;
    if( bPageDown )
        nDelta = aVerSBar.GetPageSize();
    else
        nDelta = 1;

    long nThumbPos = aVerSBar.GetThumbPos();
    long nVisibleSize = aVerSBar.GetVisibleSize();
    long nRange = aVerSBar.GetRange().Len();

    long nTmp = nThumbPos+nVisibleSize;
    while( (nDelta > 0) && (nTmp+nDelta) >= nRange )
        nDelta--;

    if( nDelta <= 0 )
        return;

    nFlags &= (~F_FILLING);
    if( bNotifyScroll )
        BeginScroll();

    aVerSBar.SetThumbPos( nThumbPos+nDelta );
    if( bPageDown )
        PageDown( (short)nDelta );
    else
        CursorDown();

    if( bNotifyScroll )
        EndScroll();
}



void SvImpLBox::InvalidateEntriesFrom( long nY ) const
{
    if( !(nFlags & F_IN_PAINT ))
    {
        Rectangle aRect( GetVisibleArea() );
        aRect.Top() = nY;
        pView->Invalidate( aRect );
    }
}

void SvImpLBox::InvalidateEntry( long nY ) const
{
    if( !(nFlags & F_IN_PAINT ))
    {
        Rectangle aRect( GetVisibleArea() );
        long nMaxBottom = aRect.Bottom();
        aRect.Top() = nY;
        aRect.Bottom() = nY; aRect.Bottom() += pView->GetEntryHeight();
        if( aRect.Top() > nMaxBottom )
            return;
        if( aRect.Bottom() > nMaxBottom )
            aRect.Bottom() = nMaxBottom;
        pView->Invalidate( aRect );
    }
}

void SvImpLBox::InvalidateEntry( SvLBoxEntry* pEntry )
{
    if( GetUpdateMode() )
    {
        long nPrev = nMostRight;
        SetMostRight( pEntry );
        if( nPrev < nMostRight )
            ShowVerSBar();
    }
    if( !(nFlags & F_IN_PAINT ))
    {
        sal_Bool bHasFocusRect = sal_False;
        if( pEntry==pCursor && pView->HasFocus() )
        {
            bHasFocusRect = sal_True;
            ShowCursor( sal_False );
        }
        InvalidateEntry( GetEntryLine( pEntry ) );
        if( bHasFocusRect )
            ShowCursor( sal_True );
    }
}


void SvImpLBox::RecalcFocusRect()
{
    if( pView->HasFocus() && pCursor )
    {
        pView->HideFocus();
        long nY = GetEntryLine( pCursor );
        Rectangle aRect = pView->GetFocusRect( pCursor, nY );
        CalcCellFocusRect( pCursor, aRect );
        Region aOldClip( pView->GetClipRegion());
        Region aClipRegion( GetClipRegionRect() );
        pView->SetClipRegion( aClipRegion );
        pView->ShowFocus( aRect );
        pView->SetClipRegion( aOldClip );
    }
}

//
//  Sets cursor. When using SingleSelection, the selection is adjusted.
//

void SvImpLBox::SetCursor( SvLBoxEntry* pEntry, sal_Bool bForceNoSelect )
{
    SvViewDataEntry* pViewDataNewCur = 0;
    if( pEntry )
        pViewDataNewCur= pView->GetViewDataEntry(pEntry);
    if( pEntry &&
        pEntry == pCursor &&
        pViewDataNewCur->HasFocus() &&
        pViewDataNewCur->IsSelected())
    {
        return;
    }

    // if this cursor is not selectable, find first visible that is and use it
    while( pEntry && pViewDataNewCur && !pViewDataNewCur->IsSelectable() )
    {
        pEntry = (SvLBoxEntry*)(pView->NextVisible( pEntry ));
        pViewDataNewCur = pEntry ? pView->GetViewDataEntry(pEntry) : 0;
    }

    SvLBoxEntry* pOldCursor = pCursor;
    if( pCursor && pEntry != pCursor )
    {
        pView->SetEntryFocus( pCursor, sal_False );
        if( bSimpleTravel )
            pView->Select( pCursor, sal_False );
        pView->HideFocus();
    }
    pCursor = pEntry;
    if( pCursor )
    {
        pViewDataNewCur->SetFocus( sal_True );
        if(!bForceNoSelect && bSimpleTravel && !(nFlags & F_DESEL_ALL) && GetUpdateMode())
        {
            pView->Select( pCursor, sal_True );
        }
        // multiple selection: select in cursor move if we're not in
        // Add mode (Ctrl-F8)
        else if( GetUpdateMode() &&
                 pView->GetSelectionMode() == MULTIPLE_SELECTION &&
                 !(nFlags & F_DESEL_ALL) && !aSelEng.IsAddMode() &&
                 !bForceNoSelect )
        {
            pView->Select( pCursor, sal_True );
        }
        else
        {
            ShowCursor( sal_True );
        }

        if( pAnchor )
        {
            DBG_ASSERT(aSelEng.GetSelectionMode() != SINGLE_SELECTION,"Mode?");
            SetAnchorSelection( pOldCursor, pCursor );
        }
    }
    nFlags &= (~F_DESEL_ALL);

    pView->OnCurrentEntryChanged();
}

void SvImpLBox::ShowCursor( sal_Bool bShow )
{
    if( !bShow || !pCursor || !pView->HasFocus() )
    {
        Region aOldClip( pView->GetClipRegion());
        Region aClipRegion( GetClipRegionRect() );
        pView->SetClipRegion( aClipRegion );
        pView->HideFocus();
        pView->SetClipRegion( aOldClip );
    }
    else
    {
        long nY = GetEntryLine( pCursor );
        Rectangle aRect = pView->GetFocusRect( pCursor, nY );
        CalcCellFocusRect( pCursor, aRect );
        Region aOldClip( pView->GetClipRegion());
        Region aClipRegion( GetClipRegionRect() );
        pView->SetClipRegion( aClipRegion );
        pView->ShowFocus( aRect );
        pView->SetClipRegion( aOldClip );
    }
}



void SvImpLBox::UpdateAll( sal_Bool bInvalidateCompleteView,
    sal_Bool bUpdateVerScrollBar )
{
    if( bUpdateVerScrollBar )
        FindMostRight(0);
    aVerSBar.SetRange( Range(0, pView->GetVisibleCount()-1 ) );
    SyncVerThumb();
    FillView();
    ShowVerSBar();
    if( bSimpleTravel && pCursor && pView->HasFocus() )
        pView->Select( pCursor, sal_True );
    ShowCursor( sal_True );
    if( bInvalidateCompleteView )
        pView->Invalidate();
    else
        pView->Invalidate( GetVisibleArea() );
}

IMPL_LINK_INLINE_START( SvImpLBox, ScrollLeftRightHdl, ScrollBar *, pScrollBar )
{
    long nDelta = pScrollBar->GetDelta();
    if( nDelta )
    {
        if( pView->IsEditingActive() )
        {
            pView->EndEditing( sal_True ); // Cancel
            pView->Update();
        }
        pView->nFocusWidth = -1;
        KeyLeftRight( nDelta );
    }
    return 0;
}
IMPL_LINK_INLINE_END( SvImpLBox, ScrollLeftRightHdl, ScrollBar *, pScrollBar )

void SvImpLBox::KeyLeftRight( long nDelta )
{
    if( !(nFlags & F_IN_RESIZE) )
        pView->Update();
    BeginScroll();
    nFlags &= (~F_FILLING);
    pView->NotifyScrolling( 0 ); // 0 == horizontal scrolling
    ShowCursor( sal_False );

    // neuen Origin berechnen
    long nPos = aHorSBar.GetThumbPos();
    Point aOrigin( -nPos, 0 );

    MapMode aMapMode( pView->GetMapMode() );
    aMapMode.SetOrigin( aOrigin );
    pView->SetMapMode( aMapMode );

    if( !(nFlags & F_IN_RESIZE) )
    {
        Rectangle aRect( GetVisibleArea() );
        pView->Scroll( -nDelta, 0, aRect, SCROLL_NOCHILDREN );
    }
    else
        pView->Invalidate();
    RecalcFocusRect();
    ShowCursor( sal_True );
    pView->NotifyScrolled();
}


// returns the last entry if position is just past the last entry
SvLBoxEntry* SvImpLBox::GetClickedEntry( const Point& rPoint ) const
{
    DBG_ASSERT( pView->GetModel(), "SvImpLBox::GetClickedEntry: how can this ever happen? Please tell me (frank.schoenheit@sun.com) how to reproduce!" );
    if ( !pView->GetModel() )
        // this is quite impossible. Nevertheless, stack traces from the crash reporter
        // suggest it isn't. Okay, make it safe, and wait for somebody to reproduce it
        // reliably :-\ ....
        // #122359# / 2005-05-23 / frank.schoenheit@sun.com
        return NULL;
    if( pView->GetEntryCount() == 0 || !pStartEntry || !pView->GetEntryHeight())
        return 0;

    sal_uInt16 nClickedEntry = (sal_uInt16)(rPoint.Y() / pView->GetEntryHeight() );
    sal_uInt16 nTemp = nClickedEntry;
    SvLBoxEntry* pEntry = (SvLBoxEntry*)(pView->NextVisible( pStartEntry, nTemp ));
    return pEntry;
}

//
//  checks if the entry was hit "the right way"
//  (Focusrect+ ContextBitmap bei TreeListBox)
//
sal_Bool SvImpLBox::EntryReallyHit(SvLBoxEntry* pEntry,const Point& rPosPixel,long nLine)
{
    sal_Bool bRet;
    // we are not too exact when it comes to "special" entries
    // (with CheckButtons etc.)
    if( pEntry->ItemCount() >= 3 )
        return sal_True;

    Rectangle aRect( pView->GetFocusRect( pEntry, nLine ));
    aRect.Right() = GetOutputSize().Width() - pView->GetMapMode().GetOrigin().X();
    if( pView->IsA() == SV_LISTBOX_ID_TREEBOX )
    {
        SvLBoxContextBmp* pBmp = (SvLBoxContextBmp*)(pEntry->GetFirstItem(SV_ITEM_ID_LBOXCONTEXTBMP));
        aRect.Left() -= pBmp->GetSize(pView,pEntry).Width();
        aRect.Left() -= 4; // a little tolerance
    }
    Point aPos( rPosPixel );
    aPos -= pView->GetMapMode().GetOrigin();
    if( aRect.IsInside( aPos ) )
        bRet = sal_True;
    else
        bRet = sal_False;
    return bRet;
}


// returns 0 if position is just past the last entry
SvLBoxEntry* SvImpLBox::GetEntry( const Point& rPoint ) const
{
    if( (pView->GetEntryCount() == 0) || !pStartEntry ||
        (rPoint.Y() > aOutputSize.Height())
        || !pView->GetEntryHeight())
        return 0;

    sal_uInt16 nClickedEntry = (sal_uInt16)(rPoint.Y() / pView->GetEntryHeight() );
    sal_uInt16 nTemp = nClickedEntry;
    SvLBoxEntry* pEntry = (SvLBoxEntry*)(pView->NextVisible( pStartEntry, nTemp ));
    if( nTemp != nClickedEntry )
        pEntry = 0;
    return pEntry;
}


SvLBoxEntry* SvImpLBox::MakePointVisible(const Point& rPoint,sal_Bool bNotifyScroll)
{
    if( !pCursor )
        return 0;
    long nY = rPoint.Y();
    SvLBoxEntry* pEntry = 0;
    long nMax = aOutputSize.Height();
    if( nY < 0 || nY >= nMax ) // aOutputSize.Height() )
    {
        if( nY < 0 )
            pEntry = (SvLBoxEntry*)(pView->PrevVisible( pCursor ));
        else
            pEntry = (SvLBoxEntry*)(pView->NextVisible( pCursor ));

        if( pEntry && pEntry != pCursor )
            pView->SetEntryFocus( pCursor, sal_False );

        if( nY < 0 )
            KeyUp( sal_False, bNotifyScroll );
        else
            KeyDown( sal_False, bNotifyScroll );
    }
    else
    {
        pEntry = GetClickedEntry( rPoint );
        if( !pEntry )
        {
            sal_uInt16 nSteps = 0xFFFF;
            // TODO: LastVisible is not yet implemented!
            pEntry = (SvLBoxEntry*)(pView->NextVisible( pStartEntry, nSteps ));
        }
        if( pEntry )
        {
            if( pEntry != pCursor &&
                 aSelEng.GetSelectionMode() == SINGLE_SELECTION
            )
                pView->Select( pCursor, sal_False );
        }
    }
    return pEntry;
}

Rectangle SvImpLBox::GetClipRegionRect() const
{
    Point aOrigin( pView->GetMapMode().GetOrigin() );
    aOrigin.X() *= -1; // conversion document coordinates
    Rectangle aClipRect( aOrigin, aOutputSize );
    aClipRect.Bottom()++;
    return aClipRect;
}


void SvImpLBox::Paint( const Rectangle& rRect )
{
    if( !pView->GetVisibleCount() )
        return;

    nFlags |= F_IN_PAINT;

    if( nFlags & F_FILLING )
    {
        SvLBoxEntry* pFirst = pView->First();
        if( pFirst != pStartEntry )
        {
            ShowCursor( sal_False );
            pStartEntry = pView->First();
            aVerSBar.SetThumbPos( 0 );
            StopUserEvent();
            ShowCursor( sal_True );
            nCurUserEvent = Application::PostUserEvent(LINK(this,SvImpLBox,MyUserEvent),(void*)1);
            return;
        }
    }

    if( !pStartEntry )
    {
        pStartEntry = pView->First();
    }

    if( nNodeBmpTabDistance == NODE_BMP_TABDIST_NOTVALID )
        SetNodeBmpTabDistance();

    long nRectHeight = rRect.GetHeight();
    long nEntryHeight = pView->GetEntryHeight();

    // calculate area for the entries we want to draw
    sal_uInt16 nStartLine = (sal_uInt16)( rRect.Top() / nEntryHeight );
    sal_uInt16 nCount = (sal_uInt16)( nRectHeight / nEntryHeight );
        nCount += 2; // don't miss a row

    long nY = nStartLine * nEntryHeight;
    SvLBoxEntry* pEntry = pStartEntry;
    while( nStartLine && pEntry )
    {
        pEntry = (SvLBoxEntry*)(pView->NextVisible( pEntry ));
        nStartLine--;
    }

    Region aClipRegion( GetClipRegionRect() );

    // first draw the lines, then clip them!
    pView->SetClipRegion();
    if( m_nStyle & ( WB_HASLINES | WB_HASLINESATROOT ) )
        DrawNet();

    pView->SetClipRegion( aClipRegion );

    for( sal_uInt16 n=0; n< nCount && pEntry; n++ )
    {
        /*long nMaxRight=*/
        pView->PaintEntry1( pEntry, nY, 0xffff, sal_True );
        nY += nEntryHeight;
        pEntry = (SvLBoxEntry*)(pView->NextVisible( pEntry ));
    }

    if ( !pCursor && ( ( nExtendedWinBits & EWB_NO_AUTO_CURENTRY ) == 0 ) )
    {
        // do not select if multiselection or explicit set
        sal_Bool bNotSelect = ( aSelEng.GetSelectionMode() == MULTIPLE_SELECTION )
                || ( ( m_nStyle & WB_NOINITIALSELECTION ) == WB_NOINITIALSELECTION );
        SetCursor( pStartEntry, bNotSelect );
    }

    nFlags &= (~F_DESEL_ALL);
    pView->SetClipRegion();
    Rectangle aRect;
    if( !(nFlags & F_PAINTED) )
    {
        nFlags |= F_PAINTED;
        RepaintScrollBars();
    }
    nFlags &= (~F_IN_PAINT);
}

void SvImpLBox::MakeVisible( SvLBoxEntry* pEntry, sal_Bool bMoveToTop )
{
    if( !pEntry )
        return;

    sal_Bool bInView = IsEntryInView( pEntry );

    if( bInView && (!bMoveToTop || pStartEntry == pEntry) )
        return;  // is already visible

    if( pStartEntry || (m_nStyle & WB_FORCE_MAKEVISIBLE) )
        nFlags &= (~F_FILLING);
    if( !bInView )
    {
        if( !pView->IsEntryVisible(pEntry) )  // Parent(s) collapsed?
        {
            SvLBoxEntry* pParent = pView->GetParent( pEntry );
            while( pParent )
            {
                if( !pView->IsExpanded( pParent ) )
                {
                    #ifdef DBG_UTIL
                    sal_Bool bRet =
                    #endif
                        pView->Expand( pParent );
                    DBG_ASSERT(bRet,"Not expanded!");
                }
                pParent = pView->GetParent( pParent );
            }
            // do the parent's children fit into the view or do we have to scroll?
            if( IsEntryInView( pEntry ) && !bMoveToTop )
                return;  // no need to scroll
        }
    }

    pStartEntry = pEntry;
    ShowCursor( sal_False );
    FillView();
    aVerSBar.SetThumbPos( (long)(pView->GetVisiblePos( pStartEntry )) );
    ShowCursor( sal_True );
    pView->Invalidate();
}

void SvImpLBox::ScrollToAbsPos( long nPos )
{
    if( pView->GetVisibleCount() == 0 )
        return;
    long nLastEntryPos = pView->GetAbsPos( pView->Last() );

    if( nPos < 0 )
        nPos = 0;
    else if( nPos > nLastEntryPos )
        nPos = nLastEntryPos;

    SvLBoxEntry* pEntry = (SvLBoxEntry*)pView->GetEntryAtAbsPos( nPos );
    if( !pEntry || pEntry == pStartEntry )
        return;

    if( pStartEntry || (m_nStyle & WB_FORCE_MAKEVISIBLE) )
        nFlags &= (~F_FILLING);

    if( pView->IsEntryVisible(pEntry) )
    {
        pStartEntry = pEntry;
        ShowCursor( sal_False );
        aVerSBar.SetThumbPos( nPos );
        ShowCursor( sal_True );
        if (GetUpdateMode())
            pView->Invalidate();
    }
}

void SvImpLBox::DrawNet()
{
    if( pView->GetVisibleCount() < 2 && !pStartEntry->HasChildrenOnDemand() &&
        !pStartEntry->HasChildren() )
        return;

    // for platforms that don't have nets, DrawNativeControl does nothing and returns true
    // so that SvImpLBox::DrawNet() doesn't draw anything either
     if(pView->IsNativeControlSupported( CTRL_LISTNET, PART_ENTIRE_CONTROL)) {
        ImplControlValue    aControlValue;
        Point  aTemp(0,0);   // temporary needed for g++ 3.3.5
        Rectangle aCtrlRegion( aTemp, Size( 0, 0 ) );
        ControlState        nState = CTRL_STATE_ENABLED;
        if( pView->DrawNativeControl( CTRL_LISTNET, PART_ENTIRE_CONTROL,
                                      aCtrlRegion, nState, aControlValue, rtl::OUString() ) )
        {
            return;
        }

    }

    long nEntryHeight = pView->GetEntryHeight();
    long nEntryHeightDIV2 = nEntryHeight / 2;
    if( nEntryHeightDIV2 && !(nEntryHeight & 0x0001))
        nEntryHeightDIV2--;

    SvLBoxEntry* pChild;
    SvLBoxEntry* pEntry = pStartEntry;

    SvLBoxTab* pFirstDynamicTab = pView->GetFirstDynamicTab();
    while( pTree->GetDepth( pEntry ) > 0 )
        pEntry = pView->GetParent( pEntry );
    sal_uInt16 nOffs = (sal_uInt16)(pView->GetVisiblePos( pStartEntry ) -
                            pView->GetVisiblePos( pEntry ));
    long nY = 0;
    nY -= ( nOffs * nEntryHeight );

    DBG_ASSERT(pFirstDynamicTab,"No Tree!");

    Color aOldLineColor = pView->GetLineColor();
    const StyleSettings& rStyleSettings = pView->GetSettings().GetStyleSettings();
    Color aCol= rStyleSettings.GetFaceColor();

    if( aCol.IsRGBEqual( pView->GetBackground().GetColor()) )
        aCol = rStyleSettings.GetShadowColor();
    pView->SetLineColor( aCol );
    Point aPos1, aPos2;
    sal_uInt16 nDistance;
    sal_uLong nMax = nVisibleCount + nOffs + 1;

    const Image& rExpandedNodeBitmap = GetExpandedNodeBmp();

    for( sal_uLong n=0; n< nMax && pEntry; n++ )
    {
        if( pView->IsExpanded(pEntry) )
        {
            aPos1.X() = pView->GetTabPos(pEntry, pFirstDynamicTab);
            // if it is not a context bitmap, go a little to the right below the
            // first text (node bitmap, too)
            if( !pView->nContextBmpWidthMax )
                aPos1.X() += rExpandedNodeBitmap.GetSizePixel().Width() / 2;

            aPos1.Y() = nY;
            aPos1.Y() += nEntryHeightDIV2;

            pChild = pView->FirstChild( pEntry );
            DBG_ASSERT(pChild,"Child?");
            pChild = pTree->LastSibling( pChild );
            nDistance = (sal_uInt16)(pView->GetVisiblePos(pChild) -
                                 pView->GetVisiblePos(pEntry));
            aPos2 = aPos1;
            aPos2.Y() += nDistance * nEntryHeight;
            pView->DrawLine( aPos1, aPos2 );
        }
        // visible in control?
        if( n>= nOffs && ((m_nStyle & WB_HASLINESATROOT) || !pTree->IsAtRootDepth(pEntry)))
        {
            // can we recycle aPos1?
            if( !pView->IsExpanded(pEntry) )
            {
                // nope
                aPos1.X() = pView->GetTabPos(pEntry, pFirstDynamicTab);
                // if it is not a context bitmap, go a little to the right below
                // the first text (node bitmap, too)
                if( !pView->nContextBmpWidthMax )
                    aPos1.X() += rExpandedNodeBitmap.GetSizePixel().Width() / 2;
                aPos1.Y() = nY;
                aPos1.Y() += nEntryHeightDIV2;
                aPos2.X() = aPos1.X();
            }
            aPos2.Y() = aPos1.Y();
            aPos2.X() -= pView->GetIndent();
            pView->DrawLine( aPos1, aPos2 );
        }
        nY += nEntryHeight;
        pEntry = (SvLBoxEntry*)(pView->NextVisible( pEntry ));
    }
    if( m_nStyle & WB_HASLINESATROOT )
    {
        pEntry = pView->First();
        aPos1.X() = pView->GetTabPos( pEntry, pFirstDynamicTab);
        // if it is not a context bitmap, go a little to the right below the
        // first text (node bitmap, too)
        if( !pView->nContextBmpWidthMax )
            aPos1.X() += rExpandedNodeBitmap.GetSizePixel().Width() / 2;
        aPos1.X() -=  pView->GetIndent();
        aPos1.Y() = GetEntryLine( pEntry );
        aPos1.Y() += nEntryHeightDIV2;
        pChild = pTree->LastSibling( pEntry );
        aPos2.X() = aPos1.X();
        aPos2.Y() = GetEntryLine( pChild );
        aPos2.Y() += nEntryHeightDIV2;
        pView->DrawLine( aPos1, aPos2 );
    }
    pView->SetLineColor( aOldLineColor );
}

void SvImpLBox::PositionScrollBars( Size& rSize, sal_uInt16 nMask )
{
    long nOverlap = 0;

    Size aVerSize( nVerSBarWidth, rSize.Height() );
    Size aHorSize( rSize.Width(), nHorSBarHeight );

    if( nMask & 0x0001 )
        aHorSize.Width() -= nVerSBarWidth;
    if( nMask & 0x0002 )
        aVerSize.Height() -= nHorSBarHeight;

    aVerSize.Height() += 2 * nOverlap;
    Point aVerPos( rSize.Width() - aVerSize.Width() + nOverlap, -nOverlap );
    aVerSBar.SetPosSizePixel( aVerPos, aVerSize );

    aHorSize.Width() += 2 * nOverlap;
    Point aHorPos( -nOverlap, rSize.Height() - aHorSize.Height() + nOverlap );

    aHorSBar.SetPosSizePixel( aHorPos, aHorSize );

    if( nMask & 0x0001 )
        rSize.Width() = aVerPos.X();
    if( nMask & 0x0002 )
        rSize.Height() = aHorPos.Y();

    if( (nMask & (0x0001|0x0002)) == (0x0001|0x0002) )
        aScrBarBox.Show();
    else
        aScrBarBox.Hide();
}

// nResult: Bit0 == VerSBar Bit1 == HorSBar
sal_uInt16 SvImpLBox::AdjustScrollBars( Size& rSize )
{
    long nEntryHeight = pView->GetEntryHeight();
    if( !nEntryHeight )
        return 0;

    sal_uInt16 nResult = 0;

    Size aOSize( pView->Control::GetOutputSizePixel() );

    const WinBits nWindowStyle = pView->GetStyle();
    sal_Bool bVerSBar = ( nWindowStyle & WB_VSCROLL ) != 0;
    sal_Bool bHorBar = sal_False;
    long nMaxRight = aOSize.Width(); //GetOutputSize().Width();
    Point aOrigin( pView->GetMapMode().GetOrigin() );
    aOrigin.X() *= -1;
    nMaxRight += aOrigin.X() - 1;
    long nVis = nMostRight - aOrigin.X();
    if( (nWindowStyle & WB_HSCROLL) &&
        (nVis < nMostRight || nMaxRight < nMostRight) )
    {
        bHorBar = sal_True;
    }

    // number of entries that are not collapsed
    sal_uLong nTotalCount = pView->GetVisibleCount();

    // number of entries visible within the view
    nVisibleCount = aOSize.Height() / nEntryHeight;

    // do we need a vertical scrollbar?
    if( bVerSBar || nTotalCount > nVisibleCount )
    {
        nResult = 1;
        nFlags |= F_HOR_SBARSIZE_WITH_VBAR;
        nMaxRight -= nVerSBarWidth;
        if( !bHorBar )
        {
            if( (nWindowStyle & WB_HSCROLL) &&
                (nVis < nMostRight || nMaxRight < nMostRight) )
                bHorBar = sal_True;
        }
    }

    // do we need a horizontal scrollbar?
    if( bHorBar )
    {
        nResult |= 0x0002;
        // the number of entries visible within the view has to be recalculated
        // because the horizontal scrollbar is now visible.
        nVisibleCount =  (aOSize.Height() - nHorSBarHeight) / nEntryHeight;
        // we might actually need a vertical scrollbar now
        if( !(nResult & 0x0001) &&
            ((nTotalCount > nVisibleCount) || bVerSBar) )
        {
            nResult = 3;
            nFlags |= F_VER_SBARSIZE_WITH_HBAR;
        }
    }

    PositionScrollBars( aOSize, nResult );

    // adapt Range, VisibleRange etc.

    // refresh output size, in case we have to scroll
    Rectangle aRect;
    aRect.SetSize( aOSize );
    aSelEng.SetVisibleArea( aRect );

    // vertical scrollbar
    long nTemp = (long)nVisibleCount;
    nTemp--;
    if( nTemp != aVerSBar.GetVisibleSize() )
    {
        if( !bInVScrollHdl )
        {
            aVerSBar.SetPageSize( nTemp - 1 );
            aVerSBar.SetVisibleSize( nTemp );
        }
        else
        {
            nFlags |= F_ENDSCROLL_SET_VIS_SIZE;
            nNextVerVisSize = nTemp;
        }
    }

    // horizontal scrollbar
    nTemp = aHorSBar.GetThumbPos();
    aHorSBar.SetVisibleSize( aOSize.Width() );
    long nNewThumbPos = aHorSBar.GetThumbPos();
    Range aRange( aHorSBar.GetRange() );
    if( aRange.Max() < nMostRight+25 )
    {
        aRange.Max() = nMostRight+25;
        aHorSBar.SetRange( aRange );
    }

    if( nTemp != nNewThumbPos )
    {
        nTemp = nNewThumbPos - nTemp;
        if( pView->IsEditingActive() )
        {
            pView->EndEditing( sal_True ); // Cancel
            pView->Update();
        }
        pView->nFocusWidth = -1;
        KeyLeftRight( nTemp );
    }

    if( nResult & 0x0001 )
        aVerSBar.Show();
    else
        aVerSBar.Hide();

    if( nResult & 0x0002 )
        aHorSBar.Show();
    else
    {
        aHorSBar.Hide();
    }
    rSize = aOSize;
    return nResult;
}

void SvImpLBox::InitScrollBarBox()
{
    aScrBarBox.SetSizePixel( Size(nVerSBarWidth, nHorSBarHeight) );
    Size aSize( pView->Control::GetOutputSizePixel() );
    aScrBarBox.SetPosPixel( Point(aSize.Width()-nVerSBarWidth, aSize.Height()-nHorSBarHeight));
}

void SvImpLBox::Resize()
{
    Size aSize( pView->Control::GetOutputSizePixel());
    if( aSize.Width() <= 0 || aSize.Height() <= 0 )
        return;
    nFlags |= F_IN_RESIZE;
    InitScrollBarBox();

    if( pView->GetEntryHeight())
    {
        AdjustScrollBars( aOutputSize );
        FillView();
    }
    // HACK, as in floating and docked windows the scrollbars might not be drawn
    // correctly/not be drawn at all after resizing!
    if( aHorSBar.IsVisible())
        aHorSBar.Invalidate();
    if( aVerSBar.IsVisible())
        aVerSBar.Invalidate();
    nFlags &= (~(F_IN_RESIZE | F_PAINTED));
}

void SvImpLBox::FillView()
{
    if( !pStartEntry )
    {
        sal_uInt16 nVisibleViewCount = (sal_uInt16)(pView->GetVisibleCount());
        sal_uInt16 nTempThumb = (sal_uInt16)aVerSBar.GetThumbPos();
        if( nTempThumb >= nVisibleViewCount )
            nTempThumb = nVisibleViewCount - 1;
        pStartEntry = (SvLBoxEntry*)(pView->GetEntryAtVisPos(nTempThumb));
    }
    if( pStartEntry )
    {
        sal_uInt16 nLast = (sal_uInt16)(pView->GetVisiblePos( (SvLBoxEntry*)(pView->LastVisible())));
        sal_uInt16 nThumb = (sal_uInt16)(pView->GetVisiblePos( pStartEntry ));
        sal_uInt16 nCurDispEntries = nLast-nThumb+1;
        if( nCurDispEntries <  nVisibleCount )
        {
            ShowCursor( sal_False );
            // fill window by moving the thumb up incrementally
            sal_Bool bFound = sal_False;
            SvLBoxEntry* pTemp = pStartEntry;
            while( nCurDispEntries < nVisibleCount && pTemp )
            {
                pTemp = (SvLBoxEntry*)(pView->PrevVisible(pStartEntry));
                if( pTemp )
                {
                    nThumb--;
                    pStartEntry = pTemp;
                    nCurDispEntries++;
                    bFound = sal_True;
                }
            }
            if( bFound )
            {
                aVerSBar.SetThumbPos( nThumb );
                ShowCursor( sal_True ); // recalculate focus rectangle
                pView->Invalidate();
            }
        }
    }
}




void SvImpLBox::ShowVerSBar()
{
    sal_Bool bVerBar = ( pView->GetStyle() & WB_VSCROLL ) != 0;
    sal_uLong nVis = 0;
    if( !bVerBar )
        nVis = pView->GetVisibleCount();
    if( bVerBar || (nVisibleCount && nVis > (sal_uLong)(nVisibleCount-1)) )
    {
        if( !aVerSBar.IsVisible() )
        {
            pView->nFocusWidth = -1;
            AdjustScrollBars( aOutputSize );
            if( GetUpdateMode() )
                aVerSBar.Update();
        }
    }
    else
    {
        if( aVerSBar.IsVisible() )
        {
            pView->nFocusWidth = -1;
            AdjustScrollBars( aOutputSize );
        }
    }

    long nMaxRight = GetOutputSize().Width();
    Point aPos( pView->GetMapMode().GetOrigin() );
    aPos.X() *= -1; // convert document coordinates
    nMaxRight = nMaxRight + aPos.X() - 1;
    if( nMaxRight < nMostRight  )
    {
        if( !aHorSBar.IsVisible() )
        {
            pView->nFocusWidth = -1;
            AdjustScrollBars( aOutputSize );
            if( GetUpdateMode() )
                aHorSBar.Update();
        }
        else
        {
            Range aRange( aHorSBar.GetRange() );
            if( aRange.Max() < nMostRight+25 )
            {
                aRange.Max() = nMostRight+25;
                aHorSBar.SetRange( aRange );
            }
            else
            {
                pView->nFocusWidth = -1;
                AdjustScrollBars( aOutputSize );
            }
        }
    }
    else
    {
        if( aHorSBar.IsVisible() )
        {
            pView->nFocusWidth = -1;
            AdjustScrollBars( aOutputSize );
        }
    }
}


void SvImpLBox::SyncVerThumb()
{
    if( pStartEntry )
    {
        long nEntryPos = pView->GetVisiblePos( pStartEntry );
        aVerSBar.SetThumbPos( nEntryPos );
    }
    else
        aVerSBar.SetThumbPos( 0 );
}

sal_Bool SvImpLBox::IsEntryInView( SvLBoxEntry* pEntry ) const
{
    // parent collapsed
    if( !pView->IsEntryVisible(pEntry) )
        return sal_False;
    long nY = GetEntryLine( pEntry );
    if( nY < 0 )
        return sal_False;
    long nMax = nVisibleCount * pView->GetEntryHeight();
    if( nY >= nMax )
        return sal_False;
    return sal_True;
}


long SvImpLBox::GetEntryLine( SvLBoxEntry* pEntry ) const
{
    if(!pStartEntry )
        return -1; // invisible position

    long nFirstVisPos = pView->GetVisiblePos( pStartEntry );
    long nEntryVisPos = pView->GetVisiblePos( pEntry );
    nFirstVisPos = nEntryVisPos - nFirstVisPos;
    nFirstVisPos *= pView->GetEntryHeight();
    return nFirstVisPos;
}

void SvImpLBox::SetEntryHeight( short /* nHeight */ )
{
    SetNodeBmpYOffset( GetExpandedNodeBmp() );
    SetNodeBmpYOffset( GetCollapsedNodeBmp() );
    if(!pView->HasViewData()) // are we within the Clear?
    {
        Size aSize = pView->Control::GetOutputSizePixel();
        AdjustScrollBars( aSize );
    }
    else
    {
        Resize();
        if( GetUpdateMode() )
            pView->Invalidate();
    }
}



// ***********************************************************************
// Callback Functions
// ***********************************************************************

void SvImpLBox::EntryExpanded( SvLBoxEntry* pEntry )
{
    // SelAllDestrAnch( sal_False, sal_True ); //DeselectAll();
    if( GetUpdateMode() )
    {
        ShowCursor( sal_False );
        long nY = GetEntryLine( pEntry );
        if( IsLineVisible(nY) )
        {
            InvalidateEntriesFrom( nY );
            FindMostRight( pEntry, 0  );
        }
        aVerSBar.SetRange( Range(0, pView->GetVisibleCount()-1 ) );
        // if we expanded before the thumb, the thumb's position has to be
        // corrected
        SyncVerThumb();
        ShowVerSBar();
        ShowCursor( sal_True );
    }
}

void SvImpLBox::EntryCollapsed( SvLBoxEntry* pEntry )
{
    if( !pView->IsEntryVisible( pEntry ) )
        return;

    ShowCursor( sal_False );

    if( !pMostRightEntry || pTree->IsChild( pEntry,pMostRightEntry ) )
    {
        FindMostRight(0);
    }

    if( pStartEntry )
    {
        long nOldThumbPos   = aVerSBar.GetThumbPos();
        sal_uLong nVisList      = pView->GetVisibleCount();
        aVerSBar.SetRange( Range(0, nVisList-1) );
        long nNewThumbPos   = aVerSBar.GetThumbPos();
        if( nNewThumbPos != nOldThumbPos  )
        {
            pStartEntry = pView->First();
            sal_uInt16 nDistance = (sal_uInt16)nNewThumbPos;
            if( nDistance )
                pStartEntry = (SvLBoxEntry*)(pView->NextVisible( pStartEntry,
                                                        nDistance));
            if( GetUpdateMode() )
                pView->Invalidate();
        }
        else
            SyncVerThumb();
        ShowVerSBar();
    }
    // has the cursor been collapsed?
    if( pTree->IsChild( pEntry, pCursor ) )
        SetCursor( pEntry );
    if( GetUpdateMode() )
        ShowVerSBar();
    ShowCursor( sal_True );
    if( GetUpdateMode() && pCursor )
        pView->Select( pCursor, sal_True );
}

void SvImpLBox::CollapsingEntry( SvLBoxEntry* pEntry )
{
    if( !pView->IsEntryVisible( pEntry ) || !pStartEntry )
        return;

    SelAllDestrAnch( sal_False, sal_True ); // deselect all

    // is the collapsed cursor visible?
    long nY = GetEntryLine( pEntry );
    if( IsLineVisible(nY) )
    {
        if( GetUpdateMode() )
            InvalidateEntriesFrom( nY );
    }
    else
    {
        if( pTree->IsChild(pEntry, pStartEntry) )
        {
            pStartEntry = pEntry;
            if( GetUpdateMode() )
                pView->Invalidate();
        }
    }
}


void SvImpLBox::SetNodeBmpYOffset( const Image& rBmp )
{
    Size aSize;
    nYoffsNodeBmp = pView->GetHeightOffset( rBmp, aSize );
    nNodeBmpWidth = aSize.Width();
}

void SvImpLBox::SetNodeBmpTabDistance()
{
    nNodeBmpTabDistance = -pView->GetIndent();
    if( pView->nContextBmpWidthMax )
    {
        // only if the first dynamic tab is centered (we currently assume that)
        Size aSize = GetExpandedNodeBmp().GetSizePixel();
        nNodeBmpTabDistance -= aSize.Width() / 2;
    }
}

//
// corrects the cursor when using SingleSelection
//
void SvImpLBox::EntrySelected( SvLBoxEntry* pEntry, sal_Bool bSelect )
{
    if( nFlags & F_IGNORE_SELECT )
        return;

    nFlags &= (~F_DESEL_ALL);
    if( bSelect &&
        aSelEng.GetSelectionMode() == SINGLE_SELECTION &&
        pEntry != pCursor )
    {
        SetCursor( pEntry );
        DBG_ASSERT(pView->GetSelectionCount()==1,"selection count?");
    }

    if( GetUpdateMode() && pView->IsEntryVisible(pEntry) )
    {
        long nY = GetEntryLine( pEntry );
        if( IsLineVisible( nY ) )
        {
            ShowCursor( sal_False );
            pView->PaintEntry1( pEntry, nY, 0xffff ); // because of ItemsetBrowser SV_LBOXTAB_SHOW_SELECTION );
            ShowCursor( sal_True );
        }
    }
}


void SvImpLBox::RemovingEntry( SvLBoxEntry* pEntry )
{
    DestroyAnchor();

    if( !pView->IsEntryVisible( pEntry ) )
    {
        // if parent is collapsed => bye!
        nFlags |= F_REMOVED_ENTRY_INVISIBLE;
        return;
    }

    if( pEntry == pMostRightEntry || (
        pEntry->HasChildren() && pView->IsExpanded(pEntry) &&
        pTree->IsChild(pEntry, pMostRightEntry)))
    {
        nFlags |= F_REMOVED_RECALC_MOST_RIGHT;
    }

    SvLBoxEntry* pOldStartEntry = pStartEntry;

    SvLBoxEntry* pParent = (SvLBoxEntry*)(pView->GetModel()->GetParent(pEntry));

    if( pParent && pView->GetModel()->GetChildList(pParent)->size() == 1 )
    {
        DBG_ASSERT( pView->IsExpanded( pParent ), "Parent not expanded");
        pParent->SetFlags( pParent->GetFlags() | SV_ENTRYFLAG_NO_NODEBMP);
        InvalidateEntry( pParent );
    }

    if( pCursor && pTree->IsChild( pEntry, pCursor) )
        pCursor = pEntry;
    if( pStartEntry && pTree->IsChild(pEntry,pStartEntry) )
        pStartEntry = pEntry;

    SvLBoxEntry* pTemp;
    if( pCursor && pCursor == pEntry )
    {
        if( bSimpleTravel )
            pView->Select( pCursor, sal_False );
        ShowCursor( sal_False );    // focus rectangle gone
        // NextSibling, because we also delete the children of the cursor
        pTemp = pView->NextSibling( pCursor );
        if( !pTemp )
            pTemp = (SvLBoxEntry*)(pView->PrevVisible( pCursor ));

        SetCursor( pTemp, sal_True );
    }
    if( pStartEntry && pStartEntry == pEntry )
    {
        pTemp = pView->NextSibling( pStartEntry );
        if( !pTemp )
            pTemp = (SvLBoxEntry*)(pView->PrevVisible( pStartEntry ));
        pStartEntry = pTemp;
    }
    if( GetUpdateMode())
    {
        // if it is the last one, we have to invalidate it, so the lines are
        // drawn correctly (in this case they're deleted)
        if( pStartEntry && (pStartEntry != pOldStartEntry || pEntry == (SvLBoxEntry*)pView->GetModel()->Last()) )
        {
            aVerSBar.SetThumbPos( pView->GetVisiblePos( pStartEntry ));
            pView->Invalidate( GetVisibleArea() );
        }
        else
            InvalidateEntriesFrom( GetEntryLine( pEntry ) );
    }
}

void SvImpLBox::EntryRemoved()
{
    if( nFlags & F_REMOVED_ENTRY_INVISIBLE )
    {
        nFlags &= (~F_REMOVED_ENTRY_INVISIBLE);
        return;
    }
    if( !pStartEntry )
        pStartEntry = pTree->First();
    if( !pCursor )
        SetCursor( pStartEntry, sal_True );

    if( pCursor && (bSimpleTravel || !pView->GetSelectionCount() ))
        pView->Select( pCursor, sal_True );

    if( GetUpdateMode())
    {
        if( nFlags & F_REMOVED_RECALC_MOST_RIGHT )
            FindMostRight(0);
        aVerSBar.SetRange( Range(0, pView->GetVisibleCount()-1 ) );
        FillView();
        if( pStartEntry )
            // if something above the thumb was deleted
            aVerSBar.SetThumbPos( pView->GetVisiblePos( pStartEntry) );

        ShowVerSBar();
        if( pCursor && pView->HasFocus() && !pView->IsSelected(pCursor) )
        {
            if( pView->GetSelectionCount() )
            {
                // is a neighboring entry selected?
                SvLBoxEntry* pNextCursor = (SvLBoxEntry*)pView->PrevVisible( pCursor );
                if( !pNextCursor || !pView->IsSelected( pNextCursor ))
                    pNextCursor = (SvLBoxEntry*)pView->NextVisible( pCursor );
                if( !pNextCursor || !pView->IsSelected( pNextCursor ))
                    // no neighbor selected: use first selected
                    pNextCursor = pView->FirstSelected();
                SetCursor( pNextCursor );
                MakeVisible( pCursor );
            }
            else
                pView->Select( pCursor, sal_True );
        }
        ShowCursor( sal_True );
    }
    nFlags &= (~F_REMOVED_RECALC_MOST_RIGHT);
}


void SvImpLBox::MovingEntry( SvLBoxEntry* pEntry )
{
    int bDeselAll = nFlags & F_DESEL_ALL;
    SelAllDestrAnch( sal_False, sal_True );  // DeselectAll();
    if( !bDeselAll )
        nFlags &= (~F_DESEL_ALL);

    if( pEntry == pCursor )
        ShowCursor( sal_False );
    if( IsEntryInView( pEntry ) )
        pView->Invalidate();
    if( pEntry == pStartEntry )
    {
        SvLBoxEntry* pNew = 0;
        if( !pEntry->HasChildren() )
        {
            pNew = (SvLBoxEntry*)(pView->NextVisible( pStartEntry ));
            if( !pNew )
                pNew = (SvLBoxEntry*)(pView->PrevVisible( pStartEntry ));
        }
        else
        {
            pNew = pTree->NextSibling( pEntry );
            if( !pNew )
                pNew = pTree->PrevSibling( pEntry );
        }
        pStartEntry = pNew;
    }
}

void SvImpLBox::EntryMoved( SvLBoxEntry* pEntry )
{
    UpdateContextBmpWidthVectorFromMovedEntry( pEntry );

    if ( !pStartEntry )
        // this might happen if the only entry in the view is moved to its very same position
        // #i97346#
        pStartEntry = pView->First();

    aVerSBar.SetRange( Range(0, pView->GetVisibleCount()-1));
    sal_uInt16 nFirstPos = (sal_uInt16)pTree->GetAbsPos( pStartEntry );
    sal_uInt16 nNewPos = (sal_uInt16)pTree->GetAbsPos( pEntry );
    FindMostRight(0);
    if( nNewPos < nFirstPos ) // HACK!
        pStartEntry = pEntry;
    SyncVerThumb();
    if( pEntry == pCursor )
    {
        if( pView->IsEntryVisible( pCursor ) )
            ShowCursor( sal_True );
        else
        {
            SvLBoxEntry* pParent = pEntry;
            do {
                pParent = pTree->GetParent( pParent );
            }
            while( !pView->IsEntryVisible( pParent ) );
            SetCursor( pParent );
        }
    }
    if( IsEntryInView( pEntry ) )
        pView->Invalidate();
}



void SvImpLBox::EntryInserted( SvLBoxEntry* pEntry )
{
    if( GetUpdateMode() )
    {
        SvLBoxEntry* pParent = (SvLBoxEntry*)pTree->GetParent(pEntry);
        if( pParent && pTree->GetChildList(pParent)->size() == 1 )
            // draw plus sign
            pTree->InvalidateEntry( pParent );

        if( !pView->IsEntryVisible( pEntry ) )
            return;
        int bDeselAll = nFlags & F_DESEL_ALL;
        if( bDeselAll )
            SelAllDestrAnch( sal_False, sal_True );
        else
            DestroyAnchor();
        //  nFlags &= (~F_DESEL_ALL);
//      ShowCursor( sal_False ); // if cursor is moved lower
        long nY = GetEntryLine( pEntry );
        sal_Bool bEntryVisible = IsLineVisible( nY );
        if( bEntryVisible )
        {
            ShowCursor( sal_False ); // if cursor is moved lower
            nY -= pView->GetEntryHeight(); // because of lines
            InvalidateEntriesFrom( nY );
        }
        else if( pStartEntry && nY < GetEntryLine(pStartEntry) )
        {
            // Check if the view is filled completely. If not, then adjust
            // pStartEntry and the Cursor (automatic scrolling).
            sal_uInt16 nLast = (sal_uInt16)(pView->GetVisiblePos( (SvLBoxEntry*)(pView->LastVisible())));
            sal_uInt16 nThumb = (sal_uInt16)(pView->GetVisiblePos( pStartEntry ));
            sal_uInt16 nCurDispEntries = nLast-nThumb+1;
            if( nCurDispEntries < nVisibleCount )
            {
                // set at the next paint event
                pStartEntry = 0;
                SetCursor( 0 );
                pView->Invalidate();
            }
        }
        else if( !pStartEntry )
            pView->Invalidate();

        SetMostRight( pEntry );
        aVerSBar.SetRange( Range(0, pView->GetVisibleCount()-1));
        SyncVerThumb(); // if something was inserted before the thumb
        ShowVerSBar();
        ShowCursor( sal_True );
        if( pStartEntry != pView->First() && (nFlags & F_FILLING) )
            pView->Update();
    }
}



// ********************************************************************
// Event handler
// ********************************************************************


// ****** Control the control animation

sal_Bool SvImpLBox::ButtonDownCheckCtrl(const MouseEvent& rMEvt, SvLBoxEntry* pEntry,
                                   long nY  )
{
    SvLBoxItem* pItem = pView->GetItem(pEntry,rMEvt.GetPosPixel().X(),&pActiveTab);
    if( pItem && (pItem->IsA()==SV_ITEM_ID_LBOXBUTTON))
    {
        pActiveButton = (SvLBoxButton*)pItem;
        pActiveEntry = pEntry;
        if( pCursor == pActiveEntry )
            pView->HideFocus();
        pView->CaptureMouse();
        pActiveButton->SetStateHilighted( sal_True );
        pView->PaintEntry1( pActiveEntry, nY,
                    SV_LBOXTAB_PUSHABLE | SV_LBOXTAB_ADJUST_CENTER |
                    SV_LBOXTAB_ADJUST_RIGHT );
        return sal_True;
    }
    else
        pActiveButton = 0;
    return sal_False;
}

sal_Bool SvImpLBox::MouseMoveCheckCtrl( const MouseEvent& rMEvt, SvLBoxEntry* pEntry)
{
    if( pActiveButton )
    {
        long nY;
        long nMouseX = rMEvt.GetPosPixel().X();
        if( pEntry == pActiveEntry &&
             pView->GetItem(pActiveEntry, nMouseX) == pActiveButton )
        {
            if( !pActiveButton->IsStateHilighted() )
            {
                pActiveButton->SetStateHilighted(sal_True );
                nY = GetEntryLine( pActiveEntry );
                pView->PaintEntry1( pActiveEntry, nY,
                    SV_LBOXTAB_PUSHABLE | SV_LBOXTAB_ADJUST_CENTER |
                    SV_LBOXTAB_ADJUST_RIGHT );
            }
        }
        else
        {
            if( pActiveButton->IsStateHilighted() )
            {
                pActiveButton->SetStateHilighted(sal_False );
                nY = GetEntryLine( pActiveEntry );
                pView->PaintEntry1( pActiveEntry, nY, SV_LBOXTAB_PUSHABLE );
            }
        }
        return sal_True;
    }
    return sal_False;
}

sal_Bool SvImpLBox::ButtonUpCheckCtrl( const MouseEvent& rMEvt )
{
    if( pActiveButton )
    {
        pView->ReleaseMouse();
        SvLBoxEntry* pEntry = GetClickedEntry( rMEvt.GetPosPixel() );
        long nY = GetEntryLine( pActiveEntry );
        pActiveButton->SetStateHilighted( sal_False );
        long nMouseX = rMEvt.GetPosPixel().X();
        if( pEntry == pActiveEntry &&
             pView->GetItem( pActiveEntry, nMouseX ) == pActiveButton )
            pActiveButton->ClickHdl( pView, pActiveEntry );
        pView->PaintEntry1( pActiveEntry, nY,
                    SV_LBOXTAB_PUSHABLE | SV_LBOXTAB_ADJUST_CENTER |
                    SV_LBOXTAB_ADJUST_RIGHT );
        if( pCursor == pActiveEntry )
            ShowCursor( sal_True );
        pActiveButton = 0;
        pActiveEntry = 0;
        pActiveTab = 0;
        return sal_True;
    }
    return sal_False;
}

// ******* Control plus/minus button for expanding/collapsing

// sal_False == no expand/collapse button hit
sal_Bool SvImpLBox::IsNodeButton( const Point& rPosPixel, SvLBoxEntry* pEntry ) const
{
    if( !pEntry->HasChildren() && !pEntry->HasChildrenOnDemand() )
        return sal_False;

    SvLBoxTab* pFirstDynamicTab = pView->GetFirstDynamicTab();
    if( !pFirstDynamicTab )
        return sal_False;

    long nMouseX = rPosPixel.X();
    // convert to document coordinates
    Point aOrigin( pView->GetMapMode().GetOrigin() );
    nMouseX -= aOrigin.X();

    long nX = pView->GetTabPos( pEntry, pFirstDynamicTab);
    nX += nNodeBmpTabDistance;
    if( nMouseX < nX )
        return sal_False;
    nX += nNodeBmpWidth;
    if( nMouseX > nX )
        return sal_False;
    return sal_True;
}

// sal_False == hit no node button
sal_Bool SvImpLBox::ButtonDownCheckExpand( const MouseEvent& rMEvt, SvLBoxEntry* pEntry, long /* nY */ )
{
    sal_Bool bRet = sal_False;

    if ( pView->IsEditingActive() && pEntry == pView->pEdEntry )
        // inplace editing -> nothing to do
        bRet = sal_True;
    else if ( IsNodeButton( rMEvt.GetPosPixel(), pEntry ) )
    {
        if ( pView->IsExpanded( pEntry ) )
        {
            pView->EndEditing( sal_True );
            pView->Collapse( pEntry );
        }
        else
        {
            // you can expand an entry, which is in editing
            pView->Expand( pEntry );
        }
        bRet = sal_True;
    }

    return bRet;
}

void SvImpLBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( !rMEvt.IsLeft() && !rMEvt.IsRight())
        return;

    aEditTimer.Stop();
    Point aPos( rMEvt.GetPosPixel());

    if( aPos.X() > aOutputSize.Width() || aPos.Y() > aOutputSize.Height() )
        return;

    SvLBoxEntry* pEntry = GetEntry( aPos );
    if ( pEntry != pCursor )
        // new entry selected -> reset current tab position to first tab
        nCurTabPos = FIRST_ENTRY_TAB;
    nFlags &= (~F_FILLING);
    pView->GrabFocus();
    // the entry can still be invalid!
    if( !pEntry || !pView->GetViewData( pEntry ))
        return;

    long nY = GetEntryLine( pEntry );
    // Node-Button?
    if( ButtonDownCheckExpand( rMEvt, pEntry, nY ) )
        return;

    if( !EntryReallyHit(pEntry,aPos,nY))
        return;

    SvLBoxItem* pXItem = pView->GetItem( pEntry, aPos.X() );
    if( pXItem )
    {
        SvLBoxTab* pXTab = pView->GetTab( pEntry, pXItem );
        if ( !rMEvt.IsMod1() && !rMEvt.IsMod2() && rMEvt.IsLeft() && pXTab->IsEditable()
            && pEntry == pView->FirstSelected() && NULL == pView->NextSelected( pEntry ) )
                // #i8234# FirstSelected() and NextSelected() ensures, that inplace editing is only triggered, when only one entry is selected
            nFlags |= F_START_EDITTIMER;
        if ( !pView->IsSelected( pEntry ) )
            nFlags &= ~F_START_EDITTIMER;
    }


    if( (rMEvt.GetClicks() % 2) == 0 )
    {
        nFlags &= (~F_START_EDITTIMER);
        pView->pHdlEntry = pEntry;
        if( pView->DoubleClickHdl() )
        {
            // if the entry was deleted within the handler
            pEntry = GetClickedEntry( aPos );
            if( !pEntry )
                return;
            if( pEntry != pView->pHdlEntry )
            {
                // select anew & bye
                if( !bSimpleTravel && !aSelEng.IsAlwaysAdding())
                    SelAllDestrAnch( sal_False, sal_True ); // DeselectAll();
                SetCursor( pEntry );

                return;
            }
            if( pEntry->HasChildren() || pEntry->HasChildrenOnDemand() )
            {
                if( pView->IsExpanded(pEntry) )
                    pView->Collapse( pEntry );
                else
                    pView->Expand( pEntry );
                if( pEntry == pCursor )  // only if Entryitem was clicked
                                          // (Nodebutton is not an Entryitem!)
                    pView->Select( pCursor, sal_True );
                return;
            }
        }
    }
    else
    {
        // CheckButton? (TreeListBox: Check + Info)
        if( ButtonDownCheckCtrl(rMEvt, pEntry, nY) == sal_True)
            return;
        // Inplace-Editing?
    }
    if ( aSelEng.GetSelectionMode() != NO_SELECTION )
        aSelEng.SelMouseButtonDown( rMEvt );
}

void SvImpLBox::MouseButtonUp( const MouseEvent& rMEvt)
{
    if ( !ButtonUpCheckCtrl( rMEvt ) && ( aSelEng.GetSelectionMode() != NO_SELECTION ) )
        aSelEng.SelMouseButtonUp( rMEvt );
    EndScroll();
    if( nFlags & F_START_EDITTIMER )
    {
        nFlags &= (~F_START_EDITTIMER);
        aEditClickPos = rMEvt.GetPosPixel();
        aEditTimer.Start();
    }

    return;
}

void SvImpLBox::MouseMove( const MouseEvent& rMEvt)
{
    SvLBoxEntry* pEntry = GetClickedEntry( rMEvt.GetPosPixel() );
    if ( !MouseMoveCheckCtrl( rMEvt, pEntry ) && ( aSelEng.GetSelectionMode() != NO_SELECTION ) )
        aSelEng.SelMouseMove( rMEvt );
    return;
}

sal_Bool SvImpLBox::KeyInput( const KeyEvent& rKEvt)
{
    aEditTimer.Stop();
    const KeyCode&  rKeyCode = rKEvt.GetKeyCode();

    if( rKeyCode.IsMod2() )
        return sal_False; // don't evaluate Alt key

    nFlags &= (~F_FILLING);

    if( !pCursor )
        pCursor = pStartEntry;
    if( !pCursor )
        return sal_False;

    sal_Bool bKeyUsed = sal_True;

    sal_uInt16  nDelta = (sal_uInt16)aVerSBar.GetPageSize();
    sal_uInt16  aCode = rKeyCode.GetCode();

    sal_Bool    bShift = rKeyCode.IsShift();
    sal_Bool    bMod1 = rKeyCode.IsMod1();

    SvLBoxEntry* pNewCursor;

    const WinBits nWindowStyle = pView->GetStyle();
    switch( aCode )
    {
        case KEY_UP:
            if( !IsEntryInView( pCursor ) )
                MakeVisible( pCursor );

            pNewCursor = pCursor;
            do
            {
                pNewCursor = (SvLBoxEntry*)(pView->PrevVisible( pNewCursor ));
            } while( pNewCursor && !IsSelectable(pNewCursor) );

            if ( pNewCursor )
                // new entry selected -> reset current tab position to first tab
                nCurTabPos = FIRST_ENTRY_TAB;
            // if there is no next entry, take the current one
            // this ensures that in case of _one_ entry in the list, this entry is selected when pressing
            // the cursor key
            if ( !pNewCursor && pCursor )
                pNewCursor = pCursor;

            if( pNewCursor )
            {
                aSelEng.CursorPosChanging( bShift, bMod1 );
                SetCursor( pNewCursor, bMod1 );     // no selection, when Ctrl is on
                if( !IsEntryInView( pNewCursor ) )
                    KeyUp( sal_False );
            }
            break;

        case KEY_DOWN:
            if( !IsEntryInView( pCursor ) )
                MakeVisible( pCursor );

            pNewCursor = pCursor;
            do
            {
                pNewCursor = (SvLBoxEntry*)(pView->NextVisible( pNewCursor ));
            } while( pNewCursor && !IsSelectable(pNewCursor) );

            if ( pNewCursor )
                // new entry selected -> reset current tab position to first tab
                nCurTabPos = FIRST_ENTRY_TAB;

            // if there is no next entry, take the current one
            // this ensures that in case of _one_ entry in the list, this entry is selected when pressing
            // the cursor key
            // 06.09.20001 - 83416 - frank.schoenheit@sun.com
            if ( !pNewCursor && pCursor )
                pNewCursor = pCursor;

            if( pNewCursor )
            {
                aSelEng.CursorPosChanging( bShift, bMod1 );
                if( IsEntryInView( pNewCursor ) )
                    SetCursor( pNewCursor, bMod1 ); // no selection, when Ctrl is on
                else
                {
                    if( pCursor )
                        pView->Select( pCursor, sal_False );
                    KeyDown( sal_False );
                    SetCursor( pNewCursor, bMod1 ); // no selection, when Ctrl is on
                }
            }
            else
                KeyDown( sal_False );   // because scrollbar range might still
                                        // allow scrolling
            break;

        case KEY_RIGHT:
        {
            if( bSubLstOpLR && IsNowExpandable() )
                pView->Expand( pCursor );
            else if ( bIsCellFocusEnabled && pCursor )
            {
                if ( nCurTabPos < ( pView->TabCount() - 1 /*!2*/ ) )
                {
                    ++nCurTabPos;
                    ShowCursor( sal_True );
                    CallEventListeners( VCLEVENT_LISTBOX_SELECT, pCursor );
                }
            }
            else if( nWindowStyle & WB_HSCROLL )
            {
                long    nThumb = aHorSBar.GetThumbPos();
                nThumb += aHorSBar.GetLineSize();
                long    nOldThumb = aHorSBar.GetThumbPos();
                aHorSBar.SetThumbPos( nThumb );
                nThumb = nOldThumb;
                nThumb -= aHorSBar.GetThumbPos();
                nThumb *= -1;
                if( nThumb )
                {
                    KeyLeftRight( nThumb );
                    EndScroll();
                }
            }
            else
                bKeyUsed = sal_False;
            break;
        }

        case KEY_LEFT:
        {
            if ( bIsCellFocusEnabled )
            {
                if ( nCurTabPos > FIRST_ENTRY_TAB )
                {
                    --nCurTabPos;
                    ShowCursor( sal_True );
                    CallEventListeners( VCLEVENT_LISTBOX_SELECT, pCursor );
                }
            }
            else if ( nWindowStyle & WB_HSCROLL )
            {
                long    nThumb = aHorSBar.GetThumbPos();
                nThumb -= aHorSBar.GetLineSize();
                long    nOldThumb = aHorSBar.GetThumbPos();
                aHorSBar.SetThumbPos( nThumb );
                nThumb = nOldThumb;
                nThumb -= aHorSBar.GetThumbPos();
                if( nThumb )
                {
                    KeyLeftRight( -nThumb );
                    EndScroll();
                }
                else if( bSubLstOpLR )
                {
                    if( IsExpandable() && pView->IsExpanded( pCursor ) )
                        pView->Collapse( pCursor );
                    else
                    {
                        pNewCursor = pView->GetParent( pCursor );
                        if( pNewCursor )
                            SetCursor( pNewCursor );
                    }
                }
            }
            else if( bSubLstOpLR && IsExpandable() )
                pView->Collapse( pCursor );
            else
                bKeyUsed = sal_False;
            break;
        }

        case KEY_PAGEUP:
            if( !bMod1 )
            {
                pNewCursor = (SvLBoxEntry*)(pView->PrevVisible( pCursor, nDelta ));

                while( nDelta && pNewCursor && !IsSelectable(pNewCursor) )
                {
                    pNewCursor = (SvLBoxEntry*)(pView->NextVisible( pNewCursor ));
                    nDelta--;
                }

                if( nDelta )
                {
                    DBG_ASSERT(pNewCursor&&(sal_uLong)pNewCursor!=(sal_uLong)pCursor,"Cursor?");
                    aSelEng.CursorPosChanging( bShift, bMod1 );
                    if( IsEntryInView( pNewCursor ) )
                        SetCursor( pNewCursor );
                    else
                    {
                        SetCursor( pNewCursor );
                        KeyUp( sal_True );
                    }
                }
            }
            else
                bKeyUsed = sal_False;
            break;

        case KEY_PAGEDOWN:
            if( !bMod1 )
            {
                pNewCursor= (SvLBoxEntry*)(pView->NextVisible( pCursor, nDelta ));

                while( nDelta && pNewCursor && !IsSelectable(pNewCursor) )
                {
                    pNewCursor = (SvLBoxEntry*)(pView->PrevVisible( pNewCursor ));
                    nDelta--;
                }

                if( nDelta )
                {
                    DBG_ASSERT(pNewCursor&&(sal_uLong)pNewCursor!=(sal_uLong)pCursor,"Cursor?");
                    aSelEng.CursorPosChanging( bShift, bMod1 );
                    if( IsEntryInView( pNewCursor ) )
                        SetCursor( pNewCursor );
                    else
                    {
                        SetCursor( pNewCursor );
                        KeyDown( sal_True );
                    }
                }
                else
                    KeyDown( sal_False ); // see also: KEY_DOWN
            }
            else
                bKeyUsed = sal_False;
            break;

        case KEY_SPACE:
            if ( pView->GetSelectionMode() != NO_SELECTION )
            {
                if ( bMod1 )
                {
                    if ( pView->GetSelectionMode() == MULTIPLE_SELECTION && !bShift )
                        // toggle selection
                        pView->Select( pCursor, !pView->IsSelected( pCursor ) );
                }
                else if ( !bShift /*&& !bMod1*/ )
                {
                    if ( aSelEng.IsAddMode() )
                    {
                        // toggle selection
                        pView->Select( pCursor, !pView->IsSelected( pCursor ) );
                    }
                    else if ( !pView->IsSelected( pCursor ) )
                    {
                        SelAllDestrAnch( sal_False );
                        pView->Select( pCursor, sal_True );
                    }
                    else
                        bKeyUsed = sal_False;
                }
                else
                    bKeyUsed = sal_False;
            }
            else
                bKeyUsed = sal_False;
            break;

        case KEY_RETURN:
            if( bSubLstOpRet && IsExpandable() )
            {
                if( pView->IsExpanded( pCursor ) )
                    pView->Collapse( pCursor );
                else
                    pView->Expand( pCursor );
            }
            else
                bKeyUsed = sal_False;
            break;

        case KEY_F2:
            if( !bShift && !bMod1 )
            {
                aEditClickPos = Point( -1, -1 );
                EditTimerCall( 0 );
            }
            else
                bKeyUsed = sal_False;
            break;

        case KEY_F8:
            if( bShift && pView->GetSelectionMode()==MULTIPLE_SELECTION &&
                !(m_nStyle & WB_SIMPLEMODE))
            {
                if( aSelEng.IsAlwaysAdding() )
                    aSelEng.AddAlways( sal_False );
                else
                    aSelEng.AddAlways( sal_True );
            }
            else
                bKeyUsed = sal_False;
            break;

        case KEY_ADD:
            if( pCursor )
            {
                if( !pView->IsExpanded(pCursor))
                    pView->Expand( pCursor );
                if( bMod1 )
                {
                    sal_uInt16 nRefDepth = pTree->GetDepth( pCursor );
                    SvLBoxEntry* pCur = pTree->Next( pCursor );
                    while( pCur && pTree->GetDepth(pCur) > nRefDepth )
                    {
                        if( pCur->HasChildren() && !pView->IsExpanded(pCur))
                            pView->Expand( pCur );
                        pCur = pTree->Next( pCur );
                    }
                }
            }
            else
                bKeyUsed = sal_False;
            break;

        case KEY_A:
            if( bMod1 )
                SelAllDestrAnch( sal_True );
            else
                bKeyUsed = sal_False;
            break;

        case KEY_SUBTRACT:
            if( pCursor )
            {
                if( pView->IsExpanded(pCursor))
                    pView->Collapse( pCursor );
                if( bMod1 )
                {
                    // collapse all parents until we get to the root
                    SvLBoxEntry* pParentToCollapse = (SvLBoxEntry*)pTree->GetRootLevelParent(pCursor);
                    if( pParentToCollapse )
                    {
                        sal_uInt16 nRefDepth;
                        // special case explorer: if the root only has a single
                        // entry, don't collapse the root entry
                        if( pTree->GetChildList(0)->size() < 2 )
                        {
                            nRefDepth = 1;
                            pParentToCollapse = pCursor;
                            while( pTree->GetParent(pParentToCollapse) &&
                                   pTree->GetDepth( pTree->GetParent(pParentToCollapse)) > 0)
                            {
                                pParentToCollapse = pTree->GetParent(pParentToCollapse);
                            }
                        }
                        else
                            nRefDepth = 0;

                        if( pView->IsExpanded(pParentToCollapse) )
                            pView->Collapse( pParentToCollapse );
                        SvLBoxEntry* pCur = pTree->Next( pParentToCollapse );
                        while( pCur && pTree->GetDepth(pCur) > nRefDepth )
                        {
                            if( pCur->HasChildren() && pView->IsExpanded(pCur) )
                                pView->Collapse( pCur );
                            pCur = pTree->Next( pCur );
                        }
                    }
                }
            }
            else
                bKeyUsed = sal_False;
            break;

        case KEY_DIVIDE :
            if( bMod1 )
                SelAllDestrAnch( sal_True );
            else
                bKeyUsed = sal_False;
            break;

        case KEY_COMMA :
            if( bMod1 )
                SelAllDestrAnch( sal_False );
            else
                bKeyUsed = sal_False;
            break;

        case KEY_HOME :
            pNewCursor = pView->GetModel()->First();

            while( pNewCursor && !IsSelectable(pNewCursor) )
            {
                pNewCursor = (SvLBoxEntry*)(pView->NextVisible( pNewCursor ));
            }

            if( pNewCursor && pNewCursor != pCursor )
            {
//              SelAllDestrAnch( sal_False );
                aSelEng.CursorPosChanging( bShift, bMod1 );
                SetCursor( pNewCursor );
                if( !IsEntryInView( pNewCursor ) )
                    MakeVisible( pNewCursor );
            }
            else
                bKeyUsed = sal_False;
            break;

        case KEY_END :
            pNewCursor = pView->GetModel()->Last();

            while( pNewCursor && !IsSelectable(pNewCursor) )
            {
                pNewCursor = (SvLBoxEntry*)(pView->PrevVisible( pNewCursor ));
            }

            if( pNewCursor && pNewCursor != pCursor)
            {
//              SelAllDestrAnch( sal_False );
                aSelEng.CursorPosChanging( bShift, bMod1 );
                SetCursor( pNewCursor );
                if( !IsEntryInView( pNewCursor ) )
                    MakeVisible( pNewCursor );
            }
            else
                bKeyUsed = sal_False;
            break;

        case KEY_ESCAPE:
        case KEY_TAB:
        case KEY_DELETE:
        case KEY_BACKSPACE:
            // must not be handled because this quits dialogs and does other magic things...
            // if there are other single keys which should not be handled, they can be added here
            bKeyUsed = sal_False;
            break;

        default:
            // is there any reason why we should eat the events here? The only place where this is called
            // is from SvTreeListBox::KeyInput. If we set bKeyUsed to sal_True here, then the key input
            // is just silenced. However, we want SvLBox::KeyInput to get a chance, to do the QuickSelection
            // handling.
            // (The old code here which intentionally set bKeyUsed to TRUE said this was because of "quick search"
            // handling, but actually there was no quick search handling anymore. We just re-implemented it.)
            // #i31275# / 2009-06-16 / frank.schoenheit@sun.com
            bKeyUsed = sal_False;
            break;
    }
    return bKeyUsed;
}

void SvImpLBox::GetFocus()
{
    if( pCursor )
    {
        pView->SetEntryFocus( pCursor, sal_True );
        ShowCursor( sal_True );
// auskommentiert wg. deselectall
//      if( bSimpleTravel && !pView->IsSelected(pCursor) )
//          pView->Select( pCursor, sal_True );
    }
    if( m_nStyle & WB_HIDESELECTION )
    {
        SvLBoxEntry* pEntry = pView->FirstSelected();
        while( pEntry )
        {
            InvalidateEntry( pEntry );
            pEntry = pView->NextSelected( pEntry );
        }
    }
}

void SvImpLBox::LoseFocus()
{
    aEditTimer.Stop();
    if( pCursor )
        pView->SetEntryFocus( pCursor,sal_False );
    ShowCursor( sal_False );

    if( m_nStyle & WB_HIDESELECTION )
    {
        SvLBoxEntry* pEntry = pView->FirstSelected();
        while( pEntry )
        {
            //SvViewData* pViewData = pView->GetViewData( pEntry );
            //pViewData->SetCursored( sal_True );
            InvalidateEntry( pEntry );
            pEntry = pView->NextSelected( pEntry );
        }
    }
}


// ********************************************************************
// SelectionEngine
// ********************************************************************

inline void SvImpLBox::SelectEntry( SvLBoxEntry* pEntry, sal_Bool bSelect )
{
    pView->Select( pEntry, bSelect );
}

ImpLBSelEng::ImpLBSelEng( SvImpLBox* pImpl, SelectionEngine* pSEng,
    SvTreeListBox* pV )
{
    pImp = pImpl;
    pSelEng = pSEng;
    pView = pV;
}

ImpLBSelEng::~ImpLBSelEng()
{
}

void ImpLBSelEng::BeginDrag()
{
    pImp->BeginDrag();
}

void ImpLBSelEng::CreateAnchor()
{
    pImp->pAnchor = pImp->pCursor;
}

void ImpLBSelEng::DestroyAnchor()
{
    pImp->pAnchor = 0;
}

sal_Bool ImpLBSelEng::SetCursorAtPoint(const Point& rPoint, sal_Bool bDontSelectAtCursor)
{
    SvLBoxEntry* pNewCursor = pImp->MakePointVisible( rPoint );
    if( pNewCursor != pImp->pCursor  )
        pImp->BeginScroll();

    if( pNewCursor )
    {
        // at SimpleTravel, the SetCursor is selected and the select handler is
        // called
        //if( !bDontSelectAtCursor && !pImp->bSimpleTravel )
        //  pImp->SelectEntry( pNewCursor, sal_True );
        pImp->SetCursor( pNewCursor, bDontSelectAtCursor );
        return sal_True;
    }
    return sal_False;
}

sal_Bool ImpLBSelEng::IsSelectionAtPoint( const Point& rPoint )
{
    SvLBoxEntry* pEntry = pImp->MakePointVisible( rPoint );
    if( pEntry )
        return pView->IsSelected(pEntry);
    return sal_False;
}

void ImpLBSelEng::DeselectAtPoint( const Point& rPoint )
{
    SvLBoxEntry* pEntry = pImp->MakePointVisible( rPoint );
    if( !pEntry )
        return;
    pImp->SelectEntry( pEntry, sal_False );
}

void ImpLBSelEng::DeselectAll()
{
    pImp->SelAllDestrAnch( sal_False, sal_False ); // don't reset SelectionEngine!
    pImp->nFlags &= (~F_DESEL_ALL);
}

// ***********************************************************************
// Selection
// ***********************************************************************

void SvImpLBox::SetAnchorSelection(SvLBoxEntry* pOldCursor,SvLBoxEntry* pNewCursor)
{
    SvLBoxEntry* pEntry;
    sal_uLong nAnchorVisPos = pView->GetVisiblePos( pAnchor );
    sal_uLong nOldVisPos = pView->GetVisiblePos( pOldCursor );
    sal_uLong nNewVisPos = pView->GetVisiblePos( pNewCursor );

    if( nOldVisPos > nAnchorVisPos ||
        ( nAnchorVisPos==nOldVisPos && nNewVisPos > nAnchorVisPos) )
    {
        if( nNewVisPos > nOldVisPos )
        {
            pEntry = pOldCursor;
            while( pEntry && pEntry != pNewCursor )
            {
                pView->Select( pEntry, sal_True );
                pEntry = (SvLBoxEntry*)(pView->NextVisible( pEntry ));
            }
            if( pEntry )
                pView->Select( pEntry, sal_True );
            return;
        }

        if( nNewVisPos < nAnchorVisPos )
        {
            pEntry = pAnchor;
            while( pEntry && pEntry != pOldCursor )
            {
                pView->Select( pEntry, sal_False );
                pEntry = (SvLBoxEntry*)(pView->NextVisible( pEntry ));
            }
            if( pEntry )
                pView->Select( pEntry, sal_False );

            pEntry = pNewCursor;
            while( pEntry && pEntry != pAnchor )
            {
                pView->Select( pEntry, sal_True );
                pEntry = (SvLBoxEntry*)(pView->NextVisible( pEntry ));
            }
            if( pEntry )
                pView->Select( pEntry, sal_True );
            return;
        }

        if( nNewVisPos < nOldVisPos )
        {
            pEntry = pNewCursor;
            pEntry = (SvLBoxEntry*)(pView->NextVisible( pEntry ));
            while( pEntry && pEntry != pOldCursor )
            {
                pView->Select( pEntry, sal_False );
                pEntry = (SvLBoxEntry*)(pView->NextVisible( pEntry ));
            }
            if( pEntry )
                pView->Select( pEntry, sal_False );
            return;
        }
    }
    else
    {
        if( nNewVisPos < nOldVisPos )  // enlarge selection
        {
            pEntry = pNewCursor;
            while( pEntry && pEntry != pOldCursor )
            {
                pView->Select( pEntry, sal_True );
                pEntry = (SvLBoxEntry*)(pView->NextVisible( pEntry ));
            }
            if( pEntry )
                pView->Select( pEntry, sal_True );
            return;
        }

        if( nNewVisPos > nAnchorVisPos )
        {
            pEntry = pOldCursor;
            while( pEntry && pEntry != pAnchor )
            {
                pView->Select( pEntry, sal_False );
                pEntry = (SvLBoxEntry*)(pView->NextVisible( pEntry ));
            }
            if( pEntry )
                pView->Select( pEntry, sal_False );
            pEntry = pAnchor;
            while( pEntry && pEntry != pNewCursor )
            {
                pView->Select( pEntry, sal_True );
                pEntry = (SvLBoxEntry*)(pView->NextVisible( pEntry ));
            }
            if( pEntry )
                pView->Select( pEntry, sal_True );
            return;
        }

        if( nNewVisPos > nOldVisPos )
        {
            pEntry = pOldCursor;
            while( pEntry && pEntry != pNewCursor )
            {
                pView->Select( pEntry, sal_False );
                pEntry = (SvLBoxEntry*)(pView->NextVisible( pEntry ));
            }
            return;
        }
    }
}

void SvImpLBox::SelAllDestrAnch( sal_Bool bSelect, sal_Bool bDestroyAnchor,
    sal_Bool bSingleSelToo )
{
    SvLBoxEntry* pEntry;
    nFlags &= (~F_DESEL_ALL);
    if( bSelect && bSimpleTravel )
    {
        if( pCursor && !pView->IsSelected( pCursor ))
        {
            pView->Select( pCursor, sal_True );
        }
        return;
    }
    if( !bSelect && pView->GetSelectionCount() == 0 )
    {
        if( bSimpleTravel && ( !GetUpdateMode() || !pCursor) )
            nFlags |= F_DESEL_ALL;
        return;
    }
    if( bSelect && pView->GetSelectionCount() == pView->GetEntryCount())
        return;
    if( !bSingleSelToo && bSimpleTravel )
        return;

    if( !bSelect && pView->GetSelectionCount()==1 && pCursor &&
        pView->IsSelected( pCursor ))
    {
        pView->Select( pCursor, sal_False );
        if( bDestroyAnchor )
            DestroyAnchor(); // delete anchor & reset SelectionEngine
        else
            pAnchor = 0; // always delete internal anchor
        return;
    }

    if( bSimpleTravel && !pCursor && !GetUpdateMode() )
        nFlags |= F_DESEL_ALL;

    ShowCursor( sal_False );
    sal_Bool bUpdate = GetUpdateMode();

    nFlags |= F_IGNORE_SELECT; // EntryInserted should not do anything
    pEntry = pTree->First();
    while( pEntry )
    {
        if( pView->Select( pEntry, bSelect ) )
        {
            if( bUpdate && pView->IsEntryVisible(pEntry) )
            {
                long nY = GetEntryLine( pEntry );
                if( IsLineVisible( nY ) )
                    pView->PaintEntry1( pEntry, nY, 0xffff ); // because of ItemsetBrowser SV_LBOXTAB_SHOW_SELECTION );
            }
        }
        pEntry = pTree->Next( pEntry );
    }
    nFlags &= ~F_IGNORE_SELECT;

    if( bDestroyAnchor )
        DestroyAnchor(); // delete anchor & reset SelectionEngine
    else
        pAnchor = 0; // always delete internal anchor
    ShowCursor( sal_True );
}

void SvImpLBox::SetSelectionMode( SelectionMode eSelMode  )
{
    aSelEng.SetSelectionMode( eSelMode);
    if( eSelMode == SINGLE_SELECTION )
        bSimpleTravel = sal_True;
    else
        bSimpleTravel = sal_False;
    if( (m_nStyle & WB_SIMPLEMODE) && (eSelMode == MULTIPLE_SELECTION) )
        aSelEng.AddAlways( sal_True );
}

// ***********************************************************************
// Drag & Drop
// ***********************************************************************

void SvImpLBox::SetDragDropMode( DragDropMode eDDMode )
{
    if( eDDMode && eDDMode != SV_DRAGDROP_APP_DROP )
    {
        aSelEng.ExpandSelectionOnMouseMove( sal_False );
        aSelEng.EnableDrag( sal_True );
    }
    else
    {
        aSelEng.ExpandSelectionOnMouseMove( sal_True );
        aSelEng.EnableDrag( sal_False );
    }
}

void SvImpLBox::BeginDrag()
{
    nFlags &= (~F_FILLING);
    if( !bAsyncBeginDrag )
    {
        BeginScroll();
        pView->StartDrag( 0, aSelEng.GetMousePosPixel() );
        EndScroll();
    }
    else
    {
        aAsyncBeginDragPos = aSelEng.GetMousePosPixel();
        aAsyncBeginDragTimer.Start();
    }
}

IMPL_LINK_NOARG(SvImpLBox, BeginDragHdl)
{
    pView->StartDrag( 0, aAsyncBeginDragPos );
    return 0;
}

void SvImpLBox::PaintDDCursor( SvLBoxEntry* pInsertionPos )
{
    long nY;
    if( pInsertionPos )
    {
        nY = GetEntryLine( pInsertionPos );
        nY += pView->GetEntryHeight();
    }
    else
        nY = 1;
    RasterOp eOldOp = pView->GetRasterOp();
    pView->SetRasterOp( ROP_INVERT );
    Color aOldLineColor = pView->GetLineColor();
    pView->SetLineColor( Color( COL_BLACK ) );
    pView->DrawLine( Point( 0, nY ), Point( aOutputSize.Width(), nY ) );
    pView->SetLineColor( aOldLineColor );
    pView->SetRasterOp( eOldOp );
}

// Delete all submenus of a PopupMenu, recursively
void lcl_DeleteSubPopups(PopupMenu* pPopup)
{
    for(sal_uInt16 i = 0; i < pPopup->GetItemCount(); i++)
    {
        PopupMenu* pSubPopup = pPopup->GetPopupMenu( pPopup->GetItemId( i ));
        if(pSubPopup)
        {
            lcl_DeleteSubPopups(pSubPopup);
            delete pSubPopup;
        }
    }
}

void SvImpLBox::Command( const CommandEvent& rCEvt )
{
    sal_uInt16              nCommand = rCEvt.GetCommand();

    if( nCommand == COMMAND_CONTEXTMENU )
        aEditTimer.Stop();

    // scroll mouse event?
    if( ( ( nCommand == COMMAND_WHEEL ) || ( nCommand == COMMAND_STARTAUTOSCROLL ) || ( nCommand == COMMAND_AUTOSCROLL ) )
        && pView->HandleScrollCommand( rCEvt, &aHorSBar, &aVerSBar ) )
            return;

    if( bContextMenuHandling && nCommand == COMMAND_CONTEXTMENU )
    {
        Point   aPopupPos;
        sal_Bool    bClickedIsFreePlace = sal_False;
        std::stack<SvLBoxEntry*> aSelRestore;

        if( rCEvt.IsMouseEvent() )
        {   // change selection, if mouse position doesn't fit to selection

            aPopupPos = rCEvt.GetMousePosPixel();

            SvLBoxEntry*    pClickedEntry = GetEntry( aPopupPos );
            if( pClickedEntry )
            {   // mouse in non empty area
                sal_Bool                bClickedIsSelected = sal_False;

                // collect the currently selected entries
                SvLBoxEntry*        pSelected = pView->FirstSelected();
                while( pSelected )
                {
                    bClickedIsSelected |= ( pClickedEntry == pSelected );
                    pSelected = pView->NextSelected( pSelected );
                }

                // if the entry which the user clicked at is not selected
                if( !bClickedIsSelected )
                {   // deselect all other and select the clicked one
                    pView->SelectAll( sal_False );
                    pView->SetCursor( pClickedEntry );
                }
            }
            else if( aSelEng.GetSelectionMode() == SINGLE_SELECTION )
            {
                bClickedIsFreePlace = sal_True;
                sal_Int32               nSelectedEntries = pView->GetSelectionCount();
                SvLBoxEntry*        pSelected = pView->FirstSelected();
                for(sal_uInt16 nSel = 0; nSel < nSelectedEntries; nSel++ )
                {
                    aSelRestore.push(pSelected);
                    pSelected = pView->NextSelected( pSelected );
                }
                pView->SelectAll( sal_False );
            }
            else
            {   // deselect all
                pView->SelectAll( sal_False );
            }


        }
        else
        {   // key event (or at least no mouse event)
            sal_Int32   nSelectionCount = pView->GetSelectionCount();

            if( nSelectionCount )
            {   // now always take first visible as base for positioning the menu
                SvLBoxEntry*    pSelected = pView->FirstSelected();
                while( pSelected )
                {
                    if( IsEntryInView( pSelected ) )
                        break;

                    pSelected = pView->NextSelected( pSelected );
                }

                if( !pSelected )
                {
                    // no one was visible
                    pSelected = pView->FirstSelected();
                    pView->MakeVisible( pSelected );
                }

                aPopupPos = pView->GetFocusRect( pSelected, pView->GetEntryPosition( pSelected ).Y() ).Center();
            }
            else
                aPopupPos = Point( 0, 0 );
        }

        PopupMenu*  pPopup = pView->CreateContextMenu();

        if( pPopup )
        {
            // do action for selected entry in popup menu
            sal_uInt16 nMenuAction = pPopup->Execute( pView, aPopupPos );
            if ( nMenuAction )
                pView->ExcecuteContextMenuAction( nMenuAction );
            lcl_DeleteSubPopups(pPopup);
            delete pPopup;
        }

        if( bClickedIsFreePlace )
        {
            while(!aSelRestore.empty())
            {
                SvLBoxEntry* pEntry = aSelRestore.top();
                //#i19717# the entry is maybe already deleted
                bool bFound = false;
                for(sal_uLong nEntry = 0; nEntry < pView->GetEntryCount(); nEntry++)
                    if(pEntry == pView->GetEntry(nEntry))
                    {
                        bFound = true;
                        break;
                    }
                if(bFound)
                    SetCurEntry( pEntry );
                aSelRestore.pop();
            }
        }
    }
#ifndef NOCOMMAND
    else
    {
        const Point& rPos = rCEvt.GetMousePosPixel();
        if( rPos.X() < aOutputSize.Width() && rPos.Y() < aOutputSize.Height() )
            aSelEng.Command( rCEvt );
    }
#endif
}

void SvImpLBox::BeginScroll()
{
    if( !(nFlags & F_IN_SCROLLING))
    {
        pView->NotifyBeginScroll();
        nFlags |= F_IN_SCROLLING;
    }
}

void SvImpLBox::EndScroll()
{
    if( nFlags & F_IN_SCROLLING)
    {
        pView->NotifyEndScroll();
        nFlags &= (~F_IN_SCROLLING);
    }
}


Rectangle SvImpLBox::GetVisibleArea() const
{
    Point aPos( pView->GetMapMode().GetOrigin() );
    aPos.X() *= -1;
    Rectangle aRect( aPos, aOutputSize );
    return aRect;
}

void SvImpLBox::Invalidate()
{
    pView->SetClipRegion();
}

void SvImpLBox::SetCurEntry( SvLBoxEntry* pEntry )
{
    if  (  ( aSelEng.GetSelectionMode() != SINGLE_SELECTION )
        && ( aSelEng.GetSelectionMode() != NO_SELECTION )
        )
        SelAllDestrAnch( sal_False, sal_True, sal_False );
    if ( pEntry )
        MakeVisible( pEntry );
    SetCursor( pEntry );
    if ( pEntry && ( aSelEng.GetSelectionMode() != NO_SELECTION ) )
        pView->Select( pEntry, sal_True );
}

IMPL_LINK_NOARG(SvImpLBox, EditTimerCall)
{
    if( pView->IsInplaceEditingEnabled() )
    {
        sal_Bool bIsMouseTriggered = aEditClickPos.X() >= 0;
        if ( bIsMouseTriggered )
        {
            Point aCurrentMousePos = pView->GetPointerPosPixel();
            if  (   ( abs( aCurrentMousePos.X() - aEditClickPos.X() ) > 5 )
                ||  ( abs( aCurrentMousePos.Y() - aEditClickPos.Y() ) > 5 )
                )
            {
                return 0L;
            }
        }

        SvLBoxEntry* pEntry = GetCurEntry();
        if( pEntry )
        {
            ShowCursor( sal_False );
            pView->ImplEditEntry( pEntry );
            ShowCursor( sal_True );
        }
    }
    return 0;
}

sal_Bool SvImpLBox::RequestHelp( const HelpEvent& rHEvt )
{
    if( rHEvt.GetMode() & HELPMODE_QUICK )
    {
        Point aPos( pView->ScreenToOutputPixel( rHEvt.GetMousePosPixel() ));
        if( !GetVisibleArea().IsInside( aPos ))
            return sal_False;

        SvLBoxEntry* pEntry = GetEntry( aPos );
        if( pEntry )
        {
            // recalculate text rectangle
            SvLBoxTab* pTab;
            SvLBoxString* pItem = (SvLBoxString*)(pView->GetItem( pEntry, aPos.X(), &pTab ));
            if( !pItem || pItem->IsA() != SV_ITEM_ID_LBOXSTRING )
                return sal_False;

            aPos = GetEntryPosition( pEntry );
            aPos.X() = pView->GetTabPos( pEntry, pTab ); //pTab->GetPos();
            Size aSize( pItem->GetSize( pView, pEntry ) );
            SvLBoxTab* pNextTab = NextTab( pTab );
            sal_Bool bItemClipped = sal_False;
            // is the item cut off by its right neighbor?
            if( pNextTab && pView->GetTabPos(pEntry,pNextTab) < aPos.X()+aSize.Width() )
            {
                aSize.Width() = pNextTab->GetPos() - pTab->GetPos();
                bItemClipped = sal_True;
            }
            Rectangle aItemRect( aPos, aSize );

            Rectangle aViewRect( GetVisibleArea() );

            if( bItemClipped || !aViewRect.IsInside( aItemRect ) )
            {
                // clip the right edge of the item at the edge of the view
                //if( aItemRect.Right() > aViewRect.Right() )
                //  aItemRect.Right() = aViewRect.Right();

                Point aPt = pView->OutputToScreenPixel( aItemRect.TopLeft() );
                aItemRect.Left()   = aPt.X();
                aItemRect.Top()    = aPt.Y();
                aPt = pView->OutputToScreenPixel( aItemRect.BottomRight() );
                aItemRect.Right()  = aPt.X();
                aItemRect.Bottom() = aPt.Y();

                Help::ShowQuickHelp( pView, aItemRect,
                                     pItem->GetText(), QUICKHELP_LEFT | QUICKHELP_VCENTER );
                return sal_True;
            }
        }
    }
    return sal_False;
}

SvLBoxTab* SvImpLBox::NextTab( SvLBoxTab* pTab )
{
    sal_uInt16 nTabCount = pView->TabCount();
    if( nTabCount <= 1 )
        return 0;
    for( sal_uInt16 nTab=0; nTab < (nTabCount-1); nTab++)
    {
        if( pView->aTabs[nTab]==pTab )
            return (SvLBoxTab*)(pView->aTabs[nTab+1]);
    }
    return 0;
}

void SvImpLBox::EndSelection()
{
    DestroyAnchor();
    nFlags &=  ~F_START_EDITTIMER;
}

void SvImpLBox::RepaintScrollBars()
{
}

void SvImpLBox::SetUpdateMode( sal_Bool bMode )
{
    if( bUpdateMode != bMode )
    {
        bUpdateMode = bMode;
        if( bUpdateMode )
            UpdateAll( sal_False );
    }
}

sal_Bool SvImpLBox::SetMostRight( SvLBoxEntry* pEntry )
{
    if( pView->nTreeFlags & TREEFLAG_RECALCTABS )
    {
        nFlags |= F_IGNORE_CHANGED_TABS;
        pView->SetTabs();
        nFlags &= ~F_IGNORE_CHANGED_TABS;
    }

    sal_uInt16 nLastTab = pView->aTabs.size() - 1;
    sal_uInt16 nLastItem = pEntry->ItemCount() - 1;
    if( !pView->aTabs.empty() && nLastItem != USHRT_MAX )
    {
        if( nLastItem < nLastTab )
            nLastTab = nLastItem;

        SvLBoxTab* pTab = pView->aTabs[ nLastTab ];
        SvLBoxItem* pItem = pEntry->GetItem( nLastTab );

        long nTabPos = pView->GetTabPos( pEntry, pTab );

        long nMaxRight = GetOutputSize().Width();
        Point aPos( pView->GetMapMode().GetOrigin() );
        aPos.X() *= -1; // conversion document coordinates
        nMaxRight = nMaxRight + aPos.X() - 1;

        long nNextTab = nTabPos < nMaxRight ? nMaxRight : nMaxRight + 50;
        long nTabWidth = nNextTab - nTabPos + 1;
        long nItemSize = pItem->GetSize(pView,pEntry).Width();
        long nOffset = pTab->CalcOffset( nItemSize, nTabWidth );

        long nRight = nTabPos + nOffset + nItemSize;
        if( nRight > nMostRight )
        {
            nMostRight = nRight;
            pMostRightEntry = pEntry;
            return sal_True;
        }
    }
    return sal_False;
}

void SvImpLBox::FindMostRight( SvLBoxEntry* pEntryToIgnore )
{
    nMostRight = -1;
    pMostRightEntry = 0;
    if( !pView->GetModel() )
        return;

    SvLBoxEntry* pEntry = (SvLBoxEntry*)pView->FirstVisible();
    while( pEntry )
    {
        if( pEntry != pEntryToIgnore )
            SetMostRight( pEntry );
        pEntry = (SvLBoxEntry*)pView->NextVisible( pEntry );
    }
}

void SvImpLBox::FindMostRight( SvLBoxEntry* pParent, SvLBoxEntry* pEntryToIgnore )
{
    if( !pParent )
        FindMostRight( pEntryToIgnore );
    else
        FindMostRight_Impl( pParent, pEntryToIgnore  );
}

void SvImpLBox::FindMostRight_Impl( SvLBoxEntry* pParent, SvLBoxEntry* pEntryToIgnore )
{
    SvTreeEntryList* pList = pTree->GetChildList( pParent );

    if( !pList )
        return;

    size_t nCount = pList->size();
    for( size_t nCur = 0; nCur < nCount; nCur++ )
    {
        SvLBoxEntry* pChild = (SvLBoxEntry*)(*pList)[ nCur ];
        if( pChild != pEntryToIgnore )
        {
            SetMostRight( pChild );
            if( pChild->HasChildren() && pView->IsExpanded( pChild ))
                FindMostRight_Impl( pChild, pEntryToIgnore );
        }
    }
}

void SvImpLBox::NotifyTabsChanged()
{
    if( GetUpdateMode() && !(nFlags & F_IGNORE_CHANGED_TABS ) &&
        nCurUserEvent == 0xffffffff )
    {
        nCurUserEvent = Application::PostUserEvent(LINK(this,SvImpLBox,MyUserEvent),(void*)0);
    }
}

IMPL_LINK(SvImpLBox,MyUserEvent,void*, pArg )
{
    nCurUserEvent = 0xffffffff;
    if( !pArg )
    {
        pView->Invalidate();
        pView->Update();
    }
    else
    {
        FindMostRight( 0 );
        ShowVerSBar();
        pView->Invalidate( GetVisibleArea() );
    }
    return 0;
}


void SvImpLBox::StopUserEvent()
{
    if( nCurUserEvent != 0xffffffff )
    {
        Application::RemoveUserEvent( nCurUserEvent );
        nCurUserEvent = 0xffffffff;
    }
}

void SvImpLBox::ShowFocusRect( const SvLBoxEntry* pEntry )
{
    if( pEntry )
    {
        long nY = GetEntryLine( (SvLBoxEntry*)pEntry );
        Rectangle aRect = pView->GetFocusRect( (SvLBoxEntry*)pEntry, nY );
        Region aOldClip( pView->GetClipRegion());
        Region aClipRegion( GetClipRegionRect() );
        pView->SetClipRegion( aClipRegion );
        pView->ShowFocus( aRect );
        pView->SetClipRegion( aOldClip );

    }
    else
    {
        pView->HideFocus();
    }
}

// -----------------------------------------------------------------------
void SvImpLBox::implInitDefaultNodeImages()
{
    if ( s_pDefCollapsed )
        // assume that all or nothing is initialized
        return;

    s_pDefCollapsed  = new Image( SvtResId( RID_IMG_TREENODE_COLLAPSED ) );
    s_pDefExpanded   = new Image( SvtResId( RID_IMG_TREENODE_EXPANDED ) );
}

// -----------------------------------------------------------------------
const Image& SvImpLBox::GetDefaultExpandedNodeImage( )
{
    implInitDefaultNodeImages();
    return *s_pDefExpanded;
}

// -----------------------------------------------------------------------
const Image& SvImpLBox::GetDefaultCollapsedNodeImage( )
{
    implInitDefaultNodeImages();
    return *s_pDefCollapsed;
}

// -----------------------------------------------------------------------
void SvImpLBox::CallEventListeners( sal_uLong nEvent, void* pData )
{
    if ( pView )
        pView->CallImplEventListeners( nEvent, pData);
}

// -----------------------------------------------------------------------

bool SvImpLBox::SetCurrentTabPos( sal_uInt16 _nNewPos )
{
    bool bRet = false;

    if ( pView && _nNewPos < ( pView->TabCount() - 2 ) )
    {
        nCurTabPos = _nNewPos;
        ShowCursor( sal_True );
        bRet = true;
    }

    return bRet;
}

// -----------------------------------------------------------------------

bool SvImpLBox::IsSelectable( const SvLBoxEntry* pEntry )
{
    if( pEntry )
    {
        SvViewDataEntry* pViewDataNewCur = pView->GetViewDataEntry(const_cast<SvLBoxEntry*>(pEntry));
        return (pViewDataNewCur == 0) || pViewDataNewCur->IsSelectable();
    }
    else
    {
        return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
