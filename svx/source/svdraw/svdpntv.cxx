/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "svx/svditer.hxx"
#include <svx/svdouno.hxx>
#include <svx/sdr/overlay/overlayobjectlist.hxx>
#include <svx/sdr/overlay/overlayrollingrectangle.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/svdglue.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdview.hxx>
#include <svx/sxlayitm.hxx>
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

// #i38135#
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/animation/objectanimator.hxx>
#include <svx/sdr/contact/viewcontact.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;

////////////////////////////////////////////////////////////////////////////////////////////////////
// #114409#-3 Migrate Encirclement
class ImplEncirclementOverlay
{
    // The OverlayObjects
    ::sdr::overlay::OverlayObjectList               maObjects;

    // The remembered second position in logical coodinates
    basegfx::B2DPoint                               maSecondPosition;

public:
    ImplEncirclementOverlay(const SdrPaintView& rView, const basegfx::B2DPoint& rStartPos);
    ~ImplEncirclementOverlay();

    void SetSecondPosition(const basegfx::B2DPoint& rNewPosition);
};

ImplEncirclementOverlay::ImplEncirclementOverlay(const SdrPaintView& rView, const basegfx::B2DPoint& rStartPos)
:   maSecondPosition(rStartPos)
{
    for(sal_uInt32 a(0L); a < rView.PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = rView.GetPaintWindow(a);
        ::sdr::overlay::OverlayManager* pTargetOverlay = pCandidate->GetOverlayManager();

        if(pTargetOverlay)
        {
            ::sdr::overlay::OverlayRollingRectangleStriped* aNew = new ::sdr::overlay::OverlayRollingRectangleStriped(
                rStartPos, rStartPos, false);
            pTargetOverlay->add(*aNew);
            maObjects.append(*aNew);
        }
    }
}

ImplEncirclementOverlay::~ImplEncirclementOverlay()
{
    // The OverlayObjects are cleared using the destructor of OverlayObjectList.
    // That destructor calls clear() at the list which removes all objects from the
    // OverlayManager and deletes them.
}

