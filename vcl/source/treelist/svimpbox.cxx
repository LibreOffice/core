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

#include <sal/config.h>

#include <o3tl/safeint.hxx>
#include <vcl/svapp.hxx>
#include <vcl/salnativewidgets.hxx>
#include <vcl/help.hxx>
#include <vcl/settings.hxx>
#include <vcl/commandevent.hxx>

#include <cstdlib>
#include <memory>
#include <stack>

#include <vcl/toolkit/treelistbox.hxx>
#include <vcl/toolkit/svlbitm.hxx>
#include <tools/wintypes.hxx>
#include <bitmaps.hlst>
#include <svimpbox.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <tools/debug.hxx>

#include <vcl/toolkit/treelistentry.hxx>
#include <vcl/toolkit/viewdataentry.hxx>

// #i27063# (pl), #i32300# (pb) never access VCL after DeInitVCL - also no destructors
Image*  SvImpLBox::s_pDefCollapsed      = nullptr;
Image*  SvImpLBox::s_pDefExpanded       = nullptr;
oslInterlockedCount SvImpLBox::s_nImageRefCount   = 0;

SvImpLBox::SvImpLBox( SvTreeListBox* pLBView, SvTreeList* pLBTree, WinBits nWinStyle)
    : m_aHorSBar(VclPtr<ScrollBar>::Create(pLBView, WB_DRAG | WB_HSCROLL))
    , m_aScrBarBox(VclPtr<ScrollBarBox>::Create(pLBView))
    , m_aFctSet(this, pLBView)
    , mbForceMakeVisible (false)
    , m_aEditIdle("SvImpLBox m_aEditIdle")
    , m_aVerSBar(VclPtr<ScrollBar>::Create(pLBView, WB_DRAG | WB_VSCROLL))
    , m_aOutputSize(0, 0)
    , mbNoAutoCurEntry(false)
    , m_aSelEng(pLBView, nullptr)
    , m_nNextVerVisSize(0)
{
    osl_atomic_increment(&s_nImageRefCount);
    m_pView = pLBView;
    m_pTree = pLBTree;
    m_aSelEng.SetFunctionSet( static_cast<FunctionSet*>(&m_aFctSet) );
    m_aSelEng.ExpandSelectionOnMouseMove( false );
    SetStyle( nWinStyle );
    SetSelectionMode( SelectionMode::Single );
    SetDragDropMode( DragDropMode::NONE );

    m_aVerSBar->SetScrollHdl( LINK( this, SvImpLBox, ScrollUpDownHdl ) );
    m_aHorSBar->SetScrollHdl( LINK( this, SvImpLBox, ScrollLeftRightHdl ) );
    m_aHorSBar->SetEndScrollHdl( LINK( this, SvImpLBox, EndScrollHdl ) );
    m_aVerSBar->SetEndScrollHdl( LINK( this, SvImpLBox, EndScrollHdl ) );
    m_aVerSBar->SetRange( Range(0,0) );
    m_aVerSBar->Hide();
    m_aHorSBar->SetRange( Range(0,0) );
    m_aHorSBar->SetPageSize( 24 ); // pixels
    m_aHorSBar->SetLineSize( 8 ); // pixels

    m_nHorSBarHeight = static_cast<short>(m_aHorSBar->GetSizePixel().Height());
    m_nVerSBarWidth = static_cast<short>(m_aVerSBar->GetSizePixel().Width());

    m_pStartEntry = nullptr;
    m_pCursor             = nullptr;
    m_pCursorOld          = nullptr;
    m_pAnchor             = nullptr;
    m_nVisibleCount       = 0;    // number of rows of data in control
    m_nNodeBmpTabDistance = NODE_BMP_TABDIST_NOTVALID;
    m_nNodeBmpWidth       = 0;

    // button animation in listbox
    m_pActiveButton = nullptr;
    m_pActiveEntry = nullptr;
    m_pActiveTab = nullptr;

    m_nFlags = LBoxFlags::NONE;

    m_aEditIdle.SetPriority( TaskPriority::LOWEST );
    m_aEditIdle.SetInvokeHandler( LINK(this,SvImpLBox,EditTimerCall) );

    m_nMostRight = -1;
    m_pMostRightEntry = nullptr;
    m_nCurUserEvent = nullptr;

    m_bUpdateMode = true;
    m_bInVScrollHdl = false;
    m_nFlags |= LBoxFlags::Filling;

    m_bSubLstOpLR = false;
}

SvImpLBox::~SvImpLBox()
{
    m_aEditIdle.Stop();
    StopUserEvent();

    if ( osl_atomic_decrement(&s_nImageRefCount) == 0 )
    {
        delete s_pDefCollapsed;
        s_pDefCollapsed = nullptr;
        delete s_pDefExpanded;
        s_pDefExpanded = nullptr;
    }
    m_aVerSBar.disposeAndClear();
    m_aHorSBar.disposeAndClear();
    m_aScrBarBox.disposeAndClear();
}

void SvImpLBox::UpdateStringSorter()
{
    const css::lang::Locale& rNewLocale = Application::GetSettings().GetLanguageTag().getLocale();

    if( m_pStringSorter )
    {
        // different Locale from the older one, drop it and force recreate
        const css::lang::Locale &aLocale = m_pStringSorter->getLocale();
        if( aLocale.Language != rNewLocale.Language ||
            aLocale.Country != rNewLocale.Country ||
            aLocale.Variant != rNewLocale.Variant )
            m_pStringSorter.reset();
    }

    if( !m_pStringSorter )
    {
        m_pStringSorter.reset(new comphelper::string::NaturalStringSorter(
                              ::comphelper::getProcessComponentContext(),
                              rNewLocale));
    }
}

short SvImpLBox::UpdateContextBmpWidthVector( SvTreeListEntry const * pEntry, short nWidth )
{
    DBG_ASSERT( m_pView->pModel, "View and Model aren't valid!" );

    sal_uInt16 nDepth = m_pView->pModel->GetDepth( pEntry );
    // initialize vector if necessary
    std::vector< short >::size_type nSize = m_aContextBmpWidthVector.size();
    while ( nDepth > nSize )
    {
        m_aContextBmpWidthVector.resize( nSize + 1 );
        m_aContextBmpWidthVector.at( nSize ) = nWidth;
        ++nSize;
    }
    if( m_aContextBmpWidthVector.size() == nDepth )
    {
        m_aContextBmpWidthVector.resize( nDepth + 1 );
        m_aContextBmpWidthVector.at( nDepth ) = 0;
    }
    short nContextBmpWidth = m_aContextBmpWidthVector[ nDepth ];
    if( nContextBmpWidth < nWidth )
    {
        m_aContextBmpWidthVector.at( nDepth ) = nWidth;
        return nWidth;
    }
    else
        return nContextBmpWidth;
}

void SvImpLBox::UpdateContextBmpWidthVectorFromMovedEntry( SvTreeListEntry* pEntry )
{
    DBG_ASSERT( pEntry, "Moved Entry is invalid!" );

    SvLBoxContextBmp* pBmpItem = static_cast< SvLBoxContextBmp* >( pEntry->GetFirstItem(SvLBoxItemType::ContextBmp) );
    short nExpWidth = static_cast<short>(pBmpItem->GetBitmap1().GetSizePixel().Width());
    short nColWidth = static_cast<short>(pBmpItem->GetBitmap2().GetSizePixel().Width());
    short nMax = std::max(nExpWidth, nColWidth);
    UpdateContextBmpWidthVector( pEntry, nMax );

    if( pEntry->HasChildren() ) // recursive call, whether expanded or not
    {
        SvTreeListEntry* pChild = m_pView->FirstChild( pEntry );
        DBG_ASSERT( pChild, "The first child is invalid!" );
        do
        {
            UpdateContextBmpWidthVectorFromMovedEntry( pChild );
            pChild = m_pView->Next( pChild );
        } while ( pChild );
    }
}

void SvImpLBox::UpdateContextBmpWidthMax( SvTreeListEntry const * pEntry )
{
    sal_uInt16 nDepth = m_pView->pModel->GetDepth( pEntry );
    if( m_aContextBmpWidthVector.empty() )
        return;
    short nWidth = m_aContextBmpWidthVector[ nDepth ];
    if( nWidth != m_pView->nContextBmpWidthMax ) {
        m_pView->nContextBmpWidthMax = nWidth;
        m_nFlags |= LBoxFlags::IgnoreChangedTabs;
        m_pView->SetTabs();
        m_nFlags &= ~LBoxFlags::IgnoreChangedTabs;
    }
}

void SvImpLBox::SetStyle( WinBits i_nWinStyle )
{
    m_nStyle = i_nWinStyle;
    if ( ( m_nStyle & WB_SIMPLEMODE) && ( m_aSelEng.GetSelectionMode() == SelectionMode::Multiple ) )
        m_aSelEng.AddAlways( true );
}

void SvImpLBox::SetNoAutoCurEntry( bool b )
{
    mbNoAutoCurEntry = b;
}

// don't touch the model any more
void SvImpLBox::Clear()
{
    StopUserEvent();
    m_pStartEntry = nullptr;
    m_pAnchor = nullptr;

    m_pActiveButton = nullptr;
    m_pActiveEntry = nullptr;
    m_pActiveTab = nullptr;

    m_nMostRight = -1;
    m_pMostRightEntry = nullptr;

    // don't touch the cursor any more
    if( m_pCursor )
    {
        if( m_pView->HasFocus() )
            m_pView->HideFocus();
        m_pCursor = nullptr;
    }
    m_pCursorOld = nullptr;
    m_aVerSBar->Hide();
    m_aVerSBar->SetThumbPos( 0 );
    Range aRange( 0, 0 );
    m_aVerSBar->SetRange( aRange );
    m_aOutputSize = m_pView->Control::GetOutputSizePixel();
    m_aHorSBar->Hide();
    m_aHorSBar->SetThumbPos( 0 );
    MapMode aMapMode( m_pView->GetMapMode());
    aMapMode.SetOrigin( Point(0,0) );
    m_pView->Control::SetMapMode( aMapMode );
    m_aHorSBar->SetRange( aRange );
    m_aHorSBar->SetSizePixel(Size(m_aOutputSize.Width(),m_nHorSBarHeight));
    m_pView->GetOutDev()->SetClipRegion();
    if( GetUpdateMode() )
        m_pView->Invalidate( GetVisibleArea() );
    m_nFlags |= LBoxFlags::Filling;
    if( !m_aHorSBar->IsVisible() && !m_aVerSBar->IsVisible() )
        m_aScrBarBox->Hide();

    m_aContextBmpWidthVector.clear();

    CallEventListeners( VclEventId::ListboxItemRemoved );
}

// *********************************************************************
// Paint, navigate, scroll
// *********************************************************************

IMPL_LINK_NOARG(SvImpLBox, EndScrollHdl, ScrollBar*, void)
{
    if( m_nFlags & LBoxFlags::EndScrollSetVisSize )
    {
        m_aVerSBar->SetVisibleSize( m_nNextVerVisSize );
        m_nFlags &= ~LBoxFlags::EndScrollSetVisSize;
    }
}

// handler for vertical scrollbar

IMPL_LINK( SvImpLBox, ScrollUpDownHdl, ScrollBar *, pScrollBar, void )
{
    DBG_ASSERT(!m_bInVScrollHdl,"Scroll handler out-paces itself!");
    tools::Long nDelta = pScrollBar->GetDelta();
    if( !nDelta )
        return;

    // when only one row don't skip lines
    if (pScrollBar->GetPageSize() == 1)
        nDelta = nDelta > 0 ? 1 : -1;

    m_nFlags &= ~LBoxFlags::Filling;

    m_bInVScrollHdl = true;

    if( m_pView->IsEditingActive() )
    {
        m_pView->EndEditing( true ); // Cancel
        m_pView->PaintImmediately();
    }

    if( nDelta > 0 )
    {
        if( nDelta == 1 && pScrollBar->GetPageSize() > 1)
            CursorDown();
        else
            PageDown( static_cast<sal_uInt16>(nDelta) );
    }
    else
    {
        nDelta *= -1;
        if( nDelta == 1 && pScrollBar->GetPageSize() > 1)
            CursorUp();
        else
            PageUp( static_cast<sal_uInt16>(nDelta) );
    }
    m_bInVScrollHdl = false;
}


void SvImpLBox::CursorDown()
{
    if (!m_pStartEntry)
        return;

    SvTreeListEntry* pNextFirstToDraw = m_pView->NextVisible(m_pStartEntry);
    if( pNextFirstToDraw )
    {
        m_nFlags &= ~LBoxFlags::Filling;
        ShowCursor( false );
        m_pView->PaintImmediately();
        m_pStartEntry = pNextFirstToDraw;
        tools::Rectangle aArea( GetVisibleArea() );
        m_pView->Scroll( 0, -(m_pView->GetEntryHeight()), aArea, ScrollFlags::NoChildren );
        m_pView->PaintImmediately();
        ShowCursor( true );
        m_pView->NotifyScrolled();
    }
}

void SvImpLBox::CursorUp()
{
    if (!m_pStartEntry)
        return;

    SvTreeListEntry* pPrevFirstToDraw = m_pView->PrevVisible(m_pStartEntry);
    if( !pPrevFirstToDraw )
        return;

    m_nFlags &= ~LBoxFlags::Filling;
    tools::Long nEntryHeight = m_pView->GetEntryHeight();
    ShowCursor( false );
    m_pView->PaintImmediately();
    m_pStartEntry = pPrevFirstToDraw;
    tools::Rectangle aArea( GetVisibleArea() );
    if (aArea.GetHeight() > nEntryHeight)
        aArea.AdjustBottom(-nEntryHeight);
    m_pView->Scroll( 0, nEntryHeight, aArea, ScrollFlags::NoChildren );
    m_pView->PaintImmediately();
    ShowCursor( true );
    m_pView->NotifyScrolled();
}

