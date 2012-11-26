/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <com/sun/star/awt/XWindow.hpp>
#include <svx/svdpntv.hxx>
#include <vcl/msgbox.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svtools/grfmgr.hxx>
#include <svx/svdmodel.hxx>

#ifdef DBG_UTIL
#include <svdibrow.hxx>
#endif

#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svl/smplhint.hxx>
#include <svx/svdpntv.hxx>
#include <editeng/editdata.hxx>
#include <svx/svdmrkv.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdview.hxx>
#include <svx/svdglue.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdattrx.hxx>
#include "svdibrow.hxx"
#include <svx/svditer.hxx>
#include <svx/svdouno.hxx>
#include <svx/sdr/overlay/overlayobjectlist.hxx>
#include <svx/sdr/overlay/overlayrollingrectangle.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/svdglue.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdview.hxx>
#include <svl/itemiter.hxx>
#include <editeng/eeitem.hxx>
#include <svl/whiter.hxx>
#include <svl/style.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/svdouno.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/animation/objectanimator.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/svdlegacy.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;

////////////////////////////////////////////////////////////////////////////////////////////////////
// tooling

basegfx::B2DPoint OrthoDistance8(const basegfx::B2DPoint& rReference, const basegfx::B2DPoint& rCandidate, bool bBigOrtho)
{
    const double fDeltaX(rCandidate.getX() - rReference.getX());

    if(basegfx::fTools::equalZero(fDeltaX))
    {
        return rCandidate;
    }

    const double fDeltaY(rCandidate.getY() - rReference.getY());

    if(basegfx::fTools::equalZero(fDeltaY))
    {
        return rCandidate;
    }

    const double fAbsDeltaX(fabs(fDeltaX));
    const double fAbsDeltaY(fabs(fDeltaY));

    if(basegfx::fTools::equal(fAbsDeltaX, fAbsDeltaY))
    {
        return rCandidate;
    }

    if(basegfx::fTools::moreOrEqual(fAbsDeltaX, fAbsDeltaY * 2.0))
    {
        return basegfx::B2DPoint(rCandidate.getX(), rReference.getY());
    }

    if(basegfx::fTools::moreOrEqual(fAbsDeltaY, fAbsDeltaX * 2.0))
    {
        return basegfx::B2DPoint(rReference.getX(), rCandidate.getY());
    }

    if(basegfx::fTools::less(fAbsDeltaX, fAbsDeltaY) != bBigOrtho)
    {
        return basegfx::B2DPoint(rCandidate.getX(), rReference.getY() + basegfx::copySign(fAbsDeltaX, fDeltaY));
    }
    else
    {
        return basegfx::B2DPoint(rReference.getX() + basegfx::copySign(fAbsDeltaY, fDeltaX), rCandidate.getY());
    }
}