void ImplEncirclementOverlay::SetSecondPosition(const basegfx::B2DPoint& rNewPosition)
{
    if(rNewPosition != maSecondPosition)
    {
        // apply to OverlayObjects
        for(sal_uInt32 a(0L); a < maObjects.count(); a++)
        {
            ::sdr::overlay::OverlayRollingRectangleStriped& rCandidate = (::sdr::overlay::OverlayRollingRectangleStriped&)maObjects.getOverlayObject(a);
            rCandidate.setSecondPosition(rNewPosition);
        }

        // remember new position
        maSecondPosition = rNewPosition;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SvxViewHint, SfxHint );

SvxViewHint::SvxViewHint (HintType eHintType)
    : meHintType(eHintType)
{
}

SvxViewHint::HintType SvxViewHint::GetHintType (void) const
{
    return meHintType;
}


////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT2(SdrPaintView,SfxListener,SfxRepeatTarget);

DBG_NAME(SdrPaintView);

void SdrPaintView::ImpClearVars()
{
#ifdef DBG_UTIL
    pItemBrowser=NULL;
#endif
    bPageVisible=sal_True;
    bPageBorderVisible=sal_True;
    bBordVisible=sal_True;
    bGridVisible=sal_True;
    bGridFront  =sal_False;
    bHlplVisible=sal_True;
    bHlplFront  =sal_True;
    bGlueVisible=sal_False;
    bGlueVisible2=sal_False;
    bGlueVisible3=sal_False;
    bGlueVisible4=sal_False;
    bSwapAsynchron=sal_False;
    bPrintPreview=sal_False;
    mbPreviewRenderer=sal_False;

    eAnimationMode = SDR_ANIMATION_ANIMATE;
    bAnimationPause = sal_False;

    nHitTolPix=2;
    nMinMovPix=3;
    nHitTolLog=0;
    nMinMovLog=0;
    pActualOutDev=NULL;
    pDragWin=NULL;
    bRestoreColors=sal_True;
    pDefaultStyleSheet=NULL;
    bSomeObjChgdFlag=sal_False;
    nGraphicManagerDrawMode = GRFMGR_DRAW_STANDARD;
    aComeBackTimer.SetTimeout(1);
    aComeBackTimer.SetTimeoutHdl(LINK(this,SdrPaintView,ImpComeBackHdl));
    String aNam;    // System::GetUserName() just return an empty string

    if (pMod)
        SetDefaultStyleSheet(pMod->GetDefaultStyleSheet(), sal_True);

    aNam.ToUpperAscii();

    maGridColor = Color( COL_BLACK );
    BrkEncirclement();
}

SdrPaintView::SdrPaintView(SdrModel* pModel1, OutputDevice* pOut)
:   mpEncirclementOverlay(0L),
    mpPageView(0L),
    aDefaultAttr(pModel1->GetItemPool()),
    mbBufferedOutputAllowed(false),
    mbBufferedOverlayAllowed(false),
    mbPagePaintingAllowed(true),
    mbHideOle(false),
    mbHideChart(false),
    mbHideDraw(false),
    mbHideFormControl(false)
{
    DBG_CTOR(SdrPaintView,NULL);
    pMod=pModel1;
    ImpClearVars();

    if(pOut)
    {
        AddWindowToPaintView(pOut);
    }

    // Flag zur Visualisierung von Gruppen
    bVisualizeEnteredGroup = sal_True;

    maColorConfig.AddListener(this);
    onChangeColorConfig();
}

SdrPaintView::~SdrPaintView()
{
    DBG_DTOR(SdrPaintView,NULL);
    if (pDefaultStyleSheet)
        EndListening(*pDefaultStyleSheet);

    maColorConfig.RemoveListener(this);
    ClearPageView();

#ifdef DBG_UTIL
    if(pItemBrowser)
    {
        delete pItemBrowser;
    }
#endif

    // delete existing SdrPaintWindows
    while(maPaintWindows.size())
    {
        delete maPaintWindows.back();
        maPaintWindows.pop_back();
    }

    // #114409#-3 Migrate HelpLine
    BrkEncirclement();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPaintView::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    //If the stylesheet has been destroyed
    if (&rBC == pDefaultStyleSheet)
    {
        if (rHint.ISA(SfxSimpleHint) && ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING)
            pDefaultStyleSheet = NULL;
        return;
    }

    sal_Bool bObjChg=!bSomeObjChgdFlag; // sal_True= auswerten fuer ComeBack-Timer
    if (bObjChg) {
        SdrHint* pSdrHint=PTR_CAST(SdrHint,&rHint);
        if (pSdrHint!=NULL) {
            SdrHintKind eKind=pSdrHint->GetKind();
            if (eKind==HINT_OBJCHG || eKind==HINT_OBJINSERTED || eKind==HINT_OBJREMOVED) {
                if (bObjChg) {
                    bSomeObjChgdFlag=sal_True;
                    aComeBackTimer.Start();
                }
            }
            if (eKind==HINT_PAGEORDERCHG) {
                const SdrPage* pPg=pSdrHint->GetPage();

                if(pPg && !pPg->IsInserted())
                {
                    if(mpPageView && mpPageView->GetPage() == pPg)
                    {
                        HideSdrPage();
                    }
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

IMPL_LINK_INLINE_START(SdrPaintView,ImpComeBackHdl,Timer*,EMPTYARG)
{
    if (bSomeObjChgdFlag) {
        bSomeObjChgdFlag=sal_False;
        ModelHasChanged();
    }
    return 0;
}
IMPL_LINK_INLINE_END(SdrPaintView,ImpComeBackHdl,Timer*,pTimer)

void SdrPaintView::FlushComeBackTimer() const
{
    if (bSomeObjChgdFlag) {
        // casting auf nonconst
        ((SdrPaintView*)this)->ImpComeBackHdl(&((SdrPaintView*)this)->aComeBackTimer);
        ((SdrPaintView*)this)->aComeBackTimer.Stop();
    }
}

void SdrPaintView::ModelHasChanged()
{
    // Auch alle PageViews benachrichtigen
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
    if(pItemBrowser)
    {
        pItemBrowser->SetDirty();
    }
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SdrPaintView::IsAction() const
{
    return IsEncirclement();
}

void SdrPaintView::MovAction(const Point& rPnt)
{
    if (IsEncirclement())
    {
        MovEncirclement(rPnt);
    }
}

void SdrPaintView::EndAction()
{
    if(IsEncirclement())
    {
        EndEncirclement();
    }
}

void SdrPaintView::BckAction()
{
    BrkEncirclement();
}

void SdrPaintView::BrkAction()
{
    BrkEncirclement();
}

void SdrPaintView::TakeActionRect(Rectangle& rRect) const
{
    if(IsEncirclement())
    {
        rRect = Rectangle(aDragStat.GetStart(),aDragStat.GetNow());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// info about TextEdit. Default is sal_False.
bool SdrPaintView::IsTextEdit() const
{
    return false;
}

// info about TextEditPageView. Default is 0L.
SdrPageView* SdrPaintView::GetTextEditPageView() const
{
    return 0L;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_uInt16 SdrPaintView::ImpGetMinMovLogic(short nMinMov, const OutputDevice* pOut) const
{
    if (nMinMov>=0) return sal_uInt16(nMinMov);
    if (pOut==NULL)
    {
        pOut = GetFirstOutputDevice();
    }
    if (pOut!=NULL) {
        return short(-pOut->PixelToLogic(Size(nMinMov,0)).Width());
    } else {
        return 0;
    }
}

sal_uInt16 SdrPaintView::ImpGetHitTolLogic(short nHitTol, const OutputDevice* pOut) const
{
    if (nHitTol>=0) return sal_uInt16(nHitTol);
    if (pOut==NULL)
    {
        pOut = GetFirstOutputDevice();
    }
    if (pOut!=NULL) {
        return short(-pOut->PixelToLogic(Size(nHitTol,0)).Width());
    } else {
        return 0;
    }
}

void SdrPaintView::TheresNewMapMode()
{
    if (pActualOutDev!=NULL) {
        nHitTolLog=(sal_uInt16)((OutputDevice*)pActualOutDev)->PixelToLogic(Size(nHitTolPix,0)).Width();
        nMinMovLog=(sal_uInt16)((OutputDevice*)pActualOutDev)->PixelToLogic(Size(nMinMovPix,0)).Width();
    }
}

void SdrPaintView::SetActualWin(const OutputDevice* pWin)
{
    pActualOutDev=pWin;
    TheresNewMapMode();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPaintView::BegEncirclement(const Point& rPnt)
{
    BrkAction();

    DBG_ASSERT(0L == mpEncirclementOverlay, "SdrSnapView::BegSetPageOrg: There exists a ImplPageOriginOverlay (!)");
    basegfx::B2DPoint aStartPos(rPnt.X(), rPnt.Y());
    mpEncirclementOverlay = new ImplEncirclementOverlay(*this, aStartPos);

    aDragStat.Reset(rPnt);
    aDragStat.SetMinMove(ImpGetMinMovLogic(-2,0L));
    aDragStat.NextPoint();
}

void SdrPaintView::MovEncirclement(const Point& rPnt)
{
    if(IsEncirclement() && aDragStat.CheckMinMoved(rPnt))
    {
        aDragStat.NextMove(rPnt);

        DBG_ASSERT(mpEncirclementOverlay, "SdrSnapView::MovSetPageOrg: no ImplPageOriginOverlay (!)");
        basegfx::B2DPoint aNewPos(rPnt.X(), rPnt.Y());
        mpEncirclementOverlay->SetSecondPosition(aNewPos);
    }
}

Rectangle SdrPaintView::EndEncirclement(sal_Bool bNoJustify)
{
    Rectangle aRetval;

    if(IsEncirclement())
    {
        if(aDragStat.IsMinMoved())
        {
            aRetval = Rectangle(aDragStat.GetStart(), aDragStat.GetNow());

            if(!bNoJustify)
            {
                aRetval.Justify();
            }
        }

        // cleanup
        BrkEncirclement();
    }

    return aRetval;
}

void SdrPaintView::BrkEncirclement()
{
    if(IsEncirclement())
    {
        DBG_ASSERT(mpEncirclementOverlay, "SdrSnapView::MovSetPageOrg: no ImplPageOriginOverlay (!)");
        delete mpEncirclementOverlay;
        mpEncirclementOverlay = 0L;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPaintView::ClearPageView()
{
    BrkAction();

    if(mpPageView)
    {
        InvalidateAllWin();
        delete mpPageView;
        mpPageView = 0L;
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

        mpPageView = new SdrPageView(pPage, *((SdrView*)this));
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
        mpPageView = 0L;
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
    if (pItemBrowser!=NULL)
        pItemBrowser->ForceParent();
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
    if (pItemBrowser!=NULL)
        pItemBrowser->ForceParent();
#endif
}

void SdrPaintView::SetLayerVisible(const XubString& rName, sal_Bool bShow)
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

void SdrPaintView::SetAllLayersVisible(sal_Bool bShow)
{
    if(mpPageView)
    {
        mpPageView->SetAllLayersVisible(bShow);
    }

    InvalidateAllWin();
}

void SdrPaintView::SetLayerLocked(const XubString& rName, sal_Bool bLock)
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

void SdrPaintView::SetAllLayersLocked(sal_Bool bLock)
{
    if(mpPageView)
    {
        mpPageView->SetAllLayersLocked(bLock);
    }
}

void SdrPaintView::SetLayerPrintable(const XubString& rName, sal_Bool bPrn)
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

void SdrPaintView::SetAllLayersPrintable(sal_Bool bPrn)
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
                    RegionHandle aRegionHandle(aOptimizedRepaintRegion.BeginEnumRects());
                    Rectangle aRegionRectangle;

                    while(aOptimizedRepaintRegion.GetEnumRects(aRegionHandle, aRegionRectangle))
                    {
                        pWindow->SetLineColor(COL_LIGHTGREEN);
                        pWindow->SetFillColor();
                        pWindow->DrawRect(aRegionRectangle);
                    }

                    aOptimizedRepaintRegion.EndEnumRects(aRegionHandle);
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
        const bool bTextEditActive(IsTextEdit() && GetTextEditPageView());

        if(bTextEditActive)
        {
            // output PreRendering and destroy it so that it is not used for FormLayer
            // or overlay
            rPaintWindow.OutputPreRenderDevice(rPaintWindow.GetRedrawRegion());

            // draw old text edit stuff before overlay to have it as part of the background
            // ATM. This will be changed to have the text editing on the overlay, bit it
            // is not an easy thing to do, see BegTextEdit and the OutlinerView stuff used...
            if(bTextEditActive)
            {
                ImpTextEditDrawing(rPaintWindow);
            }

            // draw Overlay directly to window. This will save the contents of the window
            // in the RedrawRegion to the overlay background buffer, too.
            // This may lead to problems when reading from the screen is slow from the
            // graphics driver/graphiccard combination.
            rPaintWindow.DrawOverlay(rPaintWindow.GetRedrawRegion(), false);
        }
        else
        {
            // draw Overlay, also to PreRender device if exists
            rPaintWindow.DrawOverlay(rPaintWindow.GetRedrawRegion(), true);

            // output PreRendering
            rPaintWindow.OutputPreRenderDevice(rPaintWindow.GetRedrawRegion());
        }
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
                            RegionHandle aRegionHandle(aOptimizedRepaintRegion.BeginEnumRects());
                            Rectangle aRegionRectangle;

                            while(aOptimizedRepaintRegion.GetEnumRects(aRegionHandle, aRegionRectangle))
                            {
                                pWindow->SetLineColor(COL_LIGHTGREEN);
                                pWindow->SetFillColor();
                                pWindow->DrawRect(aRegionRectangle);
                            }

                            aOptimizedRepaintRegion.EndEnumRects(aRegionHandle);
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

void SdrPaintView::ImpTextEditDrawing(SdrPaintWindow& rPaintWindow) const
{
    // draw old text edit stuff
    if(IsTextEdit())
    {
        SdrPageView* pPageView = GetTextEditPageView();

        if(pPageView)
        {
            // paint TextEdit directly to the destination OutDev
            const Region& rRedrawRegion = rPaintWindow.GetRedrawRegion();
            const Rectangle aCheckRect(rRedrawRegion.GetBoundRect());
            pPageView->PaintOutlinerView(&rPaintWindow.GetOutputDevice(), aCheckRect);
        }
    }
}

void SdrPaintView::ImpFormLayerDrawing(SdrPaintWindow& rPaintWindow) const
{
    if(mpPageView)
    {
        SdrPageWindow* pKnownTarget = mpPageView->FindPageWindow(rPaintWindow);

        if(pKnownTarget)
        {
            const SdrModel& rModel = *(GetModel());
            const SdrLayerAdmin& rLayerAdmin = rModel.GetLayerAdmin();
            const SdrLayerID nControlLayerId = rLayerAdmin.GetLayerID(rLayerAdmin.GetControlLayerName(), sal_False);

            // BUFFERED use GetTargetOutputDevice() now, it may be targeted to VDevs, too
            // need to set PreparedPageWindow to make DrawLayer use the correct ObjectContact
            mpPageView->setPreparedPageWindow(pKnownTarget);
            mpPageView->DrawLayer(nControlLayerId, &rPaintWindow.GetTargetOutputDevice());
            mpPageView->setPreparedPageWindow(0);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SdrPaintView::KeyInput(const KeyEvent& /*rKEvt*/, Window* /*pWin*/)
{
    return sal_False;
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
                sal_uIntPtr nObjAnz=pOL->GetObjCount();
                for (sal_uIntPtr nObjNum=0; nObjNum<nObjAnz; nObjNum++) {
                    const SdrObject* pObj=pOL->GetObj(nObjNum);
                    const SdrGluePointList* pGPL=pObj->GetGluePointList();
                    if (pGPL!=NULL && pGPL->GetCount()!=0) {
                        pGPL->Invalidate((Window&)rOutDev, pObj);
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
            InvalidateOneWin((Window&)pPaintWindow->GetOutputDevice());
        }
    }
}

void SdrPaintView::InvalidateAllWin(const Rectangle& rRect, sal_Bool bPlus1Pix)
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

            if (aRect.IsOver(aOutRect))
            {
                InvalidateOneWin((Window&)rOutDev, aRect);
            }
        }
    }
}

void SdrPaintView::InvalidateOneWin(Window& rWin)
{
    // #111096#
    // do not erase background, that causes flicker (!)
    rWin.Invalidate(INVALIDATE_NOERASE);
}

void SdrPaintView::InvalidateOneWin(Window& rWin, const Rectangle& rRect)
{
    // #111096#
    // do not erase background, that causes flicker (!)
    rWin.Invalidate(rRect, INVALIDATE_NOERASE);
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

void SdrPaintView::SetNotPersistDefaultAttr(const SfxItemSet& rAttr, sal_Bool /*bReplaceAll*/)
{
    // bReplaceAll hat hier keinerlei Wirkung
    sal_Bool bMeasure=ISA(SdrView) && ((SdrView*)this)->IsMeasureTool();
    const SfxPoolItem *pPoolItem=NULL;
    if (rAttr.GetItemState(SDRATTR_LAYERID,sal_True,&pPoolItem)==SFX_ITEM_SET) {
        SdrLayerID nLayerId=((const SdrLayerIdItem*)pPoolItem)->GetValue();
        const SdrLayer* pLayer=pMod->GetLayerAdmin().GetLayerPerID(nLayerId);
        if (pLayer!=NULL) {
            if (bMeasure) aMeasureLayer=pLayer->GetName();
            else aAktLayer=pLayer->GetName();
        }
    }
    if (rAttr.GetItemState(SDRATTR_LAYERNAME,sal_True,&pPoolItem)==SFX_ITEM_SET) {
        if (bMeasure) aMeasureLayer=((const SdrLayerNameItem*)pPoolItem)->GetValue();
        else aAktLayer=((const SdrLayerNameItem*)pPoolItem)->GetValue();
    }
}

void SdrPaintView::MergeNotPersistDefaultAttr(SfxItemSet& rAttr, sal_Bool /*bOnlyHardAttr*/) const
{
    // bOnlyHardAttr hat hier keinerlei Wirkung
    sal_Bool bMeasure=ISA(SdrView) && ((SdrView*)this)->IsMeasureTool();
    const XubString& aNam=bMeasure?aMeasureLayer:aAktLayer;
    rAttr.Put(SdrLayerNameItem(aNam));
    SdrLayerID nLayer=pMod->GetLayerAdmin().GetLayerID(aNam,sal_True);
    if (nLayer!=SDRLAYER_NOTFOUND) {
        rAttr.Put(SdrLayerIdItem(nLayer));
    }
}

void SdrPaintView::SetDefaultAttr(const SfxItemSet& rAttr, sal_Bool bReplaceAll)
{
#ifdef DBG_UTIL
    {
        sal_Bool bHasEEFeatureItems=sal_False;
        SfxItemIter aIter(rAttr);
        const SfxPoolItem* pItem=aIter.FirstItem();
        while (!bHasEEFeatureItems && pItem!=NULL) {
            if (!IsInvalidItem(pItem)) {
                sal_uInt16 nW=pItem->Which();
                if (nW>=EE_FEATURE_START && nW<=EE_FEATURE_END) bHasEEFeatureItems=sal_True;
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
    if (bReplaceAll) aDefaultAttr.Set(rAttr);
    else aDefaultAttr.Put(rAttr,sal_False); // FALSE= InvalidItems nicht als Default, sondern als "Loecher" betrachten
    SetNotPersistDefaultAttr(rAttr,bReplaceAll);
#ifdef DBG_UTIL
    if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
#endif
}

void SdrPaintView::SetDefaultStyleSheet(SfxStyleSheet* pStyleSheet, sal_Bool bDontRemoveHardAttr)
{
    if (pDefaultStyleSheet)
        EndListening(*pDefaultStyleSheet);
    pDefaultStyleSheet=pStyleSheet;
    if (pDefaultStyleSheet)
        StartListening(*pDefaultStyleSheet);

    if (pStyleSheet!=NULL && !bDontRemoveHardAttr) {
        SfxWhichIter aIter(pStyleSheet->GetItemSet());
        sal_uInt16 nWhich=aIter.FirstWhich();
        while (nWhich!=0) {
            if (pStyleSheet->GetItemSet().GetItemState(nWhich,sal_True)==SFX_ITEM_SET) {
                aDefaultAttr.ClearItem(nWhich);
            }
            nWhich=aIter.NextWhich();
        }
    }
#ifdef DBG_UTIL
    if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
#endif
}

/* new interface src537 */
sal_Bool SdrPaintView::GetAttributes(SfxItemSet& rTargetSet, sal_Bool bOnlyHardAttr) const
{
    if(bOnlyHardAttr || !pDefaultStyleSheet)
    {
        rTargetSet.Put(aDefaultAttr, sal_False);
    }
    else
    {
        // sonst DefStyleSheet dazumergen
        rTargetSet.Put(pDefaultStyleSheet->GetItemSet(), sal_False);
        rTargetSet.Put(aDefaultAttr, sal_False);
    }
    MergeNotPersistDefaultAttr(rTargetSet, bOnlyHardAttr);
    return sal_True;
}

sal_Bool SdrPaintView::SetAttributes(const SfxItemSet& rSet, sal_Bool bReplaceAll)
{
    SetDefaultAttr(rSet,bReplaceAll);
    return sal_True;
}

SfxStyleSheet* SdrPaintView::GetStyleSheet() const // SfxStyleSheet* SdrPaintView::GetStyleSheet(sal_Bool& rOk) const
{
    //rOk=sal_True;
    return GetDefaultStyleSheet();
}

sal_Bool SdrPaintView::SetStyleSheet(SfxStyleSheet* pStyleSheet, sal_Bool bDontRemoveHardAttr)
{
    SetDefaultStyleSheet(pStyleSheet,bDontRemoveHardAttr);
    return sal_True;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DBG_UTIL
void SdrPaintView::ShowItemBrowser(sal_Bool bShow)
{
    if (bShow) {
        if (pItemBrowser==NULL) {
            pItemBrowser=new SdrItemBrowser(*(SdrView*)this);
            pItemBrowser->SetFloatingMode(sal_True);
        }
        pItemBrowser->Show();
        pItemBrowser->GrabFocus();
    } else {
        if (pItemBrowser!=NULL) {
            pItemBrowser->Hide();
            delete pItemBrowser;
            pItemBrowser=NULL;
        }
    }
}
#endif

void SdrPaintView::MakeVisible(const Rectangle& rRect, Window& rWin)
{
    MapMode aMap(rWin.GetMapMode());
    Size aActualSize(rWin.GetOutputSize());

    if( aActualSize.Height() > 0 && aActualSize.Width() > 0 )
    {
        Size aNewSize(rRect.GetSize());
        sal_Bool bNewScale=sal_False;
        sal_Bool bNeedMoreX=aNewSize.Width()>aActualSize.Width();
        sal_Bool bNeedMoreY=aNewSize.Height()>aActualSize.Height();
        if (bNeedMoreX || bNeedMoreY)
        {
            bNewScale=sal_True;
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

void SdrPaintView::SetAnimationEnabled( sal_Bool bEnable )
{
    SetAnimationMode( bEnable ? SDR_ANIMATION_ANIMATE : SDR_ANIMATION_DISABLE );
}

void SdrPaintView::SetAnimationPause( bool bSet )
{
    if((bool)bAnimationPause != bSet)
    {
        bAnimationPause = bSet;

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
    eAnimationMode = eMode;
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

sal_Bool SdrPaintView::IsPagePaintingAllowed() const
{
    return mbPagePaintingAllowed;
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
            const SdrPageWindow& rPageWindow = *mpPageView->GetPageWindow(a);
            sdr::contact::ObjectContact& rObjectContact = rPageWindow.GetObjectContact();
            sdr::animation::primitiveAnimator& rAnimator = rObjectContact.getPrimitiveAnimator();
            rAnimator.SetTime(nTime);
        }
    }
}

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
