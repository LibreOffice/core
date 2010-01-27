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
#include "view/SlsViewOverlay.hxx"
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
#include <boost/bind.hpp>


using namespace std;
using namespace ::sd::slidesorter::model;
using namespace ::drawinglayer::primitive2d;

using ::sd::slidesorter::controller::Animator;
using ::sd::slidesorter::controller::AnimationFunction;

//#define VERBOSE

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

class AnimatedSphere
{
public:
    AnimatedSphere (
        const Size& rWindowSize,
        const double nStartTime)
        : mnCenterX(rand() * rWindowSize.Width() / RAND_MAX),
          mnCenterY(rand() * rWindowSize.Height() / RAND_MAX),
          mnStartRadius(10),
          mnEndRadius(rand() * 150 / RAND_MAX),
          maColor(GetColor()),
          mnLocalTime(-nStartTime),
          mnValue(0),
          mnStartTime(nStartTime)
    {
    }

    void SetTime (const double nTime)
    {
        mnLocalTime = nTime - mnStartTime;
        if (mnLocalTime >= 0 && mnLocalTime <= 1)
            mnValue = controller::AnimationFunction::SlowInSlowOut_0to0_Sine(mnLocalTime);
        else
            mnValue = 0;
    }

    bool IsExpired (void)
    {
        return mnLocalTime >= 1.0;
    }

    Rectangle GetBoundingBox (void)
    {
        if (mnLocalTime < 0)
            return Rectangle();

        const double nRadius (mnStartRadius*(1-mnValue) + mnEndRadius*mnValue);
        const sal_Int32 nIntRadius (ceil(nRadius)+1);
        return Rectangle(
            mnCenterX-nIntRadius,
            mnCenterY-nIntRadius,
            mnCenterX+nIntRadius,
            mnCenterY+nIntRadius);
    }

    void Paint (OutputDevice& rDevice)
    {
        if (mnLocalTime < 0)
            return;

        rDevice.SetFillColor(maColor);
        rDevice.SetLineColor();

        const Rectangle aBox (GetBoundingBox());
        const USHORT nSavedAntialiasingMode (rDevice.GetAntialiasing());
        rDevice.SetAntialiasing(nSavedAntialiasingMode | ANTIALIASING_ENABLE_B2DDRAW);
        rDevice.DrawPolygon(
            ::basegfx::tools::createPolygonFromRect(
                ::basegfx::B2DRectangle(aBox.Left(), aBox.Top(), aBox.Right(), aBox.Bottom()),
                1.0,
                1.0));
        rDevice.SetAntialiasing(nSavedAntialiasingMode);
    }

private:
    const sal_Int32 mnCenterX;
    const sal_Int32 mnCenterY;
    const double mnStartRadius;
    const double mnEndRadius;
    const Color maColor;
    double mnLocalTime;
    double mnValue;
    const double mnStartTime;

    static Color GetColor (void)
    {
        Color aColor;
        do
        {
            aColor.SetRed(rand() * 255 / RAND_MAX);
            aColor.SetGreen(rand() * 255 / RAND_MAX);
            aColor.SetBlue(rand() * 255 / RAND_MAX);
        }
        while (aColor.GetGreen()<=aColor.GetRed() || aColor.GetGreen()<=aColor.GetBlue());
        return aColor;
    }
};

