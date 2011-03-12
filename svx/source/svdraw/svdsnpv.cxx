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

#include <svx/svdsnpv.hxx>
#include <math.h>

#include <svx/svdetc.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdpage.hxx>
#include "svx/svditer.hxx"
#include <svx/sdr/overlay/overlayobjectlist.hxx>
#include <svx/sdr/overlay/overlaycrosshair.hxx>
#include <svx/sdr/overlay/overlayhelpline.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdrpaintwindow.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// #114409#-1 Migrate PageOrigin
class ImplPageOriginOverlay
{
    // The OverlayObjects
    ::sdr::overlay::OverlayObjectList               maObjects;

    // The current position in logical coodinates
    basegfx::B2DPoint                               maPosition;

public:
    ImplPageOriginOverlay(const SdrPaintView& rView, const basegfx::B2DPoint& rStartPos);
    ~ImplPageOriginOverlay();

    void SetPosition(const basegfx::B2DPoint& rNewPosition);
};

ImplPageOriginOverlay::ImplPageOriginOverlay(const SdrPaintView& rView, const basegfx::B2DPoint& rStartPos)
:   maPosition(rStartPos)
{
    for(sal_uInt32 a(0L); a < rView.PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = rView.GetPaintWindow(a);
        ::sdr::overlay::OverlayManager* pTargetOverlay = pCandidate->GetOverlayManager();

        if(pTargetOverlay)
        {
            ::sdr::overlay::OverlayCrosshairStriped* aNew = new ::sdr::overlay::OverlayCrosshairStriped(
                maPosition);
            pTargetOverlay->add(*aNew);
            maObjects.append(*aNew);
        }
    }
}

