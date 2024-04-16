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
#include <tools/UnitConversion.hxx>
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
    rRect.SetLeft( rRef.X() + basegfx::fround<tools::Long>( (rRect.Left()  - rRef.X()) * double(aXFact) ) );
    rRect.SetRight( rRef.X() + basegfx::fround<tools::Long>( (rRect.Right() - rRef.X()) * double(aXFact) ) );

    if (!aYFact.IsValid()) {
        SAL_WARN( "svx.svdraw", "invalid fraction yFract, using Fraction(1,1)" );
        aYFact = Fraction(1,1);
        tools::Long nHgt = rRect.Bottom() - rRect.Top();
        if (nHgt == 0) rRect.AdjustBottom( 1 );
    }
    rRect.SetTop( rRef.Y() + basegfx::fround<tools::Long>( (rRect.Top()    - rRef.Y()) * double(aYFact) ) );
    rRect.SetBottom( rRef.Y() + basegfx::fround<tools::Long>( (rRect.Bottom() - rRef.Y()) * double(aYFact) ) );

    rRect.Normalize();
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
        Degree100 nRefAngle=GetAngle(rRef2-rRef1);
        rPnt-=rRef1;
        Degree100 nPntAngle=GetAngle(rPnt);
        Degree100 nAngle=2_deg100*(nRefAngle-nPntAngle);
        double a = toRadians(nAngle);
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
            pC1->setY(basegfx::fround<tools::Long>(static_cast<double>(pC1->Y()) /rRad.X()*(cx-pC1->X())) );
            pC1->AdjustY(cy );
        } else {
            // move into the direction of the center, as a basic position for the rotation
            pC1->AdjustX( -x0 );
            // resize, account for the distance from the center
            tools::Long nPntRad=cy-pC1->Y();
            double nFact=static_cast<double>(nPntRad)/static_cast<double>(rRad.Y());
            pC1->setX(basegfx::fround<tools::Long>(static_cast<double>(pC1->X()) * nFact));
            pC1->AdjustX(cx );
        }
        RotatePoint(*pC1,rCenter,sn,cs);
    }
    if (bC2) {
        if (bVert) {
            // move into the direction of the center, as a basic position for the rotation
            pC2->AdjustY( -y0 );
            // resize, account for the distance from the center
            pC2->setY(basegfx::fround<tools::Long>(static_cast<double>(pC2->Y()) /rRad.X()*(rCenter.X()-pC2->X())) );
            pC2->AdjustY(cy );
        } else {
            // move into the direction of the center, as a basic position for the rotation
            pC2->AdjustX( -x0 );
            // resize, account for the distance from the center
            tools::Long nPntRad=rCenter.Y()-pC2->Y();
            double nFact=static_cast<double>(nPntRad)/static_cast<double>(rRad.Y());
            pC2->setX(basegfx::fround<tools::Long>(static_cast<double>(pC2->X()) * nFact));
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
        rPnt.setY(y0 + basegfx::fround<tools::Long>(a));
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


Degree100 GetAngle(const Point& rPnt)
{
    Degree100 a;
    if (rPnt.Y()==0) {
        if (rPnt.X()<0) a=-18000_deg100;
    } else if (rPnt.X()==0) {
        if (rPnt.Y()>0) a=-9000_deg100;
        else a=9000_deg100;
    } else {
        a = Degree100(basegfx::fround(basegfx::rad2deg<100>(atan2(-static_cast<double>(rPnt.Y()), static_cast<double>(rPnt.X())))));
    }
    return a;
}

Degree100 NormAngle18000(Degree100 a)
{
    while (a<-18000_deg100) a+=36000_deg100;
    while (a>=18000_deg100) a-=36000_deg100;
    return a;
}

Degree100 NormAngle36000(Degree100 a)
{
    a %= 36000_deg100;
    if (a < 0_deg100)
        a += 36000_deg100;
    return a;
}

sal_uInt16 GetAngleSector(Degree100 nAngle) { return (NormAngle36000(nAngle) / 9000_deg100).get(); }

tools::Long GetLen(const Point& rPnt)
{
    tools::Long x=std::abs(rPnt.X());
    tools::Long y=std::abs(rPnt.Y());
    if (x+y<0x8000) { // because 7FFF * 7FFF * 2 = 7FFE0002
        x*=x;
        y*=y;
        x+=y;
        x = basegfx::fround<tools::Long>(sqrt(x));
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
            return basegfx::fround<tools::Long>(nx);
        }
    }
}


void GeoStat::RecalcSinCos()
{
    if (nRotationAngle==0_deg100) {
        mfSinRotationAngle=0.0;
        mfCosRotationAngle=1.0;
    } else {
        double a = toRadians(nRotationAngle);
        mfSinRotationAngle=sin(a);
        mfCosRotationAngle=cos(a);
    }
}

void GeoStat::RecalcTan()
{
    if (nShearAngle==0_deg100) {
        mfTanShearAngle=0.0;
    } else {
        double a = toRadians(nShearAngle);
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
    if (rGeo.nShearAngle) ShearPoly(aPol,rRect.TopLeft(),rGeo.mfTanShearAngle);
    if (rGeo.nRotationAngle) RotatePoly(aPol,rRect.TopLeft(),rGeo.mfSinRotationAngle,rGeo.mfCosRotationAngle);
    return aPol;
}

void Poly2Rect(const tools::Polygon& rPol, tools::Rectangle& rRect, GeoStat& rGeo)
{
    rGeo.nRotationAngle=GetAngle(rPol[1]-rPol[0]);
    rGeo.nRotationAngle=NormAngle36000(rGeo.nRotationAngle);
    // rotation successful
    rGeo.RecalcSinCos();

    Point aPt1(rPol[1]-rPol[0]);
    if (rGeo.nRotationAngle) RotatePoint(aPt1,Point(0,0),-rGeo.mfSinRotationAngle,rGeo.mfCosRotationAngle); // -Sin to reverse rotation
    tools::Long nWdt=aPt1.X();

    Point aPt0(rPol[0]);
    Point aPt3(rPol[3]-rPol[0]);
    if (rGeo.nRotationAngle) RotatePoint(aPt3,Point(0,0),-rGeo.mfSinRotationAngle,rGeo.mfCosRotationAngle); // -Sin to reverse rotation
    tools::Long nHgt=aPt3.Y();


    Degree100 nShW=GetAngle(aPt3);
    nShW-=27000_deg100; // ShearWink is measured against a vertical line
    nShW=-nShW;  // negating, because '+' is shearing clock-wise

    bool bMirr=aPt3.Y()<0;
    if (bMirr) { // "exchange of points" when mirroring
        nHgt=-nHgt;
        nShW+=18000_deg100;
        aPt0=rPol[3];
    }
    nShW=NormAngle18000(nShW);
    if (nShW<-9000_deg100 || nShW>9000_deg100) {
        nShW=NormAngle18000(nShW+18000_deg100);
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
    if (!nDiv)
        return 0x7fffffff;
    return BigInt::Scale(nVal, nMul, nDiv);
}

static FrPair toPair(o3tl::Length eFrom, o3tl::Length eTo)
{
    const auto& [nNum, nDen] = o3tl::getConversionMulDiv(eFrom, eTo);
    return FrPair(nNum, nDen);
}

// How many eU units fit into a mm, respectively an inch?
// Or: How many mm, respectively inches, are there in an eU (and then give me the inverse)

static FrPair GetInchOrMM(MapUnit eU)
{
    switch (eU) {
        case MapUnit::Map1000thInch: return toPair(o3tl::Length::in, o3tl::Length::in1000);
        case MapUnit::Map100thInch : return toPair(o3tl::Length::in, o3tl::Length::in100);
        case MapUnit::Map10thInch  : return toPair(o3tl::Length::in, o3tl::Length::in10);
        case MapUnit::MapInch       : return toPair(o3tl::Length::in, o3tl::Length::in);
        case MapUnit::MapPoint      : return toPair(o3tl::Length::in, o3tl::Length::pt);
        case MapUnit::MapTwip       : return toPair(o3tl::Length::in, o3tl::Length::twip);
        case MapUnit::Map100thMM   : return toPair(o3tl::Length::mm, o3tl::Length::mm100);
        case MapUnit::Map10thMM    : return toPair(o3tl::Length::mm, o3tl::Length::mm10);
        case MapUnit::MapMM         : return toPair(o3tl::Length::mm, o3tl::Length::mm);
        case MapUnit::MapCM         : return toPair(o3tl::Length::mm, o3tl::Length::cm);
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

// Calculate the factor that we need to convert units from eS to eD.
// e. g. GetMapFactor(UNIT_MM,UNIT_100TH_MM) => 100.

FrPair GetMapFactor(MapUnit eS, MapUnit eD)
{
    if (eS==eD) return FrPair(1,1,1,1);
    const auto eFrom = MapToO3tlLength(eS, o3tl::Length::invalid);
    const auto eTo = MapToO3tlLength(eD, o3tl::Length::invalid);
    if (eFrom != o3tl::Length::invalid && eTo != o3tl::Length::invalid)
        return toPair(eFrom, eTo);
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
    auto eFrom = FieldToO3tlLength(eS), eTo = FieldToO3tlLength(eD);
    if (eFrom == o3tl::Length::invalid)
    {
        if (eTo == o3tl::Length::invalid)
            return FrPair(1,1,1,1);
        eFrom = IsInch(eD) ? o3tl::Length::in : o3tl::Length::mm;
    }
    else if (eTo == o3tl::Length::invalid)
        eTo = IsInch(eS) ? o3tl::Length::in : o3tl::Length::mm;
    return toPair(eFrom, eTo);
};

void SdrFormatter::Undirty()
{
    const o3tl::Length eFrom = MapToO3tlLength(eSrcMU, o3tl::Length::invalid);
    const o3tl::Length eTo = MapToO3tlLength(eDstMU, o3tl::Length::invalid);
    if (eFrom != o3tl::Length::invalid && eTo != o3tl::Length::invalid)
    {
        const auto& [mul, div] = o3tl::getConversionMulDiv(eFrom, eTo);
        sal_Int64 nMul = mul;
        sal_Int64 nDiv = div;
        short nComma = 0;

        // shorten trailing zeros for dividend
        while (0 == (nMul % 10))
        {
            nComma--;
            nMul /= 10;
        }

        // shorten trailing zeros for divisor
        while (0 == (nDiv % 10))
        {
            nComma++;
            nDiv /= 10;
        }
        nMul_ = nMul;
        nDiv_ = nDiv;
        nComma_ = nComma;
    }
    else
    {
        nMul_ = nDiv_ = 1;
        nComma_ = 0;
    }
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
