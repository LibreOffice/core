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
#include <svx/svdpntv.hxx>
#include <vcl/weld.hxx>
#include <vcl/window.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/svdmodel.hxx>

#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svl/hint.hxx>

#include <svx/svdview.hxx>
#include <svx/svdglue.hxx>
#include <svx/svdobj.hxx>
#include <sxlayitm.hxx>
#include <svl/itemiter.hxx>
#include <editeng/eeitem.hxx>
#include <svl/whiter.hxx>
#include <svl/style.hxx>
#include <svx/sdrpagewindow.hxx>
#include <vcl/svapp.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/animation/objectanimator.hxx>
#include <drawinglayer/primitive2d/metafileprimitive2d.hxx>
#include <drawinglayer/converters.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <comphelper/lok.hxx>
#include <svx/svdviter.hxx>
#include <svtools/optionsdrawinglayer.hxx>

using namespace ::com::sun::star;

// interface to SdrPaintWindow

SdrPaintWindow* SdrPaintView::FindPaintWindow(const OutputDevice& rOut) const
{
    // back to loop - there is more to test than a std::find_if and a lambda can do
    for(auto& candidate : maPaintWindows)
    {
        if(&(candidate->GetOutputDevice()) == &rOut)
        {
            return candidate.get();
        }

        // check for patched to allow finding in that state, too
        if(nullptr != candidate->getPatched() && &(candidate->getPatched()->GetOutputDevice()) == &rOut)
        {
            return candidate->getPatched();
        }
    }

    return nullptr;
}

SdrPaintWindow* SdrPaintView::GetPaintWindow(sal_uInt32 nIndex) const
{
    return maPaintWindows[nIndex].get();
}

void SdrPaintView::DeletePaintWindow(const SdrPaintWindow& rOld)
{
    auto aFindResult = ::std::find_if(maPaintWindows.begin(), maPaintWindows.end(),
                            [&](const std::unique_ptr<SdrPaintWindow>& p) { return p.get() == &rOld; });

    if(aFindResult != maPaintWindows.end())
    {
        maPaintWindows.erase(aFindResult);
    }
}

OutputDevice* SdrPaintView::GetFirstOutputDevice() const
{
    if(PaintWindowCount())
    {
        return &(GetPaintWindow(0)->GetOutputDevice());
    }

    return nullptr;
}


SvxViewChangedHint::SvxViewChangedHint()
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
                basegfx::utils::createScaleTranslateB2DHomMatrix(
                    rTargetRange.getRange(),
                    rTargetRange.getMinimum()),
                rMtf));
        aBitmapEx = drawinglayer::convertPrimitive2DContainerToBitmapEx(
            drawinglayer::primitive2d::Primitive2DContainer { aMtf },
            rTargetRange,
            nMaximumQuadraticPixels);
    }

    return aBitmapEx;
}

SdrPaintView::SdrPaintView(SdrModel& rSdrModel, OutputDevice* pOut)
    : mrSdrModelFromSdrView(rSdrModel)
    , mpModel(&rSdrModel)
    , mpActualOutDev(nullptr)
    , mpDragWin(nullptr)
    , mpDefaultStyleSheet(nullptr)
    , maDefaultAttr(rSdrModel.GetItemPool())
    , maComeBackIdle( "svx::SdrPaintView aComeBackIdle" )
    , meAnimationMode(SdrAnimationMode::Animate)
    , mnHitTolPix(2)
    , mnMinMovPix(3)
    , mnHitTolLog(0)
    , mnMinMovLog(0)
    , mbPageVisible(true)
    , mbPageShadowVisible(true)
    , mbPageBorderVisible(true)
    , mbBordVisible(true)
    , mbGridVisible(true)
    , mbGridFront(false)
    , mbHlplVisible(true)
    , mbHlplFront(true)
    , mbGlueVisible(false)
    , mbGlueVisible2(false)
    , mbGlueVisible3(false)
    , mbGlueVisible4(false)
    , mbSomeObjChgdFlag(false)
    , mbSwapAsynchron(false)
    , mbPrintPreview(false)
    , mbAnimationPause(false)
    , mbBufferedOutputAllowed(false)
    , mbBufferedOverlayAllowed(false)
    , mbPagePaintingAllowed(true)
    , mbPreviewRenderer(false)
    , mbHideOle(false)
    , mbHideChart(false)
    , mbHideDraw(false)
    , mbHideFormControl(false)
    , maGridColor(COL_BLACK)
{
    maComeBackIdle.SetPriority(TaskPriority::REPAINT);
    maComeBackIdle.SetInvokeHandler(LINK(this,SdrPaintView,ImpComeBackHdl));

    if (mpModel)
        SetDefaultStyleSheet(mpModel->GetDefaultStyleSheet(), true);

    if (pOut)
        AddWindowToPaintView(pOut, nullptr);

    maColorConfig.AddListener(this);
    onChangeColorConfig();
}

