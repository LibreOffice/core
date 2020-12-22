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


#include <svx/svdtrans.hxx>
#include <math.h>
#include <svx/xpoly.hxx>
#include <rtl/ustrbuf.hxx>

#include <vcl/virdev.hxx>
#include <tools/bigint.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/localedatawrapper.hxx>
#include <sal/log.hxx>

void MoveXPoly(XPolygon& rPoly, const Size& S)
{
    rPoly.Move(S.Width(),S.Height());
}

void ResizeRect(tools::Rectangle& rRect, const Point& rRef, const Fraction& rxFact, const Fraction& ryFact)
{
    Fraction aXFact(rxFact);
    Fraction aYFact(ryFact);

    if (!aXFact.IsValid()) {
        SAL_WARN( "svx.svdraw", "invalid fraction xFract, using Fraction(1,1)" );
        aXFact = Fraction(1,1);
        tools::Long nWdt = rRect.Right() - rRect.Left();
        if (nWdt == 0) rRect.AdjustRight( 1 );
    }
    rRect.SetLeft( rRef.X() + FRound( (rRect.Left()  - rRef.X()) * double(aXFact) ) );
    rRect.SetRight( rRef.X() + FRound( (rRect.Right() - rRef.X()) * double(aXFact) ) );

    if (!aYFact.IsValid()) {
        SAL_WARN( "svx.svdraw", "invalid fraction yFract, using Fraction(1,1)" );
        aYFact = Fraction(1,1);
        tools::Long nHgt = rRect.Bottom() - rRect.Top();
        if (nHgt == 0) rRect.AdjustBottom( 1 );
    }
    rRect.SetTop( rRef.Y() + FRound( (rRect.Top()    - rRef.Y()) * double(aYFact) ) );
    rRect.SetBottom( rRef.Y() + FRound( (rRect.Bottom() - rRef.Y()) * double(aYFact) ) );

    rRect.Justify();
}


void ResizePoly(tools::Polygon& rPoly, const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    sal_uInt16 nCount=rPoly.GetSize();
    for (sal_uInt16 i=0; i<nCount; i++) {
        ResizePoint(rPoly[i],rRef,xFact,yFact);
    }
}

void ResizeXPoly(XPolygon& rPoly, const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    sal_uInt16 nCount=rPoly.GetPointCount();
    for (sal_uInt16 i=0; i<nCount; i++) {
        ResizePoint(rPoly[i],rRef,xFact,yFact);
    }
}

void RotatePoly(tools::Polygon& rPoly, const Point& rRef, double sn, double cs)
{
    sal_uInt16 nCount=rPoly.GetSize();
    for (sal_uInt16 i=0; i<nCount; i++) {
        RotatePoint(rPoly[i],rRef,sn,cs);
    }
}

void RotateXPoly(XPolygon& rPoly, const Point& rRef, double sn, double cs)
{
    sal_uInt16 nCount=rPoly.GetPointCount();
    for (sal_uInt16 i=0; i<nCount; i++) {
        RotatePoint(rPoly[i],rRef,sn,cs);
    }
}

void RotateXPoly(XPolyPolygon& rPoly, const Point& rRef, double sn, double cs)
{
    sal_uInt16 nCount=rPoly.Count();
    for (sal_uInt16 i=0; i<nCount; i++) {
        RotateXPoly(rPoly[i],rRef,sn,cs);
    }
}

void MirrorPoint(Point& rPnt, const Point& rRef1, const Point& rRef2)
{
    tools::Long mx=rRef2.X()-rRef1.X();
    tools::Long my=rRef2.Y()-rRef1.Y();
    if (mx==0) { // vertical axis
        tools::Long dx=rRef1.X()-rPnt.X();
        rPnt.AdjustX(2*dx );
    } else if (my==0) { // horizontal axis
        tools::Long dy=rRef1.Y()-rPnt.Y();
        rPnt.AdjustY(2*dy );
    } else if (mx==my) { // diagonal axis '\'
        tools::Long dx1=rPnt.X()-rRef1.X();
        tools::Long dy1=rPnt.Y()-rRef1.Y();
        rPnt.setX(rRef1.X()+dy1 );
        rPnt.setY(rRef1.Y()+dx1 );
    } else if (mx==-my) { // diagonal axis '/'
        tools::Long dx1=rPnt.X()-rRef1.X();
        tools::Long dy1=rPnt.Y()-rRef1.Y();
        rPnt.setX(rRef1.X()-dy1 );
        rPnt.setY(rRef1.Y()-dx1 );
    } else { // arbitrary axis
        // TODO: Optimize this! Raise perpendicular on the mirroring axis..?
        tools::Long nRefWink=GetAngle(rRef2-rRef1);
        rPnt-=rRef1;
        tools::Long nPntWink=GetAngle(rPnt);
        tools::Long nAngle=2*(nRefWink-nPntWink);
        double a = nAngle * F_PI18000;
        double nSin=sin(a);
        double nCos=cos(a);
        RotatePoint(rPnt,Point(),nSin,nCos);
        rPnt+=rRef1;
    }
}

