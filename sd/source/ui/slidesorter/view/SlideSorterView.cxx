/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlideSorterView.cxx,v $
 * $Revision: 1.29 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "precompiled_sd.hxx"

#include "view/SlideSorterView.hxx"

#include "ViewShellBase.hxx"
#include "SlideSorter.hxx"
#include "SlideSorterViewShell.hxx"
#include "ViewShell.hxx"
#include "SlsViewCacheContext.hxx"
#include "SlsLayeredDevice.hxx"
#include "view/SlsLayouter.hxx"
#include "view/SlsPageObjectLayouter.hxx"
#include "view/SlsPageObjectPainter.hxx"
#include "view/SlsILayerPainter.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsProperties.hxx"
#include "controller/SlsAnimator.hxx"
#include "controller/SlsAnimationFunction.hxx"
#include "model/SlideSorterModel.hxx"
#include "model/SlsPageEnumerationProvider.hxx"
#include "model/SlsPageDescriptor.hxx"
#include "cache/SlsPageCache.hxx"
#include "cache/SlsPageCacheManager.hxx"
#include "cache/SlsCacheContext.hxx"
#include "taskpane/SlideSorterCacheDisplay.hxx"
#include "DrawDocShell.hxx"
#include "PaneDockingWindow.hxx"

#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "Window.hxx"
#include "sdresid.hxx"
#include "glob.hrc"

#include <svl/itempool.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdopage.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnclit.hxx>
#include <com/sun/star/presentation/FadeEffect.hpp>
#include <vcl/svapp.hxx>
#include <tools/poly.hxx>
#include <vcl/lineinfo.hxx>
#include <algorithm>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdrpagewindow.hxx>
#include <drawinglayer/processor2d/vclpixelprocessor2d.hxx>
#include <svl/itempool.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>
#include <drawinglayer/primitive2d/backgroundcolorprimitive2d.hxx>
#include <canvas/elapsedtime.hxx>
#include <boost/bind.hpp>

//#define DEBUG_TIMING
#ifdef DEBUG_TIMING
#include <vector>
#endif

using namespace std;
using namespace ::sd::slidesorter::model;
using namespace ::drawinglayer::primitive2d;

using ::sd::slidesorter::controller::Animator;
using ::sd::slidesorter::controller::AnimationFunction;


namespace sd { namespace slidesorter { namespace view {


/** Wrapper around the SlideSorterView that supports the IPainter interface
    and that allows the LayeredDevice to hold the SlideSorterView (held as
    scoped_ptr by the SlideSorter) as shared_ptr.
*/
class Painter : public ILayerPainter
{
public:
    Painter (SlideSorterView& rView) : mrView(rView) {}
    virtual ~Painter (void) {}

