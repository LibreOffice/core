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
#include <osl/diagnose.h>
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
#include <vcl/settings.hxx>
#include <vcl/commandevent.hxx>

#include <svtools/ivctrl.hxx>
#include "imivctl.hxx"
#include <svtools/svmedit.hxx>

#include <algorithm>
#include <memory>
#include <vcl/idle.hxx>

static constexpr auto DRAWTEXT_FLAGS_ICON =
    DrawTextFlags::Center | DrawTextFlags::Top | DrawTextFlags::EndEllipsis |
    DrawTextFlags::Clip | DrawTextFlags::MultiLine | DrawTextFlags::WordBreak | DrawTextFlags::Mnemonic;

#define DRAWTEXT_FLAGS_SMALLICON (DrawTextFlags::Left|DrawTextFlags::EndEllipsis|DrawTextFlags::Clip)

#define EVENTID_SHOW_CURSOR             (reinterpret_cast<void*>(1))
#define EVENTID_ADJUST_SCROLLBARS       (reinterpret_cast<void*>(2))

SvxIconChoiceCtrl_Impl::SvxIconChoiceCtrl_Impl(
    SvtIconChoiceCtrl* pCurView,
    WinBits nWinStyle
) :
    aVerSBar( VclPtr<ScrollBar>::Create(pCurView, WB_DRAG | WB_VSCROLL) ),
    aHorSBar( VclPtr<ScrollBar>::Create(pCurView, WB_DRAG | WB_HSCROLL) ),
    aScrBarBox( VclPtr<ScrollBarBox>::Create(pCurView) ),
    aAutoArrangeIdle ( "svtools contnr SvxIconChoiceCtrl_Impl AutoArrange" ),
    aDocRectChangedIdle ( "svtools contnr SvxIconChoiceCtrl_Impl DocRectChanged" ),
    aVisRectChangedIdle ( "svtools contnr SvxIconChoiceCtrl_Impl VisRectChanged" ),
    aCallSelectHdlIdle ( "svtools contnr SvxIconChoiceCtrl_Impl CallSelectHdl" ),
    aImageSize( 32 * pCurView->GetDPIScaleFactor(), 32 * pCurView->GetDPIScaleFactor())
{
    bChooseWithCursor = false;
    pEntryPaintDev = nullptr;
    pCurHighlightFrame = nullptr;
    pAnchor = nullptr;
    pHdlEntry = nullptr;
    pHead = nullptr;
    pCursor = nullptr;
    bUpdateMode = true;
    bHighlightFramePressed = false;
    eSelectionMode = SelectionMode::Multiple;
    pView = pCurView;
    ePositionMode = SvxIconChoiceCtrlPositionMode::Free;
    SetStyle( nWinStyle );
    nFlags = IconChoiceFlags::NONE;
    nUserEventAdjustScrBars = nullptr;
    nMaxVirtWidth = DEFAULT_MAX_VIRT_WIDTH;
    nMaxVirtHeight = DEFAULT_MAX_VIRT_HEIGHT;
    pDDDev = nullptr;
    pDDBufDev = nullptr;
    pDDTempDev = nullptr;
    eTextMode = SvxIconChoiceCtrlTextMode::Short;
    pImpCursor.reset( new IcnCursor_Impl( this ) );
    pGridMap.reset( new IcnGridMap_Impl( this ) );

    aVerSBar->SetScrollHdl( LINK( this, SvxIconChoiceCtrl_Impl, ScrollUpDownHdl ) );
    aHorSBar->SetScrollHdl( LINK( this, SvxIconChoiceCtrl_Impl, ScrollLeftRightHdl ) );

    nHorSBarHeight = aHorSBar->GetSizePixel().Height();
    nVerSBarWidth = aVerSBar->GetSizePixel().Width();

    aAutoArrangeIdle.SetPriority( TaskPriority::HIGH_IDLE );
    aAutoArrangeIdle.SetInvokeHandler(LINK(this,SvxIconChoiceCtrl_Impl,AutoArrangeHdl));
    aAutoArrangeIdle.SetDebugName( "svtools::SvxIconChoiceCtrl_Impl aAutoArrangeIdle" );

    aCallSelectHdlIdle.SetPriority( TaskPriority::LOWEST );
    aCallSelectHdlIdle.SetInvokeHandler( LINK(this,SvxIconChoiceCtrl_Impl,CallSelectHdlHdl));
    aCallSelectHdlIdle.SetDebugName( "svtools::SvxIconChoiceCtrl_Impl aCallSelectHdlIdle" );

    aDocRectChangedIdle.SetPriority( TaskPriority::HIGH_IDLE );
    aDocRectChangedIdle.SetInvokeHandler(LINK(this,SvxIconChoiceCtrl_Impl,DocRectChangedHdl));
    aDocRectChangedIdle.SetDebugName( "svtools::SvxIconChoiceCtrl_Impl aDocRectChangedIdle" );

    aVisRectChangedIdle.SetPriority( TaskPriority::HIGH_IDLE );
    aVisRectChangedIdle.SetInvokeHandler(LINK(this,SvxIconChoiceCtrl_Impl,VisRectChangedHdl));
    aVisRectChangedIdle.SetDebugName( "svtools::SvxIconChoiceCtrl_Impl aVisRectChangedIdle" );

    Clear( true );
    Size gridSize(100,70);
    if(pView->GetDPIScaleFactor() > 1)
    {
      gridSize.setHeight( gridSize.Height() * ( pView->GetDPIScaleFactor()) );
    }
    SetGrid(gridSize);
}

SvxIconChoiceCtrl_Impl::~SvxIconChoiceCtrl_Impl()
{
    Clear(false);
    CancelUserEvents();
    pImpCursor.reset();
    pGridMap.reset();
    pDDDev.disposeAndClear();
    pDDBufDev.disposeAndClear();
    pDDTempDev.disposeAndClear();
    pEntryPaintDev.disposeAndClear();
    ClearSelectedRectList();
    ClearColumnList();
    aVerSBar.disposeAndClear();
    aHorSBar.disposeAndClear();
    aScrBarBox.disposeAndClear();
}

void SvxIconChoiceCtrl_Impl::Clear( bool bInCtor )
{
    nSelectionCount = 0;
    pCurHighlightFrame = nullptr;
    CancelUserEvents();
    ShowCursor( false );
    bBoundRectsDirty = false;
    nMaxBoundHeight = 0;

    pCursor = nullptr;
    if( !bInCtor )
    {
        pImpCursor->Clear();
        pGridMap->Clear();
        aVirtOutputSize.setWidth( 0 );
        aVirtOutputSize.setHeight( 0 );
        Size aSize( pView->GetOutputSizePixel() );
        nMaxVirtWidth = aSize.Width() - nVerSBarWidth;
        if( nMaxVirtWidth <= 0 )
            nMaxVirtWidth = DEFAULT_MAX_VIRT_WIDTH;
        nMaxVirtHeight = aSize.Height() - nHorSBarHeight;
        if( nMaxVirtHeight <= 0 )
            nMaxVirtHeight = DEFAULT_MAX_VIRT_HEIGHT;
        maZOrderList.clear();
        SetOrigin( Point() );
        if( bUpdateMode )
            pView->Invalidate(InvalidateFlags::NoChildren);
    }
    AdjustScrollBars();
    maEntries.clear();
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
        eSelectionMode = SelectionMode::NONE;
    if( !(nWinStyle & (WB_ALIGN_TOP | WB_ALIGN_LEFT)))
        nWinBits |= WB_ALIGN_LEFT;
    if( nWinStyle & WB_DETAILS )
    {
        if (!m_pColumns)
            SetColumn( 0, SvxIconChoiceCtrlColumnInfo() );
    }
}

IMPL_LINK( SvxIconChoiceCtrl_Impl, ScrollUpDownHdl, ScrollBar*, pScrollBar, void )
{
    // arrow up: delta=-1; arrow down: delta=+1
    Scroll( 0, pScrollBar->GetDelta() );
}

IMPL_LINK( SvxIconChoiceCtrl_Impl, ScrollLeftRightHdl, ScrollBar*, pScrollBar, void )
{
    // arrow left: delta=-1; arrow right: delta=+1
    Scroll( pScrollBar->GetDelta(), 0 );
}

void SvxIconChoiceCtrl_Impl::FontModified()
{
    pDDDev.disposeAndClear();
    pDDBufDev.disposeAndClear();
    pDDTempDev.disposeAndClear();
    pEntryPaintDev.disposeAndClear();
    SetDefaultTextSize();
    ShowCursor( false );
    ShowCursor( true );
}

void SvxIconChoiceCtrl_Impl::InsertEntry( std::unique_ptr<SvxIconChoiceCtrlEntry> pEntry1, size_t nPos)
{
    auto pEntry = pEntry1.get();

    if ( nPos < maEntries.size() ) {
        maEntries.insert( maEntries.begin() + nPos, std::move(pEntry1) );
    } else {
        maEntries.push_back( std::move(pEntry1) );
    }

    if( pHead )
        pEntry->SetBacklink( pHead->pblink );

    if( (nFlags & IconChoiceFlags::EntryListPosValid) && nPos >= maEntries.size() - 1 )
        pEntry->nPos = maEntries.size() - 1;
    else
        nFlags &= ~IconChoiceFlags::EntryListPosValid;

    maZOrderList.push_back( pEntry );
    pImpCursor->Clear();

    // If the UpdateMode is true, don't set all bounding rectangles to
    // 'to be checked', but only the bounding rectangle of the new entry.
    // Thus, don't call InvalidateBoundingRect!
    pEntry->aRect.SetRight( LONG_MAX );
    if( bUpdateMode )
    {
        FindBoundingRect( pEntry );
        tools::Rectangle aOutputArea( GetOutputRect() );
        pGridMap->OccupyGrids( pEntry );
        if( !aOutputArea.IsOver( pEntry->aRect ) )
            return; // is invisible
        pView->Invalidate( pEntry->aRect );
    }
    else
        InvalidateBoundingRect( pEntry->aRect );
}

