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

#include <memory>
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

#include <editeng/editdata.hxx>
#include <svx/svdmrkv.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdview.hxx>
#include <svx/svdglue.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdograf.hxx>
#include "svdibrow.hxx"
#include "svx/svditer.hxx"
#include <svx/svdouno.hxx>
#include <svx/sdr/overlay/overlayobjectlist.hxx>
#include <sdr/overlay/overlayrollingrectangle.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <sxlayitm.hxx>
#include <svl/itemiter.hxx>
#include <editeng/eeitem.hxx>
#include <svl/whiter.hxx>
#include <svl/style.hxx>
#include <svx/sdrpagewindow.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/animation/objectanimator.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <comphelper/lok.hxx>
#include <svx/svdviter.hxx>

using namespace ::com::sun::star;

// interface to SdrPaintWindow

SdrPaintWindow* SdrPaintView::FindPaintWindow(const OutputDevice& rOut) const
{
    for(SdrPaintWindowVector::const_iterator a = maPaintWindows.begin(); a != maPaintWindows.end(); ++a)
    {
        if(&((*a)->GetOutputDevice()) == &rOut)
        {
            return *a;
        }
    }

    return nullptr;
}

SdrPaintWindow* SdrPaintView::GetPaintWindow(sal_uInt32 nIndex) const
{
    if(nIndex < maPaintWindows.size())
    {
        return maPaintWindows[nIndex];
    }

    return nullptr;
}

void SdrPaintView::AppendPaintWindow(SdrPaintWindow& rNew)
{
    maPaintWindows.push_back(&rNew);
}

SdrPaintWindow* SdrPaintView::RemovePaintWindow(SdrPaintWindow& rOld)
{
    SdrPaintWindow* pRetval = nullptr;
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

    return nullptr;
}


SvxViewHint::SvxViewHint (HintType eHintType)
    : meHintType(eHintType)
{
}


BitmapEx convertMetafileToBitmapEx(
    const GDIMetaFile& rMtf,
    const basegfx::B2DRange& rTargetRange,
    const sal_uInt32 nMaximumQuadraticPixels)
{
    BitmapEx aBitmapEx;

    if(rMtf.GetActionSize())
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


void SdrPaintView::ImpClearVars()
{
#ifdef DBG_UTIL
    mpItemBrowser=nullptr;
#endif
    mbPageVisible=true;
    mbPageShadowVisible=true;
    mbPageBorderVisible=true;
    mbBordVisible=true;
    mbGridVisible=true;
    mbGridFront  =false;
    mbHlplVisible=true;
    mbHlplFront  =true;
    mbGlueVisible=false;
    mbGlueVisible2=false;
    mbGlueVisible3=false;
    mbGlueVisible4=false;
    mbSwapAsynchron=false;
    mbPrintPreview=false;
    mbPreviewRenderer=false;

    meAnimationMode = SDR_ANIMATION_ANIMATE;
    mbAnimationPause = false;

    mnHitTolPix=2;
    mnMinMovPix=3;
    mnHitTolLog=0;
    mnMinMovLog=0;
    mpActualOutDev=nullptr;
    mpDragWin=nullptr;
    mbRestoreColors=true;
    mpDefaultStyleSheet=nullptr;
    mbSomeObjChgdFlag=false;
    mnGraphicManagerDrawMode = GraphicManagerDrawFlags::STANDARD;
    maComeBackIdle.SetPriority(SchedulerPriority::REPAINT);
    maComeBackIdle.SetIdleHdl(LINK(this,SdrPaintView,ImpComeBackHdl));

    if (mpModel)
        SetDefaultStyleSheet(mpModel->GetDefaultStyleSheet(), true);

    maGridColor = Color( COL_BLACK );
}

SdrPaintView::SdrPaintView(SdrModel* pModel, OutputDevice* pOut)
:   mpPageView(nullptr),
    maDefaultAttr(pModel->GetItemPool()),
    mbBufferedOutputAllowed(false),
    mbBufferedOverlayAllowed(false),
    mbPagePaintingAllowed(true),
    mbHideOle(false),
    mbHideChart(false),
    mbHideDraw(false),
    mbHideFormControl(false)
{
    mpModel=pModel;
    ImpClearVars();

    if(pOut)
    {
        AddWindowToPaintView(pOut, nullptr);
    }

    // flag to visualize groups
    mbVisualizeEnteredGroup = true;

    maColorConfig.AddListener(this);
    onChangeColorConfig();
}

SdrPaintView::~SdrPaintView()
{
    if (mpDefaultStyleSheet)
        EndListening(*mpDefaultStyleSheet);

    maColorConfig.RemoveListener(this);
    ClearPageView();

#ifdef DBG_UTIL
    mpItemBrowser.disposeAndClear();
#endif

    // delete existing SdrPaintWindows
    while(!maPaintWindows.empty())
    {
        delete maPaintWindows.back();
        maPaintWindows.pop_back();
    }
}


void SdrPaintView::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    //If the stylesheet has been destroyed
    if (&rBC == mpDefaultStyleSheet)
    {
        const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
        if (pSimpleHint && pSimpleHint->GetId() == SFX_HINT_DYING)
            mpDefaultStyleSheet = nullptr;
        return;
    }

    const SdrHint* pSdrHint = dynamic_cast<const SdrHint*>(&rHint);
    if (!pSdrHint)
        return;

    SdrHintKind eKind = pSdrHint->GetKind();
    if (eKind==HINT_OBJCHG || eKind==HINT_OBJINSERTED || eKind==HINT_OBJREMOVED)
    {
        bool bObjChg = !mbSomeObjChgdFlag; // if true, evaluate for ComeBack timer
        if (bObjChg)
        {
            mbSomeObjChgdFlag=true;
            maComeBackIdle.Start();
        }
    }

    if (eKind==HINT_PAGEORDERCHG)
    {
        const SdrPage* pPg=pSdrHint->GetPage();
        if (pPg && !pPg->IsInserted())
        {
            if(mpPageView && mpPageView->GetPage() == pPg)
            {
                HideSdrPage();
            }
        }
    }
}

