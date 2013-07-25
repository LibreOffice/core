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


#include <tools/bigint.hxx>
#include <tools/helpers.hxx>
#include <svx/svdopath.hxx>
#include <math.h>
#include <svx/xpool.hxx>
#include <svx/xpoly.hxx>
#include <svx/svdattr.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdetc.hxx>
#include <svx/svddrag.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdhdl.hxx>
#include <svx/svdview.hxx>  // for MovCreate when using curves
#include "svx/svdglob.hxx"  // Stringcache
#include "svx/svdstr.hrc"   // the object's name

#include <svx/xlnwtit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xflclit.hxx>
#include <svx/svdogrp.hxx>
#include <svx/polypolygoneditor.hxx>
#include <svx/xlntrit.hxx>
#include <svx/sdr/contact/viewcontactofsdrpathobj.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include "svdconv.hxx"
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <svx/sdr/attribute/sdrtextattribute.hxx>
#include <svx/sdr/primitive2d/sdrattributecreator.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/sdr/attribute/sdrformtextattribute.hxx>

using namespace sdr;

inline sal_uInt16 GetPrevPnt(sal_uInt16 nPnt, sal_uInt16 nPntMax, bool bClosed)
{
    if (nPnt>0) {
        nPnt--;
    } else {
        nPnt=nPntMax;
        if (bClosed) nPnt--;
    }
    return nPnt;
}

inline sal_uInt16 GetNextPnt(sal_uInt16 nPnt, sal_uInt16 nPntMax, bool bClosed)
{
    nPnt++;
    if (nPnt>nPntMax || (bClosed && nPnt>=nPntMax)) nPnt=0;
    return nPnt;
}

struct ImpSdrPathDragData  : public SdrDragStatUserData
{
    XPolygon                    aXP;            // section of the original polygon
    bool                        bValid;         // FALSE = too few points
    bool                        bClosed;        // closed object?
    sal_uInt16                      nPoly;          // number of the polygon in the PolyPolygon
    sal_uInt16                      nPnt;           // number of point in the above polygon
    sal_uInt16                      nPntAnz;        // number of points of the polygon
    sal_uInt16                      nPntMax;        // maximum index
    bool                        bBegPnt;        // dragged point is first point of a Polyline
    bool                        bEndPnt;        // dragged point is finishing point of a Polyline
    sal_uInt16                      nPrevPnt;       // index of previous point
    sal_uInt16                      nNextPnt;       // index of next point
    bool                        bPrevIsBegPnt;  // previous point is first point of a Polyline
    bool                        bNextIsEndPnt;  // next point is first point of a Polyline
    sal_uInt16                      nPrevPrevPnt;   // index of point before previous point
    sal_uInt16                      nNextNextPnt;   // index of point after next point
    bool                        bControl;       // point is a control point
    bool                        bIsPrevControl; // point is a control point before a support point
    bool                        bIsNextControl; // point is a control point after a support point
    bool                        bPrevIsControl; // if nPnt is a support point: a control point comes before
    bool                        bNextIsControl; // if nPnt is a support point: a control point comes after
    sal_uInt16                      nPrevPrevPnt0;
    sal_uInt16                      nPrevPnt0;
    sal_uInt16                      nPnt0;
    sal_uInt16                      nNextPnt0;
    sal_uInt16                      nNextNextPnt0;
    bool                        bEliminate;     // delete point? (is set by MovDrag)

    bool                        mbMultiPointDrag;
    const XPolyPolygon          maOrig;
    XPolyPolygon                maMove;
    std::vector<SdrHdl*>        maHandles;

public:
    ImpSdrPathDragData(const SdrPathObj& rPO, const SdrHdl& rHdl, bool bMuPoDr, const SdrDragStat& rDrag);
    void ResetPoly(const SdrPathObj& rPO);
    bool IsMultiPointDrag() const { return mbMultiPointDrag; }
};

ImpSdrPathDragData::ImpSdrPathDragData(const SdrPathObj& rPO, const SdrHdl& rHdl, bool bMuPoDr, const SdrDragStat& rDrag)
:   aXP(5),
    mbMultiPointDrag(bMuPoDr),
    maOrig(rPO.GetPathPoly()),
    maHandles(0)
{
    if(mbMultiPointDrag)
    {
        const SdrMarkView& rMarkView = *rDrag.GetView();
        const SdrHdlList& rHdlList = rMarkView.GetHdlList();
        const sal_uInt32 nHdlCount = rHdlList.GetHdlCount();
        const SdrObject* pInteractionObject(nHdlCount && rHdlList.GetHdl(0) ? rHdlList.GetHdl(0)->GetObj() : 0);

        for(sal_uInt32 a(0); a < nHdlCount; a++)
        {
            SdrHdl* pTestHdl = rHdlList.GetHdl(a);

            if(pTestHdl && pTestHdl->IsSelected() && pTestHdl->GetObj() == pInteractionObject)
            {
                maHandles.push_back(pTestHdl);
            }
        }

        maMove = maOrig;
        bValid = sal_True;
    }
    else
    {
        bValid=sal_False;
        bClosed=rPO.IsClosed();          // closed object?
        nPoly=(sal_uInt16)rHdl.GetPolyNum();            // number of the polygon in the PolyPolygon
        nPnt=(sal_uInt16)rHdl.GetPointNum();            // number of points in the above polygon
        const XPolygon aTmpXP(rPO.GetPathPoly().getB2DPolygon(nPoly));
        nPntAnz=aTmpXP.GetPointCount();        // number of point of the polygon
        if (nPntAnz==0 || (bClosed && nPntAnz==1)) return; // minimum of 1 points for Lines, minimum of 2 points for Polygon
        nPntMax=nPntAnz-1;                  // maximum index
        bBegPnt=!bClosed && nPnt==0;        // dragged point is first point of a Polyline
        bEndPnt=!bClosed && nPnt==nPntMax;  // dragged point is finishing point of a Polyline
        if (bClosed && nPntAnz<=3) {        // if polygon is only a line
            bBegPnt=(nPntAnz<3) || nPnt==0;
            bEndPnt=(nPntAnz<3) || nPnt==nPntMax-1;
        }
        nPrevPnt=nPnt;                      // index of previous point
        nNextPnt=nPnt;                      // index of next point
        if (!bBegPnt) nPrevPnt=GetPrevPnt(nPnt,nPntMax,bClosed);
        if (!bEndPnt) nNextPnt=GetNextPnt(nPnt,nPntMax,bClosed);
        bPrevIsBegPnt=bBegPnt || (!bClosed && nPrevPnt==0);
        bNextIsEndPnt=bEndPnt || (!bClosed && nNextPnt==nPntMax);
        nPrevPrevPnt=nPnt;                  // index of point before previous point
        nNextNextPnt=nPnt;                  // index of point after next point
        if (!bPrevIsBegPnt) nPrevPrevPnt=GetPrevPnt(nPrevPnt,nPntMax,bClosed);
        if (!bNextIsEndPnt) nNextNextPnt=GetNextPnt(nNextPnt,nPntMax,bClosed);
        bControl=rHdl.IsPlusHdl();          // point is a control point
        bIsPrevControl=sal_False;               // point is a control point before a support point
        bIsNextControl=sal_False;               // point is a control point after a support point
        bPrevIsControl=sal_False;               // if nPnt is a support point: a control point comes before
        bNextIsControl=sal_False;               // if nPnt is a support point: a control point comes after
        if (bControl) {
            bIsPrevControl=aTmpXP.IsControl(nPrevPnt);
            bIsNextControl=!bIsPrevControl;
        } else {
            bPrevIsControl=!bBegPnt && !bPrevIsBegPnt && aTmpXP.GetFlags(nPrevPnt)==XPOLY_CONTROL;
            bNextIsControl=!bEndPnt && !bNextIsEndPnt && aTmpXP.GetFlags(nNextPnt)==XPOLY_CONTROL;
        }
        nPrevPrevPnt0=nPrevPrevPnt;
        nPrevPnt0    =nPrevPnt;
        nPnt0        =nPnt;
        nNextPnt0    =nNextPnt;
        nNextNextPnt0=nNextNextPnt;
        nPrevPrevPnt=0;
        nPrevPnt=1;
        nPnt=2;
        nNextPnt=3;
        nNextNextPnt=4;
        bEliminate=sal_False;
        ResetPoly(rPO);
        bValid=sal_True;
    }
}

void ImpSdrPathDragData::ResetPoly(const SdrPathObj& rPO)
{
    const XPolygon aTmpXP(rPO.GetPathPoly().getB2DPolygon(nPoly));
    aXP[0]=aTmpXP[nPrevPrevPnt0];  aXP.SetFlags(0,aTmpXP.GetFlags(nPrevPrevPnt0));
    aXP[1]=aTmpXP[nPrevPnt0];      aXP.SetFlags(1,aTmpXP.GetFlags(nPrevPnt0));
    aXP[2]=aTmpXP[nPnt0];          aXP.SetFlags(2,aTmpXP.GetFlags(nPnt0));
    aXP[3]=aTmpXP[nNextPnt0];      aXP.SetFlags(3,aTmpXP.GetFlags(nNextPnt0));
    aXP[4]=aTmpXP[nNextNextPnt0];  aXP.SetFlags(4,aTmpXP.GetFlags(nNextNextPnt0));
}

/*************************************************************************/

struct ImpPathCreateUser  : public SdrDragStatUserData
{
    Point                   aBezControl0;
    Point                   aBezStart;
    Point                   aBezCtrl1;
    Point                   aBezCtrl2;
    Point                   aBezEnd;
    Point                   aCircStart;
    Point                   aCircEnd;
    Point                   aCircCenter;
    Point                   aLineStart;
    Point                   aLineEnd;
    Point                   aRectP1;
    Point                   aRectP2;
    Point                   aRectP3;
    long                    nCircRadius;
    long                    nCircStWink;
    long                    nCircRelWink;
    bool                    bBezier;
    bool                    bBezHasCtrl0;
    bool                    bCurve;
    bool                    bCircle;
    bool                    bAngleSnap;
    bool                    bLine;
    bool                    bLine90;
    bool                    bRect;
    bool                    bMixedCreate;
    sal_uInt16                  nBezierStartPoint;
    SdrObjKind              eStartKind;
    SdrObjKind              eAktKind;

public:
    ImpPathCreateUser(): nCircRadius(0),nCircStWink(0),nCircRelWink(0),
        bBezier(sal_False),bBezHasCtrl0(sal_False),bCurve(sal_False),bCircle(sal_False),bAngleSnap(sal_False),bLine(sal_False),bLine90(sal_False),bRect(sal_False),
        bMixedCreate(sal_False),nBezierStartPoint(0),eStartKind(OBJ_NONE),eAktKind(OBJ_NONE) { }

    void ResetFormFlags() { bBezier=sal_False; bCurve=sal_False; bCircle=sal_False; bLine=sal_False; bRect=sal_False; }
    bool IsFormFlag() const { return bBezier || bCurve || bCircle || bLine || bRect; }
    XPolygon GetFormPoly() const;
    bool CalcBezier(const Point& rP1, const Point& rP2, const Point& rDir, bool bMouseDown);
    XPolygon GetBezierPoly() const;
    XPolygon GetCurvePoly() const { return XPolygon(); }
    bool CalcCircle(const Point& rP1, const Point& rP2, const Point& rDir, SdrView* pView);
    XPolygon GetCirclePoly() const;
    bool CalcLine(const Point& rP1, const Point& rP2, const Point& rDir, SdrView* pView);
    Point    CalcLine(const Point& rCsr, long nDirX, long nDirY, SdrView* pView) const;
    XPolygon GetLinePoly() const;
    bool CalcRect(const Point& rP1, const Point& rP2, const Point& rDir, SdrView* pView);
    XPolygon GetRectPoly() const;
};

XPolygon ImpPathCreateUser::GetFormPoly() const
{
    if (bBezier) return GetBezierPoly();
    if (bCurve)  return GetCurvePoly();
    if (bCircle) return GetCirclePoly();
    if (bLine)   return GetLinePoly();
    if (bRect)   return GetRectPoly();
    return XPolygon();
}

bool ImpPathCreateUser::CalcBezier(const Point& rP1, const Point& rP2, const Point& rDir, bool bMouseDown)
{
    bool bRet = true;
    aBezStart=rP1;
    aBezCtrl1=rP1+rDir;
    aBezCtrl2=rP2;

    // #i21479#
    // Also copy the end point when no end point is set yet
    if (!bMouseDown || (0L == aBezEnd.X() && 0L == aBezEnd.Y())) aBezEnd=rP2;

    bBezier=bRet;
    return bRet;
}

XPolygon ImpPathCreateUser::GetBezierPoly() const
{
    XPolygon aXP(4);
    aXP[0]=aBezStart; aXP.SetFlags(0,XPOLY_SMOOTH);
    aXP[1]=aBezCtrl1; aXP.SetFlags(1,XPOLY_CONTROL);
    aXP[2]=aBezCtrl2; aXP.SetFlags(2,XPOLY_CONTROL);
    aXP[3]=aBezEnd;
    return aXP;
}

bool ImpPathCreateUser::CalcCircle(const Point& rP1, const Point& rP2, const Point& rDir, SdrView* pView)
{
    long nTangAngle=GetAngle(rDir);
    aCircStart=rP1;
    aCircEnd=rP2;
    aCircCenter=rP1;
    long dx=rP2.X()-rP1.X();
    long dy=rP2.Y()-rP1.Y();
    long dAngle=GetAngle(Point(dx,dy))-nTangAngle;
    dAngle=NormAngle360(dAngle);
    long nTmpAngle=NormAngle360(9000-dAngle);
    bool bRet=nTmpAngle!=9000 && nTmpAngle!=27000;
    long nRad=0;
    if (bRet) {
        double cs=cos(nTmpAngle*nPi180);
        double nR=(double)GetLen(Point(dx,dy))/cs/2;
        nRad=std::abs(Round(nR));
    }
    if (dAngle<18000) {
        nCircStWink=NormAngle360(nTangAngle-9000);
        nCircRelWink=NormAngle360(2*dAngle);
        aCircCenter.X()+=Round(nRad*cos((nTangAngle+9000)*nPi180));
        aCircCenter.Y()-=Round(nRad*sin((nTangAngle+9000)*nPi180));
    } else {
        nCircStWink=NormAngle360(nTangAngle+9000);
        nCircRelWink=-NormAngle360(36000-2*dAngle);
        aCircCenter.X()+=Round(nRad*cos((nTangAngle-9000)*nPi180));
        aCircCenter.Y()-=Round(nRad*sin((nTangAngle-9000)*nPi180));
    }
    bAngleSnap=pView!=NULL && pView->IsAngleSnapEnabled();
    if (bAngleSnap) {
        long nSA=pView->GetSnapAngle();
        if (nSA!=0) { // angle snapping
            bool bNeg=nCircRelWink<0;
            if (bNeg) nCircRelWink=-nCircRelWink;
            nCircRelWink+=nSA/2;
            nCircRelWink/=nSA;
            nCircRelWink*=nSA;
            nCircRelWink=NormAngle360(nCircRelWink);
            if (bNeg) nCircRelWink=-nCircRelWink;
        }
    }
    nCircRadius=nRad;
    if (nRad==0 || std::abs(nCircRelWink)<5) bRet=sal_False;
    bCircle=bRet;
    return bRet;
}

XPolygon ImpPathCreateUser::GetCirclePoly() const
{
    if (nCircRelWink>=0) {
        XPolygon aXP(aCircCenter,nCircRadius,nCircRadius,
                     sal_uInt16((nCircStWink+5)/10),sal_uInt16((nCircStWink+nCircRelWink+5)/10),sal_False);
        aXP[0]=aCircStart; aXP.SetFlags(0,XPOLY_SMOOTH);
        if (!bAngleSnap) aXP[aXP.GetPointCount()-1]=aCircEnd;
        return aXP;
    } else {
        XPolygon aXP(aCircCenter,nCircRadius,nCircRadius,
                     sal_uInt16(NormAngle360(nCircStWink+nCircRelWink+5)/10),sal_uInt16((nCircStWink+5)/10),sal_False);
        sal_uInt16 nAnz=aXP.GetPointCount();
        for (sal_uInt16 nNum=nAnz/2; nNum>0;) {
            nNum--; // reverse XPoly's order of points
            sal_uInt16 n2=nAnz-nNum-1;
            Point aPt(aXP[nNum]);
            aXP[nNum]=aXP[n2];
            aXP[n2]=aPt;
        }
        aXP[0]=aCircStart; aXP.SetFlags(0,XPOLY_SMOOTH);
        if (!bAngleSnap) aXP[aXP.GetPointCount()-1]=aCircEnd;
        return aXP;
    }
}