SdrPaintView::~SdrPaintView()
{
    if (mpDefaultStyleSheet)
        EndListening(*mpDefaultStyleSheet);

    maColorConfig.RemoveListener(this);
    ClearPageView();

    // delete existing SdrPaintWindows
    maPaintWindows.clear();
}


void SdrPaintView::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    //If the stylesheet has been destroyed
    if (&rBC == mpDefaultStyleSheet)
    {
        if (rHint.GetId() == SfxHintId::Dying)
            mpDefaultStyleSheet = nullptr;
        return;
    }

    if (rHint.GetId() != SfxHintId::ThisIsAnSdrHint)
        return;
    const SdrHint* pSdrHint = static_cast<const SdrHint*>(&rHint);
    SdrHintKind eKind = pSdrHint->GetKind();
    if (eKind==SdrHintKind::ObjectChange || eKind==SdrHintKind::ObjectInserted || eKind==SdrHintKind::ObjectRemoved)
    {
        bool bObjChg = !mbSomeObjChgdFlag; // if true, evaluate for ComeBack timer
        if (bObjChg)
        {
            mbSomeObjChgdFlag=true;
            maComeBackIdle.Start();
        }
    }

    if (eKind==SdrHintKind::PageOrderChange)
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

void SdrPaintView::ConfigurationChanged( ::utl::ConfigurationBroadcaster* , ConfigurationHints )
{
    onChangeColorConfig();
    InvalidateAllWin();
}

IMPL_LINK_NOARG(SdrPaintView, ImpComeBackHdl, Timer *, void)
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

void SdrPaintView::TakeActionRect(tools::Rectangle&) const
{
}


// info about TextEdit. Default is false.
bool SdrPaintView::IsTextEdit() const
{
    return false;
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
        mnHitTolLog=static_cast<sal_uInt16>(mpActualOutDev->PixelToLogic(Size(mnHitTolPix,0)).Width());
        mnMinMovLog=static_cast<sal_uInt16>(mpActualOutDev->PixelToLogic(Size(mnMinMovPix,0)).Width());
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
        mpPageView.reset();
    }
}

SdrPageView* SdrPaintView::ShowSdrPage(SdrPage* pPage)
{
    if(pPage && (!mpPageView || mpPageView->GetPage() != pPage))
    {
        if(mpPageView)
        {
            InvalidateAllWin();
            mpPageView.reset();
        }

        mpPageView.reset(new SdrPageView(pPage, *static_cast<SdrView*>(this)));
        mpPageView->Show();
    }

    return mpPageView.get();
}

void SdrPaintView::HideSdrPage()
{
    if(mpPageView)
    {
        mpPageView->Hide();
        mpPageView.reset();
    }
}