    virtual void Paint (OutputDevice& rDevice, const Rectangle& rRepaintArea)
    {
        mrView.Paint(rDevice,rRepaintArea);
    }
    virtual void SetLayerInvalidator (const SharedILayerInvalidator&)
    {
    }

private:
    SlideSorterView& mrView;
};




class BackgroundPainter
    : public ILayerPainter,
      public ::boost::noncopyable
{
public:
    BackgroundPainter (
        const SharedSdWindow& rpWindow,
        const Color aBackgroundColor)
        : maBackgroundColor(aBackgroundColor),
          mpWindow(rpWindow)
    {
    }

    ~BackgroundPainter (void)
    {
    }

    virtual void Paint (OutputDevice& rDevice, const Rectangle& rRepaintArea)
    {
        rDevice.SetFillColor(maBackgroundColor);
        rDevice.SetLineColor();
        rDevice.DrawRect(rRepaintArea);
    }

    virtual void SetLayerInvalidator (const SharedILayerInvalidator& rpInvalidator)
    {
        (void)rpInvalidator;
    }

private:
    const Color maBackgroundColor;
    SharedSdWindow mpWindow;
};



TYPEINIT1(SlideSorterView, ::sd::View);

SlideSorterView::SlideSorterView (SlideSorter& rSlideSorter)
    : ::sd::View (
        rSlideSorter.GetModel().GetDocument(),
            rSlideSorter.GetContentWindow().get(),
        rSlideSorter.GetViewShell()),
    mrSlideSorter(rSlideSorter),
    mrModel(rSlideSorter.GetModel()),
    mbIsDisposed(false),
    mpLayouter (new Layouter(rSlideSorter.GetContentWindow())),
    mbPageObjectVisibilitiesValid (false),
    mpPreviewCache(),
    mpLayeredDevice(new LayeredDevice(rSlideSorter.GetContentWindow())),
    maVisiblePageRange(-1,-1),
    mbModelChangedWhileModifyEnabled(true),
    maPreviewSize(0,0),
    mbPreciousFlagUpdatePending(true),
    meOrientation(Layouter::GRID),
    mpProperties(rSlideSorter.GetProperties()),
    mpPageUnderMouse(),
    mbIsMouseOverIndicationAllowed(true),
    mnButtonUnderMouse(-1),
    mpPageObjectPainter(),
    mpSelectionPainter()
{
    // Hide the page that contains the page objects.
    SetPageVisible (FALSE);

    // Wrap a shared_ptr held wrapper around this view and register it as
    // painter at the layered device.  There is no explicit destruction: in
    // the SlideSorterView destructor the layered device is destroyed and
    // with it the only reference to the wrapper which therefore is also
    // destroyed.
    mpLayeredDevice->RegisterPainter(SharedILayerPainter(new Painter(*this)), 2);
}




SlideSorterView::~SlideSorterView (void)
{
    if ( ! mbIsDisposed)
    {
        OSL_ASSERT(mbIsDisposed);
        Dispose();
    }
}




void SlideSorterView::Init (void)
{
    HandleModelChange();
}




void SlideSorterView::Dispose (void)
{
    mpSelectionPainter.reset();

    mpLayeredDevice->Dispose();
    mpPreviewCache.reset();

    // Hide the page to avoid problems in the view when deleting
    // visualized objects
    HideSdrPage();

    // Deletion of the objects and the page will be done in SdrModel
    // destructor (as long as objects and pages are added)

    OSL_ASSERT(mpLayeredDevice.unique());
    mpLayeredDevice.reset();

    mbIsDisposed = true;
}




sal_Int32 SlideSorterView::GetPageIndexAtPoint (const Point& rWindowPosition) const
{
    sal_Int32 nIndex (-1);

    SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
    if (pWindow)
    {
        nIndex = mpLayouter->GetIndexAtPoint(pWindow->PixelToLogic(rWindowPosition));

        // Clip the page index against the page count.
        if (nIndex >= mrModel.GetPageCount())
            nIndex = -1;
    }

    return nIndex;
}




Layouter& SlideSorterView::GetLayouter (void)
{
    return *mpLayouter.get();
}




void SlideSorterView::ModelHasChanged (void)
{
    // Ignore this call.  Rely on hints sent by the model to get informed of
    // model changes.
}




void SlideSorterView::LocalModelHasChanged(void)
{
    mbModelChangedWhileModifyEnabled = false;

    // First call our base class.
    View::ModelHasChanged ();
}




void SlideSorterView::PreModelChange (void)
{
    // Reset the slide under the mouse.  It will be re-set in PostModelChange().
    SetPageUnderMouse(SharedPageDescriptor());
}




void SlideSorterView::PostModelChange (void)
{
    // In PreModelChange() the page objects have been released.  Here we
    // create new ones.
    ::osl::MutexGuard aGuard (mrModel.GetMutex());

    model::PageEnumeration aPageEnumeration (
        model::PageEnumerationProvider::CreateAllPagesEnumeration(mrModel));

    // The new page objects have to be scaled and positioned.
    Layout();
    RequestRepaint();
}




/** At the moment for every model change all page objects are destroyed and
    re-created again.  This can be optimized by accepting hints that
    describe the type of change so that existing page objects can be
    reused.
*/
void SlideSorterView::HandleModelChange (void)
{
    PreModelChange ();
    PostModelChange();
}




void SlideSorterView::HandleDrawModeChange (void)
{
    // Replace the preview cache with a new and empty one.  The
    // PreviewRenderer that is used by the cache is replaced by this as
    // well.
    mpPreviewCache.reset();
    GetPreviewCache()->InvalidateCache(true);

    RequestRepaint();
}




void SlideSorterView::Resize (void)
{
    UpdateOrientation();

    if ( ! mpLayeredDevice->HasPainter(0))
        mpLayeredDevice->RegisterPainter(
            SharedILayerPainter(new BackgroundPainter(
                mrSlideSorter.GetContentWindow(),
                mrSlideSorter.GetTheme()->GetColor(Theme::Background))),
            0);

    mpLayeredDevice->Resize();
    SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
    if (mrModel.GetPageCount()>0 && pWindow)
    {
        const bool bRearrangeSuccess (
            mpLayouter->Rearrange (
                meOrientation,
                pWindow->GetSizePixel(),
                mrModel.GetPageDescriptor(0)->GetPage()->GetSize(),
                mrModel.GetPageCount()));

        if (bRearrangeSuccess)
        {
            Layout();
            RequestRepaint();
        }
    }
}




void SlideSorterView::UpdateOrientation (void)
{
    // The layout of slides depends on whether the slide sorter is
    // displayed in the center or the side pane.
    if (mrSlideSorter.GetViewShell()->IsMainViewShell())
        SetOrientation(Layouter::GRID);
    else
    {
        // Get access to the docking window.
        ::Window* pWindow = mrSlideSorter.GetContentWindow().get();
        PaneDockingWindow* pDockingWindow = NULL;
        while (pWindow!=NULL && pDockingWindow==NULL)
        {
            pDockingWindow = dynamic_cast<PaneDockingWindow*>(pWindow);
            pWindow = pWindow->GetParent();
        }

        if (pDockingWindow != NULL)
        {
            const long nScrollBarSize (
                Application::GetSettings().GetStyleSettings().GetScrollBarSize());
            switch (pDockingWindow->GetOrientation())
            {
                case PaneDockingWindow::HorizontalOrientation:
                    if (SetOrientation(Layouter::HORIZONTAL))
                    {
                        const Range aRange (mpLayouter->GetValidVerticalSizeRange());
                        pDockingWindow->SetValidSizeRange(Range(
                            aRange.Min() + nScrollBarSize,
                            aRange.Max() + nScrollBarSize));
                    }
                    break;

                case PaneDockingWindow::VerticalOrientation:
                    if (SetOrientation(Layouter::VERTICAL))
                    {
                        const Range aRange (mpLayouter->GetValidHorizontalSizeRange());
                        pDockingWindow->SetValidSizeRange(Range(
                            aRange.Min() + nScrollBarSize,
                            aRange.Max() + nScrollBarSize));
                    }
                    break;

                case PaneDockingWindow::UnknownOrientation:
                    if (SetOrientation(Layouter::GRID))
                    {
                        const sal_Int32 nAdditionalSize (10);
                        pDockingWindow->SetMinOutputSizePixel(Size(
                            mpLayouter->GetValidHorizontalSizeRange().Min()
                                + nScrollBarSize
                                + nAdditionalSize,
                            mpLayouter->GetValidVerticalSizeRange().Min()
                                + nScrollBarSize
                                + nAdditionalSize));
                    }
                    return;
            }
        }
        else
        {
            OSL_ASSERT(pDockingWindow!=NULL);
            SetOrientation(Layouter::GRID);
        }
    }
}




void SlideSorterView::Layout ()
{
    SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
    if (pWindow)
    {
        // Set the model area, i.e. the smallest rectangle that includes all
        // page objects.
        const Rectangle aViewBox (mpLayouter->GetTotalBoundingBox());
        pWindow->SetViewOrigin (aViewBox.TopLeft());
        pWindow->SetViewSize (aViewBox.GetSize());

        ::boost::shared_ptr<PageObjectLayouter> pPageObjectLayouter(
            mpLayouter->GetPageObjectLayouter());
        if (pPageObjectLayouter)
        {
            const Size aNewPreviewSize (mpLayouter->GetPageObjectLayouter()->GetPreviewSize());
            if (maPreviewSize != aNewPreviewSize && GetPreviewCache())
            {
                mpPreviewCache->ChangeSize(aNewPreviewSize, true);
                maPreviewSize = aNewPreviewSize;
            }
        }

        // Iterate over all page objects and place them relative to the
        // containing page.
        model::PageEnumeration aPageEnumeration (
            model::PageEnumerationProvider::CreateAllPagesEnumeration(mrModel));
        while (aPageEnumeration.HasMoreElements())
        {
            model::SharedPageDescriptor pDescriptor (aPageEnumeration.GetNextElement());
            pDescriptor->SetBoundingBox(mpLayouter->GetPageObjectBox(pDescriptor->GetPageIndex()));
        }

        GetPageObjectPainter()->NotifyResize();
    }

    InvalidatePageObjectVisibilities ();
}




void SlideSorterView::InvalidatePageObjectVisibilities (void)
{
    mbPageObjectVisibilitiesValid = false;
}




void SlideSorterView::DeterminePageObjectVisibilities (void)
{
    SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
    if (pWindow)
    {
        // Set this flag to true here so that an invalidate during the
        // visibility calculation can correctly invalidate it again.
        mbPageObjectVisibilitiesValid = true;

        Rectangle aViewArea (pWindow->PixelToLogic(Rectangle(Point(0,0),pWindow->GetSizePixel())));
        const Range aRange (mpLayouter->GetRangeOfVisiblePageObjects(aViewArea));
        const Range aUnion(
            ::std::min(maVisiblePageRange.Min(), aRange.Min()),
            ::std::max(maVisiblePageRange.Max(), aRange.Max()));

        // For page objects that just dropped off the visible area we
        // decrease the priority of pending requests for preview bitmaps.
        if (maVisiblePageRange != aRange)
            mbPreciousFlagUpdatePending |= true;

        model::SharedPageDescriptor pDescriptor;
        for (int nIndex=aUnion.Min(); nIndex<=aUnion.Max(); nIndex++)
        {
            pDescriptor = mrModel.GetPageDescriptor(nIndex);
            if (pDescriptor.get() != NULL)
                SetState(
                    pDescriptor,
                    PageDescriptor::ST_Visible,
                    aRange.IsInside(nIndex));
        }
        maVisiblePageRange = aRange;

        // Restore the mouse over state.
        UpdatePageUnderMouse(false);
    }
}




void SlideSorterView::UpdatePreciousFlags (void)
{
    if (mbPreciousFlagUpdatePending)
    {
        mbPreciousFlagUpdatePending = false;

        model::SharedPageDescriptor pDescriptor;
        ::boost::shared_ptr<cache::PageCache> pCache = GetPreviewCache();
        sal_Int32 nPageCount (mrModel.GetPageCount());

        for (int nIndex=0; nIndex<=nPageCount; ++nIndex)
        {
            pDescriptor = mrModel.GetPageDescriptor(nIndex);
            if (pDescriptor.get() != NULL)
            {
                pCache->SetPreciousFlag(
                    pDescriptor->GetPage(),
                    maVisiblePageRange.IsInside(nIndex));
                SSCD_SET_VISIBILITY(mrModel.GetDocument(), nIndex,
                    maVisiblePageRange.IsInside(nIndex));
            }
            else
            {
                // At least one cache entry can not be updated.  Remember to
                // repeat the whole updating later and leave the loop now.
                mbPreciousFlagUpdatePending = true;
                break;
            }
        }
    }
}




bool SlideSorterView::SetOrientation (const Layouter::Orientation eOrientation)
{
    if (meOrientation != eOrientation)
    {
        meOrientation = eOrientation;
        return true;
    }
    else
        return false;
}




Layouter::Orientation SlideSorterView::GetOrientation (void) const
{
    return meOrientation;
}




void SlideSorterView::RequestRepaint (void)
{
    SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
    if (pWindow)
    {
        pWindow->Invalidate();
        mpLayeredDevice->InvalidateAllLayers(
            Rectangle(
                pWindow->PixelToLogic(Point(0,0)),
                pWindow->PixelToLogic(pWindow->GetSizePixel())));
    }
}




void SlideSorterView::RequestRepaint (const model::SharedPageDescriptor& rpDescriptor)
{
    RequestRepaint(rpDescriptor->GetBoundingBox());
}




void SlideSorterView::RequestRepaint (const Rectangle& rRepaintBox)
{
    SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
    if (pWindow)
    {
        pWindow->Invalidate(rRepaintBox);
        mpLayeredDevice->InvalidateAllLayers(rRepaintBox);
    }
}



void SlideSorterView::RequestRepaint (const Region& rRepaintRegion)
{
    SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
    if (pWindow)
    {
        pWindow->Invalidate(rRepaintRegion);
        mpLayeredDevice->InvalidateAllLayers(rRepaintRegion);
    }
}




Rectangle SlideSorterView::GetModelArea (void)
{
    return mpLayouter->GetTotalBoundingBox();
}


#ifdef DEBUG_TIMING
static ::canvas::tools::ElapsedTime gaTimer;
static const size_t gFrameTimeCount (10);
static size_t gFrameTimeIndex (0);
static ::std::vector<double> gFrameTimes (gFrameTimeCount, 0);
static double gFrameTimeSum (0);
static const Rectangle gFrameTimeBox (10,10,150,20);
static double gnLastFrameStart = 0;
#endif

void SlideSorterView::CompleteRedraw (
    OutputDevice* pDevice,
    const Region& rPaintArea,
    sdr::contact::ViewObjectContactRedirector* pRedirector)
{
    (void)pRedirector;
#ifdef DEBUG_TIMING
    const double nStartTime (gaTimer.getElapsedTime());
    OSL_TRACE("SlideSorterView::CompleteRedraw start at %f, %s",
        nStartTime,
        mnLockRedrawSmph ? "locked" : "");
#endif

    if (pDevice == NULL || pDevice!=mrSlideSorter.GetContentWindow().get())
        return;

    // The parent implementation of CompleteRedraw is called only when
    // painting is locked.  We do all the painting ourself.  When painting
    // is locked the parent implementation keeps track of the repaint
    // requests and later, when painting is unlocked, calls CompleteRedraw
    // for all missed repaints.

    if (mnLockRedrawSmph == 0)
    {
        mrSlideSorter.GetContentWindow()->IncrementLockCount();
        mpLayeredDevice->Repaint(rPaintArea);
        mrSlideSorter.GetContentWindow()->DecrementLockCount();
    }
    else
    {
        maRedrawRegion.Union(rPaintArea);
    }

#ifdef DEBUG_TIMING
    const double nEndTime (gaTimer.getElapsedTime());
    OSL_TRACE("SlideSorterView::CompleteRedraw end at %f after %fms", nEndTime, (nEndTime-nStartTime)*1000);
    gFrameTimeSum -= gFrameTimes[gFrameTimeIndex];
    gFrameTimes[gFrameTimeIndex] = nStartTime - gnLastFrameStart;
    gnLastFrameStart = nStartTime;
    gFrameTimeSum += gFrameTimes[gFrameTimeIndex];
    gFrameTimeIndex = (gFrameTimeIndex+1) % gFrameTimeCount;


    mrSlideSorter.GetContentWindow()->SetFillColor(COL_BLUE);
    mrSlideSorter.GetContentWindow()->DrawRect(gFrameTimeBox);
    mrSlideSorter.GetContentWindow()->SetTextColor(COL_WHITE);
    mrSlideSorter.GetContentWindow()->DrawText(
        gFrameTimeBox,
        ::rtl::OUString::valueOf(1 / (gFrameTimeSum / gFrameTimeCount)),
        TEXT_DRAW_RIGHT | TEXT_DRAW_VCENTER);
    //    mrSlideSorter.GetContentWindow()->Invalidate(gFrameTimeBox);
#endif
}




void SlideSorterView::Paint (
    OutputDevice& rDevice,
    const Rectangle& rRepaintArea)
{
    if ( ! mpPageObjectPainter)
        if ( ! GetPageObjectPainter())
            return;

    // Update the page visibilities when they have been invalidated.
    if ( ! mbPageObjectVisibilitiesValid)
        DeterminePageObjectVisibilities();

    if (mbPreciousFlagUpdatePending)
        UpdatePreciousFlags();

    // Paint all page objects that are fully or partially inside the
    // repaint region.
    const Range aRange (mpLayouter->GetRangeOfVisiblePageObjects(rRepaintArea));
    for (sal_Int32 nIndex=aRange.Min(); nIndex<=aRange.Max(); ++nIndex)
    {
        model::SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nIndex));
        if (!pDescriptor || ! pDescriptor->HasState(PageDescriptor::ST_Visible))
            continue;

