/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlsScrollBarManager.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:27:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "precompiled_sd.hxx"

#include "controller/SlsScrollBarManager.hxx"

#include "SlideSorter.hxx"
#include "controller/SlideSorterController.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsViewOverlay.hxx"
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
      maScrollBorder (10,10),
      mnHorizontalScrollFactor (0.1),
      mnVerticalScrollFactor (0.1),
      mpScrollBarFiller(mrSlideSorter.GetScrollBarFiller()),
      mpContentWindow(mrSlideSorter.GetContentWindow())
{
    // Hide the scroll bars by default to prevent display errors while
    // switching between view shells:  In the short time between initiating
    // such a switch and the final rearrangement of UI controls the scroll
    // bars and the filler where displayed in the upper left corner of the
    // ViewTabBar.
    mpHorizontalScrollBar->Hide();
    mpVerticalScrollBar->Hide();
    mpScrollBarFiller->Hide();

    maAutoScrollTimer.SetTimeout(50);
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
    if (mpVerticalScrollBar != NULL)
        mpVerticalScrollBar->SetScrollHdl (
            LINK(
                this,
                ScrollBarManager,
                VerticalScrollBarHandler));
    if (mpHorizontalScrollBar != NULL)
        mpHorizontalScrollBar->SetScrollHdl (
            LINK(
                this,
                ScrollBarManager,
                HorizontalScrollBarHandler));
}




void ScrollBarManager::Disconnect (void)
{
    if (mpVerticalScrollBar != NULL)
        mpVerticalScrollBar->SetScrollHdl (Link());
    if (mpHorizontalScrollBar != NULL)
        mpHorizontalScrollBar->SetScrollHdl (Link());
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
Rectangle ScrollBarManager::PlaceScrollBars (const Rectangle& rAvailableArea)
{
    Rectangle aRemainingSpace (DetermineScrollBarVisibilities(rAvailableArea));
    PlaceHorizontalScrollBar (rAvailableArea);
    PlaceVerticalScrollBar (rAvailableArea);
    PlaceFiller (rAvailableArea);

    return aRemainingSpace;
}




void ScrollBarManager::PlaceHorizontalScrollBar (const Rectangle& aAvailableArea)
{
    if (mpHorizontalScrollBar != NULL
        && mpHorizontalScrollBar->IsVisible())
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
            (long)(0.5 + mnHorizontalPosition * mpHorizontalScrollBar->GetRange().Len()));
    }
}




void ScrollBarManager::PlaceVerticalScrollBar (const Rectangle& aArea)
{
    if (mpVerticalScrollBar != NULL
        && mpVerticalScrollBar->IsVisible())
    {
        view::Layouter::DoublePoint aLayouterPosition
            = mrSlideSorter.GetView().GetLayouter().ConvertModelToLayouterCoordinates (
                Point (0, mpVerticalScrollBar->GetThumbPos()));

        // Place the scroll bar.
        Size aScrollBarSize (mpVerticalScrollBar->GetSizePixel());
        Point aPosition (aArea.Right()-aScrollBarSize.Width()+1, aArea.Top());
        Size aSize (aScrollBarSize.Width(), aArea.GetHeight() - GetHorizontalScrollBarHeight());
        mpVerticalScrollBar->SetPosSizePixel(aPosition, aSize);

        // Restore the position.
        mpVerticalScrollBar->SetThumbPos(
            mrSlideSorter.GetView().GetLayouter().ConvertLayouterToModelCoordinates(
                aLayouterPosition).Y());
        mnVerticalPosition = double(mpVerticalScrollBar->GetThumbPos())
            / double(mpVerticalScrollBar->GetRange().Len());
    }
}




void ScrollBarManager::PlaceFiller (const Rectangle& aArea)
{
    // Place the filler when both scroll bars are visible.
    if (mpHorizontalScrollBar != NULL
        && mpVerticalScrollBar != NULL
        && mpHorizontalScrollBar->IsVisible()
        && mpVerticalScrollBar->IsVisible())
    {
        mpScrollBarFiller->SetPosSizePixel(
            Point(
                aArea.Right()-mpVerticalScrollBar->GetSizePixel().Width()+1,
                aArea.Bottom()-mpHorizontalScrollBar->GetSizePixel().Height()+1),
            Size (
                mpVerticalScrollBar->GetSizePixel().Width(),
                mpHorizontalScrollBar->GetSizePixel().Height()));
        mpScrollBarFiller->Show();
    }
    else
        mpScrollBarFiller->Hide();
}




