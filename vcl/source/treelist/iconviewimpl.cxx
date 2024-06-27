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
#include <vcl/toolkit/treelistentry.hxx>
#include <tools/debug.hxx>
#include <iconview.hxx>
#include "iconviewimpl.hxx"

IconViewImpl::IconViewImpl( SvTreeListBox* pTreeListBox, SvTreeList* pTreeList, WinBits nWinStyle )
: SvImpLBox( pTreeListBox, pTreeList, nWinStyle )
{
}

static bool IsSeparator(const SvTreeListEntry* entry)
{
    return entry && entry->GetFlags() & SvTLEntryFlags::IS_SEPARATOR;
}

Size IconViewImpl::GetEntrySize(const SvTreeListEntry& entry) const
{
    return static_cast<const IconView*>(m_pView.get())->GetEntrySize(entry);
}

void IconViewImpl::IterateVisibleEntryAreas(const IterateEntriesFunc& f, bool fromStartEntry) const
{
    tools::Long x = 0, y = 0;
    short column = 0;
    const tools::Long rowWidth = m_pView->GetEntryWidth() * m_pView->GetColumnsCount();
    tools::Long nPrevHeight = 0;
    for (auto entry = fromStartEntry ? m_pStartEntry : m_pView->FirstVisible(); entry;
         entry = m_pView->NextVisible(entry))
    {
        const Size s = GetEntrySize(*entry);
        if (x >= rowWidth || IsSeparator(entry))
        {
            column = 0;
            x = 0;
            y += nPrevHeight;
        }
        EntryAreaInfo info{ entry, column, tools::Rectangle{ Point{ x, y }, s } };
        const auto result = f(info);
        if (result == CallbackResult::Stop)
            return;
        ++column;
        x += s.Width();
        nPrevHeight = s.Height();
    }
}

tools::Long IconViewImpl::GetEntryRow(const SvTreeListEntry* entry) const
{
    tools::Long nEntryRow = -1;
    auto GetRow = [entry, &nEntryRow, row = -1](const EntryAreaInfo& info) mutable
    {
        if (info.column == 0 && !IsSeparator(info.entry))
            ++row;
        if (info.entry != entry)
            return CallbackResult::Continue;
        nEntryRow = row;
        return CallbackResult::Stop;
    };
    IterateVisibleEntryAreas(GetRow);
    return nEntryRow;
}

void IconViewImpl::SetStartEntry(SvTreeListEntry* entry)
{
    const tools::Long max = m_aVerSBar->GetRangeMax() - m_aVerSBar->GetVisibleSize();
    tools::Long row = -1;
    auto GetEntryAndRow = [&entry, &row, max, found = entry](const EntryAreaInfo& info) mutable
    {
        if (info.column == 0 && !IsSeparator(info.entry))
        {
            found = info.entry;
            ++row;
        }
        if (row >= max || info.entry == entry)
        {
            entry = found;
            return CallbackResult::Stop;
        }
        return CallbackResult::Continue;
    };
    IterateVisibleEntryAreas(GetEntryAndRow);

    m_pStartEntry = entry;
    m_aVerSBar->SetThumbPos(row);
    m_pView->Invalidate(GetVisibleArea());
}

void IconViewImpl::ScrollTo(SvTreeListEntry* entry)
{
    if (!m_aVerSBar->IsVisible())
        return;
    const tools::Long entryRow = GetEntryRow(entry);
    const tools::Long oldStartRow = m_aVerSBar->GetThumbPos();
    if (entryRow < oldStartRow)
        IconViewImpl::SetStartEntry(entry);
    const tools::Long visibleRows = m_aVerSBar->GetVisibleSize();
    const tools::Long posRelativeToBottom = entryRow - (oldStartRow + visibleRows) + 1;
    if (posRelativeToBottom > 0)
        IconViewImpl::SetStartEntry(GoToNextRow(m_pStartEntry, posRelativeToBottom));
}