Point ImpPathCreateUser::CalcLine(const Point& aCsr, long nDirX, long nDirY, SdrView* pView) const
{
    long x=aCsr.X(),x1=x,x2=x;
    long y=aCsr.Y(),y1=y,y2=y;
    bool bHLin=nDirY==0;
    bool bVLin=nDirX==0;
    if (bHLin) y=0;
    else if (bVLin) x=0;
    else {
        x1=BigMulDiv(y,nDirX,nDirY);
        y2=BigMulDiv(x,nDirY,nDirX);
        long l1=std::abs(x1)+std::abs(y1);
        long l2=std::abs(x2)+std::abs(y2);
        if ((l1<=l2) != (pView!=NULL && pView->IsBigOrtho())) {
            x=x1; y=y1;
        } else {
            x=x2; y=y2;
        }
    }
    return Point(x,y);
}

bool ImpPathCreateUser::CalcLine(const Point& rP1, const Point& rP2, const Point& rDir, SdrView* pView)
{
    aLineStart=rP1;
    aLineEnd=rP2;
    bLine90=sal_False;
    if (rP1==rP2 || (rDir.X()==0 && rDir.Y()==0)) { bLine=sal_False; return sal_False; }
    Point aTmpPt(rP2-rP1);
    long nDirX=rDir.X();
    long nDirY=rDir.Y();
    Point aP1(CalcLine(aTmpPt, nDirX, nDirY,pView)); aP1-=aTmpPt; long nQ1=std::abs(aP1.X())+std::abs(aP1.Y());
    Point aP2(CalcLine(aTmpPt, nDirY,-nDirX,pView)); aP2-=aTmpPt; long nQ2=std::abs(aP2.X())+std::abs(aP2.Y());
    if (pView!=NULL && pView->IsOrtho()) nQ1=0; // Ortho turns off at right angle
    bLine90=nQ1>2*nQ2;
    if (!bLine90) { // smooth transition
        aLineEnd+=aP1;
    } else {          // rectangular transition
        aLineEnd+=aP2;
    }
    bLine=sal_True;
    return sal_True;
}

XPolygon ImpPathCreateUser::GetLinePoly() const
{
    XPolygon aXP(2);
    aXP[0]=aLineStart; if (!bLine90) aXP.SetFlags(0,XPOLY_SMOOTH);
    aXP[1]=aLineEnd;
    return aXP;
}

bool ImpPathCreateUser::CalcRect(const Point& rP1, const Point& rP2, const Point& rDir, SdrView* pView)
{
    aRectP1=rP1;
    aRectP2=rP1;
    aRectP3=rP2;
    if (rP1==rP2 || (rDir.X()==0 && rDir.Y()==0)) { bRect=sal_False; return sal_False; }
    Point aTmpPt(rP2-rP1);
    long nDirX=rDir.X();
    long nDirY=rDir.Y();
    long x=aTmpPt.X();
    long y=aTmpPt.Y();
    bool bHLin=nDirY==0;
    bool bVLin=nDirX==0;
    if (bHLin) y=0;
    else if (bVLin) x=0;
    else {
        y=BigMulDiv(x,nDirY,nDirX);
        long nHypLen=aTmpPt.Y()-y;
        long nTangAngle=-GetAngle(rDir);
        // sin=g/h, g=h*sin
        double a=nTangAngle*nPi180;
        double sn=sin(a);
        double cs=cos(a);
        double nGKathLen=nHypLen*sn;
        y+=Round(nGKathLen*sn);
        x+=Round(nGKathLen*cs);
    }
    aRectP2.X()+=x;
    aRectP2.Y()+=y;
    if (pView!=NULL && pView->IsOrtho()) {
        long dx1=aRectP2.X()-aRectP1.X(); long dx1a=std::abs(dx1);
        long dy1=aRectP2.Y()-aRectP1.Y(); long dy1a=std::abs(dy1);
        long dx2=aRectP3.X()-aRectP2.X(); long dx2a=std::abs(dx2);
        long dy2=aRectP3.Y()-aRectP2.Y(); long dy2a=std::abs(dy2);
        bool b1MoreThan2=dx1a+dy1a>dx2a+dy2a;
        if (b1MoreThan2 != pView->IsBigOrtho()) {
            long xtemp=dy2a-dx1a; if (dx1<0) xtemp=-xtemp;
            long ytemp=dx2a-dy1a; if (dy1<0) ytemp=-ytemp;
            aRectP2.X()+=xtemp;
            aRectP2.Y()+=ytemp;
            aRectP3.X()+=xtemp;
            aRectP3.Y()+=ytemp;
        } else {
            long xtemp=dy1a-dx2a; if (dx2<0) xtemp=-xtemp;
            long ytemp=dx1a-dy2a; if (dy2<0) ytemp=-ytemp;
            aRectP3.X()+=xtemp;
            aRectP3.Y()+=ytemp;
        }
    }
    bRect=sal_True;
    return sal_True;
}

XPolygon ImpPathCreateUser::GetRectPoly() const
{
    XPolygon aXP(3);
    aXP[0]=aRectP1; aXP.SetFlags(0,XPOLY_SMOOTH);
    aXP[1]=aRectP2;
    if (aRectP3!=aRectP2) aXP[2]=aRectP3;
    return aXP;
}

/*************************************************************************/

class ImpPathForDragAndCreate
{
    SdrPathObj&                 mrSdrPathObject;
    XPolyPolygon                aPathPolygon;
    SdrObjKind                  meObjectKind;
    ImpSdrPathDragData*         mpSdrPathDragData;
    bool                        mbCreating;

public:
    ImpPathForDragAndCreate(SdrPathObj& rSdrPathObject);
    ~ImpPathForDragAndCreate();

    // drag stuff
    bool beginPathDrag( SdrDragStat& rDrag )  const;
    bool movePathDrag( SdrDragStat& rDrag ) const;
    bool endPathDrag( SdrDragStat& rDrag );
    String getSpecialDragComment(const SdrDragStat& rDrag) const;
    basegfx::B2DPolyPolygon getSpecialDragPoly(const SdrDragStat& rDrag) const;

    // create stuff
    bool BegCreate(SdrDragStat& rStat);
    bool MovCreate(SdrDragStat& rStat);
    bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);
    bool BckCreate(SdrDragStat& rStat);
    void BrkCreate(SdrDragStat& rStat);
    Pointer GetCreatePointer() const;

    // helping stuff
    bool IsClosed(SdrObjKind eKind) const { return eKind==OBJ_POLY || eKind==OBJ_PATHPOLY || eKind==OBJ_PATHFILL || eKind==OBJ_FREEFILL || eKind==OBJ_SPLNFILL; }
    bool IsFreeHand(SdrObjKind eKind) const { return eKind==OBJ_FREELINE || eKind==OBJ_FREEFILL; }
    bool IsBezier(SdrObjKind eKind) const { return eKind==OBJ_PATHLINE || eKind==OBJ_PATHFILL; }
    bool IsCreating() const { return mbCreating; }

    // get the polygon
    basegfx::B2DPolyPolygon TakeObjectPolyPolygon(const SdrDragStat& rDrag) const;
    basegfx::B2DPolyPolygon TakeDragPolyPolygon(const SdrDragStat& rDrag) const;
    basegfx::B2DPolyPolygon getModifiedPolyPolygon() const { return  aPathPolygon.getB2DPolyPolygon(); }
};

ImpPathForDragAndCreate::ImpPathForDragAndCreate(SdrPathObj& rSdrPathObject)
:   mrSdrPathObject(rSdrPathObject),
    aPathPolygon(rSdrPathObject.GetPathPoly()),
    meObjectKind(mrSdrPathObject.meKind),
    mpSdrPathDragData(0),
    mbCreating(false)
{
}

ImpPathForDragAndCreate::~ImpPathForDragAndCreate()
{
    if(mpSdrPathDragData)
    {
        delete mpSdrPathDragData;
    }
}

bool ImpPathForDragAndCreate::beginPathDrag( SdrDragStat& rDrag )  const
{
    const SdrHdl* pHdl=rDrag.GetHdl();
    if(!pHdl)
        return sal_False;

    bool bMultiPointDrag(true);

    if(aPathPolygon[(sal_uInt16)pHdl->GetPolyNum()].IsControl((sal_uInt16)pHdl->GetPointNum()))
        bMultiPointDrag = false;

    if(bMultiPointDrag)
    {
        const SdrMarkView& rMarkView = *rDrag.GetView();
        const SdrHdlList& rHdlList = rMarkView.GetHdlList();
        const sal_uInt32 nHdlCount = rHdlList.GetHdlCount();
        const SdrObject* pInteractionObject(nHdlCount && rHdlList.GetHdl(0) ? rHdlList.GetHdl(0)->GetObj() : 0);
        sal_uInt32 nSelectedPoints(0);

        for(sal_uInt32 a(0); a < nHdlCount; a++)
        {
            SdrHdl* pTestHdl = rHdlList.GetHdl(a);

            if(pTestHdl && pTestHdl->IsSelected() && pTestHdl->GetObj() == pInteractionObject)
            {
                nSelectedPoints++;
            }
        }

        if(nSelectedPoints <= 1)
            bMultiPointDrag = false;
    }

    ((ImpPathForDragAndCreate*)this)->mpSdrPathDragData = new ImpSdrPathDragData(mrSdrPathObject,*pHdl,bMultiPointDrag,rDrag);

    if(!mpSdrPathDragData || !mpSdrPathDragData->bValid)
    {
        OSL_FAIL("ImpPathForDragAndCreate::BegDrag(): ImpSdrPathDragData is invalid.");
        delete mpSdrPathDragData;
        ((ImpPathForDragAndCreate*)this)->mpSdrPathDragData = 0;
        return false;
    }

    return true;
}

bool ImpPathForDragAndCreate::movePathDrag( SdrDragStat& rDrag ) const
{
    if(!mpSdrPathDragData || !mpSdrPathDragData->bValid)
    {
        OSL_FAIL("ImpPathForDragAndCreate::MovDrag(): ImpSdrPathDragData is invalid.");
        return false;
    }

    if(mpSdrPathDragData->IsMultiPointDrag())
    {
        Point aDelta(rDrag.GetNow() - rDrag.GetStart());

        if(aDelta.X() || aDelta.Y())
        {
            for(sal_uInt32 a(0); a < mpSdrPathDragData->maHandles.size(); a++)
            {
                SdrHdl* pHandle = mpSdrPathDragData->maHandles[a];
                const sal_uInt16 nPolyIndex((sal_uInt16)pHandle->GetPolyNum());
                const sal_uInt16 nPointIndex((sal_uInt16)pHandle->GetPointNum());
                const XPolygon& rOrig = mpSdrPathDragData->maOrig[nPolyIndex];
                XPolygon& rMove = mpSdrPathDragData->maMove[nPolyIndex];
                const sal_uInt16 nPointCount(rOrig.GetPointCount());
                bool bClosed(rOrig[0] == rOrig[nPointCount-1]);

                // move point itself
                rMove[nPointIndex] = rOrig[nPointIndex] + aDelta;

                // when point is first and poly closed, move close point, too.
                if(nPointCount > 0 && !nPointIndex && bClosed)
                {
                    rMove[nPointCount - 1] = rOrig[nPointCount - 1] + aDelta;

                    // when moving the last point it may be necessary to move the
                    // control point in front of this one, too.
                    if(nPointCount > 1 && rOrig.IsControl(nPointCount - 2))
                        rMove[nPointCount - 2] = rOrig[nPointCount - 2] + aDelta;
                }

                // is a control point before this?
                if(nPointIndex > 0 && rOrig.IsControl(nPointIndex - 1))
                {
                    // Yes, move it, too
                    rMove[nPointIndex - 1] = rOrig[nPointIndex - 1] + aDelta;
                }

                // is a control point after this?
                if(nPointIndex + 1 < nPointCount && rOrig.IsControl(nPointIndex + 1))
                {
                    // Yes, move it, too
                    rMove[nPointIndex + 1] = rOrig[nPointIndex + 1] + aDelta;
                }
            }
        }
    }
    else
    {
        mpSdrPathDragData->ResetPoly(mrSdrPathObject);

        // copy certain data locally to use less code and have faster access times
        bool bClosed           =mpSdrPathDragData->bClosed       ; // closed object?
        sal_uInt16   nPnt          =mpSdrPathDragData->nPnt          ; // number of point in the above polygon
        bool bBegPnt           =mpSdrPathDragData->bBegPnt       ; // dragged point is first point of a Polyline
        bool bEndPnt           =mpSdrPathDragData->bEndPnt       ; // dragged point is last point of a Polyline
        sal_uInt16   nPrevPnt      =mpSdrPathDragData->nPrevPnt      ; // index of previous point
        sal_uInt16   nNextPnt      =mpSdrPathDragData->nNextPnt      ; // index of next point
        bool bPrevIsBegPnt     =mpSdrPathDragData->bPrevIsBegPnt ; // previous point is first point of a Polyline
        bool bNextIsEndPnt     =mpSdrPathDragData->bNextIsEndPnt ; // next point is last point of a Polyline
        sal_uInt16   nPrevPrevPnt  =mpSdrPathDragData->nPrevPrevPnt  ; // index of the point before the previous point
        sal_uInt16   nNextNextPnt  =mpSdrPathDragData->nNextNextPnt  ; // index if the point after the next point
        bool bControl          =mpSdrPathDragData->bControl      ; // point is a control point
        bool bIsNextControl    =mpSdrPathDragData->bIsNextControl; // point is a control point after a support point
        bool bPrevIsControl    =mpSdrPathDragData->bPrevIsControl; // if nPnt is a support point: there's a control point before
        bool bNextIsControl    =mpSdrPathDragData->bNextIsControl; // if nPnt is a support point: there's a control point after

        // Ortho for lines/polygons: keep angle
        if (!bControl && rDrag.GetView()!=NULL && rDrag.GetView()->IsOrtho()) {
            bool bBigOrtho=rDrag.GetView()->IsBigOrtho();
            Point  aPos(rDrag.GetNow());      // current position
            Point  aPnt(mpSdrPathDragData->aXP[nPnt]);      // the dragged point
            sal_uInt16 nPnt1=0xFFFF,nPnt2=0xFFFF; // its neighboring points
            Point  aNeuPos1,aNeuPos2;         // new alternative for aPos
            bool bPnt1 = false, bPnt2 = false; // are these valid alternatives?
            if (!bClosed && mpSdrPathDragData->nPntAnz>=2) { // minimum of 2 points for lines
                if (!bBegPnt) nPnt1=nPrevPnt;
                if (!bEndPnt) nPnt2=nNextPnt;
            }
            if (bClosed && mpSdrPathDragData->nPntAnz>=3) { // minimum of 3 points for polygon
                nPnt1=nPrevPnt;
                nPnt2=nNextPnt;
            }
            if (nPnt1!=0xFFFF && !bPrevIsControl) {
                Point aPnt1=mpSdrPathDragData->aXP[nPnt1];
                long ndx0=aPnt.X()-aPnt1.X();
                long ndy0=aPnt.Y()-aPnt1.Y();
                bool bHLin=ndy0==0;
                bool bVLin=ndx0==0;
                if (!bHLin || !bVLin) {
                    long ndx=aPos.X()-aPnt1.X();
                    long ndy=aPos.Y()-aPnt1.Y();
                    bPnt1=sal_True;
                    double nXFact=0; if (!bVLin) nXFact=(double)ndx/(double)ndx0;
                    double nYFact=0; if (!bHLin) nYFact=(double)ndy/(double)ndy0;
                    bool bHor=bHLin || (!bVLin && (nXFact>nYFact) ==bBigOrtho);
                    bool bVer=bVLin || (!bHLin && (nXFact<=nYFact)==bBigOrtho);
                    if (bHor) ndy=long(ndy0*nXFact);
                    if (bVer) ndx=long(ndx0*nYFact);
                    aNeuPos1=aPnt1;
                    aNeuPos1.X()+=ndx;
                    aNeuPos1.Y()+=ndy;
                }
            }
            if (nPnt2!=0xFFFF && !bNextIsControl) {
                Point aPnt2=mpSdrPathDragData->aXP[nPnt2];
                long ndx0=aPnt.X()-aPnt2.X();
                long ndy0=aPnt.Y()-aPnt2.Y();
                bool bHLin=ndy0==0;
                bool bVLin=ndx0==0;
                if (!bHLin || !bVLin) {
                    long ndx=aPos.X()-aPnt2.X();
                    long ndy=aPos.Y()-aPnt2.Y();
                    bPnt2=sal_True;
                    double nXFact=0; if (!bVLin) nXFact=(double)ndx/(double)ndx0;
                    double nYFact=0; if (!bHLin) nYFact=(double)ndy/(double)ndy0;
                    bool bHor=bHLin || (!bVLin && (nXFact>nYFact) ==bBigOrtho);
                    bool bVer=bVLin || (!bHLin && (nXFact<=nYFact)==bBigOrtho);
                    if (bHor) ndy=long(ndy0*nXFact);
                    if (bVer) ndx=long(ndx0*nYFact);
                    aNeuPos2=aPnt2;
                    aNeuPos2.X()+=ndx;
                    aNeuPos2.Y()+=ndy;
                }
            }
            if (bPnt1 && bPnt2) { // both alternatives exist (and compete)
                BigInt nX1(aNeuPos1.X()-aPos.X()); nX1*=nX1;
                BigInt nY1(aNeuPos1.Y()-aPos.Y()); nY1*=nY1;
                BigInt nX2(aNeuPos2.X()-aPos.X()); nX2*=nX2;
                BigInt nY2(aNeuPos2.Y()-aPos.Y()); nY2*=nY2;
                nX1+=nY1; // correction distance to square
                nX2+=nY2; // correction distance to square
                // let the alternative that allows fewer correction win
                if (nX1<nX2) bPnt2=sal_False; else bPnt1=sal_False;
            }
            if (bPnt1) rDrag.Now()=aNeuPos1;
            if (bPnt2) rDrag.Now()=aNeuPos2;
        }
        rDrag.SetActionRect(Rectangle(rDrag.GetNow(),rDrag.GetNow()));

        // specially for IBM: Eliminate points if both adjoining lines form near 180 degrees angle anyway
        if (!bControl && rDrag.GetView()!=NULL && rDrag.GetView()->IsEliminatePolyPoints() &&
            !bBegPnt && !bEndPnt && !bPrevIsControl && !bNextIsControl)
        {
            Point aPt(mpSdrPathDragData->aXP[nNextPnt]);
            aPt-=rDrag.GetNow();
            long nWink1=GetAngle(aPt);
            aPt=rDrag.GetNow();
            aPt-=mpSdrPathDragData->aXP[nPrevPnt];
            long nWink2=GetAngle(aPt);
            long nDiff=nWink1-nWink2;
            nDiff=std::abs(nDiff);
            mpSdrPathDragData->bEliminate=nDiff<=rDrag.GetView()->GetEliminatePolyPointLimitAngle();
            if (mpSdrPathDragData->bEliminate) { // adapt position, Smooth is true for the ends
                aPt=mpSdrPathDragData->aXP[nNextPnt];
                aPt+=mpSdrPathDragData->aXP[nPrevPnt];
                aPt/=2;
                rDrag.Now()=aPt;
            }
        }

        // we dragged by this distance
        Point aDiff(rDrag.GetNow()); aDiff-=mpSdrPathDragData->aXP[nPnt];

        /* There are 8 possible cases:
              X      1. A control point neither on the left nor on the right.
           o--X--o   2. There are control points on the left and the right, we are dragging a support point.
           o--X      3. There is a control point on the left, we are dragging a support point.
              X--o   4. There is a control point on the right, we are dragging a support point.
           x--O--o   5. There are control points on the left and the right, we are dragging the left one.
           x--O      6. There is a control point on the left, we are dragging it.
           o--O--x   7. There are control points on the left and the right, we are dragging the right one.
              O--x   8. There is a control point on the right, we are dragging it.
           Note: modifying a line (not a curve!) might create a curve on the other end of the line
           if Smooth is set there (with control points aligned to line).
        */

        mpSdrPathDragData->aXP[nPnt]+=aDiff;

        // now check symmetric plus handles
        if (bControl) { // cases 5,6,7,8
            sal_uInt16   nSt=nPnt;   // the associated support point
            sal_uInt16   nFix=nPnt;  // the opposing control point
            if (bIsNextControl) { // if the next one is a control point, the on before has to be a support point
                nSt=nPrevPnt;
                nFix=nPrevPrevPnt;
            } else {
                nSt=nNextPnt;
                nFix=nNextNextPnt;
            }
            if (mpSdrPathDragData->aXP.IsSmooth(nSt)) {
                mpSdrPathDragData->aXP.CalcSmoothJoin(nSt,nPnt,nFix);
            }
        }

        if (!bControl) { // Cases 1,2,3,4. In case 1, nothing happens; in cases 3 and 4, there is more following below.
            // move both control points
            if (bPrevIsControl) mpSdrPathDragData->aXP[nPrevPnt]+=aDiff;
            if (bNextIsControl) mpSdrPathDragData->aXP[nNextPnt]+=aDiff;
            // align control point to line, if appropriate
            if (mpSdrPathDragData->aXP.IsSmooth(nPnt)) {
                if (bPrevIsControl && !bNextIsControl && !bEndPnt) { // case 3
                    mpSdrPathDragData->aXP.CalcSmoothJoin(nPnt,nNextPnt,nPrevPnt);
                }
                if (bNextIsControl && !bPrevIsControl && !bBegPnt) { // case 4
                    mpSdrPathDragData->aXP.CalcSmoothJoin(nPnt,nPrevPnt,nNextPnt);
                }
            }
            // Now check the other ends of the line (nPnt+-1). If there is a
            // curve (IsControl(nPnt+-2)) with SmoothJoin (nPnt+-1), the
            // associated control point (nPnt+-2) has to be adapted.
            if (!bBegPnt && !bPrevIsControl && !bPrevIsBegPnt && mpSdrPathDragData->aXP.IsSmooth(nPrevPnt)) {
                if (mpSdrPathDragData->aXP.IsControl(nPrevPrevPnt)) {
                    mpSdrPathDragData->aXP.CalcSmoothJoin(nPrevPnt,nPnt,nPrevPrevPnt);
                }
            }
            if (!bEndPnt && !bNextIsControl && !bNextIsEndPnt && mpSdrPathDragData->aXP.IsSmooth(nNextPnt)) {
                if (mpSdrPathDragData->aXP.IsControl(nNextNextPnt)) {
                    mpSdrPathDragData->aXP.CalcSmoothJoin(nNextPnt,nPnt,nNextNextPnt);
                }
            }
        }
    }

    return true;
}