void ScrollBarManager::AdaptWindowSize (const Rectangle& rArea)
{
    Size aPixelContentSize (mpContentWindow->LogicToPixel(
        mrSlideSorter.GetView().GetLayouter().GetPageBox (
            mrSlideSorter.GetModel().GetPageCount()).GetSize()));
    int nHeightDifference = aPixelContentSize.Height() - rArea.GetHeight();
    ::Window* pParentWindow = mpContentWindow->GetParent();
    Size aNewWindowSize (pParentWindow->GetSizePixel());
    if (nHeightDifference != 0)
    {
        aNewWindowSize.Height() += nHeightDifference;
        pParentWindow->SetPosSizePixel(
            pParentWindow->GetPosPixel(),
            aNewWindowSize);
    }
}




void ScrollBarManager::UpdateScrollBars (bool bResetThumbPosition, bool bUseScrolling)
{
    Rectangle aModelArea (mrSlideSorter.GetView().GetModelArea());
    ::sd::Window* pWindow = mrSlideSorter.GetView().GetWindow();
    Size aWindowModelSize (pWindow->PixelToLogic(pWindow->GetSizePixel()));

    // The horizontal scroll bar is only shown when the window is
    // horizontally smaller than the view.
    if (mpHorizontalScrollBar != NULL && mpHorizontalScrollBar->IsVisible())
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
    if (mpVerticalScrollBar != NULL && mpVerticalScrollBar->IsVisible())
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




IMPL_LINK(ScrollBarManager, VerticalScrollBarHandler, ScrollBar*, pScrollBar)
{
    if (pScrollBar!=NULL
        && pScrollBar==mpVerticalScrollBar.get()
        && pScrollBar->IsVisible()
        && mrSlideSorter.GetView().GetWindow()!=NULL)
    {
        double nRelativePosition = double(pScrollBar->GetThumbPos())
            / double(pScrollBar->GetRange().Len());
        mrSlideSorter.GetView().InvalidatePageObjectVisibilities();
        mrSlideSorter.GetView().GetWindow()->SetVisibleXY (
            -1,
            nRelativePosition);
    }
    return TRUE;
}




IMPL_LINK(ScrollBarManager, HorizontalScrollBarHandler, ScrollBar*, pScrollBar)
{
    if (pScrollBar!=NULL
        && pScrollBar==mpHorizontalScrollBar.get()
        && pScrollBar->IsVisible()
        && mrSlideSorter.GetView().GetWindow()!=NULL)
    {
        double nRelativePosition = double(pScrollBar->GetThumbPos())
            / double(pScrollBar->GetRange().Len());
        mrSlideSorter.GetView().InvalidatePageObjectVisibilities();
        mrSlideSorter.GetView().GetWindow()->SetVisibleXY (nRelativePosition, -1);
    }
    return TRUE;
}




void ScrollBarManager::SetWindowOrigin (
    double nHorizontalPosition,
    double nVerticalPosition)
{
    mnHorizontalPosition = nHorizontalPosition;
    mnVerticalPosition = nVerticalPosition;

    ::sd::Window* pWindow = mrSlideSorter.GetView().GetWindow();
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
Rectangle ScrollBarManager::DetermineScrollBarVisibilities (const Rectangle& rAvailableArea)
{
    // Test which combination of scroll bars is the best.
    bool bShowHorizontal = false;
    bool bShowVertical = false;
    do
    {
        if (mrSlideSorter.GetModel().GetPageCount() == 0)
            // No pages => no scroll bars.
            break;

        if (TestScrollBarVisibilities(bShowHorizontal=false, bShowVertical=false, rAvailableArea))
            break;
        if (TestScrollBarVisibilities(bShowHorizontal=true, bShowVertical=false, rAvailableArea))
            break;
        if (TestScrollBarVisibilities(bShowHorizontal=false, bShowVertical=true, rAvailableArea))
            break;
        if (TestScrollBarVisibilities(bShowHorizontal=true, bShowVertical=true, rAvailableArea))
            break;
    }
    while (false);

    // Make the visibility of the scroll bars permanent.
    mpVerticalScrollBar->Show(bShowVertical);
    mpHorizontalScrollBar->Show(bShowHorizontal);

    // Adapt the remaining space accordingly.
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
    bool bAreVisibilitiesOK = true;

    // Adapt the available size by subtracting the sizes of the scroll bars
    // visible in this combination.
    Size aBrowserSize (rAvailableArea.GetSize());
    if (bHorizontalScrollBarVisible)
        aBrowserSize.Height() -= mpHorizontalScrollBar->GetSizePixel().Height();
    if (bVerticalScrollBarVisible)
        aBrowserSize.Width() -= mpVerticalScrollBar->GetSizePixel().Width();

    // Tell the view to rearrange its page objects and check whether the
    // page objects can be shown without clipping.
    bool bRearrangeSuccess (false);
    if (mrSlideSorter.GetView().GetOrientation() == view::SlideSorterView::HORIZONTAL)
    {
        bRearrangeSuccess = mrSlideSorter.GetView().GetLayouter().RearrangeHorizontal (
            aBrowserSize,
            mrSlideSorter.GetModel().GetPageDescriptor(0)->GetPage()->GetSize(),
            mpContentWindow.get(),
            mrSlideSorter.GetModel().GetPageCount());
    }
    else
    {
        bRearrangeSuccess =  mrSlideSorter.GetView().GetLayouter().RearrangeVertical (
            aBrowserSize,
            mrSlideSorter.GetModel().GetPageDescriptor(0)->GetPage()->GetSize(),
            mpContentWindow.get());
    }

    if (bRearrangeSuccess)
    {
        Size aPageSize = mrSlideSorter.GetView().GetLayouter().GetPageBox (
            mrSlideSorter.GetModel().GetPageCount()).GetSize();
        Size aWindowModelSize = mpContentWindow->PixelToLogic(aBrowserSize);

        bool bHorizontallyClipped = (aPageSize.Width() > aWindowModelSize.Width());
        bool bVerticallyClipped = (aPageSize.Height() > aWindowModelSize.Height());
        bAreVisibilitiesOK = (bHorizontallyClipped == bHorizontalScrollBarVisible)
            && (bVerticallyClipped == bVerticalScrollBarVisible);
    }
    else
        bAreVisibilitiesOK = false;

    return bAreVisibilitiesOK;
}




void ScrollBarManager::SetTop (const sal_Int32 nNewTop)
{
    if (mpVerticalScrollBar != NULL
        && mpVerticalScrollBar->GetThumbPos() != nNewTop)
    {
        // Flush pending repaints before scrolling to avoid temporary artifacts.
        mrSlideSorter.GetView().GetWindow()->Update();

        mpVerticalScrollBar->SetThumbPos(nNewTop);
        mnVerticalPosition = double(nNewTop) / double(mpVerticalScrollBar->GetRange().Len());
        mrSlideSorter.GetView().GetWindow()->SetVisibleXY (
            mnHorizontalPosition, mnVerticalPosition);
    }
}




void ScrollBarManager::SetLeft (const sal_Int32 nNewLeft)
{
    if (mpHorizontalScrollBar != NULL
        && mpHorizontalScrollBar->GetThumbPos() != nNewLeft)
    {
        // Flush pending repaints before scrolling to avoid temporary artifacts.
        mrSlideSorter.GetView().GetWindow()->Update();

        mpHorizontalScrollBar->SetThumbPos(nNewLeft);
        mnHorizontalPosition = double(nNewLeft) / double(mpHorizontalScrollBar->GetRange().Len());
        mrSlideSorter.GetView().GetWindow()->SetVisibleXY (
            mnHorizontalPosition, mnVerticalPosition);
    }
}




int ScrollBarManager::GetVerticalScrollBarWidth (void) const
{
    if (mpVerticalScrollBar != NULL && mpVerticalScrollBar->IsVisible())
        return mpVerticalScrollBar->GetSizePixel().Width();
    else
        return 0;
}




int ScrollBarManager::GetHorizontalScrollBarHeight (void) const
{
    if (mpHorizontalScrollBar != NULL && mpHorizontalScrollBar->IsVisible())
        return mpHorizontalScrollBar->GetSizePixel().Height();
    else
        return 0;
}




void ScrollBarManager::CalcAutoScrollOffset (const Point& rMouseWindowPosition)
{
    ::sd::Window* pWindow = mrSlideSorter.GetView().GetWindow();

    int nDx = 0;
    int nDy = 0;

    Size aWindowSize = pWindow->GetOutputSizePixel();
    Rectangle aWindowArea (pWindow->GetPosPixel(), aWindowSize);
    Rectangle aViewPixelArea (
        pWindow->LogicToPixel(mrSlideSorter.GetView().GetModelArea()));

    if (aWindowSize.Width() > maScrollBorder.Width() * 3
        && mpHorizontalScrollBar != NULL
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




bool ScrollBarManager::AutoScroll (const Point& rMouseWindowPosition)
{
    CalcAutoScrollOffset (rMouseWindowPosition);
    bool bResult = RepeatAutoScroll();
    if (bResult)
    {
        maAutoScrollTimer.Start();
    }
    return bResult;
}




void ScrollBarManager::StopAutoScroll (void)
{
    maAutoScrollTimer.Stop();
}




bool ScrollBarManager::RepeatAutoScroll (void)
{
    if (maAutoScrollOffset != Size(0,0))
    {
        if (mrSlideSorter.GetViewShell() != NULL)
        {
            mrSlideSorter.GetViewShell()->ScrollLines(
                maAutoScrollOffset.Width(),
                    maAutoScrollOffset.Height());
            return true;
        }
    }

    return false;
}




IMPL_LINK(ScrollBarManager, AutoScrollTimeoutHandler, Timer *, EMPTYARG)
{
    RepeatAutoScroll();

    return 0;
}


} } } // end of namespace ::sd::slidesorter::controller
