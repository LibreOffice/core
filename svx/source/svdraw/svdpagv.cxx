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

#include <svx/svdpagv.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/awt/PosSize.hpp>
#include <comphelper/processfactory.hxx>
#include <svx/svdoutl.hxx>
#include <svx/xpoly.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>
#include <svx/svdedxv.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdoutl.hxx>
#include <svx/svdpagv.hxx>
#include <editeng/outliner.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>
#include <svx/svditer.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdtypes.hxx>
#include <svx/svdoole2.hxx>
#include <svx/sdr/contact/objectcontactofpageview.hxx>
#include <svx/svdogrp.hxx>
#include <svx/sdr/contact/viewobjectcontactredirector.hxx>
#include <svx/fmview.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/svdlegacy.hxx>
#include <algorithm>

using namespace ::rtl;
using namespace ::com::sun::star;

////////////////////////////////////////////////////////////////////////////////////////////////////

SdrPageWindow* SdrPageView::FindPageWindow(SdrPaintWindow& rPaintWindow) const
{
    for(SdrPageWindowVector::const_iterator a = maPageWindows.begin(); a != maPageWindows.end(); a++)
    {
        if(&((*a)->GetPaintWindow()) == &rPaintWindow)
        {
            return *a;
        }
    }

    return 0;
}

const SdrPageWindow* SdrPageView::FindPatchedPageWindow( const OutputDevice& _rOutDev ) const
{
    for(SdrPageWindowVector::const_iterator loop = maPageWindows.begin(); loop != maPageWindows.end(); ++loop)
    {
        const SdrPageWindow& rPageWindow( *(*loop) );
        const SdrPaintWindow& rPaintWindow( rPageWindow.GetOriginalPaintWindow() ? *rPageWindow.GetOriginalPaintWindow() : rPageWindow.GetPaintWindow() );

        if ( &rPaintWindow.GetOutputDevice() == &_rOutDev )
        {
            return &rPageWindow;
        }
    }

    return 0;
}

SdrPageWindow* SdrPageView::FindPageWindow(const OutputDevice& rOutDev) const
{
    for(SdrPageWindowVector::const_iterator a = maPageWindows.begin(); a != maPageWindows.end(); a++)
    {
        if(&((*a)->GetPaintWindow().GetOutputDevice()) == &rOutDev)
        {
            return *a;
        }
    }

    return 0;
}

SdrPageWindow* SdrPageView::GetPageWindow(sal_uInt32 nIndex) const
{
    if(nIndex < maPageWindows.size())
    {
        return maPageWindows[nIndex];
    }

    return 0;
}

void SdrPageView::ClearPageWindows()
{
    for(SdrPageWindowVector::const_iterator a = maPageWindows.begin(); a != maPageWindows.end(); a++)
    {
        delete *a;
    }

    maPageWindows.clear();
}

void SdrPageView::AppendPageWindow(SdrPageWindow& rNew)
{
    maPageWindows.push_back(&rNew);
}

SdrPageWindow* SdrPageView::RemovePageWindow(sal_uInt32 nPos)
{
    SdrPageWindow* pRetval = 0;

    if(nPos < maPageWindows.size())
    {
        const SdrPageWindowVector::iterator aCandidate(maPageWindows.begin() + nPos);

        pRetval = *aCandidate;
        maPageWindows.erase(aCandidate);
    }
    else
    {
        OSL_ENSURE(false, "SdrPageView::RemovePageWindow with wrong index (!)");
    }

    return 0;
}

SdrPageWindow* SdrPageView::RemovePageWindow(SdrPageWindow& rOld)
{
    const SdrPageWindowVector::iterator aFindResult = ::std::find(maPageWindows.begin(), maPageWindows.end(), &rOld);
    SdrPageWindow* pRetval = 0;

    if(aFindResult != maPageWindows.end())
    {
        pRetval = *aFindResult;
        maPageWindows.erase(aFindResult);
    }

    return pRetval;
}