void SvImpLBox::PageDown( sal_uInt16 nDelta )
{
    sal_uInt16 nRealDelta = nDelta;

    if( !nDelta )
        return;

    if (!m_pStartEntry)
        return;

    SvTreeListEntry* pNext = m_pView->NextVisible(m_pStartEntry, nRealDelta);
    if( pNext == m_pStartEntry )
        return;

    ShowCursor( false );

    m_nFlags &= ~LBoxFlags::Filling;
    m_pStartEntry = pNext;

    if( nRealDelta >= m_nVisibleCount )
    {
        m_pView->Invalidate( GetVisibleArea() );
        m_pView->PaintImmediately();
    }
    else
    {
        tools::Rectangle aArea( GetVisibleArea() );
        tools::Long nScroll = m_pView->GetEntryHeight() * static_cast<tools::Long>(nRealDelta);
        nScroll = -nScroll;
        m_pView->PaintImmediately();
        m_pView->Scroll( 0, nScroll, aArea, ScrollFlags::NoChildren );
        m_pView->PaintImmediately();
        m_pView->NotifyScrolled();
    }

    ShowCursor( true );
}

void SvImpLBox::PageUp( sal_uInt16 nDelta )
{
    sal_uInt16 nRealDelta = nDelta;
    if( !nDelta )
        return;

    if (!m_pStartEntry)
        return;

    SvTreeListEntry* pPrev = m_pView->PrevVisible(m_pStartEntry, nRealDelta);
    if( pPrev == m_pStartEntry )
        return;

    m_nFlags &= ~LBoxFlags::Filling;
    ShowCursor( false );

    m_pStartEntry = pPrev;
    if( nRealDelta >= m_nVisibleCount )
    {
        m_pView->Invalidate( GetVisibleArea() );
        m_pView->PaintImmediately();
    }
    else
    {
        tools::Long nEntryHeight = m_pView->GetEntryHeight();
        tools::Rectangle aArea( GetVisibleArea() );
        m_pView->PaintImmediately();
        m_pView->Scroll( 0, nEntryHeight*nRealDelta, aArea, ScrollFlags::NoChildren );
        m_pView->PaintImmediately();
        m_pView->NotifyScrolled();
    }

    ShowCursor( true );
}

void SvImpLBox::KeyUp( bool bPageUp )
{
    if( !m_aVerSBar->IsVisible() )
        return;

    tools::Long nDelta;
    if( bPageUp )
        nDelta = m_aVerSBar->GetPageSize();
    else
        nDelta = 1;

    tools::Long nThumbPos = m_aVerSBar->GetThumbPos();

    if( nThumbPos < nDelta )
        nDelta = nThumbPos;

    if( nDelta <= 0 )
        return;

    m_nFlags &= ~LBoxFlags::Filling;

    m_aVerSBar->SetThumbPos( nThumbPos - nDelta );
    if( bPageUp )
        PageUp( static_cast<short>(nDelta) );
    else
        CursorUp();
}


void SvImpLBox::KeyDown( bool bPageDown )
{
    if( !m_aVerSBar->IsVisible() )
        return;

    tools::Long nDelta;
    if( bPageDown )
        nDelta = m_aVerSBar->GetPageSize();
    else
        nDelta = 1;

    tools::Long nThumbPos = m_aVerSBar->GetThumbPos();
    tools::Long nVisibleSize = m_aVerSBar->GetVisibleSize();
    tools::Long nRange = m_aVerSBar->GetRange().Len();

    tools::Long nTmp = nThumbPos+nVisibleSize;
    while( (nDelta > 0) && (nTmp+nDelta) >= nRange )
        nDelta--;

    if( nDelta <= 0 )
        return;

    m_nFlags &= ~LBoxFlags::Filling;

    m_aVerSBar->SetThumbPos( nThumbPos+nDelta );
    if( bPageDown )
        PageDown( static_cast<short>(nDelta) );
    else
        CursorDown();
}


void SvImpLBox::InvalidateEntriesFrom( tools::Long nY ) const
{
    if( !(m_nFlags & LBoxFlags::InPaint ))
    {
        tools::Rectangle aRect( GetVisibleArea() );
        aRect.SetTop( nY );
        m_pView->Invalidate( aRect );
    }
}

void SvImpLBox::InvalidateEntry( tools::Long nY ) const
{
    if( m_nFlags & LBoxFlags::InPaint )
        return;

    tools::Rectangle aRect( GetVisibleArea() );
    tools::Long nMaxBottom = aRect.Bottom();
    aRect.SetTop( nY );
    aRect.SetBottom( nY ); aRect.AdjustBottom(m_pView->GetEntryHeight() );
    if( aRect.Top() > nMaxBottom )
        return;
    if( aRect.Bottom() > nMaxBottom )
        aRect.SetBottom( nMaxBottom );
    if (m_pView->SupportsDoubleBuffering())
        // Perform full paint when flicker is to be avoided explicitly.
        m_pView->Invalidate();
    else
        m_pView->Invalidate(aRect);
}

void SvImpLBox::InvalidateEntry( SvTreeListEntry* pEntry )
{
    if( GetUpdateMode() )
    {
        tools::Long nPrev = m_nMostRight;
        SetMostRight( pEntry );
        if( nPrev < m_nMostRight )
            ShowVerSBar();
    }
    if( !(m_nFlags & LBoxFlags::InPaint ))
    {
        bool bHasFocusRect = false;
        if( pEntry==m_pCursor && m_pView->HasFocus() )
        {
            bHasFocusRect = true;
            ShowCursor( false );
        }
        InvalidateEntry( GetEntryLine( pEntry ) );
        if( bHasFocusRect )
            ShowCursor( true );
    }
}


void SvImpLBox::RecalcFocusRect()
{
    if( m_pView->HasFocus() && m_pCursor )
    {
        m_pView->HideFocus();
        tools::Long nY = GetEntryLine( m_pCursor );
        tools::Rectangle aRect = m_pView->GetFocusRect( m_pCursor, nY );
        vcl::Region aOldClip( m_pView->GetOutDev()->GetClipRegion());
        vcl::Region aClipRegion( GetClipRegionRect() );
        m_pView->GetOutDev()->SetClipRegion( aClipRegion );
        m_pView->ShowFocus( aRect );
        m_pView->GetOutDev()->SetClipRegion( aOldClip );
    }
}


//  Sets cursor. When using SingleSelection, the selection is adjusted.
void SvImpLBox::SetCursor( SvTreeListEntry* pEntry, bool bForceNoSelect )
{
    SvViewDataEntry* pViewDataNewCur = nullptr;
    if( pEntry )
        pViewDataNewCur= m_pView->GetViewDataEntry(pEntry);
    if( pEntry &&
        pEntry == m_pCursor &&
        pViewDataNewCur &&
        pViewDataNewCur->HasFocus() &&
        pViewDataNewCur->IsSelected())
    {
        return;
    }

    // if this cursor is not selectable, find first visible that is and use it
    while( pEntry && pViewDataNewCur && !pViewDataNewCur->IsSelectable() )
    {
        pEntry = m_pView->NextVisible(pEntry);
        pViewDataNewCur = pEntry ? m_pView->GetViewDataEntry(pEntry) : nullptr;
    }

    SvTreeListEntry* pOldCursor = m_pCursor;
    if( m_pCursor && pEntry != m_pCursor )
    {
        m_pView->SetEntryFocus( m_pCursor, false );
        if( m_bSimpleTravel )
            m_pView->Select( m_pCursor, false );
        m_pView->HideFocus();
    }
    m_pCursor = pEntry;
    if( m_pCursor )
    {
        if (pViewDataNewCur)
            pViewDataNewCur->SetFocus( true );
        if(!bForceNoSelect && m_bSimpleTravel && !(m_nFlags & LBoxFlags::DeselectAll) && GetUpdateMode())
        {
            m_pView->Select( m_pCursor );
            CallEventListeners( VclEventId::ListboxTreeFocus, m_pCursor );
        }
        // multiple selection: select in cursor move if we're not in
        // Add mode (Ctrl-F8)
        else if( GetUpdateMode() &&
                 m_pView->GetSelectionMode() == SelectionMode::Multiple &&
                 !(m_nFlags & LBoxFlags::DeselectAll) && !m_aSelEng.IsAddMode() &&
                 !bForceNoSelect )
        {
            m_pView->Select( m_pCursor );
            CallEventListeners( VclEventId::ListboxTreeFocus, m_pCursor );
        }
        else
        {
            ShowCursor( true );
            if (bForceNoSelect && GetUpdateMode())
            {
                CallEventListeners( VclEventId::ListboxTreeFocus, m_pCursor);
            }
        }

        if( m_pAnchor )
        {
            DBG_ASSERT(m_aSelEng.GetSelectionMode() != SelectionMode::Single,"Mode?");
            SetAnchorSelection( pOldCursor, m_pCursor );
        }
    }
    m_nFlags &= ~LBoxFlags::DeselectAll;

    m_pView->OnCurrentEntryChanged();
}

void SvImpLBox::ShowCursor( bool bShow )
{
    if( !bShow || !m_pCursor || !m_pView->HasFocus() )
    {
        vcl::Region aOldClip( m_pView->GetOutDev()->GetClipRegion());
        vcl::Region aClipRegion( GetClipRegionRect() );
        m_pView->GetOutDev()->SetClipRegion( aClipRegion );
        m_pView->HideFocus();
        m_pView->GetOutDev()->SetClipRegion( aOldClip );
    }
    else
    {
        tools::Long nY = GetEntryLine( m_pCursor );
        tools::Rectangle aRect = m_pView->GetFocusRect( m_pCursor, nY );
        vcl::Region aOldClip( m_pView->GetOutDev()->GetClipRegion());
        vcl::Region aClipRegion( GetClipRegionRect() );
        m_pView->GetOutDev()->SetClipRegion( aClipRegion );
        m_pView->ShowFocus( aRect );
        m_pView->GetOutDev()->SetClipRegion( aOldClip );
    }
}


void SvImpLBox::UpdateAll( bool bInvalidateCompleteView )
{
    FindMostRight();
    m_aVerSBar->SetRange( Range(0, m_pView->GetVisibleCount()-1 ) );
    SyncVerThumb();
    FillView();
    ShowVerSBar();
    if( m_bSimpleTravel && m_pCursor && m_pView->HasFocus() )
        m_pView->Select( m_pCursor );
    ShowCursor( true );
    if( bInvalidateCompleteView )
        m_pView->Invalidate();
    else
        m_pView->Invalidate( GetVisibleArea() );
}

IMPL_LINK( SvImpLBox, ScrollLeftRightHdl, ScrollBar *, pScrollBar, void )
{
    tools::Long nDelta = pScrollBar->GetDelta();
    if( nDelta )
    {
        if( m_pView->IsEditingActive() )
        {
            m_pView->EndEditing( true ); // Cancel
            m_pView->PaintImmediately();
        }
        m_pView->nFocusWidth = -1;
        KeyLeftRight( nDelta );
    }
}

void SvImpLBox::KeyLeftRight( tools::Long nDelta )
{
    if( !(m_nFlags & LBoxFlags::InResize) )
        m_pView->PaintImmediately();
    m_nFlags &= ~LBoxFlags::Filling;
    ShowCursor( false );

    // calculate new origin
    tools::Long nPos = m_aHorSBar->GetThumbPos();
    Point aOrigin( -nPos, 0 );

    MapMode aMapMode( m_pView->GetMapMode() );
    aMapMode.SetOrigin( aOrigin );
    m_pView->SetMapMode( aMapMode );

    if( !(m_nFlags & LBoxFlags::InResize) )
    {
        tools::Rectangle aRect( GetVisibleArea() );
        m_pView->Scroll( -nDelta, 0, aRect, ScrollFlags::NoChildren );
    }
    else
        m_pView->Invalidate();
    RecalcFocusRect();
    ShowCursor( true );
    m_pView->NotifyScrolled();
}


// returns the last entry if position is just past the last entry
SvTreeListEntry* SvImpLBox::GetClickedEntry( const Point& rPoint ) const
{
    DBG_ASSERT( m_pView->GetModel(), "SvImpLBox::GetClickedEntry: how can this ever happen? Please tell me (frank.schoenheit@sun.com) how to reproduce!" );
    if ( !m_pView->GetModel() )
        // this is quite impossible. Nevertheless, stack traces from the crash reporter
        // suggest it isn't. Okay, make it safe, and wait for somebody to reproduce it
        // reliably :-\ ...
        // #122359# / 2005-05-23 / frank.schoenheit@sun.com
        return nullptr;
    if( m_pView->GetEntryCount() == 0 || !m_pStartEntry || !m_pView->GetEntryHeight())
        return nullptr;

    sal_uInt16 nClickedEntry = static_cast<sal_uInt16>(rPoint.Y() / m_pView->GetEntryHeight() );
    sal_uInt16 nTemp = nClickedEntry;
    SvTreeListEntry* pEntry = m_pView->NextVisible(m_pStartEntry, nTemp);
    return pEntry;
}


//  checks if the entry was hit "the right way"
//  (Focusrect+ ContextBitmap at TreeListBox)

