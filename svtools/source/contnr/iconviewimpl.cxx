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

#include <vcl/svapp.hxx>
#include <svtools/iconview.hxx>
#include <iconviewimpl.hxx>
#include <tools/debug.hxx>

IconViewImpl::IconViewImpl( SvTreeListBox* pTreeListBox, SvTreeList* pTreeList, WinBits nWinStyle )
: SvImpLBox( pTreeListBox, pTreeList, nWinStyle )
{
}

void IconViewImpl::CursorUp()
{
    if (!pStartEntry)
        return;

    SvTreeListEntry* pPrevFirstToDraw = pStartEntry;

    for(short i = 0; i < pView->GetColumnsCount() && pPrevFirstToDraw; i++)
        pPrevFirstToDraw = pView->PrevVisible(pPrevFirstToDraw);

    if( !pPrevFirstToDraw )
        return;

    nFlags &= ~LBoxFlags::Filling;
    long nEntryHeight = pView->GetEntryHeight();
    ShowCursor( false );
    pView->Update();
    pStartEntry = pPrevFirstToDraw;
    tools::Rectangle aArea( GetVisibleArea() );
    aArea.AdjustBottom( -nEntryHeight );
    pView->Scroll( 0, nEntryHeight, aArea, ScrollFlags::NoChildren );
    pView->Update();
    ShowCursor( true );
    pView->NotifyScrolled();
}

void IconViewImpl::CursorDown()
{
    if (!pStartEntry)
        return;

    SvTreeListEntry* pNextFirstToDraw = pStartEntry;

    for(short i = 0; i < pView->GetColumnsCount(); i++)
        pNextFirstToDraw = pView->NextVisible(pNextFirstToDraw);

    if( pNextFirstToDraw )
    {
        nFlags &= ~LBoxFlags::Filling;
        ShowCursor( false );
        pView->Update();
        pStartEntry = pNextFirstToDraw;
        tools::Rectangle aArea( GetVisibleArea() );
        pView->Scroll( 0, -(pView->GetEntryHeight()), aArea, ScrollFlags::NoChildren );
        pView->Update();
        ShowCursor( true );
        pView->NotifyScrolled();
    }
}

void IconViewImpl::PageDown( sal_uInt16 nDelta )
{
    sal_uInt16 nRealDelta = nDelta * pView->GetColumnsCount();

    if( !nDelta )
        return;

    if (!pStartEntry)
        return;

    SvTreeListEntry* pNext = pView->NextVisible(pStartEntry, nRealDelta);
    if( pNext == pStartEntry )
        return;

    ShowCursor( false );

    nFlags &= ~LBoxFlags::Filling;
    pView->Update();
    pStartEntry = pNext;

    if( nRealDelta >= nVisibleCount )
    {
        pView->Invalidate( GetVisibleArea() );
        pView->Update();
    }
    else
    {
        tools::Rectangle aArea( GetVisibleArea() );
        long nScroll = pView->GetEntryHeight() * static_cast<long>(nRealDelta);
        nScroll = -nScroll;
        pView->Update();
        pView->Scroll( 0, nScroll, aArea, ScrollFlags::NoChildren );
        pView->Update();
        pView->NotifyScrolled();
    }

    ShowCursor( true );
}

void IconViewImpl::PageUp( sal_uInt16 nDelta )
{
    sal_uInt16 nRealDelta = nDelta * pView->GetColumnsCount();
    if( !nDelta )
        return;

    if (!pStartEntry)
        return;

    SvTreeListEntry* pPrev = pView->PrevVisible(pStartEntry, nRealDelta);
    if( pPrev == pStartEntry )
        return;

    nFlags &= ~LBoxFlags::Filling;
    ShowCursor( false );

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
        tools::Rectangle aArea( GetVisibleArea() );
        pView->Update();
        pView->Scroll( 0, nEntryHeight*nRealDelta, aArea, ScrollFlags::NoChildren );
        pView->Update();
        pView->NotifyScrolled();
    }

    ShowCursor( true );
}

void IconViewImpl::KeyDown( bool bPageDown )
{
    if( !aVerSBar->IsVisible() )
        return;

    long nDelta;
    if( bPageDown )
        nDelta = aVerSBar->GetPageSize();
    else
        nDelta = 1;

    long nThumbPos = aVerSBar->GetThumbPos();

    if( nDelta <= 0 )
        return;

    nFlags &= ~LBoxFlags::Filling;
    BeginScroll();

    aVerSBar->SetThumbPos( nThumbPos+nDelta );
    if( bPageDown )
        PageDown( static_cast<short>(nDelta) );
    else
        CursorDown();

    EndScroll();
}