void MirrorXPoly(XPolygon& rPoly, const Point& rRef1, const Point& rRef2)
{
    sal_uInt16 nCount=rPoly.GetPointCount();
    for (sal_uInt16 i=0; i<nCount; i++) {
        MirrorPoint(rPoly[i],rRef1,rRef2);
    }
}

void ShearPoly(tools::Polygon& rPoly, const Point& rRef, double tn)
{
    sal_uInt16 nCount=rPoly.GetSize();
    for (sal_uInt16 i=0; i<nCount; i++) {
        ShearPoint(rPoly[i],rRef,tn);
    }
}

void ShearXPoly(XPolygon& rPoly, const Point& rRef, double tn, bool bVShear)
{
    sal_uInt16 nCount=rPoly.GetPointCount();
    for (sal_uInt16 i=0; i<nCount; i++) {
        ShearPoint(rPoly[i],rRef,tn,bVShear);
    }
}

double CrookRotateXPoint(Point& rPnt, Point* pC1, Point* pC2, const Point& rCenter,
                         const Point& rRad, double& rSin, double& rCos, bool bVert)
{
    bool bC1=pC1!=nullptr;
    bool bC2=pC2!=nullptr;
    tools::Long x0=rPnt.X();
    tools::Long y0=rPnt.Y();
    tools::Long cx=rCenter.X();
    tools::Long cy=rCenter.Y();
    double nAngle=GetCrookAngle(rPnt,rCenter,rRad,bVert);
    double sn=sin(nAngle);
    double cs=cos(nAngle);
    RotatePoint(rPnt,rCenter,sn,cs);
    if (bC1) {
        if (bVert) {
            // move into the direction of the center, as a basic position for the rotation
            pC1->AdjustY( -y0 );
            // resize, account for the distance from the center
            pC1->setY(FRound(static_cast<double>(pC1->Y()) /rRad.X()*(cx-pC1->X())) );
            pC1->AdjustY(cy );
        } else {
            // move into the direction of the center, as a basic position for the rotation
            pC1->AdjustX( -x0 );
            // resize, account for the distance from the center
            tools::Long nPntRad=cy-pC1->Y();
            double nFact=static_cast<double>(nPntRad)/static_cast<double>(rRad.Y());
            pC1->setX(FRound(static_cast<double>(pC1->X())*nFact) );
            pC1->AdjustX(cx );
        }
        RotatePoint(*pC1,rCenter,sn,cs);
    }
    if (bC2) {
        if (bVert) {
            // move into the direction of the center, as a basic position for the rotation
            pC2->AdjustY( -y0 );
            // resize, account for the distance from the center
            pC2->setY(FRound(static_cast<double>(pC2->Y()) /rRad.X()*(rCenter.X()-pC2->X())) );
            pC2->AdjustY(cy );
        } else {
            // move into the direction of the center, as a basic position for the rotation
            pC2->AdjustX( -x0 );
            // resize, account for the distance from the center
            tools::Long nPntRad=rCenter.Y()-pC2->Y();
            double nFact=static_cast<double>(nPntRad)/static_cast<double>(rRad.Y());
            pC2->setX(FRound(static_cast<double>(pC2->X())*nFact) );
            pC2->AdjustX(cx );
        }
        RotatePoint(*pC2,rCenter,sn,cs);
    }
    rSin=sn;
    rCos=cs;
    return nAngle;
}