bool ImpPathForDragAndCreate::endPathDrag(SdrDragStat& rDrag)
{
    Point aLinePt1;
    Point aLinePt2;
    bool bLineGlueMirror(OBJ_LINE == meObjectKind);
    if (bLineGlueMirror) {
        XPolygon& rXP=aPathPolygon[0];
        aLinePt1=rXP[0];
        aLinePt2=rXP[1];
    }

    if(!mpSdrPathDragData || !mpSdrPathDragData->bValid)
    {
        OSL_FAIL("ImpPathForDragAndCreate::MovDrag(): ImpSdrPathDragData is invalid.");
        return false;
    }

    if(mpSdrPathDragData->IsMultiPointDrag())
    {
        aPathPolygon = mpSdrPathDragData->maMove;
    }
    else
    {
        const SdrHdl* pHdl=rDrag.GetHdl();

        // reference the polygon
        XPolygon& rXP=aPathPolygon[(sal_uInt16)pHdl->GetPolyNum()];

        // the 5 points that might have changed
        if (!mpSdrPathDragData->bPrevIsBegPnt) rXP[mpSdrPathDragData->nPrevPrevPnt0]=mpSdrPathDragData->aXP[mpSdrPathDragData->nPrevPrevPnt];
        if (!mpSdrPathDragData->bNextIsEndPnt) rXP[mpSdrPathDragData->nNextNextPnt0]=mpSdrPathDragData->aXP[mpSdrPathDragData->nNextNextPnt];
        if (!mpSdrPathDragData->bBegPnt)       rXP[mpSdrPathDragData->nPrevPnt0]    =mpSdrPathDragData->aXP[mpSdrPathDragData->nPrevPnt];
        if (!mpSdrPathDragData->bEndPnt)       rXP[mpSdrPathDragData->nNextPnt0]    =mpSdrPathDragData->aXP[mpSdrPathDragData->nNextPnt];
        rXP[mpSdrPathDragData->nPnt0]        =mpSdrPathDragData->aXP[mpSdrPathDragData->nPnt];

        // for closed objects: last point has to be equal to first point
        if (mpSdrPathDragData->bClosed) rXP[rXP.GetPointCount()-1]=rXP[0];

        if (mpSdrPathDragData->bEliminate)
        {
            basegfx::B2DPolyPolygon aTempPolyPolygon(aPathPolygon.getB2DPolyPolygon());
            sal_uInt32 nPoly,nPnt;

            if(PolyPolygonEditor::GetRelativePolyPoint(aTempPolyPolygon, rDrag.GetHdl()->GetSourceHdlNum(), nPoly, nPnt))
            {
                basegfx::B2DPolygon aCandidate(aTempPolyPolygon.getB2DPolygon(nPoly));
                aCandidate.remove(nPnt);

                if((IsClosed(meObjectKind) && aCandidate.count() < 3L) || aCandidate.count() < 2L)
                {
                    aTempPolyPolygon.remove(nPoly);
                }
                else
                {
                    aTempPolyPolygon.setB2DPolygon(nPoly, aCandidate);
                }
            }

            aPathPolygon = XPolyPolygon(aTempPolyPolygon);
        }

        // adapt angle for text beneath a simple line
        if (bLineGlueMirror)
        {
            Point aLinePt1_(aPathPolygon[0][0]);
            Point aLinePt2_(aPathPolygon[0][1]);
            bool bXMirr=(aLinePt1_.X()>aLinePt2_.X())!=(aLinePt1.X()>aLinePt2.X());
            bool bYMirr=(aLinePt1_.Y()>aLinePt2_.Y())!=(aLinePt1.Y()>aLinePt2.Y());
            if (bXMirr || bYMirr) {
                Point aRef1(mrSdrPathObject.GetSnapRect().Center());
                if (bXMirr) {
                    Point aRef2(aRef1);
                    aRef2.Y()++;
                    mrSdrPathObject.NbcMirrorGluePoints(aRef1,aRef2);
                }
                if (bYMirr) {
                    Point aRef2(aRef1);
                    aRef2.X()++;
                    mrSdrPathObject.NbcMirrorGluePoints(aRef1,aRef2);
                }
            }
        }
    }

    delete mpSdrPathDragData;
    mpSdrPathDragData = 0;

    return true;
}


String ImpPathForDragAndCreate::getSpecialDragComment(const SdrDragStat& rDrag) const
{
    XubString aStr;
    const SdrHdl* pHdl = rDrag.GetHdl();
    const bool bCreateComment(rDrag.GetView() && &mrSdrPathObject == rDrag.GetView()->GetCreateObj());

    if(bCreateComment && rDrag.GetUser())
    {
        // #i103058# re-add old creation comment mode
        ImpPathCreateUser* pU = (ImpPathCreateUser*)rDrag.GetUser();
        const SdrObjKind eKindMerk(meObjectKind);
        mrSdrPathObject.meKind = pU->eAktKind;
        OUString aTmp;
        mrSdrPathObject.ImpTakeDescriptionStr(STR_ViewCreateObj, aTmp);
        aStr = aTmp;
        mrSdrPathObject.meKind = eKindMerk;

        Point aPrev(rDrag.GetPrev());
        Point aNow(rDrag.GetNow());

        if(pU->bLine)
            aNow = pU->aLineEnd;

        aNow -= aPrev;
        aStr.AppendAscii(" (");

        OUString aMetr;

        if(pU->bCircle)
        {
            mrSdrPathObject.GetModel()->TakeWinkStr(std::abs(pU->nCircRelWink), aMetr);
            aStr.Append(aMetr);
            aStr.AppendAscii(" r=");
            mrSdrPathObject.GetModel()->TakeMetricStr(pU->nCircRadius, aMetr, sal_True);
            aStr.Append(aMetr);
        }

        aStr.AppendAscii("dx=");
        mrSdrPathObject.GetModel()->TakeMetricStr(aNow.X(), aMetr, sal_True);
        aStr.Append(aMetr);

        aStr.AppendAscii(" dy=");
        mrSdrPathObject.GetModel()->TakeMetricStr(aNow.Y(), aMetr, sal_True);
        aStr.Append(aMetr);

        if(!IsFreeHand(meObjectKind))
        {
            sal_Int32 nLen(GetLen(aNow));
            aStr.AppendAscii("  l=");
            mrSdrPathObject.GetModel()->TakeMetricStr(nLen, aMetr, sal_True);
            aStr.Append(aMetr);

            sal_Int32 nWink(GetAngle(aNow));
            aStr += sal_Unicode(' ');
            mrSdrPathObject.GetModel()->TakeWinkStr(nWink, aMetr);
            aStr.Append(aMetr);
        }

        aStr += sal_Unicode(')');
    }
    else if(!mrSdrPathObject.GetModel() || !pHdl)
    {
        // #i103058# fallback when no model and/or Handle, both needed
        // for else-path
        OUString aTmp;
        mrSdrPathObject.ImpTakeDescriptionStr(STR_DragPathObj, aTmp);
        aStr = aTmp;
    }
    else
    {
        // #i103058# standard for modification; model and handle needed
        ImpSdrPathDragData* pDragData = mpSdrPathDragData;

        if(!pDragData)
        {
            // getSpecialDragComment is also used from create, so fallback to GetUser()
            // when mpSdrPathDragData is not set
            pDragData = (ImpSdrPathDragData*)rDrag.GetUser();
        }

        if(!pDragData)
        {
            OSL_FAIL("ImpPathForDragAndCreate::MovDrag(): ImpSdrPathDragData is invalid.");
            return String();
        }

        if(!pDragData->IsMultiPointDrag() && pDragData->bEliminate)
        {
            // point of ...
            OUString aTmp;
            mrSdrPathObject.ImpTakeDescriptionStr(STR_ViewMarkedPoint, aTmp);
            aStr = aTmp;

            // delete %O
            XubString aStr2(ImpGetResStr(STR_EditDelete));

            // UNICODE: delete point of ...
            aStr2.SearchAndReplaceAscii("%1", aStr);

            return aStr2;
        }

        // dx=0.00 dy=0.00                -- both sides bezier
        // dx=0.00 dy=0.00  l=0.00 0.00°  -- one bezier/lever on one side, a start, or an ending
        // dx=0.00 dy=0.00  l=0.00 0.00° / l=0.00 0.00° -- in between
        OUString aMetr;
        Point aBeg(rDrag.GetStart());
        Point aNow(rDrag.GetNow());

        aStr = String();
        aStr.AppendAscii("dx=");
        mrSdrPathObject.GetModel()->TakeMetricStr(aNow.X() - aBeg.X(), aMetr, sal_True);
        aStr.Append(aMetr);

        aStr.AppendAscii(" dy=");
        mrSdrPathObject.GetModel()->TakeMetricStr(aNow.Y() - aBeg.Y(), aMetr, sal_True);
        aStr.Append(aMetr);

        if(!pDragData->IsMultiPointDrag())
        {
            sal_uInt16 nPntNum((sal_uInt16)pHdl->GetPointNum());
            const XPolygon& rXPoly = aPathPolygon[(sal_uInt16)rDrag.GetHdl()->GetPolyNum()];
            sal_uInt16 nPntAnz((sal_uInt16)rXPoly.GetPointCount());
            sal_Bool bClose(IsClosed(meObjectKind));

            if(bClose)
                nPntAnz--;

            if(pHdl->IsPlusHdl())
            {
                // lever
                sal_uInt16 nRef(nPntNum);

                if(rXPoly.IsControl(nPntNum + 1))
                    nRef--;
                else
                    nRef++;

                aNow -= rXPoly[nRef];

                sal_Int32 nLen(GetLen(aNow));
                aStr.AppendAscii("  l=");
                mrSdrPathObject.GetModel()->TakeMetricStr(nLen, aMetr, sal_True);
                aStr.Append(aMetr);

                sal_Int32 nWink(GetAngle(aNow));
                aStr += sal_Unicode(' ');
                mrSdrPathObject.GetModel()->TakeWinkStr(nWink, aMetr);
                aStr.Append(aMetr);
            }
            else if(nPntAnz > 1)
            {
                sal_uInt16 nPntMax(nPntAnz - 1);
                sal_Bool bIsClosed(IsClosed(meObjectKind));
                bool bPt1(nPntNum > 0);
                bool bPt2(nPntNum < nPntMax);

                if(bIsClosed && nPntAnz > 2)
                {
                    bPt1 = true;
                    bPt2 = true;
                }

                sal_uInt16 nPt1,nPt2;

                if(nPntNum > 0)
                    nPt1 = nPntNum - 1;
                else
                    nPt1 = nPntMax;

                if(nPntNum < nPntMax)
                    nPt2 = nPntNum + 1;
                else
                    nPt2 = 0;

                if(bPt1 && rXPoly.IsControl(nPt1))
                    bPt1 = false; // don't display

                if(bPt2 && rXPoly.IsControl(nPt2))
                    bPt2 = false; // of bezier data

                if(bPt1)
                {
                    Point aPt(aNow);
                    aPt -= rXPoly[nPt1];

                    sal_Int32 nLen(GetLen(aPt));
                    aStr.AppendAscii("  l=");
                    mrSdrPathObject.GetModel()->TakeMetricStr(nLen, aMetr, sal_True);
                    aStr += aMetr;

                    sal_Int32 nWink(GetAngle(aPt));
                    aStr += sal_Unicode(' ');
                    mrSdrPathObject.GetModel()->TakeWinkStr(nWink, aMetr);
                    aStr += aMetr;
                }

                if(bPt2)
                {
                    if(bPt1)
                        aStr.AppendAscii(" / ");
                    else
                        aStr.AppendAscii("  ");

                    Point aPt(aNow);
                    aPt -= rXPoly[nPt2];

                    sal_Int32 nLen(GetLen(aPt));
                    aStr.AppendAscii("l=");
                    mrSdrPathObject.GetModel()->TakeMetricStr(nLen, aMetr, sal_True);
                    aStr += aMetr;

                    sal_Int32 nWink(GetAngle(aPt));
                    aStr += sal_Unicode(' ');
                    mrSdrPathObject.GetModel()->TakeWinkStr(nWink, aMetr);
                    aStr += aMetr;
                }
            }
        }
    }

    return aStr;
}