        mpPageObjectPainter->PaintPageObject(rDevice, pDescriptor);
    }
}




void SlideSorterView::ConfigurationChanged (
    utl::ConfigurationBroadcaster* pBroadcaster,
    sal_uInt32 nHint)
{
    // Some changes of the configuration (some of the colors for example)
    // may affect the previews.  Throw away the old ones and create new ones.
    cache::PageCacheManager::Instance()->InvalidateAllCaches();

    ::sd::View::ConfigurationChanged(pBroadcaster, nHint);
}




::boost::shared_ptr<cache::PageCache> SlideSorterView::GetPreviewCache (void)
{
    SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
    if (pWindow && mpPreviewCache.get() == NULL)
    {
        mpPreviewCache.reset(
            new cache::PageCache(
                mpLayouter->GetPageObjectSize(),
                false,
                cache::SharedCacheContext(new ViewCacheContext(mrSlideSorter))));
    }

    return mpPreviewCache;
}




Pair SlideSorterView::GetVisiblePageRange (void)
{
    if ( ! mbPageObjectVisibilitiesValid)
        DeterminePageObjectVisibilities();
    return maVisiblePageRange;
}




void SlideSorterView::Notify (SfxBroadcaster& rBroadcaster, const SfxHint& rHint)
{
    ::sd::DrawDocShell* pDocShell = mrModel.GetDocument()->GetDocSh();
    if (pDocShell!=NULL && pDocShell->IsEnableSetModified())
        mbModelChangedWhileModifyEnabled = true;

    ::sd::View::Notify(rBroadcaster, rHint);
}




void SlideSorterView::SetIsMouseOverIndicationAllowed (const bool bIsAllowed)
{
    mbIsMouseOverIndicationAllowed = bIsAllowed;
    if ( ! mbIsMouseOverIndicationAllowed)
        SetPageUnderMouse(model::SharedPageDescriptor());
    else
        UpdatePageUnderMouse(false);
}




void SlideSorterView::UpdatePageUnderMouse (bool bAnimate)
{
    SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
    if (pWindow && ! pWindow->IsMouseCaptured())
    {
        const Window::PointerState aPointerState (pWindow->GetPointerState());
        UpdatePageUnderMouse (
            aPointerState.maPos,
            (aPointerState.mnState & MOUSE_LEFT)!=0,
            bAnimate);
    }
}




void SlideSorterView::UpdatePageUnderMouse (
    const Point& rMousePosition,
    const bool bIsMouseButtonDown,
    const bool bAnimate)
{
    if ( ! mbIsMouseOverIndicationAllowed)
    {
        SetPageUnderMouse(model::SharedPageDescriptor());
    }
    else
    {
        // Determine page under mouse and show the mouse over effect.
        model::SharedPageDescriptor pHitDescriptor (
            mrSlideSorter.GetController().GetPageAt(rMousePosition));
        SetPageUnderMouse(pHitDescriptor, bAnimate);

        // Handle the mouse being over any buttons.
        if (pHitDescriptor)
        {
            SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
            const Point aMouseModelPosition (pWindow->PixelToLogic(rMousePosition));
            const sal_Int32 nButtonIndex (
                GetLayouter().GetPageObjectLayouter()->GetButtonIndexAt (
                    pHitDescriptor,
                    aMouseModelPosition));
            SetButtonUnderMouse(nButtonIndex);
            if (bIsMouseButtonDown)
            {
                pHitDescriptor->GetVisualState().SetActiveButtonState(
                    nButtonIndex,
                    model::VisualState::BS_Pressed);
            }
        }
    }
}




void SlideSorterView::SetPageUnderMouse (
    const model::SharedPageDescriptor& rpDescriptor,
    const bool bAnimate)
{
    if (mpPageUnderMouse != rpDescriptor)
    {
        if (mpPageUnderMouse)
        {
            mpPageUnderMouse->GetVisualState().SetActiveButtonState(
                -1,
                model::VisualState::BS_Normal);
            SetState(mpPageUnderMouse, PageDescriptor::ST_MouseOver, false, bAnimate);
        }

        mpPageUnderMouse = rpDescriptor;
        SetButtonUnderMouse(-1);

        if (mpPageUnderMouse)
            SetState(mpPageUnderMouse, PageDescriptor::ST_MouseOver, true, bAnimate);

        // Change the quick help text to display the name of the page under
        // the mouse.
        SharedSdWindow pWindow (mrSlideSorter.GetContentWindow());
        if (pWindow)
        {
            ::rtl::OUString sText;
            if (mpPageUnderMouse)
            {
                SdPage* pPage = mpPageUnderMouse->GetPage();
                if (pPage != NULL)
                    sText = pPage->GetName();
                else
                {
                    OSL_ASSERT(mpPageUnderMouse->GetPage() != NULL);
                }
                if (sText.getLength() == 0)
                {
                    sText = String(SdResId(STR_PAGE));
                    sText += String::CreateFromInt32(mpPageUnderMouse->GetPageIndex()+1);
                }
            }
            pWindow->SetQuickHelpText(sText);
        }
    }
}




void SlideSorterView::SetButtonUnderMouse (const sal_Int32 nButtonIndex)
{
    if (mnButtonUnderMouse != nButtonIndex)
    {
        if (mpPageUnderMouse)
        {
            mnButtonUnderMouse = nButtonIndex;
            mpPageUnderMouse->GetVisualState().SetActiveButtonState(
                mnButtonUnderMouse,
                model::VisualState::BS_MouseOver);
            RequestRepaint(mpPageUnderMouse);
        }
    }
}




bool SlideSorterView::SetState (
    const model::SharedPageDescriptor& rpDescriptor,
    const PageDescriptor::State eState,
    const bool bStateValue,
    const bool bAnimate)
{
    model::SharedPageDescriptor pDescriptor (rpDescriptor);
    if ( ! pDescriptor)
        return false;

    const bool bModified (pDescriptor->SetState(eState, bStateValue));
    if ( ! bModified)
        return false;

    switch(eState)
    {
        case PageDescriptor::ST_Visible:
        case PageDescriptor::ST_Selected:
        case PageDescriptor::ST_Focused:
        case PageDescriptor::ST_MouseOver:
        case PageDescriptor::ST_Current:
        case PageDescriptor::ST_Excluded:
            RequestRepaint(pDescriptor);
            break;

        case PageDescriptor::ST_WasSelected:
            // Ignore.
            break;
    }

    // Fade in or out the buttons.
    if (eState == PageDescriptor::ST_MouseOver)
    {
        const double nEndAlpha (bStateValue ? 0.2 : 1.0);
        if (bAnimate)
        {
            const double nStartAlpha (pDescriptor->GetVisualState().GetButtonAlpha());

            // Stop a running animation.
            const Animator::AnimationId nId (
                pDescriptor->GetVisualState().GetButtonAlphaAnimationId());
            if (nId != Animator::NotAnAnimationId)
                mrSlideSorter.GetController().GetAnimator()->RemoveAnimation(nId);

            const ::boost::function<double(double)> aBlendFunctor (
                ::boost::bind(
                    AnimationFunction::Blend,
                nStartAlpha,
                    nEndAlpha,
                    ::boost::bind(AnimationFunction::Linear, _1)));
            pDescriptor->GetVisualState().SetButtonAlphaAnimationId(
                mrSlideSorter.GetController().GetAnimator()->AddAnimation(
                    ::boost::bind(
                        AnimationFunction::ApplyButtonAlphaChange,
                        pDescriptor,
                        ::boost::ref(*this),
                        ::boost::bind(aBlendFunctor, _1)),
                    bStateValue ? 500 : 0,
                    400,
                    ::boost::bind(
                        &VisualState::SetButtonAlphaAnimationId,
                        ::boost::ref(pDescriptor->GetVisualState()),
                        controller::Animator::NotAnAnimationId)
                    ));
        }
        else
        {
            pDescriptor->GetVisualState().SetButtonAlpha(nEndAlpha);
        }
    }

    return bModified;
}




::boost::shared_ptr<PageObjectPainter> SlideSorterView::GetPageObjectPainter (void)
{
    if ( ! mpPageObjectPainter)
        mpPageObjectPainter.reset(new PageObjectPainter(mrSlideSorter));
    return mpPageObjectPainter;
}




::boost::shared_ptr<LayeredDevice> SlideSorterView::GetLayeredDevice (void) const
{
    return mpLayeredDevice;
}




//===== Animator::DrawLock ====================================================

SlideSorterView::DrawLock::DrawLock (
    view::SlideSorterView& rView,
    const SharedSdWindow& rpWindow)
    : mrView(rView),
      mpWindow(rpWindow)
{
    if (mrView.mnLockRedrawSmph == 0)
        mrView.maRedrawRegion.SetEmpty();
    ++mrView.mnLockRedrawSmph;
}




SlideSorterView::DrawLock::DrawLock (SlideSorter& rSlideSorter)
    : mrView(rSlideSorter.GetView()),
      mpWindow(rSlideSorter.GetContentWindow())
{
    if (mrView.mnLockRedrawSmph == 0)
        mrView.maRedrawRegion.SetEmpty();
    ++mrView.mnLockRedrawSmph;
}




SlideSorterView::DrawLock::~DrawLock (void)
{
    OSL_ASSERT(mrView.mnLockRedrawSmph>0);
    --mrView.mnLockRedrawSmph;
    if (mrView.mnLockRedrawSmph == 0)
        if (mpWindow)
        {
            mpWindow->Invalidate(mrView.maRedrawRegion);
            mpWindow->Update();
        }
    /*
            mrView.CompleteRedraw(
                mpWindow.get(),
                mrView.maRedrawRegion);
    */
}




void SlideSorterView::DrawLock::Dispose (void)
{
    mpWindow.reset();
}


} } } // end of namespace ::sd::slidesorter::view