void SvxIconChoiceCtrl_Impl::CreateAutoMnemonics( MnemonicGenerator* _pGenerator )
{
    std::unique_ptr< MnemonicGenerator > pAutoDeleteOwnGenerator;
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

tools::Rectangle SvxIconChoiceCtrl_Impl::GetOutputRect() const
{
    Point aOrigin( pView->GetMapMode().GetOrigin() );
    aOrigin *= -1;
    return tools::Rectangle( aOrigin, aOutputSize );
}

void SvxIconChoiceCtrl_Impl::SetListPositions()
{
    if( nFlags & IconChoiceFlags::EntryListPosValid )
        return;

    size_t nCount = maEntries.size();
    for( size_t nCur = 0; nCur < nCount; nCur++ )
    {
        maEntries[ nCur ]->nPos = nCur;
    }
    nFlags |= IconChoiceFlags::EntryListPosValid;
}

void SvxIconChoiceCtrl_Impl::SelectEntry( SvxIconChoiceCtrlEntry* pEntry, bool bSelect,
    bool bAdd )
{
    if( eSelectionMode == SelectionMode::NONE )
        return;

    if( !bAdd )
    {
        if ( !( nFlags & IconChoiceFlags::ClearingSelection ) )
        {
            nFlags |= IconChoiceFlags::ClearingSelection;
            DeselectAllBut( pEntry );
            nFlags &= ~IconChoiceFlags::ClearingSelection;
        }
    }
    if( pEntry->IsSelected() == bSelect )
        return;

    pHdlEntry = pEntry;
    SvxIconViewFlags nEntryFlags = pEntry->GetFlags();
    if( bSelect )
    {
        nEntryFlags |= SvxIconViewFlags::SELECTED;
        pEntry->AssignFlags( nEntryFlags );
        nSelectionCount++;
        CallSelectHandler();
    }
    else
    {
        nEntryFlags &= ~SvxIconViewFlags::SELECTED;
        pEntry->AssignFlags( nEntryFlags );
        nSelectionCount--;
        CallSelectHandler();
    }
    EntrySelected( pEntry, bSelect );
}

void SvxIconChoiceCtrl_Impl::EntrySelected(SvxIconChoiceCtrlEntry* pEntry, bool bSelect)
{
    // When using SingleSelection, make sure that the cursor is always placed
    // over the (only) selected entry. (But only if a cursor exists.)
    if (bSelect && pCursor &&
        eSelectionMode == SelectionMode::Single &&
        pEntry != pCursor)
    {
        SetCursor(pEntry);
    }

    // Not when dragging though, else the loop in SelectRect doesn't work
    // correctly!
    if (!(nFlags & IconChoiceFlags::SelectingRect))
        ToTop(pEntry);
    if (bUpdateMode)
    {
        if (pEntry == pCursor)
            ShowCursor(false);
        pView->Invalidate(CalcFocusRect(pEntry));
        if (pEntry == pCursor)
            ShowCursor(true);
    }

    // #i101012# emit vcl event LISTBOX_SELECT only in case that the given entry is selected.
    if (bSelect)
    {
        CallEventListeners(VclEventId::ListboxSelect, pEntry);
    }
}

void SvxIconChoiceCtrl_Impl::ResetVirtSize()
{
    aVirtOutputSize.setWidth( 0 );
    aVirtOutputSize.setHeight( 0 );
    const size_t nCount = maEntries.size();
    for( size_t nCur = 0; nCur < nCount; nCur++ )
    {
        SvxIconChoiceCtrlEntry* pCur = maEntries[ nCur ].get();
        pCur->ClearFlags( SvxIconViewFlags::POS_MOVED );
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
                aRealOutputSize, static_cast<sal_uInt16>(nGridDX), static_cast<sal_uInt16>(nGridDY) );
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

void SvxIconChoiceCtrl_Impl::AdjustVirtSize( const tools::Rectangle& rRect )
{
    long nHeightOffs = 0;
    long nWidthOffs = 0;

    if( aVirtOutputSize.Width() < (rRect.Right()+LROFFS_WINBORDER) )
        nWidthOffs = (rRect.Right()+LROFFS_WINBORDER) - aVirtOutputSize.Width();

    if( aVirtOutputSize.Height() < (rRect.Bottom()+TBOFFS_WINBORDER) )
        nHeightOffs = (rRect.Bottom()+TBOFFS_WINBORDER) - aVirtOutputSize.Height();

    if( !(nWidthOffs || nHeightOffs) )
        return;

    Range aRange;
    aVirtOutputSize.AdjustWidth(nWidthOffs );
    aRange.Max() = aVirtOutputSize.Width();
    aHorSBar->SetRange( aRange );

    aVirtOutputSize.AdjustHeight(nHeightOffs );
    aRange.Max() = aVirtOutputSize.Height();
    aVerSBar->SetRange( aRange );

    pImpCursor->Clear();
    pGridMap->OutputSizeChanged();
    AdjustScrollBars();
    DocRectChanged();
}

void SvxIconChoiceCtrl_Impl::InitPredecessors()
{
    DBG_ASSERT(!pHead,"SvxIconChoiceCtrl_Impl::InitPredecessors() >> Already initialized");
    size_t nCount = maEntries.size();
    if( nCount )
    {
        SvxIconChoiceCtrlEntry* pPrev = maEntries[ 0 ].get();
        for( size_t nCur = 1; nCur <= nCount; nCur++ )
        {
            pPrev->ClearFlags( SvxIconViewFlags::POS_LOCKED | SvxIconViewFlags::POS_MOVED );

            SvxIconChoiceCtrlEntry* pNext;
            if( nCur == nCount )
                pNext = maEntries[ 0 ].get();
            else
                pNext = maEntries[ nCur ].get();
            pPrev->pflink = pNext;
            pNext->pblink = pPrev;
            pPrev = pNext;
        }
        pHead = maEntries[ 0 ].get();
    }
    else
        pHead = nullptr;
}

void SvxIconChoiceCtrl_Impl::ClearPredecessors()
{
    if( pHead )
    {
        size_t nCount = maEntries.size();
        for( size_t nCur = 0; nCur < nCount; nCur++ )
        {
            SvxIconChoiceCtrlEntry* pCur = maEntries[ nCur ].get();
            pCur->pflink = nullptr;
            pCur->pblink = nullptr;
        }
        pHead = nullptr;
    }
}

void SvxIconChoiceCtrl_Impl::Arrange( bool bKeepPredecessors, long nSetMaxVirtWidth, long nSetMaxVirtHeight )
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

void SvxIconChoiceCtrl_Impl::ImpArrange( bool bKeepPredecessors )
{
    static Point aEmptyPoint;

    bool bOldUpdate = bUpdateMode;
    tools::Rectangle aCurOutputArea( GetOutputRect() );
    if( (nWinBits & WB_SMART_ARRANGE) && aCurOutputArea.TopLeft() != aEmptyPoint )
        bUpdateMode = false;
    aAutoArrangeIdle.Stop();
    nFlags |= IconChoiceFlags::Arranging;
    ShowCursor( false );
    ResetVirtSize();
    if( !bKeepPredecessors )
        ClearPredecessors();
    bBoundRectsDirty = false;
    SetOrigin( Point() );
    VisRectChanged();
    RecalcAllBoundingRectsSmart();
    // TODO: the invalidation in the detail view should be more intelligent
    //if( !(nWinBits & WB_DETAILS ))
    pView->Invalidate( InvalidateFlags::NoChildren );
    nFlags &= ~IconChoiceFlags::Arranging;
    if( (nWinBits & WB_SMART_ARRANGE) && aCurOutputArea.TopLeft() != aEmptyPoint )
    {
        MakeVisible( aCurOutputArea );
        SetUpdateMode( bOldUpdate );
    }
    ShowCursor( true );
}

void SvxIconChoiceCtrl_Impl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
#if defined(OV_DRAWGRID)
    Color aOldColor (rRenderContext.GetLineColor());
    Color aCOL_BLACK);
    rRenderContext.SetLineColor( aColor );
    Point aOffs(rRenderContext.GetMapMode().GetOrigin());
    Size aXSize(GetOutputSizePixel());
    {
        Point aStart(LROFFS_WINBORDER, 0);
        Point aEnd(LROFFS_WINBORDER, aXSize.Height());
        aStart -= aOffs;
        aEnd -= aOffs;
        rRenderContext.DrawLine(aStart, aEnd);
    }
    {
        Point aStart(0, TBOFFS_WINBORDER);
        Point aEnd(aXSize.Width(), TBOFFS_WINBORDER);
        aStart -= aOffs;
        aEnd -= aOffs;
        rRenderContext.DrawLine(aStart, aEnd);
    }

    for (long nDX = nGridDX; nDX <= aXSize.Width(); nDX += nGridDX)
    {
        Point aStart( nDX+LROFFS_WINBORDER, 0 );
        Point aEnd( nDX+LROFFS_WINBORDER, aXSize.Height());
        aStart -= aOffs;
        aEnd -= aOffs;
        rRenderContext.DrawLine(aStart, aEnd);
    }
    for (long nDY = nGridDY; nDY <= aXSize.Height(); nDY += nGridDY)
    {
        Point aStart(0, nDY + TBOFFS_WINBORDER);
        Point aEnd(aXSize.Width(), nDY + TBOFFS_WINBORDER);
        aStart -= aOffs;
        aEnd -= aOffs;
        rRenderContext.DrawLine(aStart, aEnd);
    }
    rRenderContext.SetLineColor(aOldColor);
#endif

    if (!maEntries.size())
        return;
    if (!pCursor)
    {
        // set cursor to item with focus-flag
        bool bfound = false;
        for (sal_Int32 i = 0; i < pView->GetEntryCount() && !bfound; i++)
        {
            SvxIconChoiceCtrlEntry* pEntry = pView->GetEntry(i);
            if (pEntry->IsFocused())
            {
                pCursor = pEntry;
                bfound = true;
            }
        }

        if (!bfound)
            pCursor = maEntries[ 0 ].get();
    }

    size_t nCount = maZOrderList.size();
    if (!nCount)
        return;

    rRenderContext.Push(PushFlags::CLIPREGION);
    rRenderContext.SetClipRegion(vcl::Region(rRect));

    std::vector< SvxIconChoiceCtrlEntry* > aNewZOrderList;
    std::vector< SvxIconChoiceCtrlEntry* > aPaintedEntries;

    size_t nPos = 0;
    while(nCount)
    {
        SvxIconChoiceCtrlEntry* pEntry = maZOrderList[nPos];
        const tools::Rectangle& rBoundRect = GetEntryBoundRect(pEntry);
        if (rRect.IsOver(rBoundRect))
        {
            PaintEntry(pEntry, rBoundRect.TopLeft(), rRenderContext);
            // set entries to Top if they are being repainted
            aPaintedEntries.push_back(pEntry);
        }
        else
            aNewZOrderList.push_back(pEntry);

        nCount--;
        nPos++;
    }
    maZOrderList = std::move( aNewZOrderList );
    maZOrderList.insert(maZOrderList.end(), aPaintedEntries.begin(), aPaintedEntries.end());

    rRenderContext.Pop();
}

void SvxIconChoiceCtrl_Impl::RepaintSelectedEntries()
{
    const size_t nCount = maZOrderList.size();
    if (!nCount)
        return;

    tools::Rectangle aOutRect(GetOutputRect());
    for (size_t nCur = 0; nCur < nCount; nCur++)
    {
        SvxIconChoiceCtrlEntry* pEntry = maZOrderList[nCur];
        if (pEntry->GetFlags() & SvxIconViewFlags::SELECTED)
        {
            const tools::Rectangle& rBoundRect = GetEntryBoundRect(pEntry);
            if (aOutRect.IsOver(rBoundRect))
                pView->Invalidate(rBoundRect);
        }
    }
}

void SvxIconChoiceCtrl_Impl::InitScrollBarBox()
{
    aScrBarBox->SetSizePixel( Size(nVerSBarWidth-1, nHorSBarHeight-1) );
    Size aSize( pView->GetOutputSizePixel() );
    aScrBarBox->SetPosPixel( Point(aSize.Width()-nVerSBarWidth+1, aSize.Height()-nHorSBarHeight+1));
}