double CrookSlantXPoint(Point& rPnt, Point* pC1, Point* pC2, const Point& rCenter,
                        const Point& rRad, double& rSin, double& rCos, bool bVert)
{
    bool bC1=pC1!=nullptr;
    bool bC2=pC2!=nullptr;
    tools::Long x0=rPnt.X();
    tools::Long y0=rPnt.Y();
    tools::Long dx1=0,dy1=0;
    tools::Long dxC1=0,dyC1=0;
    tools::Long dxC2=0,dyC2=0;
    if (bVert) {
        tools::Long nStart=rCenter.X()-rRad.X();
        dx1=rPnt.X()-nStart;
        rPnt.setX(nStart );
        if (bC1) {
            dxC1=pC1->X()-nStart;
            pC1->setX(nStart );
        }
        if (bC2) {
            dxC2=pC2->X()-nStart;
            pC2->setX(nStart );
        }
    } else {
        tools::Long nStart=rCenter.Y()-rRad.Y();
        dy1=rPnt.Y()-nStart;
        rPnt.setY(nStart );
        if (bC1) {
            dyC1=pC1->Y()-nStart;
            pC1->setY(nStart );
        }
        if (bC2) {
            dyC2=pC2->Y()-nStart;
            pC2->setY(nStart );
        }
    }
    double nAngle=GetCrookAngle(rPnt,rCenter,rRad,bVert);
    double sn=sin(nAngle);
    double cs=cos(nAngle);
    RotatePoint(rPnt,rCenter,sn,cs);
    if (bC1) { if (bVert) pC1->AdjustY( -(y0-rCenter.Y()) ); else pC1->AdjustX( -(x0-rCenter.X()) ); RotatePoint(*pC1,rCenter,sn,cs); }
    if (bC2) { if (bVert) pC2->AdjustY( -(y0-rCenter.Y()) ); else pC2->AdjustX( -(x0-rCenter.X()) ); RotatePoint(*pC2,rCenter,sn,cs); }
    if (bVert) {
        rPnt.AdjustX(dx1 );
        if (bC1) pC1->AdjustX(dxC1 );
        if (bC2) pC2->AdjustX(dxC2 );
    } else {
        rPnt.AdjustY(dy1 );
        if (bC1) pC1->AdjustY(dyC1 );
        if (bC2) pC2->AdjustY(dyC2 );
    }
    rSin=sn;
    rCos=cs;
    return nAngle;
}

double CrookStretchXPoint(Point& rPnt, Point* pC1, Point* pC2, const Point& rCenter,
                          const Point& rRad, double& rSin, double& rCos, bool bVert,
                          const tools::Rectangle& rRefRect)
{
    tools::Long y0=rPnt.Y();
    CrookSlantXPoint(rPnt,pC1,pC2,rCenter,rRad,rSin,rCos,bVert);
    if (bVert) {
    } else {
        tools::Long nTop=rRefRect.Top();
        tools::Long nBtm=rRefRect.Bottom();
        tools::Long nHgt=nBtm-nTop;
        tools::Long dy=rPnt.Y()-y0;
        double a=static_cast<double>(y0-nTop)/nHgt;
        a*=dy;
        rPnt.setY(y0+FRound(a) );
    }
    return 0.0;
}


void CrookRotatePoly(XPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert)
{
    double nSin,nCos;
    sal_uInt16 nPointCnt=rPoly.GetPointCount();
    sal_uInt16 i=0;
    while (i<nPointCnt) {
        Point* pPnt=&rPoly[i];
        Point* pC1=nullptr;
        Point* pC2=nullptr;
        if (i+1<nPointCnt && rPoly.IsControl(i)) { // control point to the left
            pC1=pPnt;
            i++;
            pPnt=&rPoly[i];
        }
        i++;
        if (i<nPointCnt && rPoly.IsControl(i)) { // control point to the right
            pC2=&rPoly[i];
            i++;
        }
        CrookRotateXPoint(*pPnt,pC1,pC2,rCenter,rRad,nSin,nCos,bVert);
    }
}

void CrookSlantPoly(XPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert)
{
    double nSin,nCos;
    sal_uInt16 nPointCnt=rPoly.GetPointCount();
    sal_uInt16 i=0;
    while (i<nPointCnt) {
        Point* pPnt=&rPoly[i];
        Point* pC1=nullptr;
        Point* pC2=nullptr;
        if (i+1<nPointCnt && rPoly.IsControl(i)) { // control point to the left
            pC1=pPnt;
            i++;
            pPnt=&rPoly[i];
        }
        i++;
        if (i<nPointCnt && rPoly.IsControl(i)) { // control point to the right
            pC2=&rPoly[i];
            i++;
        }
        CrookSlantXPoint(*pPnt,pC1,pC2,rCenter,rRad,nSin,nCos,bVert);
    }
}

