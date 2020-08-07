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
#include <tools/debug.hxx>
#include <iconview.hxx>
#include "iconviewimpl.hxx"

IconViewImpl::IconViewImpl( SvTreeListBox* pTreeListBox, SvTreeList* pTreeList, WinBits nWinStyle )
: SvImpLBox( pTreeListBox, pTreeList, nWinStyle )
{
}

void IconViewImpl::CursorUp()
{
    if (!m_pStartEntry)
        return;

    SvTreeListEntry* pPrevFirstToDraw = m_pStartEntry;

    for(short i = 0; i < m_pView->GetColumnsCount() && pPrevFirstToDraw; i++)
        pPrevFirstToDraw = m_pView->PrevVisible(pPrevFirstToDraw);

    if( !pPrevFirstToDraw )
        return;

    m_nFlags &= ~LBoxFlags::Filling;
    long nEntryHeight = m_pView->GetEntryHeight();
    ShowCursor( false );
    m_pView->PaintImmediately();
    m_pStartEntry = pPrevFirstToDraw;
    tools::Rectangle aArea( GetVisibleArea() );
    aArea.AdjustBottom( -nEntryHeight );
    m_pView->Scroll( 0, nEntryHeight, aArea, ScrollFlags::NoChildren );
    m_pView->PaintImmediately();
    ShowCursor( true );
    m_pView->NotifyScrolled();
}

void IconViewImpl::CursorDown()
{
    if (!m_pStartEntry)
        return;

    SvTreeListEntry* pNextFirstToDraw = m_pStartEntry;

    for(short i = 0; i < m_pView->GetColumnsCount(); i++)
        pNextFirstToDraw = m_pView->NextVisible(pNextFirstToDraw);

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

void IconViewImpl::PageDown( sal_uInt16 nDelta )
{
    sal_uInt16 nRealDelta = nDelta * m_pView->GetColumnsCount();

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
    }
    else
    {
        tools::Rectangle aArea( GetVisibleArea() );
        long nScroll = m_pView->GetEntryHeight() * static_cast<long>(nRealDelta);
        nScroll = -nScroll;
        m_pView->PaintImmediately();
        m_pView->Scroll( 0, nScroll, aArea, ScrollFlags::NoChildren );
        m_pView->PaintImmediately();
        m_pView->NotifyScrolled();
    }

    ShowCursor( true );
}

void IconViewImpl::PageUp( sal_uInt16 nDelta )
{
    sal_uInt16 nRealDelta = nDelta * m_pView->GetColumnsCount();
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
    }
    else
    {
        long nEntryHeight = m_pView->GetEntryHeight();
        tools::Rectangle aArea( GetVisibleArea() );
        m_pView->PaintImmediately();
        m_pView->Scroll( 0, nEntryHeight*nRealDelta, aArea, ScrollFlags::NoChildren );
        m_pView->PaintImmediately();
        m_pView->NotifyScrolled();
    }

    ShowCursor( true );
}

void IconViewImpl::KeyDown( bool bPageDown )
{
    if( !m_aVerSBar->IsVisible() )
        return;

    long nDelta;
    if( bPageDown )
        nDelta = m_aVerSBar->GetPageSize();
    else
        nDelta = 1;

    long nThumbPos = m_aVerSBar->GetThumbPos();

    if( nDelta <= 0 )
        return;

    m_nFlags &= ~LBoxFlags::Filling;

    m_aVerSBar->SetThumbPos( nThumbPos+nDelta );
    if( bPageDown )
        PageDown( static_cast<short>(nDelta) );
    else
        CursorDown();
}

void IconViewImpl::KeyUp( bool bPageUp )
{
    if( !m_aVerSBar->IsVisible() )
        return;

    long nDelta;
    if( bPageUp )
        nDelta = m_aVerSBar->GetPageSize();
    else
        nDelta = 1;

    long nThumbPos = m_aVerSBar->GetThumbPos();

    if( nThumbPos < nDelta )
        nDelta = nThumbPos;

    if( nDelta < 0 )
        return;

    m_nFlags &= ~LBoxFlags::Filling;

    m_aVerSBar->SetThumbPos( nThumbPos - nDelta );
    if( bPageUp )
        PageUp( static_cast<short>(nDelta) );
    else
        CursorUp();
}