void SdrPaintView::AddWindowToPaintView(OutputDevice* pNewWin, vcl::Window *pWindow)
{
    DBG_ASSERT(pNewWin, "SdrPaintView::AddWindowToPaintView: No OutputDevice(!)");
    SdrPaintWindow* pNewPaintWindow = new SdrPaintWindow(*this, *pNewWin, pWindow);
    maPaintWindows.emplace_back(pNewPaintWindow);

    if(mpPageView)
    {
        mpPageView->AddPaintWindowToPageView(*pNewPaintWindow);
    }
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

        DeletePaintWindow(*pCandidate);
    }
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
    static sal_uInt32 nRepeatCount(10);
    static double fLastTimes[REMEMBERED_TIMES_COUNT];
    const sal_uInt64 nStartTime(tools::Time::GetSystemTicks());
    sal_uInt32 count(1);
    sal_uInt32 a;

    if(bDoTimerTest)
    {
        count = nRepeatCount;
    }

    for(a = 0; a < count; a++)
    {
#endif // SVX_REPAINT_TIMER_TEST

    // #i74769# check if pOut is a win and has a ClipRegion. If Yes, the Region
    // rReg may be made more granular (fine) with using it. Normally, rReg
    // does come from Window::Paint() anyways and thus is based on a single
    // rectangle which was derived from exactly that repaint region
    vcl::Region aOptimizedRepaintRegion(rReg);

    if(pOut && OUTDEV_WINDOW == pOut->GetOutDevType())
    {
        vcl::Window* pWindow = pOut->GetOwnerWindow();

        if(pWindow->IsInPaint())
        {
            if(!pWindow->GetPaintRegion().IsEmpty())
            {
                aOptimizedRepaintRegion.Intersect(pWindow->GetPaintRegion());
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
            for(a = 0; a < REMEMBERED_TIMES_COUNT; a++)
            {
                fLastTimes[a] = fTimePerPaint;
            }

            bTimesInited = true;
        }
        else
        {
            for(a = 1; a < REMEMBERED_TIMES_COUNT; a++)
            {
                fLastTimes[a - 1] = fLastTimes[a];
            }

            fLastTimes[REMEMBERED_TIMES_COUNT - 1] = fTimePerPaint;
        }

        double fAddedTimes(0.0);

        for(a = 0; a < REMEMBERED_TIMES_COUNT; a++)
        {
            fAddedTimes += fLastTimes[a];
        }

        const double fAverageTimePerPaint(fAddedTimes / (double)REMEMBERED_TIMES_COUNT);

        fprintf(stderr, "-----------(start result)----------\n");
        fprintf(stderr, "StartTime : %" SAL_PRIuUINT64 ", StopTime: %" SAL_PRIuUINT64 ", NeededTime: %" SAL_PRIuUINT64 ", TimePerPaint: %f\n", nStartTime, nStopTime, nNeededTime, fTimePerPaint);
        fprintf(stderr, "Remembered times: ");

        for(a = 0; a < REMEMBERED_TIMES_COUNT; a++)
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
        delete &rPaintWindow;
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
                        pView->TextEditDrawing(rPaintWindow);
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

void SdrPaintView::UpdateDrawLayersRegion(const OutputDevice* pOut, const vcl::Region& rReg)
{
    SdrPaintWindow* pPaintWindow = FindPaintWindow(*pOut);
    OSL_ENSURE(pPaintWindow, "SdrPaintView::UpdateDrawLayersRegion: No SdrPaintWindow (!)");

    if(mpPageView)
    {
        SdrPageWindow* pKnownTarget = mpPageView->FindPageWindow(*pPaintWindow);

        if(pKnownTarget)
        {
            vcl::Region aOptimizedRepaintRegion = OptimizeDrawLayersRegion( pOut, rReg, false/*bDisableIntersect*/ );
            pKnownTarget->GetPaintWindow().SetRedrawRegion(aOptimizedRepaintRegion);
            mpPageView->setPreparedPageWindow(pKnownTarget); // already set actually
        }
    }
}

vcl::Region SdrPaintView::OptimizeDrawLayersRegion(const OutputDevice* pOut, const vcl::Region& rReg, bool bDisableIntersect)
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
        vcl::Window* pWindow = pOut->GetOwnerWindow();

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
    if(!mpPageView)
        return;

    SdrPageWindow* pKnownTarget = mpPageView->FindPageWindow(rPaintWindow);

    if(pKnownTarget)
    {
        const SdrModel& rModel = *(GetModel());
        const SdrLayerAdmin& rLayerAdmin = rModel.GetLayerAdmin();
        const SdrLayerID nControlLayerId = rLayerAdmin.GetLayerID(rLayerAdmin.GetControlLayerName());

        // BUFFERED use GetTargetOutputDevice() now, it may be targeted to VDevs, too
        // need to set PreparedPageWindow to make DrawLayer use the correct ObjectContact
        mpPageView->setPreparedPageWindow(pKnownTarget);
        mpPageView->DrawLayer(nControlLayerId, &rPaintWindow.GetTargetOutputDevice());
        mpPageView->setPreparedPageWindow(nullptr);
    }
}


bool SdrPaintView::KeyInput(const KeyEvent& /*rKEvt*/, vcl::Window* /*pWin*/)
{
    return false;
}

void SdrPaintView::GlueInvalidate() const
{
    // Do not invalidate GluePoints in Online
    // They are handled on front-end
    if (comphelper::LibreOfficeKit::isActive())
        return;

    const sal_uInt32 nWindowCount(PaintWindowCount());

    for(sal_uInt32 nWinNum(0); nWinNum < nWindowCount; nWinNum++)
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
                        pGPL->Invalidate(*rOutDev.GetOwnerWindow(), pObj);
                    }
                }
            }
        }
    }
}

