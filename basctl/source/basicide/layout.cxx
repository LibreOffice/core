/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "layout.hxx"

#include "bastypes.hxx"
#include <vcl/settings.hxx>

#include <boost/make_shared.hpp>

namespace basctl
{

namespace
{

static long const nSplitThickness = 3;
} 



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


Layout::~Layout()
{ }


void Layout::Remove (DockingWindow* pWin)
{
    aLeftSide.Remove(pWin);
    aBottomSide.Remove(pWin);
}


void Layout::Resize()
{
    if (IsVisible())
        ArrangeWindows();
}


void Layout::ArrangeWindows ()
{
    
    static bool bInArrangeWindows = false;
    if (bInArrangeWindows)
        return;
    bInArrangeWindows = true;

    Size const aSize = GetOutputSizePixel();
    long const nWidth = aSize.Width(), nHeight = aSize.Height();
    if (nWidth && nHeight) 
    {
        
        
        
        if (bFirstSize)
        {
            bFirstSize = false;
            this->OnFirstSize(nWidth, nHeight); 
        }

        
        aBottomSide.ArrangeIn(Rectangle(Point(0, 0), aSize));
        aLeftSide.ArrangeIn(Rectangle(Point(0, 0), Size(nWidth, nHeight - aBottomSide.GetSize())));
        
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


void Layout::DataChanged (DataChangedEvent const& rDCEvt)
{
    Window::DataChanged(rDCEvt);
    if (rDCEvt.GetType() == DATACHANGED_SETTINGS && (rDCEvt.GetFlags() & SETTINGS_STYLE))
    {
        bool bInvalidate = false;
        Color aColor = GetSettings().GetStyleSettings().GetWindowColor();
        const AllSettings* pOldSettings = rDCEvt.GetOldSettings();
        if (!pOldSettings || aColor != pOldSettings->GetStyleSettings().GetWindowColor())
        {
            SetBackground(Wallpaper(aColor));
            bInvalidate = true;
        }
        aColor = GetSettings().GetStyleSettings().GetWindowTextColor();
        if (!pOldSettings || aColor != pOldSettings->GetStyleSettings().GetWindowTextColor())
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


//


Layout::SplittedSide::SplittedSide (Layout* pParent, Side eSide) :
    rLayout(*pParent),
    bVertical(eSide == Left || eSide == Right),
    bLower(eSide == Left || eSide == Top),
    nSize(0),
    aSplitter(pParent, bVertical ? WB_HSCROLL : WB_VSCROLL)
{
    InitSplitter(aSplitter);
}



void Layout::SplittedSide::Add (DockingWindow* pWin, Size const& rSize)
{
    long const nSize1 = (bVertical ? rSize.Width() : rSize.Height()) + nSplitThickness;
    long const nSize2 = bVertical ? rSize.Height() : rSize.Width();
    
    if (nSize1 > nSize)
        nSize = nSize1;
    
    Item aItem;
    aItem.pWin = pWin;
    aItem.nStartPos = vItems.empty() ? 0 : vItems.back().nEndPos + nSplitThickness;
    aItem.nEndPos = aItem.nStartPos + nSize2;
    
    if (!vItems.empty())
    {
        aItem.pSplit = boost::make_shared<Splitter>(&rLayout, bVertical ? WB_VSCROLL : WB_HSCROLL);
        aItem.pSplit->SetSplitPosPixel(aItem.nStartPos - nSplitThickness);
        InitSplitter(*aItem.pSplit);
    }
    vItems.push_back(aItem);
    
    rLayout.ArrangeWindows();
}


void Layout::SplittedSide::Remove (DockingWindow* pWin)
{
    
    unsigned iWin;
    for (iWin = 0; iWin != vItems.size(); ++iWin)
        if (vItems[iWin].pWin == pWin)
            break;
    if (iWin == vItems.size())
        return;
    
    vItems.erase(vItems.begin() + iWin);
    
    if (iWin == 0 && !vItems.empty())
        vItems.front().pSplit.reset();
}



inline Size Layout::SplittedSide::MakeSize (long A, long B) const
{
    return bVertical ? Size(B, A) : Size(A, B);
}
inline Point Layout::SplittedSide::MakePoint (long A, long B) const
{
    return bVertical ? Point(B, A) : Point(A, B);
}


bool Layout::SplittedSide::IsDocking (DockingWindow const& rWin)
{
    return rWin.IsVisible() && !rWin.IsFloatingMode();
}


bool Layout::SplittedSide::IsEmpty () const
{
    for (unsigned i = 0; i != vItems.size(); ++i)
        if (IsDocking(*vItems[i].pWin))
            return false;
    return true;
}


long Layout::SplittedSide::GetSize () const
{
    return IsEmpty() ? 0 : nSize;
}



void Layout::SplittedSide::ArrangeIn (Rectangle const& rRect)
{
    
    aRect = rRect;

    
    long const nLength = bVertical ? aRect.GetSize().Height() : aRect.GetSize().Width();
    long const nOtherSize = bVertical ? aRect.GetSize().Width() : aRect.GetSize().Height();
    
    long const nPos1 = (bVertical ? aRect.Left() : aRect.Top()) +
        (bLower ? 0 : nOtherSize - (nSize - nSplitThickness));
    
    long const nPos2 = bVertical ? aRect.Top() : aRect.Left();

    
    bool const bEmpty = IsEmpty();
    
    if (!bEmpty)
    {
        aSplitter.Show();
        
        aSplitter.SetSplitPosPixel((bLower ? nSize : nPos1) - nSplitThickness);
        
        aSplitter.SetPosSizePixel(
            MakePoint(nPos2, aSplitter.GetSplitPosPixel()),
            MakeSize(nLength, nSplitThickness)
        );
        
        aSplitter.SetDragRectPixel(aRect);
    }
    else
        aSplitter.Hide();

    
    bool bPrevDocking = false; 
    long nStartPos = 0; 
    unsigned iLastWin = vItems.size(); 

    for (unsigned i = 0; i != vItems.size(); ++i)
    {
        
        DockingWindow& rWin = *vItems[i].pWin;
        bool const bDocking = IsDocking(rWin);
        if (bDocking)
            iLastWin = i;
        
        rWin.ResizeIfDocking(
            MakePoint(nPos2 + nStartPos, nPos1),
            MakeSize(vItems[i].nEndPos - nStartPos, nSize - nSplitThickness)
        );
        
        if (i > 0)
        {
            Splitter& rSplit = *vItems[i].pSplit;
            
            
            
            
            
            if (bPrevDocking)
            {
                rSplit.Show();
                
                rSplit.SetPosSizePixel(
                    MakePoint(nPos2 + nStartPos - nSplitThickness, nPos1),
                    MakeSize(nSplitThickness, nSize - nSplitThickness)
                );
                
                rSplit.SetDragRectPixel(Rectangle(
                    MakePoint(nPos2, nPos1),
                    MakeSize(nLength, nSize - nSplitThickness)
                ));
            }
            else
                rSplit.Hide();
        }
        
        bPrevDocking = bDocking;
        if (bDocking)
            nStartPos = vItems[i].nEndPos + nSplitThickness;
        
        
    }

    
    if (!bEmpty && vItems[iLastWin].nEndPos != nLength)
    {
        Item& rItem = vItems[iLastWin];
        Size aSize = rItem.pWin->GetDockingSize();
        (bVertical ? aSize.Height() : aSize.Width()) += nLength - rItem.nEndPos;
        rItem.pWin->ResizeIfDocking(aSize);
        
        if (iLastWin < vItems.size() - 1)
            vItems[iLastWin + 1].pSplit->Hide();
    }
}

IMPL_LINK(Layout::SplittedSide, SplitHdl, Splitter*, pSplitter)
{
    
    CheckMarginsFor(pSplitter);
    
    if (pSplitter == &aSplitter)
    {
        
        if (bLower)
            nSize = pSplitter->GetSplitPosPixel();
        else
            nSize = (bVertical ? aRect.Right() : aRect.Bottom()) + 1 - pSplitter->GetSplitPosPixel();
    }
    else
    {
        
        for (unsigned i = 1; i < vItems.size(); ++i)
        {
            if (vItems[i].pSplit.get() == pSplitter)
            {
                
                vItems[i - 1].nEndPos = pSplitter->GetSplitPosPixel();
                
                vItems[i].nStartPos = pSplitter->GetSplitPosPixel() + nSplitThickness;
            }
        }
    }
    
    rLayout.ArrangeWindows();

    return 0;
}

void Layout::SplittedSide::CheckMarginsFor (Splitter* pSplitter)
{
    
    static long const nMargin = 16;
    
    if (long const nLength = pSplitter->IsHorizontal() ?
        aRect.GetWidth() : aRect.GetHeight()
    ) {
        
        long const nLower = (pSplitter->IsHorizontal() ? aRect.Left() : aRect.Top()) + nMargin;
        long const nUpper = nLower + nLength - 2*nMargin;
        
        long const nPos = pSplitter->GetSplitPosPixel();
        
        if (nPos < nLower)
            pSplitter->SetSplitPosPixel(nLower);
        if (nPos > nUpper)
            pSplitter->SetSplitPosPixel(nUpper);
    }
}

void Layout::SplittedSide::InitSplitter (Splitter& rSplitter)
{
    
    rSplitter.SetSplitHdl(LINK(this, SplittedSide, SplitHdl));
    
    Color aColor = rLayout.GetSettings().GetStyleSettings().GetShadowColor();
    rSplitter.SetLineColor(aColor);
    rSplitter.SetFillColor(aColor);
}


} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
