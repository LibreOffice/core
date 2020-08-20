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
#include <svx/svdpage.hxx>
#include <svx/svdview.hxx>

#include <svx/svdobj.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdtypes.hxx>

#include <svx/sdr/contact/viewobjectcontactredirector.hxx>

#include <algorithm>

#include <svx/sdrpagewindow.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/scopeguard.hxx>
#include <basegfx/range/b2irectangle.hxx>

using namespace ::com::sun::star;

// interface to SdrPageWindow

SdrPageWindow* SdrPageView::FindPageWindow(SdrPaintWindow& rPaintWindow) const
{
    for(auto & a : maPageWindows)
    {
        if(&(a->GetPaintWindow()) == &rPaintWindow)
        {
            return a.get();
        }
    }

    return nullptr;
}

const SdrPageWindow* SdrPageView::FindPatchedPageWindow( const OutputDevice& _rOutDev ) const
{
    for ( auto const & pPageWindow : maPageWindows )
    {
        const SdrPaintWindow& rPaintWindow( pPageWindow->GetOriginalPaintWindow() ? *pPageWindow->GetOriginalPaintWindow() : pPageWindow->GetPaintWindow() );
        if ( &rPaintWindow.GetOutputDevice() == &_rOutDev )
        {
            return pPageWindow.get();
        }
    }

    return nullptr;
}

SdrPageWindow* SdrPageView::FindPageWindow(const OutputDevice& rOutDev) const
{
    for ( auto const & pPageWindow : maPageWindows )
    {
        if(&(pPageWindow->GetPaintWindow().GetOutputDevice()) == &rOutDev)
        {
            return pPageWindow.get();
        }
    }

    return nullptr;
}

SdrPageWindow* SdrPageView::GetPageWindow(sal_uInt32 nIndex) const
{
    return maPageWindows[nIndex].get();
}

SdrPageView::SdrPageView(SdrPage* pPage1, SdrView& rNewView)
:   mrView(rNewView),
    // col_auto color lets the view takes the default SvxColorConfig entry
    maDocumentColor( COL_AUTO ),
    maBackgroundColor( COL_AUTO ), // #i48367# also react on autocolor
    mpPreparedPageWindow(nullptr) // #i72752#
{
    mpPage = pPage1;

    if(mpPage)
    {
        aPgOrg.setX(mpPage->GetLeftBorder() );
        aPgOrg.setY(mpPage->GetUpperBorder() );
    }
    // For example, in the case of charts, there is a LayerAdmin, but it has no valid values. Therefore
    // a solution like pLayerAdmin->getVisibleLayersODF(aLayerVisi) is not possible. So use the
    // generic SetAll() for now.
    aLayerVisi.SetAll();
    aLayerPrn.SetAll();

    mbHasMarked = false;
    mbVisible = false;
    pCurrentList = nullptr;
    pCurrentGroup = nullptr;
    SetCurrentGroupAndList(nullptr, mpPage);

    for(sal_uInt32 a(0); a < rNewView.PaintWindowCount(); a++)
    {
        AddPaintWindowToPageView(*rNewView.GetPaintWindow(a));
    }
}

SdrPageView::~SdrPageView()
{
}

void SdrPageView::AddPaintWindowToPageView(SdrPaintWindow& rPaintWindow)
{
    if(!FindPageWindow(rPaintWindow))
    {
        maPageWindows.emplace_back(new SdrPageWindow(*this, rPaintWindow));
    }
}

void SdrPageView::RemovePaintWindowFromPageView(SdrPaintWindow& rPaintWindow)
{
    auto it = std::find_if(maPageWindows.begin(), maPageWindows.end(),
        [&rPaintWindow](const std::unique_ptr<SdrPageWindow>& rpWindow) {
            return &(rpWindow->GetPaintWindow()) == &rPaintWindow;
        });
    if (it != maPageWindows.end())
        maPageWindows.erase(it);
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
    if (GetCurrentGroup()!=nullptr) CheckCurrentGroup();
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
        if (!comphelper::LibreOfficeKit::isActive())
        {
            InvalidateAllWin();
        }
        mbVisible = false;
        maPageWindows.clear();
    }
}

tools::Rectangle SdrPageView::GetPageRect() const
{
    if (GetPage()==nullptr) return tools::Rectangle();
    return tools::Rectangle(Point(),Size(GetPage()->GetWidth()+1,GetPage()->GetHeight()+1));
}

