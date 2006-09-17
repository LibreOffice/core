/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdsnpv.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 06:00:45 $
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

#include "svdsnpv.hxx"
#include <math.h>

#include "svdetc.hxx"
#include "svdobj.hxx"
#include "svdpagv.hxx"
#include "svdpage.hxx"
#include "svditer.hxx"

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
    bSnapEnab=TRUE;
    bGridSnap=TRUE;
    bSnapTo1Pix=TRUE;
    bBordSnap=TRUE;
    bHlplSnap=TRUE;
    bOFrmSnap=TRUE;
    bOPntSnap=FALSE;
    bOConSnap=TRUE;
    bMoveMFrmSnap=TRUE;
    bMoveOFrmSnap=TRUE;
    bMoveOPntSnap=TRUE;
    bMoveOConSnap=TRUE;
    bMoveSnapOnlyTopLeft=FALSE;
    bSetPageOrg=FALSE;
    bOrtho=FALSE;
    bBigOrtho=TRUE;
    nSnapAngle=1500;
    bAngleSnapEnab=FALSE;
    bMoveOnlyDragging=FALSE;
    bSlantButShear=FALSE;
    bCrookNoContortion=FALSE;
    eCrookMode=SDRCROOK_ROTATE;
    bHlplFixed=FALSE;
    pDragHelpLinePV=NULL;
    nDragHelpLineNum=0;
    bDragHelpLine=FALSE;
    bEliminatePolyPoints=FALSE;
    nEliminatePolyPointLimitAngle=0;
}

SdrSnapView::SdrSnapView(SdrModel* pModel1, OutputDevice* pOut):
    SdrPaintView(pModel1,pOut)
{
    ClearVars();
}