void SdrPaintView::InvalidateAllWin()
{
    const sal_uInt32 nWindowCount(PaintWindowCount());

    for(sal_uInt32 a(0); a < nWindowCount; a++)
    {
        SdrPaintWindow* pPaintWindow = GetPaintWindow(a);

        if(pPaintWindow->OutputToWindow())
        {
            InvalidateOneWin(pPaintWindow->GetOutputDevice());
        }
    }
}

void SdrPaintView::InvalidateAllWin(const tools::Rectangle& rRect)
{
    const sal_uInt32 nWindowCount(PaintWindowCount());

    for(sal_uInt32 a(0); a < nWindowCount; a++)
    {
        SdrPaintWindow* pPaintWindow = GetPaintWindow(a);

        if(pPaintWindow->OutputToWindow())
        {
            OutputDevice& rOutDev = pPaintWindow->GetOutputDevice();
            tools::Rectangle aRect(rRect);

            Point aOrg(rOutDev.GetMapMode().GetOrigin());
            aOrg.setX(-aOrg.X() ); aOrg.setY(-aOrg.Y() );
            tools::Rectangle aOutRect(aOrg, rOutDev.GetOutputSize());

            // In case of tiled rendering we want to get all invalidations, so visual area is not interesting.
            if (aRect.Overlaps(aOutRect) || comphelper::LibreOfficeKit::isActive())
            {
                InvalidateOneWin(rOutDev, aRect);
            }
        }
    }
}

void SdrPaintView::InvalidateOneWin(OutputDevice& rDevice)
{
    // do not erase background, that causes flicker (!)
    rDevice.GetOwnerWindow()->Invalidate(InvalidateFlags::NoErase);
}

void SdrPaintView::InvalidateOneWin(OutputDevice& rDevice, const tools::Rectangle& rRect)
{
    // do not erase background, that causes flicker (!)
    rDevice.GetOwnerWindow()->Invalidate(rRect, InvalidateFlags::NoErase);
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

void SdrPaintView::SetNotPersistDefaultAttr(const SfxItemSet& rAttr)
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

void SdrPaintView::MergeNotPersistDefaultAttr(SfxItemSet& rAttr) const
{
    // bOnlyHardAttr has no effect here at all.
    bool bMeasure= dynamic_cast<const SdrView*>(this) != nullptr && static_cast<const SdrView*>(this)->IsMeasureTool();
    const OUString& aNam = bMeasure ? maMeasureLayer : maActualLayer;
    rAttr.Put(SdrLayerNameItem(aNam));
    SdrLayerID nLayer=mpModel->GetLayerAdmin().GetLayerID(aNam);
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
        for (const SfxPoolItem* pItem = aIter.GetCurItem(); !bHasEEFeatureItems && pItem;
             pItem = aIter.NextItem())
        {
            if (!IsInvalidItem(pItem)) {
                sal_uInt16 nW=pItem->Which();
                if (nW>=EE_FEATURE_START && nW<=EE_FEATURE_END) bHasEEFeatureItems=true;
            }
        }

        if(bHasEEFeatureItems)
        {
            std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(nullptr,
                                                          VclMessageType::Info, VclButtonsType::Ok,
                                                          "SdrPaintView::SetDefaultAttr(): Setting EE_FEATURE items at the SdrView does not make sense! It only leads to overhead and unreadable documents."));
            xInfoBox->run();
        }
    }
#endif
    if (bReplaceAll) maDefaultAttr.Set(rAttr);
    else maDefaultAttr.Put(rAttr,false); // if FALSE, regard InvalidItems as "holes," not as Default
    SetNotPersistDefaultAttr(rAttr);
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
}

void SdrPaintView::GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const
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
    MergeNotPersistDefaultAttr(rTargetSet);
}

void SdrPaintView::SetAttributes(const SfxItemSet& rSet, bool bReplaceAll)
{
    SetDefaultAttr(rSet,bReplaceAll);
}

SfxStyleSheet* SdrPaintView::GetStyleSheet() const
{
    return mpDefaultStyleSheet;
}