void IconViewImpl::KeyUp( bool bPageUp )
{
    if( !aVerSBar->IsVisible() )
        return;

    long nDelta;
    if( bPageUp )
        nDelta = aVerSBar->GetPageSize();
    else
        nDelta = 1;

    long nThumbPos = aVerSBar->GetThumbPos();

    if( nThumbPos < nDelta )
        nDelta = nThumbPos;

    if( nDelta < 0 )
        return;

    nFlags &= ~LBoxFlags::Filling;
    BeginScroll();

    aVerSBar->SetThumbPos( nThumbPos - nDelta );
    if( bPageUp )
        PageUp( static_cast<short>(nDelta) );
    else
        CursorUp();

    EndScroll();
}

long IconViewImpl::GetEntryLine( SvTreeListEntry* pEntry ) const
{
    if(!pStartEntry )
        return -1; // invisible position

    long nFirstVisPos = pView->GetVisiblePos( pStartEntry );
    long nEntryVisPos = pView->GetVisiblePos( pEntry );
    nFirstVisPos = nEntryVisPos - nFirstVisPos;

    return nFirstVisPos;
}

Point IconViewImpl::GetEntryPosition( SvTreeListEntry* pEntry ) const
{
    const int pos = pView->GetAbsPos( pEntry );

    return Point( ( pos % pView->GetColumnsCount() ) * pView->GetEntryWidth(),
                 ( pos / pView->GetColumnsCount() ) * pView->GetEntryHeight() );
}

SvTreeListEntry* IconViewImpl::GetClickedEntry( const Point& rPoint ) const
{
    DBG_ASSERT( pView->GetModel(), "IconViewImpl::GetClickedEntry: how can this ever happen?" );
    if ( !pView->GetModel() )
        return nullptr;
    if( pView->GetEntryCount() == 0 || !pStartEntry || !pView->GetEntryHeight() || !pView->GetEntryWidth())
        return nullptr;

    sal_uInt16 nY = static_cast<sal_uInt16>(rPoint.Y() / pView->GetEntryHeight() );
    sal_uInt16 nX = static_cast<sal_uInt16>(rPoint.X() / pView->GetEntryWidth() );
    sal_uInt16 nTemp = nY * pView->GetColumnsCount() + nX;

    SvTreeListEntry* pEntry = pView->NextVisible(pStartEntry, nTemp);

    return pEntry;
}

bool IconViewImpl::IsEntryInView( SvTreeListEntry* pEntry ) const
{
    // parent collapsed
    if( !pView->IsEntryVisible(pEntry) )
        return false;

    long nY = GetEntryLine( pEntry ) / pView->GetColumnsCount() * pView->GetEntryHeight();
    if( nY < 0 )
        return false;

    long nMax = nVisibleCount / pView->GetColumnsCount() * pView->GetEntryHeight();
    if( nY >= nMax )
        return false;

    long nStart = GetEntryLine( pEntry ) - GetEntryLine( pStartEntry );
    return nStart >= 0;
}

void IconViewImpl::AdjustScrollBars( Size& rSize )
{
    long nEntryHeight = pView->GetEntryHeight();
    if( !nEntryHeight )
        return;

    sal_uInt16 nResult = 0;

    Size aOSize( pView->Control::GetOutputSizePixel() );

    const WinBits nWindowStyle = pView->GetStyle();
    bool bVerSBar = ( nWindowStyle & WB_VSCROLL ) != 0;

    // number of entries that are not collapsed
    sal_uLong nTotalCount = pView->GetVisibleCount();

    // number of entries visible within the view
    nVisibleCount = aOSize.Height() / nEntryHeight * pView->GetColumnsCount();

    long nRows = ( nTotalCount / pView->GetColumnsCount() ) + 1;

    // do we need a vertical scrollbar?
    if( bVerSBar || nTotalCount > nVisibleCount )
    {
        nResult = 1;
    }

    PositionScrollBars( aOSize, nResult );

    // adapt Range, VisibleRange etc.

    // refresh output size, in case we have to scroll
    tools::Rectangle aRect;
    aRect.SetSize( aOSize );
    aSelEng.SetVisibleArea( aRect );

    // vertical scrollbar
    if( !bInVScrollHdl )
    {
        aVerSBar->SetPageSize( nTotalCount );
        aVerSBar->SetVisibleSize( nTotalCount - nRows );
    }
    else
    {
        nFlags |= LBoxFlags::EndScrollSetVisSize;
    }

    if( nResult & 0x0001 )
        aVerSBar->Show();
    else
        aVerSBar->Hide();

    rSize = aOSize;
}

