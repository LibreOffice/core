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
#include <editeng/outliner.hxx>
#include <svx/svdetc.hxx>
#include <svx/svdobj.hxx>
#include "svx/svditer.hxx"
#include <svx/svdogrp.hxx>
#include <svx/svdtypes.hxx>
#include <svx/svdoole2.hxx>

#include <sdr/contact/objectcontactofpageview.hxx>
#include <svx/sdr/contact/viewobjectcontactredirector.hxx>
#include <svx/fmview.hxx>

#include <algorithm>

#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <comphelper/lok.hxx>

using namespace ::com::sun::star;

// interface to SdrPageWindow

SdrPageWindow* SdrPageView::FindPageWindow(SdrPaintWindow& rPaintWindow) const
{
    for(SdrPageWindowVector::const_iterator a = maPageWindows.begin(); a != maPageWindows.end(); ++a)
    {
        if(&((*a)->GetPaintWindow()) == &rPaintWindow)
        {
            return *a;
        }
    }

    return nullptr;
}

const SdrPageWindow* SdrPageView::FindPatchedPageWindow( const OutputDevice& _rOutDev ) const
{
    for (   SdrPageWindowVector::const_iterator loop = maPageWindows.begin();
            loop != maPageWindows.end();
            ++loop
        )
    {
        const SdrPageWindow& rPageWindow( *(*loop) );
        const SdrPaintWindow& rPaintWindow( rPageWindow.GetOriginalPaintWindow() ? *rPageWindow.GetOriginalPaintWindow() : rPageWindow.GetPaintWindow() );
        if ( &rPaintWindow.GetOutputDevice() == &_rOutDev )
        {
            return &rPageWindow;
        }
    }

    return nullptr;
}

SdrPageWindow* SdrPageView::FindPageWindow(const OutputDevice& rOutDev) const
{
    for(SdrPageWindowVector::const_iterator a = maPageWindows.begin(); a != maPageWindows.end(); ++a)
    {
        if(&((*a)->GetPaintWindow().GetOutputDevice()) == &rOutDev)
        {
            return *a;
        }
    }

    return nullptr;
}

SdrPageWindow* SdrPageView::GetPageWindow(sal_uInt32 nIndex) const
{
    if(nIndex < maPageWindows.size())
    {
        return maPageWindows[nIndex];
    }

    return nullptr;
}

void SdrPageView::ClearPageWindows()
{
    for(SdrPageWindowVector::const_iterator a = maPageWindows.begin(); a != maPageWindows.end(); ++a)
    {
        delete *a;
    }

    maPageWindows.clear();
}

void SdrPageView::AppendPageWindow(SdrPageWindow& rNew)
{
    maPageWindows.push_back(&rNew);
}

SdrPageWindow* SdrPageView::RemovePageWindow(SdrPageWindow& rOld)
{
    const SdrPageWindowVector::iterator aFindResult = ::std::find(maPageWindows.begin(), maPageWindows.end(), &rOld);

    if(aFindResult != maPageWindows.end())
    {
        // remember return value
        SdrPageWindow* pSdrPageWindow = *aFindResult;
        maPageWindows.erase(aFindResult);
        return pSdrPageWindow;
    }

    return nullptr;
}