bool SvxIconChoiceCtrl_Impl::MouseButtonDown( const MouseEvent& rMEvt)
{
    bool bHandled = true;
    bHighlightFramePressed = false;
    bool bGotFocus = (!pView->HasFocus() && !(nWinBits & WB_NOPOINTERFOCUS));
    if( !(nWinBits & WB_NOPOINTERFOCUS) )
        pView->GrabFocus();

    Point aDocPos( rMEvt.GetPosPixel() );
    if(aDocPos.X()>=aOutputSize.Width() || aDocPos.Y()>=aOutputSize.Height())
        return false;
    ToDocPos( aDocPos );
    SvxIconChoiceCtrlEntry* pEntry = GetEntry( aDocPos, true );
    if( pEntry )
        MakeEntryVisible( pEntry, false );

    if( rMEvt.IsShift() && eSelectionMode != SelectionMode::Single )
    {
        if( pEntry )
            SetCursor_Impl( pCursor, pEntry, rMEvt.IsMod1(), rMEvt.IsShift() );
        return true;
    }

    if( pAnchor && (rMEvt.IsShift() || rMEvt.IsMod1())) // keyboard selection?
    {
        DBG_ASSERT(eSelectionMode != SelectionMode::Single,"Invalid selection mode");
        if( rMEvt.IsMod1() )
            nFlags |= IconChoiceFlags::AddMode;

        if( rMEvt.IsShift() )
        {
            tools::Rectangle aRect( GetEntryBoundRect( pAnchor ));
            if( pEntry )
                aRect.Union( GetEntryBoundRect( pEntry ) );
            else
            {
                tools::Rectangle aTempRect( aDocPos, Size(1,1));
                aRect.Union( aTempRect );
            }
            aCurSelectionRect = aRect;
            SelectRect( aRect, bool(nFlags & IconChoiceFlags::AddMode), &aSelectedRectList );
        }
        else if( rMEvt.IsMod1() )
        {
            AddSelectedRect( aCurSelectionRect );
            pAnchor = nullptr;
            aCurSelectionRect.SetPos( aDocPos );
        }

        if( !pEntry && !(nWinBits & WB_NODRAGSELECTION))
            pView->StartTracking( StartTrackingFlags::ScrollRepeat );
        return true;
    }
    else
    {
        if( !pEntry )
        {
            if( eSelectionMode == SelectionMode::Multiple )
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
                    nFlags |= IconChoiceFlags::AddMode;
                aCurSelectionRect.SetPos( aDocPos );
                pView->StartTracking( StartTrackingFlags::ScrollRepeat );
            }
            else
                bHandled = false;
            return bHandled;
        }
    }
    bool bSelected = pEntry->IsSelected();

    if( rMEvt.GetClicks() == 2 )
    {
        DeselectAllBut( pEntry );
        SelectEntry( pEntry, true, false );
        pHdlEntry = pEntry;
        pView->ClickIcon();
    }
    else
    {
        // Inplace-Editing ?
        if( rMEvt.IsMod2() )  // Alt?
        {
        }
        else if( eSelectionMode == SelectionMode::Single )
        {
            DeselectAllBut( pEntry );
            SetCursor( pEntry );
        }
        else if( eSelectionMode == SelectionMode::NONE )
        {
            if( rMEvt.IsLeft() && (nWinBits & WB_HIGHLIGHTFRAME) )
            {
                pCurHighlightFrame = nullptr; // force repaint of frame
                bHighlightFramePressed = true;
                SetEntryHighlightFrame( pEntry, true );
            }
        }
        else
        {
            if( !rMEvt.GetModifier() && rMEvt.IsLeft() )
            {
                if( !bSelected )
                {
                    DeselectAllBut( pEntry );
                    SetCursor( pEntry );
                    SelectEntry( pEntry, true, false );
                }
                else
                {
                    // deselect only in the Up, if the Move happened via D&D!
                    nFlags |= IconChoiceFlags::DownDeselect;
                }
            }
            else if( rMEvt.IsMod1() )
                nFlags |= IconChoiceFlags::DownCtrl;
        }
    }
    return bHandled;
}

bool SvxIconChoiceCtrl_Impl::MouseButtonUp( const MouseEvent& rMEvt )
{
    bool bHandled = false;
    if( rMEvt.IsRight() && (nFlags & (IconChoiceFlags::DownCtrl | IconChoiceFlags::DownDeselect) ))
    {
        nFlags &= ~IconChoiceFlags(IconChoiceFlags::DownCtrl | IconChoiceFlags::DownDeselect);
        bHandled = true;
    }

    Point aDocPos( rMEvt.GetPosPixel() );
    ToDocPos( aDocPos );
    SvxIconChoiceCtrlEntry* pDocEntry = GetEntry( aDocPos );
    if( pDocEntry )
    {
        if( nFlags & IconChoiceFlags::DownCtrl )
        {
            // Ctrl & MultiSelection
            ToggleSelection( pDocEntry );
            SetCursor( pDocEntry );
            bHandled = true;
        }
        else if( nFlags & IconChoiceFlags::DownDeselect )
        {
            DeselectAllBut( pDocEntry );
            SetCursor( pDocEntry );
            SelectEntry( pDocEntry, true, false );
            bHandled = true;
        }
    }

    nFlags &= ~IconChoiceFlags(IconChoiceFlags::DownCtrl | IconChoiceFlags::DownDeselect);

    if((nWinBits & WB_HIGHLIGHTFRAME) && bHighlightFramePressed && pCurHighlightFrame)
    {
        bHandled = true;
        SvxIconChoiceCtrlEntry* pEntry = pCurHighlightFrame;
        pCurHighlightFrame = nullptr; // force repaint of frame
        bHighlightFramePressed = false;
        SetEntryHighlightFrame( pEntry, true );

        pHdlEntry = pCurHighlightFrame;
        pView->ClickIcon();

        // set focus on Icon
        SvxIconChoiceCtrlEntry* pOldCursor = pCursor;
        SetCursor_Impl( pOldCursor, pHdlEntry, false, false );

        pHdlEntry = nullptr;
    }
    return bHandled;
}

bool SvxIconChoiceCtrl_Impl::MouseMove( const MouseEvent& rMEvt )
{
    const Point aDocPos( pView->PixelToLogic(rMEvt.GetPosPixel()) );

    if( pView->IsTracking() )
        return false;
    else if( nWinBits & WB_HIGHLIGHTFRAME )
    {
        SvxIconChoiceCtrlEntry* pEntry = GetEntry( aDocPos, true );
        SetEntryHighlightFrame( pEntry, false );
    }
    else
        return false;
    return true;
}

void SvxIconChoiceCtrl_Impl::SetCursor_Impl( SvxIconChoiceCtrlEntry* pOldCursor,
    SvxIconChoiceCtrlEntry* pNewCursor, bool bMod1, bool bShift )
{
    if( !pNewCursor )
        return;

    SvxIconChoiceCtrlEntry* pFilterEntry = nullptr;
    bool bDeselectAll = false;
    if( eSelectionMode != SelectionMode::Single )
    {
        if( !bMod1 && !bShift )
            bDeselectAll = true;
        else if( bShift && !bMod1 && !pAnchor )
        {
            bDeselectAll = true;
            pFilterEntry = pOldCursor;
        }
    }
    if( bDeselectAll )
        DeselectAllBut( pFilterEntry );
    ShowCursor( false );
    MakeEntryVisible( pNewCursor );
    SetCursor( pNewCursor );
    if( bMod1 && !bShift )
    {
        if( pAnchor )
        {
            AddSelectedRect( pAnchor, pOldCursor );
            pAnchor = nullptr;
        }
    }
    else if( bShift )
    {
        if( !pAnchor )
            pAnchor = pOldCursor;
        if ( nWinBits & WB_ALIGN_LEFT )
            SelectRange( pAnchor, pNewCursor, bool(nFlags & IconChoiceFlags::AddMode) );
        else
            SelectRect(pAnchor,pNewCursor, bool(nFlags & IconChoiceFlags::AddMode), &aSelectedRectList);
    }
    else
    {
        SelectEntry( pCursor, true, false );
        aCurSelectionRect = GetEntryBoundRect( pCursor );
        CallEventListeners( VclEventId::ListboxSelect, pCursor );
    }
}

bool SvxIconChoiceCtrl_Impl::KeyInput( const KeyEvent& rKEvt )
{
    bool bMod2 = rKEvt.GetKeyCode().IsMod2();
    sal_Unicode cChar = rKEvt.GetCharCode();
    sal_uLong nPos = sal_uLong(-1);
    if ( bMod2 && cChar && IsMnemonicChar( cChar, nPos ) )
    {
        // shortcut is clicked
        SvxIconChoiceCtrlEntry* pNewCursor = GetEntry( nPos );
        SvxIconChoiceCtrlEntry* pOldCursor = pCursor;
        if ( pNewCursor != pOldCursor )
            SetCursor_Impl( pOldCursor, pNewCursor, false, false );
        return true;
    }

    if ( bMod2 )
        // no actions with <ALT>
        return false;

    bool bKeyUsed = true;
    bool bMod1 = rKEvt.GetKeyCode().IsMod1();
    bool bShift = rKEvt.GetKeyCode().IsShift();

    if( eSelectionMode == SelectionMode::Single || eSelectionMode == SelectionMode::NONE)
    {
        bShift = false;
        bMod1 = false;
    }

    if( bMod1 )
        nFlags |= IconChoiceFlags::AddMode;

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
                    pNewCursor = pImpCursor->GoUpDown(pCursor,false);
                else
                    pNewCursor = pImpCursor->GoPageUpDown(pCursor,false);
                SetCursor_Impl( pOldCursor, pNewCursor, bMod1, bShift );
                if( !pNewCursor )
                {
                    tools::Rectangle aRect( GetEntryBoundRect( pCursor ) );
                    if( aRect.Top())
                    {
                        aRect.AdjustBottom( -(aRect.Top()) );
                        aRect.SetTop( 0 );
                        MakeVisible( aRect );
                    }
                }

                if ( bChooseWithCursor && pNewCursor != nullptr )
                {
                    pHdlEntry = pNewCursor;//GetCurEntry();
                    pCurHighlightFrame = pHdlEntry;
                    pView->ClickIcon();
                    pCurHighlightFrame = nullptr;
                }
            }
            break;

        case KEY_DOWN:
        case KEY_PAGEDOWN:
            if( pCursor )
            {
                if( nCode == KEY_DOWN )
                    pNewCursor=pImpCursor->GoUpDown( pCursor,true );
                else
                    pNewCursor=pImpCursor->GoPageUpDown( pCursor,true );
                SetCursor_Impl( pOldCursor, pNewCursor, bMod1, bShift );

                if ( bChooseWithCursor && pNewCursor != nullptr)
                {
                    pHdlEntry = pNewCursor;//GetCurEntry();
                    pCurHighlightFrame = pHdlEntry;
                    pView->ClickIcon();
                    pCurHighlightFrame = nullptr;
                }
            }
            break;

        case KEY_RIGHT:
            if( pCursor )
            {
                pNewCursor=pImpCursor->GoLeftRight(pCursor,true );
                SetCursor_Impl( pOldCursor, pNewCursor, bMod1, bShift );
            }
            break;

        case KEY_LEFT:
            if( pCursor )
            {
                MakeEntryVisible( pCursor );
                pNewCursor = pImpCursor->GoLeftRight(pCursor,false );
                SetCursor_Impl( pOldCursor, pNewCursor, bMod1, bShift );
                if( !pNewCursor )
                {
                    tools::Rectangle aRect( GetEntryBoundRect(pCursor));
                    if( aRect.Left() )
                    {
                        aRect.AdjustRight( -(aRect.Left()) );
                        aRect.SetLeft( 0 );
                        MakeVisible( aRect );
                    }
                }
            }
            break;

        case KEY_F2:
            if( bMod1 || bShift )
                bKeyUsed = false;
            break;

        case KEY_F8:
            if( rKEvt.GetKeyCode().IsShift() )
            {
                if( nFlags & IconChoiceFlags::AddMode )
                    nFlags &= ~IconChoiceFlags::AddMode;
                else
                    nFlags |= IconChoiceFlags::AddMode;
            }
            else
                bKeyUsed = false;
            break;

        case KEY_SPACE:
            if( pCursor && eSelectionMode != SelectionMode::Single )
            {
                if( !bMod1 )
                {
                    //SelectAll( false );
                    SetNoSelection();
                    ClearSelectedRectList();

                    // click Icon with spacebar
                    SetEntryHighlightFrame( GetCurEntry(), true );
                    pView->ClickIcon();
                    pHdlEntry = pCurHighlightFrame;
                    pCurHighlightFrame=nullptr;
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
                    pView->SetEntryTextMode( SvxIconChoiceCtrlTextMode::Full, pCursor );
            }
            if( rKEvt.GetKeyCode().IsMod1() )
            {
                if( pCursor )
                    pView->SetEntryTextMode( SvxIconChoiceCtrlTextMode::Short, pCursor );
            }
            break;
#endif

        case KEY_ADD:
        case KEY_DIVIDE :
        case KEY_A:
            if( bMod1 && (eSelectionMode != SelectionMode::Single))
                SelectAll();
            else
                bKeyUsed = false;
            break;

        case KEY_SUBTRACT:
        case KEY_COMMA :
            if( bMod1 )
                SetNoSelection();
            else
                bKeyUsed = false;
            break;

        case KEY_RETURN:
            if( !bMod1 )
                bKeyUsed = false;
            break;

        case KEY_END:
            if( pCursor )
            {
                pNewCursor = maEntries.back().get();
                SetCursor_Impl( pOldCursor, pNewCursor, bMod1, bShift );
            }
            break;

        case KEY_HOME:
            if( pCursor )
            {
                pNewCursor = maEntries[ 0 ].get();
                SetCursor_Impl( pOldCursor, pNewCursor, bMod1, bShift );
            }
            break;

        default:
            bKeyUsed = false;

    }
    return bKeyUsed;
}