void SdrPageView::InvalidateAllWin()
{
    if(IsVisible() && GetPage())
    {
        tools::Rectangle aRect(Point(0,0),Size(GetPage()->GetWidth()+1,GetPage()->GetHeight()+1));
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
    if(!GetPage())
        return;

    SdrPageWindow* pPageWindow = FindPageWindow(rPaintWindow);
    std::unique_ptr<SdrPageWindow> pTempPageWindow;

    if(!pPageWindow)
    {
        // create temp PageWindow
        pTempPageWindow.reset(new SdrPageWindow(*this, rPaintWindow));
        pPageWindow = pTempPageWindow.get();
    }

    // do the redraw
    pPageWindow->PrepareRedraw(rReg);
    pPageWindow->RedrawAll(pRedirector);
}


// #i74769# use SdrPaintWindow directly

void SdrPageView::setPreparedPageWindow(SdrPageWindow* pKnownTarget)
{
    // #i72752# remember prepared SdrPageWindow
    mpPreparedPageWindow = pKnownTarget;
}

void SdrPageView::DrawLayer(SdrLayerID nID, OutputDevice* pGivenTarget,
        sdr::contact::ViewObjectContactRedirector* pRedirector,
        const tools::Rectangle& rRect, basegfx::B2IRectangle const*const pPageFrame)
{
    if(!GetPage())
        return;

    if(pGivenTarget)
    {
        SdrPageWindow* pKnownTarget = FindPageWindow(*pGivenTarget);

        if(pKnownTarget)
        {
            // paint known target
            pKnownTarget->RedrawLayer(&nID, pRedirector, nullptr);
        }
        else
        {
            // #i72752# DrawLayer() uses an OutputDevice different from BeginDrawLayer. This happens
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
                auto pPreviousWindow = pPreparedTarget->patchPaintWindow(aTemporaryPaintWindow);
                // unpatch window when leaving the scope
                const ::comphelper::ScopeGuard aGuard(
                    [&pPreviousWindow, &pPreparedTarget]() { pPreparedTarget->unpatchPaintWindow(pPreviousWindow); } );
                // redraw the layer
                pPreparedTarget->RedrawLayer(&nID, pRedirector, pPageFrame);
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
                    SdrPageWindow* pExistingPageWindow = GetPageWindow(0);
                    SdrPaintWindow& rExistingPaintWindow = pExistingPageWindow->GetPaintWindow();
                    const vcl::Region& rExistingRegion = rExistingPaintWindow.GetRedrawRegion();
                    aTemporaryPaintWindow.SetRedrawRegion(rExistingRegion);
                }

                aTemporaryPageWindow.RedrawLayer(&nID, pRedirector, nullptr);
            }
        }
    }
    else
    {
        // paint in all known windows
        for(sal_uInt32 a(0); a < PageWindowCount(); a++)
        {
            SdrPageWindow* pTarget = GetPageWindow(a);
            pTarget->RedrawLayer(&nID, pRedirector, nullptr);
        }
    }
}

void SdrPageView::SetDesignMode( bool _bDesignMode ) const
{
    for ( sal_uInt32 i = 0; i < PageWindowCount(); ++i )
    {
        const SdrPageWindow& rPageViewWindow = *GetPageWindow(i);
        rPageViewWindow.SetDesignMode( _bDesignMode );
    }
}