bool SvImpLBox::EntryReallyHit(SvTreeListEntry* pEntry, const Point& rPosPixel, tools::Long nLine)
{
    bool bRet;
    // we are not too exact when it comes to "special" entries
    // (with CheckButtons etc.)
    if( pEntry->ItemCount() >= 3 )
        return true;

    tools::Rectangle aRect( m_pView->GetFocusRect( pEntry, nLine ));
    aRect.SetRight( GetOutputSize().Width() - m_pView->GetMapMode().GetOrigin().X() );

    SvLBoxContextBmp* pBmp = static_cast<SvLBoxContextBmp*>(pEntry->GetFirstItem(SvLBoxItemType::ContextBmp));
    aRect.AdjustLeft( -pBmp->GetWidth(m_pView,pEntry) );
    aRect.AdjustLeft( -4 ); // a little tolerance

    Point aPos( rPosPixel );
    aPos -= m_pView->GetMapMode().GetOrigin();
    bRet = aRect.Contains( aPos );
    return bRet;
}


// returns 0 if position is just past the last entry
SvTreeListEntry* SvImpLBox::GetEntry( const Point& rPoint ) const
{
    if( (m_pView->GetEntryCount() == 0) || !m_pStartEntry ||
        (rPoint.Y() > m_aOutputSize.Height())
        || !m_pView->GetEntryHeight())
        return nullptr;

    sal_uInt16 nClickedEntry = static_cast<sal_uInt16>(rPoint.Y() / m_pView->GetEntryHeight() );
    sal_uInt16 nTemp = nClickedEntry;
    SvTreeListEntry* pEntry = m_pView->NextVisible(m_pStartEntry, nTemp);
    if( nTemp != nClickedEntry )
        pEntry = nullptr;
    return pEntry;
}


SvTreeListEntry* SvImpLBox::MakePointVisible(const Point& rPoint)
{
    if( !m_pCursor )
        return nullptr;
    tools::Long nY = rPoint.Y();
    SvTreeListEntry* pEntry = nullptr;
    tools::Long nMax = m_aOutputSize.Height();
    if( nY < 0 || nY >= nMax ) // aOutputSize.Height() )
    {
        if( nY < 0 )
            pEntry = m_pView->PrevVisible(m_pCursor);
        else
            pEntry = m_pView->NextVisible(m_pCursor);

        if( pEntry && pEntry != m_pCursor )
            m_pView->SetEntryFocus( m_pCursor, false );

        if( nY < 0 )
            KeyUp( false );
        else
            KeyDown( false );
    }
    else
    {
        pEntry = GetClickedEntry( rPoint );
        if( !pEntry )
        {
            sal_uInt16 nSteps = 0xFFFF;
            // TODO: LastVisible is not yet implemented!
            pEntry = m_pView->NextVisible(m_pStartEntry, nSteps);
        }
        if( pEntry )
        {
            if( pEntry != m_pCursor &&
                 m_aSelEng.GetSelectionMode() == SelectionMode::Single
            )
                m_pView->Select( m_pCursor, false );
        }
    }
    return pEntry;
}

tools::Rectangle SvImpLBox::GetClipRegionRect() const
{
    Point aOrigin( m_pView->GetMapMode().GetOrigin() );
    aOrigin.setX( aOrigin.X() * -1 ); // conversion document coordinates
    tools::Rectangle aClipRect( aOrigin, m_aOutputSize );
    aClipRect.AdjustBottom( 1 );
    return aClipRect;
}


void SvImpLBox::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    if (!m_pView->GetVisibleCount())
        return;

    m_nFlags |= LBoxFlags::InPaint;

    if (m_nFlags & LBoxFlags::Filling)
    {
        SvTreeListEntry* pFirst = m_pView->First();
        if (pFirst != m_pStartEntry)
        {
            ShowCursor(false);
            m_pStartEntry = m_pView->First();
            m_aVerSBar->SetThumbPos( 0 );
            StopUserEvent();
            ShowCursor(true);
            m_nCurUserEvent = Application::PostUserEvent(LINK(this, SvImpLBox, MyUserEvent),
                                                       reinterpret_cast<void*>(1));
            return;
        }
    }

    if (!m_pStartEntry)
    {
        m_pStartEntry = m_pView->First();
    }

    if (m_nNodeBmpTabDistance == NODE_BMP_TABDIST_NOTVALID)
        SetNodeBmpTabDistance();

    tools::Long nRectHeight = rRect.GetHeight();
    tools::Long nEntryHeight = m_pView->GetEntryHeight();

    // calculate area for the entries we want to draw
    sal_uInt16 nStartLine = static_cast<sal_uInt16>(rRect.Top() / nEntryHeight);
    sal_uInt16 nCount = static_cast<sal_uInt16>(nRectHeight / nEntryHeight);
    nCount += 2; // don't miss a row

    tools::Long nY = nStartLine * nEntryHeight;
    SvTreeListEntry* pEntry = m_pStartEntry;
    while (nStartLine && pEntry)
    {
        pEntry = m_pView->NextVisible(pEntry);
        nStartLine--;
    }

    if (!m_pCursor && !mbNoAutoCurEntry)
    {
        // do not select if multiselection or explicit set
        bool bNotSelect = (m_aSelEng.GetSelectionMode() == SelectionMode::Multiple ) || ((m_nStyle & WB_NOINITIALSELECTION) == WB_NOINITIALSELECTION);
        SetCursor(m_pStartEntry, bNotSelect);
    }

    for(sal_uInt16 n=0; n< nCount && pEntry; n++)
    {
        /*long nMaxRight=*/
        m_pView->PaintEntry1(*pEntry, nY, rRenderContext );
        nY += nEntryHeight;
        pEntry = m_pView->NextVisible(pEntry);
    }

    if (m_nStyle & (WB_HASLINES | WB_HASLINESATROOT))
        DrawNet(rRenderContext);

    m_nFlags &= ~LBoxFlags::DeselectAll;
    m_nFlags &= ~LBoxFlags::InPaint;
}

void SvImpLBox::MakeVisible( SvTreeListEntry* pEntry, bool bMoveToTop )
{
    if( !pEntry )
        return;

    bool bInView = IsEntryInView( pEntry );

    if( bInView && (!bMoveToTop || m_pStartEntry == pEntry) )
        return;  // is already visible

    if( m_pStartEntry || mbForceMakeVisible )
        m_nFlags &= ~LBoxFlags::Filling;
    if( !bInView )
    {
        if( !m_pView->IsEntryVisible(pEntry) )  // Parent(s) collapsed?
        {
            SvTreeListEntry* pParent = m_pView->GetParent( pEntry );
            while( pParent )
            {
                if( !m_pView->IsExpanded( pParent ) )
                {
                    bool bRet = m_pView->Expand( pParent );
                    DBG_ASSERT(bRet,"Not expanded!");
                }
                pParent = m_pView->GetParent( pParent );
            }
            // do the parent's children fit into the view or do we have to scroll?
            if( IsEntryInView( pEntry ) && !bMoveToTop )
                return;  // no need to scroll
        }
    }

    m_pStartEntry = pEntry;
    ShowCursor( false );
    FillView();
    m_aVerSBar->SetThumbPos( static_cast<tools::Long>(m_pView->GetVisiblePos( m_pStartEntry )) );
    ShowCursor( true );
    m_pView->Invalidate();
}

void SvImpLBox::ScrollToAbsPos( tools::Long nPos )
{
    if( m_pView->GetVisibleCount() == 0 )
        return;
    tools::Long nLastEntryPos = m_pView->GetAbsPos( m_pView->Last() );

    if( nPos < 0 )
        nPos = 0;
    else if( nPos > nLastEntryPos )
        nPos = nLastEntryPos;

    SvTreeListEntry* pEntry = m_pView->GetEntryAtAbsPos( nPos );
    if( !pEntry || pEntry == m_pStartEntry )
        return;

    if( m_pStartEntry || mbForceMakeVisible )
        m_nFlags &= ~LBoxFlags::Filling;

    if( m_pView->IsEntryVisible(pEntry) )
    {
        m_pStartEntry = pEntry;
        ShowCursor( false );
        m_aVerSBar->SetThumbPos( nPos );
        ShowCursor( true );
        if (GetUpdateMode())
            m_pView->Invalidate();
    }
}

void SvImpLBox::DrawNet(vcl::RenderContext& rRenderContext)
{
    if (m_pView->GetVisibleCount() < 2 && !m_pStartEntry->HasChildrenOnDemand() &&
        !m_pStartEntry->HasChildren())
    {
        return;
    }

    // for platforms that don't have nets, DrawNativeControl does nothing and returns true
    // so that SvImpLBox::DrawNet() doesn't draw anything either
    if (rRenderContext.IsNativeControlSupported(ControlType::ListNet, ControlPart::Entire))
    {
        ImplControlValue aControlValue;
        if (rRenderContext.DrawNativeControl(ControlType::ListNet, ControlPart::Entire,
                                             tools::Rectangle(), ControlState::ENABLED, aControlValue, OUString()))
        {
            return;
        }
    }

    tools::Long nEntryHeight = m_pView->GetEntryHeight();
    tools::Long nEntryHeightDIV2 = nEntryHeight / 2;
    if( nEntryHeightDIV2 && !(nEntryHeight & 0x0001))
        nEntryHeightDIV2--;

    SvTreeListEntry* pChild;
    SvTreeListEntry* pEntry = m_pStartEntry;

    SvLBoxTab* pFirstDynamicTab = m_pView->GetFirstDynamicTab();
    while (m_pTree->GetDepth( pEntry ) > 0)
    {
        pEntry = m_pView->GetParent(pEntry);
    }
    sal_uInt16 nOffs = static_cast<sal_uInt16>(m_pView->GetVisiblePos(m_pStartEntry) - m_pView->GetVisiblePos(pEntry));
    tools::Long nY = 0;
    nY -= (nOffs * nEntryHeight);

    DBG_ASSERT(pFirstDynamicTab,"No Tree!");

    rRenderContext.Push(vcl::PushFlags::LINECOLOR);

    const StyleSettings& rStyleSettings = rRenderContext.GetSettings().GetStyleSettings();
    Color aCol = rStyleSettings.GetFaceColor();

    if (aCol.IsRGBEqual(rRenderContext.GetBackground().GetColor()))
        aCol = rStyleSettings.GetShadowColor();
    rRenderContext.SetLineColor(aCol);
    Point aPos1, aPos2;
    sal_uInt16 nDistance;
    sal_uLong nMax = m_nVisibleCount + nOffs + 1;

    const Image& rExpandedNodeBitmap = GetExpandedNodeBmp();

    for (sal_uLong n=0; n< nMax && pEntry; n++)
    {
        if (m_pView->IsExpanded(pEntry))
        {
            // draw vertical line
            aPos1.setX(m_pView->GetTabPos(pEntry, pFirstDynamicTab) + m_nNodeBmpTabDistance +
                       rExpandedNodeBitmap.GetSizePixel().Width() / 2);
            aPos1.setY(nY + nEntryHeight);
            pChild = m_pView->FirstChild(pEntry);
            assert(pChild && "Child?");
            pChild = pChild->LastSibling();
            nDistance = static_cast<sal_uInt16>(m_pView->GetVisiblePos(pChild) -
                                                m_pView->GetVisiblePos(pEntry));
            aPos2 = aPos1;
            aPos2.AdjustY((nDistance * nEntryHeight) - (nEntryHeightDIV2 + 2));
            rRenderContext.DrawLine(aPos1, aPos2);
        }
        // visible in control?
        if (n >= nOffs && !m_pTree->IsAtRootDepth(pEntry))
        {
            // draw horizontal line
            aPos1.setX(m_pView->GetTabPos(m_pView->GetParent(pEntry), pFirstDynamicTab)
                       + m_nNodeBmpTabDistance
                       + rExpandedNodeBitmap.GetSizePixel().Width() / 2);
            aPos1.setY(nY + nEntryHeightDIV2);
            aPos2 = aPos1;
            aPos2.AdjustX(m_pView->GetIndent() / 2);
            rRenderContext.DrawLine(aPos1, aPos2);
        }
        nY += nEntryHeight;
        pEntry = m_pView->NextVisible(pEntry);
    }

    rRenderContext.Pop();
}

void SvImpLBox::PositionScrollBars( Size& rSize, sal_uInt16 nMask )
{
    tools::Long nOverlap = 0;

    Size aVerSize( m_nVerSBarWidth, rSize.Height() );
    Size aHorSize( rSize.Width(), m_nHorSBarHeight );

    if( nMask & 0x0001 )
        aHorSize.AdjustWidth( -m_nVerSBarWidth );
    if( nMask & 0x0002 )
        aVerSize.AdjustHeight( -m_nHorSBarHeight );

    aVerSize.AdjustHeight(2 * nOverlap );
    Point aVerPos( rSize.Width() - aVerSize.Width() + nOverlap, -nOverlap );
    m_aVerSBar->SetPosSizePixel( aVerPos, aVerSize );

    aHorSize.AdjustWidth(2 * nOverlap );
    Point aHorPos( -nOverlap, rSize.Height() - aHorSize.Height() + nOverlap );

    m_aHorSBar->SetPosSizePixel( aHorPos, aHorSize );

    if( nMask & 0x0001 )
        rSize.setWidth( aVerPos.X() );
    if( nMask & 0x0002 )
        rSize.setHeight( aHorPos.Y() );

    if( (nMask & (0x0001|0x0002)) == (0x0001|0x0002) )
        m_aScrBarBox->Show();
    else
        m_aScrBarBox->Hide();
}