// recalculate TopLeft of scrollbars (but not their sizes!)
void SvxIconChoiceCtrl_Impl::PositionScrollBars( long nRealWidth, long nRealHeight )
{
    // horizontal scrollbar
    Point aPos( 0, nRealHeight );
    aPos.AdjustY( -nHorSBarHeight );

    if( aHorSBar->GetPosPixel() != aPos )
        aHorSBar->SetPosPixel( aPos );

    // vertical scrollbar
    aPos.setX( nRealWidth ); aPos.setY( 0 );
    aPos.AdjustX( -nVerSBarWidth );
    aPos.AdjustX( 1 );
    aPos.AdjustY( -1 );

    if( aVerSBar->GetPosPixel() != aPos )
        aVerSBar->SetPosPixel( aPos );
}

void SvxIconChoiceCtrl_Impl::AdjustScrollBars()
{
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

    bool bVerSBar = ( nWinBits & WB_VSCROLL ) != 0;
    bool bHorSBar = ( nWinBits & WB_HSCROLL ) != 0;
    bool bNoVerSBar = ( nWinBits & WB_NOVSCROLL ) != 0;
    bool bNoHorSBar = ( nWinBits & WB_NOHSCROLL ) != 0;

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
            }
        }
    }

    // size vertical scrollbar
    long nThumb = aVerSBar->GetThumbPos();
    Size aSize( nVerSBarWidth, nRealHeight );
    aSize.AdjustHeight(2 );
    if( aSize != aVerSBar->GetSizePixel() )
        aVerSBar->SetSizePixel( aSize );
    aVerSBar->SetVisibleSize( nVisibleHeight );
    aVerSBar->SetPageSize( GetScrollBarPageSize( nVisibleHeight ));

    if( nResult & 0x0001 )
    {
        aVerSBar->SetThumbPos( nThumb );
        aVerSBar->Show();
    }
    else
    {
        aVerSBar->SetThumbPos( 0 );
        aVerSBar->Hide();
    }

    // size horizontal scrollbar
    nThumb = aHorSBar->GetThumbPos();
    aSize.setWidth( nRealWidth );
    aSize.setHeight( nHorSBarHeight );
    aSize.AdjustWidth( 1 );
    if( nResult & 0x0001 ) // vertical scrollbar?
    {
        aSize.AdjustWidth( 1 );
        nRealWidth++;
    }
    if( aSize != aHorSBar->GetSizePixel() )
        aHorSBar->SetSizePixel( aSize );
    aHorSBar->SetVisibleSize( nVisibleWidth );
    aHorSBar->SetPageSize( GetScrollBarPageSize(nVisibleWidth ));
    if( nResult & 0x0002 )
    {
        aHorSBar->SetThumbPos( nThumb );
        aHorSBar->Show();
    }
    else
    {
        aHorSBar->SetThumbPos( 0 );
        aHorSBar->Hide();
    }

    aOutputSize.setWidth( nRealWidth );
    if( nResult & 0x0002 ) // horizontal scrollbar ?
        nRealHeight++; // because lower border is clipped
    aOutputSize.setHeight( nRealHeight );

    if( (nResult & (0x0001|0x0002)) == (0x0001|0x0002) )
        aScrBarBox->Show();
    else
        aScrBarBox->Hide();
}

void SvxIconChoiceCtrl_Impl::Resize()
{
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

    VisRectChanged();
}

bool SvxIconChoiceCtrl_Impl::CheckHorScrollBar()
{
    if( maZOrderList.empty() || !aHorSBar->IsVisible() )
        return false;
    const MapMode& rMapMode = pView->GetMapMode();
    Point aOrigin( rMapMode.GetOrigin() );
    if(!( nWinBits & WB_HSCROLL) && !aOrigin.X() )
    {
        long nWidth = aOutputSize.Width();
        const size_t nCount = maZOrderList.size();
        long nMostRight = 0;
        for( size_t nCur = 0; nCur < nCount; nCur++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = maZOrderList[ nCur ];
            long nRight = GetEntryBoundRect(pEntry).Right();
            if( nRight > nWidth )
                return false;
            if( nRight > nMostRight )
                nMostRight = nRight;
        }
        aHorSBar->Hide();
        aOutputSize.AdjustHeight(nHorSBarHeight );
        aVirtOutputSize.setWidth( nMostRight );
        aHorSBar->SetThumbPos( 0 );
        Range aRange;
        aRange.Max() = nMostRight - 1;
        aHorSBar->SetRange( aRange  );
        if( aVerSBar->IsVisible() )
        {
            Size aSize( aVerSBar->GetSizePixel());
            aSize.AdjustHeight(nHorSBarHeight );
            aVerSBar->SetSizePixel( aSize );
        }
        return true;
    }
    return false;
}

bool SvxIconChoiceCtrl_Impl::CheckVerScrollBar()
{
    if( maZOrderList.empty() || !aVerSBar->IsVisible() )
        return false;
    const MapMode& rMapMode = pView->GetMapMode();
    Point aOrigin( rMapMode.GetOrigin() );
    if(!( nWinBits & WB_VSCROLL) && !aOrigin.Y() )
    {
        long nDeepest = 0;
        long nHeight = aOutputSize.Height();
        const size_t nCount = maZOrderList.size();
        for( size_t nCur = 0; nCur < nCount; nCur++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = maZOrderList[ nCur ];
            long nBottom = GetEntryBoundRect(pEntry).Bottom();
            if( nBottom > nHeight )
                return false;
            if( nBottom > nDeepest )
                nDeepest = nBottom;
        }
        aVerSBar->Hide();
        aOutputSize.AdjustWidth(nVerSBarWidth );
        aVirtOutputSize.setHeight( nDeepest );
        aVerSBar->SetThumbPos( 0 );
        Range aRange;
        aRange.Max() = nDeepest - 1;
        aVerSBar->SetRange( aRange  );
        if( aHorSBar->IsVisible() )
        {
            Size aSize( aHorSBar->GetSizePixel());
            aSize.AdjustWidth(nVerSBarWidth );
            aHorSBar->SetSizePixel( aSize );
        }
        return true;
    }
    return false;
}


// hides scrollbars if they're unnecessary
void SvxIconChoiceCtrl_Impl::CheckScrollBars()
{
    CheckVerScrollBar();
    if( CheckHorScrollBar() )
        CheckVerScrollBar();
    if( aVerSBar->IsVisible() && aHorSBar->IsVisible() )
        aScrBarBox->Show();
    else
        aScrBarBox->Hide();
}


void SvxIconChoiceCtrl_Impl::GetFocus()
{
    RepaintSelectedEntries();
    if( pCursor )
    {
        pCursor->SetFlags( SvxIconViewFlags::FOCUSED );
        ShowCursor( true );
    }
}

void SvxIconChoiceCtrl_Impl::LoseFocus()
{
    if( pCursor )
        pCursor->ClearFlags( SvxIconViewFlags::FOCUSED );
    ShowCursor( false );

//  HideFocus ();
//  pView->Invalidate ( aFocus.aRect );

    RepaintSelectedEntries();
}

void SvxIconChoiceCtrl_Impl::SetUpdateMode( bool bUpdate )
{
    if( bUpdate != bUpdateMode )
    {
        bUpdateMode = bUpdate;
        if( bUpdate )
        {
            AdjustScrollBars();
            pImpCursor->Clear();
            pGridMap->Clear();
            pView->Invalidate(InvalidateFlags::NoChildren);
        }
    }
}

// priorities of the emphasis:  bSelected
void SvxIconChoiceCtrl_Impl::PaintEmphasis(const tools::Rectangle& rTextRect, bool bSelected,
                                           vcl::RenderContext& rRenderContext)
{
    Color aOldFillColor(rRenderContext.GetFillColor());

    bool bSolidTextRect = false;

    if (!bSelected)
    {
        const Color& rFillColor = rRenderContext.GetFont().GetFillColor();
        rRenderContext.SetFillColor(rFillColor);
        if (rFillColor != COL_TRANSPARENT)
            bSolidTextRect = true;
    }

    // draw text rectangle
    if (bSolidTextRect)
    {
        rRenderContext.DrawRect(rTextRect);
    }

    rRenderContext.SetFillColor(aOldFillColor);
}


void SvxIconChoiceCtrl_Impl::PaintItem(const tools::Rectangle& rRect,
    IcnViewFieldType eItem, SvxIconChoiceCtrlEntry* pEntry, sal_uInt16 nPaintFlags,
    vcl::RenderContext& rRenderContext )
{
    if (eItem == IcnViewFieldType::Text)
    {
        OUString aText = SvtIconChoiceCtrl::GetEntryText(pEntry);

        rRenderContext.DrawText(rRect, aText, nCurTextDrawFlags);

        if (pEntry->IsFocused())
        {
            tools::Rectangle aRect (CalcFocusRect(pEntry));
            ShowFocus(aRect);
            DrawFocusRect(rRenderContext);
        }
    }
    else
    {
        Point aPos(rRect.TopLeft());
        if (nPaintFlags & PAINTFLAG_HOR_CENTERED)
            aPos.AdjustX((rRect.GetWidth() - aImageSize.Width()) / 2 );
        if (nPaintFlags & PAINTFLAG_VER_CENTERED)
            aPos.AdjustY((rRect.GetHeight() - aImageSize.Height()) / 2 );
        SvtIconChoiceCtrl::DrawEntryImage(pEntry, aPos, rRenderContext);
    }
}

void SvxIconChoiceCtrl_Impl::PaintEntry(SvxIconChoiceCtrlEntry* pEntry, const Point& rPos, vcl::RenderContext& rRenderContext)
{
    bool bSelected = false;

    if (eSelectionMode != SelectionMode::NONE)
        bSelected = pEntry->IsSelected();

    rRenderContext.Push(PushFlags::FONT | PushFlags::TEXTCOLOR);

    OUString aEntryText(SvtIconChoiceCtrl::GetEntryText(pEntry));
    tools::Rectangle aTextRect(CalcTextRect(pEntry, &rPos, &aEntryText));
    tools::Rectangle aBmpRect(CalcBmpRect(pEntry, &rPos));

    bool bShowSelection = (bSelected && (eSelectionMode != SelectionMode::NONE));

    bool bActiveSelection = (0 != (nWinBits & WB_NOHIDESELECTION)) || pView->HasFocus();

    if (bShowSelection)
    {
        const StyleSettings& rSettings = rRenderContext.GetSettings().GetStyleSettings();
        vcl::Font aNewFont(rRenderContext.GetFont());

        // font fill colors that are attributed "hard" need corresponding "hard"
        // attributed highlight colors
        if ((nWinBits & WB_NOHIDESELECTION) || pView->HasFocus())
            aNewFont.SetFillColor(rSettings.GetHighlightColor());
        else
            aNewFont.SetFillColor(rSettings.GetDeactiveColor());

        Color aWinCol = rSettings.GetWindowTextColor();
        if (!bActiveSelection && rSettings.GetFaceColor().IsBright() == aWinCol.IsBright())
            aNewFont.SetColor(rSettings.GetWindowTextColor());
        else
            aNewFont.SetColor(rSettings.GetHighlightTextColor());

        rRenderContext.SetFont(aNewFont);

        rRenderContext.SetFillColor(rRenderContext.GetBackground().GetColor());
        rRenderContext.DrawRect(CalcFocusRect(pEntry));
        rRenderContext.SetFillColor();
    }

    bool bResetClipRegion = false;
    if (!rRenderContext.IsClipRegion() && (aVerSBar->IsVisible() || aHorSBar->IsVisible()))
    {
        tools::Rectangle aOutputArea(GetOutputRect());
        if (aOutputArea.IsOver(aTextRect) || aOutputArea.IsOver(aBmpRect))
        {
            rRenderContext.SetClipRegion(vcl::Region(aOutputArea));
            bResetClipRegion = true;
        }
    }

    bool bLargeIconMode = WB_ICON == ( nWinBits & VIEWMODE_MASK );
    sal_uInt16 nBmpPaintFlags = PAINTFLAG_VER_CENTERED;
    if (bLargeIconMode)
        nBmpPaintFlags |= PAINTFLAG_HOR_CENTERED;
    sal_uInt16 nTextPaintFlags = bLargeIconMode ? PAINTFLAG_HOR_CENTERED : PAINTFLAG_VER_CENTERED;

    PaintEmphasis(aTextRect, bSelected, rRenderContext);

    if ( bShowSelection )
        vcl::RenderTools::DrawSelectionBackground(rRenderContext, *pView, CalcFocusRect(pEntry),
                                                  bActiveSelection ? 1 : 2, false, true, false);


    PaintItem(aBmpRect, IcnViewFieldType::Image, pEntry, nBmpPaintFlags, rRenderContext);

    PaintItem(aTextRect, IcnViewFieldType::Text, pEntry, nTextPaintFlags, rRenderContext);

    // draw highlight frame
    if (pEntry == pCurHighlightFrame)
        DrawHighlightFrame(rRenderContext, CalcFocusRect(pEntry));

    rRenderContext.Pop();
    if (bResetClipRegion)
        rRenderContext.SetClipRegion();
}