void SdrPageView::DrawPageViewGrid(OutputDevice& rOut, const tools::Rectangle& rRect, Color aColor)
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

    if (nx1==0)
        return;

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

    Color aOriginalLineColor( rOut.GetLineColor() );
    rOut.SetLineColor( aColor );

    bool bMap0=rOut.IsMapModeEnabled();

    long nWrX=0;
    long nWrY=0;
    Point aOrg(aPgOrg);
    long x1=GetPage()->GetLeftBorder()+1+nWrX;
    long x2=GetPage()->GetWidth()-GetPage()->GetRightBorder()-1+nWrY;
    long y1=GetPage()->GetUpperBorder()+1+nWrX;
    long y2=GetPage()->GetHeight()-GetPage()->GetLowerBorder()-1+nWrY;
    const SdrPageGridFrameList* pFrames=GetPage()->GetGridFrameList(this,nullptr);

    sal_uInt16 nGridPaintCnt=1;
    if (pFrames!=nullptr) nGridPaintCnt=pFrames->GetCount();
    for (sal_uInt16 nGridPaintNum=0; nGridPaintNum<nGridPaintCnt; nGridPaintNum++) {
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
                sal_uInt32 nRestPerStepMul1000 = nSteps ? ( ((nx1 * 1000)/ nSteps) - (nx2 * 1000) ) : 0;
                sal_uInt32 nStepOffset = 0;
                sal_uInt16 nPointOffset = 0;

                for(sal_uInt16 a=0;a<nSteps;a++)
                {
                    // draw
                    rOut.DrawGrid(
                        tools::Rectangle( xFinOrg + (a * nx2) + nPointOffset, yBigOrg, x2, y2 ),
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
                        tools::Rectangle( xBigOrg, yFinOrg + (a * ny2) + nPointOffset, x2, y2 ),
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
    rOut.SetLineColor(aOriginalLineColor);
}

void SdrPageView::AdjHdl()
{
    GetView().AdjustMarkHdl();
}

void SdrPageView::SetLayer(const OUString& rName, SdrLayerIDSet& rBS, bool bJa)
{
    if(!GetPage())
        return;

    SdrLayerID nID = GetPage()->GetLayerAdmin().GetLayerID(rName);

    if(SDRLAYER_NOTFOUND != nID)
        rBS.Set(nID, bJa);
}

bool SdrPageView::IsLayer(const OUString& rName, const SdrLayerIDSet& rBS) const
{
    if(!GetPage())
        return false;

    bool bRet(false);

    if (!rName.isEmpty())
    {
        SdrLayerID nId = GetPage()->GetLayerAdmin().GetLayerID(rName);

        if(SDRLAYER_NOTFOUND != nId)
        {
            bRet = rBS.IsSet(nId);
        }
    }

    return bRet;
}

bool SdrPageView::IsObjMarkable(SdrObject const * pObj) const
{
    if (!pObj)
        return false;
    if (pObj->IsMarkProtect())
        return false;    // excluded from selection?
    if (!pObj->IsVisible())
        return false;    // only visible are selectable
    if (!pObj->IsInserted())
        return false;    // Obj deleted?
    if (auto pObjGroup = dynamic_cast<const SdrObjGroup*>(pObj))
    {
        // If object is a Group object, visibility may depend on
        // multiple layers. If one object is markable, Group is markable.
        SdrObjList* pObjList = pObjGroup->GetSubList();

        if (pObjList && pObjList->GetObjCount())
        {
            for (size_t a = 0; a < pObjList->GetObjCount(); ++a)
            {
                SdrObject* pCandidate = pObjList->GetObj(a);
                // call recursively
                if (IsObjMarkable(pCandidate))
                    return true;
            }
            return false;
        }
        else
        {
            // #i43302#
            // Allow empty groups to be selected to be able to delete them
            return true;
        }
    }
    if (!pObj->Is3DObj() && pObj->getSdrPageFromSdrObject() != GetPage())
    {
        // Obj suddenly in different Page
        return false;
    }

    // the layer has to be visible and must not be locked
    SdrLayerID nL = pObj->GetLayer();
    if (!aLayerVisi.IsSet(nL))
        return false;
    if (aLayerLock.IsSet(nL))
        return false;
    return true;
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
    if (!(GetView().IsHlplVisible() && nNum<aHelpLines.GetCount()))        return;

    const SdrHelpLine& rHL=aHelpLines[nNum];

    for(sal_uInt32 a(0); a < GetView().PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = GetView().GetPaintWindow(a);

        if(pCandidate->OutputToWindow())
        {
            OutputDevice& rOutDev = pCandidate->GetOutputDevice();
            tools::Rectangle aR(rHL.GetBoundRect(rOutDev));
            Size aSiz(rOutDev.PixelToLogic(Size(1,1)));
            aR.AdjustLeft( -(aSiz.Width()) );
            aR.AdjustRight(aSiz.Width() );
            aR.AdjustTop( -(aSiz.Height()) );
            aR.AdjustBottom(aSiz.Height() );
            const_cast<SdrView&>(GetView()).InvalidateOneWin(rOutDev, aR);
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
    if (nNum >= aHelpLines.GetCount() || aHelpLines[nNum] == rNewHelpLine)
        return;

    bool bNeedRedraw = true;
    if (aHelpLines[nNum].GetKind()==rNewHelpLine.GetKind()) {
        switch (rNewHelpLine.GetKind()) {
            case SdrHelpLineKind::Vertical  : if (aHelpLines[nNum].GetPos().X()==rNewHelpLine.GetPos().X()) bNeedRedraw = false; break;
            case SdrHelpLineKind::Horizontal: if (aHelpLines[nNum].GetPos().Y()==rNewHelpLine.GetPos().Y()) bNeedRedraw = false; break;
            default: break;
        } // switch
    }
    if (bNeedRedraw) ImpInvalidateHelpLineArea(nNum);
    aHelpLines[nNum]=rNewHelpLine;
    if (bNeedRedraw) ImpInvalidateHelpLineArea(nNum);
}

void SdrPageView::DeleteHelpLine(sal_uInt16 nNum)
{
    if (nNum<aHelpLines.GetCount()) {
        ImpInvalidateHelpLineArea(nNum);
        aHelpLines.Delete(nNum);
    }
}

void SdrPageView::InsertHelpLine(const SdrHelpLine& rHL)
{
    sal_uInt16 nNum = aHelpLines.GetCount();
    aHelpLines.Insert(rHL,nNum);
    if (GetView().IsHlplVisible())
        ImpInvalidateHelpLineArea(nNum);
}

// set current group and list
void SdrPageView::SetCurrentGroupAndList(SdrObject* pNewGroup, SdrObjList* pNewList)
{
    if(pCurrentGroup != pNewGroup)
    {
        pCurrentGroup = pNewGroup;
    }
    if(pCurrentList != pNewList)
    {
        pCurrentList = pNewList;
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
        SetCurrentGroupAndList(pObj, pNewObjList);

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
        InvalidateAllWin();

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
    SdrObject* pLastGroup = GetCurrentGroup();
    if (!pLastGroup)
        return;

    bool bGlueInvalidate = GetView().ImpIsGlueVisible();

    if(bGlueInvalidate)
        GetView().GlueInvalidate();

    SdrObject* pParentGroup = pLastGroup->getParentSdrObjectFromSdrObject();
    SdrObjList* pParentList = GetPage();

    if(pParentGroup)
        pParentList = pParentGroup->GetSubList();

    // deselect everything
    GetView().UnmarkAll();

    // allocations, pCurrentGroup and pCurrentList need to be set
    SetCurrentGroupAndList(pParentGroup, pParentList);

    // select the group we just left
    if (GetView().GetSdrPageView())
        GetView().MarkObj(pLastGroup, GetView().GetSdrPageView());

    GetView().AdjustMarkHdl();

    // invalidate only if view wants to visualize group entering
    InvalidateAllWin();

    if(bGlueInvalidate)
        GetView().GlueInvalidate();
}

void SdrPageView::LeaveAllGroup()
{
    SdrObject* pLastGroup = GetCurrentGroup();
    if (!pLastGroup)
        return;

    bool bGlueInvalidate = GetView().ImpIsGlueVisible();

    if(bGlueInvalidate)
        GetView().GlueInvalidate();

    // deselect everything
    GetView().UnmarkAll();

    // allocations, pCurrentGroup and pCurrentList always need to be set
    SetCurrentGroupAndList(nullptr, GetPage());

    // find and select uppermost group
    while (pLastGroup->getParentSdrObjectFromSdrObject())
        pLastGroup = pLastGroup->getParentSdrObjectFromSdrObject();

    if (GetView().GetSdrPageView())
        GetView().MarkObj(pLastGroup, GetView().GetSdrPageView());

    GetView().AdjustMarkHdl();

    // invalidate only when view wants to visualize group entering
    InvalidateAllWin();

    if(bGlueInvalidate)
        GetView().GlueInvalidate();
}

sal_uInt16 SdrPageView::GetEnteredLevel() const
{
    sal_uInt16 nCount=0;
    SdrObject* pGrp=GetCurrentGroup();
    while (pGrp!=nullptr) {
        nCount++;
        pGrp=pGrp->getParentSdrObjectFromSdrObject();
    }
    return nCount;
}

void SdrPageView::CheckCurrentGroup()
{
    SdrObject* pGrp(GetCurrentGroup());

    while(nullptr != pGrp &&
        (!pGrp->IsInserted() || nullptr == pGrp->getParentSdrObjListFromSdrObject() || nullptr == pGrp->getSdrPageFromSdrObject()))
    {
        // anything outside of the borders?
        pGrp = pGrp->getParentSdrObjectFromSdrObject();
    }

    if(pGrp != GetCurrentGroup())
    {
        if(nullptr != pGrp)
        {
            EnterGroup(pGrp);
        }
        else
        {
            LeaveAllGroup();
        }
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