SvTreeListEntry* IconViewImpl::GoToPrevRow(SvTreeListEntry* pEntry, int nRows) const
{
    SvTreeListEntry* pPrev = pEntry;
    auto FindPrev = [this, pEntry, nRows, &pPrev,
                     prevs = std::vector<SvTreeListEntry*>()](const EntryAreaInfo& info) mutable
    {
        if (info.column == 0 && !IsSeparator(info.entry))
            prevs.push_back(info.entry);
        if (pEntry == info.entry)
        {
            if (prevs.size() > 1)
            {
                int i = std::max(0, static_cast<int>(prevs.size()) - nRows - 1);
                pPrev = prevs[i];
                for (short column = info.column; column; --column)
                {
                    SvTreeListEntry* pNext = m_pView->NextVisible(pPrev);
                    if (!pNext || IsSeparator(pNext))
                        break;
                    pPrev = pNext;
                }
            }
            return CallbackResult::Stop;
        }
        return CallbackResult::Continue;
    };
    IterateVisibleEntryAreas(FindPrev);

    return pPrev;
}

SvTreeListEntry* IconViewImpl::GoToNextRow(SvTreeListEntry* pEntry, int nRows) const
{
    SvTreeListEntry* pNext = pEntry;
    auto FindNext
        = [pEntry, nRows, &pNext, column = -1](const EntryAreaInfo& info) mutable
    {
        if (info.column <= column && !IsSeparator(info.entry))
        {
            if (info.column == 0 && --nRows < 0)
                return CallbackResult::Stop;
            pNext = info.entry;
            if (info.column == column && nRows == 0)
                return CallbackResult::Stop;
        }
        else if (pEntry == info.entry)
        {
            column = info.column;
        }
        return CallbackResult::Continue;
    };
    IterateVisibleEntryAreas(FindNext);

    return pNext;
}

void IconViewImpl::CursorUp()
{
    if (!m_pStartEntry)
        return;

    SvTreeListEntry* pPrevFirstToDraw = GoToPrevRow(m_pStartEntry, 1);

    m_nFlags &= ~LBoxFlags::Filling;
    ShowCursor( false );
    SetStartEntry(pPrevFirstToDraw);
    ShowCursor( true );
    m_pView->NotifyScrolled();
}

void IconViewImpl::CursorDown()
{
    if (!m_pStartEntry)
        return;

    SvTreeListEntry* pNextFirstToDraw = GoToNextRow(m_pStartEntry, 1);

    m_nFlags &= ~LBoxFlags::Filling;
    ShowCursor( false );
    SetStartEntry(pNextFirstToDraw);
    ShowCursor( true );
    m_pView->NotifyScrolled();
}

void IconViewImpl::PageDown( sal_uInt16 nDelta )
{
    if( !nDelta )
        return;

    if (!m_pStartEntry)
        return;

    SvTreeListEntry* pNext = GoToNextRow(m_pStartEntry, nDelta);

    ShowCursor( false );

    m_nFlags &= ~LBoxFlags::Filling;
    SetStartEntry(pNext);

    ShowCursor( true );
}

void IconViewImpl::PageUp( sal_uInt16 nDelta )
{
    if( !nDelta )
        return;

    if (!m_pStartEntry)
        return;

    SvTreeListEntry* pPrev = GoToPrevRow(m_pStartEntry, nDelta);

    m_nFlags &= ~LBoxFlags::Filling;
    ShowCursor( false );

    SetStartEntry(pPrev);

    ShowCursor( true );
}

void IconViewImpl::KeyDown( bool bPageDown )
{
    if( !m_aVerSBar->IsVisible() )
        return;

    tools::Long nDelta;
    if( bPageDown )
        nDelta = m_aVerSBar->GetPageSize();
    else
        nDelta = 1;

    if( nDelta <= 0 )
        return;

    m_nFlags &= ~LBoxFlags::Filling;

    if( bPageDown )
        PageDown( static_cast<short>(nDelta) );
    else
        CursorDown();
}

void IconViewImpl::KeyUp( bool bPageUp )
{
    if( !m_aVerSBar->IsVisible() )
        return;

    tools::Long nDelta;
    if( bPageUp )
        nDelta = m_aVerSBar->GetPageSize();
    else
        nDelta = 1;

    m_nFlags &= ~LBoxFlags::Filling;

    if( bPageUp )
        PageUp( static_cast<short>(nDelta) );
    else
        CursorUp();
}

tools::Long IconViewImpl::GetEntryLine(const SvTreeListEntry* pEntry) const
{
    if(!m_pStartEntry )
        return -1; // invisible position

    return IconViewImpl::GetEntryPosition(pEntry).Y();
}