void CrookStretchPoly(XPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert, const tools::Rectangle& rRefRect)
{
    double nSin,nCos;
    sal_uInt16 nPointCnt=rPoly.GetPointCount();
    sal_uInt16 i=0;
    while (i<nPointCnt) {
        Point* pPnt=&rPoly[i];
        Point* pC1=nullptr;
        Point* pC2=nullptr;
        if (i+1<nPointCnt && rPoly.IsControl(i)) { //  control point to the left
            pC1=pPnt;
            i++;
            pPnt=&rPoly[i];
        }
        i++;
        if (i<nPointCnt && rPoly.IsControl(i)) { // control point to the right
            pC2=&rPoly[i];
            i++;
        }
        CrookStretchXPoint(*pPnt,pC1,pC2,rCenter,rRad,nSin,nCos,bVert,rRefRect);
    }
}


void CrookRotatePoly(XPolyPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert)
{
    sal_uInt16 nPolyCount=rPoly.Count();
    for (sal_uInt16 nPolyNum=0; nPolyNum<nPolyCount; nPolyNum++) {
        CrookRotatePoly(rPoly[nPolyNum],rCenter,rRad,bVert);
    }
}

void CrookSlantPoly(XPolyPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert)
{
    sal_uInt16 nPolyCount=rPoly.Count();
    for (sal_uInt16 nPolyNum=0; nPolyNum<nPolyCount; nPolyNum++) {
        CrookSlantPoly(rPoly[nPolyNum],rCenter,rRad,bVert);
    }
}

void CrookStretchPoly(XPolyPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert, const tools::Rectangle& rRefRect)
{
    sal_uInt16 nPolyCount=rPoly.Count();
    for (sal_uInt16 nPolyNum=0; nPolyNum<nPolyCount; nPolyNum++) {
        CrookStretchPoly(rPoly[nPolyNum],rCenter,rRad,bVert,rRefRect);
    }
}


tools::Long GetAngle(const Point& rPnt)
{
    tools::Long a=0;
    if (rPnt.Y()==0) {
        if (rPnt.X()<0) a=-18000;
    } else if (rPnt.X()==0) {
        if (rPnt.Y()>0) a=-9000;
        else a=9000;
    } else {
        a = FRound(atan2(static_cast<double>(-rPnt.Y()), static_cast<double>(rPnt.X()))
                   / F_PI18000);
    }
    return a;
}

tools::Long NormAngle18000(tools::Long a)
{
    while (a<-18000) a+=36000;
    while (a>=18000) a-=36000;
    return a;
}

tools::Long NormAngle36000(tools::Long a)
{
    while (a<0) a+=36000;
    while (a>=36000) a-=36000;
    return a;
}

sal_uInt16 GetAngleSector(tools::Long nAngle)
{
    while (nAngle<0) nAngle+=36000;
    while (nAngle>=36000) nAngle-=36000;
    if (nAngle< 9000) return 0;
    if (nAngle<18000) return 1;
    if (nAngle<27000) return 2;
    return 3;
}

tools::Long GetLen(const Point& rPnt)
{
    tools::Long x=std::abs(rPnt.X());
    tools::Long y=std::abs(rPnt.Y());
    if (x+y<0x8000) { // because 7FFF * 7FFF * 2 = 7FFE0002
        x*=x;
        y*=y;
        x+=y;
        x=FRound(sqrt(static_cast<double>(x)));
        return x;
    } else {
        double nx=x;
        double ny=y;
        nx*=nx;
        ny*=ny;
        nx+=ny;
        nx=sqrt(nx);
        if (nx>0x7FFFFFFF) {
            return 0x7FFFFFFF; // we can't go any further, for fear of an overrun!
        } else {
            return FRound(nx);
        }
    }
}


void GeoStat::RecalcSinCos()
{
    if (nRotationAngle==0) {
        mfSinRotationAngle=0.0;
        mfCosRotationAngle=1.0;
    } else {
        double a = nRotationAngle * F_PI18000;
        mfSinRotationAngle=sin(a);
        mfCosRotationAngle=cos(a);
    }
}