void SvxIconChoiceCtrl_Impl::SetEntryPos( SvxIconChoiceCtrlEntry* pEntry, const Point& rPos )
{
    ShowCursor( false );
    tools::Rectangle aBoundRect( GetEntryBoundRect( pEntry ));
    pView->Invalidate( aBoundRect );
    ToTop( pEntry );
    if( !IsAutoArrange() )
    {
        bool bAdjustVirtSize = false;
        if( rPos != aBoundRect.TopLeft() )
        {
            Point aGridOffs(
                pEntry->aGridRect.TopLeft() - pEntry->aRect.TopLeft() );
            pImpCursor->Clear();
            pGridMap->Clear();
            aBoundRect.SetPos( rPos );
            pEntry->aRect = aBoundRect;
            pEntry->aGridRect.SetPos( rPos + aGridOffs );
            bAdjustVirtSize = true;
        }
        if( bAdjustVirtSize )
            AdjustVirtSize( pEntry->aRect );

        pView->Invalidate( pEntry->aRect );
        pGridMap->OccupyGrids( pEntry );
    }
    else
    {
        SvxIconChoiceCtrlEntry* pPrev = FindEntryPredecessor( pEntry, rPos );
        SetEntryPredecessor( pEntry, pPrev );
        aAutoArrangeIdle.Start();
    }
    ShowCursor( true );
}

void SvxIconChoiceCtrl_Impl::SetNoSelection()
{
    // block recursive calls via SelectEntry
    if( !(nFlags & IconChoiceFlags::ClearingSelection ))
    {
        nFlags |= IconChoiceFlags::ClearingSelection;
        DeselectAllBut( nullptr );
        nFlags &= ~IconChoiceFlags::ClearingSelection;
    }
}