basegfx::B2DPolyPolygon ImpPathForDragAndCreate::getSpecialDragPoly(const SdrDragStat& rDrag) const
{
    if(!mpSdrPathDragData || !mpSdrPathDragData->bValid)
    {
        OSL_FAIL("ImpPathForDragAndCreate::MovDrag(): ImpSdrPathDragData is invalid.");
        return basegfx::B2DPolyPolygon();
    }

    XPolyPolygon aRetval;

    if(mpSdrPathDragData->IsMultiPointDrag())
    {
        aRetval.Insert(mpSdrPathDragData->maMove);
    }
    else
    {
        const XPolygon& rXP=aPathPolygon[(sal_uInt16)rDrag.GetHdl()->GetPolyNum()];
        if (rXP.GetPointCount()<=2) {
            XPolygon aXPoly(rXP);
            aXPoly[(sal_uInt16)rDrag.GetHdl()->GetPointNum()]=rDrag.GetNow();
            aRetval.Insert(aXPoly);
            return aRetval.getB2DPolyPolygon();
        }
        // copy certain data locally to use less code and have faster access times
        bool bClosed           =mpSdrPathDragData->bClosed       ; // closed object?
        sal_uInt16   nPntAnz       =mpSdrPathDragData->nPntAnz       ; // number of points
        sal_uInt16   nPnt          =mpSdrPathDragData->nPnt          ; // number of points in the polygon
        bool bBegPnt           =mpSdrPathDragData->bBegPnt       ; // dragged point is the first point of a Polyline
        bool bEndPnt           =mpSdrPathDragData->bEndPnt       ; // dragged point is the last point of a Polyline
        sal_uInt16   nPrevPnt      =mpSdrPathDragData->nPrevPnt      ; // index of the previous point
        sal_uInt16   nNextPnt      =mpSdrPathDragData->nNextPnt      ; // index of the next point
        bool bPrevIsBegPnt     =mpSdrPathDragData->bPrevIsBegPnt ; // previous point is first point of a Polyline
        bool bNextIsEndPnt     =mpSdrPathDragData->bNextIsEndPnt ; // next point is last point of a Polyline
        sal_uInt16   nPrevPrevPnt  =mpSdrPathDragData->nPrevPrevPnt  ; // index of the point before the previous point
        sal_uInt16   nNextNextPnt  =mpSdrPathDragData->nNextNextPnt  ; // index of the point after the last point
        bool bControl          =mpSdrPathDragData->bControl      ; // point is a control point
        bool bIsNextControl    =mpSdrPathDragData->bIsNextControl; //point is a control point after a support point
        bool bPrevIsControl    =mpSdrPathDragData->bPrevIsControl; // if nPnt is a support point: there's a control point before
        bool bNextIsControl    =mpSdrPathDragData->bNextIsControl; // if nPnt is a support point: there's a control point after
        XPolygon aXPoly(mpSdrPathDragData->aXP);
        XPolygon aLine1(2);
        XPolygon aLine2(2);
        XPolygon aLine3(2);
        XPolygon aLine4(2);
        if (bControl) {
            aLine1[1]=mpSdrPathDragData->aXP[nPnt];
            if (bIsNextControl) { // is this a control point after the support point?
                aLine1[0]=mpSdrPathDragData->aXP[nPrevPnt];
                aLine2[0]=mpSdrPathDragData->aXP[nNextNextPnt];
                aLine2[1]=mpSdrPathDragData->aXP[nNextPnt];
                if (mpSdrPathDragData->aXP.IsSmooth(nPrevPnt) && !bPrevIsBegPnt && mpSdrPathDragData->aXP.IsControl(nPrevPrevPnt)) {
                    aXPoly.Insert(0,rXP[mpSdrPathDragData->nPrevPrevPnt0-1],XPOLY_CONTROL);
                    aXPoly.Insert(0,rXP[mpSdrPathDragData->nPrevPrevPnt0-2],XPOLY_NORMAL);
                    // leverage lines for the opposing curve segment
                    aLine3[0]=mpSdrPathDragData->aXP[nPrevPnt];
                    aLine3[1]=mpSdrPathDragData->aXP[nPrevPrevPnt];
                    aLine4[0]=rXP[mpSdrPathDragData->nPrevPrevPnt0-2];
                    aLine4[1]=rXP[mpSdrPathDragData->nPrevPrevPnt0-1];
                } else {
                    aXPoly.Remove(0,1);
                }
            } else { // else this is a control point before a support point
                aLine1[0]=mpSdrPathDragData->aXP[nNextPnt];
                aLine2[0]=mpSdrPathDragData->aXP[nPrevPrevPnt];
                aLine2[1]=mpSdrPathDragData->aXP[nPrevPnt];
                if (mpSdrPathDragData->aXP.IsSmooth(nNextPnt) && !bNextIsEndPnt && mpSdrPathDragData->aXP.IsControl(nNextNextPnt)) {
                    aXPoly.Insert(XPOLY_APPEND,rXP[mpSdrPathDragData->nNextNextPnt0+1],XPOLY_CONTROL);
                    aXPoly.Insert(XPOLY_APPEND,rXP[mpSdrPathDragData->nNextNextPnt0+2],XPOLY_NORMAL);
                    // leverage lines for the opposing curve segment
                    aLine3[0]=mpSdrPathDragData->aXP[nNextPnt];
                    aLine3[1]=mpSdrPathDragData->aXP[nNextNextPnt];
                    aLine4[0]=rXP[mpSdrPathDragData->nNextNextPnt0+2];
                    aLine4[1]=rXP[mpSdrPathDragData->nNextNextPnt0+1];
                } else {
                    aXPoly.Remove(aXPoly.GetPointCount()-1,1);
                }
            }
        } else { // else is not a control point
            if (mpSdrPathDragData->bEliminate) {
                aXPoly.Remove(2,1);
            }
            if (bPrevIsControl) aXPoly.Insert(0,rXP[mpSdrPathDragData->nPrevPrevPnt0-1],XPOLY_NORMAL);
            else if (!bBegPnt && !bPrevIsBegPnt && mpSdrPathDragData->aXP.IsControl(nPrevPrevPnt)) {
                aXPoly.Insert(0,rXP[mpSdrPathDragData->nPrevPrevPnt0-1],XPOLY_CONTROL);
                aXPoly.Insert(0,rXP[mpSdrPathDragData->nPrevPrevPnt0-2],XPOLY_NORMAL);
            } else {
                aXPoly.Remove(0,1);
                if (bBegPnt) aXPoly.Remove(0,1);
            }
            if (bNextIsControl) aXPoly.Insert(XPOLY_APPEND,rXP[mpSdrPathDragData->nNextNextPnt0+1],XPOLY_NORMAL);
            else if (!bEndPnt && !bNextIsEndPnt && mpSdrPathDragData->aXP.IsControl(nNextNextPnt)) {
                aXPoly.Insert(XPOLY_APPEND,rXP[mpSdrPathDragData->nNextNextPnt0+1],XPOLY_CONTROL);
                aXPoly.Insert(XPOLY_APPEND,rXP[mpSdrPathDragData->nNextNextPnt0+2],XPOLY_NORMAL);
            } else {
                aXPoly.Remove(aXPoly.GetPointCount()-1,1);
                if (bEndPnt) aXPoly.Remove(aXPoly.GetPointCount()-1,1);
            }
            if (bClosed) { // "pear problem": 2 lines, 1 curve, everything smoothed, a point between both lines is dragged
                if (aXPoly.GetPointCount()>nPntAnz && aXPoly.IsControl(1)) {
                    sal_uInt16 a=aXPoly.GetPointCount();
                    aXPoly[a-2]=aXPoly[2]; aXPoly.SetFlags(a-2,aXPoly.GetFlags(2));
                    aXPoly[a-1]=aXPoly[3]; aXPoly.SetFlags(a-1,aXPoly.GetFlags(3));
                    aXPoly.Remove(0,3);
                }
            }
        }
        aRetval.Insert(aXPoly);
        if (aLine1.GetPointCount()>1) aRetval.Insert(aLine1);
        if (aLine2.GetPointCount()>1) aRetval.Insert(aLine2);
        if (aLine3.GetPointCount()>1) aRetval.Insert(aLine3);
        if (aLine4.GetPointCount()>1) aRetval.Insert(aLine4);
    }

    return aRetval.getB2DPolyPolygon();
}

bool ImpPathForDragAndCreate::BegCreate(SdrDragStat& rStat)
{
    bool bFreeHand(IsFreeHand(meObjectKind));
    rStat.SetNoSnap(bFreeHand);
    rStat.SetOrtho8Possible();
    aPathPolygon.Clear();
    mbCreating=sal_True;
    bool bMakeStartPoint = true;
    SdrView* pView=rStat.GetView();
    if (pView!=NULL && pView->IsUseIncompatiblePathCreateInterface() &&
        (meObjectKind==OBJ_POLY || meObjectKind==OBJ_PLIN || meObjectKind==OBJ_PATHLINE || meObjectKind==OBJ_PATHFILL)) {
        bMakeStartPoint = false;
    }
    aPathPolygon.Insert(XPolygon());
    aPathPolygon[0][0]=rStat.GetStart();
    if (bMakeStartPoint) {
        aPathPolygon[0][1]=rStat.GetNow();
    }
    ImpPathCreateUser* pU=new ImpPathCreateUser;
    pU->eStartKind=meObjectKind;
    pU->eAktKind=meObjectKind;
    rStat.SetUser(pU);
    return true;
}

bool ImpPathForDragAndCreate::MovCreate(SdrDragStat& rStat)
{
    ImpPathCreateUser* pU=(ImpPathCreateUser*)rStat.GetUser();
    SdrView* pView=rStat.GetView();
    XPolygon& rXPoly=aPathPolygon[aPathPolygon.Count()-1];
    if (pView!=NULL && pView->IsCreateMode()) {
        // switch to different CreateTool, if appropriate
        sal_uInt16 nIdent;
        sal_uInt32 nInvent;
        pView->TakeCurrentObj(nIdent,nInvent);
        if (nInvent==SdrInventor && pU->eAktKind!=(SdrObjKind)nIdent) {
            SdrObjKind eNewKind=(SdrObjKind)nIdent;
            switch (eNewKind) {
                case OBJ_CARC: case OBJ_CIRC: case OBJ_CCUT: case OBJ_SECT: eNewKind=OBJ_CARC;
                case OBJ_RECT:
                case OBJ_LINE: case OBJ_PLIN: case OBJ_POLY:
                case OBJ_PATHLINE: case OBJ_PATHFILL:
                case OBJ_FREELINE: case OBJ_FREEFILL:
                case OBJ_SPLNLINE: case OBJ_SPLNFILL: {
                    pU->eAktKind=eNewKind;
                    pU->bMixedCreate=sal_True;
                    pU->nBezierStartPoint=rXPoly.GetPointCount();
                    if (pU->nBezierStartPoint>0) pU->nBezierStartPoint--;
                } break;
                default: break;
            } // switch
        }
    }
    sal_uInt16 nActPoint=rXPoly.GetPointCount();
    if (aPathPolygon.Count()>1 && rStat.IsMouseDown() && nActPoint<2) {
        rXPoly[0]=rStat.GetPos0();
        rXPoly[1]=rStat.GetNow();
        nActPoint=2;
    }
    if (nActPoint==0) {
        rXPoly[0]=rStat.GetPos0();
    } else nActPoint--;
    bool bFreeHand=IsFreeHand(pU->eAktKind);
    rStat.SetNoSnap(bFreeHand);
    rStat.SetOrtho8Possible(pU->eAktKind!=OBJ_CARC && pU->eAktKind!=OBJ_RECT && (!pU->bMixedCreate || pU->eAktKind!=OBJ_LINE));
    rXPoly[nActPoint]=rStat.Now();
    if (!pU->bMixedCreate && pU->eStartKind==OBJ_LINE && rXPoly.GetPointCount()>=1) {
        Point aPt(rStat.Start());
        if (pView!=NULL && pView->IsCreate1stPointAsCenter()) {
            aPt+=aPt;
            aPt-=rStat.Now();
        }
        rXPoly[0]=aPt;
    }
    OutputDevice* pOut=pView==NULL ? NULL : pView->GetFirstOutputDevice();
    if (bFreeHand) {
        if (pU->nBezierStartPoint>nActPoint) pU->nBezierStartPoint=nActPoint;
        if (rStat.IsMouseDown() && nActPoint>0) {
            // don't allow two consecutive points to occupy too similar positions
            long nMinDist=1;
            if (pView!=NULL) nMinDist=pView->GetFreeHandMinDistPix();
            if (pOut!=NULL) nMinDist=pOut->PixelToLogic(Size(nMinDist,0)).Width();
            if (nMinDist<1) nMinDist=1;

            Point aPt0(rXPoly[nActPoint-1]);
            Point aPt1(rStat.Now());
            long dx=aPt0.X()-aPt1.X(); if (dx<0) dx=-dx;
            long dy=aPt0.Y()-aPt1.Y(); if (dy<0) dy=-dy;
            if (dx<nMinDist && dy<nMinDist) return sal_False;

            // TODO: the following is copied from EndCreate (with a few smaller modifications)
            // and should be combined into a method with the code there.

            if (nActPoint-pU->nBezierStartPoint>=3 && ((nActPoint-pU->nBezierStartPoint)%3)==0) {
                rXPoly.PointsToBezier(nActPoint-3);
                rXPoly.SetFlags(nActPoint-1,XPOLY_CONTROL);
                rXPoly.SetFlags(nActPoint-2,XPOLY_CONTROL);

                if (nActPoint>=6 && rXPoly.IsControl(nActPoint-4)) {
                    rXPoly.CalcTangent(nActPoint-3,nActPoint-4,nActPoint-2);
                    rXPoly.SetFlags(nActPoint-3,XPOLY_SMOOTH);
                }
            }
            rXPoly[nActPoint+1]=rStat.Now();
            rStat.NextPoint();
        } else {
            pU->nBezierStartPoint=nActPoint;
        }
    }

    pU->ResetFormFlags();
    if (IsBezier(pU->eAktKind)) {
        if (nActPoint>=2) {
            pU->CalcBezier(rXPoly[nActPoint-1],rXPoly[nActPoint],rXPoly[nActPoint-1]-rXPoly[nActPoint-2],rStat.IsMouseDown());
        } else if (pU->bBezHasCtrl0) {
            pU->CalcBezier(rXPoly[nActPoint-1],rXPoly[nActPoint],pU->aBezControl0-rXPoly[nActPoint-1],rStat.IsMouseDown());
        }
    }
    if (pU->eAktKind==OBJ_CARC && nActPoint>=2) {
        pU->CalcCircle(rXPoly[nActPoint-1],rXPoly[nActPoint],rXPoly[nActPoint-1]-rXPoly[nActPoint-2],pView);
    }
    if (pU->eAktKind==OBJ_LINE && nActPoint>=2) {
        pU->CalcLine(rXPoly[nActPoint-1],rXPoly[nActPoint],rXPoly[nActPoint-1]-rXPoly[nActPoint-2],pView);
    }
    if (pU->eAktKind==OBJ_RECT && nActPoint>=2) {
        pU->CalcRect(rXPoly[nActPoint-1],rXPoly[nActPoint],rXPoly[nActPoint-1]-rXPoly[nActPoint-2],pView);
    }

    return true;
}