void SvImpLBox::AdjustScrollBars( Size& rSize )
{
    tools::Long nEntryHeight = m_pView->GetEntryHeight();
    if( !nEntryHeight )
        return;

    sal_uInt16 nResult = 0;

    Size aOSize( m_pView->Control::GetOutputSizePixel() );

    const WinBits nWindowStyle = m_pView->GetStyle();
    bool bVerSBar = ( nWindowStyle & WB_VSCROLL ) != 0;
    bool bHorBar = false;
    tools::Long nMaxRight = aOSize.Width(); //GetOutputSize().Width();
    Point aOrigin( m_pView->GetMapMode().GetOrigin() );
    aOrigin.setX( aOrigin.X() * -1 );
    nMaxRight += aOrigin.X() - 1;
    tools::Long nVis = m_nMostRight - aOrigin.X();
    if( (nWindowStyle & (WB_AUTOHSCROLL|WB_HSCROLL)) &&
        (nVis < m_nMostRight || nMaxRight < m_nMostRight) )
    {
        bHorBar = true;
    }

    // number of entries that are not collapsed
    sal_uLong nTotalCount = m_pView->GetVisibleCount();

    // number of entries visible within the view
    m_nVisibleCount = aOSize.Height() / nEntryHeight;

    // do we need a vertical scrollbar?
    if( bVerSBar || nTotalCount > m_nVisibleCount )
    {
        nResult = 1;
        nMaxRight -= m_nVerSBarWidth;
        if( !bHorBar )
        {
            if( (nWindowStyle & (WB_AUTOHSCROLL|WB_HSCROLL)) &&
                (nVis < m_nMostRight || nMaxRight < m_nMostRight) )
                bHorBar = true;
        }
    }

    // do we need a horizontal scrollbar?
    if( bHorBar )
    {
        nResult |= 0x0002;
        // the number of entries visible within the view has to be recalculated
        // because the horizontal scrollbar is now visible.
        m_nVisibleCount =  (aOSize.Height() - m_nHorSBarHeight) / nEntryHeight;
        // we might actually need a vertical scrollbar now
        if( !(nResult & 0x0001) &&
            ((nTotalCount > m_nVisibleCount) || bVerSBar) )
        {
            nResult = 3;
        }
    }

    PositionScrollBars( aOSize, nResult );

    // adapt Range, VisibleRange etc.

    // refresh output size, in case we have to scroll
    tools::Rectangle aRect;
    aRect.SetSize( aOSize );
    m_aSelEng.SetVisibleArea( aRect );

    // vertical scrollbar
    tools::Long nTemp = static_cast<tools::Long>(m_nVisibleCount);
    nTemp--;
    if( nTemp != m_aVerSBar->GetVisibleSize() )
    {
        if( !m_bInVScrollHdl )
        {
            m_aVerSBar->SetPageSize( nTemp - 1 );
            m_aVerSBar->SetVisibleSize( nTemp );
        }
        else
        {
            m_nFlags |= LBoxFlags::EndScrollSetVisSize;
            m_nNextVerVisSize = nTemp;
        }
    }

    // horizontal scrollbar
    nTemp = m_aHorSBar->GetThumbPos();
    m_aHorSBar->SetVisibleSize( aOSize.Width() );
    tools::Long nNewThumbPos = m_aHorSBar->GetThumbPos();
    Range aRange( m_aHorSBar->GetRange() );
    if( aRange.Max() < m_nMostRight+25 )
    {
        aRange.Max() = m_nMostRight+25;
        m_aHorSBar->SetRange( aRange );
    }

    if( nTemp != nNewThumbPos )
    {
        nTemp = nNewThumbPos - nTemp;
        if( m_pView->IsEditingActive() )
        {
            m_pView->EndEditing( true ); // Cancel
            m_pView->PaintImmediately();
        }
        m_pView->nFocusWidth = -1;
        KeyLeftRight( nTemp );
    }

    if( nResult & 0x0001 )
        m_aVerSBar->Show();
    else
        m_aVerSBar->Hide();

    if( nResult & 0x0002 )
        m_aHorSBar->Show();
    else
    {
        m_aHorSBar->Hide();
    }
    rSize = aOSize;
}

void SvImpLBox::InitScrollBarBox()
{
    m_aScrBarBox->SetSizePixel( Size(m_nVerSBarWidth, m_nHorSBarHeight) );
    Size aSize( m_pView->Control::GetOutputSizePixel() );
    m_aScrBarBox->SetPosPixel( Point(aSize.Width()-m_nVerSBarWidth, aSize.Height()-m_nHorSBarHeight));
}

void SvImpLBox::Resize()
{
    m_aOutputSize = m_pView->Control::GetOutputSizePixel();
    if( m_aOutputSize.IsEmpty() )
        return;
    m_nFlags |= LBoxFlags::InResize;
    InitScrollBarBox();

    if( m_pView->GetEntryHeight())
    {
        AdjustScrollBars( m_aOutputSize );
        UpdateAll(false);
    }
    // HACK, as in floating and docked windows the scrollbars might not be drawn
    // correctly/not be drawn at all after resizing!
    if( m_aHorSBar->IsVisible())
        m_aHorSBar->Invalidate();
    if( m_aVerSBar->IsVisible())
        m_aVerSBar->Invalidate();
    m_nFlags &= ~LBoxFlags::InResize;
}

void SvImpLBox::FillView()
{
    if( !m_pStartEntry )
    {
        sal_uLong nVisibleViewCount = m_pView->GetVisibleCount();
        tools::Long nTempThumb = m_aVerSBar->GetThumbPos();
        if( nTempThumb < 0 )
            nTempThumb = 0;
        else if( o3tl::make_unsigned(nTempThumb) >= nVisibleViewCount )
            nTempThumb = nVisibleViewCount == 0 ? 0 : nVisibleViewCount - 1;
        m_pStartEntry = m_pView->GetEntryAtVisPos(nTempThumb);
    }
    if( !m_pStartEntry )
        return;

    sal_uInt16 nLast = static_cast<sal_uInt16>(m_pView->GetVisiblePos(m_pView->LastVisible()));
    sal_uInt16 nThumb = static_cast<sal_uInt16>(m_pView->GetVisiblePos( m_pStartEntry ));
    sal_uLong nCurDispEntries = nLast-nThumb+1;
    if( nCurDispEntries >=  m_nVisibleCount )
        return;

    ShowCursor( false );
    // fill window by moving the thumb up incrementally
    bool bFound = false;
    SvTreeListEntry* pTemp = m_pStartEntry;
    while( nCurDispEntries < m_nVisibleCount && pTemp )
    {
        pTemp = m_pView->PrevVisible(m_pStartEntry);
        if( pTemp )
        {
            nThumb--;
            m_pStartEntry = pTemp;
            nCurDispEntries++;
            bFound = true;
        }
    }
    if( bFound )
    {
        m_aVerSBar->SetThumbPos( nThumb );
        ShowCursor( true ); // recalculate focus rectangle
        m_pView->Invalidate();
    }
}


void SvImpLBox::ShowVerSBar()
{
    bool bVerBar = ( m_pView->GetStyle() & WB_VSCROLL ) != 0;
    sal_uLong nVis = 0;
    if( !bVerBar )
        nVis = m_pView->GetVisibleCount();
    if( bVerBar || (m_nVisibleCount && nVis > static_cast<sal_uLong>(m_nVisibleCount-1)) )
    {
        if( !m_aVerSBar->IsVisible() )
        {
            m_pView->nFocusWidth = -1;
            AdjustScrollBars( m_aOutputSize );
            if( GetUpdateMode() )
                m_aVerSBar->Invalidate();
        }
    }
    else
    {
        if( m_aVerSBar->IsVisible() )
        {
            m_pView->nFocusWidth = -1;
            AdjustScrollBars( m_aOutputSize );
        }
    }

    tools::Long nMaxRight = GetOutputSize().Width();
    Point aPos( m_pView->GetMapMode().GetOrigin() );
    aPos.setX( aPos.X() * -1 ); // convert document coordinates
    nMaxRight = nMaxRight + aPos.X() - 1;
    if( nMaxRight < m_nMostRight  )
    {
        if( !m_aHorSBar->IsVisible() )
        {
            m_pView->nFocusWidth = -1;
            AdjustScrollBars( m_aOutputSize );
            if( GetUpdateMode() )
                m_aHorSBar->Invalidate();
        }
        else
        {
            Range aRange( m_aHorSBar->GetRange() );
            if( aRange.Max() < m_nMostRight+25 )
            {
                aRange.Max() = m_nMostRight+25;
                m_aHorSBar->SetRange( aRange );
            }
            else
            {
                m_pView->nFocusWidth = -1;
                AdjustScrollBars( m_aOutputSize );
            }
        }
    }
    else
    {
        if( m_aHorSBar->IsVisible() )
        {
            m_pView->nFocusWidth = -1;
            AdjustScrollBars( m_aOutputSize );
        }
    }
}


void SvImpLBox::SyncVerThumb()
{
    if( m_pStartEntry )
    {
        tools::Long nEntryPos = m_pView->GetVisiblePos( m_pStartEntry );
        m_aVerSBar->SetThumbPos( nEntryPos );
    }
    else
        m_aVerSBar->SetThumbPos( 0 );
}

bool SvImpLBox::IsEntryInView( SvTreeListEntry* pEntry ) const
{
    // parent collapsed
    if( !m_pView->IsEntryVisible(pEntry) )
        return false;
    tools::Long nY = GetEntryLine( pEntry );
    if( nY < 0 )
        return false;
    tools::Long nMax = m_nVisibleCount * m_pView->GetEntryHeight();
    return nY < nMax;
}


tools::Long SvImpLBox::GetEntryLine(const SvTreeListEntry* pEntry) const
{
    if(!m_pStartEntry )
        return -1; // invisible position

    tools::Long nFirstVisPos = m_pView->GetVisiblePos( m_pStartEntry );
    tools::Long nEntryVisPos = m_pView->GetVisiblePos( pEntry );
    nFirstVisPos = nEntryVisPos - nFirstVisPos;
    nFirstVisPos *= m_pView->GetEntryHeight();
    return nFirstVisPos;
}

void SvImpLBox::SetEntryHeight()
{
    SetNodeBmpWidth( GetExpandedNodeBmp() );
    SetNodeBmpWidth( GetCollapsedNodeBmp() );
    if(!m_pView->HasViewData()) // are we within the Clear?
    {
        Size aSize = m_pView->Control::GetOutputSizePixel();
        AdjustScrollBars( aSize );
    }
    else
    {
        Resize();
        if( GetUpdateMode() )
            m_pView->Invalidate();
    }
}


// ***********************************************************************
// Callback Functions
// ***********************************************************************

void SvImpLBox::EntryExpanded( SvTreeListEntry* pEntry )
{
    // SelAllDestrAnch( false, true ); //DeselectAll();
    if( !GetUpdateMode() )
        return;

    ShowCursor( false );
    tools::Long nY = GetEntryLine( pEntry );
    if( IsLineVisible(nY) )
    {
        InvalidateEntriesFrom( nY );
        FindMostRight( pEntry );
    }
    m_aVerSBar->SetRange( Range(0, m_pView->GetVisibleCount()-1 ) );
    // if we expanded before the thumb, the thumb's position has to be
    // corrected
    SyncVerThumb();
    ShowVerSBar();
    ShowCursor( true );
}

void SvImpLBox::EntryCollapsed( SvTreeListEntry* pEntry )
{
    if( !m_pView->IsEntryVisible( pEntry ) )
        return;

    ShowCursor( false );

    if( !m_pMostRightEntry || m_pTree->IsChild( pEntry,m_pMostRightEntry ) )
    {
        FindMostRight();
    }

    if( m_pStartEntry )
    {
        tools::Long nOldThumbPos   = m_aVerSBar->GetThumbPos();
        sal_uLong nVisList      = m_pView->GetVisibleCount();
        m_aVerSBar->SetRange( Range(0, nVisList-1) );
        tools::Long nNewThumbPos   = m_aVerSBar->GetThumbPos();
        if( nNewThumbPos != nOldThumbPos  )
        {
            m_pStartEntry = m_pView->First();
            sal_uInt16 nDistance = static_cast<sal_uInt16>(nNewThumbPos);
            if( nDistance )
                m_pStartEntry = m_pView->NextVisible(m_pStartEntry, nDistance);
            if( GetUpdateMode() )
                m_pView->Invalidate();
        }
        else
            SyncVerThumb();
        ShowVerSBar();
    }
    // has the cursor been collapsed?
    if( m_pTree->IsChild( pEntry, m_pCursor ) )
        SetCursor( pEntry );
    if( GetUpdateMode() )
        ShowVerSBar();
    ShowCursor( true );
    if( GetUpdateMode() && m_pCursor )
        m_pView->Select( m_pCursor );
}

void SvImpLBox::CollapsingEntry( SvTreeListEntry* pEntry )
{
    if( !m_pView->IsEntryVisible( pEntry ) || !m_pStartEntry )
        return;

    SelAllDestrAnch( false ); // deselect all

    // is the collapsed cursor visible?
    tools::Long nY = GetEntryLine( pEntry );
    if( IsLineVisible(nY) )
    {
        if( GetUpdateMode() )
            InvalidateEntriesFrom( nY );
    }
    else
    {
        if( m_pTree->IsChild(pEntry, m_pStartEntry) )
        {
            m_pStartEntry = pEntry;
            if( GetUpdateMode() )
                m_pView->Invalidate();
        }
    }
}