SdrPageView::SdrPageView(SdrPage& rSdrPageOfSdrPageView, SdrView& rNewView)
:   mrView(rNewView),
    mrSdrPageOfSdrPageView(rSdrPageOfSdrPageView),
    maPageOrigin(0.0, 0.0),
    maVisibleLayerSet(),
    maLockedLayerSet(),
    maPrintableLayerSet(),
    mpCurrentList(0),
    mpCurrentGroup(0),
    maHelpLines(),
    maDocumentColor( COL_AUTO ), // #103911# col_auto color lets the view takes the default SvxColorConfig entry
    maBackgroundColor(COL_AUTO ), // #i48367# also react on autocolor
    maPageWindows(),
    mpPreparedPageWindow(0), // #i72752#
    mbVisible(false)
{
    maPageOrigin.setX(mrSdrPageOfSdrPageView.GetLeftPageBorder());
    maPageOrigin.setY(mrSdrPageOfSdrPageView.GetTopPageBorder());

    maVisibleLayerSet.SetAll();
    maPrintableLayerSet.SetAll();

    SetCurrentGroupAndObjectList(0, &mrSdrPageOfSdrPageView);

    for(sal_uInt32 a(0); a < rNewView.PaintWindowCount(); a++)
    {
        AddPaintWindowToPageView(*rNewView.GetPaintWindow(a));
    }
}

SdrPageView::~SdrPageView()
{
    // cleanup window vector
    ClearPageWindows();
}

SdrPageWindow& SdrPageView::CreateNewPageWindowEntry(SdrPaintWindow& rPaintWindow)
{
    SdrPageWindow& rWindow = *(new SdrPageWindow(*this, rPaintWindow));
    AppendPageWindow(rWindow);

    return rWindow;
}

void SdrPageView::AddPaintWindowToPageView(SdrPaintWindow& rPaintWindow)
{
    if(!FindPageWindow(rPaintWindow))
    {
        CreateNewPageWindowEntry(rPaintWindow);
    }
}

void SdrPageView::RemovePaintWindowFromPageView(SdrPaintWindow& rPaintWindow)
{
    SdrPageWindow* pCandidate = FindPageWindow(rPaintWindow);

    if(pCandidate)
    {
        pCandidate = RemovePageWindow(*pCandidate);

        if(pCandidate)
        {
            delete pCandidate;
        }
    }
}

::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > SdrPageView::GetControlContainer( const OutputDevice& _rDevice ) const
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > xReturn;
    const SdrPageWindow* pCandidate = FindPatchedPageWindow( _rDevice );

    if ( pCandidate )
        xReturn = pCandidate->GetControlContainer( true );

    return xReturn;
}

void SdrPageView::LazyReactOnObjectChanges()
{
    if(GetCurrentGroup())
    {
        CheckAktGroup();
    }
}

bool SdrPageView::IsReadOnly() const
{
    return GetView().getSdrModelFromSdrView().IsReadOnly();
}

void SdrPageView::Show()
{
    if(!IsVisible())
    {
        mbVisible = true;
        InvalidateAllWin();

        for(sal_uInt32 a(0); a < GetView().PaintWindowCount(); a++)
        {
            AddPaintWindowToPageView(*GetView().GetPaintWindow(a));
        }
    }
}

void SdrPageView::Hide()
{
    if(IsVisible())
    {
        InvalidateAllWin();
        mbVisible = false;
        ClearPageWindows();
    }
}

basegfx::B2DRange SdrPageView::GetPageRange() const
{
    basegfx::B2DRange aRetval;

    aRetval.expand(basegfx::B2DPoint(0.0, 0.0));
    aRetval.expand(getSdrPageFromSdrPageView().GetPageScale());

    return aRetval;
}

void SdrPageView::InvalidateAllWin()
{
    if(IsVisible())
    {
        basegfx::B2DRange aAllRange(GetPageRange());

        aAllRange.expand(sdr::legacy::GetAllObjBoundRange(getSdrPageFromSdrPageView().getSdrObjectVector()));
        GetView().InvalidateAllWin(aAllRange);
    }
}