void GeoStat::RecalcTan()
{
    if (nShearAngle==0) {
        mfTanShearAngle=0.0;
    } else {
        double a = nShearAngle * F_PI18000;
        mfTanShearAngle=tan(a);
    }
}


tools::Polygon Rect2Poly(const tools::Rectangle& rRect, const GeoStat& rGeo)
{
    tools::Polygon aPol(5);
    aPol[0]=rRect.TopLeft();
    aPol[1]=rRect.TopRight();
    aPol[2]=rRect.BottomRight();
    aPol[3]=rRect.BottomLeft();
    aPol[4]=rRect.TopLeft();
    if (rGeo.nShearAngle!=0) ShearPoly(aPol,rRect.TopLeft(),rGeo.mfTanShearAngle);
    if (rGeo.nRotationAngle!=0) RotatePoly(aPol,rRect.TopLeft(),rGeo.mfSinRotationAngle,rGeo.mfCosRotationAngle);
    return aPol;
}

void Poly2Rect(const tools::Polygon& rPol, tools::Rectangle& rRect, GeoStat& rGeo)
{
    rGeo.nRotationAngle=GetAngle(rPol[1]-rPol[0]);
    rGeo.nRotationAngle=NormAngle36000(rGeo.nRotationAngle);
    // rotation successful
    rGeo.RecalcSinCos();

    Point aPt1(rPol[1]-rPol[0]);
    if (rGeo.nRotationAngle!=0) RotatePoint(aPt1,Point(0,0),-rGeo.mfSinRotationAngle,rGeo.mfCosRotationAngle); // -Sin to reverse rotation
    tools::Long nWdt=aPt1.X();

    Point aPt0(rPol[0]);
    Point aPt3(rPol[3]-rPol[0]);
    if (rGeo.nRotationAngle!=0) RotatePoint(aPt3,Point(0,0),-rGeo.mfSinRotationAngle,rGeo.mfCosRotationAngle); // -Sin to reverse rotation
    tools::Long nHgt=aPt3.Y();


    tools::Long nShW=GetAngle(aPt3);
    nShW-=27000; // ShearWink is measured against a vertical line
    nShW=-nShW;  // negating, because '+' is shearing clock-wise

    bool bMirr=aPt3.Y()<0;
    if (bMirr) { // "exchange of points" when mirroring
        nHgt=-nHgt;
        nShW+=18000;
        aPt0=rPol[3];
    }
    nShW=NormAngle18000(nShW);
    if (nShW<-9000 || nShW>9000) {
        nShW=NormAngle18000(nShW+18000);
    }
    if (nShW<-SDRMAXSHEAR) nShW=-SDRMAXSHEAR; // limit ShearWinkel (shear angle) to +/- 89.00 deg
    if (nShW>SDRMAXSHEAR)  nShW=SDRMAXSHEAR;
    rGeo.nShearAngle=nShW;
    rGeo.RecalcTan();
    Point aRU(aPt0);
    aRU.AdjustX(nWdt );
    aRU.AdjustY(nHgt );
    rRect=tools::Rectangle(aPt0,aRU);
}


void OrthoDistance8(const Point& rPt0, Point& rPt, bool bBigOrtho)
{
    tools::Long dx=rPt.X()-rPt0.X();
    tools::Long dy=rPt.Y()-rPt0.Y();
    tools::Long dxa=std::abs(dx);
    tools::Long dya=std::abs(dy);
    if (dx==0 || dy==0 || dxa==dya) return;
    if (dxa>=dya*2) { rPt.setY(rPt0.Y() ); return; }
    if (dya>=dxa*2) { rPt.setX(rPt0.X() ); return; }
    if ((dxa<dya) != bBigOrtho) {
        rPt.setY(rPt0.Y()+(dxa* (dy>=0 ? 1 : -1) ) );
    } else {
        rPt.setX(rPt0.X()+(dya* (dx>=0 ? 1 : -1) ) );
    }
}

void OrthoDistance4(const Point& rPt0, Point& rPt, bool bBigOrtho)
{
    tools::Long dx=rPt.X()-rPt0.X();
    tools::Long dy=rPt.Y()-rPt0.Y();
    tools::Long dxa=std::abs(dx);
    tools::Long dya=std::abs(dy);
    if ((dxa<dya) != bBigOrtho) {
        rPt.setY(rPt0.Y()+(dxa* (dy>=0 ? 1 : -1) ) );
    } else {
        rPt.setX(rPt0.X()+(dya* (dx>=0 ? 1 : -1) ) );
    }
}