bool ImpPathForDragAndCreate::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    ImpPathCreateUser* pU=(ImpPathCreateUser*)rStat.GetUser();
    bool bRet = false;
    SdrView* pView=rStat.GetView();
    bool bIncomp=pView!=NULL && pView->IsUseIncompatiblePathCreateInterface();
    XPolygon& rXPoly=aPathPolygon[aPathPolygon.Count()-1];
    sal_uInt16 nActPoint=rXPoly.GetPointCount()-1;
    rXPoly[nActPoint]=rStat.Now();
    if (!pU->bMixedCreate && pU->eStartKind==OBJ_LINE) {
        if (rStat.GetPointAnz()>=2) eCmd=SDRCREATE_FORCEEND;
        bRet = eCmd==SDRCREATE_FORCEEND;
        if (bRet) {
            mbCreating = sal_False;
            delete pU;
            rStat.SetUser(NULL);
        }
        return bRet;
    }

    if (!pU->bMixedCreate && IsFreeHand(pU->eStartKind)) {
        if (rStat.GetPointAnz()>=2) eCmd=SDRCREATE_FORCEEND;
        bRet=eCmd==SDRCREATE_FORCEEND;
        if (bRet) {
            mbCreating=sal_False;
            delete pU;
            rStat.SetUser(NULL);
        }
        return bRet;
    }
    if (eCmd==SDRCREATE_NEXTPOINT || eCmd==SDRCREATE_NEXTOBJECT) {
        // don't allow two consecutive points to occupy the same position
        if (nActPoint==0 || rStat.Now()!=rXPoly[nActPoint-1]) {
            if (bIncomp) {
                if (pU->nBezierStartPoint>nActPoint) pU->nBezierStartPoint=nActPoint;
                if (IsBezier(pU->eAktKind) && nActPoint-pU->nBezierStartPoint>=3 && ((nActPoint-pU->nBezierStartPoint)%3)==0) {
                    rXPoly.PointsToBezier(nActPoint-3);
                    rXPoly.SetFlags(nActPoint-1,XPOLY_CONTROL);
                    rXPoly.SetFlags(nActPoint-2,XPOLY_CONTROL);

                    if (nActPoint>=6 && rXPoly.IsControl(nActPoint-4)) {
                        rXPoly.CalcTangent(nActPoint-3,nActPoint-4,nActPoint-2);
                        rXPoly.SetFlags(nActPoint-3,XPOLY_SMOOTH);
                    }
                }
            } else {
                if (nActPoint==1 && IsBezier(pU->eAktKind) && !pU->bBezHasCtrl0) {
                    pU->aBezControl0=rStat.GetNow();
                    pU->bBezHasCtrl0=sal_True;
                    nActPoint--;
                }
                if (pU->IsFormFlag()) {
                    sal_uInt16 nPtAnz0=rXPoly.GetPointCount();
                    rXPoly.Remove(nActPoint-1,2); // remove last two points and replace by form
                    rXPoly.Insert(XPOLY_APPEND,pU->GetFormPoly());
                    sal_uInt16 nPtAnz1=rXPoly.GetPointCount();
                    for (sal_uInt16 i=nPtAnz0+1; i<nPtAnz1-1; i++) { // to make BckAction work
                        if (!rXPoly.IsControl(i)) rStat.NextPoint();
                    }
                    nActPoint=rXPoly.GetPointCount()-1;
                }
            }
            nActPoint++;
            rXPoly[nActPoint]=rStat.GetNow();
        }
        if (eCmd==SDRCREATE_NEXTOBJECT) {
            if (rXPoly.GetPointCount()>=2) {
                pU->bBezHasCtrl0=sal_False;
                // only a singular polygon may be opened, so close this
                rXPoly[nActPoint]=rXPoly[0];
                XPolygon aXP;
                aXP[0]=rStat.GetNow();
                aPathPolygon.Insert(aXP);
            }
        }
    }

    sal_uInt16 nPolyAnz=aPathPolygon.Count();
    if (nPolyAnz!=0) {
        // delete last point, if necessary
        if (eCmd==SDRCREATE_FORCEEND) {
            XPolygon& rXP=aPathPolygon[nPolyAnz-1];
            sal_uInt16 nPtAnz=rXP.GetPointCount();
            if (nPtAnz>=2) {
                if (!rXP.IsControl(nPtAnz-2)) {
                    if (rXP[nPtAnz-1]==rXP[nPtAnz-2]) {
                        rXP.Remove(nPtAnz-1,1);
                    }
                } else {
                    if (rXP[nPtAnz-3]==rXP[nPtAnz-2]) {
                        rXP.Remove(nPtAnz-3,3);
                    }
                }
            }
        }
        for (sal_uInt16 nPolyNum=nPolyAnz; nPolyNum>0;) {
            nPolyNum--;
            XPolygon& rXP=aPathPolygon[nPolyNum];
            sal_uInt16 nPtAnz=rXP.GetPointCount();
            // delete polygons with too few points
            if (nPolyNum<nPolyAnz-1 || eCmd==SDRCREATE_FORCEEND) {
                if (nPtAnz<2) aPathPolygon.Remove(nPolyNum);
            }
        }
    }
    pU->ResetFormFlags();
    bRet=eCmd==SDRCREATE_FORCEEND;
    if (bRet) {
        mbCreating=sal_False;
        delete pU;
        rStat.SetUser(NULL);
    }
    return bRet;
}

bool ImpPathForDragAndCreate::BckCreate(SdrDragStat& rStat)
{
    ImpPathCreateUser* pU=(ImpPathCreateUser*)rStat.GetUser();
    if (aPathPolygon.Count()>0) {
        XPolygon& rXPoly=aPathPolygon[aPathPolygon.Count()-1];
        sal_uInt16 nActPoint=rXPoly.GetPointCount();
        if (nActPoint>0) {
            nActPoint--;
            // make the last part of a bezier curve a line
            rXPoly.Remove(nActPoint,1);
            if (nActPoint>=3 && rXPoly.IsControl(nActPoint-1)) {
                // there should never be a bezier segment at the end, so this is just in case...
                rXPoly.Remove(nActPoint-1,1);
                if (rXPoly.IsControl(nActPoint-2)) rXPoly.Remove(nActPoint-2,1);
            }
        }
        nActPoint=rXPoly.GetPointCount();
        if (nActPoint>=4) { // no bezier segment at the end
            nActPoint--;
            if (rXPoly.IsControl(nActPoint-1)) {
                rXPoly.Remove(nActPoint-1,1);
                if (rXPoly.IsControl(nActPoint-2)) rXPoly.Remove(nActPoint-2,1);
            }
        }
        if (rXPoly.GetPointCount()<2) {
            aPathPolygon.Remove(aPathPolygon.Count()-1);
        }
        if (aPathPolygon.Count()>0) {
            XPolygon& rLocalXPoly=aPathPolygon[aPathPolygon.Count()-1];
            sal_uInt16 nLocalActPoint=rLocalXPoly.GetPointCount();
            if (nLocalActPoint>0) {
                nLocalActPoint--;
                rLocalXPoly[nLocalActPoint]=rStat.Now();
            }
        }
    }
    pU->ResetFormFlags();
    return aPathPolygon.Count()!=0;
}

void ImpPathForDragAndCreate::BrkCreate(SdrDragStat& rStat)
{
    ImpPathCreateUser* pU=(ImpPathCreateUser*)rStat.GetUser();
    aPathPolygon.Clear();
    mbCreating=sal_False;
    delete pU;
    rStat.SetUser(NULL);
}

basegfx::B2DPolyPolygon ImpPathForDragAndCreate::TakeObjectPolyPolygon(const SdrDragStat& rDrag) const
{
    basegfx::B2DPolyPolygon aRetval(aPathPolygon.getB2DPolyPolygon());
    SdrView* pView = rDrag.GetView();

    if(pView && pView->IsUseIncompatiblePathCreateInterface())
        return aRetval;

    ImpPathCreateUser* pU = (ImpPathCreateUser*)rDrag.GetUser();
    basegfx::B2DPolygon aNewPolygon(aRetval.count() ? aRetval.getB2DPolygon(aRetval.count() - 1L) : basegfx::B2DPolygon());

    if(pU->IsFormFlag() && aNewPolygon.count() > 1L)
    {
        // remove last segment and replace with current
        // do not forget to rescue the previous control point which will be lost when
        // the point it's associated with is removed
        const sal_uInt32 nChangeIndex(aNewPolygon.count() - 2);
        const basegfx::B2DPoint aSavedPrevCtrlPoint(aNewPolygon.getPrevControlPoint(nChangeIndex));

        aNewPolygon.remove(nChangeIndex, 2L);
        aNewPolygon.append(pU->GetFormPoly().getB2DPolygon());

        if(nChangeIndex < aNewPolygon.count())
        {
            // if really something was added, set the saved previous control point to the
            // point where it belongs
            aNewPolygon.setPrevControlPoint(nChangeIndex, aSavedPrevCtrlPoint);
        }
    }

    if(aRetval.count())
    {
        aRetval.setB2DPolygon(aRetval.count() - 1L, aNewPolygon);
    }
    else
    {
        aRetval.append(aNewPolygon);
    }

    return aRetval;
}

basegfx::B2DPolyPolygon ImpPathForDragAndCreate::TakeDragPolyPolygon(const SdrDragStat& rDrag) const
{
    basegfx::B2DPolyPolygon aRetval;
    SdrView* pView = rDrag.GetView();

    if(pView && pView->IsUseIncompatiblePathCreateInterface())
        return aRetval;

    ImpPathCreateUser* pU = (ImpPathCreateUser*)rDrag.GetUser();

    if(pU && pU->bBezier && rDrag.IsMouseDown())
    {
        // no more XOR, no need for complicated helplines
        basegfx::B2DPolygon aHelpline;
        aHelpline.append(basegfx::B2DPoint(pU->aBezCtrl2.X(), pU->aBezCtrl2.Y()));
        aHelpline.append(basegfx::B2DPoint(pU->aBezEnd.X(), pU->aBezEnd.Y()));
        aRetval.append(aHelpline);
    }

    return aRetval;
}

Pointer ImpPathForDragAndCreate::GetCreatePointer() const
{
    switch (meObjectKind) {
        case OBJ_LINE    : return Pointer(POINTER_DRAW_LINE);
        case OBJ_POLY    : return Pointer(POINTER_DRAW_POLYGON);
        case OBJ_PLIN    : return Pointer(POINTER_DRAW_POLYGON);
        case OBJ_PATHLINE: return Pointer(POINTER_DRAW_BEZIER);
        case OBJ_PATHFILL: return Pointer(POINTER_DRAW_BEZIER);
        case OBJ_FREELINE: return Pointer(POINTER_DRAW_FREEHAND);
        case OBJ_FREEFILL: return Pointer(POINTER_DRAW_FREEHAND);
        case OBJ_SPLNLINE: return Pointer(POINTER_DRAW_FREEHAND);
        case OBJ_SPLNFILL: return Pointer(POINTER_DRAW_FREEHAND);
        case OBJ_PATHPOLY: return Pointer(POINTER_DRAW_POLYGON);
        case OBJ_PATHPLIN: return Pointer(POINTER_DRAW_POLYGON);
        default: break;
    } // switch
    return Pointer(POINTER_CROSS);
}

/*************************************************************************/

SdrPathObjGeoData::SdrPathObjGeoData()
{
}

SdrPathObjGeoData::~SdrPathObjGeoData()
{
}

//////////////////////////////////////////////////////////////////////////////
// DrawContact section

sdr::contact::ViewContact* SdrPathObj::CreateObjectSpecificViewContact()
{
    return new sdr::contact::ViewContactOfSdrPathObj(*this);
}

/*************************************************************************/

TYPEINIT1(SdrPathObj,SdrTextObj);

SdrPathObj::SdrPathObj(SdrObjKind eNewKind)
:   meKind(eNewKind),
    mpDAC(0L),
    mdBrightness(1.0)
{
    bClosedObj = IsClosed();
}

SdrPathObj::SdrPathObj(SdrObjKind eNewKind, const basegfx::B2DPolyPolygon& rPathPoly, double dBrightness)
:   maPathPolygon(rPathPoly),
    meKind(eNewKind),
    mpDAC(0L),
    mdBrightness(dBrightness)
{
    bClosedObj = IsClosed();
    ImpForceKind();
}

SdrPathObj::~SdrPathObj()
{
    impDeleteDAC();
}

static bool lcl_ImpIsLine(const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    return (1L == rPolyPolygon.count() && 2L == rPolyPolygon.getB2DPolygon(0L).count());
}

static Rectangle lcl_ImpGetBoundRect(const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    basegfx::B2DRange aRange(basegfx::tools::getRange(rPolyPolygon));

    return Rectangle(
        FRound(aRange.getMinX()), FRound(aRange.getMinY()),
        FRound(aRange.getMaxX()), FRound(aRange.getMaxY()));
}

void SdrPathObj::ImpForceLineWink()
{
    if(OBJ_LINE == meKind && lcl_ImpIsLine(GetPathPoly()))
    {
        const basegfx::B2DPolygon aPoly(GetPathPoly().getB2DPolygon(0L));
        const basegfx::B2DPoint aB2DPoint0(aPoly.getB2DPoint(0L));
        const basegfx::B2DPoint aB2DPoint1(aPoly.getB2DPoint(1L));
        const Point aPoint0(FRound(aB2DPoint0.getX()), FRound(aB2DPoint0.getY()));
        const Point aPoint1(FRound(aB2DPoint1.getX()), FRound(aB2DPoint1.getY()));
        const Point aDelt(aPoint1 - aPoint0);

        aGeo.nDrehWink=GetAngle(aDelt);
        aGeo.nShearWink=0;
        aGeo.RecalcSinCos();
        aGeo.RecalcTan();

        // for SdrTextObj, keep aRect up to date
        aRect = Rectangle(aPoint0, aPoint1);
        aRect.Justify();
    }
}

void SdrPathObj::ImpForceKind()
{
    if (meKind==OBJ_PATHPLIN) meKind=OBJ_PLIN;
    if (meKind==OBJ_PATHPOLY) meKind=OBJ_POLY;

    if(GetPathPoly().areControlPointsUsed())
    {
        switch (meKind)
        {
            case OBJ_LINE: meKind=OBJ_PATHLINE; break;
            case OBJ_PLIN: meKind=OBJ_PATHLINE; break;
            case OBJ_POLY: meKind=OBJ_PATHFILL; break;
            default: break;
        }
    }
    else
    {
        switch (meKind)
        {
            case OBJ_PATHLINE: meKind=OBJ_PLIN; break;
            case OBJ_FREELINE: meKind=OBJ_PLIN; break;
            case OBJ_PATHFILL: meKind=OBJ_POLY; break;
            case OBJ_FREEFILL: meKind=OBJ_POLY; break;
            default: break;
        }
    }

    if (meKind==OBJ_LINE && !lcl_ImpIsLine(GetPathPoly())) meKind=OBJ_PLIN;
    if (meKind==OBJ_PLIN && lcl_ImpIsLine(GetPathPoly())) meKind=OBJ_LINE;

    bClosedObj=IsClosed();

    if (meKind==OBJ_LINE)
    {
        ImpForceLineWink();
    }
    else
    {
        // #i10659#, for polys with more than 2 points.
        //
        // Here i again need to fix something, because when Path-Polys are Copy-Pasted
        // between Apps with different measurements (e.g. 100TH_MM and TWIPS) there is
        // a scaling loop started from SdrExchangeView::Paste. In itself, this is not
        // wrong, but aRect is wrong here and not even updated by RecalcSnapRect(). If
        // this is the case, some size needs to be set here in aRect to avoid that the cycle
        // through Rect2Poly - Poly2Rect does something badly wrong since that cycle is
        // BASED on aRect. That cycle is triggered in SdrTextObj::NbcResize() which is called
        // from the local Resize() implementation.
        //
        // Basic problem is that the member aRect in SdrTextObj basically is a unrotated
        // text rectangle for the text object itself and methods at SdrTextObj do handle it
        // in that way. Many draw objects derived from SdrTextObj 'abuse' aRect as SnapRect
        // which is basically wrong. To make the SdrText methods which deal with aRect directly
        // work it is necessary to always keep aRect updated. This e.g. not done after a Clone()
        // command for SdrPathObj. Since adding this update mechanism with #101412# to
        // ImpForceLineWink() for lines was very successful, i add it to where ImpForceLineWink()
        // was called, once here below and once on a 2nd place below.

        // #i10659# for SdrTextObj, keep aRect up to date
        if(GetPathPoly().count())
        {
            aRect = lcl_ImpGetBoundRect(GetPathPoly());
        }

        // #i116244# reset rotation
        aGeo.nDrehWink = aGeo.nShearWink = 0;
        aGeo.RecalcSinCos(); aGeo.RecalcTan();
    }

    // #i75974# adapt polygon state to object type. This may include a reinterpretation
    // of a closed geometry as open one, but with identical first and last point
    for(sal_uInt32 a(0); a < maPathPolygon.count(); a++)
    {
        basegfx::B2DPolygon aCandidate(maPathPolygon.getB2DPolygon(a));

        if((bool)IsClosed() != aCandidate.isClosed())
        {
            // #i80213# really change polygon geometry; else e.g. the last point which
            // needs to be identical with the first one will be missing when opening
            // due to OBJ_PATH type
            if(aCandidate.isClosed())
            {
                basegfx::tools::openWithGeometryChange(aCandidate);
            }
            else
            {
                basegfx::tools::closeWithGeometryChange(aCandidate);
            }

            maPathPolygon.setB2DPolygon(a, aCandidate);
        }
    }
}

