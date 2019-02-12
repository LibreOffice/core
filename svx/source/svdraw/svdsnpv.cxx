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


#include <svx/svdsnpv.hxx>
#include <math.h>

#include <svx/svdetc.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>
#include <svx/sdr/overlay/overlayobjectlist.hxx>
#include <sdr/overlay/overlaycrosshair.hxx>
#include <sdr/overlay/overlayhelpline.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <tools/debug.hxx>
#include <vcl/ptrstyle.hxx>


class ImplPageOriginOverlay
{
    // The OverlayObjects
    sdr::overlay::OverlayObjectList               maObjects;

    // The current position in logical coordinates
    basegfx::B2DPoint                               maPosition;

public:
    ImplPageOriginOverlay(const SdrPaintView& rView, const basegfx::B2DPoint& rStartPos);

    // The OverlayObjects are cleared using the destructor of OverlayObjectList.
    // That destructor calls clear() at the list which removes all objects from the
    // OverlayManager and deletes them.

    void SetPosition(const basegfx::B2DPoint& rNewPosition);
};

ImplPageOriginOverlay::ImplPageOriginOverlay(const SdrPaintView& rView, const basegfx::B2DPoint& rStartPos)
:   maPosition(rStartPos)
{
    for(sal_uInt32 a(0); a < rView.PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = rView.GetPaintWindow(a);
        const rtl::Reference< sdr::overlay::OverlayManager >& xTargetOverlay = pCandidate->GetOverlayManager();

        if (xTargetOverlay.is())
        {
            std::unique_ptr<sdr::overlay::OverlayCrosshairStriped> aNew(new sdr::overlay::OverlayCrosshairStriped(
                maPosition));
            xTargetOverlay->add(*aNew);
            maObjects.append(std::move(aNew));
        }
    }
}

void ImplPageOriginOverlay::SetPosition(const basegfx::B2DPoint& rNewPosition)
{
    if(rNewPosition != maPosition)
    {
        // apply to OverlayObjects
        for(sal_uInt32 a(0); a < maObjects.count(); a++)
        {
            sdr::overlay::OverlayCrosshairStriped* pCandidate =
                static_cast< sdr::overlay::OverlayCrosshairStriped* >(&maObjects.getOverlayObject(a));

            if(pCandidate)
            {
                pCandidate->setBasePosition(rNewPosition);
            }
        }

        // remember new position
        maPosition = rNewPosition;
    }
}


class ImplHelpLineOverlay
{
    // The OverlayObjects
    sdr::overlay::OverlayObjectList               maObjects;

    // The current position in logical coordinates
    basegfx::B2DPoint                               maPosition;

    // HelpLine specific stuff
    SdrPageView* const                              mpPageView;
    sal_uInt16 const                                mnHelpLineNumber;
    SdrHelpLineKind const                           meHelpLineKind;

public:
    ImplHelpLineOverlay(const SdrPaintView& rView, const basegfx::B2DPoint& rStartPos,
        SdrPageView* pPageView, sal_uInt16 nHelpLineNumber, SdrHelpLineKind eKind);

    // The OverlayObjects are cleared using the destructor of OverlayObjectList.
    // That destructor calls clear() at the list which removes all objects from the
    // OverlayManager and deletes them.

    void SetPosition(const basegfx::B2DPoint& rNewPosition);

    // access to HelpLine specific stuff
    SdrPageView* GetPageView() const { return mpPageView; }
    sal_uInt16 GetHelpLineNumber() const { return mnHelpLineNumber; }
    SdrHelpLineKind GetHelpLineKind() const { return meHelpLineKind; }
};

ImplHelpLineOverlay::ImplHelpLineOverlay(
    const SdrPaintView& rView, const basegfx::B2DPoint& rStartPos,
    SdrPageView* pPageView, sal_uInt16 nHelpLineNumber, SdrHelpLineKind eKind)