tools::Long BigMulDiv(tools::Long nVal, tools::Long nMul, tools::Long nDiv)
{
    BigInt aVal(nVal);
    aVal*=nMul;
    if (aVal.IsNeg()!=(nDiv<0)) {
        aVal-=nDiv/2; // to round correctly
    } else {
        aVal+=nDiv/2; // to round correctly
    }
    if(nDiv)
    {
        aVal/=nDiv;
        return tools::Long(aVal);
    }
    return 0x7fffffff;
}

// How many eU units fit into a mm, respectively an inch?
// Or: How many mm, respectively inches, are there in an eU (and then give me the inverse)

static FrPair GetInchOrMM(MapUnit eU)
{
    switch (eU) {
        case MapUnit::Map1000thInch: return FrPair(1000,1);
        case MapUnit::Map100thInch : return FrPair( 100,1);
        case MapUnit::Map10thInch  : return FrPair(  10,1);
        case MapUnit::MapInch       : return FrPair(   1,1);
        case MapUnit::MapPoint      : return FrPair(  72,1);
        case MapUnit::MapTwip       : return FrPair(1440,1);
        case MapUnit::Map100thMM   : return FrPair( 100,1);
        case MapUnit::Map10thMM    : return FrPair(  10,1);
        case MapUnit::MapMM         : return FrPair(   1,1);
        case MapUnit::MapCM         : return FrPair(   1,10);
        case MapUnit::MapPixel      : {
            ScopedVclPtrInstance< VirtualDevice > pVD;
            pVD->SetMapMode(MapMode(MapUnit::Map100thMM));
            Point aP(pVD->PixelToLogic(Point(64,64))); // 64 pixels for more accuracy
            return FrPair(6400,aP.X(),6400,aP.Y());
        }
        case MapUnit::MapAppFont: case MapUnit::MapSysFont: {
            ScopedVclPtrInstance< VirtualDevice > pVD;
            pVD->SetMapMode(MapMode(eU));
            Point aP(pVD->LogicToPixel(Point(32,32))); // 32 units for more accuracy
            pVD->SetMapMode(MapMode(MapUnit::Map100thMM));
            aP=pVD->PixelToLogic(aP);
            return FrPair(3200,aP.X(),3200,aP.Y());
        }
        default: break;
    }
    return Fraction(1,1);
}

static FrPair GetInchOrMM(FieldUnit eU)
{
    switch (eU) {
        case FieldUnit::INCH       : return FrPair(   1,1);
        case FieldUnit::POINT      : return FrPair(  72,1);
        case FieldUnit::TWIP       : return FrPair(1440,1);
        case FieldUnit::MM_100TH   : return FrPair( 100,1);
        case FieldUnit::MM         : return FrPair(   1,1);
        case FieldUnit::CM         : return FrPair(   1,10);
        case FieldUnit::M          : return FrPair(   1,1000);
        case FieldUnit::KM         : return FrPair(   1,1000000);
        case FieldUnit::PICA       : return FrPair(   6,1);
        case FieldUnit::FOOT       : return FrPair(   1,12);
        case FieldUnit::MILE       : return FrPair(   1,63360);
        default: break;
    }
    return Fraction(1,1);
}

// Calculate the factor that we need to convert units from eS to eD.
// e. g. GetMapFactor(UNIT_MM,UNIT_100TH_MM) => 100.

FrPair GetMapFactor(MapUnit eS, MapUnit eD)
{
    if (eS==eD) return FrPair(1,1,1,1);
    FrPair aS(GetInchOrMM(eS));
    FrPair aD(GetInchOrMM(eD));
    bool bSInch=IsInch(eS);
    bool bDInch=IsInch(eD);
    FrPair aRet(aD.X()/aS.X(),aD.Y()/aS.Y());
    if (bSInch && !bDInch) { aRet.X()*=Fraction(127,5); aRet.Y()*=Fraction(127,5); }
    if (!bSInch && bDInch) { aRet.X()*=Fraction(5,127); aRet.Y()*=Fraction(5,127); }
    return aRet;
};