SvxIconChoiceCtrlEntry* SvxIconChoiceCtrl_Impl::GetEntry( const Point& rDocPos, bool bHit )
{
    CheckBoundingRects();
    // search through z-order list from the end
    size_t nCount = maZOrderList.size();
    while( nCount )
    {
        nCount--;
        SvxIconChoiceCtrlEntry* pEntry = maZOrderList[ nCount ];
        if( pEntry->aRect.IsInside( rDocPos ) )
        {
            if( bHit )
            {
                tools::Rectangle aRect = CalcBmpRect( pEntry );
                aRect.AdjustTop( -3 );
                aRect.AdjustBottom(3 );
                aRect.AdjustLeft( -3 );
                aRect.AdjustRight(3 );
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
    return nullptr;
}

void SvxIconChoiceCtrl_Impl::MakeEntryVisible( SvxIconChoiceCtrlEntry* pEntry, bool bBound )
{
    if ( bBound )
    {
        const tools::Rectangle& rRect = GetEntryBoundRect( pEntry );
        MakeVisible( rRect );
    }
    else
    {
        tools::Rectangle aRect = CalcBmpRect( pEntry );
        aRect.Union( CalcTextRect( pEntry ) );
        aRect.AdjustTop(TBOFFS_BOUND );
        aRect.AdjustBottom(TBOFFS_BOUND );
        aRect.AdjustLeft(LROFFS_BOUND );
        aRect.AdjustRight(LROFFS_BOUND );
        MakeVisible( aRect );
    }
}

const tools::Rectangle& SvxIconChoiceCtrl_Impl::GetEntryBoundRect( SvxIconChoiceCtrlEntry* pEntry )
{
    if( !IsBoundingRectValid( pEntry->aRect ))
        FindBoundingRect( pEntry );
    return pEntry->aRect;
}

tools::Rectangle SvxIconChoiceCtrl_Impl::CalcBmpRect( SvxIconChoiceCtrlEntry* pEntry, const Point* pPos )
{
    tools::Rectangle aBound = GetEntryBoundRect( pEntry );
    if( pPos )
        aBound.SetPos( *pPos );
    Point aPos( aBound.TopLeft() );

    switch( nWinBits & VIEWMODE_MASK )
    {
        case WB_ICON:
        {
            aPos.AdjustX(( aBound.GetWidth() - aImageSize.Width() ) / 2 );
            return tools::Rectangle( aPos, aImageSize );
        }

        case WB_SMALLICON:
        case WB_DETAILS:
            aPos.AdjustY(( aBound.GetHeight() - aImageSize.Height() ) / 2 );
            //TODO: determine horizontal distance to bounding rectangle
            return tools::Rectangle( aPos, aImageSize );

        default:
            OSL_FAIL("IconView: Viewmode not set");
            return aBound;
    }
}

tools::Rectangle SvxIconChoiceCtrl_Impl::CalcTextRect( SvxIconChoiceCtrlEntry* pEntry,
    const Point* pEntryPos, const OUString* pStr )
{
    OUString aEntryText;
    if( !pStr )
        aEntryText = SvtIconChoiceCtrl::GetEntryText( pEntry );
    else
        aEntryText = *pStr;

    const tools::Rectangle aMaxTextRect( CalcMaxTextRect( pEntry ) );
    tools::Rectangle aBound( GetEntryBoundRect( pEntry ) );
    if( pEntryPos )
        aBound.SetPos( *pEntryPos );

    tools::Rectangle aTextRect( aMaxTextRect );
    aTextRect = pView->GetTextRect( aTextRect, aEntryText, nCurTextDrawFlags );

    Size aTextSize( aTextRect.GetSize() );

    Point aPos( aBound.TopLeft() );
    long nBoundWidth = aBound.GetWidth();
    long nBoundHeight = aBound.GetHeight();

    switch( nWinBits & VIEWMODE_MASK )
    {
        case WB_ICON:
            aPos.AdjustY(aImageSize.Height() );
            aPos.AdjustY(VER_DIST_BMP_STRING );
            aPos.AdjustX((nBoundWidth - aTextSize.Width()) / 2 );
            break;

        case WB_SMALLICON:
        case WB_DETAILS:
            aPos.AdjustX(aImageSize.Width() );
            aPos.AdjustX(HOR_DIST_BMP_STRING );
            aPos.AdjustY((nBoundHeight - aTextSize.Height()) / 2 );
            break;
    }
    return tools::Rectangle( aPos, aTextSize );
}


long SvxIconChoiceCtrl_Impl::CalcBoundingWidth() const
{
    long nStringWidth = GetItemSize( IcnViewFieldType::Text ).Width();
    long nWidth = 0;

    switch( nWinBits & VIEWMODE_MASK )
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

long SvxIconChoiceCtrl_Impl::CalcBoundingHeight() const
{
    long nStringHeight = GetItemSize(IcnViewFieldType::Text).Height();
    long nHeight = 0;

    switch( nWinBits & VIEWMODE_MASK )
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
        const_cast<SvxIconChoiceCtrl_Impl*>(this)->nMaxBoundHeight = nHeight;
        const_cast<SvxIconChoiceCtrl_Impl*>(this)->aHorSBar->SetLineSize( GetScrollBarLineSize() );
        const_cast<SvxIconChoiceCtrl_Impl*>(this)->aVerSBar->SetLineSize( GetScrollBarLineSize() );
    }
    return nHeight;
}

Size SvxIconChoiceCtrl_Impl::CalcBoundingSize() const
{
    return Size( CalcBoundingWidth(), CalcBoundingHeight() );
}

void SvxIconChoiceCtrl_Impl::RecalcAllBoundingRectsSmart()
{
    nMaxBoundHeight = 0;
    maZOrderList.clear();
    size_t nCur;
    SvxIconChoiceCtrlEntry* pEntry;
    const size_t nCount = maEntries.size();

    if( !IsAutoArrange() || !pHead )
    {
        for( nCur = 0; nCur < nCount; nCur++ )
        {
            pEntry = maEntries[ nCur ].get();
            if( IsBoundingRectValid( pEntry->aRect ))
            {
                Size aBoundSize( pEntry->aRect.GetSize() );
                if( aBoundSize.Height() > nMaxBoundHeight )
                    nMaxBoundHeight = aBoundSize.Height();
            }
            else
                FindBoundingRect( pEntry );
            maZOrderList.push_back( pEntry );
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
            maZOrderList.push_back( pEntry );
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
    Size aSize( CalcBoundingSize() );
    Point aPos(pGridMap->GetGridRect(pGridMap->GetUnoccupiedGrid()).TopLeft());
    SetBoundingRect_Impl( pEntry, aPos, aSize );
}

void SvxIconChoiceCtrl_Impl::SetBoundingRect_Impl( SvxIconChoiceCtrlEntry* pEntry, const Point& rPos,
    const Size& /*rBoundingSize*/ )
{
    tools::Rectangle aGridRect( rPos, Size(nGridDX, nGridDY) );
    pEntry->aGridRect = aGridRect;
    Center( pEntry );
    AdjustVirtSize( pEntry->aRect );
    pGridMap->OccupyGrids( pEntry );
}


void SvxIconChoiceCtrl_Impl::SetCursor( SvxIconChoiceCtrlEntry* pEntry )
{
    if( pEntry == pCursor )
    {
        if( pCursor && eSelectionMode == SelectionMode::Single &&
                !pCursor->IsSelected() )
            SelectEntry( pCursor, true );
        return;
    }
    ShowCursor( false );
    SvxIconChoiceCtrlEntry* pOldCursor = pCursor;
    pCursor = pEntry;
    if( pOldCursor )
    {
        pOldCursor->ClearFlags( SvxIconViewFlags::FOCUSED );
        if( eSelectionMode == SelectionMode::Single )
            SelectEntry( pOldCursor, false ); // deselect old cursor
    }
    if( pCursor )
    {
        ToTop( pCursor );
        pCursor->SetFlags( SvxIconViewFlags::FOCUSED );
        if( eSelectionMode == SelectionMode::Single )
            SelectEntry( pCursor, true );
        ShowCursor( true );
    }
}


void SvxIconChoiceCtrl_Impl::ShowCursor( bool bShow )
{
    if( !pCursor || !bShow || !pView->HasFocus() )
    {
        pView->HideFocus();
        return;
    }
    tools::Rectangle aRect ( CalcFocusRect( pCursor ) );
    /*pView->*/ShowFocus( aRect );
}


void SvxIconChoiceCtrl_Impl::HideDDIcon()
{
    pView->Update();
    if( pDDDev )
    {
        Size aSize( pDDDev->GetOutputSizePixel() );
        // restore pView
        pView->DrawOutDev( Point(), aSize, Point(), aSize, *pDDDev );
    }
    pDDBufDev = pDDDev;
    pDDDev = nullptr;
}

bool SvxIconChoiceCtrl_Impl::HandleScrollCommand( const CommandEvent& rCmd )
{
    tools::Rectangle aDocRect( Point(), aVirtOutputSize );
    tools::Rectangle aVisRect( GetOutputRect() );
    if( aVisRect.IsInside( aDocRect ))
        return false;
    Size aDocSize( aDocRect.GetSize() );
    Size aVisSize( aVisRect.GetSize() );
    bool bHor = aDocSize.Width() > aVisSize.Width();
    bool bVer = aDocSize.Height() > aVisSize.Height();

    long nScrollDX = 0, nScrollDY = 0;

    switch( rCmd.GetCommand() )
    {
        case CommandEventId::StartAutoScroll:
        {
            pView->EndTracking();
            StartAutoScrollFlags nScrollFlags = StartAutoScrollFlags::NONE;
            if( bHor )
                nScrollFlags |= StartAutoScrollFlags::Horz;
            if( bVer )
                nScrollFlags |= StartAutoScrollFlags::Vert;
            if( nScrollFlags != StartAutoScrollFlags::NONE )
            {
                pView->StartAutoScroll( nScrollFlags );
                return true;
            }
        }
        break;

        case CommandEventId::Wheel:
        {
            const CommandWheelData* pData = rCmd.GetWheelData();
            if( pData && (CommandWheelMode::SCROLL == pData->GetMode()) && !pData->IsHorz() )
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
                    nScrollDY = pData->GetNotchDelta() * static_cast<long>(nScrollLines);
                    nScrollDY *= GetScrollBarLineSize();
                }
            }
        }
        break;

        case CommandEventId::AutoScroll:
        {
            const CommandScrollData* pData = rCmd.GetAutoScrollData();
            if( pData )
            {
                nScrollDX = pData->GetDeltaX() * GetScrollBarLineSize();
                nScrollDY = pData->GetDeltaY() * GetScrollBarLineSize();
            }
        }
        break;

        default: break;
    }

    if( nScrollDX || nScrollDY )
    {
        aVisRect.AdjustTop( -nScrollDY );
        aVisRect.AdjustBottom( -nScrollDY );
        aVisRect.AdjustLeft( -nScrollDX );
        aVisRect.AdjustRight( -nScrollDX );
        MakeVisible( aVisRect );
        return true;
    }
    return false;
}


void SvxIconChoiceCtrl_Impl::Command( const CommandEvent& rCEvt )
{
    // scroll mouse event?
    if( (rCEvt.GetCommand() == CommandEventId::Wheel) ||
        (rCEvt.GetCommand() == CommandEventId::StartAutoScroll) ||
        (rCEvt.GetCommand() == CommandEventId::AutoScroll) )
    {
        if( HandleScrollCommand( rCEvt ) )
            return;
    }
}

void SvxIconChoiceCtrl_Impl::ToTop( SvxIconChoiceCtrlEntry* pEntry )
{
    if( maZOrderList.empty() || pEntry == maZOrderList.back())
        return;

    auto it = std::find(maZOrderList.begin(), maZOrderList.end(), pEntry);
    if (it != maZOrderList.end())
    {
        maZOrderList.erase( it );
        maZOrderList.push_back( pEntry );
    }
}

void SvxIconChoiceCtrl_Impl::ClipAtVirtOutRect( tools::Rectangle& rRect ) const
{
    if( rRect.Bottom() >= aVirtOutputSize.Height() )
        rRect.SetBottom( aVirtOutputSize.Height() - 1 );
    if( rRect.Right() >= aVirtOutputSize.Width() )
        rRect.SetRight( aVirtOutputSize.Width() - 1 );
    if( rRect.Top() < 0 )
        rRect.SetTop( 0 );
    if( rRect.Left() < 0 )
        rRect.SetLeft( 0 );
}

// rRect: area of the document (in document coordinates) that we want to make
// visible
// bScrBar == true: rectangle was calculated because of a scrollbar event

void SvxIconChoiceCtrl_Impl::MakeVisible( const tools::Rectangle& rRect, bool bScrBar )
{
    tools::Rectangle aVirtRect( rRect );
    ClipAtVirtOutRect( aVirtRect );
    Point aOrigin( pView->GetMapMode().GetOrigin() );
    // convert to document coordinate
    aOrigin *= -1;
    tools::Rectangle aOutputArea( GetOutputRect() );
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

    aOrigin.AdjustX(nDx );
    aOrigin.AdjustY(nDy );
    aOutputArea.SetPos( aOrigin );
    if( GetUpdateMode() )
    {
        HideDDIcon();
        pView->Update();
        ShowCursor( false );
    }

    // invert origin for SV (so we can scroll/paint using document coordinates)
    aOrigin *= -1;
    SetOrigin( aOrigin );

    bool bScrollable = pView->GetBackground().IsScrollable();

    if( bScrollable && GetUpdateMode() )
    {
        // scroll in reverse direction!
        pView->Control::Scroll( -nDx, -nDy, aOutputArea,
            ScrollFlags::NoChildren | ScrollFlags::UseClipRegion | ScrollFlags::Clip );
    }
    else
        pView->Invalidate(InvalidateFlags::NoChildren);

    if( aHorSBar->IsVisible() || aVerSBar->IsVisible() )
    {
        if( !bScrBar )
        {
            aOrigin *= -1;
            // correct thumbs
            if(aHorSBar->IsVisible() && aHorSBar->GetThumbPos() != aOrigin.X())
                aHorSBar->SetThumbPos( aOrigin.X() );
            if(aVerSBar->IsVisible() && aVerSBar->GetThumbPos() != aOrigin.Y())
                aVerSBar->SetThumbPos( aOrigin.Y() );
        }
    }

    if( GetUpdateMode() )
        ShowCursor( true );

    // check if we still need scrollbars
    CheckScrollBars();
    if( bScrollable && GetUpdateMode() )
        pView->Update();

    // If the requested area can not be made completely visible, the
    // Vis-Rect-Changed handler is called in any case. This case may occur e.g.
    // if only few pixels of the lower border are invisible, but a scrollbar has
    // a larger line size.
    VisRectChanged();
}

sal_Int32 SvxIconChoiceCtrl_Impl::GetSelectionCount() const
{
    if( (nWinBits & WB_HIGHLIGHTFRAME) && pCurHighlightFrame )
        return 1;
    return nSelectionCount;
}

void SvxIconChoiceCtrl_Impl::ToggleSelection( SvxIconChoiceCtrlEntry* pEntry )
{
    bool bSel;
    bSel = !pEntry->IsSelected();
    SelectEntry( pEntry, bSel, true );
}

void SvxIconChoiceCtrl_Impl::DeselectAllBut( SvxIconChoiceCtrlEntry const * pThisEntryNot )
{
    ClearSelectedRectList();

    // TODO: work through z-order list, if necessary!

    size_t nCount = maEntries.size();
    for( size_t nCur = 0; nCur < nCount; nCur++ )
    {
        SvxIconChoiceCtrlEntry* pEntry = maEntries[ nCur ].get();
        if( pEntry != pThisEntryNot && pEntry->IsSelected() )
            SelectEntry( pEntry, false, true );
    }
    pAnchor = nullptr;
    nFlags &= ~IconChoiceFlags::AddMode;
}

Size SvxIconChoiceCtrl_Impl::GetMinGrid() const
{
    Size aMinSize( aImageSize );
    aMinSize.AdjustWidth(2 * LROFFS_BOUND );
    aMinSize.AdjustHeight(TBOFFS_BOUND );  // single offset is enough (FileDlg)
    OUString const aStrDummy( "XXX" );
    Size aTextSize( pView->GetTextWidth( aStrDummy ), pView->GetTextHeight() );
    if( nWinBits & WB_ICON )
    {
        aMinSize.AdjustHeight(VER_DIST_BMP_STRING );
        aMinSize.AdjustHeight(aTextSize.Height() );
    }
    else
    {
        aMinSize.AdjustWidth(HOR_DIST_BMP_STRING );
        aMinSize.AdjustWidth(aTextSize.Width() );
    }
    return aMinSize;
}

void SvxIconChoiceCtrl_Impl::SetGrid( const Size& rSize )
{
    Size aSize( rSize );
    Size aMinSize( GetMinGrid() );
    if( aSize.Width() < aMinSize.Width() )
        aSize.setWidth( aMinSize.Width() );
    if( aSize.Height() < aMinSize.Height() )
        aSize.setHeight( aMinSize.Height() );

    nGridDX = aSize.Width();
    // HACK: Detail mode is not yet fully implemented, this workaround makes it
    // fly with a single column
    if( nWinBits & WB_DETAILS )
    {
        const SvxIconChoiceCtrlColumnInfo* pCol = GetColumn( 0 );
        if( pCol )
            const_cast<SvxIconChoiceCtrlColumnInfo*>(pCol)->SetWidth( nGridDX );
    }
    nGridDY = aSize.Height();
    SetDefaultTextSize();
}

// Calculates the maximum size that the text rectangle may use within its
// bounding rectangle. In WB_ICON mode with SvxIconChoiceCtrlTextMode::Full, Bottom is set to
// LONG_MAX.

tools::Rectangle SvxIconChoiceCtrl_Impl::CalcMaxTextRect( const SvxIconChoiceCtrlEntry* pEntry ) const
{
    tools::Rectangle aBoundRect;
    // avoid infinite recursion: don't calculate the bounding rectangle here
    if( IsBoundingRectValid( pEntry->aRect ) )
        aBoundRect = pEntry->aRect;
    else
        aBoundRect = pEntry->aGridRect;

    tools::Rectangle aBmpRect( const_cast<SvxIconChoiceCtrl_Impl*>(this)->CalcBmpRect(
        const_cast<SvxIconChoiceCtrlEntry*>(pEntry) ) );
    if( nWinBits & WB_ICON )
    {
        aBoundRect.SetTop( aBmpRect.Bottom() );
        aBoundRect.AdjustTop(VER_DIST_BMP_STRING );
        if( aBoundRect.Top() > aBoundRect.Bottom())
            aBoundRect.SetTop( aBoundRect.Bottom() );
        aBoundRect.AdjustLeft(LROFFS_BOUND );
        aBoundRect.AdjustLeft( 1 );
        aBoundRect.AdjustRight( -(LROFFS_BOUND) );
        aBoundRect.AdjustRight( -1 );
        if( aBoundRect.Left() > aBoundRect.Right())
            aBoundRect.SetLeft( aBoundRect.Right() );
        if( pEntry->GetTextMode() == SvxIconChoiceCtrlTextMode::Full )
            aBoundRect.SetBottom( LONG_MAX );
    }
    else
    {
        aBoundRect.SetLeft( aBmpRect.Right() );
        aBoundRect.AdjustLeft(HOR_DIST_BMP_STRING );
        aBoundRect.AdjustRight( -(LROFFS_BOUND) );
        if( aBoundRect.Left() > aBoundRect.Right() )
            aBoundRect.SetLeft( aBoundRect.Right() );
        long nHeight = aBoundRect.GetSize().Height();
        nHeight = nHeight - aDefaultTextSize.Height();
        nHeight /= 2;
        aBoundRect.AdjustTop(nHeight );
        aBoundRect.AdjustBottom( -nHeight );
    }
    return aBoundRect;
}

void SvxIconChoiceCtrl_Impl::SetDefaultTextSize()
{
    long nDY = nGridDY;
    nDY -= aImageSize.Height();
    nDY -= VER_DIST_BMP_STRING;
    nDY -= 2 * TBOFFS_BOUND;
    if (nDY <= 0)
        nDY = 2;

    long nDX = nGridDX;
    nDX -= 2 * LROFFS_BOUND;
    nDX -= 2;
    if (nDX <= 0)
        nDX = 2;

    long nHeight = pView->GetTextHeight();
    if (nDY < nHeight)
        nDY = nHeight;
    if(pView->GetDPIScaleFactor() > 1)
    {
      nDY*=2;
    }
    aDefaultTextSize = Size(nDX, nDY);
}


void SvxIconChoiceCtrl_Impl::Center( SvxIconChoiceCtrlEntry* pEntry ) const
{
    pEntry->aRect = pEntry->aGridRect;
    Size aSize( CalcBoundingSize() );
    if( nWinBits & WB_ICON )
    {
        // center horizontally
        long nBorder = pEntry->aGridRect.GetWidth() - aSize.Width();
        pEntry->aRect.AdjustLeft(nBorder / 2 );
        pEntry->aRect.AdjustRight( -(nBorder / 2) );
    }
    // center vertically
    pEntry->aRect.SetBottom( pEntry->aRect.Top() + aSize.Height() );
}


// The deltas are the offsets by which the view is moved on the document.
// left, up: offsets < 0
// right, down: offsets > 0
void SvxIconChoiceCtrl_Impl::Scroll( long nDeltaX, long nDeltaY )
{
    const MapMode& rMapMode = pView->GetMapMode();
    Point aOrigin( rMapMode.GetOrigin() );
    // convert to document coordinate
    aOrigin *= -1;
    aOrigin.AdjustY(nDeltaY );
    aOrigin.AdjustX(nDeltaX );
    tools::Rectangle aRect( aOrigin, aOutputSize );
    MakeVisible( aRect, true/*bScrollBar*/ );
}


const Size& SvxIconChoiceCtrl_Impl::GetItemSize( IcnViewFieldType eItem ) const
{
    if (eItem == IcnViewFieldType::Text)
        return aDefaultTextSize;
    return aImageSize; // IcnViewFieldType::Image
}

tools::Rectangle SvxIconChoiceCtrl_Impl::CalcFocusRect( SvxIconChoiceCtrlEntry* pEntry )
{
    tools::Rectangle aTextRect( CalcTextRect( pEntry ) );
    tools::Rectangle aBoundRect( GetEntryBoundRect( pEntry ) );
    return tools::Rectangle(
        aBoundRect.Left(), aBoundRect.Top() - 1, aBoundRect.Right() - 1,
        aTextRect.Bottom() + 1);
}

// the hot spot is the inner 50% of the rectangle
static tools::Rectangle GetHotSpot( const tools::Rectangle& rRect )
{
    tools::Rectangle aResult( rRect );
    aResult.Justify();
    Size aSize( rRect.GetSize() );
    long nDelta = aSize.Width() / 4;
    aResult.AdjustLeft(nDelta );
    aResult.AdjustRight( -nDelta );
    nDelta = aSize.Height() / 4;
    aResult.AdjustTop(nDelta );
    aResult.AdjustBottom( -nDelta );
    return aResult;
}

void SvxIconChoiceCtrl_Impl::SelectRect( SvxIconChoiceCtrlEntry* pEntry1, SvxIconChoiceCtrlEntry* pEntry2,
    bool bAdd, std::vector<tools::Rectangle>* pOtherRects )
{
    DBG_ASSERT(pEntry1 && pEntry2,"SelectEntry: Invalid Entry-Ptr");
    tools::Rectangle aRect( GetEntryBoundRect( pEntry1 ) );
    aRect.Union( GetEntryBoundRect( pEntry2 ) );
    SelectRect( aRect, bAdd, pOtherRects );
}

void SvxIconChoiceCtrl_Impl::SelectRect( const tools::Rectangle& rRect, bool bAdd,
    std::vector<tools::Rectangle>* pOtherRects )
{
    aCurSelectionRect = rRect;
    if( maZOrderList.empty() )
        return;

    // set flag, so ToTop won't be called in Select
    bool bAlreadySelectingRect(nFlags & IconChoiceFlags::SelectingRect);
    nFlags |= IconChoiceFlags::SelectingRect;

    CheckBoundingRects();
    pView->Update();
    const size_t nCount = maZOrderList.size();

    tools::Rectangle aRect( rRect );
    aRect.Justify();
    bool bCalcOverlap = (bAdd && pOtherRects && !pOtherRects->empty());

    bool bResetClipRegion = false;
    if( !pView->IsClipRegion() )
    {
        bResetClipRegion = true;
        pView->SetClipRegion(vcl::Region(GetOutputRect()));
    }

    for( size_t nPos = 0; nPos < nCount; nPos++ )
    {
        SvxIconChoiceCtrlEntry* pEntry = maZOrderList[ nPos ];

        if( !IsBoundingRectValid( pEntry->aRect ))
            FindBoundingRect( pEntry );
        tools::Rectangle aBoundRect( GetHotSpot( pEntry->aRect ) );
        bool bSelected = pEntry->IsSelected();

        bool bOverlaps;
        if( bCalcOverlap )
            bOverlaps = IsOver( pOtherRects, aBoundRect );
        else
            bOverlaps = false;
        bool bOver = aRect.IsOver( aBoundRect );

        if( bOver && !bOverlaps )
        {
            // is inside the new selection rectangle and outside of any old one
            // => select
            if( !bSelected )
                SelectEntry( pEntry, true, true );
        }
        else if( !bAdd )
        {
            // is outside of the selection rectangle
            // => deselect
            if( bSelected )
                SelectEntry( pEntry, false, true );
        }
        else if (bOverlaps)
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
                    SelectEntry( pEntry, false, true );
            }
            else
            {
                // select entry of an old rectangle
                if( !bSelected )
                    SelectEntry( pEntry, true, true );
            }
        }
        else if( !bOver && bSelected )
        {
            // this entry is completely outside the rectangle => deselect it
            SelectEntry( pEntry, false, true );
        }
    }

    if( !bAlreadySelectingRect )
        nFlags &= ~IconChoiceFlags::SelectingRect;

    pView->Update();
    if( bResetClipRegion )
        pView->SetClipRegion();
}

void SvxIconChoiceCtrl_Impl::SelectRange(
                        SvxIconChoiceCtrlEntry const * pStart,
                        SvxIconChoiceCtrlEntry const * pEnd,
                        bool bAdd )
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
                SelectEntry( pEntry, false, true );
        }
    }

    // select everything between nFirst and nLast
    for ( i=nFirst; i<=nLast; i++ )
    {
        pEntry = GetEntry( i );
        if( ! pEntry->IsSelected() )
            SelectEntry( pEntry, true, true );
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
                SelectEntry( pEntry, false, true );
        }
    }
}