void SvImpLBox::SetNodeBmpWidth( const Image& rBmp )
{
    const Size aSize( rBmp.GetSizePixel() );
    m_nNodeBmpWidth = aSize.Width();
}

void SvImpLBox::SetNodeBmpTabDistance()
{
    m_nNodeBmpTabDistance = -m_pView->GetIndent();
    if( m_pView->nContextBmpWidthMax )
    {
        // only if the first dynamic tab is centered (we currently assume that)
        Size aSize = GetExpandedNodeBmp().GetSizePixel();
        m_nNodeBmpTabDistance -= aSize.Width() / 2;
    }
}


// corrects the cursor when using SingleSelection

void SvImpLBox::EntrySelected( SvTreeListEntry* pEntry, bool bSelect )
{
    if( m_nFlags & LBoxFlags::IgnoreSelect )
        return;

    m_nFlags &= ~LBoxFlags::DeselectAll;
    if( bSelect &&
        m_aSelEng.GetSelectionMode() == SelectionMode::Single &&
        pEntry != m_pCursor )
    {
        SetCursor( pEntry );
        DBG_ASSERT(m_pView->GetSelectionCount()==1,"selection count?");
    }

    if( GetUpdateMode() && m_pView->IsEntryVisible(pEntry) )
    {
        tools::Long nY = GetEntryLine( pEntry );
        if( IsLineVisible( nY ) )
        {
            ShowCursor(false);
            InvalidateEntry(pEntry);
            ShowCursor(true);
        }
    }
}


void SvImpLBox::RemovingEntry( SvTreeListEntry* pEntry )
{
    CallEventListeners( VclEventId::ListboxItemRemoved , pEntry );

    DestroyAnchor();

    if( !m_pView->IsEntryVisible( pEntry ) )
    {
        // if parent is collapsed => bye!
        m_nFlags |= LBoxFlags::RemovedEntryInvisible;
        return;
    }

    if( pEntry == m_pMostRightEntry || (
        pEntry->HasChildren() && m_pView->IsExpanded(pEntry) &&
        m_pTree->IsChild(pEntry, m_pMostRightEntry)))
    {
        m_nFlags |= LBoxFlags::RemovedRecalcMostRight;
    }

    SvTreeListEntry* pOldStartEntry = m_pStartEntry;

    SvTreeListEntry* pParent = m_pView->GetModel()->GetParent(pEntry);

    if (pParent && m_pView->GetModel()->GetChildList(pParent).size() == 1)
    {
        DBG_ASSERT( m_pView->IsExpanded( pParent ), "Parent not expanded");
        pParent->SetFlags( pParent->GetFlags() | SvTLEntryFlags::NO_NODEBMP);
        InvalidateEntry( pParent );
    }

    if( m_pCursor && m_pTree->IsChild( pEntry, m_pCursor) )
        m_pCursor = pEntry;
    if( m_pStartEntry && m_pTree->IsChild(pEntry,m_pStartEntry) )
        m_pStartEntry = pEntry;

    SvTreeListEntry* pTemp;
    if( m_pCursor && m_pCursor == pEntry )
    {
        if( m_bSimpleTravel )
            m_pView->Select( m_pCursor, false );
        ShowCursor( false );    // focus rectangle gone
        // NextSibling, because we also delete the children of the cursor
        pTemp = m_pCursor->NextSibling();
        if( !pTemp )
            pTemp = m_pView->PrevVisible(m_pCursor);

        SetCursor( pTemp, true );
    }
    if( m_pStartEntry && m_pStartEntry == pEntry )
    {
        pTemp = m_pStartEntry->NextSibling();
        if( !pTemp )
            pTemp = m_pView->PrevVisible(m_pStartEntry);
        m_pStartEntry = pTemp;
    }
    if( GetUpdateMode())
    {
        // if it is the last one, we have to invalidate it, so the lines are
        // drawn correctly (in this case they're deleted)
        if( m_pStartEntry && (m_pStartEntry != pOldStartEntry || pEntry == m_pView->GetModel()->Last()) )
        {
            m_aVerSBar->SetThumbPos( m_pView->GetVisiblePos( m_pStartEntry ));
            m_pView->Invalidate( GetVisibleArea() );
        }
        else
            InvalidateEntriesFrom( GetEntryLine( pEntry ) );
    }
}

void SvImpLBox::EntryRemoved()
{
    if( m_nFlags & LBoxFlags::RemovedEntryInvisible )
    {
        m_nFlags &= ~LBoxFlags::RemovedEntryInvisible;
        return;
    }
    if( !m_pStartEntry )
        m_pStartEntry = m_pTree->First();
    if( !m_pCursor )
        SetCursor( m_pStartEntry, true );

    if( m_pCursor && (m_bSimpleTravel || !m_pView->GetSelectionCount() ))
        m_pView->Select( m_pCursor );

    if( GetUpdateMode())
    {
        if( m_nFlags & LBoxFlags::RemovedRecalcMostRight )
            FindMostRight();
        m_aVerSBar->SetRange( Range(0, m_pView->GetVisibleCount()-1 ) );
        FillView();
        if( m_pStartEntry )
            // if something above the thumb was deleted
            m_aVerSBar->SetThumbPos( m_pView->GetVisiblePos( m_pStartEntry) );

        ShowVerSBar();
        if( m_pCursor && m_pView->HasFocus() && !m_pView->IsSelected(m_pCursor) )
        {
            if( m_pView->GetSelectionCount() )
            {
                // is a neighboring entry selected?
                SvTreeListEntry* pNextCursor = m_pView->PrevVisible( m_pCursor );
                if( !pNextCursor || !m_pView->IsSelected( pNextCursor ))
                    pNextCursor = m_pView->NextVisible( m_pCursor );
                if( !pNextCursor || !m_pView->IsSelected( pNextCursor ))
                    // no neighbor selected: use first selected
                    pNextCursor = m_pView->FirstSelected();
                SetCursor( pNextCursor );
                MakeVisible( m_pCursor );
            }
            else
                m_pView->Select( m_pCursor );
        }
        ShowCursor( true );
    }
    m_nFlags &= ~LBoxFlags::RemovedRecalcMostRight;
}


void SvImpLBox::MovingEntry( SvTreeListEntry* pEntry )
{
    bool bDeselAll(m_nFlags & LBoxFlags::DeselectAll);
    SelAllDestrAnch( false );  // DeselectAll();
    if( !bDeselAll )
        m_nFlags &= ~LBoxFlags::DeselectAll;

    if( pEntry == m_pCursor )
        ShowCursor( false );
    if( IsEntryInView( pEntry ) )
        m_pView->Invalidate();
    if( pEntry != m_pStartEntry )
        return;

    SvTreeListEntry* pNew = nullptr;
    if( !pEntry->HasChildren() )
    {
        pNew = m_pView->NextVisible(m_pStartEntry);
        if( !pNew )
            pNew = m_pView->PrevVisible(m_pStartEntry);
    }
    else
    {
        pNew = pEntry->NextSibling();
        if( !pNew )
            pNew = pEntry->PrevSibling();
    }
    m_pStartEntry = pNew;
}

void SvImpLBox::EntryMoved( SvTreeListEntry* pEntry )
{
    UpdateContextBmpWidthVectorFromMovedEntry( pEntry );

    if ( !m_pStartEntry )
        // this might happen if the only entry in the view is moved to its very same position
        // #i97346#
        m_pStartEntry = m_pView->First();

    m_aVerSBar->SetRange( Range(0, m_pView->GetVisibleCount()-1));
    sal_uInt16 nFirstPos = static_cast<sal_uInt16>(m_pTree->GetAbsPos( m_pStartEntry ));
    sal_uInt16 nNewPos = static_cast<sal_uInt16>(m_pTree->GetAbsPos( pEntry ));
    FindMostRight();
    if( nNewPos < nFirstPos ) // HACK!
        m_pStartEntry = pEntry;
    SyncVerThumb();
    if( pEntry == m_pCursor )
    {
        if( m_pView->IsEntryVisible( m_pCursor ) )
            ShowCursor( true );
        else
        {
            SvTreeListEntry* pParent = pEntry;
            do {
                pParent = m_pTree->GetParent( pParent );
            }
            while( !m_pView->IsEntryVisible( pParent ) );
            SetCursor( pParent );
        }
    }
    if( IsEntryInView( pEntry ) )
        m_pView->Invalidate();
}


void SvImpLBox::EntryInserted( SvTreeListEntry* pEntry )
{
    if( !GetUpdateMode() )
        return;

    SvTreeListEntry* pParent = m_pTree->GetParent(pEntry);
    if (pParent && m_pTree->GetChildList(pParent).size() == 1)
        // draw plus sign
        m_pTree->InvalidateEntry( pParent );

    if( !m_pView->IsEntryVisible( pEntry ) )
        return;
    bool bDeselAll(m_nFlags & LBoxFlags::DeselectAll);
    if( bDeselAll )
        SelAllDestrAnch( false );
    else
        DestroyAnchor();
    //  nFlags &= (~LBoxFlags::DeselectAll);
//      ShowCursor( false ); // if cursor is moved lower
    tools::Long nY = GetEntryLine( pEntry );
    bool bEntryVisible = IsLineVisible( nY );
    if( bEntryVisible )
    {
        ShowCursor( false ); // if cursor is moved lower
        nY -= m_pView->GetEntryHeight(); // because of lines
        InvalidateEntriesFrom( nY );
    }
    else if( m_pStartEntry && nY < GetEntryLine(m_pStartEntry) )
    {
        // Check if the view is filled completely. If not, then adjust
        // pStartEntry and the Cursor (automatic scrolling).
        sal_uInt16 nLast = static_cast<sal_uInt16>(m_pView->GetVisiblePos(m_pView->LastVisible()));
        sal_uInt16 nThumb = static_cast<sal_uInt16>(m_pView->GetVisiblePos( m_pStartEntry ));
        sal_uInt16 nCurDispEntries = nLast-nThumb+1;
        if( nCurDispEntries < m_nVisibleCount )
        {
            // set at the next paint event
            m_pStartEntry = nullptr;
            SetCursor( nullptr );
            m_pView->Invalidate();
        }
    }
    else if( !m_pStartEntry )
        m_pView->Invalidate();

    SetMostRight( pEntry );
    m_aVerSBar->SetRange( Range(0, m_pView->GetVisibleCount()-1));
    SyncVerThumb(); // if something was inserted before the thumb
    ShowVerSBar();
    ShowCursor( true );
    if( m_pStartEntry != m_pView->First() && (m_nFlags & LBoxFlags::Filling) )
        m_pView->PaintImmediately();
}


// ********************************************************************
// Event handler
// ********************************************************************


// ****** Control the control animation

bool SvImpLBox::ButtonDownCheckCtrl(const MouseEvent& rMEvt, SvTreeListEntry* pEntry)
{
    SvLBoxItem* pItem = m_pView->GetItem(pEntry,rMEvt.GetPosPixel().X(),&m_pActiveTab);
    if (pItem && pItem->GetType() == SvLBoxItemType::Button)
    {
        m_pActiveButton = static_cast<SvLBoxButton*>(pItem);
        m_pActiveEntry = pEntry;
        if( m_pCursor == m_pActiveEntry )
            m_pView->HideFocus();
        m_pView->CaptureMouse();
        m_pActiveButton->SetStateHilighted( true );
        InvalidateEntry(m_pActiveEntry);
        return true;
    }
    else
        m_pActiveButton = nullptr;
    return false;
}

bool SvImpLBox::MouseMoveCheckCtrl(const MouseEvent& rMEvt, SvTreeListEntry const * pEntry)
{
    if( m_pActiveButton )
    {
        tools::Long nMouseX = rMEvt.GetPosPixel().X();
        if( pEntry == m_pActiveEntry &&
             m_pView->GetItem(m_pActiveEntry, nMouseX) == m_pActiveButton )
        {
            if( !m_pActiveButton->IsStateHilighted() )
            {
                m_pActiveButton->SetStateHilighted(true );
                InvalidateEntry(m_pActiveEntry);
            }
        }
        else
        {
            if( m_pActiveButton->IsStateHilighted() )
            {
                m_pActiveButton->SetStateHilighted(false );
                InvalidateEntry(m_pActiveEntry);
            }
        }
        return true;
    }
    return false;
}

bool SvImpLBox::ButtonUpCheckCtrl( const MouseEvent& rMEvt )
{
    if( m_pActiveButton )
    {
        m_pView->ReleaseMouse();
        SvTreeListEntry* pEntry = GetClickedEntry( rMEvt.GetPosPixel() );
        m_pActiveButton->SetStateHilighted( false );
        tools::Long nMouseX = rMEvt.GetPosPixel().X();
        if (pEntry == m_pActiveEntry && m_pView->GetItem(m_pActiveEntry, nMouseX) == m_pActiveButton)
            m_pActiveButton->ClickHdl(m_pActiveEntry);
        InvalidateEntry(m_pActiveEntry);
        if (m_pCursor == m_pActiveEntry)
            ShowCursor(true);
        m_pActiveButton = nullptr;
        m_pActiveEntry = nullptr;
        m_pActiveTab = nullptr;
        return true;
    }
    return false;
}

// ******* Control plus/minus button for expanding/collapsing