class BackgroundPainter
    : public ILayerPainter,
      public ::boost::noncopyable
{
public:
    BackgroundPainter (
        const ::boost::shared_ptr<controller::Animator>& rpAnimator,
        const ::boost::shared_ptr< ::Window>& rpWindow,
        const Color aBackgroundColor,
        const bool bIsAnimated)
        : mpAnimator(rpAnimator),
          mnAnimationId(controller::Animator::NotAnAnimationId),
          maBackgroundColor(aBackgroundColor),
          maSpheres(),
          mpInvalidator(),
          mpWindow(rpWindow)
    {
        if (bIsAnimated)
        {
            mnAnimationId = mpAnimator->AddInfiniteAnimation(::boost::ref(*this), 0.01);

            for (sal_Int32 nIndex=0; nIndex<10; ++nIndex)
                maSpheres.push_back(::boost::shared_ptr<AnimatedSphere>(
                    new AnimatedSphere(rpWindow->GetSizePixel(), nIndex*0.3)));
        }
    }

    ~BackgroundPainter (void)
    {
        mpAnimator->RemoveAnimation(mnAnimationId);
    }

    virtual void Paint (OutputDevice& rDevice, const Rectangle& rRepaintArea)
    {
        rDevice.SetFillColor(maBackgroundColor);
        rDevice.SetLineColor();
        rDevice.DrawRect(rRepaintArea);

        for (SphereVector::const_iterator
                 iSphere(maSpheres.begin()),
                 iEnd(maSpheres.end());
             iSphere!=iEnd;
             ++iSphere)
        {
            if (rRepaintArea.IsOver((*iSphere)->GetBoundingBox()))
                (*iSphere)->Paint(rDevice);
        }
    }

    virtual void SetLayerInvalidator (const SharedILayerInvalidator& rpInvalidator)
    {
        Invalidate();
        mpInvalidator = rpInvalidator;
        Invalidate();
    }

    void operator () (const double nTime)
    {
        Invalidate();

        for (SphereVector::iterator
                 iSphere(maSpheres.begin()),
                 iEnd(maSpheres.end());
             iSphere!=iEnd;
             ++iSphere)
        {
            if ((*iSphere)->IsExpired())
                (*iSphere).reset(
                    new AnimatedSphere(mpWindow->GetSizePixel(), nTime));
            else
                (*iSphere)->SetTime(nTime);
        }

        Invalidate();
    }

private:
    const ::boost::shared_ptr<controller::Animator> mpAnimator;
    const Color maBackgroundColor;
    typedef ::std::vector< ::boost::shared_ptr<AnimatedSphere> > SphereVector;
    SphereVector maSpheres;
    SharedILayerInvalidator mpInvalidator;
    ::boost::shared_ptr< ::Window> mpWindow;
    controller::Animator::AnimationId mnAnimationId;

    void Invalidate (void)
    {
        if (mpInvalidator)
            for (SphereVector::const_iterator
                     iSphere(maSpheres.begin()),
                     iEnd(maSpheres.end());
                 iSphere!=iEnd;
                 ++iSphere)
            {
                mpInvalidator->Invalidate((*iSphere)->GetBoundingBox());
            }
    }
};



TYPEINIT1(SlideSorterView, ::sd::View);

SlideSorterView::SlideSorterView (SlideSorter& rSlideSorter)
    : ::sd::View (
        rSlideSorter.GetModel().GetDocument(),
        NULL,
        rSlideSorter.GetViewShell()),
    mrSlideSorter(rSlideSorter),
    mrModel(rSlideSorter.GetModel()),
    mpLayouter (new Layouter (rSlideSorter.GetContentWindow())),
    mbPageObjectVisibilitiesValid (false),
    mpPreviewCache(),
    mpLayeredDevice(new LayeredDevice(rSlideSorter.GetContentWindow())),
    mpViewOverlay (new ViewOverlay(rSlideSorter, mpLayeredDevice)),
    mnFirstVisiblePageIndex(0),
    mnLastVisiblePageIndex(-1),
    mbModelChangedWhileModifyEnabled(true),
    maPreviewSize(0,0),
    mbPreciousFlagUpdatePending(true),
    meOrientation(VERTICAL),
    mpProperties(rSlideSorter.GetProperties()),
    mpPageUnderMouse(),
    mnButtonUnderMouse(-1),
    mpPageObjectPainter()
{
    // Hide the page that contains the page objects.
    SetPageVisible (FALSE);

    // Wrap a shared_ptr held wrapper around this view and register it as
    // painter at the layered device.  There is no explicit destruction: in
    // the SlideSorterView destructor the layered device is destroyed and
    // with it the only reference to the wrapper which therefore is also
    // destroyed.
    mpLayeredDevice->RegisterPainter(SharedILayerPainter(new Painter(*this)), 1);
}