// returns 0 if position is just past the last entry
SvTreeListEntry* IconViewImpl::GetEntry( const Point& rPoint ) const
{
    if( (pView->GetEntryCount() == 0) || !pStartEntry ||
        (rPoint.Y() > aOutputSize.Height())
        || !pView->GetEntryHeight()
        || !pView->GetEntryWidth())
        return nullptr;

    sal_uInt16 nClickedEntry = static_cast<sal_uInt16>(rPoint.Y() / pView->GetEntryHeight() * pView->GetColumnsCount() + rPoint.X() / pView->GetEntryWidth() );
    sal_uInt16 nTemp = nClickedEntry;
    SvTreeListEntry* pEntry = pView->NextVisible(pStartEntry, nTemp);
    if( nTemp != nClickedEntry )
        pEntry = nullptr;
    return pEntry;
}

void IconViewImpl::SyncVerThumb()
{
    if( pStartEntry )
    {
        long nEntryPos = pView->GetVisiblePos( pStartEntry );
        aVerSBar->SetThumbPos( nEntryPos );
    }
    else
        aVerSBar->SetThumbPos( 0 );
}

void IconViewImpl::UpdateAll( bool bInvalidateCompleteView )
{
    FindMostRight( nullptr );
    aVerSBar->SetRange( Range( 0, pView->GetVisibleCount() ) );
    SyncVerThumb();
    FillView();
    ShowVerSBar();
    if( bSimpleTravel && pCursor && pView->HasFocus() )
        pView->Select( pCursor );
    ShowCursor( true );
    if( bInvalidateCompleteView )
        pView->Invalidate();
    else
        pView->Invalidate( GetVisibleArea() );
}

void IconViewImpl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
{
    if (!pView->GetVisibleCount())
        return;

    nFlags |= LBoxFlags::InPaint;

    if (nFlags & LBoxFlags::Filling)
    {
        SvTreeListEntry* pFirst = pView->First();
        if (pFirst != pStartEntry)
        {
            ShowCursor(false);
            pStartEntry = pView->First();
            aVerSBar->SetThumbPos( 0 );
            StopUserEvent();
            ShowCursor(true);
            nCurUserEvent = Application::PostUserEvent(LINK(this, SvImpLBox, MyUserEvent),
                                                       reinterpret_cast<void*>(1));
            return;
        }
    }

    if (!pStartEntry)
    {
        pStartEntry = pView->First();
    }

    long nRectHeight = rRect.GetHeight();
    long nRectWidth = rRect.GetWidth();
    long nEntryHeight = pView->GetEntryHeight();
    long nEntryWidth = pView->GetEntryWidth();

    // calculate area for the entries we want to draw
    sal_uInt16 nStartId = static_cast<sal_uInt16>(rRect.Top() / nEntryHeight * pView->GetColumnsCount() + (rRect.Left() / nEntryWidth));
    sal_uInt16 nCount = static_cast<sal_uInt16>(( nRectHeight / nEntryHeight + 1 ) * nRectWidth / nEntryWidth);
    nCount += 2; // don't miss an entry

    long nY = nStartId / pView->GetColumnsCount() * nEntryHeight;
    long nX = 0;
    SvTreeListEntry* pEntry = pStartEntry;
    while (nStartId && pEntry)
    {
        pEntry = pView->NextVisible(pEntry);
        nStartId--;
    }

    vcl::Region aClipRegion(GetClipRegionRect());

    if (!pCursor && !mbNoAutoCurEntry)
    {
        // do not select if multiselection or explicit set
        bool bNotSelect = (aSelEng.GetSelectionMode() == SelectionMode::Multiple ) || ((m_nStyle & WB_NOINITIALSELECTION) == WB_NOINITIALSELECTION);
        SetCursor(pStartEntry, bNotSelect);
    }

    for(sal_uInt16 n = 0; n< nCount && pEntry; n++)
    {
        static_cast<IconView*>(pView.get())->PaintEntry(*pEntry, nX, nY, rRenderContext);
        nX += nEntryWidth;

        if(nX + pView->GetEntryWidth() > nEntryWidth * pView->GetColumnsCount())
        {
            nY += nEntryHeight;
            nX = 0;
        }
        pEntry = pView->NextVisible(pEntry);
    }

    nFlags &= ~LBoxFlags::DeselectAll;
    rRenderContext.SetClipRegion();
    nFlags &= ~LBoxFlags::InPaint;
}

void IconViewImpl::InvalidateEntry( long nId ) const
{
    if( nFlags & LBoxFlags::InPaint )
        return;

    tools::Rectangle aRect( GetVisibleArea() );
    long nMaxBottom = aRect.Bottom();
    aRect.SetTop( nId / pView->GetColumnsCount() * pView->GetEntryHeight() );
    aRect.SetBottom( aRect.Top() ); aRect.AdjustBottom(pView->GetEntryHeight() );

    if( aRect.Top() > nMaxBottom )
        return;
    if( aRect.Bottom() > nMaxBottom )
        aRect.SetBottom( nMaxBottom );
    pView->Invalidate( aRect );
}