// false == no expand/collapse button hit
bool SvImpLBox::IsNodeButton( const Point& rPosPixel, const SvTreeListEntry* pEntry ) const
{
    if( !pEntry->HasChildren() && !pEntry->HasChildrenOnDemand() )
        return false;

    SvLBoxTab* pFirstDynamicTab = m_pView->GetFirstDynamicTab();
    if( !pFirstDynamicTab )
        return false;

    tools::Long nMouseX = rPosPixel.X();
    // convert to document coordinates
    Point aOrigin( m_pView->GetMapMode().GetOrigin() );
    nMouseX -= aOrigin.X();

    tools::Long nX = m_pView->GetTabPos( pEntry, pFirstDynamicTab);
    nX += m_nNodeBmpTabDistance;
    if( nMouseX < nX )
        return false;
    nX += m_nNodeBmpWidth;
    return nMouseX <= nX;
}

// false == hit no node button
bool SvImpLBox::ButtonDownCheckExpand( const MouseEvent& rMEvt, SvTreeListEntry* pEntry )
{
    bool bRet = false;

    if ( m_pView->IsEditingActive() && pEntry == m_pView->pEdEntry )
        // inplace editing -> nothing to do
        bRet = true;
    else if ( IsNodeButton( rMEvt.GetPosPixel(), pEntry ) )
    {
        if ( m_pView->IsExpanded( pEntry ) )
        {
            m_pView->EndEditing( true );
            m_pView->Collapse( pEntry );
        }
        else
        {
            // you can expand an entry, which is in editing
            m_pView->Expand( pEntry );
        }
        bRet = true;
    }

    return bRet;
}

void SvImpLBox::MouseButtonDown( const MouseEvent& rMEvt )
{
    if ( !rMEvt.IsLeft() && !rMEvt.IsRight())
        return;

    m_aEditIdle.Stop();
    Point aPos( rMEvt.GetPosPixel());

    if( aPos.X() > m_aOutputSize.Width() || aPos.Y() > m_aOutputSize.Height() )
        return;

    if( !m_pCursor )
        m_pCursor = m_pStartEntry;
    m_nFlags &= ~LBoxFlags::Filling;
    m_pView->GrabFocus();
    //fdo#82270 Grabbing focus can invalidate the entries, re-fetch
    SvTreeListEntry* pEntry = GetEntry(aPos);
    // the entry can still be invalid!
    if( !pEntry || !m_pView->GetViewData( pEntry ))
        return;

    tools::Long nY = GetEntryLine( pEntry );
    // Node-Button?
    if( ButtonDownCheckExpand( rMEvt, pEntry ) )
        return;

    if( !EntryReallyHit(pEntry,aPos,nY))
        return;

    SvLBoxItem* pXItem = m_pView->GetItem( pEntry, aPos.X() );
    if( pXItem )
    {
        SvLBoxTab* pXTab = m_pView->GetTab( pEntry, pXItem );
        if ( !rMEvt.IsMod1() && !rMEvt.IsMod2() && rMEvt.IsLeft() && pXTab->IsEditable()
            && pEntry == m_pView->FirstSelected() && nullptr == m_pView->NextSelected( pEntry ) )
                // #i8234# FirstSelected() and NextSelected() ensures, that inplace editing is only triggered, when only one entry is selected
            m_nFlags |= LBoxFlags::StartEditTimer;
        if ( !m_pView->IsSelected( pEntry ) )
            m_nFlags &= ~LBoxFlags::StartEditTimer;
    }


    if( (rMEvt.GetClicks() % 2) == 0)
    {
        m_nFlags &= ~LBoxFlags::StartEditTimer;
        m_pView->pHdlEntry = pEntry;
        if( !m_pView->DoubleClickHdl() )
        {
            // Handler signals nothing to be done anymore, bail out, 'this' may
            // even be dead and destroyed.
            return;
        }
        else
        {
            // if the entry was deleted within the handler
            pEntry = GetClickedEntry( aPos );
            if( !pEntry )
                return;
            if( pEntry != m_pView->pHdlEntry )
            {
                // select anew & bye
                if( !m_bSimpleTravel && !m_aSelEng.IsAlwaysAdding())
                    SelAllDestrAnch( false ); // DeselectAll();
                SetCursor( pEntry );

                return;
            }
            if( pEntry->HasChildren() || pEntry->HasChildrenOnDemand() )
            {
                if( m_pView->IsExpanded(pEntry) )
                    m_pView->Collapse( pEntry );
                else
                    m_pView->Expand( pEntry );
                if( pEntry == m_pCursor )  // only if Entryitem was clicked
                                          // (Nodebutton is not an Entryitem!)
                    m_pView->Select( m_pCursor );
                return;
            }
        }
    }
    else
    {
        // CheckButton? (TreeListBox: Check + Info)
        if( ButtonDownCheckCtrl(rMEvt, pEntry) )
            return;
        // Inplace-Editing?
    }
    if ( m_aSelEng.GetSelectionMode() != SelectionMode::NONE
         && !rMEvt.IsRight() ) // tdf#128824
        m_aSelEng.SelMouseButtonDown( rMEvt );
}

void SvImpLBox::MouseButtonUp( const MouseEvent& rMEvt)
{
    if ( !ButtonUpCheckCtrl( rMEvt ) && ( m_aSelEng.GetSelectionMode() != SelectionMode::NONE ) )
        m_aSelEng.SelMouseButtonUp( rMEvt );
    if( m_nFlags & LBoxFlags::StartEditTimer )
    {
        m_nFlags &= ~LBoxFlags::StartEditTimer;
        m_aEditClickPos = rMEvt.GetPosPixel();
        m_aEditIdle.Start();
    }

    if (m_pView->mbActivateOnSingleClick)
    {
        Point aPos(rMEvt.GetPosPixel());
        SvTreeListEntry* pEntry = GetEntry(aPos);
        // tdf#143245 ActivateOnSingleClick only
        // if the 'up' is at the active entry
        // typically selected by the 'down'
        if (!pEntry || pEntry != m_pCursor)
            return;
        m_pView->DoubleClickHdl();
    }
}

void SvImpLBox::MouseMove( const MouseEvent& rMEvt)
{
    Point aPos = rMEvt.GetPosPixel();
    SvTreeListEntry* pEntry = GetClickedEntry(aPos);
    if ( MouseMoveCheckCtrl( rMEvt, pEntry ) || ( m_aSelEng.GetSelectionMode() == SelectionMode::NONE ) )
        return;

    m_aSelEng.SelMouseMove(rMEvt);
    if (m_pView->mbHoverSelection)
    {
        if (aPos.X() < 0 || aPos.Y() < 0 || aPos.X() > m_aOutputSize.Width() || aPos.Y() > m_aOutputSize.Height())
            pEntry = nullptr;
        else
            pEntry = GetEntry(aPos);
        if (!pEntry)
            m_pView->SelectAll(false);
        else if (!m_pView->IsSelected(pEntry) && IsSelectable(pEntry))
        {
            m_pView->mbSelectingByHover = true;
            m_pView->Select(pEntry);
            m_pView->mbSelectingByHover = false;
        }
    }
}

void SvImpLBox::ExpandAll()
{
    sal_uInt16 nRefDepth = m_pTree->GetDepth(m_pCursor);
    SvTreeListEntry* pCur = m_pTree->Next(m_pCursor);
    while (pCur && m_pTree->GetDepth(pCur) > nRefDepth)
    {
        if (pCur->HasChildren() && !m_pView->IsExpanded(pCur))
            m_pView->Expand(pCur);
        pCur = m_pTree->Next(pCur);
    }
}

void SvImpLBox::CollapseTo(SvTreeListEntry* pParentToCollapse)
{
    // collapse all parents until we get to the given parent to collapse
    if (!pParentToCollapse)
        return;

    sal_uInt16 nRefDepth;
    // special case explorer: if the root only has a single
    // entry, don't collapse the root entry
    if (m_pTree->GetChildList(nullptr).size() < 2)
    {
        nRefDepth = 1;
        pParentToCollapse = m_pCursor;
        while (m_pTree->GetParent(pParentToCollapse)
               && m_pTree->GetDepth(m_pTree->GetParent(pParentToCollapse)) > 0)
        {
            pParentToCollapse = m_pTree->GetParent(pParentToCollapse);
        }
    }
    else
        nRefDepth = m_pTree->GetDepth(pParentToCollapse);

    if (m_pView->IsExpanded(pParentToCollapse))
        m_pView->Collapse(pParentToCollapse);
    SvTreeListEntry* pCur = m_pTree->Next(pParentToCollapse);
    while (pCur && m_pTree->GetDepth(pCur) > nRefDepth)
    {
        if (pCur->HasChildren() && m_pView->IsExpanded(pCur))
            m_pView->Collapse(pCur);
        pCur = m_pTree->Next(pCur);
    }
}

