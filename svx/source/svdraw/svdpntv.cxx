/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdpntv.cxx,v $
 *
 *  $Revision: 1.39 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 14:05:17 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif

#ifndef _SVDPNTV_HXX
#include <svx/svdpntv.hxx>
#endif

#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _SDRPAINTWINDOW_HXX
#include <sdrpaintwindow.hxx>
#endif

#ifndef _GRFMGR_HXX
#include <goodies/grfmgr.hxx>
#endif

#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif

#ifndef _XOUTX_HXX
#include <svx/xoutx.hxx>
#endif

#ifdef DBG_UTIL
#include <svdibrow.hxx>
#endif

#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif

#ifndef _SVDPAGV_HXX
#include <svx/svdpagv.hxx>
#endif

#ifndef _SFXSMPLHINT_HXX
#include <svtools/smplhint.hxx>
#endif

#include <svx/svdpntv.hxx>
#include <svx/editdata.hxx>
#include <svx/svdmrkv.hxx>
#include "svdxout.hxx"
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
#include "svditer.hxx"
#include <svx/svdouno.hxx>

#ifndef _SDR_OVERLAY_OVERLAYOBJECTLIST_HXX
#include <svx/sdr/overlay/overlayobjectlist.hxx>
#endif

#ifndef _SDR_OVERLAY_OVERLAYROOLINGRECTANGLE_HXX
#include <svx/sdr/overlay/overlayrollingrectangle.hxx>
#endif

#ifndef _SDR_OVERLAY_OVERLAYMANAGER_HXX
#include <svx/sdr/overlay/overlaymanager.hxx>
#endif

#ifndef _SVDGLUE_HXX
#include <svx/svdglue.hxx>
#endif

#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif

#ifndef _SVDVIEW_HXX
#include <svx/svdview.hxx>
#endif

#ifndef _SYLAYITM_HXX
#include <svx/sxlayitm.hxx>
#endif

#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif

#ifndef _EEITEM_HXX
#include <svx/eeitem.hxx>
#endif

#ifndef _SFX_WHITER_HXX
#include <svtools/whiter.hxx>
#endif

#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif

#ifndef _SDRPAGEWINDOW_HXX
#include <svx/sdrpagewindow.hxx>
#endif

#ifndef _SVDOUNO_HXX
#include <svx/svdouno.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROL_HPP_
#include <com/sun/star/awt/XControl.hpp>
#endif

// #i38135#
#ifndef _SDR_CONTACT_OBJECTCONTACT_HXX
#include <svx/sdr/contact/objectcontact.hxx>
#endif

#ifndef _SDR_ANIMATION_OBJECTANIMATOR_HXX
#include <svx/sdr/animation/objectanimator.hxx>
#endif

#ifndef _SDR_CONTACT_VIEWCONTACT_HXX
#include <svx/sdr/contact/viewcontact.hxx>
#endif

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
                rStartPos, rStartPos, sal_False);
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
    pXOut=NULL;
#ifdef DBG_UTIL
    pItemBrowser=NULL;
#endif
    bPageVisible=TRUE;
    bPageBorderVisible=TRUE;
    bBordVisible=TRUE;
    bBordVisibleOnlyLeftRight=FALSE;
    bGridVisible=TRUE;
    bGridFront  =FALSE;
    bHlplVisible=TRUE;
    bHlplFront  =TRUE;
    bGlueVisible=FALSE;
    bGlueVisible2=FALSE;
    bGlueVisible3=FALSE;
    bGlueVisible4=FALSE;
    bSwapAsynchron=FALSE;
    bPrintPreview=FALSE;
    mbPreviewRenderer=FALSE;

    eAnimationMode = SDR_ANIMATION_ANIMATE;
    bAnimationPause = FALSE;

    nHitTolPix=2;
    nMinMovPix=3;
    nHitTolLog=0;
    nMinMovLog=0;
    pActualOutDev=NULL;
    pDragWin=NULL;
    bRestoreColors=TRUE;
    pDefaultStyleSheet=NULL;
    bSomeObjChgdFlag=FALSE;
    nGraphicManagerDrawMode = GRFMGR_DRAW_STANDARD;
    aComeBackTimer.SetTimeout(1);
    aComeBackTimer.SetTimeoutHdl(LINK(this,SdrPaintView,ImpComeBackHdl));
    String aNam;    // System::GetUserName() just return an empty string

    if (pMod)
        SetDefaultStyleSheet(pMod->GetDefaultStyleSheet(), TRUE);

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
    mbPagePaintingAllowed(sal_True)
{
    DBG_CTOR(SdrPaintView,NULL);
    pMod=pModel1;
    ImpClearVars();

    if(pOut)
    {
        AddWindowToPaintView(pOut);
    }

    pXOut = new XOutputDevice(pOut);

    // Flag zur Visualisierung von Gruppen
    bVisualizeEnteredGroup = TRUE;

    StartListening( maColorConfig );
    onChangeColorConfig();
}