void SdrPaintView::ConfigurationChanged( ::utl::ConfigurationBroadcaster* , sal_uInt32 )
{
    onChangeColorConfig();
    InvalidateAllWin();
}

IMPL_LINK_NOARG_TYPED(SdrPaintView, ImpComeBackHdl, Idle *, void)
{
    if (mbSomeObjChgdFlag) {
        mbSomeObjChgdFlag=false;
        ModelHasChanged();
    }
}

void SdrPaintView::FlushComeBackTimer() const
{
    if (mbSomeObjChgdFlag) {
        // casting to nonconst
        const_cast<SdrPaintView*>(this)->ImpComeBackHdl(&const_cast<SdrPaintView*>(this)->maComeBackIdle);
        const_cast<SdrPaintView*>(this)->maComeBackIdle.Stop();
    }
}

void SdrPaintView::ModelHasChanged()
{
    // broadcast to all PageViews
    if(mpPageView && !mpPageView->GetPage()->IsInserted())
    {
        HideSdrPage();
    }

    // test mpPageView here again, HideSdrPage() may have invalidated it.
    if(mpPageView)
    {
        mpPageView->ModelHasChanged();
    }

#ifdef DBG_UTIL
    if(mpItemBrowser)
    {
        mpItemBrowser->SetDirty();
    }
#endif
}


bool SdrPaintView::IsAction() const
{
    return false;
}

void SdrPaintView::MovAction(const Point&)
{
}

void SdrPaintView::EndAction()
{
}

void SdrPaintView::BckAction()
{
}

void SdrPaintView::BrkAction()
{
}

void SdrPaintView::TakeActionRect(Rectangle&) const
{
}


// info about TextEdit. Default is false.
bool SdrPaintView::IsTextEdit() const
{
    return false;
}

// info about TextEditPageView. Default is 0L.
SdrPageView* SdrPaintView::GetTextEditPageView() const
{
    return nullptr;
}


sal_uInt16 SdrPaintView::ImpGetMinMovLogic(short nMinMov, const OutputDevice* pOut) const
{
    if (nMinMov>=0) return sal_uInt16(nMinMov);
    if (pOut==nullptr)
    {
        pOut = GetFirstOutputDevice();
    }
    if (pOut!=nullptr) {
        return short(-pOut->PixelToLogic(Size(nMinMov,0)).Width());
    } else {
        return 0;
    }
}

