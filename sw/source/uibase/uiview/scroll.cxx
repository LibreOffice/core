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

#include <scroll.hxx>

#define SCROLL_LINE_SIZE 250

SwScrollbar::SwScrollbar(vcl::Window *pWin, bool bHoriz)
    : InterimItemWindow(pWin, "modules/swriter/ui/scrollbars.ui", "ScrollBars")
    , m_bHori(bHoriz)
    , m_bAuto(false)
    , m_bVisible(false)
    , m_bSizeSet(false)
    , m_xScrollBar(m_xBuilder->weld_scrollbar(bHoriz ? "horizontal" : "vertical"))
{
    m_xScrollBar->show();

    // No mirroring for horizontal scrollbars
    if (bHoriz)
        m_xScrollBar->set_direction(false);
}

void SwScrollbar::dispose()
{
    m_xScrollBar.reset();
    InterimItemWindow::dispose();
}

// Will be called after a change of the document size
// to refresh the range of the scrollbars.
void SwScrollbar::DocSzChgd( const Size &rSize )
{
    m_aDocSz = rSize;
    SetRange( Range( 0, m_bHori ? rSize.Width() : rSize.Height()) );
    const sal_uLong nVisSize = GetVisibleSize();
    SetLineSize( SCROLL_LINE_SIZE );
    SetPageSize( nVisSize * 77 / 100 );
}

// Will be called after a change of the visible view section.
void SwScrollbar::ViewPortChgd( const tools::Rectangle &rRect )
{
    tools::Long nThumb, nVisible;
    if( m_bHori )
    {
        nThumb = rRect.Left();
        nVisible = rRect.GetWidth();
    }
    else
    {
        nThumb = rRect.Top();
        nVisible = rRect.GetHeight();
    }

    SetVisibleSize( nVisible );
    DocSzChgd(m_aDocSz);
    SetThumbPos( nThumb );
    if(m_bAuto)
        AutoShow();
}

void SwScrollbar::ExtendedShow( bool bSet )
{
    m_bVisible = bSet;
    if( (!bSet ||  !m_bAuto) && IsUpdateMode() && m_bSizeSet)
    {
        InterimItemWindow::Show(bSet);
    }
}

void SwScrollbar::SetPosSizePixel( const Point& rNewPos, const Size& rNewSize )
{
    InterimItemWindow::SetPosSizePixel(rNewPos, rNewSize);
    m_bSizeSet = true;
    if(m_bVisible)
        ExtendedShow();
}

void SwScrollbar::SetAuto(bool bSet)
{
    if(m_bAuto != bSet)
    {
        m_bAuto = bSet;

        // hide automatically - then show
        if(!m_bAuto && m_bVisible && !InterimItemWindow::IsVisible())
            ExtendedShow();
        else if(m_bAuto)
            AutoShow(); // or hide automatically
    }
}

void SwScrollbar::AutoShow()
{
    tools::Long nVis = GetVisibleSize();
    tools::Long nLen = GetRange().Len();
    if( nVis >= nLen - 1)
    {
        if(InterimItemWindow::IsVisible())
            InterimItemWindow::Show(false);
    }
    else if ( !InterimItemWindow::IsVisible() )
    {
        InterimItemWindow::Show();
    }
}

void SwScrollbar::SetRange(const Range& rRange)
{
    m_xScrollBar->adjustment_set_lower(rRange.Min());
    m_xScrollBar->adjustment_set_upper(rRange.Max());
}

Range SwScrollbar::GetRange() const
{
    return Range(m_xScrollBar->adjustment_get_lower(),
                 m_xScrollBar->adjustment_get_upper());
}

void SwScrollbar::SetRangeMin(tools::Long nNewRange)
{
    m_xScrollBar->adjustment_set_lower(nNewRange);
}

tools::Long SwScrollbar::GetRangeMin() const
{
    return m_xScrollBar->adjustment_get_lower();
}

void SwScrollbar::SetRangeMax(tools::Long nNewRange)
{
    m_xScrollBar->adjustment_set_upper(nNewRange);
}

tools::Long SwScrollbar::GetRangeMax() const
{
    return m_xScrollBar->adjustment_get_upper();
}

void SwScrollbar::SetLineSize(tools::Long nNewSize)
{
    m_xScrollBar->adjustment_set_step_increment(nNewSize);
}

tools::Long SwScrollbar::GetLineSize() const
{
    return m_xScrollBar->adjustment_get_step_increment();
}

void SwScrollbar::SetPageSize(tools::Long nNewSize)
{
    m_xScrollBar->adjustment_set_page_increment(nNewSize);
}

tools::Long SwScrollbar::GetPageSize() const
{
    return m_xScrollBar->adjustment_get_page_increment();
}

void SwScrollbar::SetVisibleSize(tools::Long nNewSize)
{
    m_xScrollBar->adjustment_set_page_size(nNewSize);
}

tools::Long SwScrollbar::GetVisibleSize() const
{
    return m_xScrollBar->adjustment_get_page_size();
}

void SwScrollbar::SetThumbPos(tools::Long nThumbPos)
{
    m_xScrollBar->adjustment_set_value(nThumbPos);
}

tools::Long SwScrollbar::GetThumbPos() const
{
    return m_xScrollBar->adjustment_get_value();
}

void SwScrollbar::SetScrollHdl(const Link<weld::Scrollbar&, void>& rLink)
{
    m_aLink = rLink;
    m_xScrollBar->connect_adjustment_changed(rLink);
}

tools::Long SwScrollbar::DoScroll(tools::Long nNewPos)
{
    const auto nOrig = m_xScrollBar->adjustment_get_value();
    m_xScrollBar->adjustment_set_value(nNewPos);
    m_aLink.Call(*m_xScrollBar);
    return m_xScrollBar->adjustment_get_value() - nOrig;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
