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
#include <vcl/settings.hxx>

#include <svtools/ivctrl.hxx>
#include "imivctl.hxx"
#include <svtools/svmedit.hxx>

#include <algorithm>
#include <memory>
#include <vcl/idle.hxx>

#define IMPICNVIEW_ACC_RETURN 1
#define IMPICNVIEW_ACC_ESCAPE 2

#define DRAWTEXT_FLAGS_ICON \
    ( DrawTextFlags::Center | DrawTextFlags::Top | DrawTextFlags::EndEllipsis | \
      DrawTextFlags::Clip | DrawTextFlags::MultiLine | DrawTextFlags::WordBreak | DrawTextFlags::Mnemonic )

#define DRAWTEXT_FLAGS_SMALLICON (DrawTextFlags::Left|DrawTextFlags::EndEllipsis|DrawTextFlags::Clip)

#define EVENTID_SHOW_CURSOR             (reinterpret_cast<void*>(1))
#define EVENTID_ADJUST_SCROLLBARS       (reinterpret_cast<void*>(2))

static bool bEndScrollInvalidate = true;

class IcnViewEdit_Impl : public MultiLineEdit
{
    Link<LinkParamNone*,void> aCallBackHdl;
    Accelerator     aAccReturn;
    Accelerator     aAccEscape;
    Idle            maLoseFocusIdle;
    bool            bCanceled;
    bool            bAlreadyInCallback;
    bool            bGrabFocus;

    void            CallCallBackHdl_Impl();
                    DECL_LINK(Timeout_Impl, Timer *, void);
                    DECL_LINK( ReturnHdl_Impl, Accelerator&, void );
                    DECL_LINK( EscapeHdl_Impl, Accelerator&, void );

public:

                    IcnViewEdit_Impl(
                        SvtIconChoiceCtrl* pParent,
                        const Point& rPos,
                        const Size& rSize,
                        const OUString& rData,
                        const Link<LinkParamNone*,void>& rNotifyEditEnd );

    virtual         ~IcnViewEdit_Impl() override;
    virtual void    dispose() override;
    virtual void    KeyInput( const KeyEvent& rKEvt ) override;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;
    bool            EditingCanceled() const { return bCanceled; }
    void            StopEditing();
    bool            IsGrabFocus() const { return bGrabFocus; }
};

SvxIconChoiceCtrl_Impl::SvxIconChoiceCtrl_Impl(
    SvtIconChoiceCtrl* pCurView,
    WinBits nWinStyle
) :
    aEntries( this ),
    aVerSBar( VclPtr<ScrollBar>::Create(pCurView, WB_DRAG | WB_VSCROLL) ),
    aHorSBar( VclPtr<ScrollBar>::Create(pCurView, WB_DRAG | WB_HSCROLL) ),
    aScrBarBox( VclPtr<ScrollBarBox>::Create(pCurView) ),
    aEditIdle( "svtools contnr SvxIconChoiceCtrl_Impl Edit" ),
    aAutoArrangeIdle ( "svtools contnr SvxIconChoiceCtrl_Impl AutoArrange" ),
    aDocRectChangedIdle ( "svtools contnr SvxIconChoiceCtrl_Impl DocRectChanged" ),
    aVisRectChangedIdle ( "svtools contnr SvxIconChoiceCtrl_Impl VisRectChanged" ),
    aCallSelectHdlIdle ( "svtools contnr SvxIconChoiceCtrl_Impl CallSelectHdl" ),
    aImageSize( 32 * pCurView->GetDPIScaleFactor(), 32 * pCurView->GetDPIScaleFactor()),
    m_pColumns( nullptr )
{
    bChooseWithCursor = false;
    pEntryPaintDev = nullptr;
    pCurEditedEntry = nullptr;
    pCurHighlightFrame = nullptr;
    pEdit = nullptr;
    pAnchor = nullptr;
    pPrevDropTarget = nullptr;
    pHdlEntry = nullptr;
    pHead = nullptr;
    pCursor = nullptr;
    bUpdateMode = true;
    bEntryEditingEnabled = false;
    bHighlightFramePressed = false;
    eSelectionMode = SelectionMode::Multiple;
    pView = pCurView;
    pZOrderList = new SvxIconChoiceCtrlEntryList_impl();
    ePositionMode = IcnViewPositionModeFree;
    SetStyle( nWinStyle );
    nFlags = IconChoiceFlags::NONE;
    nUserEventAdjustScrBars = nullptr;
    nUserEventShowCursor = nullptr;
    nMaxVirtWidth = DEFAULT_MAX_VIRT_WIDTH;
    nMaxVirtHeight = DEFAULT_MAX_VIRT_HEIGHT;
    pDDRefEntry = nullptr;
    pDDDev = nullptr;
    pDDBufDev = nullptr;
    pDDTempDev = nullptr;
    eTextMode = IcnShowTextShort;
    pImpCursor = new IcnCursor_Impl( this );
    pGridMap = new IcnGridMap_Impl( this );

    aVerSBar->SetScrollHdl( LINK( this, SvxIconChoiceCtrl_Impl, ScrollUpDownHdl ) );
    aHorSBar->SetScrollHdl( LINK( this, SvxIconChoiceCtrl_Impl, ScrollLeftRightHdl ) );

    nHorSBarHeight = aHorSBar->GetSizePixel().Height();
    nVerSBarWidth = aVerSBar->GetSizePixel().Width();

    aEditIdle.SetPriority( TaskPriority::LOWEST );
    aEditIdle.SetInvokeHandler(LINK(this,SvxIconChoiceCtrl_Impl,EditTimeoutHdl));
    aEditIdle.SetDebugName( "svtools::SvxIconChoiceCtrl_Impl aEditIdle" );

    aAutoArrangeIdle.SetPriority( TaskPriority::LOW );
    aAutoArrangeIdle.SetInvokeHandler(LINK(this,SvxIconChoiceCtrl_Impl,AutoArrangeHdl));
    aAutoArrangeIdle.SetDebugName( "svtools::SvxIconChoiceCtrl_Impl aAutoArrangeIdle" );

    aCallSelectHdlIdle.SetPriority( TaskPriority::LOWEST );
    aCallSelectHdlIdle.SetInvokeHandler( LINK(this,SvxIconChoiceCtrl_Impl,CallSelectHdlHdl));
    aCallSelectHdlIdle.SetDebugName( "svtools::SvxIconChoiceCtrl_Impl aCallSelectHdlIdle" );

    aDocRectChangedIdle.SetPriority( TaskPriority::MEDIUM );
    aDocRectChangedIdle.SetInvokeHandler(LINK(this,SvxIconChoiceCtrl_Impl,DocRectChangedHdl));
    aDocRectChangedIdle.SetDebugName( "svtools::SvxIconChoiceCtrl_Impl aDocRectChangedIdle" );

    aVisRectChangedIdle.SetPriority( TaskPriority::MEDIUM );
    aVisRectChangedIdle.SetInvokeHandler(LINK(this,SvxIconChoiceCtrl_Impl,VisRectChangedHdl));
    aVisRectChangedIdle.SetDebugName( "svtools::SvxIconChoiceCtrl_Impl aVisRectChangedIdle" );

    Clear( true );
    Size gridSize(100,70);
    if(pView->GetDPIScaleFactor() > 1)
    {
      gridSize.Height() *= pView->GetDPIScaleFactor();
    }
    SetGrid(gridSize);
}