:   maPosition(rStartPos),
    mpPageView(pPageView),
    mnHelpLineNumber(nHelpLineNumber),
    meHelpLineKind(eKind)
{
    for(sal_uInt32 a(0); a < rView.PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = rView.GetPaintWindow(a);
        const rtl::Reference< sdr::overlay::OverlayManager >& xTargetOverlay = pCandidate->GetOverlayManager();

        if (xTargetOverlay.is())
        {
            std::unique_ptr<sdr::overlay::OverlayHelplineStriped> aNew(new sdr::overlay::OverlayHelplineStriped(
                maPosition, meHelpLineKind));
            xTargetOverlay->add(*aNew);
            maObjects.append(std::move(aNew));
        }
    }
}

void ImplHelpLineOverlay::SetPosition(const basegfx::B2DPoint& rNewPosition)
{
    if(rNewPosition != maPosition)
    {
        // apply to OverlayObjects
        for(sal_uInt32 a(0); a < maObjects.count(); a++)
        {
            sdr::overlay::OverlayHelplineStriped* pCandidate =
                static_cast< sdr::overlay::OverlayHelplineStriped* >(&maObjects.getOverlayObject(a));

            if(pCandidate)
            {
                pCandidate->setBasePosition(rNewPosition);
            }
        }

        // remember new position
        maPosition = rNewPosition;
    }
}

SdrSnapView::SdrSnapView(
    SdrModel& rSdrModel,
    OutputDevice* pOut)
:   SdrPaintView(rSdrModel, pOut)
    ,mpPageOriginOverlay(nullptr)
    ,mpHelpLineOverlay(nullptr)
    ,nMagnSizPix(4)
    ,nSnapAngle(1500)
    ,nEliminatePolyPointLimitAngle(0)
    ,eCrookMode(SdrCrookMode::Rotate)
    ,bSnapEnab(true)
    ,bGridSnap(true)
    ,bBordSnap(true)
    ,bHlplSnap(true)
    ,bOFrmSnap(true)
    ,bOPntSnap(false)
    ,bOConSnap(true)
    ,bMoveSnapOnlyTopLeft(false)
    ,bOrtho(false)
    ,bBigOrtho(true)
    ,bAngleSnapEnab(false)
    ,bMoveOnlyDragging(false)
    ,bSlantButShear(false)
    ,bCrookNoContortion(false)
    ,bEliminatePolyPoints(false)
{
}

SdrSnapView::~SdrSnapView()
{
    BrkSetPageOrg();
    BrkDragHelpLine();
}


bool SdrSnapView::IsAction() const
{
    return IsSetPageOrg() || IsDragHelpLine() || SdrPaintView::IsAction();
}

void SdrSnapView::MovAction(const Point& rPnt)
{
    SdrPaintView::MovAction(rPnt);
    if (IsSetPageOrg()) {
        MovSetPageOrg(rPnt);
    }
    if (IsDragHelpLine()) {
        MovDragHelpLine(rPnt);
    }
}

void SdrSnapView::EndAction()
{
    if (IsSetPageOrg()) {
        EndSetPageOrg();
    }
    if (IsDragHelpLine()) {
        EndDragHelpLine();
    }
    SdrPaintView::EndAction();
}

void SdrSnapView::BckAction()
{
    BrkSetPageOrg();
    BrkDragHelpLine();
    SdrPaintView::BckAction();
}

void SdrSnapView::BrkAction()
{
    BrkSetPageOrg();
    BrkDragHelpLine();
    SdrPaintView::BrkAction();
}

void SdrSnapView::TakeActionRect(tools::Rectangle& rRect) const
{
    if (IsSetPageOrg() || IsDragHelpLine()) {
        rRect=tools::Rectangle(maDragStat.GetNow(),maDragStat.GetNow());
    } else {
        SdrPaintView::TakeActionRect(rRect);
    }
}


Point SdrSnapView::GetSnapPos(const Point& rPnt, const SdrPageView* pPV) const
{
    Point aPt(rPnt);
    SnapPos(aPt,pPV);
    return aPt;
}