SdrPageView::SdrPageView(SdrPage* pPage1, SdrView& rNewView)
:   mrView(rNewView),
    // col_auto color lets the view takes the default SvxColorConfig entry
    maDocumentColor( COL_AUTO ),
    maBackgroundColor(COL_AUTO ), // #i48367# also react on autocolor
    mpPreparedPageWindow(nullptr) // #i72752#
{
    mpPage = pPage1;

    if(mpPage)
    {
        aPgOrg.X()=mpPage->GetLftBorder();
        aPgOrg.Y()=mpPage->GetUppBorder();
    }
    mbHasMarked = false;
    aLayerVisi.SetAll();
    aLayerPrn.SetAll();

    mbVisible = false;
    pAktList = nullptr;
    pAktGroup = nullptr;
    SetAktGroupAndList(nullptr, mpPage);

    for(sal_uInt32 a(0L); a < rNewView.PaintWindowCount(); a++)
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

css::uno::Reference< css::awt::XControlContainer > SdrPageView::GetControlContainer( const OutputDevice& _rDevice ) const
{
    css::uno::Reference< css::awt::XControlContainer > xReturn;
    const SdrPageWindow* pCandidate = FindPatchedPageWindow( _rDevice );

    if ( pCandidate )
        xReturn = pCandidate->GetControlContainer();

    return xReturn;
}

void SdrPageView::ModelHasChanged()
{
    if (GetAktGroup()!=nullptr) CheckAktGroup();
}

bool SdrPageView::IsReadOnly() const
{
    return (nullptr == GetPage() || GetView().GetModel()->IsReadOnly() || GetPage()->IsReadOnly() || GetObjList()->IsReadOnly());
}

void SdrPageView::Show()
{
    if(!IsVisible())
    {
        mbVisible = true;

        for(sal_uInt32 a(0L); a < GetView().PaintWindowCount(); a++)
        {
            AddPaintWindowToPageView(*GetView().GetPaintWindow(a));
        }
    }
}

void SdrPageView::Hide()
{
    if(IsVisible())
    {
        if (!comphelper::LibreOfficeKit::isActive())
        {
            InvalidateAllWin();
        }
        mbVisible = false;
        ClearPageWindows();
    }
}

Rectangle SdrPageView::GetPageRect() const
{
    if (GetPage()==nullptr) return Rectangle();
    return Rectangle(Point(),Size(GetPage()->GetWdt()+1,GetPage()->GetHgt()+1));
}

void SdrPageView::InvalidateAllWin()
{
    if(IsVisible() && GetPage())
    {
        Rectangle aRect(Point(0,0),Size(GetPage()->GetWdt()+1,GetPage()->GetHgt()+1));
        aRect.Union(GetPage()->GetAllObjBoundRect());
        GetView().InvalidateAllWin(aRect);
    }
}


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

void SdrPageView::CompleteRedraw(
    SdrPaintWindow& rPaintWindow, const vcl::Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector )
{
    if(GetPage())
    {
        SdrPageWindow* pPageWindow = FindPageWindow(rPaintWindow);
        bool bIsTempTarget(false);

        if(!pPageWindow)
        {
            // create temp PageWindow
            pPageWindow = new SdrPageWindow(*this, rPaintWindow);
            bIsTempTarget = true;
        }

        // do the redraw
        pPageWindow->PrepareRedraw(rReg);
        pPageWindow->RedrawAll(pRedirector);

        // get rid of temp PageWindow
        if(bIsTempTarget)
        {
            delete pPageWindow;
            pPageWindow = nullptr;
        }
    }
}


// #i74769# use SdrPaintWindow directly

void SdrPageView::setPreparedPageWindow(SdrPageWindow* pKnownTarget)
{
    // #i72752# remember prepared SdrPageWindow
    mpPreparedPageWindow = pKnownTarget;
}

void SdrPageView::DrawLayer( SdrLayerID nID, OutputDevice* pGivenTarget, sdr::contact::ViewObjectContactRedirector* pRedirector, const Rectangle& rRect )
{
    if(GetPage())
    {
        if(pGivenTarget)
        {
            SdrPageWindow* pKnownTarget = FindPageWindow(*pGivenTarget);

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
                    const vcl::Region& rExistingRegion = rExistingPaintWindow.GetRedrawRegion();
                    bool bUseRect(false);
                    if (!rRect.IsEmpty())
                    {
                        vcl::Region r(rExistingRegion);
                        r.Intersect(rRect);
                        // fdo#74435: FIXME: visibility check broken if empty
                        if (!r.IsEmpty())
                            bUseRect = true;
                    }
                    if (!bUseRect)
                        aTemporaryPaintWindow.SetRedrawRegion(rExistingRegion);
                    else
                        aTemporaryPaintWindow.SetRedrawRegion(vcl::Region(rRect));

                    // patch the ExistingPageWindow
                    pPreparedTarget->patchPaintWindow(aTemporaryPaintWindow);

                    // redraw the layer
                    pPreparedTarget->RedrawLayer(&nID, pRedirector);

                    // restore the ExistingPageWindow
                    pPreparedTarget->unpatchPaintWindow();
                }
                else
                {
                    OSL_FAIL("SdrPageView::DrawLayer: Creating temporary SdrPageWindow (ObjectContact), this should never be needed (!)");

                    // None of the known OutputDevices is the target of this paint, use
                    // a temporary SdrPageWindow for this Redraw.
                    SdrPaintWindow aTemporaryPaintWindow(mrView, *pGivenTarget);
                    SdrPageWindow aTemporaryPageWindow(*this, aTemporaryPaintWindow);

                    // #i72752#
                    // Copy existing paint region if other PageWindows exist, this was created by
                    // PrepareRedraw() from BeginDrawLayer(). Needs to be used e.g. when suddenly SW
                    // paints into an unknown device other than the view was created for (e.g. VirtualDevice)
                    if(PageWindowCount())
                    {
                        SdrPageWindow* pExistingPageWindow = GetPageWindow(0L);
                        SdrPaintWindow& rExistingPaintWindow = pExistingPageWindow->GetPaintWindow();
                        const vcl::Region& rExistingRegion = rExistingPaintWindow.GetRedrawRegion();
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
}

void SdrPageView::SetDesignMode( bool _bDesignMode ) const
{
    for ( sal_uInt32 i = 0L; i < PageWindowCount(); ++i )
    {
        const SdrPageWindow& rPageViewWindow = *GetPageWindow(i);
        rPageViewWindow.SetDesignMode( _bDesignMode );
    }
}


void SdrPageView::DrawPageViewGrid(OutputDevice& rOut, const Rectangle& rRect, Color aColor)
{
    if (GetPage()==nullptr)
        return;

    long nx1=GetView().maGridBig.Width();
    long nx2=GetView().maGridFin.Width();
    long ny1=GetView().maGridBig.Height();
    long ny2=GetView().maGridFin.Height();

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
        { // e. g. 640x480
            nMinDotPix=2;
            nMinLinPix=4;
        }
        Size aMinDotDist(rOut.PixelToLogic(Size(nMinDotPix,nMinDotPix)));
        Size aMinLinDist(rOut.PixelToLogic(Size(nMinLinPix,nMinLinPix)));
        bool bHoriSolid=nx2<aMinDotDist.Width();
        bool bVertSolid=ny2<aMinDotDist.Height();
        // enlarge line offset (minimum 4 pixels)
        // enlarge by: *2 *5 *10 *20 *50 *100 ...
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

        bool bHoriFine=nx2<nx1;
        bool bVertFine=ny2<ny1;
        bool bHoriLines=bHoriSolid || bHoriFine || !bVertFine;
        bool bVertLines=bVertSolid || bVertFine;

        Color aColorMerk( rOut.GetLineColor() );
        rOut.SetLineColor( aColor );

        bool bMap0=rOut.IsMapModeEnabled();

        long nWrX=0;
        long nWrY=0;
        Point aOrg(aPgOrg);
        long x1=GetPage()->GetLftBorder()+1+nWrX;
        long x2=GetPage()->GetWdt()-GetPage()->GetRgtBorder()-1+nWrY;
        long y1=GetPage()->GetUppBorder()+1+nWrX;
        long y2=GetPage()->GetHgt()-GetPage()->GetLwrBorder()-1+nWrY;
        const SdrPageGridFrameList* pFrames=GetPage()->GetGridFrameList(this,nullptr);

        sal_uInt16 nGridPaintAnz=1;
        if (pFrames!=nullptr) nGridPaintAnz=pFrames->GetCount();
        for (sal_uInt16 nGridPaintNum=0; nGridPaintNum<nGridPaintAnz; nGridPaintNum++) {
            if (pFrames!=nullptr) {
                const SdrPageGridFrame& rGF=(*pFrames)[nGridPaintNum];
                nWrX=rGF.GetPaperRect().Left();
                nWrY=rGF.GetPaperRect().Top();
                x1=rGF.GetUserArea().Left();
                x2=rGF.GetUserArea().Right();
                y1=rGF.GetUserArea().Top();
                y2=rGF.GetUserArea().Bottom();
                aOrg=rGF.GetUserArea().TopLeft();
                aOrg-=rGF.GetPaperRect().TopLeft();
            }
            if (!rRect.IsEmpty()) {
                Size a1PixSiz(rOut.PixelToLogic(Size(1,1)));
                long nX1Pix=a1PixSiz.Width();  // add 1 pixel of tolerance
                long nY1Pix=a1PixSiz.Height();
                if (x1<rRect.Left()  -nX1Pix) x1=rRect.Left()  -nX1Pix;
                if (x2>rRect.Right() +nX1Pix) x2=rRect.Right() +nX1Pix;
                if (y1<rRect.Top()   -nY1Pix) y1=rRect.Top()   -nY1Pix;
                if (y2>rRect.Bottom()+nY1Pix) y2=rRect.Bottom()+nY1Pix;
            }

            long xBigOrg=aOrg.X()+nWrX;
            while (xBigOrg>=x1) xBigOrg-=nx1;
            while (xBigOrg<x1) xBigOrg+=nx1;
            long xFinOrg=xBigOrg;
            while (xFinOrg>=x1) xFinOrg-=nx2;
            while (xFinOrg<x1) xFinOrg+=nx2;

            long yBigOrg=aOrg.Y()+nWrY;
            while (yBigOrg>=y1) yBigOrg-=ny1;
            while (yBigOrg<y1) yBigOrg+=ny1;
            long yFinOrg=yBigOrg;
            while (yFinOrg>=y1) yFinOrg-=ny2;
            while (yFinOrg<y1) yFinOrg+=ny2;

            if( x1 <= x2 && y1 <= y2 )
            {
                if( bHoriLines )
                {
                    DrawGridFlags nGridFlags = ( bHoriSolid ? DrawGridFlags::HorzLines : DrawGridFlags::Dots );
                    sal_uInt16 nSteps = sal_uInt16(nx1 / nx2);
                    sal_uInt32 nRestPerStepMul1000 = nSteps ? ( ((nx1 * 1000L)/ nSteps) - (nx2 * 1000L) ) : 0;
                    sal_uInt32 nStepOffset = 0;
                    sal_uInt16 nPointOffset = 0;

                    for(sal_uInt16 a=0;a<nSteps;a++)
                    {
                        // draw
                        rOut.DrawGrid(
                            Rectangle( xFinOrg + (a * nx2) + nPointOffset, yBigOrg, x2, y2 ),
                            Size( nx1, ny1 ), nGridFlags );

                        // do a step
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
                    DrawGridFlags nGridFlags = ( bVertSolid ? DrawGridFlags::VertLines : DrawGridFlags::Dots );
                    sal_uInt16 nSteps = sal_uInt16(ny1 / ny2);
                    sal_uInt32 nRestPerStepMul1000 = nSteps ? ( ((ny1 * 1000L)/ nSteps) - (ny2 * 1000L) ) : 0;
                    sal_uInt32 nStepOffset = 0;
                    sal_uInt16 nPointOffset = 0;

                    for(sal_uInt16 a=0;a<nSteps;a++)
                    {
                        // draw
                        rOut.DrawGrid(
                            Rectangle( xBigOrg, yFinOrg + (a * ny2) + nPointOffset, x2, y2 ),
                            Size( nx1, ny1 ), nGridFlags );

                        // do a step
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
    GetView().AdjustMarkHdl();
}

void SdrPageView::SetLayer(const OUString& rName, SetOfByte& rBS, bool bJa)
{
    if(!GetPage())
        return;

    SdrLayerID nID = GetPage()->GetLayerAdmin().GetLayerID(rName, true);

    if(SDRLAYER_NOTFOUND != nID)
        rBS.Set(nID, bJa);
}

bool SdrPageView::IsLayer(const OUString& rName, const SetOfByte& rBS) const
{
    if(!GetPage())
        return false;

    bool bRet(false);

    if (!rName.isEmpty())
    {
        SdrLayerID nId = GetPage()->GetLayerAdmin().GetLayerID(rName, true);

        if(SDRLAYER_NOTFOUND != nId)
        {
            bRet = rBS.IsSet(nId);
        }
    }

    return bRet;
}

bool SdrPageView::IsObjMarkable(SdrObject* pObj) const
{
    if(pObj)
    {
        // excluded from selection?
        if(pObj->IsMarkProtect())
        {
            return false;
        }

        // only visible are selectable
        if( !pObj->IsVisible() )
        {
            return false;
        }

        if(dynamic_cast<const SdrObjGroup*>( pObj) !=  nullptr)
        {
            // If object is a Group object, visibility may depend on
            // multiple layers. If one object is markable, Group is markable.
            SdrObjList* pObjList = static_cast<SdrObjGroup*>(pObj)->GetSubList();

            if(pObjList && pObjList->GetObjCount())
            {
                bool bGroupIsMarkable(false);

                for(size_t a = 0; !bGroupIsMarkable && a < pObjList->GetObjCount(); ++a)
                {
                    SdrObject* pCandidate = pObjList->GetObj(a);

                    // call recursively
                    if(IsObjMarkable(pCandidate))
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
            // the layer has to be visible and must not be locked
            SdrLayerID nL = pObj->GetLayer();
            return (aLayerVisi.IsSet(sal_uInt8(nL)) && !aLayerLock.IsSet(sal_uInt8(nL)));
        }
    }

    return false;
}

void SdrPageView::SetPageOrigin(const Point& rOrg)
{
    if (rOrg!=aPgOrg) {
        aPgOrg=rOrg;
        if (GetView().IsGridVisible()) {
            InvalidateAllWin();
        }
    }
}

void SdrPageView::ImpInvalidateHelpLineArea(sal_uInt16 nNum) const
{
    if (GetView().IsHlplVisible() && nNum<aHelpLines.GetCount()) {
        const SdrHelpLine& rHL=aHelpLines[nNum];

        for(sal_uInt32 a(0L); a < GetView().PaintWindowCount(); a++)
        {
            SdrPaintWindow* pCandidate = GetView().GetPaintWindow(a);

            if(pCandidate->OutputToWindow())
            {
                OutputDevice& rOutDev = pCandidate->GetOutputDevice();
                Rectangle aR(rHL.GetBoundRect(rOutDev));
                Size aSiz(rOutDev.PixelToLogic(Size(1,1)));
                aR.Left() -= aSiz.Width();
                aR.Right() += aSiz.Width();
                aR.Top() -= aSiz.Height();
                aR.Bottom() += aSiz.Height();
                ((SdrView&)GetView()).InvalidateOneWin(static_cast<vcl::Window&>(rOutDev), aR);
            }
        }
    }
}

void SdrPageView::SetHelpLines(const SdrHelpLineList& rHLL)
{
    aHelpLines=rHLL;
    InvalidateAllWin();
}

void SdrPageView::SetHelpLine(sal_uInt16 nNum, const SdrHelpLine& rNewHelpLine)
{
    if (nNum<aHelpLines.GetCount() && aHelpLines[nNum]!=rNewHelpLine) {
        bool bNeedRedraw = true;
        if (aHelpLines[nNum].GetKind()==rNewHelpLine.GetKind()) {
            switch (rNewHelpLine.GetKind()) {
                case SDRHELPLINE_VERTICAL  : if (aHelpLines[nNum].GetPos().X()==rNewHelpLine.GetPos().X()) bNeedRedraw = false; break;
                case SDRHELPLINE_HORIZONTAL: if (aHelpLines[nNum].GetPos().Y()==rNewHelpLine.GetPos().Y()) bNeedRedraw = false; break;
                default: break;
            } // switch
        }
        if (bNeedRedraw) ImpInvalidateHelpLineArea(nNum);
        aHelpLines[nNum]=rNewHelpLine;
        if (bNeedRedraw) ImpInvalidateHelpLineArea(nNum);
    }
}

void SdrPageView::DeleteHelpLine(sal_uInt16 nNum)
{
    if (nNum<aHelpLines.GetCount()) {
        ImpInvalidateHelpLineArea(nNum);
        aHelpLines.Delete(nNum);
    }
}

void SdrPageView::InsertHelpLine(const SdrHelpLine& rHL, sal_uInt16 nNum)
{
    if (nNum > aHelpLines.GetCount())
        nNum = aHelpLines.GetCount();
    aHelpLines.Insert(rHL,nNum);
    if (GetView().IsHlplVisible())
        ImpInvalidateHelpLineArea(nNum);
}

// set current group and list
void SdrPageView::SetAktGroupAndList(SdrObject* pNewGroup, SdrObjList* pNewList)
{
    if(pAktGroup != pNewGroup)
    {
        pAktGroup = pNewGroup;
    }
    if(pAktList != pNewList)
    {
        pAktList = pNewList;
    }
}

bool SdrPageView::EnterGroup(SdrObject* pObj)
{
    bool bRet(false);

    if(pObj && pObj->IsGroupObject())
    {
        bool bGlueInvalidate(GetView().ImpIsGlueVisible());

        if(bGlueInvalidate)
        {
            GetView().GlueInvalidate();
        }

        // deselect all
        GetView().UnmarkAll();

        // set current group and list
        SdrObjList* pNewObjList = pObj->GetSubList();
        SetAktGroupAndList(pObj, pNewObjList);

        // select contained object if only one object is contained,
        // else select nothing and let the user decide what to do next
        if(pNewObjList && pNewObjList->GetObjCount() == 1)
        {
            SdrObject* pFirstObject = pNewObjList->GetObj(0);

            if(GetView().GetSdrPageView())
            {
                GetView().MarkObj(pFirstObject, GetView().GetSdrPageView());
            }
        }

        // build new handles
        GetView().AdjustMarkHdl();

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
    if(GetAktGroup())
    {
        bool bGlueInvalidate = (GetView().ImpIsGlueVisible());

        if(bGlueInvalidate)
            GetView().GlueInvalidate();

        SdrObject* pLastGroup = GetAktGroup();
        SdrObject* pParentGroup = GetAktGroup()->GetUpGroup();
        SdrObjList* pParentList = GetPage();

        if(pParentGroup)
            pParentList = pParentGroup->GetSubList();

        // deselect everything
        GetView().UnmarkAll();

        // allocations, pAktGroup and pAktList need to be set
        SetAktGroupAndList(pParentGroup, pParentList);

        // select the group we just left
        if(pLastGroup)
            if(GetView().GetSdrPageView())
                GetView().MarkObj(pLastGroup, GetView().GetSdrPageView());

        GetView().AdjustMarkHdl();

        // invalidate only if view wants to visualize group entering
        if(GetView().DoVisualizeEnteredGroup())
            InvalidateAllWin();

        if(bGlueInvalidate)
            GetView().GlueInvalidate();
    }
}

void SdrPageView::LeaveAllGroup()
{
    if(GetAktGroup())
    {
        bool bGlueInvalidate = (GetView().ImpIsGlueVisible());

        if(bGlueInvalidate)
            GetView().GlueInvalidate();

        SdrObject* pLastGroup = GetAktGroup();

        // deselect everything
        GetView().UnmarkAll();

        // allocations, pAktGroup and pAktList always need to be set
        SetAktGroupAndList(nullptr, GetPage());

        // find and select uppermost group
        if(pLastGroup)
        {
            while(pLastGroup->GetUpGroup())
                pLastGroup = pLastGroup->GetUpGroup();

            if(GetView().GetSdrPageView())
                GetView().MarkObj(pLastGroup, GetView().GetSdrPageView());
        }

        GetView().AdjustMarkHdl();

        // invalidate only when view wants to visualize group entering
        if(GetView().DoVisualizeEnteredGroup())
            InvalidateAllWin();

        if(bGlueInvalidate)
            GetView().GlueInvalidate();
    }
}

sal_uInt16 SdrPageView::GetEnteredLevel() const
{
    sal_uInt16 nCount=0;
    SdrObject* pGrp=GetAktGroup();
    while (pGrp!=nullptr) {
        nCount++;
        pGrp=pGrp->GetUpGroup();
    }
    return nCount;
}

void SdrPageView::CheckAktGroup()
{
    SdrObject* pGrp=GetAktGroup();
    while (pGrp!=nullptr &&
           (!pGrp->IsInserted() || pGrp->GetObjList()==nullptr ||
            pGrp->GetPage()==nullptr || pGrp->GetModel()==nullptr)) { // anything outside of the borders?
        pGrp=pGrp->GetUpGroup();
    }
    if (pGrp!=GetAktGroup()) {
        if (pGrp!=nullptr) EnterGroup(pGrp);
        else LeaveAllGroup();
    }
}

// Set background color for svx at SdrPageViews
void SdrPageView::SetApplicationBackgroundColor(Color aBackgroundColor)
{
    maBackgroundColor = aBackgroundColor;
}


// Set document color for svx at SdrPageViews
void SdrPageView::SetApplicationDocumentColor(Color aDocumentColor)
{
    maDocumentColor = aDocumentColor;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