SvxIconChoiceCtrl_Impl::~SvxIconChoiceCtrl_Impl()
{
    pCurEditedEntry = nullptr;
    pEdit.disposeAndClear();
    Clear(false);
    StopEditTimer();
    CancelUserEvents();
    delete pZOrderList;
    delete pImpCursor;
    delete pGridMap;
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
    StopEntryEditing();
    nSelectionCount = 0;
    pCurHighlightFrame = nullptr;
    StopEditTimer();
    CancelUserEvents();
    ShowCursor( false );
    bBoundRectsDirty = false;
    nMaxBoundHeight = 0;

    pCursor = nullptr;
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
            pView->Invalidate(InvalidateFlags::NoChildren);
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
        eSelectionMode = SelectionMode::NONE;
    if( !(nWinStyle & (WB_ALIGN_TOP | WB_ALIGN_LEFT)))
        nWinBits |= WB_ALIGN_LEFT;
    if( (nWinStyle & WB_DETAILS))
    {
        if (!m_pColumns)
            SetColumn( 0, SvxIconChoiceCtrlColumnInfo() );
    }
}

IMPL_LINK( SvxIconChoiceCtrl_Impl, ScrollUpDownHdl, ScrollBar*, pScrollBar, void )
{
    StopEntryEditing();
    // arrow up: delta=-1; arrow down: delta=+1
    Scroll( 0, pScrollBar->GetDelta() );
    bEndScrollInvalidate = true;
}

IMPL_LINK( SvxIconChoiceCtrl_Impl, ScrollLeftRightHdl, ScrollBar*, pScrollBar, void )
{
    StopEntryEditing();
    // arrow left: delta=-1; arrow right: delta=+1
    Scroll( pScrollBar->GetDelta(), 0 );
    bEndScrollInvalidate = true;
}

void SvxIconChoiceCtrl_Impl::FontModified()
{
    StopEditTimer();
    pDDDev.disposeAndClear();
    pDDBufDev.disposeAndClear();
    pDDTempDev.disposeAndClear();
    pEntryPaintDev.disposeAndClear();
    SetDefaultTextSize();
    ShowCursor( false );
    ShowCursor( true );
}

