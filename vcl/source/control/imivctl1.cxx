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
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <tools/poly.hxx>
#include <vcl/lineinfo.hxx>
#include <vcl/i18nhelp.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/settings.hxx>
#include <vcl/commandevent.hxx>

#include <vcl/toolkit/ivctrl.hxx>
#include "imivctl.hxx"

#include <algorithm>
#include <memory>
#include <vcl/idle.hxx>

constexpr auto DRAWTEXT_FLAGS_ICON =
    DrawTextFlags::Center | DrawTextFlags::Top | DrawTextFlags::EndEllipsis |
    DrawTextFlags::Clip | DrawTextFlags::MultiLine | DrawTextFlags::WordBreak | DrawTextFlags::Mnemonic;

#define DRAWTEXT_FLAGS_SMALLICON (DrawTextFlags::Left|DrawTextFlags::EndEllipsis|DrawTextFlags::Clip)

#define EVENTID_ADJUST_SCROLLBARS       (reinterpret_cast<void*>(1))

SvxIconChoiceCtrl_Impl::SvxIconChoiceCtrl_Impl(
    SvtIconChoiceCtrl* pCurView,
    WinBits nWinStyle
) :
    aVerSBar( VclPtr<ScrollBar>::Create(pCurView, WB_DRAG | WB_VSCROLL) ),
    aHorSBar( VclPtr<ScrollBar>::Create(pCurView, WB_DRAG | WB_HSCROLL) ),
    aScrBarBox( VclPtr<ScrollBarBox>::Create(pCurView) ),
    aDocRectChangedIdle( "svtools::SvxIconChoiceCtrl_Impl aDocRectChangedIdle" ),
    aVisRectChangedIdle( "svtools::SvxIconChoiceCtrl_Impl aVisRectChangedIdle" ),
    aImageSize( 32 * pCurView->GetDPIScaleFactor(), 32 * pCurView->GetDPIScaleFactor()),
    pView(pCurView), nMaxVirtHeight(DEFAULT_MAX_VIRT_HEIGHT),
    nFlags(IconChoiceFlags::NONE), nUserEventAdjustScrBars(nullptr),
    pCurHighlightFrame(nullptr),
    pCursor(nullptr)
{
    SetStyle( nWinStyle );
    pImpCursor.reset( new IcnCursor_Impl( this ) );
    pGridMap.reset( new IcnGridMap_Impl( this ) );

    aVerSBar->SetScrollHdl( LINK( this, SvxIconChoiceCtrl_Impl, ScrollUpDownHdl ) );
    aHorSBar->SetScrollHdl( LINK( this, SvxIconChoiceCtrl_Impl, ScrollLeftRightHdl ) );

    nHorSBarHeight = aHorSBar->GetSizePixel().Height();
    nVerSBarWidth = aVerSBar->GetSizePixel().Width();

    aDocRectChangedIdle.SetPriority( TaskPriority::HIGH_IDLE );
    aDocRectChangedIdle.SetInvokeHandler(LINK(this,SvxIconChoiceCtrl_Impl,DocRectChangedHdl));

    aVisRectChangedIdle.SetPriority( TaskPriority::HIGH_IDLE );
    aVisRectChangedIdle.SetInvokeHandler(LINK(this,SvxIconChoiceCtrl_Impl,VisRectChangedHdl));

    Clear( true );
    Size gridSize((nWinStyle & WB_DETAILS) ? 150 : 140, (nWinStyle & WB_DETAILS) ?  26 : 70);
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
    aVerSBar.disposeAndClear();
    aHorSBar.disposeAndClear();
    aScrBarBox.disposeAndClear();
}

void SvxIconChoiceCtrl_Impl::Clear( bool bInCtor )
{
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
        nMaxVirtHeight = aSize.Height() - nHorSBarHeight;
        if( nMaxVirtHeight <= 0 )
            nMaxVirtHeight = DEFAULT_MAX_VIRT_HEIGHT;
        maZOrderList.clear();
        SetOrigin( Point() );
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

    maZOrderList.push_back( pEntry );
    pImpCursor->Clear();

    // don't set all bounding rectangles to
    // 'to be checked', but only the bounding rectangle of the new entry.
    // Thus, don't call InvalidateBoundingRect!
    pEntry->aRect.SetRight( LONG_MAX );
    FindBoundingRect(pEntry);
    tools::Rectangle aOutputArea(GetOutputRect());
    pGridMap->OccupyGrids(pEntry);
    if (!aOutputArea.Overlaps(pEntry->aRect))
        return; // is invisible
    pView->Invalidate(pEntry->aRect);
}