Point IconViewImpl::GetEntryPosition(const SvTreeListEntry* pEntry) const
{
    Point result{ -m_pView->GetEntryWidth(), -m_pView->GetEntryHeight() }; // invisible
    auto FindEntryPos = [pEntry, &result](const EntryAreaInfo& info)
    {
        if (pEntry == info.entry)
        {
            result = info.area.TopLeft();
            return CallbackResult::Stop;
        }
        return CallbackResult::Continue;
    };
    IterateVisibleEntryAreas(FindEntryPos, true);

    return result;
}

// Returns the last entry (in respective row) if position is just past the last entry
SvTreeListEntry* IconViewImpl::GetClickedEntry( const Point& rPoint ) const
{
    DBG_ASSERT( m_pView->GetModel(), "IconViewImpl::GetClickedEntry: how can this ever happen?" );
    if ( !m_pView->GetModel() )
        return nullptr;
    if( m_pView->GetEntryCount() == 0 || !m_pStartEntry || !m_pView->GetEntryHeight() || !m_pView->GetEntryWidth())
        return nullptr;

    SvTreeListEntry* pEntry = nullptr;
    auto FindEntryByPos = [&pEntry, &rPoint](const EntryAreaInfo& info)
    {
        if (info.area.Contains(rPoint))
        {
            pEntry = info.entry;
            return CallbackResult::Stop;
        }
        else if (info.area.Top() > rPoint.Y())
        {
            return CallbackResult::Stop; // we are already below the clicked row
        }
        else if (info.area.Bottom() > rPoint.Y())
        {
            pEntry = info.entry; // Same row; store the entry in case the click is past all entries
        }
        return CallbackResult::Continue;
    };
    IterateVisibleEntryAreas(FindEntryByPos, true);

    return pEntry;
}

bool IconViewImpl::IsEntryInView( SvTreeListEntry* pEntry ) const
{
    // parent collapsed
    if( !m_pView->IsEntryVisible(pEntry) )
        return false;

    tools::Long nY = GetEntryLine( pEntry );
    if( nY < 0 )
        return false;

    tools::Long height = GetEntrySize(*pEntry).Height();
    if (nY + height > m_aOutputSize.Height())
        return false;

    return true;
}

void IconViewImpl::AdjustScrollBars( Size& rSize )
{
    tools::Long nEntryHeight = m_pView->GetEntryHeight();
    if( !nEntryHeight )
        return;

    sal_uInt16 nResult = 0;

    Size aOSize( m_pView->Control::GetOutputSizePixel() );

    const WinBits nWindowStyle = m_pView->GetStyle();
    bool bVerSBar = ( nWindowStyle & WB_VSCROLL ) != 0;

    // number of entries visible within the view
    const tools::Long nVisibleRows = aOSize.Height() / nEntryHeight;
    m_nVisibleCount = nVisibleRows * m_pView->GetColumnsCount();

    tools::Long nTotalRows = 0;
    tools::Long totalHeight = 0;
    auto CountRowsAndHeight = [&nTotalRows, &totalHeight](const EntryAreaInfo& info)
    {
        totalHeight = std::max(totalHeight, info.area.Bottom());
        if (info.column == 0 && !IsSeparator(info.entry))
            ++nTotalRows;
        return CallbackResult::Continue;
    };
    IterateVisibleEntryAreas(CountRowsAndHeight);

    // do we need a vertical scrollbar?
    if( bVerSBar || totalHeight > aOSize.Height())
    {
        nResult = 1;
    }

    // do we need a Horizontal scrollbar?
    bool bHorSBar = (nWindowStyle & WB_HSCROLL) != 0;
    if (bHorSBar || m_pView->GetEntryWidth() > aOSize.Width())
    {
        nResult += 2;
        m_aHorSBar->SetRange(Range(0, m_pView->GetEntryWidth()));
        m_aHorSBar->SetVisibleSize(aOSize.Width());
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
        m_aVerSBar->SetRange(Range(0, nTotalRows));
        m_aVerSBar->SetPageSize(nVisibleRows);
        m_aVerSBar->SetVisibleSize(nVisibleRows);
    }
    else
    {
        m_nFlags |= LBoxFlags::EndScrollSetVisSize;
    }

    if( nResult & 0x0001 )
        m_aVerSBar->Show();
    else
        m_aVerSBar->Hide();

    if (nResult & 0x0002)
        m_aHorSBar->Show();
    else
        m_aHorSBar->Hide();

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

    SvTreeListEntry* pEntry = nullptr;
    auto FindEntryByPos = [&pEntry, &rPoint](const EntryAreaInfo& info)
    {
        if (info.area.Contains(rPoint))
        {
            pEntry = info.entry;
            return CallbackResult::Stop;
        }
        else if (info.area.Top() > rPoint.Y())
        {
            return CallbackResult::Stop; // we are already below the clicked row
        }
        return CallbackResult::Continue;
    };
    IterateVisibleEntryAreas(FindEntryByPos, true);

    return pEntry;
}