basegfx::B2DPoint OrthoDistance4(const basegfx::B2DPoint& rReference, const basegfx::B2DPoint& rCandidate, bool bBigOrtho)
{
    const basegfx::B2DVector aDelta(rCandidate - rReference);
    const basegfx::B2DVector aAbsDelta(absolute(aDelta));

    if(basegfx::fTools::less(aAbsDelta.getX(), aAbsDelta.getY()) != bBigOrtho)
    {
        return basegfx::B2DPoint(rCandidate.getX(), rReference.getY() + basegfx::copySign(aAbsDelta.getX(), aDelta.getY()));
    }
    else
    {
        return basegfx::B2DPoint(rReference.getX() + basegfx::copySign(aAbsDelta.getY(), aDelta.getX()), rCandidate.getY());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SvxViewHint::SvxViewHint(HintType eHintType)
:   meHintType(eHintType)
{
}

SvxViewHint::HintType SvxViewHint::GetHintType (void) const
{
    return meHintType;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

class LazyObjectChangeReactor : public Timer
{
private:
    // the SdrPaintView this incarnation belongs to
    SdrPaintView&           mrSdrPaintView;

    // derived from Timer
    virtual void Timeout()
    {
        mrSdrPaintView.LazyReactOnObjectChanges();
    }

public:
    LazyObjectChangeReactor(SdrPaintView& rSdrPaintView)
    :   Timer(),
        mrSdrPaintView(rSdrPaintView)
    {}

    void delay()
    {
        // trigger a delayed call
        SetTimeout(1);
        Start();
    }

    void force()
    {
        if(IsActive())
        {
            Stop();
            Timeout();
        }
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

BitmapEx convertMetafileToBitmapEx(
    const GDIMetaFile& rMtf,
    const basegfx::B2DRange& rTargetRange,
    const sal_uInt32 nMaximumQuadraticPixels)
{
    BitmapEx aBitmapEx;

    if(rMtf.GetActionCount())
    {
        const drawinglayer::primitive2d::Primitive2DReference aMtf(
            new drawinglayer::primitive2d::MetafilePrimitive2D(
                basegfx::tools::createScaleTranslateB2DHomMatrix(
                    rTargetRange.getRange(),
                    rTargetRange.getMinimum()),
                rMtf));
        aBitmapEx = convertPrimitive2DSequenceToBitmapEx(
            drawinglayer::primitive2d::Primitive2DSequence(&aMtf, 1),
            rTargetRange,
            nMaximumQuadraticPixels);
    }

    return aBitmapEx;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrPaintWindow* SdrPaintView::FindPaintWindow(const OutputDevice& rOut) const
{
    for(SdrPaintWindowVector::const_iterator a = maPaintWindows.begin(); a != maPaintWindows.end(); a++)
    {
        if(&((*a)->GetOutputDevice()) == &rOut)
        {
            return *a;
        }
    }

    return 0L;
}

SdrPaintWindow* SdrPaintView::GetPaintWindow(sal_uInt32 nIndex) const
{
    if(nIndex < maPaintWindows.size())
    {
        return maPaintWindows[nIndex];
    }

    return 0L;
}

void SdrPaintView::AppendPaintWindow(SdrPaintWindow& rNew)
{
    maPaintWindows.push_back(&rNew);
}

SdrPaintWindow* SdrPaintView::RemovePaintWindow(SdrPaintWindow& rOld)
{
    SdrPaintWindow* pRetval = 0L;
    const SdrPaintWindowVector::iterator aFindResult = ::std::find(maPaintWindows.begin(), maPaintWindows.end(), &rOld);

    if(aFindResult != maPaintWindows.end())
    {
        // remember return value, aFindResult is no longer valid after deletion
        pRetval = *aFindResult;
        maPaintWindows.erase(aFindResult);
    }

    return pRetval;
}

OutputDevice* SdrPaintView::GetFirstOutputDevice() const
{
    if(PaintWindowCount())
    {
        return &(GetPaintWindow(0)->GetOutputDevice());
    }

    return 0L;
}

SdrPaintView::SdrPaintView(SdrModel& rModel1, OutputDevice* pOut)
:   SfxListener(),
    SfxRepeatTarget(),
    SfxBroadcaster(),
    ::utl::ConfigurationListener(),
    mpLazyObjectChangeReactor(new LazyObjectChangeReactor(*this)),
    mrModelFromSdrView(rModel1),
#ifdef DBG_UTIL
    mpItemBrowser(0),
#endif
    mpActualOutDev(0),
    mpDefaultStyleSheet(0),
    maAktLayer(),
    maMeasureLayer(),
    mpPageView(0),
    maPaintWindows(),
    maGridBig(),
    maGridFin(),
    maDragStat(*getAsSdrView()),
    maMaxWorkArea(),
    maDefaultAttr(rModel1.GetItemPool()),
    meAnimationMode(SDR_ANIMATION_ANIMATE),
    mfHitTolPix(2.0),
    mfMinMovPix(3.0),
    maDrawinglayerOpt(),
    mbPageVisible(true),
    mbPageBorderVisible(true),
    mbBordVisible(true),
    mbGridVisible(true),
    mbGridFront(false),
    mbHlplVisible(true),
    mbHlplFront(true),
    mbGlueVisible(false),
    mbGlueVisible2(false),
    mbGlueVisible3(false),
    mbGlueVisible4(false),
    mbSwapAsynchron(false),
    mbPrintPreview(false),
    mbVisualizeEnteredGroup(true),
    mbAnimationPause(false),
    mbBufferedOutputAllowed(false),
    mbBufferedOverlayAllowed(false),
    mbPagePaintingAllowed(true),
    mbPreviewRenderer(false),
    mbHideOle(false),
    mbHideChart(false),
    mbHideDraw(false),
    mbHideFormControl(false),
    maColorConfig(),
    maGridColor(COL_BLACK)
{
    SetDefaultStyleSheet(getSdrModelFromSdrView().GetDefaultStyleSheet(), true);

    if(pOut)
    {
        AddWindowToPaintView(pOut);
    }

    maColorConfig.AddListener(this);
    onChangeColorConfig();
}

SdrPaintView::~SdrPaintView()
{
    if(mpLazyObjectChangeReactor)
    {
        delete mpLazyObjectChangeReactor;
    }

    if (GetDefaultStyleSheet())
    {
        EndListening(*GetDefaultStyleSheet());
    }

    maColorConfig.RemoveListener(this);

    // HideSdrPage() gets called in derived destructors (e.g. in SdrView::~SdrView) to have it executed
    // when the SDrView is still fully valid
    OSL_ENSURE(!mpPageView, "SdrView destruction: Call HideSdrPage() before deleting SdrView (!)");

#ifdef DBG_UTIL
    if(GetItemBrowser())
    {
        delete mpItemBrowser;
    }
#endif

    // delete existing SdrPaintWindows
    while(!maPaintWindows.empty())
    {
        delete maPaintWindows.back();
        maPaintWindows.pop_back();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void __EXPORT SdrPaintView::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    //If the stylesheet has been destroyed
    if (&rBC == GetDefaultStyleSheet())
    {
        const SfxSimpleHint* pSfxSimpleHint = dynamic_cast< const SfxSimpleHint* >(&rHint);

        if(pSfxSimpleHint && SFX_HINT_DYING == pSfxSimpleHint->GetId())
        {
            mpDefaultStyleSheet = 0;
        }

        return;
    }

    const SdrBaseHint* pSdrHint = dynamic_cast< const SdrBaseHint* >(&rHint);

    if(pSdrHint)
    {
        const SdrHintKind eKind(pSdrHint->GetSdrHintKind());

        if(HINT_OBJCHG_MOVE == eKind
            || HINT_OBJCHG_RESIZE == eKind
            || HINT_OBJCHG_ATTR == eKind
            || HINT_OBJINSERTED == eKind
            || HINT_OBJREMOVED == eKind)
        {
            mpLazyObjectChangeReactor->delay();
        }

        if(HINT_PAGEORDERCHG == eKind)
        {
            const SdrPage* pPg = pSdrHint->GetSdrHintPage();

            if(pPg && !pPg->IsInserted())
            {
                if(mpPageView && &mpPageView->getSdrPageFromSdrPageView() == pPg)
                {
                    HideSdrPage();
                }
            }
        }
    }
}

void SdrPaintView::ConfigurationChanged( ::utl::ConfigurationBroadcaster* , sal_uInt32 )
{
    onChangeColorConfig();
    InvalidateAllWin();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPaintView::ForceLazyReactOnObjectChanges() const
{
    mpLazyObjectChangeReactor->force();
}

void SdrPaintView::LazyReactOnObjectChanges()
{
    // Auch alle PageViews benachrichtigen
    if(mpPageView && !mpPageView->getSdrPageFromSdrPageView().IsInserted())
    {
        HideSdrPage();
    }

    // test mpPageView here again, HideSdrPage() may have invalidated it.
    if(mpPageView)
    {
        mpPageView->LazyReactOnObjectChanges();
    }

#ifdef DBG_UTIL
    if(GetItemBrowser())
    {
        mpItemBrowser->SetDirty();
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// info about TextEdit. Default is false.
bool SdrPaintView::IsTextEdit() const
{
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPaintView::HideSdrPage()
{
    if(mpPageView)
    {
        // deselect all
        getAsSdrView()->clearSdrObjectSelection();

        // initiate repaint
        InvalidateAllWin();

        // delete SdrPageView
        delete mpPageView;
        mpPageView = 0;
    }
}

void SdrPaintView::ShowSdrPage(SdrPage& rPage)
{
    if(!mpPageView || &mpPageView->getSdrPageFromSdrPageView() != &rPage)
    {
        HideSdrPage();

        mpPageView = new SdrPageView(rPage, *getAsSdrView());
        mpPageView->Show();
    }
}

void SdrPaintView::AddWindowToPaintView(OutputDevice* pNewWin)
{
    DBG_ASSERT(pNewWin, "SdrPaintView::AddWindowToPaintView: No OutputDevice(!)");
    SdrPaintWindow* pNewPaintWindow = new SdrPaintWindow(*this, *pNewWin);
    AppendPaintWindow(*pNewPaintWindow);

    if(mpPageView)
    {
        mpPageView->AddPaintWindowToPageView(*pNewPaintWindow);
    }

#ifdef DBG_UTIL
    if (GetItemBrowser())
    {
        mpItemBrowser->ForceParent();
    }
#endif
}

void SdrPaintView::DeleteWindowFromPaintView(OutputDevice* pOldWin)
{
    DBG_ASSERT(pOldWin, "SdrPaintView::DeleteWindowFromPaintView: No OutputDevice(!)");
    SdrPaintWindow* pCandidate = FindPaintWindow(*pOldWin);

    if(pCandidate)
    {
        if(mpPageView)
        {
            mpPageView->RemovePaintWindowFromPageView(*pCandidate);
        }

        RemovePaintWindow(*pCandidate);
        delete pCandidate;
    }

#ifdef DBG_UTIL
    if (GetItemBrowser())
    {
        mpItemBrowser->ForceParent();
    }
#endif
}

void SdrPaintView::SetLayerVisible(const XubString& rName, bool bShow)
{
    if(mpPageView)
    {
        mpPageView->SetLayerVisible(rName,bShow);
    }

    InvalidateAllWin();
}

bool SdrPaintView::IsLayerVisible(const XubString& rName) const
{
    if(mpPageView)
    {
        return mpPageView->IsLayerVisible(rName);
    }

    return false;
}

void SdrPaintView::SetAllLayersVisible(bool bShow)
{
    if(mpPageView)
    {
        mpPageView->SetAllLayersVisible(bShow);
    }

    InvalidateAllWin();
}

void SdrPaintView::SetLayerLocked(const XubString& rName, bool bLock)
{
    if(mpPageView)
    {
        mpPageView->SetLayerLocked(rName,bLock);
    }
}

bool SdrPaintView::IsLayerLocked(const XubString& rName) const
{
    if(mpPageView)
    {
        return mpPageView->IsLayerLocked(rName);
    }

    return false;
}

void SdrPaintView::SetAllLayersLocked(bool bLock)
{
    if(mpPageView)
    {
        mpPageView->SetAllLayersLocked(bLock);
    }
}

void SdrPaintView::SetLayerPrintable(const XubString& rName, bool bPrn)
{
    if(mpPageView)
    {
        mpPageView->SetLayerPrintable(rName,bPrn);
    }
}

bool SdrPaintView::IsLayerPrintable(const XubString& rName) const
{
    if(mpPageView)
    {
        return mpPageView->IsLayerPrintable(rName);
    }

    return false;
}

void SdrPaintView::SetAllLayersPrintable(bool bPrn)
{
    if(mpPageView)
    {
        mpPageView->SetAllLayersPrintable(bPrn);
    }
}

void SdrPaintView::PrePaint()
{
    if(mpPageView)
    {
        mpPageView->PrePaint();
    }
}

void SdrPaintView::PostPaint()
{
    if(mpPageView)
    {
        mpPageView->PostPaint();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// #define SVX_REPAINT_TIMER_TEST

void SdrPaintView::CompleteRedraw(OutputDevice* pOut, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector)
{
#ifdef SVX_REPAINT_TIMER_TEST
#define REMEMBERED_TIMES_COUNT  (10)
    static bool bDoTimerTest(false);
    static bool bTimesInited(false);
    static sal_uInt32 nRepeatCount(10L);
    static double fLastTimes[REMEMBERED_TIMES_COUNT];
    const sal_uInt32 nStartTime(Time::GetSystemTicks());
    sal_uInt32 count(1L);
    sal_uInt32 a;

    if(bDoTimerTest)
    {
        count = nRepeatCount;
    }

    for(a = 0L; a < count; a++)
    {
#endif // SVX_REPAINT_TIMER_TEST

    // #i74769# check if pOut is a win and has a ClipRegion. If Yes, the Region
    // rReg may be made more granular (fine) with using it. Normally, rReg
    // does come from Window::Paint() anyways and thus is based on a single
    // rectangle which was derived from exactly that repaint region
    Region aOptimizedRepaintRegion(rReg);

    if(pOut && OUTDEV_WINDOW == pOut->GetOutDevType())
    {
        Window* pWindow = (Window*)pOut;

        if(pWindow->IsInPaint())
        {
            if(!pWindow->GetPaintRegion().IsEmpty())
            {
                aOptimizedRepaintRegion.Intersect(pWindow->GetPaintRegion());

#ifdef DBG_UTIL
                // #i74769# test-paint repaint region
                static bool bDoPaintForVisualControl(false);

                if(bDoPaintForVisualControl)
                {
                    RectangleVector aRectangles;
                    aOptimizedRepaintRegion.GetRegionRectangles(aRectangles);

                    pWindow->SetLineColor(COL_LIGHTGREEN);
                    pWindow->SetFillColor();

                    for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); aRectIter++)
                    {
                        pWindow->DrawRect(*aRectIter);
                    }

                    //RegionHandle aRegionHandle(aOptimizedRepaintRegion.BeginEnumRects());
                    //Rectangle aRegionRectangle;
                    //
                    //while(aOptimizedRepaintRegion.GetEnumRects(aRegionHandle, aRegionRectangle))
                    //{
                    //  pWindow->SetLineColor(COL_LIGHTGREEN);
                    //  pWindow->SetFillColor();
                    //  pWindow->DrawRect(aRegionRectangle);
                    //}
                    //
                    //aOptimizedRepaintRegion.EndEnumRects(aRegionHandle);
                }
#endif
            }
        }
    }

    SdrPaintWindow* pPaintWindow = BeginCompleteRedraw(pOut);
    OSL_ENSURE(pPaintWindow, "SdrPaintView::CompleteRedraw: No OutDev (!)");

    DoCompleteRedraw(*pPaintWindow, aOptimizedRepaintRegion, pRedirector);
    EndCompleteRedraw(*pPaintWindow, true);

#ifdef SVX_REPAINT_TIMER_TEST
    }

    if(bDoTimerTest)
    {
        const sal_uInt32 nStopTime(Time::GetSystemTicks());
        const sal_uInt32 nNeededTime(nStopTime - nStartTime);
        const double fTimePerPaint((double)nNeededTime / (double)nRepeatCount);

        if(!bTimesInited)
        {
            for(a = 0L; a < REMEMBERED_TIMES_COUNT; a++)
            {
                fLastTimes[a] = fTimePerPaint;
            }

            bTimesInited = true;
        }
        else
        {
            for(a = 1L; a < REMEMBERED_TIMES_COUNT; a++)
            {
                fLastTimes[a - 1L] = fLastTimes[a];
            }

            fLastTimes[REMEMBERED_TIMES_COUNT - 1L] = fTimePerPaint;
        }

        double fAddedTimes(0.0);

        for(a = 0L; a < REMEMBERED_TIMES_COUNT; a++)
        {
            fAddedTimes += fLastTimes[a];
        }

        const double fAverageTimePerPaint(fAddedTimes / (double)REMEMBERED_TIMES_COUNT);

        fprintf(stderr, "-----------(start result)----------\n");
        fprintf(stderr, "StartTime : %u, StopTime: %u, NeededTime: %u, TimePerPaint: %f\n", nStartTime, nStopTime, nNeededTime, fTimePerPaint);
        fprintf(stderr, "Remembered times: ");

        for(a = 0L; a < REMEMBERED_TIMES_COUNT; a++)
        {
            fprintf(stderr, "%d: %f ", a, fLastTimes[a]);
        }

        fprintf(stderr, "\n");
        fprintf(stderr, "AverageTimePerPaint: %f\n", fAverageTimePerPaint);
        fprintf(stderr, "-----------(stop result)----------\n");
    }
#endif // SVX_REPAINT_TIMER_TEST
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// #i72889#

SdrPaintWindow* SdrPaintView::BeginCompleteRedraw(OutputDevice* pOut)
{
    OSL_ENSURE(pOut, "SdrPaintView::BeginCompleteRedraw: No OutDev (!)");
    SdrPaintWindow* pPaintWindow = FindPaintWindow(*pOut);

    if(pPaintWindow)
    {
        // draw preprocessing, only for known devices
        // prepare PreRendering
        pPaintWindow->PreparePreRenderDevice();
    }
    else
    {
        // None of the known OutputDevices is the target of this paint, use
        // a temporary SdrPaintWindow for this Redraw.
        pPaintWindow = new SdrPaintWindow(*this, *pOut);
        pPaintWindow->setTemporaryTarget(true);
    }

    return pPaintWindow;
}

void SdrPaintView::DoCompleteRedraw(SdrPaintWindow& rPaintWindow, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector)
{
    // redraw all PageViews with the target. This may expand the RedrawRegion
    // at the PaintWindow, plus taking care of FormLayer expansion
    if(mpPageView)
    {
        mpPageView->CompleteRedraw(rPaintWindow, rReg, pRedirector);
    }
}

void SdrPaintView::EndCompleteRedraw(SdrPaintWindow& rPaintWindow, bool bPaintFormLayer)
{
    if(rPaintWindow.getTemporaryTarget())
    {
        // get rid of temp target again
        delete (&rPaintWindow);
    }
    else
    {
        // draw postprocessing, only for known devices
        // it is necessary to always paint FormLayer
        if(bPaintFormLayer)
        {
            ImpFormLayerDrawing(rPaintWindow);
        }

        // look for active TextEdit. As long as this cannot be painted to a VDev,
        // it cannot get part of buffering. In that case, output evtl. prerender
        // early and paint text edit to window.
        if(IsTextEdit() && GetSdrPageView())
        {
            static_cast< SdrView* >(this)->TextEditDrawing(rPaintWindow);
        }

        // draw Overlay, also to PreRender device if exists
        rPaintWindow.DrawOverlay(rPaintWindow.GetRedrawRegion());

        // output PreRendering
        rPaintWindow.OutputPreRenderDevice(rPaintWindow.GetRedrawRegion());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrPaintWindow* SdrPaintView::BeginDrawLayers(OutputDevice* pOut, const Region& rReg, bool bDisableIntersect)
{
    // #i74769# use BeginCompleteRedraw() as common base
    SdrPaintWindow* pPaintWindow = BeginCompleteRedraw(pOut);
    OSL_ENSURE(pPaintWindow, "SdrPaintView::BeginDrawLayers: No SdrPaintWindow (!)");

    if(mpPageView)
    {
        SdrPageWindow* pKnownTarget = mpPageView->FindPageWindow(*pPaintWindow);

        if(pKnownTarget)
        {
            // #i74769# check if pOut is a win and has a ClipRegion. If Yes, the Region
            // rReg may be made more granular (fine) with using it. Normally, rReg
            // does come from Window::Paint() anyways and thus is based on a single
            // rectangle which was derived from exactly that repaint region
            Region aOptimizedRepaintRegion(rReg);

            // #i76114# Intersecting the region with the Window's paint region is disabled
            // for print preview in Calc, because the intersection can be empty (if the paint
            // region is outside of the table area of the page), and then no clip region
            // would be set.
            if(pOut && OUTDEV_WINDOW == pOut->GetOutDevType() && !bDisableIntersect)
            {
                Window* pWindow = (Window*)pOut;

                if(pWindow->IsInPaint())
                {
                    if(!pWindow->GetPaintRegion().IsEmpty())
                    {
                        aOptimizedRepaintRegion.Intersect(pWindow->GetPaintRegion());

#ifdef DBG_UTIL
                        // #i74769# test-paint repaint region
                        static bool bDoPaintForVisualControl(false);

                        if(bDoPaintForVisualControl)
                        {
                            RectangleVector aRectangles;
                            aOptimizedRepaintRegion.GetRegionRectangles(aRectangles);

                            pWindow->SetLineColor(COL_LIGHTGREEN);
                            pWindow->SetFillColor();

                            for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); aRectIter++)
                            {
                                pWindow->DrawRect(*aRectIter);
                            }

                            //RegionHandle aRegionHandle(aOptimizedRepaintRegion.BeginEnumRects());
                            //Rectangle aRegionRectangle;
                            //
                            //while(aOptimizedRepaintRegion.GetEnumRects(aRegionHandle, aRegionRectangle))
                            //{
                            //  pWindow->SetLineColor(COL_LIGHTGREEN);
                            //  pWindow->SetFillColor();
                            //  pWindow->DrawRect(aRegionRectangle);
                            //}
                            //
                            //aOptimizedRepaintRegion.EndEnumRects(aRegionHandle);
                        }
#endif
                    }
                }
            }

            // prepare redraw
            pKnownTarget->PrepareRedraw(aOptimizedRepaintRegion);

            // remember prepared SdrPageWindow
            mpPageView->setPreparedPageWindow(pKnownTarget);
        }
    }

    return pPaintWindow;
}

void SdrPaintView::EndDrawLayers(SdrPaintWindow& rPaintWindow, bool bPaintFormLayer)
{
    // #i74769# use EndCompleteRedraw() as common base
    EndCompleteRedraw(rPaintWindow, bPaintFormLayer);

    if(mpPageView)
    {
        // forget prepared SdrPageWindow
        mpPageView->setPreparedPageWindow(0);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPaintView::ImpFormLayerDrawing(SdrPaintWindow& rPaintWindow) const
{
    if(mpPageView)
    {
        SdrPageWindow* pKnownTarget = mpPageView->FindPageWindow(rPaintWindow);

        if(pKnownTarget)
        {
            const SdrLayerAdmin& rLayerAdmin = getSdrModelFromSdrView().GetModelLayerAdmin();
            const SdrLayerID nControlLayerId = rLayerAdmin.GetLayerID(rLayerAdmin.GetControlLayerName(), false);

            // BUFFERED use GetTargetOutputDevice() now, it may be targeted to VDevs, too
            // need to set PreparedPageWindow to make DrawLayer use the correct ObjectContact
            mpPageView->setPreparedPageWindow(pKnownTarget);
            mpPageView->DrawLayer(nControlLayerId, &rPaintWindow.GetTargetOutputDevice());
            mpPageView->setPreparedPageWindow(0);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrPaintView::KeyInput(const KeyEvent& /*rKEvt*/, Window* /*pWin*/)
{
    return false;
}

void SdrPaintView::GlueInvalidate() const
{
    const sal_uInt32 nWindowCount(PaintWindowCount());

    for(sal_uInt32 nWinNum(0); nWinNum < nWindowCount; nWinNum++)
    {
        SdrPaintWindow* pPaintWindow = GetPaintWindow(nWinNum);

        if(pPaintWindow->OutputToWindow())
        {
            OutputDevice& rOutDev = pPaintWindow->GetOutputDevice();
            const basegfx::B2DVector aLogicHalfSevenPix(rOutDev.GetInverseViewTransformation() * basegfx::B2DVector(3.5, 3.5));

            if(mpPageView)
            {
                const SdrObjList* pOL = mpPageView->GetCurrentObjectList();
                const sal_uInt32 nObjAnz(pOL->GetObjCount());

                for(sal_uInt32 nObjNum(0); nObjNum < nObjAnz; nObjNum++)
                {
                    const SdrObject* pObj=pOL->GetObj(nObjNum);
                    const SdrGluePointList* pGPL=pObj->GetGluePointList();

                    if(pGPL && pGPL->GetCount())
                    {
                        const basegfx::B2DRange aObjectRange(sdr::legacy::GetSnapRange(*pObj));

                        for(sal_uInt32 a(0); a < pGPL->GetCount(); a++)
                        {
                            const SdrGluePoint& rCandidate = (*pGPL)[a];
                            const basegfx::B2DPoint aPos(rCandidate.GetAbsolutePos(aObjectRange));
                            const basegfx::B2DRange aRange(aPos - aLogicHalfSevenPix, aPos + aLogicHalfSevenPix);

                            InvalidateOneWin((Window&)rOutDev, aRange);
                        }
                    }
                }
            }
        }
    }
}

void SdrPaintView::InvalidateAllWin() const
{
    const sal_uInt32 nWindowCount(PaintWindowCount());

    for(sal_uInt32 a(0); a < nWindowCount; a++)
    {
        SdrPaintWindow* pPaintWindow = GetPaintWindow(a);

        if(pPaintWindow->OutputToWindow())
        {
            InvalidateOneWin((Window&)pPaintWindow->GetOutputDevice());
        }
    }
}

void SdrPaintView::InvalidateAllWin(const basegfx::B2DRange& rRange, bool bPlus1Pix) const
{
    const sal_uInt32 nWindowCount(PaintWindowCount());

    for(sal_uInt32 a(0); a < nWindowCount; a++)
    {
        SdrPaintWindow* pPaintWindow = GetPaintWindow(a);

        if(pPaintWindow->OutputToWindow())
        {
            OutputDevice& rOutDev = pPaintWindow->GetOutputDevice();
            basegfx::B2DRange aRange(rRange);

            if(bPlus1Pix)
            {
                const basegfx::B2DVector aLogicPixel(rOutDev.GetInverseViewTransformation() * basegfx::B2DVector(1.0, 1.0));

                aRange.grow((aLogicPixel.getX() + aLogicPixel.getY()) * 0.5);
            }

            const basegfx::B2DRange aViewRange(rOutDev.GetLogicRange());

            if(aRange.overlaps(aViewRange))
            {
                InvalidateOneWin((Window&)rOutDev, aRange);
            }
        }
    }
}

void SdrPaintView::InvalidateOneWin(Window& rWin) const
{
    // #111096#
    // do not erase background, that causes flicker (!)
    rWin.Invalidate(INVALIDATE_NOERASE);
}

void SdrPaintView::InvalidateOneWin(Window& rWin, const basegfx::B2DRange& rRange) const
{
    const Rectangle aRectangle(
        (sal_Int32)floor(rRange.getMinX()), (sal_Int32)floor(rRange.getMinY()),
        (sal_Int32)ceil(rRange.getMaxX()), (sal_Int32)ceil(rRange.getMaxY()));

    // do not erase background, that causes flicker (!)
    rWin.Invalidate(aRectangle, INVALIDATE_NOERASE);
}

void SdrPaintView::LeaveOneGroup()
{
    if(mpPageView)
    {
        mpPageView->LeaveOneGroup();
    }
}

void SdrPaintView::LeaveAllGroup()
{
    if(mpPageView)
    {
        mpPageView->LeaveAllGroup();
    }
}

bool SdrPaintView::IsGroupEntered() const
{
    if(mpPageView)
    {
        return (0 != mpPageView->GetEnteredLevel());
    }

    return false;
}

void SdrPaintView::SetNotPersistDefaultAttr(const SfxItemSet& rAttr, bool /*bReplaceAll*/)
{
    // bReplaceAll hat hier keinerlei Wirkung
    const bool bMeasure(static_cast< SdrView* >(this)->IsMeasureTool());
    const SfxPoolItem *pPoolItem = 0;

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_LAYERID, true, &pPoolItem))
    {
        const SdrLayerID nLayerId = SdrLayerID(((const SfxUInt16Item*)pPoolItem)->GetValue());
        const SdrLayer* pLayer = getSdrModelFromSdrView().GetModelLayerAdmin().GetLayerPerID(nLayerId);

        if(pLayer)
        {
            if(bMeasure)
            {
                maMeasureLayer = pLayer->GetName();
            }
            else
            {
                SetActiveLayer(pLayer->GetName());
            }
        }
    }

    if(SFX_ITEM_SET == rAttr.GetItemState(SDRATTR_LAYERNAME, true, &pPoolItem))
    {
        if(bMeasure)
        {
            maMeasureLayer = ((const SfxStringItem*)pPoolItem)->GetValue();
        }
        else
        {
            SetActiveLayer(((const SfxStringItem*)pPoolItem)->GetValue());
        }
    }
}

void SdrPaintView::MergeNotPersistDefaultAttr(SfxItemSet& rAttr, bool /*bOnlyHardAttr*/) const
{
    // bOnlyHardAttr hat hier keinerlei Wirkung
    const bool bMeasure(static_cast< const SdrView* >(this)->IsMeasureTool());
    const XubString& aNam = bMeasure ? maMeasureLayer : GetActiveLayer();

    rAttr.Put(SfxStringItem(SDRATTR_LAYERNAME, aNam));
    const SdrLayerID nLayer(getSdrModelFromSdrView().GetModelLayerAdmin().GetLayerID(aNam, true));

    if(SDRLAYER_NOTFOUND != nLayer)
    {
        rAttr.Put(SfxUInt16Item(SDRATTR_LAYERID, nLayer));
    }
}

void SdrPaintView::SetDefaultAttr(const SfxItemSet& rAttr, bool bReplaceAll)
{
#ifdef DBG_UTIL
    {
        bool bHasEEFeatureItems=false;
        SfxItemIter aIter(rAttr);
        const SfxPoolItem* pItem=aIter.FirstItem();
        while (!bHasEEFeatureItems && pItem!=NULL) {
            if (!IsInvalidItem(pItem)) {
                sal_uInt16 nW=pItem->Which();
                if (nW>=EE_FEATURE_START && nW<=EE_FEATURE_END) bHasEEFeatureItems=true;
            }
            pItem=aIter.NextItem();
        }

        if(bHasEEFeatureItems)
        {
            String aMessage;
            aMessage.AppendAscii("SdrPaintView::SetDefaultAttr(): Das setzen von EE_FEATURE-Items an der SdrView macht keinen Sinn! Es fuehrt nur zu Overhead und nicht mehr lesbaren Dokumenten.");
            InfoBox(NULL, aMessage).Execute();
        }
    }
#endif
    if(bReplaceAll)
    {
        maDefaultAttr.Set(rAttr);
    }
    else
    {
        maDefaultAttr.Put(rAttr, false); // false= InvalidItems nicht als Default, sondern als "Loecher" betrachten
    }

    SetNotPersistDefaultAttr(rAttr,bReplaceAll);

#ifdef DBG_UTIL
    if (GetItemBrowser())
    {
        mpItemBrowser->SetDirty();
    }
#endif
}

void SdrPaintView::SetDefaultStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr)
{
    if (GetDefaultStyleSheet())
    {
        EndListening(*GetDefaultStyleSheet());
    }

    mpDefaultStyleSheet = pStyleSheet;

    if (GetDefaultStyleSheet())
    {
        StartListening(*GetDefaultStyleSheet());
    }

    if(pStyleSheet && !bDontRemoveHardAttr)
    {
        SfxWhichIter aIter(pStyleSheet->GetItemSet());
        sal_uInt16 nWhich=aIter.FirstWhich();

        while(nWhich)
        {
            if(SFX_ITEM_SET == pStyleSheet->GetItemSet().GetItemState(nWhich, true))
            {
                maDefaultAttr.ClearItem(nWhich);
            }

            nWhich=aIter.NextWhich();
        }
    }

#ifdef DBG_UTIL
    if(GetItemBrowser())
    {
        mpItemBrowser->SetDirty();
    }
#endif
}

/* new interface src537 */
bool SdrPaintView::GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const
{
    if(bOnlyHardAttr || !GetDefaultStyleSheet())
    {
        rTargetSet.Put(GetDefaultAttr(), false);
    }
    else
    {
        // sonst DefStyleSheet dazumergen
        rTargetSet.Put(GetDefaultStyleSheet()->GetItemSet(), false);
        rTargetSet.Put(GetDefaultAttr(), false);
    }

    MergeNotPersistDefaultAttr(rTargetSet, bOnlyHardAttr);

    return true;
}

bool SdrPaintView::SetAttributes(const SfxItemSet& rSet, bool bReplaceAll)
{
    SetDefaultAttr(rSet,bReplaceAll);

    return true;
}

SfxStyleSheet* SdrPaintView::GetStyleSheet() const // SfxStyleSheet* SdrPaintView::GetStyleSheet(bool& rOk) const
{
    return GetDefaultStyleSheet();
}

bool SdrPaintView::SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr)
{
    SetDefaultStyleSheet(pStyleSheet,bDontRemoveHardAttr);

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DBG_UTIL
void SdrPaintView::ShowItemBrowser(bool bShow)
{
    if (bShow) {
        if (!GetItemBrowser()) {
            mpItemBrowser = new SdrItemBrowser(*getAsSdrView());
            mpItemBrowser->SetFloatingMode(true);
        }
        mpItemBrowser->Show();
        mpItemBrowser->GrabFocus();
    } else {
        if (GetItemBrowser()) {
            mpItemBrowser->Hide();
            delete mpItemBrowser;
            mpItemBrowser = 0;
        }
    }
}
#endif

void SdrPaintView::MakeVisibleAtView(const basegfx::B2DRange& rRange, Window& rWin)
{
    Size aActualSize(rWin.GetOutputSize());

    if( aActualSize.Height() > 0 && aActualSize.Width() > 0 )
    {
        const Rectangle aRect(
                (sal_Int32)floor(rRange.getMinX()), (sal_Int32)floor(rRange.getMinY()),
                (sal_Int32)ceil(rRange.getMaxX()), (sal_Int32)ceil(rRange.getMaxY()));

        Size aNewSize(aRect.GetSize());
        MapMode aMap(rWin.GetMapMode());

        const bool bNeedMoreX(aNewSize.Width() > aActualSize.Width());
        const bool bNeedMoreY(aNewSize.Height() > aActualSize.Height());
        const bool bNewScale(bNeedMoreX || bNeedMoreY);

        if(bNewScale)
        {
            // Neuen MapMode (Size+Org) setzen und dabei alles invalidieren
            Fraction aXFact(aNewSize.Width(),aActualSize.Width());
            Fraction aYFact(aNewSize.Height(),aActualSize.Height());
            if (aYFact>aXFact) aXFact=aYFact;
            aXFact*=aMap.GetScaleX();
            aXFact.ReduceInaccurate(10); // Um Ueberlaeufe und BigInt-Mapping zu vermeiden
            aMap.SetScaleX(aXFact);
            aMap.SetScaleY(aYFact);
            rWin.SetMapMode(aMap);
            aActualSize=rWin.GetOutputSize();
        }

        Point aOrg(aMap.GetOrigin());
        long dx=0,dy=0;
        long l=-aOrg.X();
        long r=-aOrg.X()+aActualSize.Width()-1;
        long o=-aOrg.Y();
        long u=-aOrg.Y()+aActualSize.Height()-1;

        if (l>aRect.Left()) dx=aRect.Left()-l;
        else if (r<aRect.Right()) dx=aRect.Right()-r;
        if (o>aRect.Top()) dy=aRect.Top()-o;
        else if (u<aRect.Bottom()) dy=aRect.Bottom()-u;

        aMap.SetOrigin(Point(aOrg.X()-dx,aOrg.Y()-dy));

        if (!bNewScale)
        {
            if (dx!=0 || dy!=0)
            {
                rWin.Scroll(-dx,-dy);
                rWin.SetMapMode(aMap);
                rWin.Update();
            }
        }
        else
        {
            rWin.SetMapMode(aMap);
            InvalidateOneWin(rWin);
        }
    }
}

void SdrPaintView::DoConnect(SdrOle2Obj* /*pOleObj*/)
{
}

void SdrPaintView::SetAnimationEnabled( bool bEnable )
{
    SetAnimationMode( bEnable ? SDR_ANIMATION_ANIMATE : SDR_ANIMATION_DISABLE );
}

void SdrPaintView::SetAnimationPause( bool bSet )
{
    if(mbAnimationPause != bSet)
    {
        mbAnimationPause = bSet;

        if(mpPageView)
        {
            for(sal_uInt32 b(0L); b < mpPageView->PageWindowCount(); b++)
            {
                const SdrPageWindow& rPageWindow = *(mpPageView->GetPageWindow(b));
                sdr::contact::ObjectContact& rObjectContact = rPageWindow.GetObjectContact();
                sdr::animation::primitiveAnimator& rAnimator = rObjectContact.getPrimitiveAnimator();

                if(rAnimator.IsPaused() != bSet)
                {
                    rAnimator.SetPaused(bSet);
                }
            }
        }
    }
}

void SdrPaintView::SetAnimationMode( const SdrAnimationMode eMode )
{
    if(meAnimationMode != eMode)
    {
        meAnimationMode = eMode;
    }
}

void SdrPaintView::VisAreaChanged(const OutputDevice* pOut)
{
    if(mpPageView)
    {
        if (pOut)
        {
            SdrPageWindow* pWindow = mpPageView->FindPageWindow(*((OutputDevice*)pOut));

            if(pWindow)
            {
                Broadcast(SvxViewHint(SvxViewHint::SVX_HINT_VIEWCHANGED));
            }
        }
        else
        {
            if(mpPageView->PageWindowCount())
            {
                Broadcast(SvxViewHint(SvxViewHint::SVX_HINT_VIEWCHANGED));
            }
        }
    }
}

const svtools::ColorConfig& SdrPaintView::getColorConfig() const
{
    return maColorConfig;
}

void SdrPaintView::onChangeColorConfig()
{
    SetGridColor( Color( maColorConfig.GetColorValue( svtools::DRAWGRID ).nColor ) );
}

void SdrPaintView::SetGridColor( Color aColor )
{
    maGridColor = aColor;
}

Color SdrPaintView::GetGridColor() const
{
    return maGridColor;
}

// #103834# Set background color for svx at SdrPageViews
void SdrPaintView::SetApplicationBackgroundColor(Color aBackgroundColor)
{
    if(mpPageView)
    {
        mpPageView->SetApplicationBackgroundColor(aBackgroundColor);
    }
}

// #103911# Set document color for svx at SdrPageViews
void SdrPaintView::SetApplicationDocumentColor(Color aDocumentColor)
{
    if(mpPageView)
    {
        mpPageView->SetApplicationDocumentColor(aDocumentColor);
    }
}

// #114898#
bool SdrPaintView::IsBufferedOutputAllowed() const
{
    return (mbBufferedOutputAllowed && maDrawinglayerOpt.IsPaintBuffer());
}

// #114898#
void SdrPaintView::SetBufferedOutputAllowed(bool bNew)
{
    if(bNew != mbBufferedOutputAllowed)
    {
        mbBufferedOutputAllowed = bNew;
    }
}

bool SdrPaintView::IsBufferedOverlayAllowed() const
{
    return (mbBufferedOverlayAllowed && maDrawinglayerOpt.IsOverlayBuffer());
}

void SdrPaintView::SetBufferedOverlayAllowed(bool bNew)
{
    if(bNew != mbBufferedOverlayAllowed)
    {
        mbBufferedOverlayAllowed = bNew;
    }
}

bool SdrPaintView::IsPagePaintingAllowed() const
{
    return mbPagePaintingAllowed;
}

void SdrPaintView::SetPagePaintingAllowed(bool bNew)
{
    if(bNew != mbPagePaintingAllowed)
    {
        mbPagePaintingAllowed = bNew;
    }
}

// #i38135# Sets the timer for Object animations and restarts.
void SdrPaintView::SetAnimationTimer(sal_uInt32 nTime)
{
    if(mpPageView)
    {
        // first, reset all timers at all windows to 0
        for(sal_uInt32 a(0); a < mpPageView->PageWindowCount(); a++)
        {
            const SdrPageWindow& rPageWindow = *mpPageView->GetPageWindow(a);
            sdr::contact::ObjectContact& rObjectContact = rPageWindow.GetObjectContact();
            sdr::animation::primitiveAnimator& rAnimator = rObjectContact.getPrimitiveAnimator();

            rAnimator.SetTime(nTime);
        }
    }
}

double SdrPaintView::getHitTolLog() const
{
    if(GetActualOutDev())
    {
        return basegfx::B2DVector(GetActualOutDev()->GetInverseViewTransformation() * basegfx::B2DVector(mfHitTolPix, 0.0)).getLength();
    }

    return 0.0;
}

double SdrPaintView::getMinMovLog() const
{
    if(GetActualOutDev())
    {
        return basegfx::B2DVector(GetActualOutDev()->GetInverseViewTransformation() * basegfx::B2DVector(mfMinMovPix, 0.0)).getLength();
    }

    return 0.0;
}

void SdrPaintView::handleSelectionChange()
{
    // nothing to do in base class
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