SlideSorterView::~SlideSorterView (void)
{
    mpLayeredDevice->Dispose();

    mpPreviewCache.reset();

    // hide the page to avoid problems in the view when deleting
    // visualized objects
    HideSdrPage();

    // Deletion of the objects and the page will be done in SdrModel
    // destructor (as long as objects and pages are added)

    OSL_ASSERT(mpViewOverlay.unique());
    mpViewOverlay.reset();

    OSL_ASSERT(mpLayeredDevice.unique());
    mpLayeredDevice.reset();
}




void SlideSorterView::Dispose (void)
{
}




sal_Int32 SlideSorterView::GetPageIndexAtPoint (const Point& rPosition) const
{
    sal_Int32 nIndex (-1);

    ::boost::shared_ptr<sd::Window> pWindow (mrSlideSorter.GetContentWindow());
    if (pWindow)
    {
        nIndex = mpLayouter->GetIndexAtPoint (pWindow->PixelToLogic (rPosition));

        // Clip the page index against the page count.
        if (nIndex >= mrModel.GetPageCount())
            nIndex = -1;
    }

    return nIndex;
}




sal_Int32 SlideSorterView::GetFadePageIndexAtPoint (
    const Point& rPosition) const
{
    sal_Int32 nIndex (-1);

    ::boost::shared_ptr<sd::Window> pWindow (mrSlideSorter.GetContentWindow());
    if (pWindow)
    {
        Point aModelPosition (pWindow->PixelToLogic (rPosition));
        nIndex = mpLayouter->GetIndexAtPoint(
            aModelPosition,
            true // Include page borders into hit test
            );

        // Clip the page index against the page count.
        if (nIndex >= mrModel.GetPageCount())
            nIndex = -1;

        if (nIndex >= 0)
        {
            // Now test whether the given position is inside the area of the
            // fade effect indicator.
            const Rectangle aBox (
                mpLayouter->GetPageObjectLayouter()->GetBoundingBox(
                    mrModel.GetPageDescriptor(nIndex),
                    PageObjectLayouter::TransitionEffectIndicator,
                    PageObjectLayouter::WindowCoordinateSystem));
            const Point aPoint (aModelPosition.getX(), aModelPosition.getY());
            if ( ! aBox.IsInside(aPoint))
                nIndex = -1;
        }
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
    /*
    if (mbModelChangedWhileModifyEnabled)
    {
        controller::SlideSorterController::ModelChangeLock alock( mrSlideSorter.GetController() );
        mrSlideSorter.GetController().HandleModelChange();
        LocalModelHasChanged();
    }
    */
}




void SlideSorterView::LocalModelHasChanged(void)
{
    mbModelChangedWhileModifyEnabled = false;

    // First call our base class.
    View::ModelHasChanged ();

    // Initialize everything that depends on a page view, now that we have
    // one.
    //      SetApplicationDocumentColor(
    //          Application::GetSettings().GetStyleSettings().GetWindowColor());
}




void SlideSorterView::PreModelChange (void)
{
    // Reset the slide under the mouse.  It will be set to the correct slide
    // on the next mouse motion.
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
    Layout ();
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
    if ( ! mpLayeredDevice->HasPainter(0))
        mpLayeredDevice->RegisterPainter(
            SharedILayerPainter(new BackgroundPainter(
                mrSlideSorter.GetController().GetAnimator(),
                mrSlideSorter.GetContentWindow(),
                Color(0xf9fafa),//mpProperties->GetBackgroundColor(),
                false)),
            0);

    mpLayeredDevice->Resize();
    ::boost::shared_ptr<sd::Window> pWindow (mrSlideSorter.GetContentWindow());
    if (mrModel.GetPageCount()>0 && pWindow)
    {
        bool bRearrangeSuccess (false);
        if (meOrientation == HORIZONTAL)
        {
            bRearrangeSuccess = mpLayouter->RearrangeHorizontal (
                pWindow->GetSizePixel(),
                mrModel.GetPageDescriptor(0)->GetPage()->GetSize(),
                mrModel.GetPageCount());
        }
        else
        {
            bRearrangeSuccess = mpLayouter->RearrangeVertical (
                pWindow->GetSizePixel(),
                mrModel.GetPageDescriptor(0)->GetPage()->GetSize(),
                mrModel.GetPageCount());
        }

        if (bRearrangeSuccess)
        {
            Layout();
        }
    }
}




void SlideSorterView::Layout ()
{
    const ::boost::shared_ptr<sd::Window> pWindow (mrSlideSorter.GetContentWindow());
    if (pWindow)
    {
        // Set the model area, i.e. the smallest rectangle that includes all
        // page objects.
        const Rectangle aViewBox (mpLayouter->GetPageBox(mrModel.GetPageCount()));
        pWindow->SetViewOrigin (aViewBox.TopLeft());
        pWindow->SetViewSize (aViewBox.GetSize());

        ::boost::shared_ptr<PageObjectLayouter> pPageObjectLayouter(
            mpLayouter->GetPageObjectLayouter());
        if (pPageObjectLayouter)
        {
            const Size aNewPreviewSize (mpLayouter->GetPageObjectLayouter()->GetPreviewSize());
            if (maPreviewSize != aNewPreviewSize && GetPreviewCache())
            {
                mpPreviewCache->ChangeSize(aNewPreviewSize, false);
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
    }

    InvalidatePageObjectVisibilities ();
}




void SlideSorterView::InvalidatePageObjectVisibilities (void)
{
    mbPageObjectVisibilitiesValid = false;
}




void SlideSorterView::DeterminePageObjectVisibilities (void)
{
    ::boost::shared_ptr<sd::Window> pWindow (mrSlideSorter.GetContentWindow());
    if (pWindow)
    {
        // Set this flag to true here so that an invalidate during the
        // visibility calculation can correctly invalidate it again.
        mbPageObjectVisibilitiesValid = true;

        Rectangle aViewArea (pWindow->PixelToLogic(Rectangle(Point(0,0),pWindow->GetSizePixel())));
        const int nFirstIndex (mpLayouter->GetIndexOfFirstVisiblePageObject(aViewArea));
        const int nLastIndex (mpLayouter->GetIndexOfLastVisiblePageObject(aViewArea));
        const int nMinIndex (::std::min (mnFirstVisiblePageIndex, nFirstIndex));
        const int nMaxIndex (::std::max (mnLastVisiblePageIndex, nLastIndex));

        // For page objects that just dropped off the visible area we
        // decrease the priority of pending requests for preview bitmaps.
        if (mnFirstVisiblePageIndex!=nFirstIndex || mnLastVisiblePageIndex!=nLastIndex)
            mbPreciousFlagUpdatePending |= true;

        model::SharedPageDescriptor pDescriptor;
        for (int nIndex=nMinIndex; nIndex<=nMaxIndex; nIndex++)
        {
            pDescriptor = mrModel.GetPageDescriptor(nIndex);
            if (pDescriptor.get() != NULL)
                SetState(
                    pDescriptor,
                    PageDescriptor::ST_Visible,
                    nIndex>=nFirstIndex && nIndex<=nLastIndex);
        }
        mnFirstVisiblePageIndex = nFirstIndex;
        mnLastVisiblePageIndex = nLastIndex;
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
                    (nIndex>=mnFirstVisiblePageIndex && nIndex<=mnLastVisiblePageIndex));
                SSCD_SET_VISIBILITY(mrModel.GetDocument(), nIndex,
                    (nIndex>=mnFirstVisiblePageIndex && nIndex<=mnLastVisiblePageIndex));
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




void SlideSorterView::SetOrientation (const Orientation eOrientation)
{
    meOrientation = eOrientation;
    RequestRepaint();
}




SlideSorterView::Orientation SlideSorterView::GetOrientation (void) const
{
    return meOrientation;
}




void SlideSorterView::RequestRepaint (void)
{
    ::boost::shared_ptr<sd::Window> pWindow (mrSlideSorter.GetContentWindow());
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
    ::boost::shared_ptr<sd::Window> pWindow (mrSlideSorter.GetContentWindow());
    if (pWindow)
    {
        pWindow->Invalidate(rRepaintBox);
        mpLayeredDevice->InvalidateAllLayers(rRepaintBox);
    }
}




Rectangle SlideSorterView::GetModelArea (void)
{
    return mpLayouter->GetPageBox(mrModel.GetPageCount());
}


static sal_Int32 nPaintIndex = 0;

void SlideSorterView::CompleteRedraw (
    OutputDevice* pDevice,
    const Region& rPaintArea,
    sdr::contact::ViewObjectContactRedirector* pRedirector)
{
    if (pDevice == NULL || pDevice!=mrSlideSorter.GetContentWindow().get())
        return;

    // The parent implementation of CompleteRedraw is called only when
    // painting is locked.  We do all the painting ourself.  When painting
    // is locked the parent implementation keeps track of the repaint
    // requests and later, when painting is unlocked, calls CompleteRedraw
    // for all missed repaints.

    if (mnLockRedrawSmph == 0)
    {
#ifdef VERBOSE
        OSL_TRACE("%5d CompleteRedraw %d %d %d %d",
            nPaintIndex++,
            rPaintArea.GetBoundRect().Left(),
            rPaintArea.GetBoundRect().Top(),
            rPaintArea.GetBoundRect().GetWidth(),
            rPaintArea.GetBoundRect().GetHeight());
#endif

        mrSlideSorter.GetContentWindow()->IncrementLockCount();
        mpLayeredDevice->Repaint(rPaintArea);
        mrSlideSorter.GetContentWindow()->DecrementLockCount();

#ifdef VERBOSE
        OSL_TRACE("CompleteRedraw done");
#endif
    }
    else
    {
#ifdef VERBOSE
        OSL_TRACE("%5d CompleteRedraw while locked", nPaintIndex++);
#endif
        View::CompleteRedraw(pDevice, rPaintArea, pRedirector);
    }
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
    sal_Int32 nFirst (mpLayouter->GetIndexOfFirstVisiblePageObject(rRepaintArea));
    sal_Int32 nLast (std::min(
        mpLayouter->GetIndexOfLastVisiblePageObject(rRepaintArea),
        mrModel.GetPageCount()));
    for (sal_Int32 nIndex=nFirst; nIndex<=nLast; ++nIndex)
    {
        model::SharedPageDescriptor pDescriptor (mrModel.GetPageDescriptor(nIndex));
        if (!pDescriptor || ! pDescriptor->HasState(PageDescriptor::ST_Visible))
            continue;

        mpPageObjectPainter->PaintPageObject(rDevice, pDescriptor);
    }
}




::boost::shared_ptr<cache::PageCache> SlideSorterView::GetPreviewCache (void)
{
    ::boost::shared_ptr<sd::Window> pWindow (mrSlideSorter.GetContentWindow());
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




ViewOverlay& SlideSorterView::GetOverlay (void)
{
    return *mpViewOverlay.get();
}




SlideSorterView::PageRange SlideSorterView::GetVisiblePageRange (void)
{
    const int nMaxPageIndex (mrModel.GetPageCount() - 1);
    if ( ! mbPageObjectVisibilitiesValid)
        DeterminePageObjectVisibilities();
    return PageRange(
        ::std::min(mnFirstVisiblePageIndex,nMaxPageIndex),
        ::std::min(mnLastVisiblePageIndex, nMaxPageIndex));
}




void SlideSorterView::Notify (SfxBroadcaster& rBroadcaster, const SfxHint& rHint)
{
    ::sd::DrawDocShell* pDocShell = mrModel.GetDocument()->GetDocSh();
    if (pDocShell!=NULL && pDocShell->IsEnableSetModified())
        mbModelChangedWhileModifyEnabled = true;

    ::sd::View::Notify(rBroadcaster, rHint);
}




void SlideSorterView::SetPageUnderMouse (const model::SharedPageDescriptor& rpDescriptor)
{
    if (mpPageUnderMouse != rpDescriptor)
    {
        if (mpPageUnderMouse)
        {
            mpPageUnderMouse->GetVisualState().SetActiveButtonState(
                -1,
                model::VisualState::BS_Normal);
            SetState(mpPageUnderMouse, PageDescriptor::ST_MouseOver, false);
        }

        mpPageUnderMouse = rpDescriptor;
        SetButtonUnderMouse(-1);

        if (mpPageUnderMouse)
            SetState(mpPageUnderMouse, PageDescriptor::ST_MouseOver, true);
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




void SlideSorterView::AddVisualStateAnimation (const model::SharedPageDescriptor& rpDescriptor)
{
    // Stop a state animation for the given descriptor that is still running.
    const Animator::AnimationId nId (rpDescriptor->GetVisualState().GetStateAnimationId());
    if (nId != Animator::NotAnAnimationId)
    {
        mrSlideSorter.GetController().GetAnimator()->RemoveAnimation(nId);
    }

    rpDescriptor->GetVisualState().SetStateAnimationId(
        mrSlideSorter.GetController().GetAnimator()->AddAnimation(
            ::boost::bind(
                controller::AnimationFunction::ApplyVisualStateChange,
                rpDescriptor,
                ::boost::ref(*this),
                ::boost::bind(AnimationFunction::FastInSlowOut_Sine, _1)),
            350,
            ::boost::bind(
                &VisualState::SetStateAnimationId,
                ::boost::ref(rpDescriptor->GetVisualState()),
                controller::Animator::NotAnAnimationId)));
}




bool SlideSorterView::SetState (
    const model::SharedPageDescriptor& rpDescriptor,
    const PageDescriptor::State eState,
    const bool bStateValue)
{
    const bool bModified (rpDescriptor->SetState(eState, bStateValue));
    if ( ! bModified)
        return false;

    switch(eState)
    {
        case PageDescriptor::ST_Visible:
            RequestRepaint(rpDescriptor);
            break;

        case PageDescriptor::ST_Selected:
        case PageDescriptor::ST_Focused:
        case PageDescriptor::ST_MouseOver:
        case PageDescriptor::ST_Current:
        case PageDescriptor::ST_Excluded:
            AddVisualStateAnimation(rpDescriptor);
            break;
    }

    // Fade in or out the buttons.
    if (eState == PageDescriptor::ST_MouseOver)
    {
        // Stop a running animation.
        const Animator::AnimationId nId (
            rpDescriptor->GetVisualState().GetButtonAlphaAnimationId());
        if (nId != Animator::NotAnAnimationId)
        {
            mrSlideSorter.GetController().GetAnimator()->RemoveAnimation(nId);
        }

        const double nStartAlpha (rpDescriptor->GetVisualState().GetButtonAlpha());
        const double nEndAlpha (bStateValue ? 0.2 : 1.0);
        const ::boost::function<double(double)> aBlendFunctor (
            ::boost::bind(
                AnimationFunction::Blend,
                nStartAlpha,
                nEndAlpha,
                ::boost::bind(AnimationFunction::FastInSlowOut_Root, _1)));
        rpDescriptor->GetVisualState().SetButtonAlphaAnimationId(
            mrSlideSorter.GetController().GetAnimator()->AddAnimation(
                ::boost::bind(
                    AnimationFunction::ApplyButtonAlphaChange,
                    rpDescriptor,
                    ::boost::ref(*this),
                    ::boost::bind(aBlendFunctor, _1)),
                400,
                ::boost::bind(
                    &VisualState::SetButtonAlphaAnimationId,
                    ::boost::ref(rpDescriptor->GetVisualState()),
                    controller::Animator::NotAnAnimationId)
                ));
    }

    return bModified;
}




::boost::shared_ptr<PageObjectPainter> SlideSorterView::GetPageObjectPainter (void)
{
    if ( ! mpPageObjectPainter)
        mpPageObjectPainter.reset(new PageObjectPainter(mrSlideSorter));
    return mpPageObjectPainter;
}




//===== Animator::DrawLock ====================================================

SlideSorterView::DrawLock::DrawLock (view::SlideSorterView& rView)
    : mrView(rView)
{
    mrView.LockRedraw(TRUE);
}




SlideSorterView::DrawLock::DrawLock (SlideSorter& rSlideSorter)
    : mrView(rSlideSorter.GetView())
{
    mrView.LockRedraw(TRUE);
}




SlideSorterView::DrawLock::~DrawLock (void)
{
    mrView.LockRedraw(FALSE);
}



} } } // end of namespace ::sd::slidesorter::view
