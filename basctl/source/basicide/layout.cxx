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

#include "layout.hxx"

#include "bastypes.hxx"

#include <boost/make_shared.hpp>

namespace basctl
{

namespace
{
// the thickness of the splitting lines
static long const nSplitThickness = 3;
} // namespace

// ctor for derived classes
// pParent: the parent window (Shell)
Layout::Layout (Window* pParent) :
    Window(pParent, WB_CLIPCHILDREN),
    pChild(0),
    bFirstSize(true),
    aLeftSide(this, SplittedSide::Left),
    aBottomSide(this, SplittedSide::Bottom)
{
    SetBackground(GetSettings().GetStyleSettings().GetWindowColor());

    Font aFont = GetFont();
    Size aSz = aFont.GetSize();
    aSz.Height() *= 1.5;
    aFont.SetSize(aSz);
    aFont.SetWeight(WEIGHT_BOLD);
    aFont.SetColor(GetSettings().GetStyleSettings().GetWindowTextColor());
    SetFont(aFont);
}

// virtual dtor
Layout::~Layout()
{ }

// removes a docking window
void Layout::Remove (DockingWindow* pWin)
{
    aLeftSide.Remove(pWin);
    aBottomSide.Remove(pWin);
}

// called by Window when resized
void Layout::Resize()
{
    if (IsVisible())
        ArrangeWindows();
}

// ArrangeWindows() -- arranges the child windows
void Layout::ArrangeWindows ()
{
    // prevent recursion via OnFirstSize() -> Add() -> ArrangeWindows()
    static bool bInArrangeWindows = false;
    if (bInArrangeWindows)
        return;
    bInArrangeWindows = true;

    Size const aSize = GetOutputSizePixel();
    long const nWidth = aSize.Width(), nHeight = aSize.Height();
    if (nWidth && nHeight) // non-empty size
    {
        // On first call the derived classes initializes the sizes of the
        // docking windows. This cannot be done at construction because
        // the Layout has empty size at that point.
        if (bFirstSize)
        {
            bFirstSize = false;
            this->OnFirstSize(nWidth, nHeight); // virtual
        }

        // sides
        aBottomSide.ArrangeIn(Rectangle(Point(0, 0), aSize));
        aLeftSide.ArrangeIn(Rectangle(Point(0, 0), Size(nWidth, nHeight - aBottomSide.GetSize())));
        // child in the middle
        pChild->SetPosSizePixel(
            Point(aLeftSide.GetSize(), 0),
            Size(nWidth - aLeftSide.GetSize(), nHeight - aBottomSide.GetSize())
        );
    }

    bInArrangeWindows = false;
}

void Layout::DockaWindow (DockingWindow*)
{
    ArrangeWindows();
}

void Layout::Activating (BaseWindow& rWindow)
{
    // first activation
    pChild = &rWindow;
    ArrangeWindows();
    Show();
    pChild->Activating();
}

void Layout::Deactivating ()
{
    if (pChild)
        pChild->Deactivating();
    Hide();
    pChild = 0;
}

// virtual
void Layout::DataChanged (DataChangedEvent const& rDCEvt)
{
    Window::DataChanged(rDCEvt);
    if (rDCEvt.GetType() == DATACHANGED_SETTINGS && (rDCEvt.GetFlags() & SETTINGS_STYLE))
    {
        bool bInvalidate = false;
        Color aColor = GetSettings().GetStyleSettings().GetWindowColor();
        if (aColor != rDCEvt.GetOldSettings()->GetStyleSettings().GetWindowColor())
        {
            SetBackground(Wallpaper(aColor));
            bInvalidate = true;
        }
        aColor = GetSettings().GetStyleSettings().GetWindowTextColor();
        if (aColor != rDCEvt.GetOldSettings()->GetStyleSettings().GetWindowTextColor())
        {
            Font aFont(GetFont());
            aFont.SetColor(aColor);
            SetFont(aFont);
            bInvalidate = true;
        }
        if (bInvalidate)
            Invalidate();
    }
}

//
// SplittedSide
// ============
//

// ctor
Layout::SplittedSide::SplittedSide (Layout* pParent, Side eSide) :
    rLayout(*pParent),
    bVertical(eSide == Left || eSide == Right),
    bLower(eSide == Left || eSide == Top),
    nSize(0),
    nLastPos(0),
    aSplitter(pParent, bVertical ? WB_HSCROLL : WB_VSCROLL)
{
    InitSplitter(aSplitter);
}


// Add() -- adds a new window to the side (after construction)
void Layout::SplittedSide::Add (DockingWindow* pWin, Size const& rSize)
{
    long const nSize1 = (bVertical ? rSize.Width() : rSize.Height()) + nSplitThickness;
    long const nSize2 = bVertical ? rSize.Height() : rSize.Width();
    // nSize
    if (nSize1 > nSize)
        nSize = nSize1;
    // window
    Item aItem;
    aItem.pWin = pWin;
    aItem.nStartPos = vItems.empty() ? 0 : vItems.back().nEndPos + nSplitThickness;
    aItem.nEndPos = aItem.nStartPos + nSize2;
    // splitter
    if (!vItems.empty())
    {
        aItem.pSplit = boost::make_shared<Splitter>(&rLayout, bVertical ? WB_VSCROLL : WB_HSCROLL);
        aItem.pSplit->SetSplitPosPixel(aItem.nStartPos - nSplitThickness);
        InitSplitter(*aItem.pSplit);
    }
    vItems.push_back(aItem);
    // refresh
    rLayout.ArrangeWindows();
}

// Remove() -- removes a window from the side (if contains)
void Layout::SplittedSide::Remove (DockingWindow* pWin)
{
    // contains?
    unsigned iWin;
    for (iWin = 0; iWin != vItems.size(); ++iWin)
        if (vItems[iWin].pWin == pWin)
            break;
    if (iWin == vItems.size())
        return;
    // remove
    vItems.erase(vItems.begin() + iWin);
    // if that was the first one, remove the first splitter line
    if (iWin == 0 && !vItems.empty())
        vItems.front().pSplit.reset();
}

// creating a Point or a Size object
// The coordinate order depends on bVertical (reversed if true).
inline Size Layout::SplittedSide::MakeSize (long A, long B) const
{
    return bVertical ? Size(B, A) : Size(A, B);
}
inline Point Layout::SplittedSide::MakePoint (long A, long B) const
{
    return bVertical ? Point(B, A) : Point(A, B);
}

// IsDocking() -- is this window currently docking in the strip?
bool Layout::SplittedSide::IsDocking (DockingWindow const& rWin)
{
    return rWin.IsVisible() && !rWin.IsFloatingMode();
}

// IsEmpty() -- are there no windows docked in this strip?
bool Layout::SplittedSide::IsEmpty () const
{
    for (unsigned i = 0; i != vItems.size(); ++i)
        if (IsDocking(*vItems[i].pWin))
            return false;
    return true;
}

// GetSize() -- returns the width or height of the strip (depending on the direction)
long Layout::SplittedSide::GetSize () const
{
    return IsEmpty() ? 0 : nSize;
}

// Arrange() -- arranges the docking windows
// rRect: the available space
void Layout::SplittedSide::ArrangeIn (Rectangle const& rRect)
{
    // saving the rectangle
    aRect = rRect;

    // the length of the side
    long const nLength = bVertical ? aRect.GetSize().Height() : aRect.GetSize().Width();
    long const nOtherSize = bVertical ? aRect.GetSize().Width() : aRect.GetSize().Height();
    // bVertical ? horizontal pozition : vertical pozition
    long const nPos1 = (bVertical ? aRect.Left() : aRect.Top()) +
        (bLower ? 0 : nOtherSize - (nSize - nSplitThickness));
    // bVertical ? vertical position : horizontal position
    long const nPos2 = bVertical ? aRect.Top() : aRect.Left();

    // main line
    bool const bEmpty = IsEmpty();
    // shown if any of the windows is docked
    if (!bEmpty)
    {
        aSplitter.Show();
        // split position
        aSplitter.SetSplitPosPixel((bLower ? nSize : nPos1) - nSplitThickness);
        // the actual position and size
        aSplitter.SetPosSizePixel(
            MakePoint(nPos2, aSplitter.GetSplitPosPixel()),
            MakeSize(nLength, nSplitThickness)
        );
        // dragging rectangle
        aSplitter.SetDragRectPixel(aRect);
    }
    else
        aSplitter.Hide();

    // positioning separator lines and windows
    bool bPrevDocking = false; // is the previous window docked?
    long nStartPos = 0; // window position in the strip
    unsigned iLastWin; // index of last docking window in the strip
    // (iLastWin will be initialized if !bEmpty)
    for (unsigned i = 0; i != vItems.size(); ++i)
    {
        // window
        DockingWindow& rWin = *vItems[i].pWin;
        bool const bDocking = IsDocking(rWin);
        if (bDocking)
            iLastWin = i;
        // sizing window
        rWin.ResizeIfDocking(
            MakePoint(nPos2 + nStartPos, nPos1),
            MakeSize(vItems[i].nEndPos - nStartPos, nSize - nSplitThickness)
        );
        // splitting line before the window
        if (i > 0)
        {
            Splitter& rSplit = *vItems[i].pSplit;
            // If neither of two adjacent windows are docked,
            // the splitting line is hidden.
            // If this window is docking but the previous isn't,
            // then the splitting line is also hidden, because this window
            // will occupy the space of the previous.
            if (bPrevDocking)
            {
                rSplit.Show();
                // the actual pozition and size of the line
                rSplit.SetPosSizePixel(
                    MakePoint(nPos2 + nStartPos - nSplitThickness, nPos1),
                    MakeSize(nSplitThickness, nSize - nSplitThickness)
                );
                // the dragging rectangle
                rSplit.SetDragRectPixel(Rectangle(
                    MakePoint(nPos2, nPos1),
                    MakeSize(nLength, nSize - nSplitThickness)
                ));
            }
            else
                rSplit.Hide();
        }
        // next
        bPrevDocking = bDocking;
        if (bDocking)
            nStartPos = vItems[i].nEndPos + nSplitThickness;
        // We only set nStartPos if this window is docking, because otherwise
        // the next window will occupy also the space of this window.
    }

    // filling the remaining space with the last docking window
    if (!bEmpty && vItems[iLastWin].nEndPos != nLength)
    {
        Item& rItem = vItems[iLastWin];
        Size aSize = rItem.pWin->GetDockingSize();
        (bVertical ? aSize.Height() : aSize.Width()) += nLength - rItem.nEndPos;
        rItem.pWin->ResizeIfDocking(aSize);
        // and hiding the split line after the window
        if (iLastWin < vItems.size() - 1)
            vItems[iLastWin + 1].pSplit->Hide();
    }
}

IMPL_LINK(Layout::SplittedSide, SplitHdl, Splitter*, pSplitter)
{
    // checking margins
    CheckMarginsFor(pSplitter);
    // changing stored sizes
    if (pSplitter == &aSplitter)
    {
        // nSize
        if (bLower)
            nSize = pSplitter->GetSplitPosPixel();
        else
            nSize = (bVertical ? aRect.Right() : aRect.Bottom()) + 1 - pSplitter->GetSplitPosPixel();
    }
    else
    {
        // Item::nStartPos, Item::nLength
        for (unsigned i = 1; i < vItems.size(); ++i)
        {
            if (vItems[i].pSplit.get() == pSplitter)
            {
                // before the line
                vItems[i - 1].nEndPos = pSplitter->GetSplitPosPixel();
                // after the line
                vItems[i].nStartPos = pSplitter->GetSplitPosPixel() + nSplitThickness;
            }
        }
    }
    // arranging windows
    rLayout.ArrangeWindows();

    return 0;
}

void Layout::SplittedSide::CheckMarginsFor (Splitter* pSplitter)
{
    // The splitter line cannot be closer to the edges than nMargin pixels.
    static long const nMargin = 16;
    // Checking margins:
    if (long const nLength = pSplitter->IsHorizontal() ?
        aRect.GetWidth() : aRect.GetHeight()
    ) {
        // bounds
        long const nLower = (pSplitter->IsHorizontal() ? aRect.Left() : aRect.Top()) + nMargin;
        long const nUpper = nLower + nLength - 2*nMargin;
        // split position
        long const nPos = pSplitter->GetSplitPosPixel();
        // checking bounds
        if (nPos < nLower)
            pSplitter->SetSplitPosPixel(nLower);
        if (nPos > nUpper)
            pSplitter->SetSplitPosPixel(nUpper);
    }
}

void Layout::SplittedSide::InitSplitter (Splitter& rSplitter)
{
    // link
    rSplitter.SetSplitHdl(LINK(this, SplittedSide, SplitHdl));
    // color
    Color aColor = rLayout.GetSettings().GetStyleSettings().GetShadowColor();
    rSplitter.SetLineColor(aColor);
    rSplitter.SetFillColor(aColor);
}


} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