sal_uInt16 SdrPaintView::ImpGetHitTolLogic(short nHitTol, const OutputDevice* pOut) const
{
    if (nHitTol>=0) return sal_uInt16(nHitTol);
    if (pOut==nullptr)
    {
        pOut = GetFirstOutputDevice();
    }
    if (pOut!=nullptr) {
        return short(-pOut->PixelToLogic(Size(nHitTol,0)).Width());
    } else {
        return 0;
    }
}

void SdrPaintView::TheresNewMapMode()
{
    if (mpActualOutDev) {
        mnHitTolLog=(sal_uInt16)mpActualOutDev->PixelToLogic(Size(mnHitTolPix,0)).Width();
        mnMinMovLog=(sal_uInt16)mpActualOutDev->PixelToLogic(Size(mnMinMovPix,0)).Width();
    }
}

void SdrPaintView::SetActualWin(const OutputDevice* pWin)
{
    mpActualOutDev = const_cast<OutputDevice *>(pWin);
    TheresNewMapMode();
}


void SdrPaintView::ClearPageView()
{
    BrkAction();

    if(mpPageView)
    {
        InvalidateAllWin();
        delete mpPageView;
        mpPageView = nullptr;
    }
}

SdrPageView* SdrPaintView::ShowSdrPage(SdrPage* pPage)
{
    if(pPage && (!mpPageView || mpPageView->GetPage() != pPage))
    {
        if(mpPageView)
        {
            InvalidateAllWin();
            delete mpPageView;
        }

        mpPageView = new SdrPageView(pPage, *static_cast<SdrView*>(this));
        mpPageView->Show();
    }

    return mpPageView;
}

void SdrPaintView::HideSdrPage()
{
    if(mpPageView)
    {
        mpPageView->Hide();
        delete mpPageView;
        mpPageView = nullptr;
    }
}

void SdrPaintView::AddWindowToPaintView(OutputDevice* pNewWin, vcl::Window *pWindow)
{
    DBG_ASSERT(pNewWin, "SdrPaintView::AddWindowToPaintView: No OutputDevice(!)");
    SdrPaintWindow* pNewPaintWindow = new SdrPaintWindow(*this, *pNewWin, pWindow);
    AppendPaintWindow(*pNewPaintWindow);

    if(mpPageView)
    {
        mpPageView->AddPaintWindowToPageView(*pNewPaintWindow);
    }

#ifdef DBG_UTIL
    if (mpItemBrowser!=nullptr)
        mpItemBrowser->ForceParent();
#endif
}

void SdrPaintView::DeleteWindowFromPaintView(OutputDevice* pOldWin)
{
    assert(pOldWin && "SdrPaintView::DeleteWindowFromPaintView: No OutputDevice(!)");
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
    if (mpItemBrowser!=nullptr)
        mpItemBrowser->ForceParent();
#endif
}

void SdrPaintView::SetLayerVisible(const OUString& rName, bool bShow)
{
    if(mpPageView)
    {
        mpPageView->SetLayerVisible(rName, bShow);
    }

    InvalidateAllWin();
}

bool SdrPaintView::IsLayerVisible(const OUString& rName) const
{
    if(mpPageView)
    {
        return mpPageView->IsLayerVisible(rName);
    }

    return false;
}

void SdrPaintView::SetLayerLocked(const OUString& rName, bool bLock)
{
    if(mpPageView)
    {
        mpPageView->SetLayerLocked(rName,bLock);
    }
}

bool SdrPaintView::IsLayerLocked(const OUString& rName) const
{
    if(mpPageView)
    {
        return mpPageView->IsLayerLocked(rName);
    }

    return false;
}

void SdrPaintView::SetLayerPrintable(const OUString& rName, bool bPrn)
{
    if(mpPageView)
    {
        mpPageView->SetLayerPrintable(rName,bPrn);
    }
}

bool SdrPaintView::IsLayerPrintable(const OUString& rName) const
{
    if(mpPageView)
    {
        return mpPageView->IsLayerPrintable(rName);
    }

    return false;
}

void SdrPaintView::PrePaint()
{
    if(mpPageView)
    {
        mpPageView->PrePaint();
    }
}


// #define SVX_REPAINT_TIMER_TEST