SdrSnapView::SdrSnapView(SdrModel* pModel1, XOutputDevice* _pXOut):
    SdrPaintView(pModel1,_pXOut)
{
    ClearVars();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL SdrSnapView::IsAction() const
{
    return IsSetPageOrg() || IsDragHelpLine() || SdrPaintView::IsAction();
}

void SdrSnapView::MovAction(const Point& rPnt)
{
    aLastMausPosNoSnap=rPnt;
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

void SdrSnapView::ToggleShownXor(OutputDevice* pOut, const Region* pRegion) const
{
    SdrPaintView::ToggleShownXor(pOut,pRegion);
    if (IsSetPageOrg()) {
        DrawSetPageOrg(pOut);
    }
    if (IsDragHelpLine()) {
        DrawDragHelpLine(pOut);
    }
}

void SdrSnapView::SnapMove()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Point SdrSnapView::GetSnapPos(const Point& rPnt, const SdrPageView* pPV) const
{
    Point aPt(rPnt);
    SnapPos(aPt,pPV);
    return aPt;
}

#define NOT_SNAPPED 0x7FFFFFFF
USHORT SdrSnapView::SnapPos(Point& rPnt, const SdrPageView* pPV) const
{
    if (!bSnapEnab) return SDRSNAP_NOTSNAPPED;
    BOOL bPVOfs=FALSE;
    long x=rPnt.X();
    long y=rPnt.Y();
    long xOffs=0;
    long yOffs=0;
    if (pPV==NULL) {
        bPVOfs=TRUE;
        pPV=GetPageView(rPnt);
        if (pPV==NULL) return SDRSNAP_NOTSNAPPED;
    }
    bPVOfs=TRUE;
    // !!!!!!hier stattdessen mal die GridFrames verwenden!!!!!!!!
    // SdrPage::GetOffset() kann dann entsorgt werden.

    // #i3694#
    // The Page::GetOffset() method is not needed anymore, it even leads to errors.
    // I will completely remove it, only Writer implements it (with a constant
    // of (DOCUMENTBORDER, DOCUMENTBORDER ).
    // Point aWriterPageOffset(pPV->GetPage()->GetOffset());
    xOffs=pPV->GetOffset().X(); // +aWriterPageOffset.X();
    yOffs=pPV->GetOffset().Y(); // +aWriterPageOffset.Y();
    x-=xOffs;
    y-=yOffs;

    long dx=NOT_SNAPPED;
    long dy=NOT_SNAPPED;
    long dx1,dy1;
    long mx=aMagnSiz.Width();
    long my=aMagnSiz.Height();
    if (bHlplVisible && bHlplSnap && !bDragHelpLine) {
        const SdrHelpLineList& rHLL=pPV->GetHelpLines();
        USHORT nAnz=rHLL.GetCount();
        for (USHORT i=nAnz; i>0;) {
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
        ULONG nMaxPointSnapCount=200;
        ULONG nMaxFrameSnapCount=200;

        // #97981# go back to IM_DEEPNOGROUPS runthrough for snap to object comparisons
        // SdrObjListIter aIter(*pPV->GetPage(),IM_FLAT/*IM_DEEPNOGROUPS*/,TRUE);
        SdrObjListIter aIter(*pPV->GetPage(),/*IM_FLAT*/IM_DEEPNOGROUPS,TRUE);

        while (aIter.IsMore() && (nMaxPointSnapCount>0 || nMaxFrameSnapCount>0)) {
            SdrObject* pO=aIter.Next();
            Rectangle aRect(pO->GetCurrentBoundRect());
            aRect.Left  ()-=mx;
            aRect.Right ()+=mx;
            aRect.Top   ()-=my;
            aRect.Bottom()+=my;
            if (aRect.IsInside(rPnt)) {
                if (bOPntSnap && nMaxPointSnapCount>0) {
                    USHORT nAnz=pO->GetSnapPointCount();
                    for (USHORT i=0; i<nAnz && nMaxPointSnapCount>0; i++) {
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
    BOOL bRet=SDRSNAP_NOTSNAPPED;
    if (dx==NOT_SNAPPED) dx=0; else bRet|=SDRSNAP_XSNAPPED;
    if (dy==NOT_SNAPPED) dy=0; else bRet|=SDRSNAP_YSNAPPED;
    if (bPVOfs) {
        x+=xOffs;
        y+=yOffs;
    }
    rPnt.X()=x+dx;
    rPnt.Y()=y+dy;
    return bRet;
}

void SdrSnapView::CheckSnap(const Point& rPt, const SdrPageView* pPV,
    long& nBestXSnap, long& nBestYSnap,
    BOOL& bXSnapped, BOOL& bYSnapped) const
{
    Point aPt(rPt);
    USHORT nRet=SnapPos(aPt,pPV);
    aPt-=rPt;
    if ((nRet & SDRSNAP_XSNAPPED) !=0) {
        if (bXSnapped) {
            if (Abs(aPt.X())<Abs(nBestXSnap)) {
                nBestXSnap=aPt.X();
            }
        } else {
            nBestXSnap=aPt.X();
            bXSnapped=TRUE;
        }
    }
    if ((nRet & SDRSNAP_YSNAPPED) !=0) {
        if (bYSnapped) {
            if (Abs(aPt.Y())<Abs(nBestYSnap)) {
                nBestYSnap=aPt.Y();
            }
        } else {
            nBestYSnap=aPt.Y();
            bYSnapped=TRUE;
        }
    }
}

USHORT SdrSnapView::SnapRect(const Rectangle& rRect, const SdrPageView* pPV, long& rDX, long& rDY) const
{
    long nBestXSnap=0;
    long nBestYSnap=0;
    BOOL bXSnapped=FALSE;
    BOOL bYSnapped=FALSE;
    CheckSnap(rRect.TopLeft()    ,pPV,nBestXSnap,nBestYSnap,bXSnapped,bYSnapped);
    if (!bMoveSnapOnlyTopLeft) {
        CheckSnap(rRect.TopRight()   ,pPV,nBestXSnap,nBestYSnap,bXSnapped,bYSnapped);
        CheckSnap(rRect.BottomLeft() ,pPV,nBestXSnap,nBestYSnap,bXSnapped,bYSnapped);
        CheckSnap(rRect.BottomRight(),pPV,nBestXSnap,nBestYSnap,bXSnapped,bYSnapped);
    }
    rDX=nBestXSnap;
    rDY=nBestYSnap;
    USHORT nRet=0;
    if (bXSnapped) nRet+=SDRSNAP_XSNAPPED;
    if (bYSnapped) nRet+=SDRSNAP_YSNAPPED;
    return nRet;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL SdrSnapView::BegSetPageOrg(const Point& rPnt, OutputDevice* pOut, short nMinMov)
{
    BrkAction();
    aAni.Reset();
    aAni.SetCrossHair(TRUE);
    aDragStat.Reset(GetSnapPos(rPnt,NULL));
    aDragStat.SetMinMove(ImpGetMinMovLogic(nMinMov,pOut));
    if (nMinMov==0) aDragStat.SetMinMoved();
    pDragWin=pOut;
    bSetPageOrg=TRUE;
    if (aDragStat.IsMinMoved()) ShowSetPageOrg(pOut);
    return TRUE;
}

void SdrSnapView::MovSetPageOrg(const Point& rPnt)
{
    if (bSetPageOrg) {
        if (aDragStat.IsMinMoved()) HideSetPageOrg(pDragWin);
        aDragStat.NextMove(GetSnapPos(rPnt,NULL));
        if (aDragStat.CheckMinMoved(rPnt)) ShowSetPageOrg(pDragWin);
    }
}

BOOL SdrSnapView::EndSetPageOrg()
{
    BOOL bRet=FALSE;
    if (bSetPageOrg) {
        HideSetPageOrg(pDragWin);
        bSetPageOrg=FALSE;
        Point aPnt=aDragStat.GetNow();
        SdrPageView* pPV=HitPage(aPnt);

        // According to bug #99937# the page origin shall allways be set,
        // even when it lies outside the actual page area.  Therefore, the
        // first page is used as a fallback when no other is found.
        if (pPV == NULL)
            pPV = GetPageViewPvNum (0);

        if (pPV!=NULL) {
            aPnt-=pPV->GetOffset();
            pPV->SetPageOrigin(aPnt);
            bRet=TRUE;
        }
    }
    return bRet;
}

void SdrSnapView::BrkSetPageOrg()
{
    if (bSetPageOrg) {
        HideSetPageOrg(pDragWin);
        bSetPageOrg=FALSE;
    }
}

void SdrSnapView::ShowSetPageOrg(OutputDevice* pOut)
{
    if (bSetPageOrg && !aDragStat.IsShown()) {
        DrawSetPageOrg(pOut);
        aDragStat.SetShown(TRUE);
        aAni.Start();
    }
}

void SdrSnapView::HideSetPageOrg(OutputDevice* pOut)
{
    if (bSetPageOrg && aDragStat.IsShown()) {
        aAni.Stop();
        DrawSetPageOrg(pOut);
        aDragStat.SetShown(FALSE);
    }
}

void SdrSnapView::DrawSetPageOrg(OutputDevice* pOut) const
{
    if (bSetPageOrg) {
        aAni.SetP1(aDragStat.GetNow());
        aAni.Invert(pOut);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL SdrSnapView::PickHelpLine(const Point& rPnt, short nTol, const OutputDevice& rOut, USHORT& rnHelpLineNum, SdrPageView*& rpPV) const
{
    rpPV=NULL;
    nTol=ImpGetHitTolLogic(nTol,&rOut);
    for (USHORT nv=GetPageViewCount(); nv>0;) {
        nv--;
        SdrPageView* pPV=GetPageViewPvNum(nv);
        Point aPnt(rPnt);
        aPnt-=pPV->GetOffset();
        USHORT nIndex=pPV->GetHelpLines().HitTest(aPnt,USHORT(nTol),rOut);
        if (nIndex!=SDRHELPLINE_NOTFOUND) {
            rpPV=pPV;
            rnHelpLineNum=nIndex;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL SdrSnapView::BegDragHelpLine(USHORT nHelpLineNum, SdrPageView* pPV, OutputDevice* pOut, short nMinMov)
{
    BOOL bRet=FALSE;
    if (bHlplFixed) return FALSE;
    BrkAction();
    if (pPV!=NULL && nHelpLineNum<pPV->GetHelpLines().GetCount()) {
        pDragHelpLinePV=pPV;
        nDragHelpLineNum=nHelpLineNum;
        aDragHelpLine=pPV->GetHelpLines()[nHelpLineNum];
        Point aPnt(aDragHelpLine.GetPos());
        aPnt+=pPV->GetOffset();
        aDragHelpLine.SetPos(aPnt);
        pDragWin=pOut;
        aDragStat.Reset(GetSnapPos(aPnt,pPV));
        aDragStat.SetMinMove(ImpGetMinMovLogic(nMinMov,pOut));
        if (nMinMov==0) aDragStat.SetMinMoved();
        bDragHelpLine=TRUE;
        if (aDragStat.IsMinMoved()) ShowDragHelpLine(pOut);
        bRet=TRUE;
    }
    return bRet;
}

BOOL SdrSnapView::BegDragHelpLine(const Point& rPnt, SdrHelpLineKind eNewKind, OutputDevice* pOut, short nMinMov)
{
    BOOL bRet=FALSE;
    BrkAction();
    if (GetPageViewCount()>0) {
        pDragHelpLinePV=NULL;
        nDragHelpLineNum=0xFFFF;
        aDragStat.Reset(GetSnapPos(rPnt,NULL));
        aDragStat.SetMinMove(ImpGetMinMovLogic(nMinMov,pOut));
        if (nMinMov==0) aDragStat.SetMinMoved();
        aDragHelpLine.SetPos(aDragStat.GetNow());
        aDragHelpLine.SetKind(eNewKind);
        pDragWin=pOut;
        bDragHelpLine=TRUE;
        if (aDragStat.IsMinMoved()) ShowDragHelpLine(pOut);
        bRet=TRUE;
    }
    return bRet;
}

void SdrSnapView::SetDraggedHelpLineKind(SdrHelpLineKind eNewKind)
{
    if (bDragHelpLine) {
        //BOOL bVis=aDragStat.IsShown();
        HideDragHelpLine(pDragWin);
        aDragHelpLine.SetKind(eNewKind);
        aDragStat.SetMinMoved();
        ShowDragHelpLine(pDragWin);
    }
}

void SdrSnapView::MovDragHelpLine(const Point& rPnt)
{
    if (bDragHelpLine && aDragStat.CheckMinMoved(rPnt)) {
        Point aPnt(GetSnapPos(rPnt,NULL));
        if (aPnt!=aDragStat.GetNow()) {
            if (aDragStat.IsMinMoved()) HideDragHelpLine(pDragWin);
            aDragStat.NextMove(aPnt);
            aDragHelpLine.SetPos(aDragStat.GetNow());
            if (aDragStat.CheckMinMoved(rPnt)) ShowDragHelpLine(pDragWin);
        }
    }
}

BOOL SdrSnapView::EndDragHelpLine()
{
    BOOL bRet=FALSE;
    if (bDragHelpLine) {
        if (aDragStat.IsMinMoved()) {
            HideDragHelpLine(pDragWin);
            if (pDragHelpLinePV!=NULL) {
                Point aPnt(aDragStat.GetNow());
                aPnt-=pDragHelpLinePV->GetOffset(); // an die PageView anpassen
                aDragHelpLine.SetPos(aPnt);
                pDragHelpLinePV->SetHelpLine(nDragHelpLineNum,aDragHelpLine);
                bRet=TRUE;
            } else { // ansonsten neue Hilfslinie
                Point aPnt(aDragStat.GetNow());
                SdrPageView* pPV=GetPageView(aPnt);
                if (pPV!=NULL) {
                    aPnt-=pPV->GetOffset(); // an die PageView anpassen
                    aDragHelpLine.SetPos(aPnt);
                    pDragHelpLinePV=pPV;
                    nDragHelpLineNum=pPV->GetHelpLines().GetCount();
                    pPV->InsertHelpLine(aDragHelpLine);
                    bRet=TRUE;
                }
            }
            bDragHelpLine=FALSE;
        } else {
            BrkDragHelpLine();
        }
    }
    return bRet;
}

void SdrSnapView::BrkDragHelpLine()
{
    if (bDragHelpLine) {
        HideDragHelpLine(pDragWin);
        bDragHelpLine=FALSE;
        pDragHelpLinePV=NULL;
    }
}

void SdrSnapView::ShowDragHelpLine(OutputDevice* pOut)
{
    if (bDragHelpLine && !aDragStat.IsShown()) {
        DrawDragHelpLine(pOut);
        aDragStat.SetShown(TRUE);
    }
}

void SdrSnapView::HideDragHelpLine(OutputDevice* pOut)
{
    if (bDragHelpLine && aDragStat.IsShown()) {
        DrawDragHelpLine(pOut);
        aDragStat.SetShown(FALSE);
    }
}

void SdrSnapView::DrawDragHelpLine(OutputDevice* pOut) const
{
    if (bDragHelpLine)
    {
        USHORT i=0;
        do
        {
            OutputDevice* pO=pOut;
            if (pO==NULL)
            {
                pO=GetWin(i);
                i++;
            }
            if (pO!=NULL)
            {
                bool bDontDraw = false;

                // see if there is already a help line on same position
                Point aPnt(aDragStat.GetNow());
                SdrPageView* pPV=GetPageView(aPnt);
                if (pPV!=NULL)
                {
                    const SdrHelpLineList& rList = pPV->GetHelpLines();
                    sal_uInt16 nAnz = rList.GetCount(),j;

                    for(j=0; j<nAnz; j++)
                    {
                        const SdrHelpLine rHelpLine = rList[j];

                        // check if we already drawn a help line like this one
                        if( aDragHelpLine.IsVisibleEqual( rHelpLine, *pO) )
                            bDontDraw = true;
                    }
                }

                if( !bDontDraw )
                {
                    RasterOp eRop0=pO->GetRasterOp();
                    pO->SetRasterOp(ROP_INVERT);
                    Color aColor0( pO->GetLineColor() );
                    Color aBlackColor( COL_BLACK );
                    pO->SetLineColor( aBlackColor );
                    aDragHelpLine.Draw(*pO,Point());
                    pO->SetRasterOp(eRop0);
                    pO->SetLineColor( aColor0 );
                }
            }
        } while (pOut==NULL && i<GetWinCount());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

//BFS01void SdrSnapView::WriteRecords(SvStream& rOut) const
//BFS01{
//BFS01 SdrPaintView::WriteRecords(rOut);
//BFS01 {
//BFS01     SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWSNAP);
//BFS01     rOut<<aMagnSiz;
//BFS01     rOut<<aSnapSiz;
//BFS01
//BFS01     rOut << (BOOL)bSnapEnab;
//BFS01     rOut << (BOOL)bGridSnap;
//BFS01     rOut << (BOOL)bBordSnap;
//BFS01     rOut << (BOOL)bHlplSnap;
//BFS01     rOut << (BOOL)bOFrmSnap;
//BFS01     rOut << (BOOL)bOPntSnap;
//BFS01     rOut << (BOOL)bOConSnap;
//BFS01     rOut << (BOOL)bMoveMFrmSnap;
//BFS01     rOut << (BOOL)bMoveOFrmSnap;
//BFS01     rOut << (BOOL)bMoveOPntSnap;
//BFS01     rOut << (BOOL)bMoveOConSnap;
//BFS01     rOut << (BOOL)bHlplFixed;
//BFS01
//BFS01     rOut<<nMagnSizPix;
//BFS01
//BFS01     rOut << (BOOL)bSnapTo1Pix;
//BFS01     rOut << (BOOL)bMoveSnapOnlyTopLeft;
//BFS01
//BFS01     rOut<<aSnapWdtX;
//BFS01     rOut<<aSnapWdtY;
//BFS01 } {
//BFS01     SdrNamedSubRecord aSubRecord(rOut,STREAM_WRITE,SdrInventor,SDRIORECNAME_VIEWORTHO);
//BFS01
//BFS01     rOut << (BOOL)bOrtho;
//BFS01     rOut << (BOOL)bBigOrtho;
//BFS01
//BFS01     rOut<<nSnapAngle;
//BFS01
//BFS01     rOut << (BOOL)bAngleSnapEnab;
//BFS01     rOut << (BOOL)bMoveOnlyDragging;
//BFS01     rOut << (BOOL)bSlantButShear;
//BFS01     rOut << (BOOL)bCrookNoContortion;
//BFS01
//BFS01     rOut<<USHORT(eCrookMode);
//BFS01 }
//BFS01}

//BFS01BOOL SdrSnapView::ReadRecord(const SdrIOHeader& rViewHead,
//BFS01 const SdrNamedSubRecord& rSubHead,
//BFS01 SvStream& rIn)
//BFS01{
//BFS01 BOOL bRet=FALSE;
//BFS01 if (rSubHead.GetInventor()==SdrInventor)
//BFS01 {
//BFS01     bRet=TRUE;
//BFS01     BOOL bZwi;
//BFS01
//BFS01     switch (rSubHead.GetIdentifier())
//BFS01     {
//BFS01         case SDRIORECNAME_VIEWSNAP:
//BFS01         {
//BFS01             rIn>>aMagnSiz;
//BFS01             rIn>>aSnapSiz;
//BFS01
//BFS01             rIn >> bZwi; bSnapEnab = bZwi;
//BFS01             rIn >> bZwi; bGridSnap = bZwi;
//BFS01             rIn >> bZwi; bBordSnap = bZwi;
//BFS01             rIn >> bZwi; bHlplSnap = bZwi;
//BFS01             rIn >> bZwi; bOFrmSnap = bZwi;
//BFS01             rIn >> bZwi; bOPntSnap = bZwi;
//BFS01             rIn >> bZwi; bOConSnap = bZwi;
//BFS01             rIn >> bZwi; bMoveMFrmSnap = bZwi;
//BFS01             rIn >> bZwi; bMoveOFrmSnap = bZwi;
//BFS01             rIn >> bZwi; bMoveOPntSnap = bZwi;
//BFS01             rIn >> bZwi; bMoveOConSnap = bZwi;
//BFS01
//BFS01             if (rSubHead.GetBytesLeft()>0)
//BFS01             {
//BFS01                 rIn >> bZwi; bHlplFixed = bZwi; // kam erst spaeter dazu
//BFS01             }
//BFS01             if (rSubHead.GetBytesLeft()>0)
//BFS01             {
//BFS01                 // kam erst spaeter dazu
//BFS01                 rIn>>nMagnSizPix;
//BFS01                 rIn >> bZwi; bSnapTo1Pix = bZwi;
//BFS01             }
//BFS01             if (rSubHead.GetBytesLeft()>0)
//BFS01             {
//BFS01                 // kam erst spaeter dazu
//BFS01                 rIn >> bZwi; bMoveSnapOnlyTopLeft = bZwi;
//BFS01             }
//BFS01             if (rSubHead.GetBytesLeft()>0)
//BFS01             {
//BFS01                 // kam erst spaeter dazu
//BFS01                 rIn>>aSnapWdtX;
//BFS01                 rIn>>aSnapWdtY;
//BFS01             }
//BFS01         } break;
//BFS01         case SDRIORECNAME_VIEWORTHO:
//BFS01         {
//BFS01             rIn >> bZwi; bOrtho = bZwi;
//BFS01             rIn >> bZwi; bBigOrtho = bZwi;
//BFS01
//BFS01             rIn>>nSnapAngle;
//BFS01
//BFS01             if (rSubHead.GetBytesLeft()>0)
//BFS01             {
//BFS01                 // kam erst spaeter dazu
//BFS01                 rIn >> bZwi; bAngleSnapEnab = bZwi;
//BFS01                 rIn >> bZwi; bMoveOnlyDragging = bZwi;
//BFS01                 rIn >> bZwi; bSlantButShear = bZwi;
//BFS01                 rIn >> bZwi; bCrookNoContortion = bZwi;
//BFS01             }
//BFS01             if (rSubHead.GetBytesLeft()>0)
//BFS01             {
//BFS01                 // kam erst spaeter dazu
//BFS01                 USHORT nCrookMode;
//BFS01                 rIn>>nCrookMode;
//BFS01                 eCrookMode=SdrCrookMode(nCrookMode);
//BFS01             }
//BFS01         } break;
//BFS01         default: bRet=FALSE;
//BFS01     }
//BFS01 }
//BFS01 if (!bRet) bRet=SdrPaintView::ReadRecord(rViewHead,rSubHead,rIn);
//BFS01 return bRet;
//BFS01}