void SdrPathObj::ImpSetClosed(sal_Bool bClose)
{
    if(bClose)
    {
        switch (meKind)
        {
            case OBJ_LINE    : meKind=OBJ_POLY;     break;
            case OBJ_PLIN    : meKind=OBJ_POLY;     break;
            case OBJ_PATHLINE: meKind=OBJ_PATHFILL; break;
            case OBJ_FREELINE: meKind=OBJ_FREEFILL; break;
            case OBJ_SPLNLINE: meKind=OBJ_SPLNFILL; break;
            default: break;
        }

        bClosedObj = sal_True;
    }
    else
    {
        switch (meKind)
        {
            case OBJ_POLY    : meKind=OBJ_PLIN;     break;
            case OBJ_PATHFILL: meKind=OBJ_PATHLINE; break;
            case OBJ_FREEFILL: meKind=OBJ_FREELINE; break;
            case OBJ_SPLNFILL: meKind=OBJ_SPLNLINE; break;
            default: break;
        }

        bClosedObj = sal_False;
    }

    ImpForceKind();
}

void SdrPathObj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bNoContortion=sal_False;

    bool bCanConv = !HasText() || ImpCanConvTextToCurve();
    bool bIsPath = IsBezier() || IsSpline();

    rInfo.bEdgeRadiusAllowed    = sal_False;
    rInfo.bCanConvToPath = bCanConv && !bIsPath;
    rInfo.bCanConvToPoly = bCanConv && bIsPath;
    rInfo.bCanConvToContour = !IsFontwork() && (rInfo.bCanConvToPoly || LineGeometryUsageIsNecessary());
}

sal_uInt16 SdrPathObj::GetObjIdentifier() const
{
    return sal_uInt16(meKind);
}

SdrPathObj* SdrPathObj::Clone() const
{
    return CloneHelper< SdrPathObj >();
}

SdrPathObj& SdrPathObj::operator=(const SdrPathObj& rObj)
{
    if( this == &rObj )
        return *this;
    SdrTextObj::operator=(rObj);
    maPathPolygon=rObj.GetPathPoly();
    return *this;
}

OUString SdrPathObj::TakeObjNameSingul() const
{
    OUStringBuffer sName;

    if(OBJ_LINE == meKind)
    {
        sal_uInt16 nId(STR_ObjNameSingulLINE);

        if(lcl_ImpIsLine(GetPathPoly()))
        {
            const basegfx::B2DPolygon aPoly(GetPathPoly().getB2DPolygon(0L));
            const basegfx::B2DPoint aB2DPoint0(aPoly.getB2DPoint(0L));
            const basegfx::B2DPoint aB2DPoint1(aPoly.getB2DPoint(1L));

            if(aB2DPoint0 != aB2DPoint1)
            {
                if(aB2DPoint0.getY() == aB2DPoint1.getY())
                {
                    nId = STR_ObjNameSingulLINE_Hori;
                }
                else if(aB2DPoint0.getX() == aB2DPoint1.getX())
                {
                    nId = STR_ObjNameSingulLINE_Vert;
                }
                else
                {
                    const double fDx(fabs(aB2DPoint0.getX() - aB2DPoint1.getX()));
                    const double fDy(fabs(aB2DPoint0.getY() - aB2DPoint1.getY()));

                    if(fDx == fDy)
                    {
                        nId = STR_ObjNameSingulLINE_Diag;
                    }
                }
            }
        }

        sName.append(ImpGetResStr(nId));
    }
    else if(OBJ_PLIN == meKind || OBJ_POLY == meKind)
    {
        const bool bClosed(OBJ_POLY == meKind);
        sal_uInt16 nId(0);

        if(mpDAC && mpDAC->IsCreating())
        {
            if(bClosed)
            {
                nId = STR_ObjNameSingulPOLY;
            }
            else
            {
                nId = STR_ObjNameSingulPLIN;
            }

            sName.append(ImpGetResStr(nId));
        }
        else
        {
            // get point count
            sal_uInt32 nPointCount(0L);
            const sal_uInt32 nPolyCount(GetPathPoly().count());

            for(sal_uInt32 a(0L); a < nPolyCount; a++)
            {
                nPointCount += GetPathPoly().getB2DPolygon(a).count();
            }

            if(bClosed)
            {
                nId = STR_ObjNameSingulPOLY_PntAnz;
            }
            else
            {
                nId = STR_ObjNameSingulPLIN_PntAnz;
            }

            OUString sTemp(ImpGetResStr(nId));
            // #i96537#
            sName.append(sTemp.replaceFirst("%2", OUString::number(nPointCount)));
        }
    }
    else
    {
        switch (meKind)
        {
            case OBJ_PATHLINE: sName.append(ImpGetResStr(STR_ObjNameSingulPATHLINE)); break;
            case OBJ_FREELINE: sName.append(ImpGetResStr(STR_ObjNameSingulFREELINE)); break;
            case OBJ_SPLNLINE: sName.append(ImpGetResStr(STR_ObjNameSingulNATSPLN)); break;
            case OBJ_PATHFILL: sName.append(ImpGetResStr(STR_ObjNameSingulPATHFILL)); break;
            case OBJ_FREEFILL: sName.append(ImpGetResStr(STR_ObjNameSingulFREEFILL)); break;
            case OBJ_SPLNFILL: sName.append(ImpGetResStr(STR_ObjNameSingulPERSPLN)); break;
            default: break;
        }
    }

    OUString aName(GetName());
    if (!aName.isEmpty())
    {
        sName.append(' ');
        sName.append('\'');
        sName.append(aName);
        sName.append('\'');
    }

    return sName.makeStringAndClear();
}

void SdrPathObj::TakeObjNamePlural(XubString& rName) const
{
    switch(meKind)
    {
        case OBJ_LINE    : rName=ImpGetResStr(STR_ObjNamePluralLINE    ); break;
        case OBJ_PLIN    : rName=ImpGetResStr(STR_ObjNamePluralPLIN    ); break;
        case OBJ_POLY    : rName=ImpGetResStr(STR_ObjNamePluralPOLY    ); break;
        case OBJ_PATHLINE: rName=ImpGetResStr(STR_ObjNamePluralPATHLINE); break;
        case OBJ_FREELINE: rName=ImpGetResStr(STR_ObjNamePluralFREELINE); break;
        case OBJ_SPLNLINE: rName=ImpGetResStr(STR_ObjNamePluralNATSPLN); break;
        case OBJ_PATHFILL: rName=ImpGetResStr(STR_ObjNamePluralPATHFILL); break;
        case OBJ_FREEFILL: rName=ImpGetResStr(STR_ObjNamePluralFREEFILL); break;
        case OBJ_SPLNFILL: rName=ImpGetResStr(STR_ObjNamePluralPERSPLN); break;
        default: break;
    }
}

basegfx::B2DPolyPolygon SdrPathObj::TakeXorPoly() const
{
    return GetPathPoly();
}

sal_uInt32 SdrPathObj::GetHdlCount() const
{
    sal_uInt32 nRetval(0L);
    const sal_uInt32 nPolyCount(GetPathPoly().count());

    for(sal_uInt32 a(0L); a < nPolyCount; a++)
    {
        nRetval += GetPathPoly().getB2DPolygon(a).count();
    }

    return nRetval;
}

SdrHdl* SdrPathObj::GetHdl(sal_uInt32 nHdlNum) const
{
    // #i73248#
    // Warn the user that this is ineffective and show alternatives. Should not be used at all.
    OSL_FAIL("SdrPathObj::GetHdl(): ineffective, use AddToHdlList instead (!)");

    // to have an alternative, get single handle using the ineffective way
    SdrHdl* pRetval = 0;
    SdrHdlList aLocalList(0);
    AddToHdlList(aLocalList);
    const sal_uInt32 nHdlCount(aLocalList.GetHdlCount());

    if(nHdlCount && nHdlNum < nHdlCount)
    {
        // remove and remember. The other created handles will be deleted again with the
        // destruction of the local list
        pRetval = aLocalList.RemoveHdl(nHdlNum);
    }

    return pRetval;
}

void SdrPathObj::AddToHdlList(SdrHdlList& rHdlList) const
{
    // keep old stuff to be able to keep old SdrHdl stuff, too
    const XPolyPolygon aOldPathPolygon(GetPathPoly());
    sal_uInt16 nPolyCnt=aOldPathPolygon.Count();
    bool bClosed=IsClosed();
    sal_uInt16 nIdx=0;

    for (sal_uInt16 i=0; i<nPolyCnt; i++) {
        const XPolygon& rXPoly=aOldPathPolygon.GetObject(i);
        sal_uInt16 nPntCnt=rXPoly.GetPointCount();
        if (bClosed && nPntCnt>1) nPntCnt--;

        for (sal_uInt16 j=0; j<nPntCnt; j++) {
            if (rXPoly.GetFlags(j)!=XPOLY_CONTROL) {
                const Point& rPnt=rXPoly[j];
                SdrHdl* pHdl=new SdrHdl(rPnt,HDL_POLY);
                pHdl->SetPolyNum(i);
                pHdl->SetPointNum(j);
                pHdl->Set1PixMore(j==0);
                pHdl->SetSourceHdlNum(nIdx);
                nIdx++;
                rHdlList.AddHdl(pHdl);
            }
        }
    }
}

sal_uInt32 SdrPathObj::GetPlusHdlCount(const SdrHdl& rHdl) const
{
    // keep old stuff to be able to keep old SdrHdl stuff, too
    const XPolyPolygon aOldPathPolygon(GetPathPoly());
    sal_uInt16 nCnt = 0;
    sal_uInt16 nPnt = (sal_uInt16)rHdl.GetPointNum();
    sal_uInt16 nPolyNum = (sal_uInt16)rHdl.GetPolyNum();

    if(nPolyNum < aOldPathPolygon.Count())
    {
        const XPolygon& rXPoly = aOldPathPolygon[nPolyNum];
        sal_uInt16 nPntMax = rXPoly.GetPointCount();
        if (nPntMax>0)
        {
            nPntMax--;
            if (nPnt<=nPntMax)
            {
                if (rXPoly.GetFlags(nPnt)!=XPOLY_CONTROL)
                {
                    if (nPnt==0 && IsClosed()) nPnt=nPntMax;
                    if (nPnt>0 && rXPoly.GetFlags(nPnt-1)==XPOLY_CONTROL) nCnt++;
                    if (nPnt==nPntMax && IsClosed()) nPnt=0;
                    if (nPnt<nPntMax && rXPoly.GetFlags(nPnt+1)==XPOLY_CONTROL) nCnt++;
                }
            }
        }
    }

    return nCnt;
}

SdrHdl* SdrPathObj::GetPlusHdl(const SdrHdl& rHdl, sal_uInt32 nPlusNum) const
{
    // keep old stuff to be able to keep old SdrHdl stuff, too
    const XPolyPolygon aOldPathPolygon(GetPathPoly());
    SdrHdl* pHdl = 0L;
    sal_uInt16 nPnt = (sal_uInt16)rHdl.GetPointNum();
    sal_uInt16 nPolyNum = (sal_uInt16)rHdl.GetPolyNum();

    if (nPolyNum<aOldPathPolygon.Count())
    {
        const XPolygon& rXPoly = aOldPathPolygon[nPolyNum];
        sal_uInt16 nPntMax = rXPoly.GetPointCount();

        if (nPntMax>0)
        {
            nPntMax--;
            if (nPnt<=nPntMax)
            {
                pHdl=new SdrHdlBezWgt(&rHdl);
                pHdl->SetPolyNum(rHdl.GetPolyNum());

                if (nPnt==0 && IsClosed()) nPnt=nPntMax;
                if (nPnt>0 && rXPoly.GetFlags(nPnt-1)==XPOLY_CONTROL && nPlusNum==0)
                {
                    pHdl->SetPos(rXPoly[nPnt-1]);
                    pHdl->SetPointNum(nPnt-1);
                }
                else
                {
                    if (nPnt==nPntMax && IsClosed()) nPnt=0;
                    if (nPnt<rXPoly.GetPointCount()-1 && rXPoly.GetFlags(nPnt+1)==XPOLY_CONTROL)
                    {
                        pHdl->SetPos(rXPoly[nPnt+1]);
                        pHdl->SetPointNum(nPnt+1);
                    }
                }

                pHdl->SetSourceHdlNum(rHdl.GetSourceHdlNum());
                pHdl->SetPlusHdl(sal_True);
            }
        }
    }
    return pHdl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrPathObj::hasSpecialDrag() const
{
    return true;
}

bool SdrPathObj::beginSpecialDrag(SdrDragStat& rDrag) const
{
    ImpPathForDragAndCreate aDragAndCreate(*((SdrPathObj*)this));

    return aDragAndCreate.beginPathDrag(rDrag);
}

bool SdrPathObj::applySpecialDrag(SdrDragStat& rDrag)
{
    ImpPathForDragAndCreate aDragAndCreate(*this);
    bool bRetval(aDragAndCreate.beginPathDrag(rDrag));

    if(bRetval)
    {
        bRetval = aDragAndCreate.movePathDrag(rDrag);
    }

    if(bRetval)
    {
        bRetval = aDragAndCreate.endPathDrag(rDrag);
    }

    if(bRetval)
    {
           NbcSetPathPoly(aDragAndCreate.getModifiedPolyPolygon());
    }

    return bRetval;
}

String SdrPathObj::getSpecialDragComment(const SdrDragStat& rDrag) const
{
    String aRetval;

    if(mpDAC)
    {
        // #i103058# also get a comment when in creation
        const bool bCreateComment(rDrag.GetView() && this == rDrag.GetView()->GetCreateObj());

        if(bCreateComment)
        {
            aRetval = mpDAC->getSpecialDragComment(rDrag);
        }
    }
    else
    {
        ImpPathForDragAndCreate aDragAndCreate(*((SdrPathObj*)this));
        bool bDidWork(aDragAndCreate.beginPathDrag((SdrDragStat&)rDrag));

        if(bDidWork)
        {
            aRetval = aDragAndCreate.getSpecialDragComment(rDrag);
        }
    }

    return aRetval;
}

basegfx::B2DPolyPolygon SdrPathObj::getSpecialDragPoly(const SdrDragStat& rDrag) const
{
    basegfx::B2DPolyPolygon aRetval;
    ImpPathForDragAndCreate aDragAndCreate(*((SdrPathObj*)this));
    bool bDidWork(aDragAndCreate.beginPathDrag((SdrDragStat&)rDrag));

    if(bDidWork)
    {
        aRetval = aDragAndCreate.getSpecialDragPoly(rDrag);
    }

    return aRetval;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrPathObj::BegCreate(SdrDragStat& rStat)
{
    impDeleteDAC();
    return impGetDAC().BegCreate(rStat);
}

bool SdrPathObj::MovCreate(SdrDragStat& rStat)
{
    return impGetDAC().MovCreate(rStat);
}

bool SdrPathObj::EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd)
{
    bool bRetval(impGetDAC().EndCreate(rStat, eCmd));

    if(bRetval && mpDAC)
    {
        SetPathPoly(mpDAC->getModifiedPolyPolygon());

        // #i75974# Check for AutoClose feature. Moved here from ImpPathForDragAndCreate::EndCreate
        // to be able to use the type-changing ImpSetClosed method
        if(!IsClosedObj())
        {
            SdrView* pView = rStat.GetView();

            if(pView && pView->IsAutoClosePolys() && !pView->IsUseIncompatiblePathCreateInterface())
            {
                OutputDevice* pOut = pView->GetFirstOutputDevice();

                if(pOut)
                {
                    if(GetPathPoly().count())
                    {
                        const basegfx::B2DPolygon aCandidate(GetPathPoly().getB2DPolygon(0));

                        if(aCandidate.count() > 2)
                        {
                            // check distance of first and last point
                            const sal_Int32 nCloseDist(pOut->PixelToLogic(Size(pView->GetAutoCloseDistPix(), 0)).Width());
                            const basegfx::B2DVector aDistVector(aCandidate.getB2DPoint(aCandidate.count() - 1) - aCandidate.getB2DPoint(0));

                            if(aDistVector.getLength() <= (double)nCloseDist)
                            {
                                // close it
                                ImpSetClosed(true);
                            }
                        }
                    }
                }
            }
        }

        impDeleteDAC();
    }

    return bRetval;
}

bool SdrPathObj::BckCreate(SdrDragStat& rStat)
{
    return impGetDAC().BckCreate(rStat);
}

void SdrPathObj::BrkCreate(SdrDragStat& rStat)
{
    impGetDAC().BrkCreate(rStat);
    impDeleteDAC();
}

basegfx::B2DPolyPolygon SdrPathObj::TakeCreatePoly(const SdrDragStat& rDrag) const
{
    basegfx::B2DPolyPolygon aRetval;

    if(mpDAC)
    {
        aRetval = mpDAC->TakeObjectPolyPolygon(rDrag);
        aRetval.append(mpDAC->TakeDragPolyPolygon(rDrag));
    }

    return aRetval;
}

// during drag or create, allow accessing the so-far created/modified polyPolygon
basegfx::B2DPolyPolygon SdrPathObj::getObjectPolyPolygon(const SdrDragStat& rDrag) const
{
    basegfx::B2DPolyPolygon aRetval;

    if(mpDAC)
    {
        aRetval = mpDAC->TakeObjectPolyPolygon(rDrag);
    }

    return aRetval;
}

basegfx::B2DPolyPolygon SdrPathObj::getDragPolyPolygon(const SdrDragStat& rDrag) const
{
    basegfx::B2DPolyPolygon aRetval;

    if(mpDAC)
    {
        aRetval = mpDAC->TakeDragPolyPolygon(rDrag);
    }

    return aRetval;
}

Pointer SdrPathObj::GetCreatePointer() const
{
    return impGetDAC().GetCreatePointer();
}

void SdrPathObj::NbcMove(const Size& rSiz)
{
    maPathPolygon.transform(basegfx::tools::createTranslateB2DHomMatrix(rSiz.Width(), rSiz.Height()));

    // #i19871# first modify locally, then call parent (to get correct SnapRect with GluePoints)
    SdrTextObj::NbcMove(rSiz);
}

void SdrPathObj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    basegfx::B2DHomMatrix aTrans(basegfx::tools::createTranslateB2DHomMatrix(-rRef.X(), -rRef.Y()));
    aTrans = basegfx::tools::createScaleTranslateB2DHomMatrix(
        double(xFact), double(yFact), rRef.X(), rRef.Y()) * aTrans;
    maPathPolygon.transform(aTrans);

    // #i19871# first modify locally, then call parent (to get correct SnapRect with GluePoints)
    SdrTextObj::NbcResize(rRef,xFact,yFact);
}