void IconViewImpl::SyncVerThumb()
{
    m_aVerSBar->SetThumbPos(GetEntryRow(m_pStartEntry));
}

void IconViewImpl::UpdateAll( bool bInvalidateCompleteView )
{
    FindMostRight();
    AdjustScrollBars(m_aOutputSize);
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

    if (!m_pCursor && !mbNoAutoCurEntry)
    {
        // do not select if multiselection or explicit set
        bool bNotSelect = (m_aSelEng.GetSelectionMode() == SelectionMode::Multiple ) || ((m_nStyle & WB_NOINITIALSELECTION) == WB_NOINITIALSELECTION);
        SetCursor(m_pStartEntry, bNotSelect);
    }

    auto PaintEntry = [iconView = static_cast<IconView*>(m_pView.get()), &rRect,
                       &rRenderContext](const EntryAreaInfo& info)
    {
        if (!info.area.GetIntersection(rRect).IsEmpty())
        {
            iconView->PaintEntry(*info.entry, info.area.Left(), info.area.Top(), rRenderContext);
        }
        else if (info.area.Top() > rRect.Bottom())
        {
            return CallbackResult::Stop; // we are already below the last visible row
        }
        return CallbackResult::Continue;
    };
    IterateVisibleEntryAreas(PaintEntry, true);

    m_nFlags &= ~LBoxFlags::DeselectAll;
    rRenderContext.SetClipRegion();
    m_nFlags &= ~LBoxFlags::InPaint;
}

void IconViewImpl::InvalidateEntry( tools::Long nId ) const
{
    if( m_nFlags & LBoxFlags::InPaint )
        return;
    if (nId < 0)
        return;

    // nId is a Y coordinate of the top of the element, coming from GetEntryLine
    tools::Rectangle aRect( GetVisibleArea() );
    if (nId > aRect.Bottom())
        return;
    aRect.SetTop(nId); // Invalidate everything below
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
            pNewCursor = GoToPrevRow(m_pCursor, 1);

            if( pNewCursor )
            {
                m_aSelEng.CursorPosChanging( bShift, bMod1 );
                SetCursor( pNewCursor, bMod1 );     // no selection, when Ctrl is on
                ScrollTo(pNewCursor);
            }
            break;
        }

        case KEY_DOWN:
        {
            pNewCursor = GoToNextRow(m_pCursor, 1);

            if( pNewCursor )
            {
                m_aSelEng.CursorPosChanging( bShift, bMod1 );
                ScrollTo(pNewCursor);
                SetCursor(pNewCursor, bMod1); // no selection, when Ctrl is on
            }
            else
                KeyDown( false );   // because scrollbar range might still
                                        // allow scrolling
            break;
        }

        case KEY_PAGEUP:
            if (!bMod1)
            {
                const sal_uInt16 nDelta = m_aVerSBar->GetPageSize();
                pNewCursor = GoToPrevRow(m_pCursor, nDelta);

                if (pNewCursor)
                {
                    m_aSelEng.CursorPosChanging(bShift, bMod1);
                    ScrollTo(pNewCursor);
                    SetCursor(pNewCursor);
                }
            }
            else
                bHandled = false;
            break;

        case KEY_PAGEDOWN:
            if (!bMod1)
            {
                const sal_uInt16 nDelta = m_aVerSBar->GetPageSize();
                pNewCursor = GoToNextRow(m_pCursor, nDelta);

                if (pNewCursor)
                {
                    m_aSelEng.CursorPosChanging(bShift, bMod1);
                    ScrollTo(pNewCursor);
                    SetCursor(pNewCursor);
                }
                else
                    KeyDown(false);
            }
            else
                bHandled = false;
            break;

        case KEY_RETURN:
        case KEY_SPACE:
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

            SetStartEntry(pNewCursor);

            if( pNewCursor && pNewCursor != m_pCursor)
            {
//              SelAllDestrAnch( false );
                m_aSelEng.CursorPosChanging( bShift, bMod1 );
                SetCursor( pNewCursor );
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