#define NOT_SNAPPED 0x7FFFFFFF
SdrSnap SdrSnapView::SnapPos(Point& rPnt, const SdrPageView* pPV) const
{
    if (!bSnapEnab) return SdrSnap::NOTSNAPPED;
    long x=rPnt.X();
    long y=rPnt.Y();
    if (pPV==nullptr) {
        pPV=GetSdrPageView();
        if (pPV==nullptr) return SdrSnap::NOTSNAPPED;
    }

    long dx=NOT_SNAPPED;
    long dy=NOT_SNAPPED;
    long dx1,dy1;
    long mx=aMagnSiz.Width();
    long my=aMagnSiz.Height();
    if (mbHlplVisible && bHlplSnap && !IsDragHelpLine())
    {
        const SdrHelpLineList& rHLL=pPV->GetHelpLines();
        sal_uInt16 nCount=rHLL.GetCount();
        for (sal_uInt16 i=nCount; i>0;) {
            i--;
            const SdrHelpLine& rHL=rHLL[i];
            const Point& rPos=rHL.GetPos();
            switch (rHL.GetKind()) {
                case SdrHelpLineKind::Vertical: {
                    long a=x-rPos.X();
                    if (std::abs(a)<=mx) { dx1=-a; if (std::abs(dx1)<std::abs(dx)) dx=dx1; }
                } break;
                case SdrHelpLineKind::Horizontal: {
                    long b=y-rPos.Y();
                    if (std::abs(b)<=my) { dy1=-b; if (std::abs(dy1)<std::abs(dy)) dy=dy1; }
                } break;
                case SdrHelpLineKind::Point: {
                    long a=x-rPos.X();
                    long b=y-rPos.Y();
                    if (std::abs(a)<=mx && std::abs(b)<=my) {
                        dx1=-a; dy1=-b;
                        if (std::abs(dx1)<std::abs(dx) && std::abs(dy1)<std::abs(dy)) { dx=dx1; dy=dy1; }
                    }
                } break;
            } // switch
        }
    }
    if (mbBordVisible && bBordSnap) {
        SdrPage* pPage=pPV->GetPage();
        long xs=pPage->GetWidth();
        long ys=pPage->GetHeight();
        long lft=pPage->GetLeftBorder();
        long rgt=pPage->GetRightBorder();
        long upp=pPage->GetUpperBorder();
        long lwr=pPage->GetLowerBorder();
        long a;
        a=x- lft    ; if (std::abs(a)<=mx) { dx1=-a; if (std::abs(dx1)<std::abs(dx)) dx=dx1; } // left margin
        a=x-(xs-rgt); if (std::abs(a)<=mx) { dx1=-a; if (std::abs(dx1)<std::abs(dx)) dx=dx1; } // right margin
        a=x         ; if (std::abs(a)<=mx) { dx1=-a; if (std::abs(dx1)<std::abs(dx)) dx=dx1; } // left edge of paper
        a=x- xs     ; if (std::abs(a)<=mx) { dx1=-a; if (std::abs(dx1)<std::abs(dx)) dx=dx1; } // right edge of paper
        a=y- upp    ; if (std::abs(a)<=my) { dy1=-a; if (std::abs(dy1)<std::abs(dy)) dy=dy1; } // left margin
        a=y-(ys-lwr); if (std::abs(a)<=my) { dy1=-a; if (std::abs(dy1)<std::abs(dy)) dy=dy1; } // right margin
        a=y         ; if (std::abs(a)<=my) { dy1=-a; if (std::abs(dy1)<std::abs(dy)) dy=dy1; } // left edge of paper
        a=y- ys     ; if (std::abs(a)<=my) { dy1=-a; if (std::abs(dy1)<std::abs(dy)) dy=dy1; } // right edge of paper
    }
    if (bOFrmSnap || bOPntSnap) {
        sal_uInt32 nMaxPointSnapCount=200;
        sal_uInt32 nMaxFrameSnapCount=200;

        // go back to SdrIterMode::DeepNoGroups runthrough for snap to object comparisons
        SdrObjListIter aIter(pPV->GetPage(),SdrIterMode::DeepNoGroups,true);

        while (aIter.IsMore() && (nMaxPointSnapCount>0 || nMaxFrameSnapCount>0)) {
            SdrObject* pO=aIter.Next();
            tools::Rectangle aRect(pO->GetCurrentBoundRect());
            aRect.AdjustLeft( -mx );
            aRect.AdjustRight(mx );
            aRect.AdjustTop( -my );
            aRect.AdjustBottom(my );
            if (aRect.IsInside(rPnt)) {
                if (bOPntSnap && nMaxPointSnapCount>0)
                {
                    sal_uInt32 nCount(pO->GetSnapPointCount());
                    for (sal_uInt32 i(0); i < nCount && nMaxPointSnapCount > 0; i++)
                    {
                        Point aP(pO->GetSnapPoint(i));
                        dx1=x-aP.X();
                        dy1=y-aP.Y();
                        if (std::abs(dx1)<=mx && std::abs(dy1)<=my && std::abs(dx1)<std::abs(dx) && std::abs(dy1)<std::abs(dy)) {
                            dx=-dx1;
                            dy=-dy1;
                        }
                        nMaxPointSnapCount--;
                    }
                }
                if (bOFrmSnap && nMaxFrameSnapCount>0) {
                    tools::Rectangle aLog(pO->GetSnapRect());
                    tools::Rectangle aR1(aLog);
                    aR1.AdjustLeft( -mx );
                    aR1.AdjustRight(mx );
                    aR1.AdjustTop( -my );
                    aR1.AdjustBottom(my );
                    if (aR1.IsInside(rPnt)) {
                        if (std::abs(x-aLog.Left  ())<=mx) { dx1=-(x-aLog.Left  ()); if (std::abs(dx1)<std::abs(dx)) dx=dx1; }
                        if (std::abs(x-aLog.Right ())<=mx) { dx1=-(x-aLog.Right ()); if (std::abs(dx1)<std::abs(dx)) dx=dx1; }
                        if (std::abs(y-aLog.Top   ())<=my) { dy1=-(y-aLog.Top   ()); if (std::abs(dy1)<std::abs(dy)) dy=dy1; }
                        if (std::abs(y-aLog.Bottom())<=my) { dy1=-(y-aLog.Bottom()); if (std::abs(dy1)<std::abs(dy)) dy=dy1; }
                    }
                    nMaxFrameSnapCount--;
                }
            }
        }
    }
    if(bGridSnap)
    {
        double fSnapWidth(aSnapWdtX);
        if(dx == NOT_SNAPPED && fSnapWidth != 0.0)
        {
            double fx = static_cast<double>(x);

            // round instead of trunc
            if(fx - static_cast<double>(pPV->GetPageOrigin().X()) >= 0.0)
                fx += fSnapWidth / 2.0;
            else
                fx -= fSnapWidth / 2.0;

            x = static_cast<long>((fx - static_cast<double>(pPV->GetPageOrigin().X())) / fSnapWidth);
            x = static_cast<long>(static_cast<double>(x) * fSnapWidth + static_cast<double>(pPV->GetPageOrigin().X()));
            dx = 0;
        }
        fSnapWidth = double(aSnapWdtY);
        if(dy == NOT_SNAPPED && fSnapWidth)
        {
            double fy = static_cast<double>(y);

            // round instead of trunc
            if(fy - static_cast<double>(pPV->GetPageOrigin().Y()) >= 0.0)
                fy += fSnapWidth / 2.0;
            else
                fy -= fSnapWidth / 2.0;

            y = static_cast<long>((fy - static_cast<double>(pPV->GetPageOrigin().Y())) / fSnapWidth);
            y = static_cast<long>(static_cast<double>(y) * fSnapWidth + static_cast<double>(pPV->GetPageOrigin().Y()));
            dy = 0;
        }
    }
    SdrSnap bRet=SdrSnap::NOTSNAPPED;
    if (dx==NOT_SNAPPED) dx=0; else bRet|=SdrSnap::XSNAPPED;
    if (dy==NOT_SNAPPED) dy=0; else bRet|=SdrSnap::YSNAPPED;
    rPnt.setX(x+dx );
    rPnt.setY(y+dy );
    return bRet;
}