long IconViewImpl::GetEntryLine(const SvTreeListEntry* pEntry) const
{
    if(!m_pStartEntry )
        return -1; // invisible position

    long nFirstVisPos = m_pView->GetVisiblePos( m_pStartEntry );
    long nEntryVisPos = m_pView->GetVisiblePos( pEntry );
    nFirstVisPos = nEntryVisPos - nFirstVisPos;

    return nFirstVisPos;
}

Point IconViewImpl::GetEntryPosition(const SvTreeListEntry* pEntry) const
{
    const int pos = m_pView->GetAbsPos( pEntry );

    return Point( ( pos % m_pView->GetColumnsCount() ) * m_pView->GetEntryWidth(),
                 ( pos / m_pView->GetColumnsCount() ) * m_pView->GetEntryHeight() );
}

SvTreeListEntry* IconViewImpl::GetClickedEntry( const Point& rPoint ) const
{
    DBG_ASSERT( m_pView->GetModel(), "IconViewImpl::GetClickedEntry: how can this ever happen?" );
    if ( !m_pView->GetModel() )
        return nullptr;
    if( m_pView->GetEntryCount() == 0 || !m_pStartEntry || !m_pView->GetEntryHeight() || !m_pView->GetEntryWidth())
        return nullptr;

    sal_uInt16 nY = static_cast<sal_uInt16>(rPoint.Y() / m_pView->GetEntryHeight() );
    sal_uInt16 nX = static_cast<sal_uInt16>(rPoint.X() / m_pView->GetEntryWidth() );
    sal_uInt16 nTemp = nY * m_pView->GetColumnsCount() + nX;

    SvTreeListEntry* pEntry = m_pView->NextVisible(m_pStartEntry, nTemp);

    return pEntry;
}

bool IconViewImpl::IsEntryInView( SvTreeListEntry* pEntry ) const
{
    // parent collapsed
    if( !m_pView->IsEntryVisible(pEntry) )
        return false;

    long nY = GetEntryLine( pEntry ) / m_pView->GetColumnsCount() * m_pView->GetEntryHeight();
    if( nY < 0 )
        return false;

    long nMax = m_nVisibleCount / m_pView->GetColumnsCount() * m_pView->GetEntryHeight();
    if( nY >= nMax )
        return false;

    long nStart = GetEntryLine( pEntry ) - GetEntryLine( m_pStartEntry );
    return nStart >= 0;
}

void IconViewImpl::AdjustScrollBars( Size& rSize )
{
    long nEntryHeight = m_pView->GetEntryHeight();
    if( !nEntryHeight )
        return;

    sal_uInt16 nResult = 0;

    Size aOSize( m_pView->Control::GetOutputSizePixel() );

    const WinBits nWindowStyle = m_pView->GetStyle();
    bool bVerSBar = ( nWindowStyle & WB_VSCROLL ) != 0;

    // number of entries that are not collapsed
    sal_uLong nTotalCount = m_pView->GetVisibleCount();

    // number of entries visible within the view
    m_nVisibleCount = aOSize.Height() / nEntryHeight * m_pView->GetColumnsCount();

    long nRows = ( nTotalCount / m_pView->GetColumnsCount() ) + 1;

    // do we need a vertical scrollbar?
    if( bVerSBar || nTotalCount > m_nVisibleCount )
    {
        nResult = 1;
    }

    PositionScrollBars( aOSize, nResult );

    // adapt Range, VisibleRange etc.

    // refresh output size, in case we have to scroll
    tools::Rectangle aRect;
    aRect.SetSize( aOSize );
    m_aSelEng.SetVisibleArea( aRect );

    // vertical scrollbar
    if( !m_bInVScrollHdl )
    {
        m_aVerSBar->SetPageSize( nTotalCount );
        m_aVerSBar->SetVisibleSize( nTotalCount - nRows );
    }
    else
    {
        m_nFlags |= LBoxFlags::EndScrollSetVisSize;
    }

    if( nResult & 0x0001 )
        m_aVerSBar->Show();
    else
        m_aVerSBar->Hide();

    rSize = aOSize;
}