void SvxIconChoiceCtrl_Impl::InsertEntry( SvxIconChoiceCtrlEntry* pEntry, size_t nPos)
{
    StopEditTimer();
    aEntries.insert( nPos, pEntry );
    if( (nFlags & IconChoiceFlags::EntryListPosValid) && nPos >= aEntries.size() - 1 )
        pEntry->nPos = aEntries.size() - 1;
    else
        nFlags &= ~IconChoiceFlags::EntryListPosValid;

    pZOrderList->push_back( pEntry );
    pImpCursor->Clear();

    // If the UpdateMode is true, don't set all bounding rectangles to
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

Rectangle SvxIconChoiceCtrl_Impl::GetOutputRect() const
{
    Point aOrigin( pView->GetMapMode().GetOrigin() );
    aOrigin *= -1;
    return Rectangle( aOrigin, aOutputSize );
}

void SvxIconChoiceCtrl_Impl::SetListPositions()
{
    if( nFlags & IconChoiceFlags::EntryListPosValid )
        return;

    size_t nCount = aEntries.size();
    for( size_t nCur = 0; nCur < nCount; nCur++ )
    {
        SvxIconChoiceCtrlEntry* pEntry = aEntries[ nCur ];
        pEntry->nPos = nCur;
    }
    nFlags |= IconChoiceFlags::EntryListPosValid;
}

void SvxIconChoiceCtrl_Impl::SelectEntry( SvxIconChoiceCtrlEntry* pEntry, bool bSelect,
    bool bAdd, bool bSyncPaint )
{
    if( eSelectionMode == SelectionMode::NONE )
        return;

    if( !bAdd )
    {
        if ( !( nFlags & IconChoiceFlags::ClearingSelection ) )
        {
            nFlags |= IconChoiceFlags::ClearingSelection;
            DeselectAllBut( pEntry, true );
            nFlags &= ~IconChoiceFlags::ClearingSelection;
        }
    }
    if( pEntry->IsSelected() != bSelect )
    {
        pHdlEntry = pEntry;
        SvxIconViewFlags nEntryFlags = pEntry->GetFlags();
        if( bSelect )
        {
            nEntryFlags |= SvxIconViewFlags::SELECTED;
            pEntry->AssignFlags( nEntryFlags );
            nSelectionCount++;
            CallSelectHandler( pEntry );
        }
        else
        {
            nEntryFlags &= ~( SvxIconViewFlags::SELECTED);
            pEntry->AssignFlags( nEntryFlags );
            nSelectionCount--;
            CallSelectHandler( nullptr );
        }
        EntrySelected( pEntry, bSelect, bSyncPaint );
    }
}

void SvxIconChoiceCtrl_Impl::EntrySelected(SvxIconChoiceCtrlEntry* pEntry, bool bSelect, bool /*bSyncPaint*/)
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
    StopEditTimer();
    aVirtOutputSize.Width() = 0;
    aVirtOutputSize.Height() = 0;
    const size_t nCount = aEntries.size();
    for( size_t nCur = 0; nCur < nCount; nCur++ )
    {
        SvxIconChoiceCtrlEntry* pCur = aEntries[ nCur ];
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
        aHorSBar->SetRange( aRange );

        aVirtOutputSize.Height() += nHeightOffs;
        aRange.Max() = aVirtOutputSize.Height();
        aVerSBar->SetRange( aRange );

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
            pPrev->ClearFlags( SvxIconViewFlags::POS_LOCKED | SvxIconViewFlags::POS_MOVED |
                                SvxIconViewFlags::PRED_SET);

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
        pHead = nullptr;
}

void SvxIconChoiceCtrl_Impl::ClearPredecessors()
{
    if( pHead )
    {
        size_t nCount = aEntries.size();
        for( size_t nCur = 0; nCur < nCount; nCur++ )
        {
            SvxIconChoiceCtrlEntry* pCur = aEntries[ nCur ];
            pCur->pflink = nullptr;
            pCur->pblink = nullptr;
            pCur->ClearFlags( SvxIconViewFlags::PRED_SET );
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
    Rectangle aCurOutputArea( GetOutputRect() );
    if( (nWinBits & WB_SMART_ARRANGE) && aCurOutputArea.TopLeft() != aEmptyPoint )
        bUpdateMode = false;
    aAutoArrangeIdle.Stop();
    nFlags |= IconChoiceFlags::Arranging;
    StopEditTimer();
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

void SvxIconChoiceCtrl_Impl::Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect)
{
    bEndScrollInvalidate = false;

#if defined(OV_DRAWGRID)
    Color aOldColor (rRenderContext.GetLineColor());
    Color aColor(COL_BLACK);
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

    if (!aEntries.size())
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
            pCursor = aEntries[ 0 ];
    }

    size_t nCount = pZOrderList->size();
    if (!nCount)
        return;

    rRenderContext.Push(PushFlags::CLIPREGION);
    rRenderContext.SetClipRegion(vcl::Region(rRect));

    SvxIconChoiceCtrlEntryList_impl* pNewZOrderList = new SvxIconChoiceCtrlEntryList_impl();
    std::unique_ptr<SvxIconChoiceCtrlEntryList_impl> pPaintedEntries(new SvxIconChoiceCtrlEntryList_impl());

    size_t nPos = 0;
    while(nCount)
    {
        SvxIconChoiceCtrlEntry* pEntry = (*pZOrderList)[nPos];
        const Rectangle& rBoundRect = GetEntryBoundRect(pEntry);
        if (rRect.IsOver(rBoundRect))
        {
            PaintEntry(pEntry, rBoundRect.TopLeft(), rRenderContext);
            // set entries to Top if they are being repainted
            pPaintedEntries->push_back(pEntry);
        }
        else
            pNewZOrderList->push_back(pEntry);

        nCount--;
        nPos++;
    }
    delete pZOrderList;
    pZOrderList = pNewZOrderList;
    nCount = pPaintedEntries->size();
    if (nCount)
    {
        for (size_t nCur = 0; nCur < nCount; nCur++)
            pZOrderList->push_back((*pPaintedEntries)[nCur]);
    }
    pPaintedEntries.reset();

    rRenderContext.Pop();
}

void SvxIconChoiceCtrl_Impl::RepaintSelectedEntries()
{
    const size_t nCount = pZOrderList->size();
    if (!nCount)
        return;

    Rectangle aOutRect(GetOutputRect());
    for (size_t nCur = 0; nCur < nCount; nCur++)
    {
        SvxIconChoiceCtrlEntry* pEntry = (*pZOrderList)[nCur];
        if (pEntry->GetFlags() & SvxIconViewFlags::SELECTED)
        {
            const Rectangle& rBoundRect = GetEntryBoundRect(pEntry);
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
    StopEditTimer();
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
            SetCursor_Impl( pCursor, pEntry, rMEvt.IsMod1(), rMEvt.IsShift(), true);
        return true;
    }

    if( pAnchor && (rMEvt.IsShift() || rMEvt.IsMod1())) // keyboard selection?
    {
        DBG_ASSERT(eSelectionMode != SelectionMode::Single,"Invalid selection mode");
        if( rMEvt.IsMod1() )
            nFlags |= IconChoiceFlags::AddMode;

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
    bool bEditingEnabled = bEntryEditingEnabled;

    if( rMEvt.GetClicks() == 2 )
    {
        DeselectAllBut( pEntry );
        SelectEntry( pEntry, true, false, true );
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
                EditEntry( pEntry );
            }
        }
        else if( eSelectionMode == SelectionMode::Single )
        {
            DeselectAllBut( pEntry );
            SetCursor( pEntry );
            if( bEditingEnabled && bSelected && !rMEvt.GetModifier() &&
                rMEvt.IsLeft() && IsTextHit( pEntry, aDocPos ) )
            {
                nFlags |= IconChoiceFlags::StartEditTimerInMouseUp;
            }
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
                    DeselectAllBut( pEntry, true /* paint synchronously */ );
                    SetCursor( pEntry );
                    SelectEntry( pEntry, true, false, true );
                }
                else
                {
                    // deselect only in the Up, if the Move happened via D&D!
                    nFlags |= IconChoiceFlags::DownDeselect;
                    if( bEditingEnabled && IsTextHit( pEntry, aDocPos ) &&
                        rMEvt.IsLeft())
                    {
                        nFlags |= IconChoiceFlags::StartEditTimerInMouseUp;
                    }
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
            SelectEntry( pDocEntry, true, false, true );
            bHandled = true;
        }
    }

    nFlags &= ~IconChoiceFlags(IconChoiceFlags::DownCtrl | IconChoiceFlags::DownDeselect);
    if( nFlags & IconChoiceFlags::StartEditTimerInMouseUp )
    {
        bHandled = true;
        StartEditTimer();
        nFlags &= ~IconChoiceFlags::StartEditTimerInMouseUp;
    }

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
        SetCursor_Impl( pOldCursor, pHdlEntry, false, false, true );

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
    SvxIconChoiceCtrlEntry* pNewCursor, bool bMod1, bool bShift, bool bPaintSync )
{
    if( pNewCursor )
    {
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
            DeselectAllBut( pFilterEntry, bPaintSync );
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
            SelectEntry( pCursor, true, false, bPaintSync );
            aCurSelectionRect = GetEntryBoundRect( pCursor );
            CallEventListeners( VclEventId::ListboxSelect, pCursor );
        }
    }
}

bool SvxIconChoiceCtrl_Impl::KeyInput( const KeyEvent& rKEvt )
{
    StopEditTimer();

    bool bMod2 = rKEvt.GetKeyCode().IsMod2();
    sal_Unicode cChar = rKEvt.GetCharCode();
    sal_uLong nPos = (sal_uLong)-1;
    if ( bMod2 && cChar && IsMnemonicChar( cChar, nPos ) )
    {
        // shortcut is clicked
        SvxIconChoiceCtrlEntry* pNewCursor = GetEntry( nPos );
        SvxIconChoiceCtrlEntry* pOldCursor = pCursor;
        if ( pNewCursor != pOldCursor )
            SetCursor_Impl( pOldCursor, pNewCursor, false, false, false );
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
                SetCursor_Impl( pOldCursor, pNewCursor, bMod1, bShift, true );
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
                SetCursor_Impl( pOldCursor, pNewCursor, bMod1, bShift, true );

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
                SetCursor_Impl( pOldCursor, pNewCursor, bMod1, bShift, true );
            }
            break;

        case KEY_LEFT:
            if( pCursor )
            {
                MakeEntryVisible( pCursor );
                pNewCursor = pImpCursor->GoLeftRight(pCursor,false );
                SetCursor_Impl( pOldCursor, pNewCursor, bMod1, bShift, true );
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
                EditTimeoutHdl( nullptr );
            else
                bKeyUsed = false;
            break;

        case KEY_F8:
            if( rKEvt.GetKeyCode().IsShift() )
            {
                if( nFlags & IconChoiceFlags::AddMode )
                    nFlags &= (~IconChoiceFlags::AddMode);
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
            if( bMod1 )
            {
                if( pCursor && bEntryEditingEnabled )
                    /*pView->*/EditEntry( pCursor );
            }
            else
                bKeyUsed = false;
            break;

        case KEY_END:
            if( pCursor )
            {
                pNewCursor = aEntries[ aEntries.size() - 1 ];
                SetCursor_Impl( pOldCursor, pNewCursor, bMod1, bShift, true );
            }
            break;

        case KEY_HOME:
            if( pCursor )
            {
                pNewCursor = aEntries[ 0 ];
                SetCursor_Impl( pOldCursor, pNewCursor, bMod1, bShift, true );
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
    aPos.Y() -= nHorSBarHeight;

    if( aHorSBar->GetPosPixel() != aPos )
        aHorSBar->SetPosPixel( aPos );

    // vertical scrollbar
    aPos.X() = nRealWidth; aPos.Y() = 0;
    aPos.X() -= nVerSBarWidth;
    aPos.X()++;
    aPos.Y()--;

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
    aSize.Height() += 2;
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
    aSize.Width() = nRealWidth;
    aSize.Height() = nHorSBarHeight;
    aSize.Width()++;
    if( nResult & 0x0001 ) // vertical scrollbar?
    {
        aSize.Width()++;
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

    aOutputSize.Width() = nRealWidth;
    if( nResult & 0x0002 ) // horizontal scrollbar ?
        nRealHeight++; // because lower border is clipped
    aOutputSize.Height() = nRealHeight;

    if( (nResult & (0x0001|0x0002)) == (0x0001|0x0002) )
        aScrBarBox->Show();
    else
        aScrBarBox->Hide();
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

    VisRectChanged();
}

bool SvxIconChoiceCtrl_Impl::CheckHorScrollBar()
{
    if( !pZOrderList || !aHorSBar->IsVisible() )
        return false;
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
                return false;
            if( nRight > nMostRight )
                nMostRight = nRight;
        }
        aHorSBar->Hide();
        aOutputSize.Height() += nHorSBarHeight;
        aVirtOutputSize.Width() = nMostRight;
        aHorSBar->SetThumbPos( 0 );
        Range aRange;
        aRange.Max() = nMostRight - 1;
        aHorSBar->SetRange( aRange  );
        if( aVerSBar->IsVisible() )
        {
            Size aSize( aVerSBar->GetSizePixel());
            aSize.Height() += nHorSBarHeight;
            aVerSBar->SetSizePixel( aSize );
        }
        return true;
    }
    return false;
}

bool SvxIconChoiceCtrl_Impl::CheckVerScrollBar()
{
    if( !pZOrderList || !aVerSBar->IsVisible() )
        return false;
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
                return false;
            if( nBottom > nDeepest )
                nDeepest = nBottom;
        }
        aVerSBar->Hide();
        aOutputSize.Width() += nVerSBarWidth;
        aVirtOutputSize.Height() = nDeepest;
        aVerSBar->SetThumbPos( 0 );
        Range aRange;
        aRange.Max() = nDeepest - 1;
        aVerSBar->SetRange( aRange  );
        if( aHorSBar->IsVisible() )
        {
            Size aSize( aHorSBar->GetSizePixel());
            aSize.Width() += nVerSBarWidth;
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
    StopEditTimer();
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

// priorities of the emphasis:  bDropTarget => bCursored => bSelected
void SvxIconChoiceCtrl_Impl::PaintEmphasis(const Rectangle& rTextRect, bool bSelected,
                                           bool bDropTarget, bool bCursored, vcl::RenderContext& rRenderContext)
{
    static Color aTransparent(COL_TRANSPARENT);

    const StyleSettings& rSettings = rRenderContext.GetSettings().GetStyleSettings();
    Color aOldFillColor(rRenderContext.GetFillColor());

    bool bSolidTextRect = false;

    if(bDropTarget && (eSelectionMode != SelectionMode::NONE))
    {
        rRenderContext.SetFillColor(rSettings.GetHighlightColor());
        bSolidTextRect = true;
    }
    else
    {
        if (!bSelected || bCursored)
        {
            const Color& rFillColor = rRenderContext.GetFont().GetFillColor();
            rRenderContext.SetFillColor(rFillColor);
            if (rFillColor != aTransparent)
                bSolidTextRect = true;
        }
    }

    // draw text rectangle
    if (bSolidTextRect)
    {
        Color aOldLineColor;
        if (bCursored)
        {
            aOldLineColor = rRenderContext.GetLineColor();
            rRenderContext.SetLineColor(Color(COL_GRAY));
        }
        rRenderContext.DrawRect(rTextRect);
        if (bCursored)
            rRenderContext.SetLineColor(aOldLineColor);
    }

    rRenderContext.SetFillColor(aOldFillColor);
}


void SvxIconChoiceCtrl_Impl::PaintItem(const Rectangle& rRect,
    IcnViewFieldType eItem, SvxIconChoiceCtrlEntry* pEntry, sal_uInt16 nPaintFlags,
    vcl::RenderContext& rRenderContext )
{
    if (eItem == IcnViewFieldType::Text)
    {
        OUString aText = SvtIconChoiceCtrl::GetEntryText(pEntry, false);

        rRenderContext.DrawText(rRect, aText, nCurTextDrawFlags);

        if (pEntry->IsFocused())
        {
            Rectangle aRect (CalcFocusRect(pEntry));
            ShowFocus(aRect);
            DrawFocusRect(rRenderContext);
        }
    }
    else
    {
        Point aPos(rRect.TopLeft());
        if (nPaintFlags & PAINTFLAG_HOR_CENTERED)
            aPos.X() += (rRect.GetWidth() - aImageSize.Width()) / 2;
        if (nPaintFlags & PAINTFLAG_VER_CENTERED)
            aPos.Y() += (rRect.GetHeight() - aImageSize.Height()) / 2;
        SvtIconChoiceCtrl::DrawEntryImage(pEntry, aPos, rRenderContext);
    }
}

void SvxIconChoiceCtrl_Impl::PaintEntry(SvxIconChoiceCtrlEntry* pEntry, const Point& rPos, vcl::RenderContext& rRenderContext)
{
    bool bSelected = false;

    if (eSelectionMode != SelectionMode::NONE)
        bSelected = pEntry->IsSelected();

    bool bCursored = pEntry->IsCursored();
    bool bDropTarget = pEntry->IsDropTarget();
    bool bNoEmphasis = pEntry->IsBlockingEmphasis();

    rRenderContext.Push(PushFlags::FONT | PushFlags::TEXTCOLOR);

    OUString aEntryText(SvtIconChoiceCtrl::GetEntryText(pEntry, false));
    Rectangle aTextRect(CalcTextRect(pEntry, &rPos, false, &aEntryText));
    Rectangle aBmpRect(CalcBmpRect(pEntry, &rPos));

    bool bShowSelection = ((bSelected && !bCursored) && !bNoEmphasis && (eSelectionMode != SelectionMode::NONE));

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
        Rectangle aOutputArea(GetOutputRect());
        if (aOutputArea.IsOver(aTextRect) || aOutputArea.IsOver(aBmpRect))
        {
            rRenderContext.SetClipRegion(vcl::Region(aOutputArea));
            bResetClipRegion = true;
        }
    }

    bool bLargeIconMode = WB_ICON == ( nWinBits & (VIEWMODE_MASK) );
    sal_uInt16 nBmpPaintFlags = PAINTFLAG_VER_CENTERED;
    if (bLargeIconMode)
        nBmpPaintFlags |= PAINTFLAG_HOR_CENTERED;
    sal_uInt16 nTextPaintFlags = bLargeIconMode ? PAINTFLAG_HOR_CENTERED : PAINTFLAG_VER_CENTERED;

    if( !bNoEmphasis )
        PaintEmphasis(aTextRect, bSelected, bDropTarget, bCursored, rRenderContext);

    if ( bShowSelection )
        vcl::RenderTools::DrawSelectionBackground(rRenderContext, *pView.get(), CalcFocusRect(pEntry),
                                                  bActiveSelection ? 1 : 2, false, true, false);


    PaintItem(aBmpRect, IcnViewFieldType::Image, pEntry, nBmpPaintFlags, rRenderContext);

    PaintItem(aTextRect, IcnViewFieldType::Text, pEntry, nTextPaintFlags, rRenderContext);

    // draw highlight frame
    if (pEntry == pCurHighlightFrame && !bNoEmphasis)
        DrawHighlightFrame(rRenderContext, CalcFocusRect(pEntry));

    rRenderContext.Pop();
    if (bResetClipRegion)
        rRenderContext.SetClipRegion();
}

void SvxIconChoiceCtrl_Impl::SetEntryPos( SvxIconChoiceCtrlEntry* pEntry, const Point& rPos )
{
    ShowCursor( false );
    Rectangle aBoundRect( GetEntryBoundRect( pEntry ));
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
        DeselectAllBut( nullptr, true );
        nFlags &= ~IconChoiceFlags::ClearingSelection;
    }
}

SvxIconChoiceCtrlEntry* SvxIconChoiceCtrl_Impl::GetEntry( const Point& rDocPos, bool bHit )
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
    return nullptr;
}

void SvxIconChoiceCtrl_Impl::MakeEntryVisible( SvxIconChoiceCtrlEntry* pEntry, bool bBound )
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
    const Point* pEntryPos, bool bEdit, const OUString* pStr )
{
    OUString aEntryText;
    if( !pStr )
        aEntryText = SvtIconChoiceCtrl::GetEntryText( pEntry, bEdit );
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
    long nStringWidth = GetItemSize( pEntry, IcnViewFieldType::Text ).Width();
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
    long nStringHeight = GetItemSize( pEntry, IcnViewFieldType::Text).Height();
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
        const_cast<SvxIconChoiceCtrl_Impl*>(this)->nMaxBoundHeight = nHeight;
        const_cast<SvxIconChoiceCtrl_Impl*>(this)->aHorSBar->SetLineSize( GetScrollBarLineSize() );
        const_cast<SvxIconChoiceCtrl_Impl*>(this)->aVerSBar->SetLineSize( GetScrollBarLineSize() );
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
    Point aPos(pGridMap->GetGridRect(pGridMap->GetUnoccupiedGrid()).TopLeft());
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
    Rectangle aRect ( CalcFocusRect( pCursor ) );
    /*pView->*/ShowFocus( aRect );
}


