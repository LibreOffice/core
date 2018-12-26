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

#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/listctrl.hxx>
#include <vcl/settings.hxx>

ListControl::ListControl(vcl::Window* pParent, WinBits nBits):
    Control(pParent, nBits),
    mbHasScrollBar(false),
    mpScrollBar(VclPtr<ScrollBar>::Create(this, WB_VERT))
{
    mpScrollBar->SetScrollHdl( LINK( this, ListControl, ScrollHdl ) );
    mpScrollBar->EnableDrag();
}

ListControl::~ListControl()
{
    disposeOnce();
}

void ListControl::dispose()
{
    mpScrollBar.disposeAndClear();
    for (auto& aEntry : maEntries)
        aEntry.disposeAndClear();
    maEntries.clear();

    Control::dispose();
}

void ListControl::RecalcAll()
{
    // avoid recalculating while we are disposing
    // children. This just leads to complex invalid memory
    // access patterns that are not fixable.
    if (isDisposed())
        return;

    sal_Int32 nTotalHeight = 0;
    for (const auto& item : maEntries)
    {
        if (!item)
            continue;
        nTotalHeight += item->GetSizePixel().Height();
    }

    Size aCtrlSize = GetOutputSize();
    long nSrcBarSize = GetSettings().GetStyleSettings().GetScrollBarSize();
    if(nTotalHeight > GetSizePixel().Height())
    {
        mbHasScrollBar = true;
        mpScrollBar->SetPosSizePixel(Point(aCtrlSize.Width() -nSrcBarSize, 0),
                Size(nSrcBarSize, aCtrlSize.Height()) );
        mpScrollBar->SetRangeMax(nTotalHeight);
        mpScrollBar->SetVisibleSize(aCtrlSize.Height());
        mpScrollBar->Show();
    }
    else
    {
        mbHasScrollBar = false;
        mpScrollBar->Hide();
    }

    Point aPoint(0,-1*mpScrollBar->GetThumbPos());
    for (const auto& item : maEntries)
    {
        if (!item)
            continue;
        item->SetPosPixel(aPoint);
        Size aSize = item->GetSizePixel();
        if(mbHasScrollBar)
            aSize.setWidth( aCtrlSize.Width() - nSrcBarSize );
        else
            aSize.setWidth( aCtrlSize.Width() );
        item->SetSizePixel(aSize);

        aPoint.AdjustY(item->GetSizePixel().Height() );
    }
}

Size ListControl::GetOptimalSize() const
{
    return LogicToPixel(Size(300, 185), MapMode(MapUnit::MapAppFont));
}

void ListControl::Resize()
{
    Control::Resize();
    RecalcAll();
}

void ListControl::queue_resize(StateChangedType eReason)
{
    Control::queue_resize(eReason);
    RecalcAll();
}

void ListControl::DoScroll(long nDelta)
{
    Point aNewPoint = mpScrollBar->GetPosPixel();
    tools::Rectangle aRect(Point(), GetOutputSize());
    aRect.AdjustRight( -(mpScrollBar->GetSizePixel().Width()) );
    Scroll( 0, -nDelta, aRect );
    mpScrollBar->SetPosPixel(aNewPoint);
}

IMPL_LINK_NOARG( ListControl, ScrollHdl, ScrollBar*, void )
{
    DoScroll(mpScrollBar->GetDelta());
}

void ListControl::addEntry(const VclPtr<Window>& xEntry, sal_uInt32 nPos)
{
    xEntry->Show();
    if (nPos < maEntries.size())
    {
        maEntries.insert(maEntries.begin() + nPos, xEntry);
    }
    else
    {
        maEntries.push_back(xEntry);
    }
    RecalcAll();
}

void ListControl::deleteEntry(sal_uInt32 nPos)
{
    if (nPos >= maEntries.size())
        return;

    maEntries[nPos].disposeAndClear();
    maEntries.erase(maEntries.begin() + nPos);
    RecalcAll();
}

std::vector<VclPtr<vcl::Window>> const & ListControl::getEntries() const
{
    return maEntries;
}

bool ListControl::EventNotify( NotifyEvent& rNEvt )
{
    if (rNEvt.GetType() == MouseNotifyEvent::COMMAND)
    {
        const CommandEvent* pEvent = rNEvt.GetCommandEvent();
        if (pEvent && pEvent->GetCommand() == CommandEventId::Wheel)
        {
            HandleScrollCommand(*pEvent, nullptr, mpScrollBar.get());
        }
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