void SdrPathObj::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
{
    // Thank JOE, the angles are defined mirrored to the mathematical meanings
    const basegfx::B2DHomMatrix aTrans(basegfx::tools::createRotateAroundPoint(rRef.X(), rRef.Y(), -nWink * nPi180));
    maPathPolygon.transform(aTrans);

    // #i19871# first modify locally, then call parent (to get correct SnapRect with GluePoints)
    SdrTextObj::NbcRotate(rRef,nWink,sn,cs);
}

void SdrPathObj::NbcShear(const Point& rRefPnt, long nAngle, double fTan, bool bVShear)
{
    basegfx::B2DHomMatrix aTrans(basegfx::tools::createTranslateB2DHomMatrix(-rRefPnt.X(), -rRefPnt.Y()));

    if(bVShear)
    {
        // Thank JOE, the angles are defined mirrored to the mathematical meanings
        aTrans.shearY(-fTan);
    }
    else
    {
        aTrans.shearX(-fTan);
    }

    aTrans.translate(rRefPnt.X(), rRefPnt.Y());
    maPathPolygon.transform(aTrans);

    // #i19871# first modify locally, then call parent (to get correct SnapRect with GluePoints)
    SdrTextObj::NbcShear(rRefPnt,nAngle,fTan,bVShear);
}

void SdrPathObj::NbcMirror(const Point& rRefPnt1, const Point& rRefPnt2)
{
    const double fDiffX(rRefPnt2.X() - rRefPnt1.X());
    const double fDiffY(rRefPnt2.Y() - rRefPnt1.Y());
    const double fRot(atan2(fDiffY, fDiffX));
    basegfx::B2DHomMatrix aTrans(basegfx::tools::createTranslateB2DHomMatrix(-rRefPnt1.X(), -rRefPnt1.Y()));
    aTrans.rotate(-fRot);
    aTrans.scale(1.0, -1.0);
    aTrans.rotate(fRot);
    aTrans.translate(rRefPnt1.X(), rRefPnt1.Y());
    maPathPolygon.transform(aTrans);

    // Do Joe's special handling for lines when mirroring, too
    ImpForceKind();

    // #i19871# first modify locally, then call parent (to get correct SnapRect with GluePoints)
    SdrTextObj::NbcMirror(rRefPnt1,rRefPnt2);
}

void SdrPathObj::TakeUnrotatedSnapRect(Rectangle& rRect) const
{
    if(!aGeo.nDrehWink)
    {
        rRect = GetSnapRect();
    }
    else
    {
        XPolyPolygon aXPP(GetPathPoly());
        RotateXPoly(aXPP,Point(),-aGeo.nSin,aGeo.nCos);
        rRect=aXPP.GetBoundRect();
        Point aTmp(rRect.TopLeft());
        RotatePoint(aTmp,Point(),aGeo.nSin,aGeo.nCos);
        aTmp-=rRect.TopLeft();
        rRect.Move(aTmp.X(),aTmp.Y());
    }
}

void SdrPathObj::RecalcSnapRect()
{
    if(GetPathPoly().count())
    {
        maSnapRect = lcl_ImpGetBoundRect(GetPathPoly());
    }
}

void SdrPathObj::NbcSetSnapRect(const Rectangle& rRect)
{
    Rectangle aOld(GetSnapRect());

    // Take RECT_EMPTY into account when calculating scale factors
    long nMulX = (RECT_EMPTY == rRect.Right()) ? 0 : rRect.Right()  - rRect.Left();

    long nDivX = aOld.Right()   - aOld.Left();

    // Take RECT_EMPTY into account when calculating scale factors
    long nMulY = (RECT_EMPTY == rRect.Bottom()) ? 0 : rRect.Bottom() - rRect.Top();

    long nDivY = aOld.Bottom()  - aOld.Top();
    if ( nDivX == 0 ) { nMulX = 1; nDivX = 1; }
    if ( nDivY == 0 ) { nMulY = 1; nDivY = 1; }
    Fraction aX(nMulX,nDivX);
    Fraction aY(nMulY,nDivY);
    NbcResize(aOld.TopLeft(), aX, aY);
    NbcMove(Size(rRect.Left() - aOld.Left(), rRect.Top() - aOld.Top()));
}

sal_uInt32 SdrPathObj::GetSnapPointCount() const
{
    return GetHdlCount();
}

Point SdrPathObj::GetSnapPoint(sal_uInt32 nSnapPnt) const
{
    sal_uInt32 nPoly,nPnt;
    if(!PolyPolygonEditor::GetRelativePolyPoint(GetPathPoly(), nSnapPnt, nPoly, nPnt))
    {
        DBG_ASSERT(sal_False,"SdrPathObj::GetSnapPoint: Point nSnapPnt does not exist.");
    }

    const basegfx::B2DPoint aB2DPoint(GetPathPoly().getB2DPolygon(nPoly).getB2DPoint(nPnt));
    return Point(FRound(aB2DPoint.getX()), FRound(aB2DPoint.getY()));
}

sal_Bool SdrPathObj::IsPolyObj() const
{
    return sal_True;
}

sal_uInt32 SdrPathObj::GetPointCount() const
{
    const sal_uInt32 nPolyCount(GetPathPoly().count());
    sal_uInt32 nRetval(0L);

    for(sal_uInt32 a(0L); a < nPolyCount; a++)
    {
        nRetval += GetPathPoly().getB2DPolygon(a).count();
    }

    return nRetval;
}

Point SdrPathObj::GetPoint(sal_uInt32 nHdlNum) const
{
    Point aRetval;
    sal_uInt32 nPoly,nPnt;

    if(PolyPolygonEditor::GetRelativePolyPoint(GetPathPoly(), nHdlNum, nPoly, nPnt))
    {
        const basegfx::B2DPolygon aPoly(GetPathPoly().getB2DPolygon(nPoly));
        const basegfx::B2DPoint aPoint(aPoly.getB2DPoint(nPnt));
        aRetval = Point(FRound(aPoint.getX()), FRound(aPoint.getY()));
    }

    return aRetval;
}

void SdrPathObj::NbcSetPoint(const Point& rPnt, sal_uInt32 nHdlNum)
{
    sal_uInt32 nPoly,nPnt;

    if(PolyPolygonEditor::GetRelativePolyPoint(GetPathPoly(), nHdlNum, nPoly, nPnt))
    {
        basegfx::B2DPolygon aNewPolygon(GetPathPoly().getB2DPolygon(nPoly));
        aNewPolygon.setB2DPoint(nPnt, basegfx::B2DPoint(rPnt.X(), rPnt.Y()));
        maPathPolygon.setB2DPolygon(nPoly, aNewPolygon);

        if(meKind==OBJ_LINE)
        {
            ImpForceLineWink();
        }
        else
        {
            if(GetPathPoly().count())
            {
                // #i10659# for SdrTextObj, keep aRect up to date
                aRect = lcl_ImpGetBoundRect(GetPathPoly());
            }
        }

        SetRectsDirty();
    }
}

sal_uInt32 SdrPathObj::NbcInsPointOld(const Point& rPos, sal_Bool bNewObj, sal_Bool bHideHim)
{
    sal_uInt32 nNewHdl;

    if(bNewObj)
    {
        nNewHdl = NbcInsPoint(0L, rPos, sal_True, bHideHim);
    }
    else
    {
        // look for smallest distance data
        const basegfx::B2DPoint aTestPoint(rPos.X(), rPos.Y());
        sal_uInt32 nSmallestPolyIndex(0L);
        sal_uInt32 nSmallestEdgeIndex(0L);
        double fSmallestCut;
        basegfx::tools::getSmallestDistancePointToPolyPolygon(GetPathPoly(), aTestPoint, nSmallestPolyIndex, nSmallestEdgeIndex, fSmallestCut);

        // create old polygon index from it
        sal_uInt32 nPolyIndex(nSmallestEdgeIndex);

        for(sal_uInt32 a(0L); a < nSmallestPolyIndex; a++)
        {
            nPolyIndex += GetPathPoly().getB2DPolygon(a).count();
        }

        nNewHdl = NbcInsPoint(nPolyIndex, rPos, sal_False, bHideHim);
    }

    ImpForceKind();
    return nNewHdl;
}

sal_uInt32 SdrPathObj::NbcInsPoint(sal_uInt32 /*nHdlNum*/, const Point& rPos, sal_Bool bNewObj, sal_Bool /*bHideHim*/)
{
    sal_uInt32 nNewHdl;

    if(bNewObj)
    {
        basegfx::B2DPolygon aNewPoly;
        const basegfx::B2DPoint aPoint(rPos.X(), rPos.Y());
        aNewPoly.append(aPoint);
        aNewPoly.setClosed(IsClosed());
        maPathPolygon.append(aNewPoly);
        SetRectsDirty();
        nNewHdl = GetHdlCount();
    }
    else
    {
        // look for smallest distance data
        const basegfx::B2DPoint aTestPoint(rPos.X(), rPos.Y());
        sal_uInt32 nSmallestPolyIndex(0L);
        sal_uInt32 nSmallestEdgeIndex(0L);
        double fSmallestCut;
        basegfx::tools::getSmallestDistancePointToPolyPolygon(GetPathPoly(), aTestPoint, nSmallestPolyIndex, nSmallestEdgeIndex, fSmallestCut);
        basegfx::B2DPolygon aCandidate(GetPathPoly().getB2DPolygon(nSmallestPolyIndex));
        const bool bBefore(!aCandidate.isClosed() && 0L == nSmallestEdgeIndex && 0.0 == fSmallestCut);
        const bool bAfter(!aCandidate.isClosed() && aCandidate.count() == nSmallestEdgeIndex + 2L && 1.0 == fSmallestCut);

        if(bBefore)
        {
            // before first point
            aCandidate.insert(0L, aTestPoint);

            if(aCandidate.areControlPointsUsed())
            {
                if(aCandidate.isNextControlPointUsed(1))
                {
                    aCandidate.setNextControlPoint(0, interpolate(aTestPoint, aCandidate.getB2DPoint(1), (1.0 / 3.0)));
                    aCandidate.setPrevControlPoint(1, interpolate(aTestPoint, aCandidate.getB2DPoint(1), (2.0 / 3.0)));
                }
            }

            nNewHdl = 0L;
        }
        else if(bAfter)
        {
            // after last point
            aCandidate.append(aTestPoint);

            if(aCandidate.areControlPointsUsed())
            {
                if(aCandidate.isPrevControlPointUsed(aCandidate.count() - 2))
                {
                    aCandidate.setNextControlPoint(aCandidate.count() - 2, interpolate(aCandidate.getB2DPoint(aCandidate.count() - 2), aTestPoint, (1.0 / 3.0)));
                    aCandidate.setPrevControlPoint(aCandidate.count() - 1, interpolate(aCandidate.getB2DPoint(aCandidate.count() - 2), aTestPoint, (2.0 / 3.0)));
                }
            }

            nNewHdl = aCandidate.count() - 1L;
        }
        else
        {
            // in between
            bool bSegmentSplit(false);
            const sal_uInt32 nNextIndex((nSmallestEdgeIndex + 1) % aCandidate.count());

            if(aCandidate.areControlPointsUsed())
            {
                if(aCandidate.isNextControlPointUsed(nSmallestEdgeIndex) || aCandidate.isPrevControlPointUsed(nNextIndex))
                {
                    bSegmentSplit = true;
                }
            }

            if(bSegmentSplit)
            {
                // rebuild original segment to get the split data
                basegfx::B2DCubicBezier aBezierA, aBezierB;
                const basegfx::B2DCubicBezier aBezier(
                    aCandidate.getB2DPoint(nSmallestEdgeIndex),
                    aCandidate.getNextControlPoint(nSmallestEdgeIndex),
                    aCandidate.getPrevControlPoint(nNextIndex),
                    aCandidate.getB2DPoint(nNextIndex));

                // split and insert hit point
                aBezier.split(fSmallestCut, &aBezierA, &aBezierB);
                aCandidate.insert(nSmallestEdgeIndex + 1, aTestPoint);

                // since we inserted hit point and not split point, we need to add an offset
                // to the control points to get the C1 continuity we want to achieve
                const basegfx::B2DVector aOffset(aTestPoint - aBezierA.getEndPoint());
                aCandidate.setNextControlPoint(nSmallestEdgeIndex, aBezierA.getControlPointA() + aOffset);
                aCandidate.setPrevControlPoint(nSmallestEdgeIndex + 1, aBezierA.getControlPointB() + aOffset);
                aCandidate.setNextControlPoint(nSmallestEdgeIndex + 1, aBezierB.getControlPointA() + aOffset);
                aCandidate.setPrevControlPoint((nSmallestEdgeIndex + 2) % aCandidate.count(), aBezierB.getControlPointB() + aOffset);
            }
            else
            {
                aCandidate.insert(nSmallestEdgeIndex + 1L, aTestPoint);
            }

            nNewHdl = nSmallestEdgeIndex + 1L;
        }

        maPathPolygon.setB2DPolygon(nSmallestPolyIndex, aCandidate);

        // create old polygon index from it
        for(sal_uInt32 a(0L); a < nSmallestPolyIndex; a++)
        {
            nNewHdl += GetPathPoly().getB2DPolygon(a).count();
        }
    }

    ImpForceKind();
    return nNewHdl;
}