void SvxIconChoiceCtrl_Impl::HideDDIcon()
{
    pView->Update();
    if( pDDDev )
    {
        Size aSize( pDDDev->GetOutputSizePixel() );
        // restore pView
        pView->DrawOutDev( aDDLastRectPos, aSize, Point(), aSize, *pDDDev );
    }
    pDDBufDev = pDDDev;
    pDDDev = nullptr;
}

bool SvxIconChoiceCtrl_Impl::HandleScrollCommand( const CommandEvent& rCmd )
{
    Rectangle aDocRect( Point(), aVirtOutputSize );
    Rectangle aVisRect( GetOutputRect() );
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
                    nScrollDY = pData->GetNotchDelta() * (long)nScrollLines;
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
        aVisRect.Top() -= nScrollDY;
        aVisRect.Bottom() -= nScrollDY;
        aVisRect.Left() -= nScrollDX;
        aVisRect.Right() -= nScrollDX;
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
// bScrBar == true: rectangle was calculated because of a scrollbar event

void SvxIconChoiceCtrl_Impl::MakeVisible( const Rectangle& rRect, bool bScrBar )
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
    if( pEntry->IsSelected() )
        bSel = false;
    else
        bSel = true;
    SelectEntry( pEntry, bSel, true );
}

void SvxIconChoiceCtrl_Impl::DeselectAllBut( SvxIconChoiceCtrlEntry* pThisEntryNot,
    bool bPaintSync )
{
    ClearSelectedRectList();

    // TODO: work through z-order list, if necessary!

    size_t nCount = aEntries.size();
    for( size_t nCur = 0; nCur < nCount; nCur++ )
    {
        SvxIconChoiceCtrlEntry* pEntry = aEntries[ nCur ];
        if( pEntry != pThisEntryNot && pEntry->IsSelected() )
            SelectEntry( pEntry, false, true, bPaintSync );
    }
    pAnchor = nullptr;
    nFlags &= (~IconChoiceFlags::AddMode);
}