void SdrPageView::InvalidateAllWin(const basegfx::B2DRange& rRange, bool bPlus1Pix)
{
    if(IsVisible())
    {
        GetView().InvalidateAllWin(rRange, bPlus1Pix);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPageView::PrePaint()
{
    const sal_uInt32 nCount(PageWindowCount());

    for(sal_uInt32 a(0); a < nCount; a++)
    {
        SdrPageWindow* pCandidate = GetPageWindow(a);

        if(pCandidate)
        {
            pCandidate->PrePaint();
        }
    }
}

void SdrPageView::PostPaint()
{
    const sal_uInt32 nCount(PageWindowCount());

    for(sal_uInt32 a(0); a < nCount; a++)
    {
        SdrPageWindow* pCandidate = GetPageWindow(a);

        if(pCandidate)
        {
            pCandidate->PostPaint();
        }
    }
}

void SdrPageView::CompleteRedraw(SdrPaintWindow& rPaintWindow, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector) const
{
    SdrPageWindow* pPageWindow = FindPageWindow(rPaintWindow);
    bool bIsTempTarget(false);

    if(!pPageWindow)
    {
        // create temp PageWindow
        pPageWindow = new SdrPageWindow(*((SdrPageView*)this), rPaintWindow);
        bIsTempTarget = true;
    }

    // do the redraw
    pPageWindow->PrepareRedraw(rReg);
    pPageWindow->RedrawAll(pRedirector);

    // get rid of temp PageWindow
    if(bIsTempTarget)
    {
        delete pPageWindow;
        pPageWindow = 0L;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// #i74769# use SdrPaintWindow directly

void SdrPageView::setPreparedPageWindow(SdrPageWindow* pKnownTarget)
{
    // #i72752# remember prepared SdrPageWindow
    mpPreparedPageWindow = pKnownTarget;
}

void SdrPageView::DrawLayer(SdrLayerID nID, OutputDevice* pGivenTarget, sdr::contact::ViewObjectContactRedirector* pRedirector) const
{
    if(pGivenTarget)
    {
        const SdrPageWindow* pKnownTarget = FindPageWindow(*pGivenTarget);

        if(pKnownTarget)
        {
            // paint known target
            pKnownTarget->RedrawLayer(&nID, pRedirector);
        }
        else
        {
            // #i72752# DrawLayer() uses a OutputDevice different from BeginDrawLayer. This happens
            // e.g. when SW paints a single text line in text edit mode. Try to use it
            SdrPageWindow* pPreparedTarget = mpPreparedPageWindow;

            if(pPreparedTarget)
            {
                // if we have a prepared target, do not use a new SdrPageWindow since this
                // works but is expensive. Just use a temporary PaintWindow
                SdrPaintWindow aTemporaryPaintWindow(mrView, *pGivenTarget);

                // Copy existing paint region to use the same as prepared in BeginDrawLayer
                SdrPaintWindow& rExistingPaintWindow = pPreparedTarget->GetPaintWindow();
                const Region& rExistingRegion = rExistingPaintWindow.GetRedrawRegion();
                aTemporaryPaintWindow.SetRedrawRegion(rExistingRegion);

                // patch the ExistingPageWindow
                pPreparedTarget->patchPaintWindow(aTemporaryPaintWindow);

                // redraw the layer
                pPreparedTarget->RedrawLayer(&nID, pRedirector);

                // restore the ExistingPageWindow
                pPreparedTarget->unpatchPaintWindow();
            }
            else
            {
                OSL_ENSURE(false, "SdrPageView::DrawLayer: Creating temporary SdrPageWindow (ObjectContact), this should never be needed (!)");

                // None of the known OutputDevices is the target of this paint, use
                // a temporary SdrPageWindow for this Redraw.
                SdrPaintWindow aTemporaryPaintWindow(mrView, *pGivenTarget);
                SdrPageWindow aTemporaryPageWindow(*((SdrPageView*)this), aTemporaryPaintWindow);

                // #i72752#
                // Copy existing paint region if other PageWindows exist, this was created by
                // PrepareRedraw() from BeginDrawLayer(). Needs to be used e.g. when suddenly SW
                // paints into an unknown device other than the view was created for (e.g. VirtualDevice)
                if(PageWindowCount())
                {
                    SdrPageWindow* pExistingPageWindow = GetPageWindow(0L);
                    SdrPaintWindow& rExistingPaintWindow = pExistingPageWindow->GetPaintWindow();
                    const Region& rExistingRegion = rExistingPaintWindow.GetRedrawRegion();
                    aTemporaryPaintWindow.SetRedrawRegion(rExistingRegion);
                }

                aTemporaryPageWindow.RedrawLayer(&nID, pRedirector);
            }
        }
    }
    else
    {
        // paint in all known windows
        for(sal_uInt32 a(0L); a < PageWindowCount(); a++)
        {
            SdrPageWindow* pTarget = GetPageWindow(a);
            pTarget->RedrawLayer(&nID, pRedirector);
        }
    }
}

void SdrPageView::SetDesignMode( bool _bDesignMode )
{
    for ( sal_uInt32 i = 0L; i < PageWindowCount(); ++i )
    {
        SdrPageWindow& rPageViewWindow = *GetPageWindow(i);
        rPageViewWindow.SetDesignMode( _bDesignMode );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrPageView::DrawPageViewGrid(OutputDevice& rOut, const basegfx::B2DRange& rRange, Color aColor)
{
    long nx1=GetView().GetGridCoarse().Width();
    long nx2=GetView().GetGridFine().Width();
    long ny1=GetView().GetGridCoarse().Height();
    long ny2=GetView().GetGridFine().Height();

    if (nx1==0) nx1=nx2;
    if (nx2==0) nx2=nx1;
    if (ny1==0) ny1=ny2;
    if (ny2==0) ny2=ny1;
    if (nx1==0) { nx1=ny1; nx2=ny2; }
    if (ny1==0) { ny1=nx1; ny2=nx2; }
    if (nx1<0) nx1=-nx1;
    if (nx2<0) nx2=-nx2;
    if (ny1<0) ny1=-ny1;
    if (ny2<0) ny2=-ny2;

    if (nx1!=0)
    {
        // no more global output size, use window size instead to decide grid sizes
        long nScreenWdt = rOut.GetOutputSizePixel().Width();
        // old: long nScreenWdt=System::GetDesktopRectPixel().GetWidth();

        // Grid bei kleinen Zoomstufen etwas erweitern
        //Size a1PixSiz(rOut.PixelToLogic(Size(1,1)));
        long nMinDotPix=2;
        long nMinLinPix=4;

        if (nScreenWdt>=1600)
        {
            nMinDotPix=4;
            nMinLinPix=8;
        }
        else if (nScreenWdt>=1024)
        {
            nMinDotPix=3;
            nMinLinPix=6;
        }
        else
        { // z.B. 640x480
            nMinDotPix=2;
            nMinLinPix=4;
        }
        Size aMinDotDist(rOut.PixelToLogic(Size(nMinDotPix,nMinDotPix)));
        //Size a3PixSiz(rOut.PixelToLogic(Size(2,2)));
        Size aMinLinDist(rOut.PixelToLogic(Size(nMinLinPix,nMinLinPix)));
        bool bHoriSolid=nx2<aMinDotDist.Width();
        bool bVertSolid=ny2<aMinDotDist.Height();
        // Linienabstand vergroessern (mind. 4 Pixel)
        // Vergroesserung: *2 *5 *10 *20 *50 *100 ...
        int nTgl=0;
        long nVal0=nx1;
        while (nx1<aMinLinDist.Width())
        {
            long a=nx1;

            if (nTgl==0) nx1*=2;
            if (nTgl==1) nx1=nVal0*5; // => nx1*=2.5
            if (nTgl==2) nx1*=2;

            nVal0=a;
            nTgl++; if (nTgl>=3) nTgl=0;
        }
        nTgl=0;
        nVal0=ny1;
        while (ny1<aMinLinDist.Height())
        {
            long a=ny1;

            if (nTgl==0) ny1*=2;
            if (nTgl==1) ny1=nVal0*5; // => ny1*=2.5
            if (nTgl==2) ny1*=2;

            nVal0=a;
            nTgl++;

            if (nTgl>=3) nTgl=0;
        }
        // Keine Zwischenpunkte, wenn...
        //if (nx2<a2PixSiz.Width()) nx2=nx1;
        //if (ny2<a2PixSiz.Height()) ny2=ny1;

        bool bHoriFine=nx2<nx1;
        bool bVertFine=ny2<ny1;
        bool bHoriLines=bHoriSolid || bHoriFine || !bVertFine;
        bool bVertLines=bVertSolid || bVertFine;

        Color aColorMerk( rOut.GetLineColor() );
        rOut.SetLineColor( aColor );

        bool bMap0=rOut.IsMapModeEnabled();

        double fWrX(0.0);
        double fWrY(0.0);
        basegfx::B2DPoint aOrg(maPageOrigin);
        const basegfx::B2DRange& rInnerPageRange(getSdrPageFromSdrPageView().GetInnerPageRange());
        double x1(rInnerPageRange.getMinX());
        double x2(rInnerPageRange.getMaxX());
        double y1(rInnerPageRange.getMinY());
        double y2(rInnerPageRange.getMaxY());
        const SdrPageGridFrameList* pFrames = getSdrPageFromSdrPageView().GetGridFrameList(GetView(), 0);
        sal_uInt32 nGridPaintAnz(1);

        if(pFrames)
        {
            nGridPaintAnz = pFrames->GetCount();
        }

        for(sal_uInt32 nGridPaintNum(0); nGridPaintNum < nGridPaintAnz; nGridPaintNum++)
        {
            if(pFrames)
            {
                const SdrPageGridFrame& rGF=(*pFrames)[nGridPaintNum];
                fWrX = rGF.GetPaperRect().getMinX();
                fWrY = rGF.GetPaperRect().getMinY();
                x1 = rGF.GetUserArea().getMinX();
                x2 = rGF.GetUserArea().getMaxX();
                y1 = rGF.GetUserArea().getMinY();
                y2 = rGF.GetUserArea().getMaxY();

                aOrg.setX(rGF.GetUserArea().getMinX() - rGF.GetPaperRect().getMinX());
                aOrg.setY(rGF.GetUserArea().getMinY() - rGF.GetPaperRect().getMinY());
            }

            if(!rRange.isEmpty())
            {
                Size a1PixSiz(rOut.PixelToLogic(Size(1,1)));
                long nX1Pix=a1PixSiz.Width();  // 1 Pixel Toleranz drauf
                long nY1Pix=a1PixSiz.Height();

                if(x1 < rRange.getMinX() - nX1Pix) x1 = rRange.getMinX() - nX1Pix;
                if(x2 > rRange.getMaxX() + nX1Pix) x2 = rRange.getMaxX() + nX1Pix;
                if(y1 < rRange.getMinY() - nY1Pix) y1 = rRange.getMinY() - nY1Pix;
                if(y2 > rRange.getMaxY() + nY1Pix) y2 = rRange.getMaxY() + nY1Pix;
            }

            double xBigOrg(aOrg.getX() + fWrX);
            while (xBigOrg>=x1) xBigOrg-=nx1;
            while (xBigOrg<x1) xBigOrg+=nx1;

            double xFinOrg(xBigOrg);
            while (xFinOrg>=x1) xFinOrg-=nx2;
            while (xFinOrg<x1) xFinOrg+=nx2;

            double yBigOrg(aOrg.getY() + fWrY);
            while (yBigOrg>=y1) yBigOrg-=ny1;
            while (yBigOrg<y1) yBigOrg+=ny1;

            double yFinOrg(yBigOrg);
            while (yFinOrg>=y1) yFinOrg-=ny2;
            while (yFinOrg<y1) yFinOrg+=ny2;

            if( x1 <= x2 && y1 <= y2 )
            {
                if( bHoriLines )
                {
                    sal_uInt32 nGridFlags = ( bHoriSolid ? GRID_HORZLINES : GRID_DOTS );
                    sal_uInt16 nSteps = sal_uInt16(nx1 / nx2);
                    sal_uInt32 nRestPerStepMul1000 = nSteps ? ( ((nx1 * 1000L)/ nSteps) - (nx2 * 1000L) ) : 0;
                    sal_uInt32 nStepOffset = 0;
                    sal_uInt16 nPointOffset = 0;

                    for(sal_uInt16 a(0); a < nSteps; a++)
                    {
                        // Zeichnen
                        rOut.DrawGrid(
                            Rectangle(
                                basegfx::fround(xFinOrg) + (a * nx2) + nPointOffset,
                                basegfx::fround(yBigOrg),
                                basegfx::fround(x2),
                                basegfx::fround(y2)),
                            Size(nx1, ny1),
                            nGridFlags);

                        // Schritt machen
                        nStepOffset += nRestPerStepMul1000;

                        while(nStepOffset >= 1000)
                        {
                            nStepOffset -= 1000;
                            nPointOffset++;
                        }
                    }
                }

                if( bVertLines )
                {
                    sal_uInt32 nGridFlags = ( bVertSolid ? GRID_VERTLINES : GRID_DOTS );
                    sal_uInt16 nSteps = sal_uInt16(ny1 / ny2);
                    sal_uInt32 nRestPerStepMul1000 = nSteps ? ( ((ny1 * 1000L)/ nSteps) - (ny2 * 1000L) ) : 0;
                    sal_uInt32 nStepOffset = 0;
                    sal_uInt16 nPointOffset = 0;

                    for(sal_uInt16 a(0); a < nSteps; a++)
                    {
                        // Zeichnen
                        rOut.DrawGrid(
                            Rectangle(
                                basegfx::fround(xBigOrg),
                                basegfx::fround(yFinOrg) + (a * ny2) + nPointOffset,
                                basegfx::fround(x2),
                                basegfx::fround(y2)),
                            Size(nx1, ny1),
                            nGridFlags);

                        // Schritt machen
                        nStepOffset += nRestPerStepMul1000;

                        while(nStepOffset >= 1000)
                        {
                            nStepOffset -= 1000;
                            nPointOffset++;
                        }
                    }
                }
            }
        }

        rOut.EnableMapMode(bMap0);
        rOut.SetLineColor(aColorMerk);
    }
}

void SdrPageView::AdjHdl()
{
    GetView().SetMarkHandles();
}

void SdrPageView::SetLayer(const XubString& rName, SetOfByte& rBS, bool bJa)
{
    SdrLayerID nID = getSdrPageFromSdrPageView().GetPageLayerAdmin().GetLayerID(rName, true);

    if(SDRLAYER_NOTFOUND != nID)
        rBS.Set(nID, bJa);
}

bool SdrPageView::IsLayer(const XubString& rName, const SetOfByte& rBS) const
{
    bool bRet(false);

    if(rName.Len())
    {
        SdrLayerID nId = getSdrPageFromSdrPageView().GetPageLayerAdmin().GetLayerID(rName, true);

        if(SDRLAYER_NOTFOUND != nId)
        {
            bRet = rBS.IsSet(nId);
        }
    }

    return bRet;
}

void SdrPageView::SetAllLayers(SetOfByte& rB, bool bJa)
{
    if(bJa)
    {
        rB.SetAll();
        rB.Clear(SDRLAYER_NOTFOUND);
    }
    else
    {
        rB.ClearAll();
    }
}

bool SdrPageView::IsObjMarkable(const SdrObject& rObj) const
{
    // Vom Markieren ausgeschlossen?
    if(rObj.IsMarkProtect())
    {
        return false;
    }

        // only visible are markable
    if( !rObj.IsVisible() )
    {
        return false;
    }

        // #112440#
    if(dynamic_cast< const SdrObjGroup* >(&rObj))
    {
        // If object is a Group object, visibility depends evtl. on
        // multiple layers. If one object is markable, Group is markable.
        SdrObjList* pObjList = rObj.getChildrenOfSdrObject();

        if(pObjList && pObjList->GetObjCount())
        {
            bool bGroupIsMarkable(false);

            for(sal_uInt32 a(0); !bGroupIsMarkable && a < pObjList->GetObjCount(); a++)
            {
                SdrObject* pCandidate = pObjList->GetObj(a);

                // call recursively
                if(IsObjMarkable(*pCandidate))
                {
                    bGroupIsMarkable = true;
                }
            }

            return bGroupIsMarkable;
        }
        else
        {
            // #i43302#
            // Allow empty groups to be selected to be able to delete them
            return true;
        }
    }
    else
    {
        // Der Layer muss sichtbar und darf nicht gesperrt sein
        SdrLayerID nL = rObj.GetLayer();
        return (maVisibleLayerSet.IsSet(sal_uInt8(nL)) && !maLockedLayerSet.IsSet(sal_uInt8(nL)));
    }

    return false;
}

void SdrPageView::SetPageOrigin(const basegfx::B2DPoint& rOrg)
{
    if(rOrg != GetPageOrigin())
    {
        maPageOrigin = rOrg;

        if(GetView().IsGridVisible())
        {
            InvalidateAllWin();
        }
    }
}

void SdrPageView::ImpInvalidateHelpLineArea(sal_uInt16 nNum) const
{
    if(GetView().IsHlplVisible() && nNum < maHelpLines.GetCount())
    {
        const SdrHelpLine& rHelpLine = maHelpLines[nNum];

        for(sal_uInt32 a(0); a < GetView().PaintWindowCount(); a++)
        {
            SdrPaintWindow* pCandidate = GetView().GetPaintWindow(a);

            if(pCandidate->OutputToWindow())
            {
                OutputDevice& rOutDev = pCandidate->GetOutputDevice();
                const basegfx::B2DRange aViewRange(rOutDev.GetLogicRange());
                const basegfx::B2DVector aLogicPixel(rOutDev.GetInverseViewTransformation() * basegfx::B2DVector(1.0, 1.0));
                basegfx::B2DRange aRange;

                switch(rHelpLine.GetKind())
                {
                    case SDRHELPLINE_VERTICAL  :
                    {
                        aRange.expand(basegfx::B2DPoint(rHelpLine.GetPos().getX(), aViewRange.getMinY()));
                        aRange.expand(basegfx::B2DPoint(rHelpLine.GetPos().getX(), aViewRange.getMaxY()));
                        break;
                    }
                    case SDRHELPLINE_HORIZONTAL:
                    {
                        aRange.expand(basegfx::B2DPoint(aViewRange.getMinX(), rHelpLine.GetPos().getY()));
                        aRange.expand(basegfx::B2DPoint(aViewRange.getMaxX(), rHelpLine.GetPos().getY()));
                        break;
                    }
                    default: //case SDRHELPLINE_POINT     :
                    {
                        const double fViewFixValue(15.0);
                        const basegfx::B2DVector aGrow(aLogicPixel * fViewFixValue);
                        aRange.expand(rHelpLine.GetPos() - aGrow);
                        aRange.expand(rHelpLine.GetPos() + aGrow);
                        break;
                    }
                }

                aRange.grow((aLogicPixel.getX() + aLogicPixel.getY()) * 0.5);

                ((SdrView&)GetView()).InvalidateOneWin((Window&)rOutDev, aRange);
            }
        }
    }
}

void SdrPageView::SetHelpLines(const SdrHelpLineList& rHLL)
{
    maHelpLines = rHLL;
    InvalidateAllWin();
}

void SdrPageView::SetHelpLine(sal_uInt32 nNum, const SdrHelpLine& rNewHelpLine)
{
    if(nNum < GetHelpLines().GetCount() && GetHelpLines()[nNum] != rNewHelpLine)
    {
        bool bNeedRedraw(true);

        if(GetHelpLines()[nNum].GetKind() == rNewHelpLine.GetKind())
        {
            switch(rNewHelpLine.GetKind())
            {
                case SDRHELPLINE_VERTICAL  :
                {
                    if(basegfx::fTools::equal(GetHelpLines()[nNum].GetPos().getX(), rNewHelpLine.GetPos().getX()))
                    {
                        bNeedRedraw = false;
                    }

                    break;
                }
                case SDRHELPLINE_HORIZONTAL:
                {
                    if(basegfx::fTools::equal(GetHelpLines()[nNum].GetPos().getY(), rNewHelpLine.GetPos().getY()))
                    {
                        bNeedRedraw = false;
                    }

                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        if(bNeedRedraw)
        {
            ImpInvalidateHelpLineArea(nNum);
        }

        maHelpLines[nNum] = rNewHelpLine;

        if(bNeedRedraw)
        {
            ImpInvalidateHelpLineArea(nNum);
        }
    }
}

void SdrPageView::DeleteHelpLine(sal_uInt32 nNum)
{
    if(nNum < maHelpLines.GetCount())
    {
        ImpInvalidateHelpLineArea(nNum);
        maHelpLines.Delete(nNum);
    }
}

void SdrPageView::InsertHelpLine(const SdrHelpLine& rHL)
{
    maHelpLines.Insert(rHL);

    if(GetView().IsHlplVisible() && GetHelpLines().GetCount())
    {
        ImpInvalidateHelpLineArea(GetHelpLines().GetCount() - 1);
    }
}

// Betretene Gruppe und Liste setzen
void SdrPageView::SetCurrentGroupAndObjectList(SdrObject* pNewGroup, SdrObjList* pNewList)
{
    if(mpCurrentGroup != pNewGroup)
    {
        mpCurrentGroup = pNewGroup;
    }

    if(mpCurrentList != pNewList)
    {
        mpCurrentList = pNewList;
    }
}

bool SdrPageView::EnterGroup(SdrObject* pObj)
{
    bool bRet(false);

    if(pObj && pObj->getChildrenOfSdrObject())
    {
        bool bGlueInvalidate(GetView().ImpIsGlueVisible());

        if(bGlueInvalidate)
        {
            GetView().GlueInvalidate();
        }

        // deselect all
        GetView().UnmarkAll();

        // set current group and list
        SdrObjList* pNewObjList = pObj->getChildrenOfSdrObject();
        SetCurrentGroupAndObjectList(pObj, pNewObjList);

        // select contained object if only one object is contained,
        // else select nothing and let the user decide what to do next
        if(pNewObjList && pNewObjList->GetObjCount() == 1)
        {
            SdrObject* pFirstObject = pNewObjList->GetObj(0);

            if(GetView().GetSdrPageView())
            {
                GetView().MarkObj(*pFirstObject);
            }
        }

        // build new handles
        GetView().SetMarkHandles();

        // invalidate only when view wants to visualize group entering
        if(GetView().DoVisualizeEnteredGroup())
        {
            InvalidateAllWin();
        }

        if (bGlueInvalidate)
        {
            GetView().GlueInvalidate();
        }

        bRet = true;
    }

    return bRet;
}

void SdrPageView::LeaveOneGroup()
{
    if(GetCurrentGroup())
    {
        bool bGlueInvalidate(GetView().ImpIsGlueVisible());

        if(bGlueInvalidate)
        {
            GetView().GlueInvalidate();
        }

        SdrObject* pLastGroup = GetCurrentGroup();
        SdrObject* pParentGroup = GetCurrentGroup()->GetParentSdrObject();
        SdrObjList* pParentList = &getSdrPageFromSdrPageView();

        if(pParentGroup)
        {
            pParentList = pParentGroup->getChildrenOfSdrObject();
        }

        // Alles deselektieren
        GetView().UnmarkAll();

        // Zuweisungen, mpCurrentGroup und mpCurrentList muessen gesetzt sein
        SetCurrentGroupAndObjectList(pParentGroup, pParentList);

        // gerade verlassene Gruppe selektieren
        if(pLastGroup)
        {
            if(GetView().GetSdrPageView())
            {
                GetView().MarkObj(*pLastGroup);
            }
        }

        GetView().SetMarkHandles();

        // invalidate only when view wants to visualize group entering
        if(GetView().DoVisualizeEnteredGroup())
        {
            InvalidateAllWin();
        }

        if(bGlueInvalidate)
        {
            GetView().GlueInvalidate();
        }
    }
}

void SdrPageView::LeaveAllGroup()
{
    if(GetCurrentGroup())
    {
        bool bGlueInvalidate(GetView().ImpIsGlueVisible());

        if(bGlueInvalidate)
        {
            GetView().GlueInvalidate();
        }

        SdrObject* pLastGroup = GetCurrentGroup();

        // Alles deselektieren
        GetView().UnmarkAll();

        // Zuweisungen, mpCurrentGroup und mpCurrentList muessen gesetzt sein
        SetCurrentGroupAndObjectList(0, &getSdrPageFromSdrPageView());

        // Oberste letzte Gruppe finden und selektieren
        if(pLastGroup)
        {
            while(pLastGroup->GetParentSdrObject())
            {
                pLastGroup = pLastGroup->GetParentSdrObject();
            }

            if(GetView().GetSdrPageView())
            {
                GetView().MarkObj(*pLastGroup);
            }
        }

        GetView().SetMarkHandles();

        // invalidate only when view wants to visualize group entering
        if(GetView().DoVisualizeEnteredGroup())
        {
            InvalidateAllWin();
        }

        if(bGlueInvalidate)
        {
            GetView().GlueInvalidate();
        }
    }
}

sal_uInt32 SdrPageView::GetEnteredLevel() const
{
    sal_uInt32 nAnz(0);
    SdrObject* pGrp = GetCurrentGroup();

    while(pGrp)
    {
        nAnz++;
        pGrp = pGrp->GetParentSdrObject();
    }

    return nAnz;
}

XubString SdrPageView::GetActualGroupName() const
{
    if(GetCurrentGroup())
    {
        XubString aStr(GetCurrentGroup()->GetName());

        if(!aStr.Len())
        {
            aStr += sal_Unicode('?');
        }

        return aStr;
    }
    else
    {
        return String();
    }
}

XubString SdrPageView::GetActualPathName(sal_Unicode cSep) const
{
    XubString aStr;
    bool bNamFnd(false);
    SdrObject* pGrp = GetCurrentGroup();

    while(pGrp)
    {
        XubString aStr1(pGrp->GetName());

        if(!aStr1.Len())
        {
            aStr1 += sal_Unicode('?');
        }
        else
        {
            bNamFnd = true;
        }

        aStr += aStr1;
        pGrp = pGrp->GetParentSdrObject();

        if(pGrp)
        {
            aStr += cSep;
        }
    }

    if(!bNamFnd && GetCurrentGroup())
    {
        aStr = String();
        aStr += sal_Unicode('(');
        aStr += String::CreateFromInt32( GetEnteredLevel() );
        aStr += sal_Unicode(')');
    }

    return aStr;
}

void SdrPageView::CheckAktGroup()
{
    SdrObject* pGrp = GetCurrentGroup();

    while(pGrp && (!pGrp->IsObjectInserted() || !pGrp->getParentOfSdrObject() || !pGrp->getSdrPageFromSdrObject() ))
    {
        // irgendwas daneben?
        pGrp = pGrp->GetParentSdrObject();
    }

    if(pGrp != GetCurrentGroup())
    {
        if(pGrp)
        {
            EnterGroup(pGrp);
        }
        else
        {
            LeaveAllGroup();
        }
    }
}

// #103834# Set background color for svx at SdrPageViews
void SdrPageView::SetApplicationBackgroundColor(Color aBackgroundColor)
{
    maBackgroundColor = aBackgroundColor;
}

Color SdrPageView::GetApplicationBackgroundColor() const
{
    return maBackgroundColor;
}

// #103911# Set document color for svx at SdrPageViews
void SdrPageView::SetApplicationDocumentColor(Color aDocumentColor)
{
    maDocumentColor = aDocumentColor;
}

Color SdrPageView::GetApplicationDocumentColor() const
{
    return maDocumentColor;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