void SdrSnapView::CheckSnap(const Point& rPt, long& nBestXSnap, long& nBestYSnap, bool& bXSnapped, bool& bYSnapped) const
{
    Point aPt(rPt);
    SdrSnap nRet=SnapPos(aPt,nullptr);
    aPt-=rPt;
    if (nRet & SdrSnap::XSNAPPED) {
        if (bXSnapped) {
            if (std::abs(aPt.X())<std::abs(nBestXSnap)) {
                nBestXSnap=aPt.X();
            }
        } else {
            nBestXSnap=aPt.X();
            bXSnapped=true;
        }
    }
    if (nRet & SdrSnap::YSNAPPED) {
        if (bYSnapped) {
            if (std::abs(aPt.Y())<std::abs(nBestYSnap)) {
                nBestYSnap=aPt.Y();
            }
        } else {
            nBestYSnap=aPt.Y();
            bYSnapped=true;
        }
    }
}


void SdrSnapView::BegSetPageOrg(const Point& rPnt)
{
    BrkAction();

    DBG_ASSERT(nullptr == mpPageOriginOverlay, "SdrSnapView::BegSetPageOrg: There exists a ImplPageOriginOverlay (!)");
    basegfx::B2DPoint aStartPos(rPnt.X(), rPnt.Y());
    mpPageOriginOverlay = new ImplPageOriginOverlay(*this, aStartPos);
    maDragStat.Reset(GetSnapPos(rPnt,nullptr));
}