void SdrPaintView::CompleteRedraw(OutputDevice* pOut, const vcl::Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector)
{
#ifdef SVX_REPAINT_TIMER_TEST
#define REMEMBERED_TIMES_COUNT  (10)
    static bool bDoTimerTest(false);
    static bool bTimesInited(false);
    static sal_uInt32 nRepeatCount(10L);
    static double fLastTimes[REMEMBERED_TIMES_COUNT];
    const sal_uInt64 nStartTime(tools::Time::GetSystemTicks());
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
    vcl::Region aOptimizedRepaintRegion(rReg);

    if(pOut && OUTDEV_WINDOW == pOut->GetOutDevType())
    {
        vcl::Window* pWindow = static_cast<vcl::Window*>(pOut);

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

                    for(RectangleVector::const_iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
                    {
                        pWindow->DrawRect(*aRectIter);
                    }

                    //RegionHandle aRegionHandle(aOptimizedRepaintRegion.BeginEnumRects());
                    //Rectangle aRegionRectangle;

                    //while(aOptimizedRepaintRegion.GetEnumRects(aRegionHandle, aRegionRectangle))
                    //{
                    //  pWindow->SetLineColor(COL_LIGHTGREEN);
                    //  pWindow->SetFillColor();
                    //  pWindow->DrawRect(aRegionRectangle);
                    //}

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
        const sal_uInt64 nStopTime(tools::Time::GetSystemTicks());
        const sal_uInt64 nNeededTime(nStopTime - nStartTime);
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
        fprintf(stderr, "StartTime : %" SAL_PRIuUINT64 ", StopTime: %" SAL_PRIuUINT64 ", NeededTime: %" SAL_PRIuUINT64 ", TimePerPaint: %f\n", nStartTime, nStopTime, nNeededTime, fTimePerPaint);
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

void SdrPaintView::DoCompleteRedraw(SdrPaintWindow& rPaintWindow, const vcl::Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector)
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
    std::unique_ptr<SdrPaintWindow> pPaintWindow;
    if (comphelper::LibreOfficeKit::isActive() && rPaintWindow.getTemporaryTarget())
    {
        // Tiled rendering, we must paint the TextEdit to the output device.
        pPaintWindow.reset(&rPaintWindow);
        pPaintWindow->setTemporaryTarget(false);
    }

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

        if (comphelper::LibreOfficeKit::isActive())
        {
            // Look for active text edits in other views showing the same page,
            // and show them as well.
            if (SdrPageView* pPageView = GetSdrPageView())
            {
                SdrViewIter aIter(pPageView->GetPage());
                for (SdrView* pView = aIter.FirstView(); pView; pView = aIter.NextView())
                {
                    if (pView == this)
                        continue;

                    if (pView->IsTextEdit() && pView->GetSdrPageView())
                    {
                        static_cast<SdrView*>(pView)->TextEditDrawing(rPaintWindow);
                    }
                }
            }
        }

        // draw Overlay, also to PreRender device if exists
        rPaintWindow.DrawOverlay(rPaintWindow.GetRedrawRegion());

        // output PreRendering
        rPaintWindow.OutputPreRenderDevice(rPaintWindow.GetRedrawRegion());
    }
}


SdrPaintWindow* SdrPaintView::BeginDrawLayers(OutputDevice* pOut, const vcl::Region& rReg, bool bDisableIntersect)
{
    // #i74769# use BeginCompleteRedraw() as common base
    SdrPaintWindow* pPaintWindow = BeginCompleteRedraw(pOut);
    OSL_ENSURE(pPaintWindow, "SdrPaintView::BeginDrawLayers: No SdrPaintWindow (!)");

    if(mpPageView)
    {
        SdrPageWindow* pKnownTarget = mpPageView->FindPageWindow(*pPaintWindow);

        if(pKnownTarget)
        {
            vcl::Region aOptimizedRepaintRegion = OptimizeDrawLayersRegion( pOut, rReg, bDisableIntersect );

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
        mpPageView->setPreparedPageWindow(nullptr);
    }
}

void SdrPaintView::UpdateDrawLayersRegion(OutputDevice* pOut, const vcl::Region& rReg, bool bDisableIntersect)
{
    SdrPaintWindow* pPaintWindow = FindPaintWindow(*pOut);
    OSL_ENSURE(pPaintWindow, "SdrPaintView::UpdateDrawLayersRegion: No SdrPaintWindow (!)");

    if(mpPageView)
    {
        SdrPageWindow* pKnownTarget = mpPageView->FindPageWindow(*pPaintWindow);

        if(pKnownTarget)
        {
            vcl::Region aOptimizedRepaintRegion = OptimizeDrawLayersRegion( pOut, rReg, bDisableIntersect );
            pKnownTarget->GetPaintWindow().SetRedrawRegion(aOptimizedRepaintRegion);
            mpPageView->setPreparedPageWindow(pKnownTarget); // already set actually
        }
    }
}

vcl::Region SdrPaintView::OptimizeDrawLayersRegion(OutputDevice* pOut, const vcl::Region& rReg, bool bDisableIntersect)
{
    // #i74769# check if pOut is a win and has a ClipRegion. If Yes, the Region
    // rReg may be made more granular (fine) with using it. Normally, rReg
    // does come from Window::Paint() anyways and thus is based on a single
    // rectangle which was derived from exactly that repaint region
    vcl::Region aOptimizedRepaintRegion(rReg);

    // #i76114# Intersecting the region with the Window's paint region is disabled
    // for print preview in Calc, because the intersection can be empty (if the paint
    // region is outside of the table area of the page), and then no clip region
    // would be set.
    if(pOut && OUTDEV_WINDOW == pOut->GetOutDevType() && !bDisableIntersect)
    {
        vcl::Window* pWindow = static_cast<vcl::Window*>(pOut);

        if(pWindow->IsInPaint())
        {
            if(!pWindow->GetPaintRegion().IsEmpty())
            {
                aOptimizedRepaintRegion.Intersect(pWindow->GetPaintRegion());
            }
        }
    }
    return aOptimizedRepaintRegion;
}


void SdrPaintView::ImpFormLayerDrawing( SdrPaintWindow& rPaintWindow )
{
    if(mpPageView)
    {
        SdrPageWindow* pKnownTarget = mpPageView->FindPageWindow(rPaintWindow);

        if(pKnownTarget)
        {
            const SdrModel& rModel = *(GetModel());
            const SdrLayerAdmin& rLayerAdmin = rModel.GetLayerAdmin();
            const SdrLayerID nControlLayerId = rLayerAdmin.GetLayerID(rLayerAdmin.GetControlLayerName(), false);

            // BUFFERED use GetTargetOutputDevice() now, it may be targeted to VDevs, too
            // need to set PreparedPageWindow to make DrawLayer use the correct ObjectContact
            mpPageView->setPreparedPageWindow(pKnownTarget);
            mpPageView->DrawLayer(nControlLayerId, &rPaintWindow.GetTargetOutputDevice());
            mpPageView->setPreparedPageWindow(nullptr);
        }
    }
}


bool SdrPaintView::KeyInput(const KeyEvent& /*rKEvt*/, vcl::Window* /*pWin*/)
{
    return false;
}

void SdrPaintView::GlueInvalidate() const
{
    const sal_uInt32 nWindowCount(PaintWindowCount());

    for(sal_uInt32 nWinNum(0L); nWinNum < nWindowCount; nWinNum++)
    {
        SdrPaintWindow* pPaintWindow = GetPaintWindow(nWinNum);

        if(pPaintWindow->OutputToWindow())
        {
            OutputDevice& rOutDev = pPaintWindow->GetOutputDevice();

            if(mpPageView)
            {
                const SdrObjList* pOL=mpPageView->GetObjList();
                const size_t nObjCount = pOL->GetObjCount();
                for (size_t nObjNum=0; nObjNum<nObjCount; ++nObjNum) {
                    const SdrObject* pObj=pOL->GetObj(nObjNum);
                    const SdrGluePointList* pGPL=pObj->GetGluePointList();
                    if (pGPL!=nullptr && pGPL->GetCount()!=0) {
                        pGPL->Invalidate(static_cast<vcl::Window&>(rOutDev), pObj);
                    }
                }
            }
        }
    }
}

void SdrPaintView::InvalidateAllWin()
{
    const sal_uInt32 nWindowCount(PaintWindowCount());

    for(sal_uInt32 a(0L); a < nWindowCount; a++)
    {
        SdrPaintWindow* pPaintWindow = GetPaintWindow(a);

        if(pPaintWindow->OutputToWindow())
        {
            InvalidateOneWin(static_cast<vcl::Window&>(pPaintWindow->GetOutputDevice()));
        }
    }
}

void SdrPaintView::InvalidateAllWin(const Rectangle& rRect, bool bPlus1Pix)
{
    const sal_uInt32 nWindowCount(PaintWindowCount());

    for(sal_uInt32 a(0L); a < nWindowCount; a++)
    {
        SdrPaintWindow* pPaintWindow = GetPaintWindow(a);

        if(pPaintWindow->OutputToWindow())
        {
            OutputDevice& rOutDev = pPaintWindow->GetOutputDevice();
            Rectangle aRect(rRect);

            if(bPlus1Pix)
            {
                Size aPixSiz(1,1);
                Size aSiz(rOutDev.PixelToLogic(aPixSiz));
                aRect.Left  ()-=aSiz.Width();
                aRect.Top   ()-=aSiz.Height();
                aRect.Right ()+=aSiz.Width();
                aRect.Bottom()+=aSiz.Height();
            }

            Point aOrg(rOutDev.GetMapMode().GetOrigin());
            aOrg.X()=-aOrg.X(); aOrg.Y()=-aOrg.Y();
            Rectangle aOutRect(aOrg, rOutDev.GetOutputSize());

            // In case of tiled rendering we want to get all invalidations, so visual area is not interesting.
            if (aRect.IsOver(aOutRect) || comphelper::LibreOfficeKit::isActive())
            {
                InvalidateOneWin(static_cast<vcl::Window&>(rOutDev), aRect);
            }
        }
    }
}

void SdrPaintView::InvalidateOneWin(vcl::Window& rWin)
{
    // do not erase background, that causes flicker (!)
    rWin.Invalidate(InvalidateFlags::NoErase);
}

void SdrPaintView::InvalidateOneWin(vcl::Window& rWin, const Rectangle& rRect)
{
    // do not erase background, that causes flicker (!)
    rWin.Invalidate(rRect, InvalidateFlags::NoErase);
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
        return (mpPageView->GetEnteredLevel() != 0);
    }

    return false;
}

void SdrPaintView::SetNotPersistDefaultAttr(const SfxItemSet& rAttr, bool /*bReplaceAll*/)
{
    // bReplaceAll has no effect here at all.
    bool bMeasure= dynamic_cast<const SdrView*>(this) != nullptr && static_cast<SdrView*>(this)->IsMeasureTool();
    const SfxPoolItem *pPoolItem=nullptr;
    if (rAttr.GetItemState(SDRATTR_LAYERID,true,&pPoolItem)==SfxItemState::SET) {
        SdrLayerID nLayerId=static_cast<const SdrLayerIdItem*>(pPoolItem)->GetValue();
        const SdrLayer* pLayer=mpModel->GetLayerAdmin().GetLayerPerID(nLayerId);
        if (pLayer!=nullptr) {
            if (bMeasure) maMeasureLayer=pLayer->GetName();
            else maActualLayer=pLayer->GetName();
        }
    }
    if (rAttr.GetItemState(SDRATTR_LAYERNAME,true,&pPoolItem)==SfxItemState::SET) {
        if (bMeasure) maMeasureLayer=static_cast<const SdrLayerNameItem*>(pPoolItem)->GetValue();
        else maActualLayer=static_cast<const SdrLayerNameItem*>(pPoolItem)->GetValue();
    }
}

void SdrPaintView::MergeNotPersistDefaultAttr(SfxItemSet& rAttr, bool /*bOnlyHardAttr*/) const
{
    // bOnlyHardAttr has no effect here at all.
    bool bMeasure= dynamic_cast<const SdrView*>(this) != nullptr && static_cast<const SdrView*>(this)->IsMeasureTool();
    const OUString& aNam = bMeasure ? maMeasureLayer : maActualLayer;
    rAttr.Put(SdrLayerNameItem(aNam));
    SdrLayerID nLayer=mpModel->GetLayerAdmin().GetLayerID(aNam,true);
    if (nLayer!=SDRLAYER_NOTFOUND) {
        rAttr.Put(SdrLayerIdItem(nLayer));
    }
}

void SdrPaintView::SetDefaultAttr(const SfxItemSet& rAttr, bool bReplaceAll)
{
#ifdef DBG_UTIL
    {
        bool bHasEEFeatureItems=false;
        SfxItemIter aIter(rAttr);
        const SfxPoolItem* pItem=aIter.FirstItem();
        while (!bHasEEFeatureItems && pItem!=nullptr) {
            if (!IsInvalidItem(pItem)) {
                sal_uInt16 nW=pItem->Which();
                if (nW>=EE_FEATURE_START && nW<=EE_FEATURE_END) bHasEEFeatureItems=true;
            }
            pItem=aIter.NextItem();
        }

        if(bHasEEFeatureItems)
        {
            OUString aMessage("SdrPaintView::SetDefaultAttr(): Setting EE_FEATURE items at the SdrView does not make sense! It only leads to overhead and unreadable documents.");
            ScopedVclPtr<InfoBox>::Create(nullptr, aMessage)->Execute();
        }
    }
#endif
    if (bReplaceAll) maDefaultAttr.Set(rAttr);
    else maDefaultAttr.Put(rAttr,false); // if FALSE, regard InvalidItems as "holes," not as Default
    SetNotPersistDefaultAttr(rAttr,bReplaceAll);
#ifdef DBG_UTIL
    if (mpItemBrowser!=nullptr) mpItemBrowser->SetDirty();
#endif
}

void SdrPaintView::SetDefaultStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr)
{
    if (mpDefaultStyleSheet)
        EndListening(*mpDefaultStyleSheet);
    mpDefaultStyleSheet=pStyleSheet;
    if (mpDefaultStyleSheet)
        StartListening(*mpDefaultStyleSheet);

    if (pStyleSheet!=nullptr && !bDontRemoveHardAttr) {
        SfxWhichIter aIter(pStyleSheet->GetItemSet());
        sal_uInt16 nWhich=aIter.FirstWhich();
        while (nWhich!=0) {
            if (pStyleSheet->GetItemSet().GetItemState(nWhich)==SfxItemState::SET) {
                maDefaultAttr.ClearItem(nWhich);
            }
            nWhich=aIter.NextWhich();
        }
    }
#ifdef DBG_UTIL
    if (mpItemBrowser!=nullptr) mpItemBrowser->SetDirty();
#endif
}