// returns 0 if position is just past the last entry
SvTreeListEntry* IconViewImpl::GetEntry( const Point& rPoint ) const
{
    if( (m_pView->GetEntryCount() == 0) || !m_pStartEntry ||
        (rPoint.Y() > m_aOutputSize.Height())
        || !m_pView->GetEntryHeight()
        || !m_pView->GetEntryWidth())
        return nullptr;

    sal_uInt16 nClickedEntry = static_cast<sal_uInt16>(rPoint.Y() / m_pView->GetEntryHeight() * m_pView->GetColumnsCount() + rPoint.X() / m_pView->GetEntryWidth() );
    sal_uInt16 nTemp = nClickedEntry;
    SvTreeListEntry* pEntry = m_pView->NextVisible(m_pStartEntry, nTemp);
    if( nTemp != nClickedEntry )
        pEntry = nullptr;
    return pEntry;
}

void IconViewImpl::SyncVerThumb()
{
    if( m_pStartEntry )
    {
        long nEntryPos = m_pView->GetVisiblePos( m_pStartEntry );
        m_aVerSBar->SetThumbPos( nEntryPos );
    }
    else
        m_aVerSBar->SetThumbPos( 0 );
}

void IconViewImpl::UpdateAll( bool bInvalidateCompleteView )
{
    FindMostRight();
    m_aVerSBar->SetRange( Range( 0, m_pView->GetVisibleCount() ) );
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

void IconViewImpl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect)
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

    long nRectHeight = rRect.GetHeight();
    long nRectWidth = rRect.GetWidth();
    long nEntryHeight = m_pView->GetEntryHeight();
    long nEntryWidth = m_pView->GetEntryWidth();

    // calculate area for the entries we want to draw
    sal_uInt16 nStartId = static_cast<sal_uInt16>(rRect.Top() / nEntryHeight * m_pView->GetColumnsCount() + (rRect.Left() / nEntryWidth));
    sal_uInt16 nCount = static_cast<sal_uInt16>(( nRectHeight / nEntryHeight + 1 ) * nRectWidth / nEntryWidth);
    nCount += 2; // don't miss an entry

    long nY = nStartId / m_pView->GetColumnsCount() * nEntryHeight;
    long nX = 0;
    SvTreeListEntry* pEntry = m_pStartEntry;
    while (nStartId && pEntry)
    {
        pEntry = m_pView->NextVisible(pEntry);
        nStartId--;
    }

    if (!m_pCursor && !mbNoAutoCurEntry)
    {
        // do not select if multiselection or explicit set
        bool bNotSelect = (m_aSelEng.GetSelectionMode() == SelectionMode::Multiple ) || ((m_nStyle & WB_NOINITIALSELECTION) == WB_NOINITIALSELECTION);
        SetCursor(m_pStartEntry, bNotSelect);
    }

    for(sal_uInt16 n = 0; n< nCount && pEntry; n++)
    {
        static_cast<IconView*>(m_pView.get())->PaintEntry(*pEntry, nX, nY, rRenderContext);
        nX += nEntryWidth;

        if(nX + m_pView->GetEntryWidth() > nEntryWidth * m_pView->GetColumnsCount())
        {
            nY += nEntryHeight;
            nX = 0;
        }
        pEntry = m_pView->NextVisible(pEntry);
    }

    m_nFlags &= ~LBoxFlags::DeselectAll;
    rRenderContext.SetClipRegion();
    m_nFlags &= ~LBoxFlags::InPaint;
}