void SdrPaintView::SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr)
{
    SetDefaultStyleSheet(pStyleSheet,bDontRemoveHardAttr);
}

void SdrPaintView::MakeVisible(const tools::Rectangle& rRect, vcl::Window& rWin)
{
    // TODO: handle when the text cursor goes out of the chart area
    // However this hack avoids that the cursor gets misplaced wrt the text.
    if (comphelper::LibreOfficeKit::isActive() && rWin.IsChart())
    {
        return;
    }

    MapMode aMap(rWin.GetMapMode());
    Size aActualSize(rWin.GetOutDev()->GetOutputSize());

    if( aActualSize.IsEmpty() )
        return;

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
        aActualSize=rWin.GetOutDev()->GetOutputSize();
    }
    Point aOrg(aMap.GetOrigin());
    tools::Long dx=0,dy=0;
    tools::Long l=-aOrg.X();
    tools::Long r=-aOrg.X()+aActualSize.Width()-1;
    tools::Long o=-aOrg.Y();
    tools::Long u=-aOrg.Y()+aActualSize.Height()-1;
    if (l>rRect.Left()) dx=rRect.Left()-l;
    else if (r<rRect.Right()) dx=rRect.Right()-r;
    if (o>rRect.Top()) dy=rRect.Top()-o;
    else if (u<rRect.Bottom()) dy=rRect.Bottom()-u;
    aMap.SetOrigin(Point(aOrg.X()-dx,aOrg.Y()-dy));
    if (!bNewScale) {
        if (dx!=0 || dy!=0) {
            rWin.Scroll(-dx,-dy);
            rWin.SetMapMode(aMap);
            rWin.PaintImmediately();
        }
    } else {
        rWin.SetMapMode(aMap);
        InvalidateOneWin(*rWin.GetOutDev());
    }
}

void SdrPaintView::DoConnect(SdrOle2Obj* /*pOleObj*/)
{
}

void SdrPaintView::SetAnimationEnabled( bool bEnable )
{
    SetAnimationMode( bEnable ? SdrAnimationMode::Animate : SdrAnimationMode::Disable );
}

void SdrPaintView::SetAnimationPause( bool bSet )
{
    if(mbAnimationPause == bSet)
        return;

    mbAnimationPause = bSet;

    if(!mpPageView)
        return;

    for(sal_uInt32 b(0); b < mpPageView->PageWindowCount(); b++)
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

void SdrPaintView::SetAnimationMode( const SdrAnimationMode eMode )
{
    meAnimationMode = eMode;
}

void SdrPaintView::VisAreaChanged(const OutputDevice* pOut)
{
    if(!mpPageView)
        return;

    if (pOut)
    {
        SdrPageWindow* pWindow = mpPageView->FindPageWindow(*const_cast<OutputDevice*>(pOut));

        if(pWindow)
        {
            VisAreaChanged();
        }
    }
    else
    {
        VisAreaChanged();
    }
}

void SdrPaintView::VisAreaChanged()
{
    // notify SfxListener
    Broadcast(SvxViewChangedHint());
}


void SdrPaintView::onChangeColorConfig()
{
    maGridColor = maColorConfig.GetColorValue( svtools::DRAWGRID ).nColor;
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
    return (mbBufferedOutputAllowed && SvtOptionsDrawinglayer::IsPaintBuffer());
}

void SdrPaintView::SetBufferedOutputAllowed(bool bNew)
{
    if(bNew != mbBufferedOutputAllowed)
    {
        mbBufferedOutputAllowed = bNew;
    }
}

bool SdrPaintView::IsBufferedOverlayAllowed() const
{
    return (mbBufferedOverlayAllowed && SvtOptionsDrawinglayer::IsOverlayBuffer());
}

void SdrPaintView::SetBufferedOverlayAllowed(bool bNew)
{
    if(bNew != mbBufferedOverlayAllowed)
    {
        mbBufferedOverlayAllowed = bNew;
    }
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
        // first, reset all timers at all windows to 0L
        for(sal_uInt32 a(0); a < mpPageView->PageWindowCount(); a++)
        {
            SdrPageWindow& rPageWindow = *mpPageView->GetPageWindow(a);
            sdr::contact::ObjectContact& rObjectContact = rPageWindow.GetObjectContact();
            sdr::animation::primitiveAnimator& rAnimator = rObjectContact.getPrimitiveAnimator();
            rAnimator.SetTime(nTime);
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