bool SdrPaintView::GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const
{
    if(bOnlyHardAttr || !mpDefaultStyleSheet)
    {
        rTargetSet.Put(maDefaultAttr, false);
    }
    else
    {
        // else merge with DefStyleSheet
        rTargetSet.Put(mpDefaultStyleSheet->GetItemSet(), false);
        rTargetSet.Put(maDefaultAttr, false);
    }
    MergeNotPersistDefaultAttr(rTargetSet, bOnlyHardAttr);
    return true;
}

bool SdrPaintView::SetAttributes(const SfxItemSet& rSet, bool bReplaceAll)
{
    SetDefaultAttr(rSet,bReplaceAll);
    return true;
}

SfxStyleSheet* SdrPaintView::GetStyleSheet() const
{
    return GetDefaultStyleSheet();
}

bool SdrPaintView::SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr)
{
    SetDefaultStyleSheet(pStyleSheet,bDontRemoveHardAttr);
    return true;
}


#ifdef DBG_UTIL
void SdrPaintView::ShowItemBrowser(bool bShow)
{
    if (bShow) {
        if (mpItemBrowser==nullptr) {
            mpItemBrowser=VclPtr<SdrItemBrowser>::Create(*static_cast<SdrView*>(this));
        }
        mpItemBrowser->Show();
        mpItemBrowser->GrabFocus();
    } else {
        if (mpItemBrowser!=nullptr) {
            mpItemBrowser->Hide();
            mpItemBrowser.disposeAndClear();
        }
    }
}
#endif

