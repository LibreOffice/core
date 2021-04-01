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

#include <controller/SlsScrollBarManager.hxx>

#include <SlideSorter.hxx>
#include <ViewShell.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsVisibleAreaManager.hxx>
#include <model/SlideSorterModel.hxx>
#include <model/SlsPageDescriptor.hxx>
#include <view/SlideSorterView.hxx>
#include <view/SlsLayouter.hxx>
#include <Window.hxx>
#include <sdpage.hxx>
#include <osl/diagnose.h>

#include <vcl/scrbar.hxx>

namespace sd::slidesorter::controller {

constexpr double gnHorizontalScrollFactor(0.15);
constexpr double gnVerticalScrollFactor(0.25);

ScrollBarManager::ScrollBarManager (SlideSorter& rSlideSorter)
    : mrSlideSorter(rSlideSorter),
      mpHorizontalScrollBar(mrSlideSorter.GetHorizontalScrollBar()),
      mpVerticalScrollBar(mrSlideSorter.GetVerticalScrollBar()),
      mnHorizontalPosition (0),
      mnVerticalPosition (0),
      maScrollBorder (20,20),
      mpScrollBarFiller(mrSlideSorter.GetScrollBarFiller()),
      maAutoScrollTimer(),
      maAutoScrollOffset(0,0),
      mbIsAutoScrollActive(false),
      mpContentWindow(mrSlideSorter.GetContentWindow()),
      maAutoScrollFunctor()
{
    // Hide the scroll bars by default to prevent display errors while
    // switching between view shells:  In the short time between initiating
    // such a switch and the final rearrangement of UI controls the scroll
    // bars and the filler where displayed in the upper left corner of the
    // ViewTabBar.
    mpHorizontalScrollBar->Hide();
    mpVerticalScrollBar->Hide();
    mpScrollBarFiller->Hide();

    maAutoScrollTimer.SetTimeout(25);
    maAutoScrollTimer.SetInvokeHandler (
        LINK(this, ScrollBarManager, AutoScrollTimeoutHandler));
}

ScrollBarManager::~ScrollBarManager()
{
}

void ScrollBarManager::Connect()
{
    if (mpVerticalScrollBar != nullptr)
    {
        mpVerticalScrollBar->SetScrollHdl (
            LINK(this, ScrollBarManager, VerticalScrollBarHandler));
    }
    if (mpHorizontalScrollBar != nullptr)
    {
        mpHorizontalScrollBar->SetScrollHdl(
            LINK(this, ScrollBarManager, HorizontalScrollBarHandler));
    }
}

void ScrollBarManager::Disconnect()
{
    if (mpVerticalScrollBar != nullptr)
    {
        mpVerticalScrollBar->SetScrollHdl( Link<ScrollBar*,void>() );
    }
    if (mpHorizontalScrollBar != nullptr)
    {
        mpHorizontalScrollBar->SetScrollHdl( Link<ScrollBar*,void>() );
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
::tools::Rectangle ScrollBarManager::PlaceScrollBars (
    const ::tools::Rectangle& rAvailableArea,
    const bool bIsHorizontalScrollBarAllowed,
    const bool bIsVerticalScrollBarAllowed)
{
    ::tools::Rectangle aRemainingSpace (DetermineScrollBarVisibilities(
        rAvailableArea,
        bIsHorizontalScrollBarAllowed,
        bIsVerticalScrollBarAllowed));

    if (mpHorizontalScrollBar!=nullptr && mpHorizontalScrollBar->IsVisible())
        PlaceHorizontalScrollBar (rAvailableArea);

    if (mpVerticalScrollBar!=nullptr && mpVerticalScrollBar->IsVisible())
        PlaceVerticalScrollBar (rAvailableArea);

    if (mpScrollBarFiller!=nullptr && mpScrollBarFiller->IsVisible())
        PlaceFiller (rAvailableArea);

    return aRemainingSpace;
}

void ScrollBarManager::PlaceHorizontalScrollBar (const ::tools::Rectangle& aAvailableArea)
{
    // Save the current relative position.
    mnHorizontalPosition = double(mpHorizontalScrollBar->GetThumbPos())
        / double(mpHorizontalScrollBar->GetRange().Len());

    // Place the scroll bar.
    Size aScrollBarSize (mpHorizontalScrollBar->GetSizePixel());
    mpHorizontalScrollBar->SetPosSizePixel (
        Point(aAvailableArea.Left(),
            aAvailableArea.Bottom()-aScrollBarSize.Height()+1),
        Size (aAvailableArea.GetWidth() - GetVerticalScrollBarWidth(),
            aScrollBarSize.Height()));

    // Restore the relative position.
    mpHorizontalScrollBar->SetThumbPos(
        static_cast<::tools::Long>(0.5 + mnHorizontalPosition * mpHorizontalScrollBar->GetRange().Len()));
}

void ScrollBarManager::PlaceVerticalScrollBar (const ::tools::Rectangle& aArea)
{
    const sal_Int32 nThumbPosition (mpVerticalScrollBar->GetThumbPos());

    // Place the scroll bar.
    Size aScrollBarSize (mpVerticalScrollBar->GetSizePixel());
    Point aPosition (aArea.Right()-aScrollBarSize.Width()+1, aArea.Top());
    Size aSize (aScrollBarSize.Width(), aArea.GetHeight() - GetHorizontalScrollBarHeight());
    mpVerticalScrollBar->SetPosSizePixel(aPosition, aSize);

    // Restore the position.
    mpVerticalScrollBar->SetThumbPos(static_cast<::tools::Long>(nThumbPosition));
    mnVerticalPosition = nThumbPosition / double(mpVerticalScrollBar->GetRange().Len());
}

void ScrollBarManager::PlaceFiller (const ::tools::Rectangle& aArea)
{
    mpScrollBarFiller->SetPosSizePixel(
        Point(
            aArea.Right()-mpVerticalScrollBar->GetSizePixel().Width()+1,
            aArea.Bottom()-mpHorizontalScrollBar->GetSizePixel().Height()+1),
        Size (
            mpVerticalScrollBar->GetSizePixel().Width(),
            mpHorizontalScrollBar->GetSizePixel().Height()));
}

void ScrollBarManager::UpdateScrollBars(bool bUseScrolling)
{
    ::tools::Rectangle aModelArea (mrSlideSorter.GetView().GetModelArea());
    sd::Window *pWindow (mrSlideSorter.GetContentWindow().get());
    Size aWindowModelSize (pWindow->PixelToLogic(pWindow->GetSizePixel()));

    // The horizontal scroll bar is only shown when the window is
    // horizontally smaller than the view.
    if (mpHorizontalScrollBar != nullptr && mpHorizontalScrollBar->IsVisible())
    {
        mpHorizontalScrollBar->Show();
        mpHorizontalScrollBar->SetRange (
            Range(aModelArea.Left(), aModelArea.Right()));
        mnHorizontalPosition =
                double(mpHorizontalScrollBar->GetThumbPos())
                / double(mpHorizontalScrollBar->GetRange().Len());

        mpHorizontalScrollBar->SetVisibleSize (aWindowModelSize.Width());

        const ::tools::Long nWidth (mpContentWindow->PixelToLogic(
            mpContentWindow->GetSizePixel()).Width());
        // Make the line size about 10% of the visible width.
        mpHorizontalScrollBar->SetLineSize (nWidth / 10);
        // Make the page size about 90% of the visible width.
        mpHorizontalScrollBar->SetPageSize ((nWidth * 9) / 10);
    }
    else
    {
        mnHorizontalPosition = 0;
    }

    // The vertical scroll bar is always shown.
    if (mpVerticalScrollBar != nullptr && mpVerticalScrollBar->IsVisible())
    {
        mpVerticalScrollBar->SetRange (
            Range(aModelArea.Top(), aModelArea.Bottom()));
        mnVerticalPosition =
                double(mpVerticalScrollBar->GetThumbPos())
                / double(mpVerticalScrollBar->GetRange().Len());

        mpVerticalScrollBar->SetVisibleSize (aWindowModelSize.Height());

        const ::tools::Long nHeight (mpContentWindow->PixelToLogic(
            mpContentWindow->GetSizePixel()).Height());
        // Make the line size about 10% of the visible height.
        mpVerticalScrollBar->SetLineSize (nHeight / 10);
        // Make the page size about 90% of the visible height.
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

IMPL_LINK(ScrollBarManager, VerticalScrollBarHandler, ScrollBar*, pScrollBar, void)
{
    if (pScrollBar!=nullptr
        && pScrollBar==mpVerticalScrollBar.get()
        && pScrollBar->IsVisible()
        && mrSlideSorter.GetContentWindow())
    {
        double nRelativePosition = double(pScrollBar->GetThumbPos())
            / double(pScrollBar->GetRange().Len());
        mrSlideSorter.GetView().InvalidatePageObjectVisibilities();
        mrSlideSorter.GetContentWindow()->SetVisibleXY(-1, nRelativePosition);
        mrSlideSorter.GetController().GetVisibleAreaManager().DeactivateCurrentSlideTracking();
    }
}

IMPL_LINK(ScrollBarManager, HorizontalScrollBarHandler, ScrollBar*, pScrollBar, void)
{
    if (pScrollBar!=nullptr
        && pScrollBar==mpHorizontalScrollBar.get()
        && pScrollBar->IsVisible()
        && mrSlideSorter.GetContentWindow())
    {
        double nRelativePosition = double(pScrollBar->GetThumbPos())
            / double(pScrollBar->GetRange().Len());
        mrSlideSorter.GetView().InvalidatePageObjectVisibilities();
        mrSlideSorter.GetContentWindow()->SetVisibleXY(nRelativePosition, -1);
        mrSlideSorter.GetController().GetVisibleAreaManager().DeactivateCurrentSlideTracking();
    }
}

void ScrollBarManager::SetWindowOrigin (
    double nHorizontalPosition,
    double nVerticalPosition)
{
    mnHorizontalPosition = nHorizontalPosition;
    mnVerticalPosition = nVerticalPosition;

    sd::Window *pWindow (mrSlideSorter.GetContentWindow().get());
    Size aViewSize (pWindow->GetViewSize());
    Point aOrigin (
        static_cast<::tools::Long>(mnHorizontalPosition * aViewSize.Width()),
        static_cast<::tools::Long>(mnVerticalPosition * aViewSize.Height()));

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
::tools::Rectangle ScrollBarManager::DetermineScrollBarVisibilities (
    const ::tools::Rectangle& rAvailableArea,
    const bool bIsHorizontalScrollBarAllowed,
    const bool bIsVerticalScrollBarAllowed)
{
    // Test which combination of scroll bars is the best.
    bool bShowHorizontal = false;
    bool bShowVertical = false;
    if (mrSlideSorter.GetModel().GetPageCount() == 0)
    {
        // No pages => no scroll bars.
    }
    else if (TestScrollBarVisibilities(false, false, rAvailableArea))
    {
        // Nothing to be done.
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

    // Make the visibility of the scroll bars permanent.
    mpVerticalScrollBar->Show(bShowVertical);
    mpHorizontalScrollBar->Show(bShowHorizontal);
    mpScrollBarFiller->Show(bShowVertical && bShowHorizontal);

    // Adapt the remaining space accordingly.
    ::tools::Rectangle aRemainingSpace (rAvailableArea);
    if (bShowVertical)
        aRemainingSpace.AdjustRight( -(mpVerticalScrollBar->GetSizePixel().Width()) );
    if (bShowHorizontal)
        aRemainingSpace.AdjustBottom( -(mpHorizontalScrollBar->GetSizePixel().Height()) );

    return aRemainingSpace;
}

bool ScrollBarManager::TestScrollBarVisibilities (
    bool bHorizontalScrollBarVisible,
    bool bVerticalScrollBarVisible,
    const ::tools::Rectangle& rAvailableArea)
{
    model::SlideSorterModel& rModel (mrSlideSorter.GetModel());

    // Adapt the available size by subtracting the sizes of the scroll bars
    // visible in this combination.
    Size aBrowserSize (rAvailableArea.GetSize());
    if (bHorizontalScrollBarVisible)
        aBrowserSize.AdjustHeight( -(mpHorizontalScrollBar->GetSizePixel().Height()) );
    if (bVerticalScrollBarVisible)
        aBrowserSize.AdjustWidth( -(mpVerticalScrollBar->GetSizePixel().Width()) );

    // Tell the view to rearrange its page objects and check whether the
    // page objects can be shown without clipping.
    bool bRearrangeSuccess (mrSlideSorter.GetView().GetLayouter().Rearrange (
        mrSlideSorter.GetView().GetOrientation(),
        aBrowserSize,
        rModel.GetPageDescriptor(0)->GetPage()->GetSize(),
        rModel.GetPageCount()));

    if (bRearrangeSuccess)
    {
        Size aPageSize = mrSlideSorter.GetView().GetLayouter().GetTotalBoundingBox().GetSize();
        Size aWindowModelSize = mpContentWindow->PixelToLogic(aBrowserSize);

        // The content may be clipped, i.e. not fully visible, in one
        // direction only when the scroll bar is visible in that direction.
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

void ScrollBarManager::SetTopLeft(const Point& rNewTopLeft)
{
    if (( ! mpVerticalScrollBar
            || mpVerticalScrollBar->GetThumbPos() == rNewTopLeft.Y())
        && ( ! mpHorizontalScrollBar
            || mpHorizontalScrollBar->GetThumbPos() == rNewTopLeft.X()))
        return;

    // Flush pending repaints before scrolling to avoid temporary artifacts.
    mrSlideSorter.GetContentWindow()->PaintImmediately();

    if (mpVerticalScrollBar)
    {
        mpVerticalScrollBar->SetThumbPos(rNewTopLeft.Y());
        mnVerticalPosition = rNewTopLeft.Y() / double(mpVerticalScrollBar->GetRange().Len());
    }
    if (mpHorizontalScrollBar)
    {
        mpHorizontalScrollBar->SetThumbPos(rNewTopLeft.X());
        mnHorizontalPosition = rNewTopLeft.X() / double(mpHorizontalScrollBar->GetRange().Len());
    }

    mrSlideSorter.GetContentWindow()->SetVisibleXY(mnHorizontalPosition, mnVerticalPosition);
    mrSlideSorter.GetView().InvalidatePageObjectVisibilities();
}

int ScrollBarManager::GetVerticalScrollBarWidth() const
{
    if (mpVerticalScrollBar != nullptr && mpVerticalScrollBar->IsVisible())
        return mpVerticalScrollBar->GetSizePixel().Width();
    else
        return 0;
}

int ScrollBarManager::GetHorizontalScrollBarHeight() const
{
    if (mpHorizontalScrollBar != nullptr && mpHorizontalScrollBar->IsVisible())
        return mpHorizontalScrollBar->GetSizePixel().Height();
    else
        return 0;
}

void ScrollBarManager::CalcAutoScrollOffset (const Point& rMouseWindowPosition)
{
    sd::Window *pWindow (mrSlideSorter.GetContentWindow().get());

    int nDx = 0;
    int nDy = 0;

    Size aWindowSize = pWindow->GetOutputSizePixel();
    ::tools::Rectangle aWindowArea (pWindow->GetPosPixel(), aWindowSize);
    ::tools::Rectangle aViewPixelArea (
        pWindow->LogicToPixel(mrSlideSorter.GetView().GetModelArea()));

    if (aWindowSize.Width() > maScrollBorder.Width() * 3
        && mpHorizontalScrollBar != nullptr
        && mpHorizontalScrollBar->IsVisible())
    {
        if (rMouseWindowPosition.X() < maScrollBorder.Width()
            && aWindowArea.Left() > aViewPixelArea.Left())
        {
            nDx = -1 + static_cast<int>(gnHorizontalScrollFactor
                * (rMouseWindowPosition.X() - maScrollBorder.Width()));
        }

        if (rMouseWindowPosition.X() >= (aWindowSize.Width() - maScrollBorder.Width())
            && aWindowArea.Right() < aViewPixelArea.Right())
        {
            nDx = 1 + static_cast<int>(gnHorizontalScrollFactor
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
            nDy = -1 + static_cast<int>(gnVerticalScrollFactor
                * (rMouseWindowPosition.Y() - maScrollBorder.Height()));
        }

        if (rMouseWindowPosition.Y() >= (aWindowSize.Height() - maScrollBorder.Height())
            && aWindowArea.Bottom() < aViewPixelArea.Bottom())
        {
            nDy = 1 + static_cast<int>(gnVerticalScrollFactor
                * (rMouseWindowPosition.Y() - aWindowSize.Height()
                    + maScrollBorder.Height()));
        }
    }

    maAutoScrollOffset = Size(nDx,nDy);
}

bool ScrollBarManager::AutoScroll (
    const Point& rMouseWindowPosition,
    const ::std::function<void ()>& rAutoScrollFunctor)
{
    maAutoScrollFunctor = rAutoScrollFunctor;
    CalcAutoScrollOffset(rMouseWindowPosition);
    bool bResult (true);
    if ( ! mbIsAutoScrollActive)
        bResult = RepeatAutoScroll();

    return bResult;
}

void ScrollBarManager::StopAutoScroll()
{
    maAutoScrollTimer.Stop();
    mbIsAutoScrollActive = false;
}

bool ScrollBarManager::RepeatAutoScroll()
{
    if (maAutoScrollOffset != Size(0,0))
    {
        if (mrSlideSorter.GetViewShell() != nullptr)
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
    maAutoScrollFunctor = ::std::function<void ()>();
}

IMPL_LINK_NOARG(ScrollBarManager, AutoScrollTimeoutHandler, Timer *, void)
{
    RepeatAutoScroll();
}

void ScrollBarManager::Scroll(
    const Orientation eOrientation,
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

    view::Layouter& rLayouter (mrSlideSorter.GetView().GetLayouter());

    // Calculate estimate of new location.
    if (bIsVertical)
        aNewTopLeft.AdjustY(nDistance * rLayouter.GetPageObjectSize().Height() );
    else
        aNewTopLeft.AdjustX(nDistance * rLayouter.GetPageObjectSize().Width() );

    // Adapt location to show whole slides.
    if (bIsVertical)
        if (nDistance > 0)
        {
            const sal_Int32 nIndex (rLayouter.GetIndexAtPoint(
                Point(aNewTopLeft.X(), aNewTopLeft.Y()+mpVerticalScrollBar->GetVisibleSize()),
                true));
            aNewTopLeft.setY( rLayouter.GetPageObjectBox(nIndex,true).Bottom()
                - mpVerticalScrollBar->GetVisibleSize() );
        }
        else
        {
            const sal_Int32 nIndex (rLayouter.GetIndexAtPoint(
                Point(aNewTopLeft.X(), aNewTopLeft.Y()),
                true));
            aNewTopLeft.setY( rLayouter.GetPageObjectBox(nIndex,true).Top() );
        }
    else
        if (nDistance > 0)
        {
            const sal_Int32 nIndex (rLayouter.GetIndexAtPoint(
                Point(aNewTopLeft.X()+mpVerticalScrollBar->GetVisibleSize(), aNewTopLeft.Y()),
                true));
            aNewTopLeft.setX( rLayouter.GetPageObjectBox(nIndex,true).Right()
                - mpVerticalScrollBar->GetVisibleSize() );
        }
        else
        {
            const sal_Int32 nIndex (rLayouter.GetIndexAtPoint(
                Point(aNewTopLeft.X(), aNewTopLeft.Y()),
                    true));
            aNewTopLeft.setX( rLayouter.GetPageObjectBox(nIndex,true).Left() );
        }

    mrSlideSorter.GetController().GetVisibleAreaManager().DeactivateCurrentSlideTracking();
    SetTopLeft(aNewTopLeft);
}

} // end of namespace ::sd::slidesorter::controller

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