bool IconViewImpl::KeyInput( const KeyEvent& rKEvt )
{
    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();

    if( rKeyCode.IsMod2() )
        return false; // don't evaluate Alt key

    nFlags &= ~LBoxFlags::Filling;

    if( !pCursor )
        pCursor = pStartEntry;
    if( !pCursor )
        return false;

    sal_uInt16  aCode = rKeyCode.GetCode();

    bool    bShift = rKeyCode.IsShift();
    bool    bMod1 = rKeyCode.IsMod1();

    SvTreeListEntry* pNewCursor;

    bool bHandled = true;

    long i;
    long nColumns = pView->GetColumnsCount();

    switch( aCode )
    {
        case KEY_LEFT:
            if( !IsEntryInView( pCursor ) )
                MakeVisible( pCursor );

            pNewCursor = pCursor;
            do
            {
                pNewCursor = pView->PrevVisible(pNewCursor);
            } while( pNewCursor && !IsSelectable(pNewCursor) );

            // if there is no next entry, take the current one
            // this ensures that in case of _one_ entry in the list, this entry is selected when pressing
            // the cursor key
            if (!pNewCursor)
                pNewCursor = pCursor;

            aSelEng.CursorPosChanging( bShift, bMod1 );
            SetCursor( pNewCursor, bMod1 );     // no selection, when Ctrl is on
            if( !IsEntryInView( pNewCursor ) )
                KeyUp( false );
            break;

        case KEY_RIGHT:
            if( !IsEntryInView( pCursor ) )
                MakeVisible( pCursor );

            pNewCursor = pCursor;
            do
            {
                pNewCursor = pView->NextVisible(pNewCursor);
            } while( pNewCursor && !IsSelectable(pNewCursor) );

            // if there is no next entry, take the current one
            // this ensures that in case of _one_ entry in the list, this entry is selected when pressing
            // the cursor key
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
                        pView->Select( pCursor, false );
                    KeyDown( false );
                    SetCursor( pNewCursor, bMod1 ); // no selection, when Ctrl is on
                }
            }
            else
                KeyDown( false );   // because scrollbar range might still
                                        // allow scrolling
            break;

        case KEY_UP:
        {
            if( !IsEntryInView( pCursor ) )
                MakeVisible( pCursor );

            pNewCursor = pCursor;
            for( i = 0; i < nColumns && pNewCursor; i++)
            {
                do
                {
                    pNewCursor = pView->PrevVisible(pNewCursor);
                } while( pNewCursor && !IsSelectable(pNewCursor) );
            }

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
                    KeyUp( false );
            }
            break;
        }

        case KEY_DOWN:
        {
            if( !IsEntryInView( pCursor ) )
                MakeVisible( pCursor );

            pNewCursor = pCursor;
            for( i = 0; i < nColumns && pNewCursor; i++)
            {
                do
                {
                    pNewCursor = pView->NextVisible(pNewCursor);
                } while( pNewCursor && !IsSelectable(pNewCursor) );
            }

            // if there is no next entry, take the current one
            // this ensures that in case of _one_ entry in the list, this entry is selected when pressing
            // the cursor key
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
                        pView->Select( pCursor, false );
                    KeyDown( false );
                    SetCursor( pNewCursor, bMod1 ); // no selection, when Ctrl is on
                }
            }
            else
                KeyDown( false );   // because scrollbar range might still
                                        // allow scrolling
            break;
        }

        case KEY_RETURN:
        {
            pView->aDoubleClickHdl.Call( pView );
            bHandled = true;

            break;
        }

        case KEY_END:
        {
            pNewCursor = pView->GetModel()->Last();

            while( pNewCursor && !IsSelectable(pNewCursor) )
            {
                pNewCursor = pView->PrevVisible(pNewCursor);
            }

            pStartEntry = pNewCursor;

            while( pStartEntry && pView->GetAbsPos( pStartEntry ) % pView->GetColumnsCount() != 0 )
            {
                pStartEntry = pView->PrevVisible(pStartEntry);
            }

            if( pNewCursor && pNewCursor != pCursor)
            {
//              SelAllDestrAnch( false );
                aSelEng.CursorPosChanging( bShift, bMod1 );
                SetCursor( pNewCursor );
                SyncVerThumb();
            }

            bHandled = true;

            break;
        }

        default:
        {
            bHandled = false;
            break;
        }
    }

    if(!bHandled)
        return SvImpLBox::KeyInput( rKEvt );

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