void SdrPaintView::MakeVisible(const Rectangle& rRect, vcl::Window& rWin)
{
    MapMode aMap(rWin.GetMapMode());
    Size aActualSize(rWin.GetOutputSize());

    if( aActualSize.Height() > 0 && aActualSize.Width() > 0 )
    {
        Size aNewSize(rRect.GetSize());
        bool bNewScale=false;
        bool bNeedMoreX=aNewSize.Width()>aActualSize.Width();
        bool bNeedMoreY=aNewSize.Height()>aActualSize.Height();
        if (bNeedMoreX || bNeedMoreY)
        {
            bNewScale=true;
            // set new MapMode (Size+Org) and invalidate everything
            Fraction aXFact(aNewSize.Width(),aActualSize.Width());
            Fraction aYFact(aNewSize.Height(),aActualSize.Height());
            if (aYFact>aXFact) aXFact=aYFact;
            aXFact*=aMap.GetScaleX();
            aXFact.ReduceInaccurate(10); // to avoid runovers and BigInt mapping
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
        if (l>rRect.Left()) dx=rRect.Left()-l;
        else if (r<rRect.Right()) dx=rRect.Right()-r;
        if (o>rRect.Top()) dy=rRect.Top()-o;
        else if (u<rRect.Bottom()) dy=rRect.Bottom()-u;
        aMap.SetOrigin(Point(aOrg.X()-dx,aOrg.Y()-dy));
        if (!bNewScale) {
            if (dx!=0 || dy!=0) {
                rWin.Scroll(-dx,-dy);
                rWin.SetMapMode(aMap);
                rWin.Update();
            }
        } else {
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

#if defined DBG_UTIL
vcl::Window* SdrPaintView::GetItemBrowser() const
{
    return mpItemBrowser;
}
#endif

void SdrPaintView::SetAnimationPause( bool bSet )
{
    if((bool)mbAnimationPause != bSet)
    {
        mbAnimationPause = bSet;

        if(mpPageView)
        {
            for(sal_uInt32 b(0L); b < mpPageView->PageWindowCount(); b++)
            {
                SdrPageWindow& rPageWindow = *(mpPageView->GetPageWindow(b));
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
    meAnimationMode = eMode;
}

void SdrPaintView::VisAreaChanged(const OutputDevice* pOut)
{
    if(mpPageView)
    {
        if (pOut)
        {
            SdrPageWindow* pWindow = mpPageView->FindPageWindow(*const_cast<OutputDevice*>(pOut));

            if(pWindow)
            {
                VisAreaChanged(*pWindow);
            }
        }
        else
        {
            for(sal_uInt32 a(0L); a < mpPageView->PageWindowCount(); a++)
            {
                VisAreaChanged(*mpPageView->GetPageWindow(a));
            }
        }
    }
}

void SdrPaintView::VisAreaChanged(const SdrPageWindow& /*rWindow*/)
{
    // notify SfxListener
    Broadcast(SvxViewHint(SvxViewHint::SVX_HINT_VIEWCHANGED));
}


void SdrPaintView::onChangeColorConfig()
{
    SetGridColor( Color( maColorConfig.GetColorValue( svtools::DRAWGRID ).nColor ) );
}

void SdrPaintView::SetGridColor( Color aColor )
{
    maGridColor = aColor;
}


// Set background color for svx at SdrPageViews
void SdrPaintView::SetApplicationBackgroundColor(Color aBackgroundColor)
{
    if(mpPageView)
    {
        mpPageView->SetApplicationBackgroundColor(aBackgroundColor);
    }
}

// Set document color for svx at SdrPageViews
void SdrPaintView::SetApplicationDocumentColor(Color aDocumentColor)
{
    if(mpPageView)
    {
        mpPageView->SetApplicationDocumentColor(aDocumentColor);
    }
}

bool SdrPaintView::IsBufferedOutputAllowed() const
{
    return (mbBufferedOutputAllowed && maDrawinglayerOpt.IsPaintBuffer());
}

void SdrPaintView::SetBufferedOutputAllowed(bool bNew)
{
    if(bNew != (bool)mbBufferedOutputAllowed)
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
    if(bNew != (bool)mbBufferedOverlayAllowed)
    {
        mbBufferedOverlayAllowed = bNew;
    }
}


void SdrPaintView::SetPagePaintingAllowed(bool bNew)
{
    if(bNew != (bool)mbPagePaintingAllowed)
    {
        mbPagePaintingAllowed = bNew;
    }
}

// #i38135# Sets the timer for Object animations and restarts.
void SdrPaintView::SetAnimationTimer(sal_uInt32 nTime)
{
    if(mpPageView)
    {
        // first, reset all timers at all windows to 0L
        for(sal_uInt32 a(0L); a < mpPageView->PageWindowCount(); a++)
        {
            SdrPageWindow& rPageWindow = *mpPageView->GetPageWindow(a);
            sdr::contact::ObjectContact& rObjectContact = rPageWindow.GetObjectContact();
            sdr::animation::primitiveAnimator& rAnimator = rObjectContact.getPrimitiveAnimator();
            rAnimator.SetTime(nTime);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