void SvxIconChoiceCtrl_Impl::RemoveEntry(size_t nPos)
{
    pImpCursor->Clear();
    maEntries.erase(maEntries.begin() + nPos);
    RecalcAllBoundingRectsSmart();
}

tools::Rectangle SvxIconChoiceCtrl_Impl::GetOutputRect() const
{
    Point aOrigin( pView->GetMapMode().GetOrigin() );
    aOrigin *= -1;
    return tools::Rectangle( aOrigin, aOutputSize );
}

void SvxIconChoiceCtrl_Impl::SelectEntry( SvxIconChoiceCtrlEntry* pEntry, bool bSelect,
    bool bAdd )
{

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

    SvxIconViewFlags nEntryFlags = pEntry->GetFlags();
    if( bSelect )
    {
        nEntryFlags |= SvxIconViewFlags::SELECTED;
        pEntry->AssignFlags( nEntryFlags );
        pView->ClickIcon();
    }
    else
    {
        nEntryFlags &= ~SvxIconViewFlags::SELECTED;
        pEntry->AssignFlags( nEntryFlags );
        pView->ClickIcon();
    }
    EntrySelected( pEntry, bSelect );
}

void SvxIconChoiceCtrl_Impl::EntrySelected(SvxIconChoiceCtrlEntry* pEntry, bool bSelect)
{
    // make sure that the cursor is always placed
    // over the (only) selected entry. (But only if a cursor exists.)
    if (bSelect && pCursor && pEntry != pCursor)
    {
        SetCursor(pEntry);
    }

    ToTop(pEntry);

    if (pEntry == pCursor)
        ShowCursor(false);
    pView->Invalidate(CalcFocusRect(pEntry));
    if (pEntry == pCursor)
        ShowCursor(true);

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
        InvalidateBoundingRect(pCur->aRect);
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
                nMaxVirtHeight = aRealOutputSize.Height() - nHorSBarHeight;
        }
    }

    pImpCursor->Clear();
    pGridMap->Clear();
    VisRectChanged();
}