Size SvxIconChoiceCtrl_Impl::GetMinGrid() const
{
    Size aMinSize( aImageSize );
    aMinSize.Width() += 2 * LROFFS_BOUND;
    aMinSize.Height() += TBOFFS_BOUND;  // single offset is enough (FileDlg)
    OUString aStrDummy( "XXX" );
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
            const_cast<SvxIconChoiceCtrlColumnInfo*>(pCol)->SetWidth( nGridDX );
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

    Rectangle aBmpRect( const_cast<SvxIconChoiceCtrl_Impl*>(this)->CalcBmpRect(
        const_cast<SvxIconChoiceCtrlEntry*>(pEntry) ) );
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
void SvxIconChoiceCtrl_Impl::Scroll( long nDeltaX, long nDeltaY )
{
    const MapMode& rMapMode = pView->GetMapMode();
    Point aOrigin( rMapMode.GetOrigin() );
    // convert to document coordinate
    aOrigin *= -1;
    aOrigin.Y() += nDeltaY;
    aOrigin.X() += nDeltaX;
    Rectangle aRect( aOrigin, aOutputSize );
    MakeVisible( aRect, true/*bScrollBar*/ );
}


const Size& SvxIconChoiceCtrl_Impl::GetItemSize( SvxIconChoiceCtrlEntry*,
    IcnViewFieldType eItem ) const
{
    if (eItem == IcnViewFieldType::Text)
        return aDefaultTextSize;
    return aImageSize; // IcnViewFieldType::Image
}

Rectangle SvxIconChoiceCtrl_Impl::CalcFocusRect( SvxIconChoiceCtrlEntry* pEntry )
{
    Rectangle aTextRect( CalcTextRect( pEntry ) );
    Rectangle aBoundRect( GetEntryBoundRect( pEntry ) );
    return Rectangle(
        aBoundRect.Left(), aBoundRect.Top() - 1, aBoundRect.Right() - 1,
        aTextRect.Bottom() + 1);
}

// the hot spot is the inner 50% of the rectangle
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
    bool bAdd, std::vector<Rectangle*>* pOtherRects )
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
    bool bAlreadySelectingRect(nFlags & IconChoiceFlags::SelectingRect);
    nFlags |= IconChoiceFlags::SelectingRect;

    CheckBoundingRects();
    pView->Update();
    const size_t nCount = pZOrderList->size();

    Rectangle aRect( rRect );
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
        SvxIconChoiceCtrlEntry* pEntry = (*pZOrderList)[ nPos ];

        if( !IsBoundingRectValid( pEntry->aRect ))
            FindBoundingRect( pEntry );
        Rectangle aBoundRect( GetHotSpot( pEntry->aRect ) );
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
                        SvxIconChoiceCtrlEntry* pStart,
                        SvxIconChoiceCtrlEntry* pEnd,
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
                SelectEntry( pEntry, false, true, true );
        }
    }

    // select everything between nFirst and nLast
    for ( i=nFirst; i<=nLast; i++ )
    {
        pEntry = GetEntry( i );
        if( ! pEntry->IsSelected() )
            SelectEntry( pEntry, true, true, true );
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
                SelectEntry( pEntry, false, true, true );
        }
    }
}

