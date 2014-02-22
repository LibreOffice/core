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


#include "controller/SlsScrollBarManager.hxx"

#include "SlideSorter.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsVisibleAreaManager.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsTheme.hxx"
#include "Window.hxx"
#include "sdpage.hxx"

#include <boost/limits.hpp>

#include <vcl/scrbar.hxx>

namespace sd { namespace slidesorter { namespace controller {

ScrollBarManager::ScrollBarManager (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      mpHorizontalScrollBar(mrSlideSorter.GetHorizontalScrollBar()),
      mpVerticalScrollBar(mrSlideSorter.GetVerticalScrollBar()),
      mnHorizontalPosition (0),
      mnVerticalPosition (0),
      maScrollBorder (20,20),
      mnHorizontalScrollFactor (0.15),
      mnVerticalScrollFactor (0.25),
      mpScrollBarFiller(mrSlideSorter.GetScrollBarFiller()),
      maAutoScrollTimer(),
      maAutoScrollOffset(0,0),
      mbIsAutoScrollActive(false),
      mpContentWindow(mrSlideSorter.GetContentWindow()),
      maAutoScrollFunctor()
{
    
    
    
    
    
    mpHorizontalScrollBar->Hide();
    mpVerticalScrollBar->Hide();
    mpScrollBarFiller->Hide();

    maAutoScrollTimer.SetTimeout(25);
    maAutoScrollTimer.SetTimeoutHdl (
        LINK(this, ScrollBarManager, AutoScrollTimeoutHandler));
}




ScrollBarManager::~ScrollBarManager (void)
{
}




void ScrollBarManager::LateInitialization (void)
{
}




void ScrollBarManager::Connect (void)
{
    if (mpVerticalScrollBar != 0)
    {
        mpVerticalScrollBar->SetScrollHdl (
            LINK(this, ScrollBarManager, VerticalScrollBarHandler));
    }
    if (mpHorizontalScrollBar != 0)
    {
        mpHorizontalScrollBar->SetScrollHdl(
            LINK(this, ScrollBarManager, HorizontalScrollBarHandler));
    }
}




void ScrollBarManager::Disconnect (void)
{
    if (mpVerticalScrollBar != 0)
    {
        mpVerticalScrollBar->SetScrollHdl (Link());
    }
    if (mpHorizontalScrollBar != 0)
    {
        mpHorizontalScrollBar->SetScrollHdl (Link());
    }
}




/** Placing the scroll bars is an iterative process.  The visibility of one
    scroll bar affects the remaining size and thus may lead to the other
    scroll bar becoming visible.

    First we determine the visibility of the horizontal scroll bar.  After
    that we do the same for the vertical scroll bar.  To have an initial
    value for the required size we call the layouter before that.  When one
    of the two scroll bars is made visible then the size of the browser
    window changes and a second call to the layouter becomes necessary.
    That call is made anyway after this method returns.
*/
Rectangle ScrollBarManager::PlaceScrollBars (
    const Rectangle& rAvailableArea,
    const bool bIsHorizontalScrollBarAllowed,
    const bool bIsVerticalScrollBarAllowed)
{
    Rectangle aRemainingSpace (DetermineScrollBarVisibilities(
        rAvailableArea,
        bIsHorizontalScrollBarAllowed,
        bIsVerticalScrollBarAllowed));

    if (mpHorizontalScrollBar!=0 && mpHorizontalScrollBar->IsVisible())
        PlaceHorizontalScrollBar (rAvailableArea);

    if (mpVerticalScrollBar!=0 && mpVerticalScrollBar->IsVisible())
        PlaceVerticalScrollBar (rAvailableArea);

    if (mpScrollBarFiller!=0 && mpScrollBarFiller->IsVisible())
        PlaceFiller (rAvailableArea);

    return aRemainingSpace;
}




void ScrollBarManager::PlaceHorizontalScrollBar (const Rectangle& aAvailableArea)
{
    
    mnHorizontalPosition = double(mpHorizontalScrollBar->GetThumbPos())
        / double(mpHorizontalScrollBar->GetRange().Len());

    
    Size aScrollBarSize (mpHorizontalScrollBar->GetSizePixel());
    mpHorizontalScrollBar->SetPosSizePixel (
        Point(aAvailableArea.Left(),
            aAvailableArea.Bottom()-aScrollBarSize.Height()+1),
        Size (aAvailableArea.GetWidth() - GetVerticalScrollBarWidth(),
            aScrollBarSize.Height()));

    
    mpHorizontalScrollBar->SetThumbPos(
        (long)(0.5 + mnHorizontalPosition * mpHorizontalScrollBar->GetRange().Len()));
}




void ScrollBarManager::PlaceVerticalScrollBar (const Rectangle& aArea)
{
    const sal_Int32 nThumbPosition (mpVerticalScrollBar->GetThumbPos());

    
    Size aScrollBarSize (mpVerticalScrollBar->GetSizePixel());
    Point aPosition (aArea.Right()-aScrollBarSize.Width()+1, aArea.Top());
    Size aSize (aScrollBarSize.Width(), aArea.GetHeight() - GetHorizontalScrollBarHeight());
    mpVerticalScrollBar->SetPosSizePixel(aPosition, aSize);

    
    mpVerticalScrollBar->SetThumbPos(static_cast<long>(nThumbPosition));
    mnVerticalPosition = nThumbPosition / double(mpVerticalScrollBar->GetRange().Len());
}




void ScrollBarManager::PlaceFiller (const Rectangle& aArea)
{
    mpScrollBarFiller->SetPosSizePixel(
        Point(
            aArea.Right()-mpVerticalScrollBar->GetSizePixel().Width()+1,
            aArea.Bottom()-mpHorizontalScrollBar->GetSizePixel().Height()+1),
        Size (
            mpVerticalScrollBar->GetSizePixel().Width(),
            mpHorizontalScrollBar->GetSizePixel().Height()));
}




void ScrollBarManager::UpdateScrollBars (bool bResetThumbPosition, bool bUseScrolling)
{
    Rectangle aModelArea (mrSlideSorter.GetView().GetModelArea());
    SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
    Size aWindowModelSize (pWindow->PixelToLogic(pWindow->GetSizePixel()));

    
    
    if (mpHorizontalScrollBar != 0 && mpHorizontalScrollBar->IsVisible())
    {
        mpHorizontalScrollBar->Show();
        mpHorizontalScrollBar->SetRange (
            Range(aModelArea.Left(), aModelArea.Right()));
        if (bResetThumbPosition)
        {
            mpHorizontalScrollBar->SetThumbPos (0);
            mnHorizontalPosition = 0;
        }
        else
            mnHorizontalPosition =
                double(mpHorizontalScrollBar->GetThumbPos())
                / double(mpHorizontalScrollBar->GetRange().Len());

        mpHorizontalScrollBar->SetVisibleSize (aWindowModelSize.Width());

        const long nWidth (mpContentWindow->PixelToLogic(
            mpContentWindow->GetSizePixel()).Width());
        
        mpHorizontalScrollBar->SetLineSize (nWidth / 10);
        
        mpHorizontalScrollBar->SetPageSize ((nWidth * 9) / 10);
    }
    else
    {
        mnHorizontalPosition = 0;
    }

    
    if (mpVerticalScrollBar != 0 && mpVerticalScrollBar->IsVisible())
    {
        mpVerticalScrollBar->SetRange (
            Range(aModelArea.Top(), aModelArea.Bottom()));
        if (bResetThumbPosition)
        {
            mpVerticalScrollBar->SetThumbPos (0);
            mnVerticalPosition = 0;
        }
        else
            mnVerticalPosition =
                double(mpVerticalScrollBar->GetThumbPos())
                / double(mpVerticalScrollBar->GetRange().Len());

        mpVerticalScrollBar->SetVisibleSize (aWindowModelSize.Height());

        const long nHeight (mpContentWindow->PixelToLogic(
            mpContentWindow->GetSizePixel()).Height());
        
        mpVerticalScrollBar->SetLineSize (nHeight / 10);
        
        mpVerticalScrollBar->SetPageSize ((nHeight * 9) / 10);
    }
    else
    {
        mnVerticalPosition = 0;
    }


    double nEps (::std::numeric_limits<double>::epsilon());
    if (fabs(mnHorizontalPosition-pWindow->GetVisibleX()) > nEps
        || fabs(mnVerticalPosition-pWindow->GetVisibleY()) > nEps)
    {
        mrSlideSorter.GetView().InvalidatePageObjectVisibilities();
        if (bUseScrolling)
            pWindow->SetVisibleXY(mnHorizontalPosition, mnVerticalPosition);
        else
            SetWindowOrigin(mnHorizontalPosition, mnVerticalPosition);
    }
}




IMPL_LINK(ScrollBarManager, VerticalScrollBarHandler, ScrollBar*, pScrollBar)
{
    if (pScrollBar!=NULL
        && pScrollBar==mpVerticalScrollBar.get()
        && pScrollBar->IsVisible()
        && mrSlideSorter.GetContentWindow()!=0)
    {
        double nRelativePosition = double(pScrollBar->GetThumbPos())
            / double(pScrollBar->GetRange().Len());
        mrSlideSorter.GetView().InvalidatePageObjectVisibilities();
        mrSlideSorter.GetContentWindow()->SetVisibleXY(-1, nRelativePosition);
        mrSlideSorter.GetController().GetVisibleAreaManager().DeactivateCurrentSlideTracking();
    }
    return sal_True;
}




IMPL_LINK(ScrollBarManager, HorizontalScrollBarHandler, ScrollBar*, pScrollBar)
{
    if (pScrollBar!=NULL
        && pScrollBar==mpHorizontalScrollBar.get()
        && pScrollBar->IsVisible()
        && mrSlideSorter.GetContentWindow()!=0)
    {
        double nRelativePosition = double(pScrollBar->GetThumbPos())
            / double(pScrollBar->GetRange().Len());
        mrSlideSorter.GetView().InvalidatePageObjectVisibilities();
        mrSlideSorter.GetContentWindow()->SetVisibleXY(nRelativePosition, -1);
        mrSlideSorter.GetController().GetVisibleAreaManager().DeactivateCurrentSlideTracking();
    }
    return sal_True;
}




void ScrollBarManager::SetWindowOrigin (
    double nHorizontalPosition,
    double nVerticalPosition)
{
    mnHorizontalPosition = nHorizontalPosition;
    mnVerticalPosition = nVerticalPosition;

    SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
    Size aViewSize (pWindow->GetViewSize());
    Point aOrigin (
        (long int) (mnHorizontalPosition * aViewSize.Width()),
        (long int) (mnVerticalPosition * aViewSize.Height()));

    pWindow->SetWinViewPos (aOrigin);
    pWindow->UpdateMapMode ();
    pWindow->Invalidate ();
}




/** Determining the visibility of the scroll bars is quite complicated.  The
    visibility of one influences that of the other because showing a scroll
    bar makes the available space smaller and may lead to the need of
    displaying the other.
    To solve this we test all four combinations of showing or hiding each
    scroll bar and use the best one.  The best one is that combination that
    a) shows the least number of scroll bars with preference of showing the
    vertical over showing the horizontal and
    b) when not showing a scroll bar the area used by the page objects fits
    into the available area in the scroll bars orientation.
*/
Rectangle ScrollBarManager::DetermineScrollBarVisibilities (
    const Rectangle& rAvailableArea,
    const bool bIsHorizontalScrollBarAllowed,
    const bool bIsVerticalScrollBarAllowed)
{
    
    bool bShowHorizontal = false;
    bool bShowVertical = false;
    if (mrSlideSorter.GetModel().GetPageCount() == 0)
    {
        
    }
    else if (TestScrollBarVisibilities(false, false, rAvailableArea))
    {
        
    }
    else if (bIsHorizontalScrollBarAllowed
        && TestScrollBarVisibilities(true, false, rAvailableArea))
    {
        bShowHorizontal = true;
    }
    else if (bIsVerticalScrollBarAllowed
        && TestScrollBarVisibilities(false, true, rAvailableArea))
    {
        bShowVertical = true;
    }
    else
    {
        bShowHorizontal = true;
        bShowVertical = true;
    }

    
    mpVerticalScrollBar->Show(bShowVertical);
    mpHorizontalScrollBar->Show(bShowHorizontal);
    mpScrollBarFiller->Show(bShowVertical && bShowHorizontal);

    
    Rectangle aRemainingSpace (rAvailableArea);
    if (bShowVertical)
        aRemainingSpace.Right() -= mpVerticalScrollBar->GetSizePixel().Width();
    if (bShowHorizontal)
        aRemainingSpace.Bottom() -= mpHorizontalScrollBar->GetSizePixel().Height();

    return aRemainingSpace;
}




bool ScrollBarManager::TestScrollBarVisibilities (
    bool bHorizontalScrollBarVisible,
    bool bVerticalScrollBarVisible,
    const Rectangle& rAvailableArea)
{
    model::SlideSorterModel& rModel (mrSlideSorter.GetModel());

    
    
    Size aBrowserSize (rAvailableArea.GetSize());
    if (bHorizontalScrollBarVisible)
        aBrowserSize.Height() -= mpHorizontalScrollBar->GetSizePixel().Height();
    if (bVerticalScrollBarVisible)
        aBrowserSize.Width() -= mpVerticalScrollBar->GetSizePixel().Width();

    
    
    bool bRearrangeSuccess (mrSlideSorter.GetView().GetLayouter().Rearrange (
        mrSlideSorter.GetView().GetOrientation(),
        aBrowserSize,
        rModel.GetPageDescriptor(0)->GetPage()->GetSize(),
        rModel.GetPageCount()));

    if (bRearrangeSuccess)
    {
        Size aPageSize = mrSlideSorter.GetView().GetLayouter().GetTotalBoundingBox().GetSize();
        Size aWindowModelSize = mpContentWindow->PixelToLogic(aBrowserSize);

        
        
        if (aPageSize.Width() > aWindowModelSize.Width())
            if ( ! bHorizontalScrollBarVisible)
                return false;
        if (aPageSize.Height() > aWindowModelSize.Height())
            if ( ! bVerticalScrollBarVisible)
                return false;

        return true;
    }
    else
        return false;
}




void ScrollBarManager::SetTopLeft (const Point aNewTopLeft)
{
    if (( ! mpVerticalScrollBar
            || mpVerticalScrollBar->GetThumbPos() == aNewTopLeft.Y())
        && ( ! mpHorizontalScrollBar
            || mpHorizontalScrollBar->GetThumbPos() == aNewTopLeft.X()))
        return;

    
    mrSlideSorter.GetContentWindow()->Update();

    if (mpVerticalScrollBar)
    {
        mpVerticalScrollBar->SetThumbPos(aNewTopLeft.Y());
        mnVerticalPosition = aNewTopLeft.Y() / double(mpVerticalScrollBar->GetRange().Len());
    }
    if (mpHorizontalScrollBar)
    {
        mpHorizontalScrollBar->SetThumbPos(aNewTopLeft.X());
        mnHorizontalPosition = aNewTopLeft.X() / double(mpHorizontalScrollBar->GetRange().Len());
    }

    mrSlideSorter.GetContentWindow()->SetVisibleXY(mnHorizontalPosition, mnVerticalPosition);
    mrSlideSorter.GetView().InvalidatePageObjectVisibilities();
}




int ScrollBarManager::GetVerticalScrollBarWidth (void) const
{
    if (mpVerticalScrollBar != 0 && mpVerticalScrollBar->IsVisible())
        return mpVerticalScrollBar->GetSizePixel().Width();
    else
        return 0;
}




int ScrollBarManager::GetHorizontalScrollBarHeight (void) const
{
    if (mpHorizontalScrollBar != 0 && mpHorizontalScrollBar->IsVisible())
        return mpHorizontalScrollBar->GetSizePixel().Height();
    else
        return 0;
}




void ScrollBarManager::CalcAutoScrollOffset (const Point& rMouseWindowPosition)
{
    SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());

    int nDx = 0;
    int nDy = 0;

    Size aWindowSize = pWindow->GetOutputSizePixel();
    Rectangle aWindowArea (pWindow->GetPosPixel(), aWindowSize);
    Rectangle aViewPixelArea (
        pWindow->LogicToPixel(mrSlideSorter.GetView().GetModelArea()));

    if (aWindowSize.Width() > maScrollBorder.Width() * 3
        && mpHorizontalScrollBar != 0
        && mpHorizontalScrollBar->IsVisible())
    {
        if (rMouseWindowPosition.X() < maScrollBorder.Width()
            && aWindowArea.Left() > aViewPixelArea.Left())
        {
            nDx = -1 + (int)(mnHorizontalScrollFactor
                * (rMouseWindowPosition.X() - maScrollBorder.Width()));
        }

        if (rMouseWindowPosition.X() >= (aWindowSize.Width() - maScrollBorder.Width())
            && aWindowArea.Right() < aViewPixelArea.Right())
        {
            nDx = 1 + (int)(mnHorizontalScrollFactor
                * (rMouseWindowPosition.X() - aWindowSize.Width()
                    + maScrollBorder.Width()));
        }
    }

    if (aWindowSize.Height() > maScrollBorder.Height() * 3
        && aWindowSize.Height() < aViewPixelArea.GetHeight())
    {
        if (rMouseWindowPosition.Y() < maScrollBorder.Height()
            && aWindowArea.Top() > aViewPixelArea.Top())
        {
            nDy = -1 + (int)(mnVerticalScrollFactor
                * (rMouseWindowPosition.Y() - maScrollBorder.Height()));
        }

        if (rMouseWindowPosition.Y() >= (aWindowSize.Height() - maScrollBorder.Height())
            && aWindowArea.Bottom() < aViewPixelArea.Bottom())
        {
            nDy = 1 + (int)(mnVerticalScrollFactor
                * (rMouseWindowPosition.Y() - aWindowSize.Height()
                    + maScrollBorder.Height()));
        }
    }

    maAutoScrollOffset = Size(nDx,nDy);
}




bool ScrollBarManager::AutoScroll (
    const Point& rMouseWindowPosition,
    const ::boost::function<void(void)>& rAutoScrollFunctor)
{
    maAutoScrollFunctor = rAutoScrollFunctor;
    CalcAutoScrollOffset(rMouseWindowPosition);
    bool bResult (true);
    if ( ! mbIsAutoScrollActive)
        bResult = RepeatAutoScroll();

    return bResult;
}




void ScrollBarManager::StopAutoScroll (void)
{
    maAutoScrollTimer.Stop();
    mbIsAutoScrollActive = false;
}




bool ScrollBarManager::RepeatAutoScroll (void)
{
    if (maAutoScrollOffset != Size(0,0))
    {
        if (mrSlideSorter.GetViewShell() != NULL)
        {
            mrSlideSorter.GetViewShell()->Scroll(
                maAutoScrollOffset.Width(),
                maAutoScrollOffset.Height());
            mrSlideSorter.GetView().InvalidatePageObjectVisibilities();

            if (maAutoScrollFunctor)
                maAutoScrollFunctor();

            mbIsAutoScrollActive = true;
            maAutoScrollTimer.Start();

            return true;
        }
    }

    clearAutoScrollFunctor();
    mbIsAutoScrollActive = false;
    return false;
}

void ScrollBarManager::clearAutoScrollFunctor()
{
    maAutoScrollFunctor = ::boost::function<void(void)>();
}

IMPL_LINK_NOARG(ScrollBarManager, AutoScrollTimeoutHandler)
{
    RepeatAutoScroll();

    return 0;
}




void ScrollBarManager::Scroll(
    const Orientation eOrientation,
    const Unit eUnit,
    const sal_Int32 nDistance)
{
    bool bIsVertical (false);
    switch (eOrientation)
    {
        case Orientation_Horizontal: bIsVertical = false; break;
        case Orientation_Vertical: bIsVertical = true; break;
        default:
            OSL_ASSERT(eOrientation==Orientation_Horizontal || eOrientation==Orientation_Vertical);
            return;
    }

    Point aNewTopLeft (
        mpHorizontalScrollBar ? mpHorizontalScrollBar->GetThumbPos() : 0,
        mpVerticalScrollBar ? mpVerticalScrollBar->GetThumbPos() : 0);
    switch (eUnit)
    {
        case Unit_Pixel:
            if (bIsVertical)
                aNewTopLeft.Y() += nDistance;
            else
                aNewTopLeft.X() += nDistance;
            break;

        case Unit_Slide:
        {
            view::Layouter& rLayouter (mrSlideSorter.GetView().GetLayouter());

            
            if (bIsVertical)
                aNewTopLeft.Y() += nDistance * rLayouter.GetPageObjectSize().Height();
            else
                aNewTopLeft.X() += nDistance * rLayouter.GetPageObjectSize().Width();

            
            if (bIsVertical)
                if (nDistance > 0)
                {
                    const sal_Int32 nIndex (rLayouter.GetIndexAtPoint(
                        Point(aNewTopLeft.X(), aNewTopLeft.Y()+mpVerticalScrollBar->GetVisibleSize()),
                        true));
                    aNewTopLeft.Y() = rLayouter.GetPageObjectBox(nIndex,true).Bottom()
                        - mpVerticalScrollBar->GetVisibleSize();
                }
                else
                {
                    const sal_Int32 nIndex (rLayouter.GetIndexAtPoint(
                        Point(aNewTopLeft.X(), aNewTopLeft.Y()),
                        true));
                    aNewTopLeft.Y() = rLayouter.GetPageObjectBox(nIndex,true).Top();
                }
            else
                if (nDistance > 0)
                {
                    const sal_Int32 nIndex (rLayouter.GetIndexAtPoint(
                        Point(aNewTopLeft.X()+mpVerticalScrollBar->GetVisibleSize(), aNewTopLeft.Y()),
                        true));
                    aNewTopLeft.X() = rLayouter.GetPageObjectBox(nIndex,true).Right()
                        - mpVerticalScrollBar->GetVisibleSize();
                }
                else
                {
                    const sal_Int32 nIndex (rLayouter.GetIndexAtPoint(
                        Point(aNewTopLeft.X(), aNewTopLeft.Y()),
                            true));
                    aNewTopLeft.X() = rLayouter.GetPageObjectBox(nIndex,true).Left();
                }
        }
    }
    mrSlideSorter.GetController().GetVisibleAreaManager().DeactivateCurrentSlideTracking();
    SetTopLeft(aNewTopLeft);
}


} } } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