ImplPageOriginOverlay::~ImplPageOriginOverlay()
{
    // The OverlayObjects are cleared using the destructor of OverlayObjectList.
    // That destructor calls clear() at the list which removes all objects from the
    // OverlayManager and deletes them.
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

////////////////////////////////////////////////////////////////////////////////////////////////////
// #114409#-2 Migrate HelpLine
class ImplHelpLineOverlay
{
    // The OverlayObjects
    ::sdr::overlay::OverlayObjectList               maObjects;

    // The current position in logical coodinates
    basegfx::B2DPoint                               maPosition;

    // HelpLine specific stuff
    SdrPageView*                                    mpPageView;
    sal_uInt16                                      mnHelpLineNumber;
    SdrHelpLineKind                                 meHelpLineKind;

public:
    ImplHelpLineOverlay(const SdrPaintView& rView, const basegfx::B2DPoint& rStartPos,
        SdrPageView* pPageView, sal_uInt16 nHelpLineNumber, SdrHelpLineKind eKind);
    ~ImplHelpLineOverlay();

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
    for(sal_uInt32 a(0L); a < rView.PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = rView.GetPaintWindow(a);
        ::sdr::overlay::OverlayManager* pTargetOverlay = pCandidate->GetOverlayManager();

        if(pTargetOverlay)
        {
            ::sdr::overlay::OverlayHelplineStriped* aNew = new ::sdr::overlay::OverlayHelplineStriped(
                maPosition, meHelpLineKind);
            pTargetOverlay->add(*aNew);
            maObjects.append(*aNew);
        }
    }
}

ImplHelpLineOverlay::~ImplHelpLineOverlay()
{
    // The OverlayObjects are cleared using the destructor of OverlayObjectList.
    // That destructor calls clear() at the list which removes all objects from the
    // OverlayManager and deletes them.
}

void ImplHelpLineOverlay::SetPosition(const basegfx::B2DPoint& rNewPosition)
{
    if(rNewPosition != maPosition)
    {
        // apply to OverlayObjects
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

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@@@  @@  @@  @@@@  @@@@@   @@ @@ @@ @@@@@ @@   @@
// @@  @@ @@@ @@ @@  @@ @@  @@  @@ @@ @@ @@    @@   @@
// @@     @@@@@@ @@  @@ @@  @@  @@ @@ @@ @@    @@ @ @@
//  @@@@  @@@@@@ @@@@@@ @@@@@   @@@@@ @@ @@@@  @@@@@@@
//     @@ @@ @@@ @@  @@ @@       @@@  @@ @@    @@@@@@@
// @@  @@ @@  @@ @@  @@ @@       @@@  @@ @@    @@@ @@@
//  @@@@  @@  @@ @@  @@ @@        @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrSnapView::ClearVars()
{
    nMagnSizPix=4;
    bSnapEnab=sal_True;
    bGridSnap=sal_True;
    bSnapTo1Pix=sal_True;
    bBordSnap=sal_True;
    bHlplSnap=sal_True;
    bOFrmSnap=sal_True;
    bOPntSnap=sal_False;
    bOConSnap=sal_True;
    bMoveMFrmSnap=sal_True;
    bMoveOFrmSnap=sal_True;
    bMoveOPntSnap=sal_True;
    bMoveOConSnap=sal_True;
    bMoveSnapOnlyTopLeft=sal_False;
    bOrtho=sal_False;
    bBigOrtho=sal_True;
    nSnapAngle=1500;
    bAngleSnapEnab=sal_False;
    bMoveOnlyDragging=sal_False;
    bSlantButShear=sal_False;
    bCrookNoContortion=sal_False;
    eCrookMode=SDRCROOK_ROTATE;
    bHlplFixed=sal_False;
    bEliminatePolyPoints=sal_False;
    nEliminatePolyPointLimitAngle=0;

    // #114409#-1 Migrate PageOrigin
    BrkSetPageOrg();

    // #114409#-2 Migrate HelpLine
    BrkDragHelpLine();
}

SdrSnapView::SdrSnapView(SdrModel* pModel1, OutputDevice* pOut):
    SdrPaintView(pModel1,pOut),
    // #114409#-1 Migrate PageOrigin
    mpPageOriginOverlay(0L),
    // #114409#-2 Migrate HelpLine
    mpHelpLineOverlay(0L)
{
    ClearVars();
}

// #114409#-1 Migrate PageOrigin
SdrSnapView::~SdrSnapView()
{
    // #114409#-1 Migrate PageOrigin
    BrkSetPageOrg();

    // #114409#-2 Migrate HelpLine
    BrkDragHelpLine();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SdrSnapView::IsAction() const
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

void SdrSnapView::TakeActionRect(Rectangle& rRect) const
{
    if (IsSetPageOrg() || IsDragHelpLine()) {
        rRect=Rectangle(aDragStat.GetNow(),aDragStat.GetNow());
    } else {
        SdrPaintView::TakeActionRect(rRect);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Point SdrSnapView::GetSnapPos(const Point& rPnt, const SdrPageView* pPV) const
{
    Point aPt(rPnt);
    SnapPos(aPt,pPV);
    return aPt;
}

#define NOT_SNAPPED 0x7FFFFFFF
sal_uInt16 SdrSnapView::SnapPos(Point& rPnt, const SdrPageView* pPV) const
{
    if (!bSnapEnab) return SDRSNAP_NOTSNAPPED;
    long x=rPnt.X();
    long y=rPnt.Y();
    if (pPV==NULL) {
        pPV=GetSdrPageView();
        if (pPV==NULL) return SDRSNAP_NOTSNAPPED;
    }

    long dx=NOT_SNAPPED;
    long dy=NOT_SNAPPED;
    long dx1,dy1;
    long mx=aMagnSiz.Width();
    long my=aMagnSiz.Height();
    if (bHlplVisible && bHlplSnap && !IsDragHelpLine())
    {
        const SdrHelpLineList& rHLL=pPV->GetHelpLines();
        sal_uInt16 nAnz=rHLL.GetCount();
        for (sal_uInt16 i=nAnz; i>0;) {
            i--;
            const SdrHelpLine& rHL=rHLL[i];
            const Point& rPos=rHL.GetPos();
            switch (rHL.GetKind()) {
                case SDRHELPLINE_VERTICAL: {
                    long a=x-rPos.X();
                    if (Abs(a)<=mx) { dx1=-a; if (Abs(dx1)<Abs(dx)) dx=dx1; }
                } break;
                case SDRHELPLINE_HORIZONTAL: {
                    long b=y-rPos.Y();
                    if (Abs(b)<=my) { dy1=-b; if (Abs(dy1)<Abs(dy)) dy=dy1; }
                } break;
                case SDRHELPLINE_POINT: {
                    long a=x-rPos.X();
                    long b=y-rPos.Y();
                    if (Abs(a)<=mx && Abs(b)<=my) {
                        dx1=-a; dy1=-b;
                        if (Abs(dx1)<Abs(dx) && Abs(dy1)<Abs(dy)) { dx=dx1; dy=dy1; }
                    }
                } break;
            } // switch
        }
    }
    if (bBordVisible && bBordSnap) {
        SdrPage* pPage=pPV->GetPage();
        long xs=pPage->GetWdt();
        long ys=pPage->GetHgt();
        long lft=pPage->GetLftBorder();
        long rgt=pPage->GetRgtBorder();
        long upp=pPage->GetUppBorder();
        long lwr=pPage->GetLwrBorder();
        long a;
        a=x- lft    ; if (Abs(a)<=mx) { dx1=-a; if (Abs(dx1)<Abs(dx)) dx=dx1; } // linker Rand
        a=x-(xs-rgt); if (Abs(a)<=mx) { dx1=-a; if (Abs(dx1)<Abs(dx)) dx=dx1; } // rechter Rand
        a=x         ; if (Abs(a)<=mx) { dx1=-a; if (Abs(dx1)<Abs(dx)) dx=dx1; } // linke Papierkante
        a=x- xs     ; if (Abs(a)<=mx) { dx1=-a; if (Abs(dx1)<Abs(dx)) dx=dx1; } // rechte Papierkante
        a=y- upp    ; if (Abs(a)<=my) { dy1=-a; if (Abs(dy1)<Abs(dy)) dy=dy1; } // linker Rand
        a=y-(ys-lwr); if (Abs(a)<=my) { dy1=-a; if (Abs(dy1)<Abs(dy)) dy=dy1; } // rechter Rand
        a=y         ; if (Abs(a)<=my) { dy1=-a; if (Abs(dy1)<Abs(dy)) dy=dy1; } // linke Papierkante
        a=y- ys     ; if (Abs(a)<=my) { dy1=-a; if (Abs(dy1)<Abs(dy)) dy=dy1; } // rechte Papierkante
    }
    if (bOFrmSnap || bOPntSnap /*|| (bConnVisible && bOConSnap)*/) {
        sal_uIntPtr nMaxPointSnapCount=200;
        sal_uIntPtr nMaxFrameSnapCount=200;

        // #97981# go back to IM_DEEPNOGROUPS runthrough for snap to object comparisons
        SdrObjListIter aIter(*pPV->GetPage(),/*IM_FLAT*/IM_DEEPNOGROUPS,sal_True);

        while (aIter.IsMore() && (nMaxPointSnapCount>0 || nMaxFrameSnapCount>0)) {
            SdrObject* pO=aIter.Next();
            Rectangle aRect(pO->GetCurrentBoundRect());
            aRect.Left  ()-=mx;
            aRect.Right ()+=mx;
            aRect.Top   ()-=my;
            aRect.Bottom()+=my;
            if (aRect.IsInside(rPnt)) {
                if (bOPntSnap && nMaxPointSnapCount>0)
                {
                    sal_uInt32 nAnz(pO->GetSnapPointCount());
                    for (sal_uInt32 i(0L); i < nAnz && nMaxPointSnapCount > 0L; i++)
                    {
                        Point aP(pO->GetSnapPoint(i));
                        dx1=x-aP.X();
                        dy1=y-aP.Y();
                        if (Abs(dx1)<=mx && Abs(dy1)<=my && Abs(dx1)<Abs(dx) && Abs(dy1)<Abs(dy)) {
                            dx=-dx1;
                            dy=-dy1;
                        }
                        nMaxPointSnapCount--;
                    }
                }
                if (bOFrmSnap && nMaxFrameSnapCount>0) {
                    Rectangle aLog(pO->GetSnapRect());
                    Rectangle aR1(aLog);
                    aR1.Left  ()-=mx;
                    aR1.Right ()+=mx;
                    aR1.Top   ()-=my;
                    aR1.Bottom()+=my;
                    if (aR1.IsInside(rPnt)) {
                        if (Abs(x-aLog.Left  ())<=mx) { dx1=-(x-aLog.Left  ()); if (Abs(dx1)<Abs(dx)) dx=dx1; }
                        if (Abs(x-aLog.Right ())<=mx) { dx1=-(x-aLog.Right ()); if (Abs(dx1)<Abs(dx)) dx=dx1; }
                        if (Abs(y-aLog.Top   ())<=my) { dy1=-(y-aLog.Top   ()); if (Abs(dy1)<Abs(dy)) dy=dy1; }
                        if (Abs(y-aLog.Bottom())<=my) { dy1=-(y-aLog.Bottom()); if (Abs(dy1)<Abs(dy)) dy=dy1; }
                    }
                    nMaxFrameSnapCount--;
                }
            }
        }
    }
    if(bGridSnap)
    {
        double fSnapWidth = aSnapWdtX;
        if(dx == NOT_SNAPPED && fSnapWidth != 0.0)
        {
            double fx = (double)x;

            // round statt trunc
            if(fx - (double)pPV->GetPageOrigin().X() >= 0.0)
                fx += fSnapWidth / 2.0;
            else
                fx -= fSnapWidth / 2.0;

            x = (long)((fx - (double)pPV->GetPageOrigin().X()) / fSnapWidth);
            x = (long)((double)x * fSnapWidth + (double)pPV->GetPageOrigin().X());
            dx = 0;
        }
        fSnapWidth = aSnapWdtY;
        if(dy == NOT_SNAPPED && fSnapWidth)
        {
            double fy = (double)y;

            // round statt trunc
            if(fy - (double)pPV->GetPageOrigin().Y() >= 0.0)
                fy += fSnapWidth / 2.0;
            else
                fy -= fSnapWidth / 2.0;

            y = (long)((fy - (double)pPV->GetPageOrigin().Y()) / fSnapWidth);
            y = (long)((double)y * fSnapWidth + (double)pPV->GetPageOrigin().Y());
            dy = 0;
        }
    }
    sal_Bool bRet=SDRSNAP_NOTSNAPPED;
    if (dx==NOT_SNAPPED) dx=0; else bRet|=SDRSNAP_XSNAPPED;
    if (dy==NOT_SNAPPED) dy=0; else bRet|=SDRSNAP_YSNAPPED;
    rPnt.X()=x+dx;
    rPnt.Y()=y+dy;
    return bRet;
}

void SdrSnapView::CheckSnap(const Point& rPt, const SdrPageView* pPV, long& nBestXSnap, long& nBestYSnap, bool& bXSnapped, bool& bYSnapped) const
{
    Point aPt(rPt);
    sal_uInt16 nRet=SnapPos(aPt,pPV);
    aPt-=rPt;
    if ((nRet & SDRSNAP_XSNAPPED) !=0) {
        if (bXSnapped) {
            if (Abs(aPt.X())<Abs(nBestXSnap)) {
                nBestXSnap=aPt.X();
            }
        } else {
            nBestXSnap=aPt.X();
            bXSnapped=sal_True;
        }
    }
    if ((nRet & SDRSNAP_YSNAPPED) !=0) {
        if (bYSnapped) {
            if (Abs(aPt.Y())<Abs(nBestYSnap)) {
                nBestYSnap=aPt.Y();
            }
        } else {
            nBestYSnap=aPt.Y();
            bYSnapped=sal_True;
        }
    }
}

sal_uInt16 SdrSnapView::SnapRect(const Rectangle& rRect, const SdrPageView* pPV, long& rDX, long& rDY) const
{
    long nBestXSnap=0;
    long nBestYSnap=0;
    bool bXSnapped=sal_False;
    bool bYSnapped=sal_False;
    CheckSnap(rRect.TopLeft()    ,pPV,nBestXSnap,nBestYSnap,bXSnapped,bYSnapped);
    if (!bMoveSnapOnlyTopLeft) {
        CheckSnap(rRect.TopRight()   ,pPV,nBestXSnap,nBestYSnap,bXSnapped,bYSnapped);
        CheckSnap(rRect.BottomLeft() ,pPV,nBestXSnap,nBestYSnap,bXSnapped,bYSnapped);
        CheckSnap(rRect.BottomRight(),pPV,nBestXSnap,nBestYSnap,bXSnapped,bYSnapped);
    }
    rDX=nBestXSnap;
    rDY=nBestYSnap;
    sal_uInt16 nRet=0;
    if (bXSnapped) nRet+=SDRSNAP_XSNAPPED;
    if (bYSnapped) nRet+=SDRSNAP_YSNAPPED;
    return nRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SdrSnapView::BegSetPageOrg(const Point& rPnt)
{
    BrkAction();

    DBG_ASSERT(0L == mpPageOriginOverlay, "SdrSnapView::BegSetPageOrg: There exists a ImplPageOriginOverlay (!)");
    basegfx::B2DPoint aStartPos(rPnt.X(), rPnt.Y());
    mpPageOriginOverlay = new ImplPageOriginOverlay(*this, aStartPos);
    aDragStat.Reset(GetSnapPos(rPnt,NULL));

    return sal_True;
}

void SdrSnapView::MovSetPageOrg(const Point& rPnt)
{
    if(IsSetPageOrg())
    {
        aDragStat.NextMove(GetSnapPos(rPnt,NULL));
        DBG_ASSERT(mpPageOriginOverlay, "SdrSnapView::MovSetPageOrg: no ImplPageOriginOverlay (!)");
        basegfx::B2DPoint aNewPos(aDragStat.GetNow().X(), aDragStat.GetNow().Y());
        mpPageOriginOverlay->SetPosition(aNewPos);
    }
}

sal_Bool SdrSnapView::EndSetPageOrg()
{
    sal_Bool bRet(sal_False);

    if(IsSetPageOrg())
    {
        SdrPageView* pPV = GetSdrPageView();

        if(pPV)
        {
            Point aPnt(aDragStat.GetNow());
            pPV->SetPageOrigin(aPnt);
            bRet = sal_True;
        }

        // cleanup
        BrkSetPageOrg();
    }

    return bRet;
}

void SdrSnapView::BrkSetPageOrg()
{
    if(IsSetPageOrg())
    {
        DBG_ASSERT(mpPageOriginOverlay, "SdrSnapView::MovSetPageOrg: no ImplPageOriginOverlay (!)");
        delete mpPageOriginOverlay;
        mpPageOriginOverlay = 0L;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

sal_Bool SdrSnapView::PickHelpLine(const Point& rPnt, short nTol, const OutputDevice& rOut, sal_uInt16& rnHelpLineNum, SdrPageView*& rpPV) const
{
    rpPV=NULL;
    nTol=ImpGetHitTolLogic(nTol,&rOut);
    SdrPageView* pPV = GetSdrPageView();

    if(pPV)
    {
        Point aPnt(rPnt);
        sal_uInt16 nIndex=pPV->GetHelpLines().HitTest(aPnt,sal_uInt16(nTol),rOut);
        if (nIndex!=SDRHELPLINE_NOTFOUND) {
            rpPV=pPV;
            rnHelpLineNum=nIndex;
            return sal_True;
        }
    }
    return sal_False;
}

// start HelpLine drag for new HelpLine
sal_Bool SdrSnapView::BegDragHelpLine(sal_uInt16 nHelpLineNum, SdrPageView* pPV)
{
    sal_Bool bRet(sal_False);

    if(!bHlplFixed)
    {
        BrkAction();

        if(pPV && nHelpLineNum < pPV->GetHelpLines().GetCount())
        {
            const SdrHelpLineList& rHelpLines = pPV->GetHelpLines();
            const SdrHelpLine& rHelpLine = rHelpLines[nHelpLineNum];
            Point aHelpLinePos = rHelpLine.GetPos(); // + pPV->GetOffset();
            basegfx::B2DPoint aStartPos(aHelpLinePos.X(), aHelpLinePos.Y());

            DBG_ASSERT(0L == mpHelpLineOverlay, "SdrSnapView::BegDragHelpLine: There exists a ImplHelpLineOverlay (!)");
            mpHelpLineOverlay = new ImplHelpLineOverlay(*this, aStartPos, pPV, nHelpLineNum, rHelpLine.GetKind());

            aDragStat.Reset(GetSnapPos(aHelpLinePos, pPV));
            aDragStat.SetMinMove(ImpGetMinMovLogic(-3, 0L));

            bRet = sal_True;
        }
    }

    return bRet;
}

// start HelpLine drag with existing HelpLine
sal_Bool SdrSnapView::BegDragHelpLine(const Point& rPnt, SdrHelpLineKind eNewKind)
{
    sal_Bool bRet(sal_False);

    BrkAction();

    if(GetSdrPageView())
    {
        DBG_ASSERT(0L == mpHelpLineOverlay, "SdrSnapView::BegDragHelpLine: There exists a ImplHelpLineOverlay (!)");
        basegfx::B2DPoint aStartPos(rPnt.X(), rPnt.Y());
        mpHelpLineOverlay = new ImplHelpLineOverlay(*this, aStartPos, 0L, 0, eNewKind);
        aDragStat.Reset(GetSnapPos(rPnt, 0L));
        bRet = sal_True;
    }

    return bRet;
}

Pointer SdrSnapView::GetDraggedHelpLinePointer() const
{
    if(IsDragHelpLine())
    {
        switch(mpHelpLineOverlay->GetHelpLineKind())
        {
            case SDRHELPLINE_VERTICAL  : return Pointer(POINTER_ESIZE);
            case SDRHELPLINE_HORIZONTAL: return Pointer(POINTER_SSIZE);
            default                    : return Pointer(POINTER_MOVE);
        }
    }

    return Pointer(POINTER_MOVE);
}

void SdrSnapView::MovDragHelpLine(const Point& rPnt)
{
    if(IsDragHelpLine() && aDragStat.CheckMinMoved(rPnt))
    {
        Point aPnt(GetSnapPos(rPnt, 0L));

        if(aPnt != aDragStat.GetNow())
        {
            aDragStat.NextMove(aPnt);
            DBG_ASSERT(mpHelpLineOverlay, "SdrSnapView::MovDragHelpLine: no ImplHelpLineOverlay (!)");
            basegfx::B2DPoint aNewPos(aDragStat.GetNow().X(), aDragStat.GetNow().Y());
            mpHelpLineOverlay->SetPosition(aNewPos);
        }
    }
}

sal_Bool SdrSnapView::EndDragHelpLine()
{
    sal_Bool bRet(sal_False);

    if(IsDragHelpLine())
    {
        if(aDragStat.IsMinMoved())
        {
            SdrPageView* pPageView = mpHelpLineOverlay->GetPageView();

            if(pPageView)
            {
                // moved existing one
                Point aPnt(aDragStat.GetNow());
                const SdrHelpLineList& rHelpLines = pPageView->GetHelpLines();
                SdrHelpLine aChangedHelpLine = rHelpLines[mpHelpLineOverlay->GetHelpLineNumber()];
                aChangedHelpLine.SetPos(aPnt);
                pPageView->SetHelpLine(mpHelpLineOverlay->GetHelpLineNumber(), aChangedHelpLine);

                bRet = sal_True;
            }
            else
            {
                // create new one
                pPageView = GetSdrPageView();

                if(pPageView)
                {
                    Point aPnt(aDragStat.GetNow());
                    SdrHelpLine aNewHelpLine(mpHelpLineOverlay->GetHelpLineKind(), aPnt);
                    pPageView->InsertHelpLine(aNewHelpLine);

                    bRet = sal_True;
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
        mpHelpLineOverlay = 0L;
    }
}

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