void SdrSnapView::MovSetPageOrg(const Point& rPnt)
{
    if(IsSetPageOrg())
    {
        maDragStat.NextMove(GetSnapPos(rPnt,nullptr));
        DBG_ASSERT(mpPageOriginOverlay, "SdrSnapView::MovSetPageOrg: no ImplPageOriginOverlay (!)");
        basegfx::B2DPoint aNewPos(maDragStat.GetNow().X(), maDragStat.GetNow().Y());
        mpPageOriginOverlay->SetPosition(aNewPos);
    }
}

void SdrSnapView::EndSetPageOrg()
{
    if(IsSetPageOrg())
    {
        SdrPageView* pPV = GetSdrPageView();

        if(pPV)
        {
            Point aPnt(maDragStat.GetNow());
            pPV->SetPageOrigin(aPnt);
        }

        // cleanup
        BrkSetPageOrg();
    }
}

void SdrSnapView::BrkSetPageOrg()
{
    if(IsSetPageOrg())
    {
        DBG_ASSERT(mpPageOriginOverlay, "SdrSnapView::MovSetPageOrg: no ImplPageOriginOverlay (!)");
        delete mpPageOriginOverlay;
        mpPageOriginOverlay = nullptr;
    }
}


bool SdrSnapView::PickHelpLine(const Point& rPnt, short nTol, const OutputDevice& rOut, sal_uInt16& rnHelpLineNum, SdrPageView*& rpPV) const
{
    rpPV=nullptr;
    nTol=ImpGetHitTolLogic(nTol,&rOut);
    SdrPageView* pPV = GetSdrPageView();

    if(pPV)
    {
        Point aPnt(rPnt);
        sal_uInt16 nIndex=pPV->GetHelpLines().HitTest(aPnt,sal_uInt16(nTol),rOut);
        if (nIndex!=SDRHELPLINE_NOTFOUND) {
            rpPV=pPV;
            rnHelpLineNum=nIndex;
            return true;
        }
    }
    return false;
}

