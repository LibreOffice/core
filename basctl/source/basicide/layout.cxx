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
static int const nSplitThickness = 3;
} // namespace

// ctor for derived classes
// pParent: the parent window (BasicIDEShell)
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

// called by Window when resized
void Layout::Resize()
{
    if (IsVisible())
        ArrangeWindows();
}

// ArrangeWindows() -- arranges the child windows
void Layout::ArrangeWindows ()
{
    Size const aSize = GetOutputSizePixel();
    int const nWidth = aSize.Width(), nHeight = aSize.Height();
    if (!nWidth || !nHeight) // empty size
        return;
    if (bFirstSize)
    {
        this->OnFirstSize(nWidth, nHeight); // virtual
        bFirstSize = false;
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

void Layout::DockaWindow (DockingWindow*)
{
    ArrangeWindows();
}

void Layout::Activating (IDEBaseWindow& rIdeWindow)
{
    // first activation
    pChild = &rIdeWindow;
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
    bFirstArrange(true),
    bVertical(eSide == Left || eSide == Right),
    bLower(eSide == Left || eSide == Top),
    nSize(0),
    nLastPos(0),
    aSplitter(pParent, bVertical ? WB_HSCROLL : WB_VSCROLL)
{
    InitSplitter(aSplitter);
}


// Add() -- adds a new window to the side (after construction)
void Layout::SplittedSide::Add (BasicDockingWindow* pWin, Size const& rSize)
{
    int const nSize1 = (bVertical ? rSize.Width() : rSize.Height()) + nSplitThickness;
    int const nSize2 = bVertical ? rSize.Height() : rSize.Width();
    // nSize
    if (nSize1 > nSize)
        nSize = nSize1;
    // window
    vWindows.push_back(pWin);
    // split line
    if (vWindows.size() > 1)
    {
        vSplitters.push_back(boost::make_shared<Splitter>(
            &rLayout, bVertical ? WB_VSCROLL : WB_HSCROLL
        ));
        Splitter& rSplitter = *vSplitters.back();
        rSplitter.SetSplitPosPixel(nLastPos - nSplitThickness);
        InitSplitter(rSplitter);
    }
    // nLastPos
    nLastPos += nSize2 + nSplitThickness;
}

// creating a Point or a Size object
// The coordinate order depends on bVertical (reversed if true).
inline Size Layout::SplittedSide::MakeSize (int A, int B) const
{
    return bVertical ? Size(B, A) : Size(A, B);
}
inline Point Layout::SplittedSide::MakePoint (int A, int B) const
{
    return bVertical ? Point(B, A) : Point(A, B);
}

// IsEmpty() -- are there no windows docked in this strip?
bool Layout::SplittedSide::IsEmpty () const
{
    for (unsigned i = 0; i != vWindows.size(); ++i)
        if (vWindows[i]->IsVisible() && !vWindows[i]->IsFloatingMode())
            return false;
    return true;
}

// GetSize() -- returns the width or height of the strip (depending on the direction)
int Layout::SplittedSide::GetSize () const
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
    int const nLength = bVertical ? aRect.GetSize().Height() : aRect.GetSize().Width();
    int const nOtherSize = bVertical ? aRect.GetSize().Width() : aRect.GetSize().Height();
    // bVertical ? horizontal pozition : vertical pozition
    int const nPos1 = (bVertical ? aRect.Left() : aRect.Top()) +
        (bLower ? 0 : nOtherSize - (nSize - nSplitThickness));
    // bVertical ? vertical position : horizontal position
    int const nPos2 = bVertical ? aRect.Top() : aRect.Left();

    // main line
    {
        // shown if any of the windows is docked
        if (!IsEmpty())
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
    }

    // positioning separator lines and windows
    bool bPrevDocked = false; // is the previous window docked?
    int nStartPos = nPos2; // window position in the strip
    for (unsigned i = 0; i != vWindows.size(); ++i)
    {
        // window
        BasicDockingWindow& rWin = *vWindows[i];
        bool const bDocked = rWin.IsVisible() && !rWin.IsFloatingMode();
        // The window is docked between nStartPos and nEndPos along.
        int const nEndPos = i == vWindows.size() - 1 ?
            nPos2 + nLength : vSplitters[i]->GetSplitPosPixel();
        rWin.ResizeIfDocking(
            MakePoint(nStartPos, nPos1),
            MakeSize(nEndPos - nStartPos, nSize - nSplitThickness)
        );
        // splitting line before the window
        if (i > 0)
        {
            Splitter& rSplit = *vSplitters[i - 1];
            // If neither of two adjacent windows are docked,
            // the splitting line is hidden.
            if (bDocked || bPrevDocked)
            {
                rSplit.Show();
                // the actual pozition and size of the line
                rSplit.SetPosSizePixel(
                    MakePoint(nStartPos - nSplitThickness, nPos1),
                    MakeSize(nSplitThickness, nSize - nSplitThickness)
                );
                // the dragging rectangle
                rSplit.SetDragRectPixel(Rectangle(
                    MakePoint(bVertical ? aRect.Top() : aRect.Left(), nPos1),
                    MakeSize(nLength, nSize - nSplitThickness)
                ));
            }
            else
                rSplit.Hide();
        }
        // next
        bPrevDocked = bDocked;
        nStartPos = nEndPos + nSplitThickness;
    }

    // first arrange
    bFirstArrange = false;
}

IMPL_LINK(Layout::SplittedSide, SplitHdl, Splitter*, pSplitter)
{
    // checking margins
    CheckMarginsFor(pSplitter);
    // nSize has to be changed?
    if (pSplitter == &aSplitter)
    {
        if (bLower)
            nSize = pSplitter->GetSplitPosPixel();
        else
            nSize = (bVertical ? aRect.Right() : aRect.Bottom()) + 1 - pSplitter->GetSplitPosPixel();
    }
    // arranging windows
    rLayout.ArrangeWindows();

    return 0;
}

void Layout::SplittedSide::CheckMarginsFor (Splitter* pSplitter)
{
    // The splitter line cannot be closer to the edges than nMargin pixels.
    static int const nMargin = 16;
    // Checking margins:
    if (int const nLength = pSplitter->IsHorizontal() ?
        aRect.GetWidth() : aRect.GetHeight()
    ) {
        // bounds
        int const nLower = (pSplitter->IsHorizontal() ? aRect.Left() : aRect.Top()) + nMargin;
        int const nUpper = nLower + nLength - 2*nMargin;
        // split position
        int const nPos = pSplitter->GetSplitPosPixel();
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