bool SvxIconChoiceCtrl_Impl::IsOver( std::vector<Rectangle*>* pRectList, const Rectangle& rBoundRect )
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

bool SvxIconChoiceCtrl_Impl::IsTextHit( SvxIconChoiceCtrlEntry* pEntry, const Point& rDocPos )
{
    Rectangle aRect( CalcTextRect( pEntry ));
    if( aRect.IsInside( rDocPos ) )
        return true;
    return false;
}

IMPL_LINK_NOARG(SvxIconChoiceCtrl_Impl, EditTimeoutHdl, Timer *, void)
{
    SvxIconChoiceCtrlEntry* pEntry = GetCurEntry();
    if( bEntryEditingEnabled && pEntry &&
        pEntry->IsSelected())
    {
        EditEntry( pEntry );
    }
}


// Function to align entries to the grid


// pStart == 0: align all entries
// else: align all entries of the row from pStart on (including pStart)
void SvxIconChoiceCtrl_Impl::AdjustEntryAtGrid()
{
    IconChoiceMap aLists;
    pImpCursor->CreateGridAjustData( aLists );
    for (IconChoiceMap::const_iterator iter = aLists.begin();
            iter != aLists.end(); ++iter)
    {
        AdjustAtGrid(iter->second);
    }
    IcnCursor_Impl::DestroyGridAdjustData( aLists );
    CheckScrollBars();
}