// start HelpLine drag for new HelpLine
bool SdrSnapView::BegDragHelpLine(sal_uInt16 nHelpLineNum, SdrPageView* pPV)
{
    bool bRet(false);

    BrkAction();

    if(pPV && nHelpLineNum < pPV->GetHelpLines().GetCount())
    {
        const SdrHelpLineList& rHelpLines = pPV->GetHelpLines();
        const SdrHelpLine& rHelpLine = rHelpLines[nHelpLineNum];
        Point aHelpLinePos = rHelpLine.GetPos();
        basegfx::B2DPoint aStartPos(aHelpLinePos.X(), aHelpLinePos.Y());

        DBG_ASSERT(nullptr == mpHelpLineOverlay, "SdrSnapView::BegDragHelpLine: There exists a ImplHelpLineOverlay (!)");
        mpHelpLineOverlay = new ImplHelpLineOverlay(*this, aStartPos, pPV, nHelpLineNum, rHelpLine.GetKind());

        maDragStat.Reset(GetSnapPos(aHelpLinePos, pPV));
        maDragStat.SetMinMove(ImpGetMinMovLogic(-3, nullptr));

        bRet = true;
    }

    return bRet;
}

// start HelpLine drag with existing HelpLine
void SdrSnapView::BegDragHelpLine(const Point& rPnt, SdrHelpLineKind eNewKind)
{
    BrkAction();

    if(GetSdrPageView())
    {
        DBG_ASSERT(nullptr == mpHelpLineOverlay, "SdrSnapView::BegDragHelpLine: There exists a ImplHelpLineOverlay (!)");
        basegfx::B2DPoint aStartPos(rPnt.X(), rPnt.Y());
        mpHelpLineOverlay = new ImplHelpLineOverlay(*this, aStartPos, nullptr, 0, eNewKind);
        maDragStat.Reset(GetSnapPos(rPnt, nullptr));
    }
}

PointerStyle SdrSnapView::GetDraggedHelpLinePointer() const
{
    if(IsDragHelpLine())
    {
        switch(mpHelpLineOverlay->GetHelpLineKind())
        {
            case SdrHelpLineKind::Vertical  : return PointerStyle::ESize;
            case SdrHelpLineKind::Horizontal: return PointerStyle::SSize;
            default                    : return PointerStyle::Move;
        }
    }

    return PointerStyle::Move;
}

void SdrSnapView::MovDragHelpLine(const Point& rPnt)
{
    if(IsDragHelpLine() && maDragStat.CheckMinMoved(rPnt))
    {
        Point aPnt(GetSnapPos(rPnt, nullptr));

        if(aPnt != maDragStat.GetNow())
        {
            maDragStat.NextMove(aPnt);
            DBG_ASSERT(mpHelpLineOverlay, "SdrSnapView::MovDragHelpLine: no ImplHelpLineOverlay (!)");
            basegfx::B2DPoint aNewPos(maDragStat.GetNow().X(), maDragStat.GetNow().Y());
            mpHelpLineOverlay->SetPosition(aNewPos);
        }
    }
}

bool SdrSnapView::EndDragHelpLine()
{
    bool bRet(false);

    if(IsDragHelpLine())
    {
        if(maDragStat.IsMinMoved())
        {
            SdrPageView* pPageView = mpHelpLineOverlay->GetPageView();

            if(pPageView)
            {
                // moved existing one
                Point aPnt(maDragStat.GetNow());
                const SdrHelpLineList& rHelpLines = pPageView->GetHelpLines();
                SdrHelpLine aChangedHelpLine = rHelpLines[mpHelpLineOverlay->GetHelpLineNumber()];
                aChangedHelpLine.SetPos(aPnt);
                pPageView->SetHelpLine(mpHelpLineOverlay->GetHelpLineNumber(), aChangedHelpLine);

                bRet = true;
            }
            else
            {
                // create new one
                pPageView = GetSdrPageView();

                if(pPageView)
                {
                    Point aPnt(maDragStat.GetNow());
                    SdrHelpLine aNewHelpLine(mpHelpLineOverlay->GetHelpLineKind(), aPnt);
                    pPageView->InsertHelpLine(aNewHelpLine);

                    bRet = true;
                }
            }
        }

        // cleanup
        BrkDragHelpLine();
    }

    return bRet;
}

void SdrSnapView::BrkDragHelpLine()
{
    if(IsDragHelpLine())
    {
        DBG_ASSERT(mpHelpLineOverlay, "SdrSnapView::EndDragHelpLine: no ImplHelpLineOverlay (!)");
        delete mpHelpLineOverlay;
        mpHelpLineOverlay = nullptr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
