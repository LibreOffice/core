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

#include <svtools/scrolladaptor.hxx>

ScrollAdaptor::ScrollAdaptor(vcl::Window* pWin, bool bHoriz)
    : InterimItemWindow(pWin, "svt/ui/scrollbars.ui", "ScrollBars")
    , m_xScrollBar(
          m_xBuilder->weld_scrollbar(bHoriz ? OUString("horizontal") : OUString("vertical")))
    , m_bHori(bHoriz)
{
    m_xScrollBar->show();
    SetSizePixel(GetOptimalSize());
}

void ScrollAdaptor::dispose()
{
    m_xScrollBar.reset();
    InterimItemWindow::dispose();
}

void ScrollAdaptor::SetRange(const Range& rRange)
{
    m_xScrollBar->adjustment_set_lower(rRange.Min());
    m_xScrollBar->adjustment_set_upper(rRange.Max());
}

Range ScrollAdaptor::GetRange() const
{
    return Range(m_xScrollBar->adjustment_get_lower(), m_xScrollBar->adjustment_get_upper());
}

void ScrollAdaptor::SetRangeMin(tools::Long nNewRange)
{
    m_xScrollBar->adjustment_set_lower(nNewRange);
}

tools::Long ScrollAdaptor::GetRangeMin() const { return m_xScrollBar->adjustment_get_lower(); }

void ScrollAdaptor::SetRangeMax(tools::Long nNewRange)
{
    m_xScrollBar->adjustment_set_upper(nNewRange);
}

tools::Long ScrollAdaptor::GetRangeMax() const { return m_xScrollBar->adjustment_get_upper(); }

void ScrollAdaptor::SetLineSize(tools::Long nNewSize)
{
    m_xScrollBar->adjustment_set_step_increment(nNewSize);
}

tools::Long ScrollAdaptor::GetLineSize() const
{
    return m_xScrollBar->adjustment_get_step_increment();
}

void ScrollAdaptor::SetPageSize(tools::Long nNewSize)
{
    m_xScrollBar->adjustment_set_page_increment(nNewSize);
}

tools::Long ScrollAdaptor::GetPageSize() const
{
    return m_xScrollBar->adjustment_get_page_increment();
}

void ScrollAdaptor::SetVisibleSize(tools::Long nNewSize)
{
    m_xScrollBar->adjustment_set_page_size(nNewSize);
}

tools::Long ScrollAdaptor::GetVisibleSize() const
{
    return m_xScrollBar->adjustment_get_page_size();
}

void ScrollAdaptor::SetThumbPos(tools::Long nThumbPos)
{
    m_xScrollBar->adjustment_set_value(nThumbPos);
}

tools::Long ScrollAdaptor::GetThumbPos() const { return m_xScrollBar->adjustment_get_value(); }

ScrollType ScrollAdaptor::GetScrollType() const { return m_xScrollBar->get_scroll_type(); }

void ScrollAdaptor::EnableRTL(bool bEnable) { m_xScrollBar->set_direction(bEnable); }

void ScrollAdaptor::SetScrollHdl(const Link<weld::Scrollbar&, void>& rLink)
{
    m_aLink = rLink;
    m_xScrollBar->connect_adjustment_changed(rLink);
}

void ScrollAdaptor::SetMouseReleaseHdl(const Link<const MouseEvent&, bool>& rLink)
{
    m_xScrollBar->connect_mouse_release(rLink);
}

tools::Long ScrollAdaptor::DoScroll(tools::Long nNewPos)
{
    const auto nOrig = m_xScrollBar->adjustment_get_value();
    m_xScrollBar->adjustment_set_value(nNewPos);
    m_aLink.Call(*m_xScrollBar);
    return m_xScrollBar->adjustment_get_value() - nOrig;
}

void ScrollAdaptor::SetThickness(int nThickness) { m_xScrollBar->set_scroll_thickness(nThickness); }

void ScrollAdaptor::SetSwapArrows(bool bSwap) { m_xScrollBar->set_scroll_swap_arrows(bSwap); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