// align a row, might expand width, doesn't break the line
void SvxIconChoiceCtrl_Impl::AdjustAtGrid( const SvxIconChoiceCtrlEntryPtrVec& rRow )
{
    if( rRow.empty() )
        return;

    long nCurRight = 0;
    for(SvxIconChoiceCtrlEntry* pCur : rRow)
    {
        // Decisive (for our eye) is the bitmap, else, the entry might jump too
        // much within long texts.
        const Rectangle& rBoundRect = GetEntryBoundRect( pCur );
        Rectangle aCenterRect( CalcBmpRect( pCur ));
        if( !pCur->IsPosLocked() )
        {
            long nWidth = aCenterRect.GetSize().Width();
            Point aNewPos( AdjustAtGrid( aCenterRect, rBoundRect ) );
            while( aNewPos.X() < nCurRight )
                aNewPos.X() += nGridDX;
            if( aNewPos != rBoundRect.TopLeft() )
            {
                SetEntryPos( pCur, aNewPos );
                pCur->SetFlags( SvxIconViewFlags::POS_MOVED );
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

#ifdef DBG_UTIL
void SvxIconChoiceCtrl_Impl::SetEntryTextMode( SvxIconChoiceCtrlTextMode eMode, SvxIconChoiceCtrlEntry* pEntry )
{
    if( !pEntry )
    {
        if( eTextMode != eMode )
        {
            if( eTextMode == IcnShowTextDontKnow )
                eTextMode = IcnShowTextShort;
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

SvxIconChoiceCtrlTextMode SvxIconChoiceCtrl_Impl::GetEntryTextModeSmart( const SvxIconChoiceCtrlEntry* pEntry ) const
{
    DBG_ASSERT(pEntry,"GetEntryTextModeSmart: Entry not set");
    SvxIconChoiceCtrlTextMode eMode = pEntry->GetTextMode();
    if( eMode == IcnShowTextDontKnow )
        return eTextMode;
    return eMode;
}


// Draw my own focusrect, because the focusrect of the outputdevice has got the inverted color
// of the background. But what will we see, if the backgroundcolor is gray ? - We will see
// a gray focusrect on a gray background !!!

void SvxIconChoiceCtrl_Impl::ShowFocus ( Rectangle& rRect )
{
    Color aBkgColor(pView->GetBackground().GetColor());
    Color aPenColor;
    sal_uInt16 nColor = ( aBkgColor.GetRed() + aBkgColor.GetGreen() + aBkgColor.GetBlue() ) / 3;
    if (nColor > 128)
        aPenColor.SetColor(COL_BLACK);
    else
        aPenColor.SetColor(COL_WHITE);

    aFocus.bOn = true;
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
    aLineInfo.SetDotLen(1L);
    aLineInfo.SetDistance(1L);
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
        nUserEventShowCursor = nullptr;
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
    if( nUserEventShowCursor )
    {
        Application::RemoveUserEvent( nUserEventShowCursor );
        nUserEventShowCursor = nullptr;
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

void SvxIconChoiceCtrl_Impl::EditEntry( SvxIconChoiceCtrlEntry* pEntry )
{
    DBG_ASSERT(pEntry,"EditEntry: Entry not set");
    if( !pEntry )
        return;

    StopEntryEditing();
    pEdit.disposeAndClear();
    SetNoSelection();

    pCurEditedEntry = pEntry;
    OUString aEntryText( SvtIconChoiceCtrl::GetEntryText( pEntry, true ) );
    Rectangle aRect( CalcTextRect( pEntry, nullptr, true, &aEntryText ) );
    MakeVisible( aRect );
    Point aPos( aRect.TopLeft() );
    aPos = pView->GetPixelPos( aPos );
    aRect.SetPos( aPos );
    pView->HideFocus();
    pEdit = VclPtr<IcnViewEdit_Impl>::Create(

        pView,
        aRect.TopLeft(),
        aRect.GetSize(),
        aEntryText,
        LINK( this, SvxIconChoiceCtrl_Impl, TextEditEndedHdl ) );
}

IMPL_LINK_NOARG(SvxIconChoiceCtrl_Impl, TextEditEndedHdl, LinkParamNone*, void)
{
    DBG_ASSERT(pEdit,"TextEditEnded: pEdit not set");
    if( !pEdit )
    {
        pCurEditedEntry = nullptr;
        return;
    }
    DBG_ASSERT(pCurEditedEntry,"TextEditEnded: pCurEditedEntry not set");

    if( !pCurEditedEntry )
    {
        pEdit->Hide();
        if( pEdit->IsGrabFocus() )
            pView->GrabFocus();
        return;
    }

    OUString aText;
    if ( !pEdit->EditingCanceled() )
        aText = pEdit->GetText();
    else
        aText = pEdit->GetSavedValue();

    InvalidateEntry( pCurEditedEntry );
    if( !GetSelectionCount() )
        SelectEntry( pCurEditedEntry, true );

    pEdit->Hide();
    if( pEdit->IsGrabFocus() )
        pView->GrabFocus();
    // The edit can not be deleted here, because it is not within a handler. It
    // will be deleted in the dtor or in the next EditEntry.
    pCurEditedEntry = nullptr;
}

void SvxIconChoiceCtrl_Impl::StopEntryEditing()
{
    if( pEdit )
        pEdit->StopEditing();
}

SvxIconChoiceCtrlEntry* SvxIconChoiceCtrl_Impl::GetFirstSelectedEntry() const
{
    if( !GetSelectionCount() )
        return nullptr;

    if( (nWinBits & WB_HIGHLIGHTFRAME) && (eSelectionMode == SelectionMode::NONE) )
    {
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
    bool bPaint = true;

    size_t nCount = aEntries.size();
    for( size_t nCur = 0; nCur < nCount; nCur++ )
    {
        SvxIconChoiceCtrlEntry* pEntry = aEntries[ nCur ];
        SelectEntry( pEntry, true/*bSelect*/, true, bPaint );
    }
    nFlags &= (~IconChoiceFlags::AddMode);
    pAnchor = nullptr;
}

IcnViewEdit_Impl::IcnViewEdit_Impl( SvtIconChoiceCtrl* pParent, const Point& rPos,
    const Size& rSize, const OUString& rData, const Link<LinkParamNone*,void>& rNotifyEditEnd ) :
    MultiLineEdit( pParent, (pParent->GetStyle() & WB_ICON) ? WB_CENTER : WB_LEFT),
    aCallBackHdl( rNotifyEditEnd ),
    bCanceled( false ),
    bAlreadyInCallback( false ),
    bGrabFocus( false )
{
    maLoseFocusIdle.SetPriority(TaskPriority::REPAINT);
    maLoseFocusIdle.SetInvokeHandler(LINK(this,IcnViewEdit_Impl,Timeout_Impl));
    maLoseFocusIdle.SetDebugName( "svx::IcnViewEdit_Impl maLoseFocusIdle" );

    // FIXME: Outside of Paint Hierarchy
    vcl::Font aFont(pParent->GetPointFont(*this));
    aFont.SetTransparent( false );
    SetControlFont(aFont);
    SetControlBackground(aFont.GetFillColor());
    SetControlForeground(aFont.GetColor());
    SetPosPixel(rPos);
    SetSizePixel(CalcAdjustedSize(rSize));
    SetText(rData);
    SaveValue();

    aAccReturn.InsertItem( IMPICNVIEW_ACC_RETURN, vcl::KeyCode(KEY_RETURN) );
    aAccEscape.InsertItem( IMPICNVIEW_ACC_ESCAPE, vcl::KeyCode(KEY_ESCAPE) );

    aAccReturn.SetActivateHdl( LINK( this, IcnViewEdit_Impl, ReturnHdl_Impl) );
    aAccEscape.SetActivateHdl( LINK( this, IcnViewEdit_Impl, EscapeHdl_Impl) );
    Application::InsertAccel( &aAccReturn);//, ACCEL_ALWAYS );
    Application::InsertAccel( &aAccEscape);//, ACCEL_ALWAYS );
    Show();
    GrabFocus();
}

IcnViewEdit_Impl::~IcnViewEdit_Impl()
{
    disposeOnce();
}

void IcnViewEdit_Impl::dispose()
{
    if( !bAlreadyInCallback )
    {
        Application::RemoveAccel( &aAccReturn );
        Application::RemoveAccel( &aAccEscape );
    }
    MultiLineEdit::dispose();
}

void IcnViewEdit_Impl::CallCallBackHdl_Impl()
{
    maLoseFocusIdle.Stop();
    if ( !bAlreadyInCallback )
    {
        bAlreadyInCallback = true;
        Application::RemoveAccel( &aAccReturn );
        Application::RemoveAccel( &aAccEscape );
        Hide();
        aCallBackHdl.Call( nullptr );
    }
}

IMPL_LINK_NOARG(IcnViewEdit_Impl, Timeout_Impl, Timer *, void)
{
    CallCallBackHdl_Impl();
}

IMPL_LINK_NOARG( IcnViewEdit_Impl, ReturnHdl_Impl, Accelerator&, void )
{
    bCanceled = false;
    bGrabFocus = true;
    CallCallBackHdl_Impl();
}

IMPL_LINK_NOARG( IcnViewEdit_Impl, EscapeHdl_Impl, Accelerator&, void )
{
    bCanceled = true;
    bGrabFocus = true;
    CallCallBackHdl_Impl();
}

void IcnViewEdit_Impl::KeyInput( const KeyEvent& rKEvt )
{
    vcl::KeyCode aCode = rKEvt.GetKeyCode();
    sal_uInt16 nCode = aCode.GetCode();

    switch ( nCode )
    {
        case KEY_ESCAPE:
            bCanceled = true;
            bGrabFocus = true;
            CallCallBackHdl_Impl();
            break;

        case KEY_RETURN:
            bCanceled = false;
            bGrabFocus = true;
            CallCallBackHdl_Impl();
            break;

        default:
            MultiLineEdit::KeyInput( rKEvt );
    }
}

bool IcnViewEdit_Impl::PreNotify( NotifyEvent& rNEvt )
{
    if( rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
    {
        if ( !bAlreadyInCallback &&
            ((!Application::GetFocusWindow()) || !IsChild(Application::GetFocusWindow())))
        {
            bCanceled = false;
            maLoseFocusIdle.Start();
        }
    }
    return false;
}

void IcnViewEdit_Impl::StopEditing()
{
    if ( !bAlreadyInCallback )
    {
        bCanceled = true;
        CallCallBackHdl_Impl();
    }
}

sal_Int32 SvxIconChoiceCtrl_Impl::GetEntryListPos( SvxIconChoiceCtrlEntry* pEntry ) const
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
    if( nScrBarSize != nHorSBarHeight || nScrBarSize != nVerSBarWidth )
    {
        nHorSBarHeight = nScrBarSize;
        Size aSize( aHorSBar->GetSizePixel() );
        aSize.Height() = nScrBarSize;
        aHorSBar->Hide();
        aHorSBar->SetSizePixel( aSize );

        nVerSBarWidth = nScrBarSize;
        aSize = aVerSBar->GetSizePixel();
        aSize.Width() = nScrBarSize;
        aVerSBar->Hide();
        aVerSBar->SetSizePixel( aSize );

        Size aOSize( pView->Control::GetOutputSizePixel() );
        PositionScrollBars( aOSize.Width(), aOSize.Height() );
        AdjustScrollBars();
    }
}

EntryList_Impl::EntryList_Impl( SvxIconChoiceCtrl_Impl* pOwner ) :
    _pOwner( pOwner )
{
    _pOwner->pHead = nullptr;
}

EntryList_Impl::~EntryList_Impl()
{
    _pOwner->pHead = nullptr;
}

void EntryList_Impl::clear()
{
    _pOwner->pHead = nullptr;
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
            aAutoArrangeIdle.Start();
        return;
    }

    if( ePositionMode == IcnViewPositionModeAutoArrange )
    {
        for( size_t nCur = 0; nCur < nCount; nCur++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = aEntries[ nCur ];
            if( pEntry->GetFlags() & SvxIconViewFlags(SvxIconViewFlags::POS_LOCKED | SvxIconViewFlags::POS_MOVED))
                SetEntryPos(pEntry, GetEntryBoundRect( pEntry ).TopLeft());
        }

        if( aEntries.size() )
            aAutoArrangeIdle.Start();
    }
    else if( ePositionMode == IcnViewPositionModeAutoAdjust )
    {
        AdjustEntryAtGrid();
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
    pEntry->SetFlags( SvxIconViewFlags::PRED_SET );
    aAutoArrangeIdle.Start();
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
        return nullptr;
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
    OUString aEntryText( SvtIconChoiceCtrl::GetEntryText( pEntry, false ) );
    Rectangle aTextRect( CalcTextRect( pEntry, nullptr, false, &aEntryText ) );
    if ( ( !aTextRect.IsInside( aPos ) || aEntryText.isEmpty() ) && sQuickHelpText.isEmpty() )
        return false;

    Rectangle aOptTextRect( aTextRect );
    aOptTextRect.Bottom() = LONG_MAX;
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
        aPt.Y() -= 1;
        aPt.X() -= 3;
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
    if (!m_pColumns)
        return;

    m_pColumns->clear();
    DELETEZ(m_pColumns);
}

void SvxIconChoiceCtrl_Impl::SetColumn( sal_uInt16 nIndex, const SvxIconChoiceCtrlColumnInfo& rInfo)
{
    if (!m_pColumns)
        m_pColumns = new SvxIconChoiceCtrlColumnInfoMap;

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

void SvxIconChoiceCtrl_Impl::DrawHighlightFrame(vcl::RenderContext& rRenderContext, const Rectangle& rBmpRect)
{
    Rectangle aBmpRect(rBmpRect);
    long nBorder = 2;
    if (aImageSize.Width() < 32)
        nBorder = 1;
    aBmpRect.Right() += nBorder;
    aBmpRect.Left() -= nBorder;
    aBmpRect.Bottom() += nBorder;
    aBmpRect.Top() -= nBorder;

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
        Rectangle aInvalidationRect(GetEntryBoundRect(pCurHighlightFrame));
        aInvalidationRect.expand(5);
        pCurHighlightFrame = nullptr;
        pView->Invalidate(aInvalidationRect);
    }

    pCurHighlightFrame = pEntry;
    if (pEntry)
    {
        Rectangle aInvalidationRect(GetEntryBoundRect(pEntry));
        aInvalidationRect.expand(5);
        pView->Invalidate(aInvalidationRect);
    }
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