bool SvImpLBox::KeyInput( const KeyEvent& rKEvt)
{
    m_aEditIdle.Stop();
    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();

    if( rKeyCode.IsMod2() )
        return false; // don't evaluate Alt key

    m_nFlags &= ~LBoxFlags::Filling;

    if( !m_pCursor )
        m_pCursor = m_pStartEntry;
    if( !m_pCursor )
        return false;

    bool bKeyUsed = true;

    sal_uInt16  nDelta = static_cast<sal_uInt16>(m_aVerSBar->GetPageSize());
    sal_uInt16  aCode = rKeyCode.GetCode();

    bool    bShift = rKeyCode.IsShift();
    bool    bMod1 = rKeyCode.IsMod1();

    SvTreeListEntry* pNewCursor;

    switch( aCode )
    {
        case KEY_UP:
            if( !IsEntryInView( m_pCursor ) )
                MakeVisible( m_pCursor );

            pNewCursor = m_pCursor;
            do
            {
                pNewCursor = m_pView->PrevVisible(pNewCursor);
            } while( pNewCursor && !IsSelectable(pNewCursor) );

            // if there is no next entry, take the current one
            // this ensures that in case of _one_ entry in the list, this entry is selected when pressing
            // the cursor key
            if (!pNewCursor)
                pNewCursor = m_pCursor;

            m_aSelEng.CursorPosChanging( bShift, bMod1 );
            SetCursor( pNewCursor, bMod1 );     // no selection, when Ctrl is on
            if( !IsEntryInView( pNewCursor ) )
                KeyUp( false );
            break;

        case KEY_DOWN:
            if( !IsEntryInView( m_pCursor ) )
                MakeVisible( m_pCursor );

            pNewCursor = m_pCursor;
            do
            {
                pNewCursor = m_pView->NextVisible(pNewCursor);
            } while( pNewCursor && !IsSelectable(pNewCursor) );

            // if there is no next entry, take the current one
            // this ensures that in case of _one_ entry in the list, this entry is selected when pressing
            // the cursor key
            // 06.09.20001 - 83416 - frank.schoenheit@sun.com
            if ( !pNewCursor && m_pCursor )
                pNewCursor = m_pCursor;

            if( pNewCursor )
            {
                m_aSelEng.CursorPosChanging( bShift, bMod1 );
                if( IsEntryInView( pNewCursor ) )
                    SetCursor( pNewCursor, bMod1 ); // no selection, when Ctrl is on
                else
                {
                    if( m_pCursor )
                        m_pView->Select( m_pCursor, false );
                    KeyDown( false );
                    SetCursor( pNewCursor, bMod1 ); // no selection, when Ctrl is on
                }
            }
            else
                KeyDown( false );   // because scrollbar range might still
                                        // allow scrolling
            break;

        case KEY_RIGHT:
        {
            if( m_bSubLstOpLR )
            {
                // only try to expand if sublist is expandable,
                // otherwise ignore the key press
                if( IsExpandable() && !m_pView->IsExpanded( m_pCursor ) )
                    m_pView->Expand( m_pCursor );
            }
            else if (m_aHorSBar->IsVisible())
            {
                tools::Long    nThumb = m_aHorSBar->GetThumbPos();
                nThumb += m_aHorSBar->GetLineSize();
                tools::Long    nOldThumb = m_aHorSBar->GetThumbPos();
                m_aHorSBar->SetThumbPos( nThumb );
                nThumb = nOldThumb;
                nThumb -= m_aHorSBar->GetThumbPos();
                nThumb *= -1;
                if( nThumb )
                {
                    KeyLeftRight( nThumb );
                }
            }
            else
                bKeyUsed = false;
            break;
        }

        case KEY_LEFT:
        {
            if (m_aHorSBar->IsVisible())
            {
                tools::Long    nThumb = m_aHorSBar->GetThumbPos();
                nThumb -= m_aHorSBar->GetLineSize();
                tools::Long    nOldThumb = m_aHorSBar->GetThumbPos();
                m_aHorSBar->SetThumbPos( nThumb );
                nThumb = nOldThumb;
                nThumb -= m_aHorSBar->GetThumbPos();
                if( nThumb )
                {
                    KeyLeftRight( -nThumb );
                }
                else if( m_bSubLstOpLR )
                {
                    if( IsExpandable() && m_pView->IsExpanded( m_pCursor ) )
                        m_pView->Collapse( m_pCursor );
                    else
                    {
                        pNewCursor = m_pView->GetParent( m_pCursor );
                        if( pNewCursor )
                            SetCursor( pNewCursor );
                    }
                }
            }
            else if( m_bSubLstOpLR )
            {
                if( IsExpandable() && m_pView->IsExpanded( m_pCursor ) )
                    m_pView->Collapse( m_pCursor );
                else
                {
                    pNewCursor = m_pView->GetParent( m_pCursor );
                    if( pNewCursor )
                        SetCursor( pNewCursor );
                }
            }
            else
                bKeyUsed = false;
            break;
        }

        case KEY_PAGEUP:
            if( !bMod1 )
            {
                pNewCursor = m_pView->PrevVisible(m_pCursor, nDelta);

                while( nDelta && pNewCursor && !IsSelectable(pNewCursor) )
                {
                    pNewCursor = m_pView->NextVisible(pNewCursor);
                    nDelta--;
                }

                if( nDelta )
                {
                    DBG_ASSERT(pNewCursor && pNewCursor!=m_pCursor, "Cursor?");
                    m_aSelEng.CursorPosChanging( bShift, bMod1 );
                    if( IsEntryInView( pNewCursor ) )
                        SetCursor( pNewCursor );
                    else
                    {
                        SetCursor( pNewCursor );
                        KeyUp( true );
                    }
                }
            }
            else
                bKeyUsed = false;
            break;

        case KEY_PAGEDOWN:
            if( !bMod1 )
            {
                pNewCursor= m_pView->NextVisible(m_pCursor, nDelta);

                while( nDelta && pNewCursor && !IsSelectable(pNewCursor) )
                {
                    pNewCursor = m_pView->PrevVisible(pNewCursor);
                    nDelta--;
                }

                if( nDelta && pNewCursor )
                {
                    DBG_ASSERT(pNewCursor && pNewCursor!=m_pCursor, "Cursor?");
                    m_aSelEng.CursorPosChanging( bShift, bMod1 );
                    if( IsEntryInView( pNewCursor ) )
                        SetCursor( pNewCursor );
                    else
                    {
                        SetCursor( pNewCursor );
                        KeyDown( true );
                    }
                }
                else
                    KeyDown( false ); // see also: KEY_DOWN
            }
            else
                bKeyUsed = false;
            break;

        case KEY_SPACE:
            if ( m_pView->GetSelectionMode() != SelectionMode::NONE )
            {
                if ( bMod1 )
                {
                    if ( m_pView->GetSelectionMode() == SelectionMode::Multiple && !bShift )
                        // toggle selection
                        m_pView->Select( m_pCursor, !m_pView->IsSelected( m_pCursor ) );
                }
                else if ( !bShift /*&& !bMod1*/ )
                {
                    if ( m_aSelEng.IsAddMode() )
                    {
                        // toggle selection
                        m_pView->Select( m_pCursor, !m_pView->IsSelected( m_pCursor ) );
                    }
                    else if ( !m_pView->IsSelected( m_pCursor ) )
                    {
                        SelAllDestrAnch( false );
                        m_pView->Select( m_pCursor );
                    }
                    else
                        bKeyUsed = false;
                }
                else
                    bKeyUsed = false;
            }
            else
                bKeyUsed = false;
            break;

        case KEY_RETURN:
            bKeyUsed = !m_pView->DoubleClickHdl();
            break;

        case KEY_F2:
            if( !bShift && !bMod1 )
            {
                m_aEditClickPos = Point( -1, -1 );
                EditTimerCall( nullptr );
            }
            else
                bKeyUsed = false;
            break;

        case KEY_F8:
            if( bShift && m_pView->GetSelectionMode()==SelectionMode::Multiple &&
                !(m_nStyle & WB_SIMPLEMODE))
            {
                if( m_aSelEng.IsAlwaysAdding() )
                    m_aSelEng.AddAlways( false );
                else
                    m_aSelEng.AddAlways( true );
            }
            else
                bKeyUsed = false;
            break;

        case KEY_ADD:
            if (!m_pView->IsExpanded(m_pCursor))
                m_pView->Expand(m_pCursor);
            if (bMod1)
                ExpandAll();
            break;

        case KEY_A:
            if( bMod1 )
                SelAllDestrAnch( true );
            else
                bKeyUsed = false;
            break;

        case KEY_SUBTRACT:
            if (m_pView->IsExpanded(m_pCursor))
                m_pView->Collapse(m_pCursor);
            if (bMod1)
                CollapseTo(m_pTree->GetRootLevelParent(m_pCursor));
            break;

        case KEY_MULTIPLY:
            if( bMod1 )
            {
                // only try to expand/collapse if sublist is expandable,
                // otherwise ignore the key press
                if( IsExpandable() )
                {
                    if (!m_pView->IsAllExpanded(m_pCursor))
                    {
                        m_pView->Expand(m_pCursor);
                        ExpandAll();
                    }
                    else
                        CollapseTo(m_pCursor);
                }
            }
            else
                bKeyUsed = false;
            break;

        case KEY_DIVIDE :
            if( bMod1 )
                SelAllDestrAnch( true );
            else
                bKeyUsed = false;
            break;

        case KEY_COMMA :
            if( bMod1 )
                SelAllDestrAnch( false );
            else
                bKeyUsed = false;
            break;

        case KEY_HOME :
            pNewCursor = m_pView->GetModel()->First();

            while( pNewCursor && !IsSelectable(pNewCursor) )
            {
                pNewCursor = m_pView->NextVisible(pNewCursor);
            }

            if( pNewCursor && pNewCursor != m_pCursor )
            {
//              SelAllDestrAnch( false );
                m_aSelEng.CursorPosChanging( bShift, bMod1 );
                SetCursor( pNewCursor );
                if( !IsEntryInView( pNewCursor ) )
                    MakeVisible( pNewCursor );
            }
            else
                bKeyUsed = false;
            break;

        case KEY_END :
            pNewCursor = m_pView->GetModel()->Last();

            while( pNewCursor && !IsSelectable(pNewCursor) )
            {
                pNewCursor = m_pView->PrevVisible(pNewCursor);
            }

            if( pNewCursor && pNewCursor != m_pCursor)
            {
//              SelAllDestrAnch( false );
                m_aSelEng.CursorPosChanging( bShift, bMod1 );
                SetCursor( pNewCursor );
                if( !IsEntryInView( pNewCursor ) )
                    MakeVisible( pNewCursor );
            }
            else
                bKeyUsed = false;
            break;

        case KEY_ESCAPE:
        case KEY_TAB:
        case KEY_DELETE:
        case KEY_BACKSPACE:
            // must not be handled because this quits dialogs and does other magic things...
            // if there are other single keys which should not be handled, they can be added here
            bKeyUsed = false;
            break;

        default:
            // is there any reason why we should eat the events here? The only place where this is called
            // is from SvTreeListBox::KeyInput. If we set bKeyUsed to true here, then the key input
            // is just silenced. However, we want SvLBox::KeyInput to get a chance, to do the QuickSelection
            // handling.
            // (The old code here which intentionally set bKeyUsed to sal_True said this was because of "quick search"
            // handling, but actually there was no quick search handling anymore. We just re-implemented it.)
            // #i31275# / 2009-06-16 / frank.schoenheit@sun.com
            bKeyUsed = false;
            break;
    }
    return bKeyUsed;
}

void SvImpLBox::GetFocus()
{
    if( m_pCursor )
    {
        m_pView->SetEntryFocus( m_pCursor, true );
        ShowCursor( true );
// auskommentiert wg. deselectall
//      if( bSimpleTravel && !pView->IsSelected(pCursor) )
//          pView->Select( pCursor, true );
    }
    if( m_nStyle & WB_HIDESELECTION )
    {
        SvTreeListEntry* pEntry = m_pView->FirstSelected();
        while( pEntry )
        {
            InvalidateEntry( pEntry );
            pEntry = m_pView->NextSelected( pEntry );
        }
    }
}

void SvImpLBox::LoseFocus()
{
    m_aEditIdle.Stop();
    if( m_pCursor )
        m_pView->SetEntryFocus( m_pCursor,false );
    ShowCursor( false );

    if( m_nStyle & WB_HIDESELECTION )
    {
        SvTreeListEntry* pEntry = m_pView ?  m_pView->FirstSelected() : nullptr;
        while( pEntry )
        {
            InvalidateEntry( pEntry );
            pEntry = m_pView->NextSelected( pEntry );
        }
    }
}


// ********************************************************************
// SelectionEngine
// ********************************************************************

void SvImpLBox::SelectEntry( SvTreeListEntry* pEntry, bool bSelect )
{
    m_pView->Select( pEntry, bSelect );
}

ImpLBSelEng::ImpLBSelEng( SvImpLBox* pImpl, SvTreeListBox* pV )
{
    pImp = pImpl;
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
    pImp->m_pAnchor = pImp->m_pCursor;
}

void ImpLBSelEng::DestroyAnchor()
{
    pImp->m_pAnchor = nullptr;
}

void ImpLBSelEng::SetCursorAtPoint(const Point& rPoint, bool bDontSelectAtCursor)
{
    SvTreeListEntry* pNewCursor = pImp->MakePointVisible( rPoint );
    if( pNewCursor )
    {
        // at SimpleTravel, the SetCursor is selected and the select handler is
        // called
        //if( !bDontSelectAtCursor && !pImp->bSimpleTravel )
        //  pImp->SelectEntry( pNewCursor, true );
        pImp->SetCursor( pNewCursor, bDontSelectAtCursor );
    }
}

bool ImpLBSelEng::IsSelectionAtPoint( const Point& rPoint )
{
    SvTreeListEntry* pEntry = pImp->MakePointVisible( rPoint );
    if( pEntry )
        return pView->IsSelected(pEntry);
    return false;
}

void ImpLBSelEng::DeselectAtPoint( const Point& rPoint )
{
    SvTreeListEntry* pEntry = pImp->MakePointVisible( rPoint );
    if( !pEntry )
        return;
    pImp->SelectEntry( pEntry, false );
}

void ImpLBSelEng::DeselectAll()
{
    pImp->SelAllDestrAnch( false, false ); // don't reset SelectionEngine!
    pImp->m_nFlags &= ~LBoxFlags::DeselectAll;
}

// ***********************************************************************
// Selection
// ***********************************************************************

void SvImpLBox::SetAnchorSelection(SvTreeListEntry* pOldCursor,SvTreeListEntry* pNewCursor)
{
    SvTreeListEntry* pEntry;
    sal_uLong nAnchorVisPos = m_pView->GetVisiblePos( m_pAnchor );
    sal_uLong nOldVisPos = m_pView->GetVisiblePos( pOldCursor );
    sal_uLong nNewVisPos = m_pView->GetVisiblePos( pNewCursor );

    if( nOldVisPos > nAnchorVisPos ||
        ( nAnchorVisPos==nOldVisPos && nNewVisPos > nAnchorVisPos) )
    {
        if( nNewVisPos > nOldVisPos )
        {
            pEntry = pOldCursor;
            while( pEntry && pEntry != pNewCursor )
            {
                m_pView->Select( pEntry );
                pEntry = m_pView->NextVisible(pEntry);
            }
            if( pEntry )
                m_pView->Select( pEntry );
            return;
        }

        if( nNewVisPos < nAnchorVisPos )
        {
            pEntry = m_pAnchor;
            while( pEntry && pEntry != pOldCursor )
            {
                m_pView->Select( pEntry, false );
                pEntry = m_pView->NextVisible(pEntry);
            }
            if( pEntry )
                m_pView->Select( pEntry, false );

            pEntry = pNewCursor;
            while( pEntry && pEntry != m_pAnchor )
            {
                m_pView->Select( pEntry );
                pEntry = m_pView->NextVisible(pEntry);
            }
            if( pEntry )
                m_pView->Select( pEntry );
            return;
        }

        if( nNewVisPos < nOldVisPos )
        {
            pEntry = m_pView->NextVisible(pNewCursor);
            while( pEntry && pEntry != pOldCursor )
            {
                m_pView->Select( pEntry, false );
                pEntry = m_pView->NextVisible(pEntry);
            }
            if( pEntry )
                m_pView->Select( pEntry, false );
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
                m_pView->Select( pEntry );
                pEntry = m_pView->NextVisible(pEntry);
            }
            if( pEntry )
                m_pView->Select( pEntry );
            return;
        }

        if( nNewVisPos > nAnchorVisPos )
        {
            pEntry = pOldCursor;
            while( pEntry && pEntry != m_pAnchor )
            {
                m_pView->Select( pEntry, false );
                pEntry = m_pView->NextVisible(pEntry);
            }
            if( pEntry )
                m_pView->Select( pEntry, false );
            pEntry = m_pAnchor;
            while( pEntry && pEntry != pNewCursor )
            {
                m_pView->Select( pEntry );
                pEntry = m_pView->NextVisible(pEntry);
            }
            if( pEntry )
                m_pView->Select( pEntry );
            return;
        }

        if( nNewVisPos > nOldVisPos )
        {
            pEntry = pOldCursor;
            while( pEntry && pEntry != pNewCursor )
            {
                m_pView->Select( pEntry, false );
                pEntry = m_pView->NextVisible(pEntry);
            }
            return;
        }
    }
}

void SvImpLBox::SelAllDestrAnch(
    bool bSelect, bool bDestroyAnchor, bool bSingleSelToo )
{
    SvTreeListEntry* pEntry;
    m_nFlags &= ~LBoxFlags::DeselectAll;
    if( bSelect && m_bSimpleTravel )
    {
        if( m_pCursor && !m_pView->IsSelected( m_pCursor ))
        {
            m_pView->Select( m_pCursor );
        }
        return;
    }
    if( !bSelect && m_pView->GetSelectionCount() == 0 )
    {
        if( m_bSimpleTravel && ( !GetUpdateMode() || !m_pCursor) )
            m_nFlags |= LBoxFlags::DeselectAll;
        return;
    }
    if( bSelect && m_pView->GetSelectionCount() == m_pView->GetEntryCount())
        return;
    if( !bSingleSelToo && m_bSimpleTravel )
        return;

    if( !bSelect && m_pView->GetSelectionCount()==1 && m_pCursor &&
        m_pView->IsSelected( m_pCursor ))
    {
        m_pView->Select( m_pCursor, false );
        if( bDestroyAnchor )
            DestroyAnchor(); // delete anchor & reset SelectionEngine
        else
            m_pAnchor = nullptr; // always delete internal anchor
        return;
    }

    if( m_bSimpleTravel && !m_pCursor && !GetUpdateMode() )
        m_nFlags |= LBoxFlags::DeselectAll;

    ShowCursor( false );
    bool bUpdate = GetUpdateMode();

    m_nFlags |= LBoxFlags::IgnoreSelect; // EntryInserted should not do anything
    pEntry = m_pTree->First();
    while( pEntry )
    {
        if( m_pView->Select( pEntry, bSelect ) )
        {
            if( bUpdate && m_pView->IsEntryVisible(pEntry) )
            {
                tools::Long nY = GetEntryLine( pEntry );
                if( IsLineVisible( nY ) )
                    InvalidateEntry(pEntry);
            }
        }
        pEntry = m_pTree->Next( pEntry );
    }
    m_nFlags &= ~LBoxFlags::IgnoreSelect;

    if( bDestroyAnchor )
        DestroyAnchor(); // delete anchor & reset SelectionEngine
    else
        m_pAnchor = nullptr; // always delete internal anchor
    ShowCursor( true );
}