bool SvxIconChoiceCtrl_Impl::IsOver( std::vector<tools::Rectangle>* pRectList, const tools::Rectangle& rBoundRect )
{
    const sal_uInt16 nCount = pRectList->size();
    for( sal_uInt16 nCur = 0; nCur < nCount; nCur++ )
    {
        tools::Rectangle& rRect = (*pRectList)[ nCur ];
        if( rBoundRect.IsOver( rRect ))
            return true;
    }
    return false;
}

void SvxIconChoiceCtrl_Impl::AddSelectedRect( SvxIconChoiceCtrlEntry* pEntry1,
    SvxIconChoiceCtrlEntry* pEntry2 )
{
    DBG_ASSERT(pEntry1 && pEntry2,"SelectEntry: Invalid Entry-Ptr");
    tools::Rectangle aRect( GetEntryBoundRect( pEntry1 ) );
    aRect.Union( GetEntryBoundRect( pEntry2 ) );
    AddSelectedRect( aRect );
}

void SvxIconChoiceCtrl_Impl::AddSelectedRect( const tools::Rectangle& rRect )
{
    tools::Rectangle newRect = rRect;
    newRect.Justify();
    aSelectedRectList.push_back( newRect );
}

void SvxIconChoiceCtrl_Impl::ClearSelectedRectList()
{
    aSelectedRectList.clear();
}

IMPL_LINK_NOARG(SvxIconChoiceCtrl_Impl, AutoArrangeHdl, Timer *, void)
{
    aAutoArrangeIdle.Stop();
    Arrange( IsAutoArrange(), 0, 0 );
}

IMPL_LINK_NOARG(SvxIconChoiceCtrl_Impl, VisRectChangedHdl, Timer *, void)
{
    aVisRectChangedIdle.Stop();
}

IMPL_LINK_NOARG(SvxIconChoiceCtrl_Impl, DocRectChangedHdl, Timer *, void)
{
    aDocRectChangedIdle.Stop();
}