FrPair GetMapFactor(FieldUnit eS, FieldUnit eD)
{
    if (eS==eD) return FrPair(1,1,1,1);
    FrPair aS(GetInchOrMM(eS));
    FrPair aD(GetInchOrMM(eD));
    bool bSInch=IsInch(eS);
    bool bDInch=IsInch(eD);
    FrPair aRet(aD.X()/aS.X(),aD.Y()/aS.Y());
    if (bSInch && !bDInch) { aRet.X()*=Fraction(127,5); aRet.Y()*=Fraction(127,5); }
    if (!bSInch && bDInch) { aRet.X()*=Fraction(5,127); aRet.Y()*=Fraction(5,127); }
    return aRet;
};


    // 1 mile    =  8 furlong = 63.360" = 1.609.344,0mm
    // 1 furlong = 10 chains  =  7.920" =   201.168,0mm
    // 1 chain   =  4 poles   =    792" =    20.116,8mm
    // 1 pole    =  5 1/2 yd  =    198" =     5.029,2mm
    // 1 yd      =  3 ft      =     36" =       914,4mm
    // 1 ft      = 12 "       =      1" =       304,8mm

static void GetMeterOrInch(MapUnit eMU, short& rnComma, tools::Long& rnMul, tools::Long& rnDiv, bool& rbMetr, bool& rbInch)
{
    rnMul=1; rnDiv=1;
    short nComma=0;
    bool bMetr = false, bInch = false;
    switch (eMU) {
        // Metrically
        case MapUnit::Map100thMM   : bMetr = true; nComma=5; break;
        case MapUnit::Map10thMM    : bMetr = true; nComma=4; break;
        case MapUnit::MapMM         : bMetr = true; nComma=3; break;
        case MapUnit::MapCM         : bMetr = true; nComma=2; break;
        // Inch
        case MapUnit::Map1000thInch: bInch = true; nComma=3; break;
        case MapUnit::Map100thInch : bInch = true; nComma=2; break;
        case MapUnit::Map10thInch  : bInch = true; nComma=1; break;
        case MapUnit::MapInch       : bInch = true; nComma=0; break;
        case MapUnit::MapPoint      : bInch = true; rnDiv=72;  break;          // 1Pt   = 1/72"
        case MapUnit::MapTwip       : bInch = true; rnDiv=144; nComma=1; break; // 1Twip = 1/1440"
        // Others
        case MapUnit::MapPixel      : break;
        case MapUnit::MapSysFont    : break;
        case MapUnit::MapAppFont    : break;
        case MapUnit::MapRelative   : break;
        default: break;
    } // switch
    rnComma=nComma;
    rbMetr=bMetr;
    rbInch=bInch;
}


void SdrFormatter::Undirty()
{
    bool bSrcMetr,bSrcInch,bDstMetr,bDstInch;
    tools::Long nMul1,nDiv1,nMul2,nDiv2;
    short nComma1,nComma2;
    // first: normalize to m or in
    GetMeterOrInch(eSrcMU,nComma1,nMul1,nDiv1,bSrcMetr,bSrcInch);
    GetMeterOrInch(eDstMU,nComma2,nMul2,nDiv2,bDstMetr,bDstInch);
    nMul1*=nDiv2;
    nDiv1*=nMul2;
    nComma1=nComma1-nComma2;

    if (bSrcInch && bDstMetr) {
        nComma1+=4;
        nMul1*=254;
    }
    if (bSrcMetr && bDstInch) {
        nComma1-=4;
        nDiv1*=254;
    }

    // temporary fraction for canceling
    Fraction aTempFract(nMul1,nDiv1);
    nMul1=aTempFract.GetNumerator();
    nDiv1=aTempFract.GetDenominator();

    nMul_=nMul1;
    nDiv_=nDiv1;
    nComma_=nComma1;
    bDirty=false;
}