void IconViewImpl::InvalidateEntry( long nId ) const
{
    if( m_nFlags & LBoxFlags::InPaint )
        return;

    tools::Rectangle aRect( GetVisibleArea() );
    long nMaxBottom = aRect.Bottom();
    aRect.SetTop( nId / m_pView->GetColumnsCount() * m_pView->GetEntryHeight() );
    aRect.SetBottom( aRect.Top() ); aRect.AdjustBottom(m_pView->GetEntryHeight() );

    if( aRect.Top() > nMaxBottom )
        return;
    if( aRect.Bottom() > nMaxBottom )
        aRect.SetBottom( nMaxBottom );
    m_pView->Invalidate( aRect );
}

bool IconViewImpl::KeyInput( const KeyEvent& rKEvt )
{
    const vcl::KeyCode& rKeyCode = rKEvt.GetKeyCode();

    if( rKeyCode.IsMod2() )
        return false; // don't evaluate Alt key

    m_nFlags &= ~LBoxFlags::Filling;

    if( !m_pCursor )
        m_pCursor = m_pStartEntry;
    if( !m_pCursor )
        return false;

    sal_uInt16  aCode = rKeyCode.GetCode();

    bool    bShift = rKeyCode.IsShift();
    bool    bMod1 = rKeyCode.IsMod1();

    SvTreeListEntry* pNewCursor;

    bool bHandled = true;

    long i;
    long nColumns = m_pView->GetColumnsCount();

    switch( aCode )
    {
        case KEY_LEFT:
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

        case KEY_RIGHT:
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

        case KEY_UP:
        {
            if( !IsEntryInView( m_pCursor ) )
                MakeVisible( m_pCursor );

            pNewCursor = m_pCursor;
            for( i = 0; i < nColumns && pNewCursor; i++)
            {
                do
                {
                    pNewCursor = m_pView->PrevVisible(pNewCursor);
                } while( pNewCursor && !IsSelectable(pNewCursor) );
            }

            // if there is no next entry, take the current one
            // this ensures that in case of _one_ entry in the list, this entry is selected when pressing
            // the cursor key
            if ( !pNewCursor && m_pCursor )
                pNewCursor = m_pCursor;

            if( pNewCursor )
            {
                m_aSelEng.CursorPosChanging( bShift, bMod1 );
                SetCursor( pNewCursor, bMod1 );     // no selection, when Ctrl is on
                if( !IsEntryInView( pNewCursor ) )
                    KeyUp( false );
            }
            break;
        }

        case KEY_DOWN:
        {
            if( !IsEntryInView( m_pCursor ) )
                MakeVisible( m_pCursor );

            pNewCursor = m_pCursor;
            for( i = 0; i < nColumns && pNewCursor; i++)
            {
                do
                {
                    pNewCursor = m_pView->NextVisible(pNewCursor);
                } while( pNewCursor && !IsSelectable(pNewCursor) );
            }

            // if there is no next entry, take the current one
            // this ensures that in case of _one_ entry in the list, this entry is selected when pressing
            // the cursor key
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
        }

        case KEY_RETURN:
        {
            bHandled = !m_pView->aDoubleClickHdl.Call(m_pView);
            break;
        }

        case KEY_END:
        {
            pNewCursor = m_pView->GetModel()->Last();

            while( pNewCursor && !IsSelectable(pNewCursor) )
            {
                pNewCursor = m_pView->PrevVisible(pNewCursor);
            }

            m_pStartEntry = pNewCursor;

            while( m_pStartEntry && m_pView->GetAbsPos( m_pStartEntry ) % m_pView->GetColumnsCount() != 0 )
            {
                m_pStartEntry = m_pView->PrevVisible(m_pStartEntry);
            }

            if( pNewCursor && pNewCursor != m_pCursor)
            {
//              SelAllDestrAnch( false );
                m_aSelEng.CursorPosChanging( bShift, bMod1 );
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