SdrPaintView::~SdrPaintView()
{
    DBG_DTOR(SdrPaintView,NULL);
    EndListening( maColorConfig );
    ClearPageView();

    if(pXOut)
    {
        delete pXOut;
    }

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

void __EXPORT SdrPaintView::SFX_NOTIFY(SfxBroadcaster& /*rBC*/, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType)
{
    BOOL bObjChg=!bSomeObjChgdFlag; // TRUE= auswerten fuer ComeBack-Timer
    if (bObjChg) {
        SdrHint* pSdrHint=PTR_CAST(SdrHint,&rHint);
        if (pSdrHint!=NULL) {
            SdrHintKind eKind=pSdrHint->GetKind();
            if (eKind==HINT_OBJCHG || eKind==HINT_OBJINSERTED || eKind==HINT_OBJREMOVED) {
                if (bObjChg) {
                    bSomeObjChgdFlag=TRUE;
                    aComeBackTimer.Start();
                }
            }
            if (eKind==HINT_PAGEORDERCHG) {
                const SdrPage* pPg=pSdrHint->GetPage();

                if(!pPg->IsInserted())
                {
                    if(mpPageView && mpPageView->GetPage() == pPg)
                    {
                        HideSdrPage();
                    }
                }
            }
        }
    }

    if( rHint.ISA( SfxSimpleHint ) && ( (SfxSimpleHint&) rHint ).GetId() == SFX_HINT_COLORS_CHANGED )
    {
        onChangeColorConfig();
        InvalidateAllWin();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

IMPL_LINK_INLINE_START(SdrPaintView,ImpComeBackHdl,Timer*,EMPTYARG)
{
    if (bSomeObjChgdFlag) {
        bSomeObjChgdFlag=FALSE;
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

BOOL SdrPaintView::IsAction() const
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

USHORT SdrPaintView::ImpGetMinMovLogic(short nMinMov, const OutputDevice* pOut) const
{
    if (nMinMov>=0) return USHORT(nMinMov);
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

USHORT SdrPaintView::ImpGetHitTolLogic(short nHitTol, const OutputDevice* pOut) const
{
    if (nHitTol>=0) return USHORT(nHitTol);
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
        nHitTolLog=(USHORT)((OutputDevice*)pActualOutDev)->PixelToLogic(Size(nHitTolPix,0)).Width();
        nMinMovLog=(USHORT)((OutputDevice*)pActualOutDev)->PixelToLogic(Size(nMinMovPix,0)).Width();
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

void SdrPaintView::SetLayerVisible(const XubString& rName, BOOL bShow)
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

void SdrPaintView::SetAllLayersVisible(BOOL bShow)
{
    if(mpPageView)
    {
        mpPageView->SetAllLayersVisible(bShow);
    }

    InvalidateAllWin();
}

void SdrPaintView::SetLayerLocked(const XubString& rName, BOOL bLock)
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

void SdrPaintView::SetAllLayersLocked(BOOL bLock)
{
    if(mpPageView)
    {
        mpPageView->SetAllLayersLocked(bLock);
    }
}

void SdrPaintView::SetLayerPrintable(const XubString& rName, BOOL bPrn)
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

void SdrPaintView::SetAllLayersPrintable(BOOL bPrn)
{
    if(mpPageView)
    {
        mpPageView->SetAllLayersPrintable(bPrn);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// #define SVX_REPAINT_TIMER_TEST

void SdrPaintView::CompleteRedraw(OutputDevice* pOut, const Region& rReg, USHORT nPaintMod, ::sdr::contact::ViewObjectContactRedirector* pRedirector)
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

    DoCompleteRedraw(*pPaintWindow, aOptimizedRepaintRegion, nPaintMod, pRedirector);
    EndCompleteRedraw(*pPaintWindow);

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

void SdrPaintView::DoCompleteRedraw(SdrPaintWindow& rPaintWindow, const Region& rReg, USHORT nPaintMod, ::sdr::contact::ViewObjectContactRedirector* pRedirector)
{
    // redraw all PageViews with the target. This may expand the RedrawRegion
    // at the PaintWindow, plus taking care of FormLayer expansion
    if(mpPageView)
    {
        mpPageView->CompleteRedraw(rPaintWindow, rReg, nPaintMod, pRedirector);
    }
}

void SdrPaintView::EndCompleteRedraw(SdrPaintWindow& rPaintWindow)
{
    if(rPaintWindow.getTemporaryTarget())
    {
        // get rid of temp target again
        delete (&rPaintWindow);
    }
    else
    {
        // draw postprocessing, only for known devices

        // #i74769# it is necessary to always paint FormLayer:
        //
        // - AreFormControlsUsed() is not very reliable since ATM ShouldPaintObject may create the
        //   objects, so XControlContainer may be empty
        // - In life mode (and thus always since we have controls which even exist as a child window
        //   when not in life mode) the child windows always need to be positioned and scaled, even
        //   when not visible (to not stay at their last visible position, but move outside reliably)
        // - last but not least: There may be other objects on the form layer (an error, but happened)
        ImpFormLayerDrawing(rPaintWindow);

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

void SdrPaintView::EndDrawLayers(SdrPaintWindow& rPaintWindow)
{
    // #i74769# use EndCompleteRedraw() as common base
    EndCompleteRedraw(rPaintWindow);

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

BOOL SdrPaintView::KeyInput(const KeyEvent& /*rKEvt*/, Window* /*pWin*/)
{
    return FALSE;
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
            pXOut->SetOutDev(&rOutDev);

            if(mpPageView)
            {
                const SdrObjList* pOL=mpPageView->GetObjList();
                pXOut->SetOffset(Point()); // pPV->GetOffset());
                ULONG nObjAnz=pOL->GetObjCount();
                for (ULONG nObjNum=0; nObjNum<nObjAnz; nObjNum++) {
                    const SdrObject* pObj=pOL->GetObj(nObjNum);
                    const SdrGluePointList* pGPL=pObj->GetGluePointList();
                    if (pGPL!=NULL && pGPL->GetCount()!=0) {
                        pGPL->Invalidate((Window&)rOutDev, pObj);
                    }
                }
            }
        }

        //pXOut->SetOffset(Point(0,0));
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

void SdrPaintView::InvalidateAllWin(const Rectangle& rRect, BOOL bPlus1Pix)
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

void SdrPaintView::SetNotPersistDefaultAttr(const SfxItemSet& rAttr, BOOL /*bReplaceAll*/)
{
    // bReplaceAll hat hier keinerlei Wirkung
    BOOL bMeasure=ISA(SdrView) && ((SdrView*)this)->IsMeasureTool();
    const SfxPoolItem *pPoolItem=NULL;
    if (rAttr.GetItemState(SDRATTR_LAYERID,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        SdrLayerID nLayerId=((const SdrLayerIdItem*)pPoolItem)->GetValue();
        const SdrLayer* pLayer=pMod->GetLayerAdmin().GetLayerPerID(nLayerId);
        if (pLayer!=NULL) {
            if (bMeasure) aMeasureLayer=pLayer->GetName();
            else aAktLayer=pLayer->GetName();
        }
    }
    if (rAttr.GetItemState(SDRATTR_LAYERNAME,TRUE,&pPoolItem)==SFX_ITEM_SET) {
        if (bMeasure) aMeasureLayer=((const SdrLayerNameItem*)pPoolItem)->GetValue();
        else aAktLayer=((const SdrLayerNameItem*)pPoolItem)->GetValue();
    }
}

void SdrPaintView::MergeNotPersistDefaultAttr(SfxItemSet& rAttr, BOOL /*bOnlyHardAttr*/) const
{
    // bOnlyHardAttr hat hier keinerlei Wirkung
    BOOL bMeasure=ISA(SdrView) && ((SdrView*)this)->IsMeasureTool();
    const XubString& aNam=bMeasure?aMeasureLayer:aAktLayer;
    rAttr.Put(SdrLayerNameItem(aNam));
    SdrLayerID nLayer=pMod->GetLayerAdmin().GetLayerID(aNam,TRUE);
    if (nLayer!=SDRLAYER_NOTFOUND) {
        rAttr.Put(SdrLayerIdItem(nLayer));
    }
}

void SdrPaintView::SetDefaultAttr(const SfxItemSet& rAttr, BOOL bReplaceAll)
{
#ifdef DBG_UTIL
    {
        BOOL bHasEEFeatureItems=FALSE;
        SfxItemIter aIter(rAttr);
        const SfxPoolItem* pItem=aIter.FirstItem();
        while (!bHasEEFeatureItems && pItem!=NULL) {
            if (!IsInvalidItem(pItem)) {
                USHORT nW=pItem->Which();
                if (nW>=EE_FEATURE_START && nW<=EE_FEATURE_END) bHasEEFeatureItems=TRUE;
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
    else aDefaultAttr.Put(rAttr,FALSE); // FALSE= InvalidItems nicht als Default, sondern als "Loecher" betrachten
    SetNotPersistDefaultAttr(rAttr,bReplaceAll);
#ifdef DBG_UTIL
    if (pItemBrowser!=NULL) pItemBrowser->SetDirty();
#endif
}

void SdrPaintView::SetDefaultStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr)
{
    pDefaultStyleSheet=pStyleSheet;
    if (pStyleSheet!=NULL && !bDontRemoveHardAttr) {
        SfxWhichIter aIter(pStyleSheet->GetItemSet());
        USHORT nWhich=aIter.FirstWhich();
        while (nWhich!=0) {
            if (pStyleSheet->GetItemSet().GetItemState(nWhich,TRUE)==SFX_ITEM_SET) {
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
BOOL SdrPaintView::GetAttributes(SfxItemSet& rTargetSet, BOOL bOnlyHardAttr) const
{
    if(bOnlyHardAttr || !pDefaultStyleSheet)
    {
        rTargetSet.Put(aDefaultAttr, FALSE);
    }
    else
    {
        // sonst DefStyleSheet dazumergen
        rTargetSet.Put(pDefaultStyleSheet->GetItemSet(), FALSE);
        rTargetSet.Put(aDefaultAttr, FALSE);
    }
    MergeNotPersistDefaultAttr(rTargetSet, bOnlyHardAttr);
    return TRUE;
}

BOOL SdrPaintView::SetAttributes(const SfxItemSet& rSet, BOOL bReplaceAll)
{
    SetDefaultAttr(rSet,bReplaceAll);
    return TRUE;
}

SfxStyleSheet* SdrPaintView::GetStyleSheet() const // SfxStyleSheet* SdrPaintView::GetStyleSheet(BOOL& rOk) const
{
    //rOk=TRUE;
    return GetDefaultStyleSheet();
}

BOOL SdrPaintView::SetStyleSheet(SfxStyleSheet* pStyleSheet, BOOL bDontRemoveHardAttr)
{
    SetDefaultStyleSheet(pStyleSheet,bDontRemoveHardAttr);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DBG_UTIL
void SdrPaintView::ShowItemBrowser(BOOL bShow)
{
    if (bShow) {
        if (pItemBrowser==NULL) {
            pItemBrowser=new SdrItemBrowser(*(SdrView*)this);
            pItemBrowser->SetFloatingMode(TRUE);
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
        BOOL bNewScale=FALSE;
        BOOL bNeedMoreX=aNewSize.Width()>aActualSize.Width();
        BOOL bNeedMoreY=aNewSize.Height()>aActualSize.Height();
        if (bNeedMoreX || bNeedMoreY)
        {
            bNewScale=TRUE;
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

void SdrPaintView::SetAnimationEnabled( BOOL bEnable )
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

                if(rObjectContact.HasObjectAnimator())
                {
                    sdr::animation::ObjectAnimator& rAnimator = rObjectContact.GetObjectAnimator();

                    if(rAnimator.IsPaused() != bSet)
                    {
                        rAnimator.SetPaused(bSet);
                    }
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

// declaration extracted from svdedxv.cxx
#define SPOTCOUNT   5

Color SdrPaintView::CalcBackgroundColor( const Rectangle& rArea,
                                         const SetOfByte& rVisibleLayers,
                                         const SdrPage&   rCurrPage ) const
{
    // code extracted from SdrObjEditView::ImpGetTextEditBackgroundColor
    svtools::ColorConfig aColorConfig;
    Color aBackground(aColorConfig.GetColorValue(svtools::DOCCOLOR).nColor);

    // #98988# test if we are in High contrast mode; if yes, take
    // application background color
    // #10049# wrong, always use svtools::DOCCOLOR as default and use document settings if
    //         not hc mode
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

    if(!rStyleSettings.GetHighContrastMode())
    {
        // Ok, dann eben die Page durchsuchen!
        Point aSpotPos[SPOTCOUNT];
        Color aSpotColor[SPOTCOUNT];
        ULONG nHeight( rArea.GetSize().Height() );
        ULONG nWidth( rArea.GetSize().Width() );
        ULONG nWidth14  = nWidth / 4;
        ULONG nHeight14 = nHeight / 4;
        ULONG nWidth34  = ( 3 * nWidth ) / 4;
        ULONG nHeight34 = ( 3 * nHeight ) / 4;

        USHORT i;
        for ( i = 0; i < SPOTCOUNT; i++ )
        {
            // Es wird anhand von fuenf Spots die Farbe untersucht
            switch ( i )
            {
                case 0 :
                {
                    // Center-Spot
                    aSpotPos[i] = rArea.Center();
                }
                break;

                case 1 :
                {
                    // TopLeft-Spot
                    aSpotPos[i] = rArea.TopLeft();
                    aSpotPos[i].X() += nWidth14;
                    aSpotPos[i].Y() += nHeight14;
                }
                break;

                case 2 :
                {
                    // TopRight-Spot
                    aSpotPos[i] = rArea.TopLeft();
                    aSpotPos[i].X() += nWidth34;
                    aSpotPos[i].Y() += nHeight14;
                }
                break;

                case 3 :
                {
                    // BottomLeft-Spot
                    aSpotPos[i] = rArea.TopLeft();
                    aSpotPos[i].X() += nWidth14;
                    aSpotPos[i].Y() += nHeight34;
                }
                break;

                case 4 :
                {
                    // BottomRight-Spot
                    aSpotPos[i] = rArea.TopLeft();
                    aSpotPos[i].X() += nWidth34;
                    aSpotPos[i].Y() += nHeight34;
                }
                break;

            }

            aSpotColor[i] = Color( COL_WHITE );
            rCurrPage.GetFillColor(aSpotPos[i], rVisibleLayers, /*bLayerSortedRedraw,*/ aSpotColor[i]);
        }

        USHORT aMatch[SPOTCOUNT];

        for ( i = 0; i < SPOTCOUNT; i++ )
        {
            // Wurden gleiche Spot-Farben gefuden?
            aMatch[i] = 0;

            for ( USHORT j = 0; j < SPOTCOUNT; j++ )
            {
                if( j != i )
                {
                    if( aSpotColor[i] == aSpotColor[j] )
                    {
                        aMatch[i]++;
                    }
                }
            }
        }

        // Das hoechste Gewicht hat der Spot in der Mitte
        aBackground = aSpotColor[0];

        for ( USHORT nMatchCount = SPOTCOUNT - 1; nMatchCount > 1; nMatchCount-- )
        {
            // Welche Spot-Farbe wurde am haeufigsten gefunden?
            for ( i = 0; i < SPOTCOUNT; i++ )
            {
                if( aMatch[i] == nMatchCount )
                {
                    aBackground = aSpotColor[i];
                    nMatchCount = 1;   // Abbruch auch der aeusseren for-Schleife
                    break;
                }
            }
        }
    }

    return aBackground;
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

            if(rObjectContact.HasObjectAnimator())
            {
                sdr::animation::ObjectAnimator& rAnimator = rObjectContact.GetObjectAnimator();
                rAnimator.SetTime(nTime);
            }
        }
    }
}

// eof