void SvxIconChoiceCtrl_Impl::AdjustVirtSize( const tools::Rectangle& rRect )
{
    tools::Long nHeightOffs = 0;
    tools::Long nWidthOffs = 0;

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

void SvxIconChoiceCtrl_Impl::Arrange(tools::Long nSetMaxVirtHeight)
{
    if ( nSetMaxVirtHeight != 0 )
        nMaxVirtHeight = nSetMaxVirtHeight;
    else
        nMaxVirtHeight = aOutputSize.Height();

    ImpArrange();
}

void SvxIconChoiceCtrl_Impl::ImpArrange()
{
    nFlags |= IconChoiceFlags::Arranging;
    ShowCursor( false );
    ResetVirtSize();
    bBoundRectsDirty = false;
    SetOrigin( Point() );
    VisRectChanged();
    RecalcAllBoundingRectsSmart();
    pView->Invalidate( InvalidateFlags::NoChildren );
    nFlags &= ~IconChoiceFlags::Arranging;
    ShowCursor( true );
}

void SvxIconChoiceCtrl_Impl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
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

    rRenderContext.Push(vcl::PushFlags::CLIPREGION);
    rRenderContext.SetClipRegion(vcl::Region(rRect));

    std::vector< SvxIconChoiceCtrlEntry* > aNewZOrderList;
    std::vector< SvxIconChoiceCtrlEntry* > aPaintedEntries;

    size_t nPos = 0;
    while(nCount)
    {
        SvxIconChoiceCtrlEntry* pEntry = maZOrderList[nPos];
        const tools::Rectangle& rBoundRect = GetEntryBoundRect(pEntry);
        if (rRect.Overlaps(rBoundRect))
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
    tools::Rectangle aOutRect(GetOutputRect());
    for (SvxIconChoiceCtrlEntry* pEntry : maZOrderList)
    {
        if (pEntry->GetFlags() & SvxIconViewFlags::SELECTED)
        {
            const tools::Rectangle& rBoundRect = GetEntryBoundRect(pEntry);
            if (aOutRect.Overlaps(rBoundRect))
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
    if( !(nWinBits & WB_NOPOINTERFOCUS) )
        pView->GrabFocus();

    Point aDocPos( rMEvt.GetPosPixel() );
    if(aDocPos.X()>=aOutputSize.Width() || aDocPos.Y()>=aOutputSize.Height())
        return false;
    ToDocPos( aDocPos );
    SvxIconChoiceCtrlEntry* pEntry = GetEntry( aDocPos );
    if( pEntry )
        MakeEntryVisible(pEntry);

    if( !pEntry )
        return false;

    if( rMEvt.GetClicks() == 2 )
    {
        DeselectAllBut( pEntry );
        SelectEntry( pEntry, true, false );
        pView->ClickIcon();
    }
    else
    {
        // Inplace-Editing ?
        if( rMEvt.IsMod2() )  // Alt?
        {
        }
        else
        {
            DeselectAllBut( pEntry );
            SetCursor( pEntry );
        }
    }
    return bHandled;
}

bool SvxIconChoiceCtrl_Impl::MouseMove( const MouseEvent& rMEvt )
{
    if( pView->IsTracking() )
        return false;

    SvxIconChoiceCtrlEntry* pEntry = nullptr;
    if (!rMEvt.IsLeaveWindow())
    {
        const Point aDocPos(pView->PixelToLogic(rMEvt.GetPosPixel()));
        pEntry = GetEntry(aDocPos);
    }
    SetEntryHighlightFrame(pEntry);
    return true;
}

void SvxIconChoiceCtrl_Impl::SetCursor_Impl(SvxIconChoiceCtrlEntry* pNewCursor)
{
    if( !pNewCursor )
        return;

    ShowCursor( false );
    MakeEntryVisible( pNewCursor );
    SetCursor( pNewCursor );

    SelectEntry( pCursor, true, false );
    CallEventListeners( VclEventId::ListboxSelect, pCursor );
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
            SetCursor_Impl(pNewCursor);
        return true;
    }

    if ( bMod2 )
        // no actions with <ALT>
        return false;

    bool bKeyUsed = true;

    SvxIconChoiceCtrlEntry* pNewCursor;

    sal_uInt16 nCode = rKEvt.GetKeyCode().GetCode();
    switch( nCode )
    {
        case KEY_UP:
        case KEY_PAGEUP:
            if( pCursor )
            {
                MakeEntryVisible( pCursor );
                if( nCode == KEY_UP || (rKEvt.GetKeyCode().IsMod1() && nCode == KEY_PAGEUP))
                    pNewCursor = pImpCursor->GoUpDown(pCursor,false);
                else
                    pNewCursor = pImpCursor->GoPageUpDown(pCursor,false);
                SetCursor_Impl(pNewCursor);
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
            }
            break;

        case KEY_DOWN:
        case KEY_PAGEDOWN:
            if( pCursor )
            {
                if( nCode == KEY_DOWN || (rKEvt.GetKeyCode().IsMod1() && nCode == KEY_PAGEDOWN) )
                    pNewCursor=pImpCursor->GoUpDown( pCursor,true );
                else
                    pNewCursor=pImpCursor->GoPageUpDown( pCursor,true );
                SetCursor_Impl(pNewCursor);
            }
            break;

        case KEY_RIGHT:
            if( pCursor )
            {
                pNewCursor=pImpCursor->GoLeftRight(pCursor,true );
                SetCursor_Impl(pNewCursor);
            }
            break;

        case KEY_LEFT:
            if( pCursor )
            {
                MakeEntryVisible( pCursor );
                pNewCursor = pImpCursor->GoLeftRight(pCursor,false );
                SetCursor_Impl(pNewCursor);
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
            break;
        case KEY_ADD:
        case KEY_DIVIDE :
        case KEY_A:
        case KEY_SUBTRACT:
        case KEY_COMMA :
        case KEY_RETURN:
            bKeyUsed = false;
            break;

        case KEY_END:
            if( pCursor )
            {
                pNewCursor = maEntries.back().get();
                SetCursor_Impl(pNewCursor);
            }
            break;

        case KEY_HOME:
            if( pCursor )
            {
                pNewCursor = maEntries[ 0 ].get();
                SetCursor_Impl(pNewCursor);
            }
            break;

        default:
            bKeyUsed = false;

    }
    return bKeyUsed;
}

// recalculate TopLeft of scrollbars (but not their sizes!)
void SvxIconChoiceCtrl_Impl::PositionScrollBars( tools::Long nRealWidth, tools::Long nRealHeight )
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
    tools::Long nVirtHeight = aVirtOutputSize.Height();
    tools::Long nVirtWidth = aVirtOutputSize.Width();

    Size aOSize( pView->GetOutputSizePixel() );
    tools::Long nRealHeight = aOSize.Height();
    tools::Long nRealWidth = aOSize.Width();

    PositionScrollBars( nRealWidth, nRealHeight );

    const MapMode& rMapMode = pView->GetMapMode();
    Point aOrigin( rMapMode.GetOrigin() );

    tools::Long nVisibleWidth;
    if( nRealWidth > nVirtWidth )
        nVisibleWidth = nVirtWidth + aOrigin.X();
    else
        nVisibleWidth = nRealWidth;

    tools::Long nVisibleHeight;
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
    tools::Long nThumb = aVerSBar->GetThumbPos();
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

    const Size aSize = pView->GetOutputSizePixel();
    PositionScrollBars( aSize.Width(), aSize.Height() );
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
        tools::Long nWidth = aOutputSize.Width();
        const size_t nCount = maZOrderList.size();
        tools::Long nMostRight = 0;
        for( size_t nCur = 0; nCur < nCount; nCur++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = maZOrderList[ nCur ];
            tools::Long nRight = GetEntryBoundRect(pEntry).Right();
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
        tools::Long nDeepest = 0;
        tools::Long nHeight = aOutputSize.Height();
        const size_t nCount = maZOrderList.size();
        for( size_t nCur = 0; nCur < nCount; nCur++ )
        {
            SvxIconChoiceCtrlEntry* pEntry = maZOrderList[ nCur ];
            tools::Long nBottom = GetEntryBoundRect(pEntry).Bottom();
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

// priorities of the emphasis:  bSelected
void SvxIconChoiceCtrl_Impl::PaintEmphasis(const tools::Rectangle& rTextRect,
                                           vcl::RenderContext& rRenderContext)
{
    rRenderContext.Push(vcl::PushFlags::FILLCOLOR);
    const Color& rFillColor = rRenderContext.GetFont().GetFillColor();
    rRenderContext.SetFillColor(rFillColor);
    // draw text rectangle
    if (rFillColor != COL_TRANSPARENT)
        rRenderContext.DrawRect(rTextRect);

    rRenderContext.Pop();
}


void SvxIconChoiceCtrl_Impl::PaintItem(const tools::Rectangle& rRect,
    IcnViewFieldType eItem, SvxIconChoiceCtrlEntry* pEntry, sal_uInt16 nPaintFlags,
    vcl::RenderContext& rRenderContext )
{
    if (eItem == IcnViewFieldType::Text)
    {
        if (nWinBits & WB_DETAILS)
        {
            // Vertically center text when the entry is text-only
            tools::Long nBoundingHeight(CalcBoundingHeight());
            tools::Long nStringHeight = GetItemSize(IcnViewFieldType::Text).Height();
            tools::Long nNewY = (nBoundingHeight - nStringHeight) / 2;
            Point aRectTL(rRect.TopLeft().getX(), rRect.TopLeft().getY() + nNewY);
            tools::Rectangle aTextRect(aRectTL, rRect.GetSize());
            rRenderContext.DrawText(aTextRect, pEntry->GetText(), nCurTextDrawFlags);
        }
        else
        {
            rRenderContext.DrawText(rRect, pEntry->GetText(), nCurTextDrawFlags);
        }
    }
    else
    {
        Point aPos(rRect.TopLeft());
        if (nPaintFlags & PAINTFLAG_HOR_CENTERED)
            aPos.AdjustX((rRect.GetWidth() - aImageSize.Width()) / 2 );
        if (nPaintFlags & PAINTFLAG_VER_CENTERED)
            aPos.AdjustY((rRect.GetHeight() - aImageSize.Height()) / 2 );
        rRenderContext.DrawImage(aPos, pEntry->GetImage());
    }
}

void SvxIconChoiceCtrl_Impl::PaintEntry(SvxIconChoiceCtrlEntry* pEntry, const Point& rPos, vcl::RenderContext& rRenderContext)
{
    rRenderContext.Push(vcl::PushFlags::FONT | vcl::PushFlags::TEXTCOLOR);

    tools::Rectangle aTextRect(CalcTextRect(pEntry, &rPos));
    tools::Rectangle aBmpRect(CalcBmpRect(pEntry, &rPos));

    const bool bMouseHovered = pEntry == pCurHighlightFrame;
    const bool bSelected = pEntry->IsSelected();

    const StyleSettings& rSettings = rRenderContext.GetSettings().GetStyleSettings();
    vcl::Font aNewFont(rRenderContext.GetFont());
    if (bSelected)
        aNewFont.SetColor(rSettings.GetTabHighlightTextColor());
    else if (bMouseHovered)
        aNewFont.SetColor(rSettings.GetTabRolloverTextColor());
    else
        aNewFont.SetColor(rSettings.GetTabTextColor());
    rRenderContext.SetFont(aNewFont);

    bool bResetClipRegion = false;
    if (!rRenderContext.IsClipRegion() && (aVerSBar->IsVisible() || aHorSBar->IsVisible()))
    {
        tools::Rectangle aOutputArea(GetOutputRect());
        if (aOutputArea.Overlaps(aTextRect) || aOutputArea.Overlaps(aBmpRect))
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

    tools::Rectangle aFocusRect(CalcFocusRect(pEntry));

    bool bNativeOK
        = rRenderContext.IsNativeControlSupported(ControlType::TabItem, ControlPart::Entire);
    if (bNativeOK)
    {
        ControlState nState = ControlState::ENABLED;
        if (bSelected)
            nState |= ControlState::SELECTED;
        if (pEntry->IsFocused())
            nState |= ControlState::FOCUSED;
        if (bMouseHovered)
            nState |= ControlState::ROLLOVER;

        TabitemValue tiValue(aFocusRect, TabBarPosition::Left);
        bNativeOK = rRenderContext.DrawNativeControl(ControlType::TabItem, ControlPart::Entire,
                                                     aFocusRect, nState, tiValue, OUString());
    }

    if (!bNativeOK)
    {
        if (bSelected)
            vcl::RenderTools::DrawSelectionBackground(
                rRenderContext, *pView, aFocusRect, pView->HasFocus() ? 1 : 2, false, false, false);
        else
            PaintEmphasis(aTextRect, rRenderContext);

        if (pEntry->IsFocused())
            DrawFocusRect(rRenderContext, pEntry);

        // highlight mouse-hovered entry
        if (bMouseHovered)
            DrawHighlightFrame(rRenderContext, aFocusRect);
    }

    PaintItem(aBmpRect, IcnViewFieldType::Image, pEntry, nBmpPaintFlags, rRenderContext);

    // Move text a bit to the right for native controls due to potential tab mark (applies to text-only entries)
    if (bNativeOK && (nWinBits & WB_DETAILS))
        aTextRect.SetPos(Point(aTextRect.GetPos().X() + TAB_MARK_WIDTH, aTextRect.GetPos().Y()));

    PaintItem(aTextRect, IcnViewFieldType::Text, pEntry, nTextPaintFlags, rRenderContext);

    rRenderContext.Pop();
    if (bResetClipRegion)
        rRenderContext.SetClipRegion();
}

SvxIconChoiceCtrlEntry* SvxIconChoiceCtrl_Impl::GetEntry( const Point& rDocPos )
{
    CheckBoundingRects();
    // search through z-order list from the end
    size_t nCount = maZOrderList.size();
    while( nCount )
    {
        nCount--;
        SvxIconChoiceCtrlEntry* pEntry = maZOrderList[ nCount ];
        tools::Rectangle aBoundingRect(GetEntryBoundRect(pEntry));
        if( aBoundingRect.Contains( rDocPos ) )
            return pEntry;
    }
    return nullptr;
}

void SvxIconChoiceCtrl_Impl::MakeEntryVisible(SvxIconChoiceCtrlEntry* pEntry)
{
    const tools::Rectangle& rRect = GetEntryBoundRect( pEntry );
    MakeVisible(rRect);
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

        case WB_DETAILS:
            return tools::Rectangle(aPos, Size(0,0));
        case WB_SMALLICON:
            aPos.AdjustY(( aBound.GetHeight() - aImageSize.Height() ) / 2 );
            //TODO: determine horizontal distance to bounding rectangle
            return tools::Rectangle( aPos, aImageSize );

        default:
            assert(false && "IconView: Viewmode not set");
            return aBound;
    }
}

tools::Rectangle SvxIconChoiceCtrl_Impl::CalcTextRect( SvxIconChoiceCtrlEntry* pEntry,
    const Point* pEntryPos)
{
    const tools::Rectangle aMaxTextRect( CalcMaxTextRect( pEntry ) );
    tools::Rectangle aBound( GetEntryBoundRect( pEntry ) );
    if( pEntryPos )
        aBound.SetPos( *pEntryPos );

    const OUString aEntryText = pEntry->GetText();
    tools::Rectangle aTextRect = pView->GetTextRect( aMaxTextRect, aEntryText, nCurTextDrawFlags );

    Size aTextSize( aTextRect.GetSize() );

    Point aPos( aBound.TopLeft() );
    tools::Long nBoundWidth = aBound.GetWidth();
    tools::Long nBoundHeight = aBound.GetHeight();

    switch( nWinBits & VIEWMODE_MASK )
    {
        case WB_ICON:
            aPos.AdjustY(aImageSize.Height() );
            aPos.AdjustY(VER_DIST_BMP_STRING );
            aPos.AdjustX((nBoundWidth - aTextSize.Width()) / 2 );
            break;

        case WB_DETAILS:
            break;

        case WB_SMALLICON:
            aPos.AdjustX(aImageSize.Width() );
            aPos.AdjustX(HOR_DIST_BMP_STRING );
            aPos.AdjustY((nBoundHeight - aTextSize.Height()) / 2 );
            break;
    }
    return tools::Rectangle( aPos, aTextSize );
}


tools::Long SvxIconChoiceCtrl_Impl::CalcBoundingWidth() const
{
    tools::Long nStringWidth = GetItemSize( IcnViewFieldType::Text ).Width();
    tools::Long nWidth = 0;

    switch( nWinBits & VIEWMODE_MASK )
    {
        case WB_ICON:
            nWidth = std::max( nStringWidth, aImageSize.Width() );
            break;

        case WB_DETAILS:
            nWidth = nStringWidth;
            break;

        case WB_SMALLICON:
            nWidth = aImageSize.Width();
            nWidth += HOR_DIST_BMP_STRING;
            nWidth += nStringWidth;
            break;
    }
    return nWidth;
}

tools::Long SvxIconChoiceCtrl_Impl::CalcBoundingHeight() const
{
    tools::Long nStringHeight = GetItemSize(IcnViewFieldType::Text).Height();
    tools::Long nHeight = 0;

    switch( nWinBits & VIEWMODE_MASK )
    {
        case WB_ICON:
            nHeight = aImageSize.Height();
            nHeight += VER_DIST_BMP_STRING;
            nHeight += nStringHeight;
            break;

        case WB_DETAILS:
            nHeight = nStringHeight + 2 * VERT_TEXT_PADDING;;
            break;

        case WB_SMALLICON:
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
    AdjustScrollBars();
}

void SvxIconChoiceCtrl_Impl::FindBoundingRect( SvxIconChoiceCtrlEntry* pEntry )
{
    CalcBoundingSize();
    Point aPos(pGridMap->GetGridRect(pGridMap->GetUnoccupiedGrid()).TopLeft());

    tools::Rectangle aGridRect(aPos, Size(nGridDX, nGridDY));
    pEntry->aRect = aGridRect;
    AdjustVirtSize( pEntry->aRect );
    pGridMap->OccupyGrids( pEntry );
}


void SvxIconChoiceCtrl_Impl::SetCursor( SvxIconChoiceCtrlEntry* pEntry )
{
    if( pEntry == pCursor )
    {
        if (pCursor && !pCursor->IsSelected())
            SelectEntry( pCursor, true );
        return;
    }
    ShowCursor( false );
    SvxIconChoiceCtrlEntry* pOldCursor = pCursor;
    pCursor = pEntry;
    if( pOldCursor )
    {
        pOldCursor->ClearFlags( SvxIconViewFlags::FOCUSED );
        SelectEntry(pOldCursor, false); // deselect old cursor
    }
    if( pCursor )
    {
        ToTop( pCursor );
        pCursor->SetFlags( SvxIconViewFlags::FOCUSED );
        SelectEntry(pCursor, true);
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

bool SvxIconChoiceCtrl_Impl::HandleScrollCommand( const CommandEvent& rCmd )
{
    tools::Rectangle aDocRect( Point(), aVirtOutputSize );
    tools::Rectangle aVisRect( GetOutputRect() );
    if( aVisRect.Contains( aDocRect ))
        return false;
    Size aDocSize( aDocRect.GetSize() );
    Size aVisSize( aVisRect.GetSize() );
    bool bHor = aDocSize.Width() > aVisSize.Width();
    bool bVer = aDocSize.Height() > aVisSize.Height();

    tools::Long nScrollDX = 0, nScrollDY = 0;

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
                double nScrollLines = pData->GetScrollLines();
                if( nScrollLines == COMMAND_WHEEL_PAGESCROLL )
                {
                    nScrollDY = GetScrollBarPageSize( aVisSize.Width() );
                    if( pData->GetDelta() < 0 )
                        nScrollDY *= -1;
                }
                else
                {
                    nScrollDY = pData->GetNotchDelta() * static_cast<tools::Long>(nScrollLines);
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
    if( aOutputArea.Contains( aVirtRect ) )
        return; // is already visible

    tools::Long nDy;
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

    tools::Long nDx = 0;

    // no horizontal scrolling needed in list mode
    if (!(nWinBits & WB_DETAILS))
    {
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
    }

    aOrigin.AdjustX(nDx );
    aOrigin.AdjustY(nDy );
    aOutputArea.SetPos( aOrigin );

    pView->PaintImmediately();
    ShowCursor(false);

    // invert origin for SV (so we can scroll/paint using document coordinates)
    aOrigin *= -1;
    SetOrigin( aOrigin );

    bool bScrollable = pView->GetBackground().IsScrollable();

    if (bScrollable)
    {
        // scroll in reverse direction!
        pView->Scroll( -nDx, -nDy, aOutputArea,
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

    ShowCursor(true);

    // check if we still need scrollbars
    CheckScrollBars();
    if (bScrollable)
        pView->PaintImmediately();

    // If the requested area can not be made completely visible, the
    // Vis-Rect-Changed handler is called in any case. This case may occur e.g.
    // if only few pixels of the lower border are invisible, but a scrollbar has
    // a larger line size.
    VisRectChanged();
}

void SvxIconChoiceCtrl_Impl::DeselectAllBut( SvxIconChoiceCtrlEntry const * pThisEntryNot )
{
    // TODO: work through z-order list, if necessary!

    size_t nCount = maEntries.size();
    for( size_t nCur = 0; nCur < nCount; nCur++ )
    {
        SvxIconChoiceCtrlEntry* pEntry = maEntries[ nCur ].get();
        if( pEntry != pThisEntryNot && pEntry->IsSelected() )
            SelectEntry( pEntry, false, true );
    }
    nFlags &= ~IconChoiceFlags::AddMode;
}

Size SvxIconChoiceCtrl_Impl::GetMinGrid() const
{
    Size aTextSize( pView->GetTextWidth( u"XXX"_ustr ), pView->GetTextHeight() );
    if (nWinBits & WB_DETAILS)
        return Size(aTextSize.Width(), aTextSize.Height());

    Size aMinSize( aImageSize );
    aMinSize.AdjustWidth(2 * LROFFS_BOUND );
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
    nGridDY = aSize.Height();
    SetDefaultTextSize();
}

// Calculates the maximum size that the text rectangle may use within its
// bounding rectangle.

tools::Rectangle SvxIconChoiceCtrl_Impl::CalcMaxTextRect( const SvxIconChoiceCtrlEntry* pEntry ) const
{
    assert(IsBoundingRectValid(pEntry->aRect) && "Bounding rect for entry hasn't been calculated yet.");
    tools::Rectangle aBoundRect = pEntry->aRect;

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
    }
    else if (nWinBits & WB_SMALLICON)
    {
        aBoundRect.SetLeft( aBmpRect.Right() );
        aBoundRect.AdjustLeft(HOR_DIST_BMP_STRING );
        aBoundRect.AdjustRight( -(LROFFS_BOUND) );
        if( aBoundRect.Left() > aBoundRect.Right() )
            aBoundRect.SetLeft( aBoundRect.Right() );
        tools::Long nHeight = aBoundRect.GetSize().Height();
        nHeight = nHeight - aDefaultTextSize.Height();
        nHeight /= 2;
        aBoundRect.AdjustTop(nHeight );
        aBoundRect.AdjustBottom( -nHeight );
    }
    return aBoundRect;
}

void SvxIconChoiceCtrl_Impl::SetDefaultTextSize()
{
    tools::Long nDY = nGridDY;
    nDY -= aImageSize.Height();
    nDY -= VER_DIST_BMP_STRING;
    if (nDY <= 0)
        nDY = 2;

    tools::Long nDX = nGridDX;
    nDX -= 2 * LROFFS_BOUND;
    nDX -= 2;
    if (nDX <= 0)
        nDX = 2;

    tools::Long nHeight = pView->GetTextHeight();
    if (nDY < nHeight)
        nDY = nHeight;
    if(pView->GetDPIScaleFactor() > 1)
    {
      nDY*=2;
    }
    aDefaultTextSize = Size(nDX, nDY);
}

// The deltas are the offsets by which the view is moved on the document.
// left, up: offsets < 0
// right, down: offsets > 0
void SvxIconChoiceCtrl_Impl::Scroll( tools::Long nDeltaX, tools::Long nDeltaY )
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
    tools::Rectangle aBoundRect( GetEntryBoundRect( pEntry ) );

    // Remove left margin
    if (nWinBits & WB_DETAILS)
        aBoundRect.SetPos(Point(0, aBoundRect.GetPos().Y()));

    return aBoundRect;
}

IMPL_LINK_NOARG(SvxIconChoiceCtrl_Impl, VisRectChangedHdl, Timer *, void)
{
    aVisRectChangedIdle.Stop();
}

IMPL_LINK_NOARG(SvxIconChoiceCtrl_Impl, DocRectChangedHdl, Timer *, void)
{
    aDocRectChangedIdle.Stop();
}

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

void SvxIconChoiceCtrl_Impl::DrawFocusRect(vcl::RenderContext& rRenderContext,  SvxIconChoiceCtrlEntry* pEntry)
{
    tools::Rectangle aRect (CalcFocusRect(pEntry));
    ShowFocus(aRect);

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
}

void SvxIconChoiceCtrl_Impl::CancelUserEvents()
{
    if( nUserEventAdjustScrBars )
    {
        Application::RemoveUserEvent( nUserEventAdjustScrBars );
        nUserEventAdjustScrBars = nullptr;
    }
}

SvxIconChoiceCtrlEntry* SvxIconChoiceCtrl_Impl::GetFirstSelectedEntry() const
{
    size_t nCount = maEntries.size();
    for( size_t nCur = 0; nCur < nCount; nCur++ )
    {
        SvxIconChoiceCtrlEntry* pEntry = maEntries[ nCur ].get();
        if( pEntry->IsSelected() )
        {
            return pEntry;
        }
    }
    return nullptr;
}

sal_Int32 SvxIconChoiceCtrl_Impl::GetEntryListPos( SvxIconChoiceCtrlEntry const * pEntry ) const
{
    auto it = std::find_if(maEntries.begin(), maEntries.end(),
                           [pEntry](auto& rIt) { return rIt.get() == pEntry; });
    if (it != maEntries.end())
        return std::distance(maEntries.begin(), it);

    return -1;
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

    tools::Long nScrBarSize = rStyleSettings.GetScrollBarSize();
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

    Size aOSize(pView->GetOutputSizePixel());
    PositionScrollBars( aOSize.Width(), aOSize.Height() );
    AdjustScrollBars();
}

bool SvxIconChoiceCtrl_Impl::RequestHelp( const HelpEvent& rHEvt )
{
    if ( !(rHEvt.GetMode() & HelpEventMode::QUICK ) )
        return false;

    Point aPos( pView->ScreenToOutputPixel(rHEvt.GetMousePosPixel() ) );
    aPos -= pView->GetMapMode().GetOrigin();
    SvxIconChoiceCtrlEntry* pEntry = GetEntry( aPos );

    if ( !pEntry )
        return false;

    OUString sQuickHelpText = pEntry->GetQuickHelpText();
    tools::Rectangle aTextRect(CalcTextRect(pEntry, nullptr));
    const OUString aEntryText = pEntry->GetText();
    if ( ( !aTextRect.Contains( aPos ) || aEntryText.isEmpty() ) && sQuickHelpText.isEmpty() )
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
        Help::ShowQuickHelp(pView, aOptTextRect, sHelpText, QuickHelpFlags::Left | QuickHelpFlags::VCenter);
    }

    return true;
}

void SvxIconChoiceCtrl_Impl::DrawHighlightFrame(vcl::RenderContext& rRenderContext,
                                                const tools::Rectangle& rRect)
{
    DecorationView aDecoView(&rRenderContext);
    aDecoView.DrawHighlightFrame(rRect);
}

void SvxIconChoiceCtrl_Impl::SetEntryHighlightFrame(SvxIconChoiceCtrlEntry* pEntry)
{
    if( pEntry == pCurHighlightFrame )
        return;

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