OUString SdrFormatter::GetStr(tools::Long nVal) const
{
    const OUString aNullCode("0");

    if(!nVal)
    {
        return aNullCode;
    }

    // we may lose some decimal places here, because of MulDiv instead of Real
    bool bNeg(nVal < 0);
    SvtSysLocale aSysLoc;
    const LocaleDataWrapper& rLoc = aSysLoc.GetLocaleData();

    if (bDirty)
        const_cast<SdrFormatter*>(this)->Undirty();

    sal_Int16 nC(nComma_);

    if(bNeg)
        nVal = -nVal;

    while(nC <= -3)
    {
        nVal *= 1000;
        nC += 3;
    }

    while(nC <= -1)
    {
        nVal *= 10;
        nC++;
    }

    if(nMul_ != nDiv_)
        nVal = BigMulDiv(nVal, nMul_, nDiv_);

    OUStringBuffer aStr = OUString::number(nVal);

    if(nC > 0 && aStr.getLength() <= nC )
    {
        // decimal separator necessary
        sal_Int32 nCount(nC - aStr.getLength());

        if(nCount >= 0 && LocaleDataWrapper::isNumLeadingZero())
            nCount++;

        for(sal_Int32  i=0; i<nCount; i++)
            aStr.insert(0, aNullCode);

        // remove superfluous decimal points
        sal_Int32 nNumDigits(LocaleDataWrapper::getNumDigits());
        sal_Int32 nWeg(nC - nNumDigits);

        if(nWeg > 0)
        {
            // TODO: we should round here
            aStr.remove(aStr.getLength() - nWeg, nWeg);
            nC = nNumDigits;
        }
    }

    // remember everything before the decimal separator for later
    sal_Int32 nForComma(aStr.getLength() - nC);

    if(nC > 0)
    {
        // insert comma char (decimal separator)
        // remove trailing zeros
        while(nC > 0 && aStr[aStr.getLength() - 1] == aNullCode[0])
        {
            aStr.remove(aStr.getLength() - 1, 1);
            nC--;
        }

        if(nC > 0)
        {
            // do we still have decimal places?
            sal_Unicode cDec(rLoc.getNumDecimalSep()[0]);
            aStr.insert(nForComma, cDec);
        }
    }

    // add in thousands separator (if necessary)
    if( nForComma > 3 )
    {
        const OUString& aThoSep( rLoc.getNumThousandSep() );
        if ( aThoSep.getLength() > 0 )
        {
            sal_Unicode cTho( aThoSep[0] );
            sal_Int32 i(nForComma - 3);

            while(i > 0)
            {
                aStr.insert(i, cTho);
                i -= 3;
            }
        }
    }

    if(aStr.isEmpty())
        aStr.append(aNullCode);

    if(bNeg && (aStr.getLength() > 1 || aStr[0] != aNullCode[0]))
    {
        aStr.insert(0, "-");
    }

    return aStr.makeStringAndClear();
}

OUString SdrFormatter::GetUnitStr(MapUnit eUnit)
{
    switch(eUnit)
    {
        // metrically
        case MapUnit::Map100thMM   :
            return "/100mm";
        case MapUnit::Map10thMM    :
            return "/10mm";
        case MapUnit::MapMM         :
            return "mm";
        case MapUnit::MapCM         :
            return "cm";

        // Inch
        case MapUnit::Map1000thInch:
            return "/1000\"";
        case MapUnit::Map100thInch :
            return "/100\"";
        case MapUnit::Map10thInch  :
            return "/10\"";
        case MapUnit::MapInch       :
            return "\"";
        case MapUnit::MapPoint      :
            return "pt";
        case MapUnit::MapTwip       :
            return "twip";

        // others
        case MapUnit::MapPixel      :
            return "pixel";
        case MapUnit::MapSysFont    :
            return "sysfont";
        case MapUnit::MapAppFont    :
            return "appfont";
        case MapUnit::MapRelative   :
            return "%";
        default:
            return OUString();
    }
}

OUString SdrFormatter::GetUnitStr(FieldUnit eUnit)
{
    switch(eUnit)
    {
        default             :
        case FieldUnit::NONE     :
        case FieldUnit::CUSTOM   :
            return OUString();

        // metrically
        case FieldUnit::MM_100TH:
            return "/100mm";
        case FieldUnit::MM     :
            return "mm";
        case FieldUnit::CM     :
            return "cm";
        case FieldUnit::M      :
            return "m";
        case FieldUnit::KM     :
            return "km";

        // Inch
        case FieldUnit::TWIP   :
            return "twip";
        case FieldUnit::POINT  :
            return "pt";
        case FieldUnit::PICA   :
            return "pica";
        case FieldUnit::INCH   :
            return "\"";
        case FieldUnit::FOOT   :
            return "ft";
        case FieldUnit::MILE   :
            return "mile(s)";

        // others
        case FieldUnit::PERCENT:
            return "%";
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