void SvImpLBox::SetSelectionMode( SelectionMode eSelMode  )
{
    m_aSelEng.SetSelectionMode( eSelMode);
    if( eSelMode == SelectionMode::Single )
        m_bSimpleTravel = true;
    else
        m_bSimpleTravel = false;
    if( (m_nStyle & WB_SIMPLEMODE) && (eSelMode == SelectionMode::Multiple) )
        m_aSelEng.AddAlways( true );
}

// ***********************************************************************
// Drag & Drop
// ***********************************************************************

void SvImpLBox::SetDragDropMode( DragDropMode eDDMode )
{
    if( eDDMode != DragDropMode::NONE )
    {
        m_aSelEng.ExpandSelectionOnMouseMove( false );
        m_aSelEng.EnableDrag( true );
    }
    else
    {
        m_aSelEng.ExpandSelectionOnMouseMove();
        m_aSelEng.EnableDrag( false );
    }
}

void SvImpLBox::BeginDrag()
{
    m_nFlags &= ~LBoxFlags::Filling;
    m_pView->StartDrag( 0, m_aSelEng.GetMousePosPixel() );
}

void SvImpLBox::PaintDDCursor(SvTreeListEntry* pEntry, bool bShow)
{
    if (pEntry)
    {

        SvViewDataEntry* pViewData = m_pView->GetViewData(pEntry);
        pViewData->SetDragTarget(bShow);
#ifdef MACOSX
        // in MacOS we need to draw directly (as we are synchronous) or no invalidation happens
        m_pView->PaintEntry1(*pEntry, GetEntryLine(pEntry), *m_pView->GetOutDev());
#else
        InvalidateEntry(pEntry);
#endif
    }
}

void SvImpLBox::Command( const CommandEvent& rCEvt )
{
    CommandEventId nCommand = rCEvt.GetCommand();

    if( nCommand == CommandEventId::ContextMenu )
        m_aEditIdle.Stop();

    // scroll mouse event?
    if (nCommand == CommandEventId::Wheel ||
        nCommand == CommandEventId::StartAutoScroll ||
        nCommand == CommandEventId::AutoScroll ||
        nCommand == CommandEventId::Gesture)
    {
        if (m_pView->HandleScrollCommand(rCEvt, m_aHorSBar.get(), m_aVerSBar.get()))
            return;
    }

    const Point& rPos = rCEvt.GetMousePosPixel();
    if( rPos.X() < m_aOutputSize.Width() && rPos.Y() < m_aOutputSize.Height() )
        m_aSelEng.Command( rCEvt );
}

tools::Rectangle SvImpLBox::GetVisibleArea() const
{
    Point aPos( m_pView->GetMapMode().GetOrigin() );
    aPos.setX( aPos.X() * -1 );
    tools::Rectangle aRect( aPos, m_aOutputSize );
    return aRect;
}

void SvImpLBox::Invalidate()
{
    m_pView->GetOutDev()->SetClipRegion();
}

void SvImpLBox::SetCurEntry( SvTreeListEntry* pEntry )
{
    if  (  ( m_aSelEng.GetSelectionMode() != SelectionMode::Single )
        && ( m_aSelEng.GetSelectionMode() != SelectionMode::NONE )
        )
        SelAllDestrAnch( false );
    if ( pEntry )
        MakeVisible( pEntry );
    SetCursor( pEntry );
    if ( pEntry && ( m_aSelEng.GetSelectionMode() != SelectionMode::NONE ) )
        m_pView->Select( pEntry );
}

IMPL_LINK_NOARG(SvImpLBox, EditTimerCall, Timer *, void)
{
    if( !m_pView->IsInplaceEditingEnabled() )
        return;

    bool bIsMouseTriggered = m_aEditClickPos.X() >= 0;
    if ( bIsMouseTriggered )
    {
        Point aCurrentMousePos = m_pView->GetPointerPosPixel();
        if  (   ( std::abs( aCurrentMousePos.X() - m_aEditClickPos.X() ) > 5 )
            ||  ( std::abs( aCurrentMousePos.Y() - m_aEditClickPos.Y() ) > 5 )
            )
        {
            return;
        }
    }

    SvTreeListEntry* pEntry = GetCurEntry();
    if( pEntry )
    {
        ShowCursor( false );
        m_pView->ImplEditEntry( pEntry );
        ShowCursor( true );
    }
}

bool SvImpLBox::RequestHelp( const HelpEvent& rHEvt )
{
    if( rHEvt.GetMode() & HelpEventMode::QUICK )
    {
        Point aPos( m_pView->ScreenToOutputPixel( rHEvt.GetMousePosPixel() ));
        if( !GetVisibleArea().Contains( aPos ))
            return false;

        SvTreeListEntry* pEntry = GetEntry( aPos );
        if( pEntry )
        {
            // recalculate text rectangle
            SvLBoxTab* pTab;
            SvLBoxItem* pItem = m_pView->GetItem( pEntry, aPos.X(), &pTab );
            if (!pItem || pItem->GetType() != SvLBoxItemType::String)
                return false;

            aPos = GetEntryPosition( pEntry );
            aPos.setX( m_pView->GetTabPos( pEntry, pTab ) ); //pTab->GetPos();
            Size aSize(pItem->GetWidth(m_pView, pEntry), pItem->GetHeight(m_pView, pEntry));
            SvLBoxTab* pNextTab = NextTab( pTab );
            bool bItemClipped = false;
            // is the item cut off by its right neighbor?
            if( pNextTab && m_pView->GetTabPos(pEntry,pNextTab) < aPos.X()+aSize.Width() )
            {
                aSize.setWidth( pNextTab->GetPos() - pTab->GetPos() );
                bItemClipped = true;
            }
            tools::Rectangle aItemRect( aPos, aSize );

            tools::Rectangle aViewRect( GetVisibleArea() );

            if( bItemClipped || !aViewRect.Contains( aItemRect ) )
            {
                // clip the right edge of the item at the edge of the view
                //if( aItemRect.Right() > aViewRect.Right() )
                //  aItemRect.Right() = aViewRect.Right();

                Point aPt = m_pView->OutputToScreenPixel( aItemRect.TopLeft() );
                aItemRect.SetLeft( aPt.X() );
                aItemRect.SetTop( aPt.Y() );
                aPt = m_pView->OutputToScreenPixel( aItemRect.BottomRight() );
                aItemRect.SetRight( aPt.X() );
                aItemRect.SetBottom( aPt.Y() );

                Help::ShowQuickHelp( m_pView, aItemRect,
                                     static_cast<SvLBoxString*>(pItem)->GetText(), QuickHelpFlags::Left | QuickHelpFlags::VCenter );
                return true;
            }
        }
    }
    return false;
}

SvLBoxTab* SvImpLBox::NextTab( SvLBoxTab const * pTab )
{
    sal_uInt16 nTabCount = m_pView->TabCount();
    if( nTabCount <= 1 )
        return nullptr;
    for( int nTab=0; nTab < (nTabCount-1); nTab++)
    {
        if( m_pView->aTabs[nTab].get() == pTab )
            return m_pView->aTabs[nTab+1].get();
    }
    return nullptr;
}

void SvImpLBox::SetUpdateMode( bool bMode )
{
    if( m_bUpdateMode != bMode )
    {
        m_bUpdateMode = bMode;
        if( m_bUpdateMode )
            UpdateAll( false );
    }
}

void SvImpLBox::SetMostRight( SvTreeListEntry* pEntry )
{
    if( m_pView->nTreeFlags & SvTreeFlags::RECALCTABS )
    {
        m_nFlags |= LBoxFlags::IgnoreChangedTabs;
        m_pView->SetTabs();
        m_nFlags &= ~LBoxFlags::IgnoreChangedTabs;
    }

    sal_uInt16 nLastTab = m_pView->aTabs.size() - 1;
    sal_uInt16 nLastItem = pEntry->ItemCount() - 1;
    if( m_pView->aTabs.empty() || nLastItem == USHRT_MAX )
        return;

    if( nLastItem < nLastTab )
        nLastTab = nLastItem;

    SvLBoxTab* pTab = m_pView->aTabs[ nLastTab ].get();
    SvLBoxItem& rItem = pEntry->GetItem( nLastTab );

    tools::Long nTabPos = m_pView->GetTabPos( pEntry, pTab );

    tools::Long nMaxRight = GetOutputSize().Width();
    Point aPos( m_pView->GetMapMode().GetOrigin() );
    aPos.setX( aPos.X() * -1 ); // conversion document coordinates
    nMaxRight = nMaxRight + aPos.X() - 1;

    tools::Long nNextTab = nTabPos < nMaxRight ? nMaxRight : nMaxRight + 50;
    tools::Long nTabWidth = nNextTab - nTabPos + 1;
    auto nItemSize = rItem.GetWidth(m_pView,pEntry);
    tools::Long nOffset = pTab->CalcOffset( nItemSize, nTabWidth );

    tools::Long nRight = nTabPos + nOffset + nItemSize;
    if( nRight > m_nMostRight )
    {
        m_nMostRight = nRight;
        m_pMostRightEntry = pEntry;
    }
}

void SvImpLBox::FindMostRight()
{
    m_nMostRight = -1;
    m_pMostRightEntry = nullptr;
    if( !m_pView->GetModel() )
        return;

    SvTreeListEntry* pEntry = m_pView->FirstVisible();
    while( pEntry )
    {
        SetMostRight( pEntry );
        pEntry = m_pView->NextVisible( pEntry );
    }
}

void SvImpLBox::FindMostRight( SvTreeListEntry* pParent )
{
    if( !pParent )
        FindMostRight();
    else
        FindMostRight_Impl( pParent );
}

void SvImpLBox::FindMostRight_Impl( SvTreeListEntry* pParent )
{
    SvTreeListEntries& rList = m_pTree->GetChildList( pParent );

    size_t nCount = rList.size();
    for( size_t nCur = 0; nCur < nCount; nCur++ )
    {
        SvTreeListEntry* pChild = rList[nCur].get();
        SetMostRight( pChild );
        if( pChild->HasChildren() && m_pView->IsExpanded( pChild ))
            FindMostRight_Impl( pChild );
    }
}

void SvImpLBox::NotifyTabsChanged()
{
    if( GetUpdateMode() && !(m_nFlags & LBoxFlags::IgnoreChangedTabs ) &&
        m_nCurUserEvent == nullptr )
    {
        m_nCurUserEvent = Application::PostUserEvent(LINK(this,SvImpLBox,MyUserEvent));
    }
}

bool SvImpLBox::IsExpandable() const
{
    return m_pCursor->HasChildren() || m_pCursor->HasChildrenOnDemand();
}

IMPL_LINK(SvImpLBox, MyUserEvent, void*, pArg, void )
{
    m_nCurUserEvent = nullptr;
    if( !pArg )
    {
        m_pView->Invalidate();
        m_pView->PaintImmediately();
    }
    else
    {
        FindMostRight();
        ShowVerSBar();
        m_pView->Invalidate( GetVisibleArea() );
    }
}


void SvImpLBox::StopUserEvent()
{
    if( m_nCurUserEvent != nullptr )
    {
        Application::RemoveUserEvent( m_nCurUserEvent );
        m_nCurUserEvent = nullptr;
    }
}

void SvImpLBox::implInitDefaultNodeImages()
{
    if ( s_pDefCollapsed )
        // assume that all or nothing is initialized
        return;

    s_pDefCollapsed = new Image(StockImage::Yes, RID_BMP_TREENODE_COLLAPSED);
    s_pDefExpanded = new Image(StockImage::Yes, RID_BMP_TREENODE_EXPANDED);
}


const Image& SvImpLBox::GetDefaultExpandedNodeImage( )
{
    implInitDefaultNodeImages();
    return *s_pDefExpanded;
}


const Image& SvImpLBox::GetDefaultCollapsedNodeImage( )
{
    implInitDefaultNodeImages();
    return *s_pDefCollapsed;
}


void SvImpLBox::CallEventListeners( VclEventId nEvent, void* pData )
{
    if ( m_pView )
        m_pView->CallImplEventListeners( nEvent, pData);
}


bool SvImpLBox::IsSelectable( const SvTreeListEntry* pEntry ) const
{
    if( pEntry )
    {
        SvViewDataEntry* pViewDataNewCur = m_pView->GetViewDataEntry(pEntry);
        return (pViewDataNewCur == nullptr) || pViewDataNewCur->IsSelectable();
    }
    else
    {
        return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