#ifdef DBG_UTIL
void SvxIconChoiceCtrl_Impl::SetEntryTextMode( SvxIconChoiceCtrlTextMode eMode, SvxIconChoiceCtrlEntry* pEntry )
{
    if( !pEntry )
    {
        if( eTextMode != eMode )
        {
            eTextMode = eMode;
            Arrange( true, 0, 0 );
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
#endif

// Draw my own focusrect, because the focusrect of the outputdevice has got the inverted color
// of the background. But what will we see, if the backgroundcolor is gray ? - We will see
// a gray focusrect on a gray background !!!

void SvxIconChoiceCtrl_Impl::ShowFocus ( tools::Rectangle const & rRect )
{
    Color aBkgColor(pView->GetBackground().GetColor());
    Color aPenColor;
    sal_uInt16 nColor = ( aBkgColor.GetRed() + aBkgColor.GetGreen() + aBkgColor.GetBlue() ) / 3;
    if (nColor > 128)
        aPenColor = COL_BLACK;
    else
        aPenColor = COL_WHITE;

    aFocus.aPenColor = aPenColor;
    aFocus.aRect = rRect;
}

void SvxIconChoiceCtrl_Impl::DrawFocusRect(vcl::RenderContext& rRenderContext)
{
    rRenderContext.SetLineColor(aFocus.aPenColor);
    rRenderContext.SetFillColor();
    tools::Polygon aPolygon (aFocus.aRect);

    LineInfo aLineInfo(LineStyle::Dash);

    aLineInfo.SetDashLen(1);
    aLineInfo.SetDotLen(1);
    aLineInfo.SetDistance(1);
    aLineInfo.SetDotCount(1);

    rRenderContext.DrawPolyLine(aPolygon, aLineInfo);
}

bool SvxIconChoiceCtrl_Impl::IsMnemonicChar( sal_Unicode cChar, sal_uLong& rPos ) const
{
    bool bRet = false;
    const vcl::I18nHelper& rI18nHelper = Application::GetSettings().GetUILocaleI18nHelper();
    size_t nEntryCount = GetEntryCount();
    for ( size_t i = 0; i < nEntryCount; ++i )
    {
        if ( rI18nHelper.MatchMnemonic( GetEntry( i )->GetText(), cChar ) )
        {
            bRet = true;
            rPos = i;
            break;
        }
    }

    return bRet;
}


IMPL_LINK(SvxIconChoiceCtrl_Impl, UserEventHdl, void*, nId, void )
{
    if( nId == EVENTID_ADJUST_SCROLLBARS )
    {
        nUserEventAdjustScrBars = nullptr;
        AdjustScrollBars();
    }
    else if( nId == EVENTID_SHOW_CURSOR )
    {
        ShowCursor( true );
    }
}

void SvxIconChoiceCtrl_Impl::CancelUserEvents()
{
    if( nUserEventAdjustScrBars )
    {
        Application::RemoveUserEvent( nUserEventAdjustScrBars );
        nUserEventAdjustScrBars = nullptr;
    }
}

void SvxIconChoiceCtrl_Impl::InvalidateEntry( SvxIconChoiceCtrlEntry* pEntry )
{
    if( pEntry == pCursor )
        ShowCursor( false );
    pView->Invalidate( pEntry->aRect );
    Center( pEntry );
    pView->Invalidate( pEntry->aRect );
    if( pEntry == pCursor )
        ShowCursor( true );
}

SvxIconChoiceCtrlEntry* SvxIconChoiceCtrl_Impl::GetFirstSelectedEntry() const
{
    if( !GetSelectionCount() )
        return nullptr;

    if( (nWinBits & WB_HIGHLIGHTFRAME) && (eSelectionMode == SelectionMode::NONE) )
    {
        return pCurHighlightFrame;
    }

    size_t nCount = maEntries.size();
    if( !pHead )
    {
        for( size_t nCur = 0; nCur < nCount; nCur++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = maEntries[ nCur ].get();
            if( pEntry->IsSelected() )
            {
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
                return pEntry;
            }
            pEntry = pEntry->pflink;
            if( nCount && pEntry == pHead )
            {
                OSL_FAIL("SvxIconChoiceCtrl_Impl::GetFirstSelectedEntry > infinite loop!");
                return nullptr;
            }
        }
    }
    return nullptr;
}

void SvxIconChoiceCtrl_Impl::SelectAll()
{
    size_t nCount = maEntries.size();
    for( size_t nCur = 0; nCur < nCount; nCur++ )
    {
        SvxIconChoiceCtrlEntry* pEntry = maEntries[ nCur ].get();
        SelectEntry( pEntry, true/*bSelect*/, true );
    }
    nFlags &= ~IconChoiceFlags::AddMode;
    pAnchor = nullptr;
}




sal_Int32 SvxIconChoiceCtrl_Impl::GetEntryListPos( SvxIconChoiceCtrlEntry const * pEntry ) const
{
    if( !(nFlags & IconChoiceFlags::EntryListPosValid ))
        const_cast<SvxIconChoiceCtrl_Impl*>(this)->SetListPositions();
    return pEntry->nPos;
}

void SvxIconChoiceCtrl_Impl::InitSettings()
{
    const StyleSettings& rStyleSettings = pView->GetSettings().GetStyleSettings();

    // unit (from settings) is Point
    vcl::Font aFont( rStyleSettings.GetFieldFont() );
    aFont.SetColor( rStyleSettings.GetWindowTextColor() );
    pView->SetPointFont( aFont );
    SetDefaultTextSize();

    pView->SetTextColor( rStyleSettings.GetFieldTextColor() );
    pView->SetTextFillColor();

    pView->SetBackground( rStyleSettings.GetFieldColor());

    long nScrBarSize = rStyleSettings.GetScrollBarSize();
    if( nScrBarSize == nHorSBarHeight && nScrBarSize == nVerSBarWidth )
        return;

    nHorSBarHeight = nScrBarSize;
    Size aSize( aHorSBar->GetSizePixel() );
    aSize.setHeight( nScrBarSize );
    aHorSBar->Hide();
    aHorSBar->SetSizePixel( aSize );

    nVerSBarWidth = nScrBarSize;
    aSize = aVerSBar->GetSizePixel();
    aSize.setWidth( nScrBarSize );
    aVerSBar->Hide();
    aVerSBar->SetSizePixel( aSize );

    Size aOSize( pView->Control::GetOutputSizePixel() );
    PositionScrollBars( aOSize.Width(), aOSize.Height() );
    AdjustScrollBars();
}

void SvxIconChoiceCtrl_Impl::SetPositionMode( SvxIconChoiceCtrlPositionMode eMode )
{
    if( eMode == ePositionMode )
        return;

    SvxIconChoiceCtrlPositionMode eOldMode = ePositionMode;
    ePositionMode = eMode;
    size_t nCount = maEntries.size();

    if( eOldMode == SvxIconChoiceCtrlPositionMode::AutoArrange )
    {
        // when positioning moved entries "hard", there are problems with
        // unwanted overlaps, as these entries aren't taken into account in
        // Arrange.
        if( maEntries.size() )
            aAutoArrangeIdle.Start();
        return;
    }

    if( ePositionMode == SvxIconChoiceCtrlPositionMode::AutoArrange )
    {
        for( size_t nCur = 0; nCur < nCount; nCur++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = maEntries[ nCur ].get();
            if( pEntry->GetFlags() & SvxIconViewFlags(SvxIconViewFlags::POS_LOCKED | SvxIconViewFlags::POS_MOVED))
                SetEntryPos(pEntry, GetEntryBoundRect( pEntry ).TopLeft());
        }

        if( maEntries.size() )
            aAutoArrangeIdle.Start();
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

    bool bSetHead = false;
    if( !pPredecessor )
    {
        bSetHead = true;
        pPredecessor = pHead->pblink;
    }
    if( pEntry == pHead )
    {
        pHead = pHead->pflink;
        bSetHead = false;
    }
    if( pEntry != pPredecessor )
    {
        pEntry->Unlink();
        pEntry->SetBacklink( pPredecessor );
    }
    if( bSetHead )
        pHead = pEntry;
    aAutoArrangeIdle.Start();
}

SvxIconChoiceCtrlEntry* SvxIconChoiceCtrl_Impl::FindEntryPredecessor( SvxIconChoiceCtrlEntry* pEntry,
    const Point& rPosTopLeft )
{
    Point aPos( rPosTopLeft ); //TopLeft
    tools::Rectangle aCenterRect( CalcBmpRect( pEntry, &aPos ));
    Point aNewPos( aCenterRect.Center() );
    sal_uLong nGrid = GetPredecessorGrid( aNewPos );
    size_t nCount = maEntries.size();
    if( nGrid == ULONG_MAX )
        return nullptr;
    if( nGrid >= nCount )
        nGrid = nCount - 1;
    if( !pHead )
        return maEntries[ nGrid ].get();

    SvxIconChoiceCtrlEntry* pCur = pHead; // Grid 0
    // TODO: go through list from the end if nGrid > nCount/2
    for( sal_uLong nCur = 0; nCur < nGrid; nCur++ )
        pCur = pCur->pflink;

    return pCur;
}

sal_uLong SvxIconChoiceCtrl_Impl::GetPredecessorGrid( const Point& rPos) const
{
    Point aPos( rPos );
    aPos.AdjustX( -(LROFFS_WINBORDER) );
    aPos.AdjustY( -(TBOFFS_WINBORDER) );
    long nMaxCol = aVirtOutputSize.Width() / nGridDX;
    if( nMaxCol )
        nMaxCol--;
    long nGridX = aPos.X() / nGridDX;
    if( nGridX > nMaxCol )
        nGridX = nMaxCol;
    long nGridY = aPos.Y() / nGridDY;
    long nGridsX = aOutputSize.Width() / nGridDX;
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

bool SvxIconChoiceCtrl_Impl::RequestHelp( const HelpEvent& rHEvt )
{
    if ( !(rHEvt.GetMode() & HelpEventMode::QUICK ) )
        return false;

    Point aPos( pView->ScreenToOutputPixel(rHEvt.GetMousePosPixel() ) );
    aPos -= pView->GetMapMode().GetOrigin();
    SvxIconChoiceCtrlEntry* pEntry = GetEntry( aPos, true );

    if ( !pEntry )
        return false;

    OUString sQuickHelpText = pEntry->GetQuickHelpText();
    OUString aEntryText( SvtIconChoiceCtrl::GetEntryText( pEntry ) );
    tools::Rectangle aTextRect( CalcTextRect( pEntry, nullptr, &aEntryText ) );
    if ( ( !aTextRect.IsInside( aPos ) || aEntryText.isEmpty() ) && sQuickHelpText.isEmpty() )
        return false;

    tools::Rectangle aOptTextRect( aTextRect );
    aOptTextRect.SetBottom( LONG_MAX );
    DrawTextFlags nNewFlags = nCurTextDrawFlags;
    nNewFlags &= ~DrawTextFlags( DrawTextFlags::Clip | DrawTextFlags::EndEllipsis );
    aOptTextRect = pView->GetTextRect( aOptTextRect, aEntryText, nNewFlags );
    if ( aOptTextRect != aTextRect || !sQuickHelpText.isEmpty() )
    {
        //aTextRect.Right() = aTextRect.Left() + aRealSize.Width() + 4;
        Point aPt( aOptTextRect.TopLeft() );
        aPt += pView->GetMapMode().GetOrigin();
        aPt = pView->OutputToScreenPixel( aPt );
        // subtract border of tooltip help
        aPt.AdjustY( -1 );
        aPt.AdjustX( -3 );
        aOptTextRect.SetPos( aPt );
        OUString sHelpText;
        if ( !sQuickHelpText.isEmpty() )
            sHelpText = sQuickHelpText;
        else
            sHelpText = aEntryText;
        Help::ShowQuickHelp( static_cast<vcl::Window*>(pView), aOptTextRect, sHelpText, QuickHelpFlags::Left | QuickHelpFlags::VCenter );
    }

    return true;
}

void SvxIconChoiceCtrl_Impl::ClearColumnList()
{
    m_pColumns.reset();
}

void SvxIconChoiceCtrl_Impl::SetColumn( sal_uInt16 nIndex, const SvxIconChoiceCtrlColumnInfo& rInfo)
{
    if (!m_pColumns)
        m_pColumns.reset(new SvxIconChoiceCtrlColumnInfoMap);

    SvxIconChoiceCtrlColumnInfo* pInfo = new SvxIconChoiceCtrlColumnInfo( rInfo );
    m_pColumns->insert(std::make_pair(nIndex, std::unique_ptr<SvxIconChoiceCtrlColumnInfo>(pInfo)));

    // HACK: Detail mode is not yet fully implemented, this workaround makes it
    // fly with a single column
    if( !nIndex && (nWinBits & WB_DETAILS) )
        nGridDX = pInfo->GetWidth();

    if( GetUpdateMode() )
        Arrange( IsAutoArrange(), 0, 0 );
}

const SvxIconChoiceCtrlColumnInfo* SvxIconChoiceCtrl_Impl::GetColumn( sal_uInt16 nIndex ) const
{
    if (!m_pColumns)
        return nullptr;
    auto const it = m_pColumns->find( nIndex );
    if (it == m_pColumns->end())
        return nullptr;
    return it->second.get();
}

void SvxIconChoiceCtrl_Impl::DrawHighlightFrame(vcl::RenderContext& rRenderContext, const tools::Rectangle& rBmpRect)
{
    tools::Rectangle aBmpRect(rBmpRect);
    long nBorder = 2;
    if (aImageSize.Width() < 32)
        nBorder = 1;
    aBmpRect.AdjustRight(nBorder );
    aBmpRect.AdjustLeft( -nBorder );
    aBmpRect.AdjustBottom(nBorder );
    aBmpRect.AdjustTop( -nBorder );

    DecorationView aDecoView(&rRenderContext);
    DrawHighlightFrameStyle nDecoFlags;
    if (bHighlightFramePressed)
        nDecoFlags = DrawHighlightFrameStyle::In;
    else
        nDecoFlags = DrawHighlightFrameStyle::Out;
    aDecoView.DrawHighlightFrame(aBmpRect, nDecoFlags);
}

void SvxIconChoiceCtrl_Impl::SetEntryHighlightFrame( SvxIconChoiceCtrlEntry* pEntry,
    bool bKeepHighlightFlags )
{
    if( pEntry == pCurHighlightFrame )
        return;

    if( !bKeepHighlightFlags )
        bHighlightFramePressed = false;

    if (pCurHighlightFrame)
    {
        tools::Rectangle aInvalidationRect(GetEntryBoundRect(pCurHighlightFrame));
        aInvalidationRect.expand(5);
        pCurHighlightFrame = nullptr;
        pView->Invalidate(aInvalidationRect);
    }

    pCurHighlightFrame = pEntry;
    if (pEntry)
    {
        tools::Rectangle aInvalidationRect(GetEntryBoundRect(pEntry));
        aInvalidationRect.expand(5);
        pView->Invalidate(aInvalidationRect);
    }
}

void SvxIconChoiceCtrl_Impl::CallSelectHandler()
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
        pHdlEntry = nullptr;
        pView->ClickIcon();
        //pView->Select();
    }
    else
        aCallSelectHdlIdle.Start();
}

IMPL_LINK_NOARG(SvxIconChoiceCtrl_Impl, CallSelectHdlHdl, Timer *, void)
{
    pHdlEntry = nullptr;
    pView->ClickIcon();
    //pView->Select();
}

void SvxIconChoiceCtrl_Impl::SetOrigin( const Point& rPos )
{
    MapMode aMapMode( pView->GetMapMode() );
    aMapMode.SetOrigin( rPos );
    pView->SetMapMode( aMapMode );
}

void SvxIconChoiceCtrl_Impl::CallEventListeners( VclEventId nEvent, void* pData )
{
    pView->CallImplEventListeners( nEvent, pData );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