SdrObject* SdrPathObj::RipPoint(sal_uInt32 nHdlNum, sal_uInt32& rNewPt0Index)
{
    SdrPathObj* pNewObj = 0L;
    const basegfx::B2DPolyPolygon aLocalPolyPolygon(GetPathPoly());
    sal_uInt32 nPoly, nPnt;

    if(PolyPolygonEditor::GetRelativePolyPoint(aLocalPolyPolygon, nHdlNum, nPoly, nPnt))
    {
        if(0L == nPoly)
        {
            const basegfx::B2DPolygon aCandidate(aLocalPolyPolygon.getB2DPolygon(nPoly));
            const sal_uInt32 nPointCount(aCandidate.count());

            if(nPointCount)
            {
                if(IsClosed())
                {
                    // when closed, RipPoint means to open the polygon at the selected point. To
                    // be able to do that, it is necessary to make the selected point the first one
                    basegfx::B2DPolygon aNewPolygon(basegfx::tools::makeStartPoint(aCandidate, nPnt));
                    SetPathPoly(basegfx::B2DPolyPolygon(aNewPolygon));
                    ToggleClosed();

                    // give back new position of old start point (historical reasons)
                    rNewPt0Index = (nPointCount - nPnt) % nPointCount;
                }
                else
                {
                    if(nPointCount >= 3L && nPnt != 0L && nPnt + 1L < nPointCount)
                    {
                        // split in two objects at point nPnt
                        basegfx::B2DPolygon aSplitPolyA(aCandidate, 0L, nPnt + 1L);
                        SetPathPoly(basegfx::B2DPolyPolygon(aSplitPolyA));

                        pNewObj = (SdrPathObj*)Clone();
                        basegfx::B2DPolygon aSplitPolyB(aCandidate, nPnt, nPointCount - nPnt);
                        pNewObj->SetPathPoly(basegfx::B2DPolyPolygon(aSplitPolyB));
                    }
                }
            }
        }
    }

    return pNewObj;
}

SdrObject* SdrPathObj::DoConvertToPolyObj(sal_Bool bBezier, bool bAddText) const
{
    // #i89784# check for FontWork with activated HideContour
    const drawinglayer::attribute::SdrTextAttribute aText(
        drawinglayer::primitive2d::createNewSdrTextAttribute(GetObjectItemSet(), *getText(0)));
    const bool bHideContour(
        !aText.isDefault() && !aText.getSdrFormTextAttribute().isDefault() && aText.isHideContour());

    SdrObject* pRet = bHideContour ?
        0 :
        ImpConvertMakeObj(GetPathPoly(), IsClosed(), bBezier);

    SdrPathObj* pPath = PTR_CAST(SdrPathObj, pRet);

    if(pPath)
    {
        if(pPath->GetPathPoly().areControlPointsUsed())
        {
            if(!bBezier)
            {
                // reduce all bezier curves
                pPath->SetPathPoly(basegfx::tools::adaptiveSubdivideByAngle(pPath->GetPathPoly()));
            }
        }
        else
        {
            if(bBezier)
            {
                // create bezier curves
                pPath->SetPathPoly(basegfx::tools::expandToCurve(pPath->GetPathPoly()));
            }
        }
    }

    if(bAddText)
    {
        pRet = ImpConvertAddText(pRet, bBezier);
    }

    return pRet;
}

SdrObjGeoData* SdrPathObj::NewGeoData() const
{
    return new SdrPathObjGeoData;
}

void SdrPathObj::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrTextObj::SaveGeoData(rGeo);
    SdrPathObjGeoData& rPGeo = (SdrPathObjGeoData&) rGeo;
    rPGeo.maPathPolygon=GetPathPoly();
    rPGeo.meKind=meKind;
}

void SdrPathObj::RestGeoData(const SdrObjGeoData& rGeo)
{
    SdrTextObj::RestGeoData(rGeo);
    SdrPathObjGeoData& rPGeo=(SdrPathObjGeoData&)rGeo;
    maPathPolygon=rPGeo.maPathPolygon;
    meKind=rPGeo.meKind;
    ImpForceKind(); // to set bClosed (among other things)
}

void SdrPathObj::NbcSetPathPoly(const basegfx::B2DPolyPolygon& rPathPoly)
{
    if(GetPathPoly() != rPathPoly)
    {
        maPathPolygon=rPathPoly;
        ImpForceKind();
        SetRectsDirty();
    }
}

void SdrPathObj::SetPathPoly(const basegfx::B2DPolyPolygon& rPathPoly)
{
    if(GetPathPoly() != rPathPoly)
    {
        Rectangle aBoundRect0; if (pUserCall!=NULL) aBoundRect0=GetLastBoundRect();
        NbcSetPathPoly(rPathPoly);
        SetChanged();
        BroadcastObjectChange();
        SendUserCall(SDRUSERCALL_RESIZE,aBoundRect0);
    }
}

void SdrPathObj::ToggleClosed()
{
    Rectangle aBoundRect0;
    if(pUserCall != NULL)
        aBoundRect0 = GetLastBoundRect();
    ImpSetClosed(!IsClosed()); // set new ObjKind
    ImpForceKind(); // because we want Line -> Poly -> PolyLine instead of Line -> Poly -> Line
    SetRectsDirty();
    SetChanged();
    BroadcastObjectChange();
    SendUserCall(SDRUSERCALL_RESIZE, aBoundRect0);
}

// for friend class SdrPolyEditView in some compilers:
void SdrPathObj::SetRectsDirty(sal_Bool bNotMyself)
{
    SdrTextObj::SetRectsDirty(bNotMyself);
}

ImpPathForDragAndCreate& SdrPathObj::impGetDAC() const
{
    if(!mpDAC)
    {
        ((SdrPathObj*)this)->mpDAC = new ImpPathForDragAndCreate(*((SdrPathObj*)this));
    }

    return *mpDAC;
}

void SdrPathObj::impDeleteDAC() const
{
    if(mpDAC)
    {
        delete mpDAC;
        ((SdrPathObj*)this)->mpDAC = 0L;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// transformation interface for StarOfficeAPI. This implements support for
// homogeneous 3x3 matrices containing the transformation of the SdrObject. At the
// moment it contains a shearX, rotation and translation, but for setting all linear
// transforms like Scale, ShearX, ShearY, Rotate and Translate are supported.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
// gets base transformation and rectangle of object. If it's an SdrPathObj it fills the PolyPolygon
// with the base geometry and returns TRUE. Otherwise it returns FALSE.
sal_Bool SdrPathObj::TRGetBaseGeometry(basegfx::B2DHomMatrix& rMatrix, basegfx::B2DPolyPolygon& rPolyPolygon) const
{
    double fRotate(0.0);
    double fShearX(0.0);
    basegfx::B2DTuple aScale(1.0, 1.0);
    basegfx::B2DTuple aTranslate(0.0, 0.0);

    if(GetPathPoly().count())
    {
        // copy geometry
        basegfx::B2DHomMatrix aMoveToZeroMatrix;
        rPolyPolygon = GetPathPoly();

        if(OBJ_LINE == meKind)
        {
            // ignore shear and rotate, just use scale and translate
            OSL_ENSURE(GetPathPoly().count() > 0L && GetPathPoly().getB2DPolygon(0L).count() > 1L, "OBJ_LINE with too few polygons (!)");
            // #i72287# use polygon without control points for range calculation. Do not change rPolyPolygon
            // itself, else this method will no longer return the full polygon information (curve will
            // be lost)
            const basegfx::B2DRange aPolyRangeNoCurve(basegfx::tools::getRange(rPolyPolygon));
            aScale = aPolyRangeNoCurve.getRange();
            aTranslate = aPolyRangeNoCurve.getMinimum();

            // define matrix for move polygon to zero point
            aMoveToZeroMatrix.translate(-aTranslate.getX(), -aTranslate.getY());
        }
        else
        {
            if(aGeo.nShearWink || aGeo.nDrehWink)
            {
                // get rotate and shear in drawingLayer notation
                fRotate = aGeo.nDrehWink * F_PI18000;
                fShearX = aGeo.nShearWink * F_PI18000;

                // build mathematically correct (negative shear and rotate) object transform
                // containing shear and rotate to extract unsheared, unrotated polygon
                basegfx::B2DHomMatrix aObjectMatrix;
                aObjectMatrix.shearX(tan((36000 - aGeo.nShearWink) * F_PI18000));
                aObjectMatrix.rotate((36000 - aGeo.nDrehWink) * F_PI18000);

                // create inverse from it and back-transform polygon
                basegfx::B2DHomMatrix aInvObjectMatrix(aObjectMatrix);
                aInvObjectMatrix.invert();
                rPolyPolygon.transform(aInvObjectMatrix);

                // get range from unsheared, unrotated polygon and extract scale and translate.
                // transform topLeft from it back to transformed state to get original
                // topLeft (rotation center)
                // #i72287# use polygon without control points for range calculation. Do not change rPolyPolygon
                // itself, else this method will no longer return the full polygon information (curve will
                // be lost)
                const basegfx::B2DRange aCorrectedRangeNoCurve(basegfx::tools::getRange(rPolyPolygon));
                aTranslate = aObjectMatrix * aCorrectedRangeNoCurve.getMinimum();
                aScale = aCorrectedRangeNoCurve.getRange();

                // define matrix for move polygon to zero point
                // #i112280# Added missing minus for Y-Translation
                aMoveToZeroMatrix.translate(-aCorrectedRangeNoCurve.getMinX(), -aCorrectedRangeNoCurve.getMinY());
            }
            else
            {
                // get scale and translate from unsheared, unrotated polygon
                // #i72287# use polygon without control points for range calculation. Do not change rPolyPolygon
                // itself, else this method will no longer return the full polygon information (curve will
                // be lost)
                const basegfx::B2DRange aPolyRangeNoCurve(basegfx::tools::getRange(rPolyPolygon));
                aScale = aPolyRangeNoCurve.getRange();
                aTranslate = aPolyRangeNoCurve.getMinimum();

                // define matrix for move polygon to zero point
                aMoveToZeroMatrix.translate(-aTranslate.getX(), -aTranslate.getY());
            }
        }

        // move polygon to zero point with pre-defined matrix
        rPolyPolygon.transform(aMoveToZeroMatrix);
    }

    // position maybe relative to anchorpos, convert
    if( pModel && pModel->IsWriter() )
    {
        if(GetAnchorPos().X() || GetAnchorPos().Y())
        {
            aTranslate -= basegfx::B2DTuple(GetAnchorPos().X(), GetAnchorPos().Y());
        }
    }

    // force MapUnit to 100th mm
    const SfxMapUnit eMapUnit(GetObjectMapUnit());
    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
    {
        switch(eMapUnit)
        {
            case SFX_MAPUNIT_TWIP :
            {
                // position
                aTranslate.setX(ImplTwipsToMM(aTranslate.getX()));
                aTranslate.setY(ImplTwipsToMM(aTranslate.getY()));

                // size
                aScale.setX(ImplTwipsToMM(aScale.getX()));
                aScale.setY(ImplTwipsToMM(aScale.getY()));

                // polygon
                basegfx::B2DHomMatrix aTwipsToMM;
                const double fFactorTwipsToMM(127.0 / 72.0);
                aTwipsToMM.scale(fFactorTwipsToMM, fFactorTwipsToMM);
                rPolyPolygon.transform(aTwipsToMM);

                break;
            }
            default:
            {
                OSL_FAIL("TRGetBaseGeometry: Missing unit translation to 100th mm!");
            }
        }
    }

    // build return value matrix
    rMatrix = basegfx::tools::createScaleShearXRotateTranslateB2DHomMatrix(
        aScale,
        basegfx::fTools::equalZero(fShearX) ? 0.0 : tan(fShearX),
        basegfx::fTools::equalZero(fRotate) ? 0.0 : -fRotate,
        aTranslate);

    return sal_True;
}

// Sets the base geometry of the object using infos contained in the homogeneous 3x3 matrix.
// If it's an SdrPathObj it will use the provided geometry information. The Polygon has
// to use (0,0) as upper left and will be scaled to the given size in the matrix.
void SdrPathObj::TRSetBaseGeometry(const basegfx::B2DHomMatrix& rMatrix, const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    // break up matrix
    basegfx::B2DTuple aScale;
    basegfx::B2DTuple aTranslate;
    double fRotate, fShearX;
    rMatrix.decompose(aScale, aTranslate, fRotate, fShearX);

    // #i75086# Old DrawingLayer (GeoStat and geometry) does not support holding negative scalings
    // in X and Y which equal a 180 degree rotation. Recognize it and react accordingly
    if(basegfx::fTools::less(aScale.getX(), 0.0) && basegfx::fTools::less(aScale.getY(), 0.0))
    {
        aScale.setX(fabs(aScale.getX()));
        aScale.setY(fabs(aScale.getY()));
        fRotate = fmod(fRotate + F_PI, F_2PI);
    }

    // copy poly
    basegfx::B2DPolyPolygon aNewPolyPolygon(rPolyPolygon);

    // reset object shear and rotations
    aGeo.nDrehWink = 0;
    aGeo.RecalcSinCos();
    aGeo.nShearWink = 0;
    aGeo.RecalcTan();

    // force metric to pool metric
    const SfxMapUnit eMapUnit(GetObjectMapUnit());
    if(eMapUnit != SFX_MAPUNIT_100TH_MM)
    {
        switch(eMapUnit)
        {
            case SFX_MAPUNIT_TWIP :
            {
                // position
                aTranslate.setX(ImplMMToTwips(aTranslate.getX()));
                aTranslate.setY(ImplMMToTwips(aTranslate.getY()));

                // size
                aScale.setX(ImplMMToTwips(aScale.getX()));
                aScale.setY(ImplMMToTwips(aScale.getY()));

                // polygon
                basegfx::B2DHomMatrix aMMToTwips;
                const double fFactorMMToTwips(72.0 / 127.0);
                aMMToTwips.scale(fFactorMMToTwips, fFactorMMToTwips);
                aNewPolyPolygon.transform(aMMToTwips);

                break;
            }
            default:
            {
                OSL_FAIL("TRSetBaseGeometry: Missing unit translation to PoolMetric!");
            }
        }
    }

    if( pModel && pModel->IsWriter() )
    {
        // if anchor is used, make position relative to it
        if(GetAnchorPos().X() || GetAnchorPos().Y())
        {
            aTranslate += basegfx::B2DTuple(GetAnchorPos().X(), GetAnchorPos().Y());
        }
    }

    // create transformation for polygon, set values at aGeo direct
    basegfx::B2DHomMatrix aTransform;

    // #i75086#
    // Given polygon is already scaled (for historical reasons), but not mirrored yet.
    // Thus, when scale is negative in X or Y, apply the needed mirroring accordingly.
    if(basegfx::fTools::less(aScale.getX(), 0.0) || basegfx::fTools::less(aScale.getY(), 0.0))
    {
        aTransform.scale(
            basegfx::fTools::less(aScale.getX(), 0.0) ? -1.0 : 1.0,
            basegfx::fTools::less(aScale.getY(), 0.0) ? -1.0 : 1.0);
    }

    if(!basegfx::fTools::equalZero(fShearX))
    {
        aTransform.shearX(tan(-atan(fShearX)));
        aGeo.nShearWink = FRound(atan(fShearX) / F_PI18000);
        aGeo.RecalcTan();
    }

    if(!basegfx::fTools::equalZero(fRotate))
    {
        // #i78696#
        // fRotate is mathematically correct for linear transformations, so it's
        // the one to use for the geometry change
        aTransform.rotate(fRotate);

        // #i78696#
        // fRotate is mathematically correct, but aGeoStat.nDrehWink is
        // mirrored -> mirror value here
        aGeo.nDrehWink = NormAngle360(FRound(-fRotate / F_PI18000));
        aGeo.RecalcSinCos();
    }

    if(!aTranslate.equalZero())
    {
        // #i39529# absolute positioning, so get current position (without control points (!))
        const basegfx::B2DRange aCurrentRange(basegfx::tools::getRange(aNewPolyPolygon));
        aTransform.translate(aTranslate.getX() - aCurrentRange.getMinX(), aTranslate.getY() - aCurrentRange.getMinY());
    }

    // transform polygon and trigger change
    aNewPolyPolygon.transform(aTransform);
    SetPathPoly(aNewPolyPolygon);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
