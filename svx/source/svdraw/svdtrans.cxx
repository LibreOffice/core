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

#include <vcl/virdev.hxx>
#include <tools/bigint.hxx>
#include <tools/debug.hxx>
#include <unotools/syslocale.hxx>

void MoveXPoly(XPolygon& rPoly, const Size& S)
{
    rPoly.Move(S.Width(),S.Height());
}

void ResizeRect(Rectangle& rRect, const Point& rRef, const Fraction& rxFact, const Fraction& ryFact, bool bNoJustify)
{
    Fraction xFact(rxFact);
    Fraction yFact(ryFact);

    {
        if (xFact.GetDenominator()==0) {
            long nWdt=rRect.Right()-rRect.Left();
            if (xFact.GetNumerator()>=0) { // catch divisions by zero
                xFact=Fraction(xFact.GetNumerator(),1);
                if (nWdt==0) rRect.Right()++;
            } else {
                xFact=Fraction(xFact.GetNumerator(),-1);
                if (nWdt==0) rRect.Left()--;
            }
        }
        rRect.Left()  =rRef.X()+Round(((double)(rRect.Left()  -rRef.X())*xFact.GetNumerator())/xFact.GetDenominator());
        rRect.Right() =rRef.X()+Round(((double)(rRect.Right() -rRef.X())*xFact.GetNumerator())/xFact.GetDenominator());
    }
    {
        if (yFact.GetDenominator()==0) {
            long nHgt=rRect.Bottom()-rRect.Top();
            if (yFact.GetNumerator()>=0) { // catch divisions by zero
                yFact=Fraction(yFact.GetNumerator(),1);
                if (nHgt==0) rRect.Bottom()++;
            } else {
                yFact=Fraction(yFact.GetNumerator(),-1);
                if (nHgt==0) rRect.Top()--;
            }

            yFact=Fraction(yFact.GetNumerator(),1); // catch divisions by zero
        }
        rRect.Top()   =rRef.Y()+Round(((double)(rRect.Top()   -rRef.Y())*yFact.GetNumerator())/yFact.GetDenominator());
        rRect.Bottom()=rRef.Y()+Round(((double)(rRect.Bottom()-rRef.Y())*yFact.GetNumerator())/yFact.GetDenominator());
    }
    if (!bNoJustify) rRect.Justify();
}


void ResizePoly(Polygon& rPoly, const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    sal_uInt16 nAnz=rPoly.GetSize();
    for (sal_uInt16 i=0; i<nAnz; i++) {
        ResizePoint(rPoly[i],rRef,xFact,yFact);
    }
}

void ResizeXPoly(XPolygon& rPoly, const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    sal_uInt16 nAnz=rPoly.GetPointCount();
    for (sal_uInt16 i=0; i<nAnz; i++) {
        ResizePoint(rPoly[i],rRef,xFact,yFact);
    }
}

void RotatePoly(Polygon& rPoly, const Point& rRef, double sn, double cs)
{
    sal_uInt16 nAnz=rPoly.GetSize();
    for (sal_uInt16 i=0; i<nAnz; i++) {
        RotatePoint(rPoly[i],rRef,sn,cs);
    }
}

void RotateXPoly(XPolygon& rPoly, const Point& rRef, double sn, double cs)
{
    sal_uInt16 nAnz=rPoly.GetPointCount();
    for (sal_uInt16 i=0; i<nAnz; i++) {
        RotatePoint(rPoly[i],rRef,sn,cs);
    }
}

void RotateXPoly(XPolyPolygon& rPoly, const Point& rRef, double sn, double cs)
{
    sal_uInt16 nAnz=rPoly.Count();
    for (sal_uInt16 i=0; i<nAnz; i++) {
        RotateXPoly(rPoly[i],rRef,sn,cs);
    }
}

void MirrorPoint(Point& rPnt, const Point& rRef1, const Point& rRef2)
{
    long mx=rRef2.X()-rRef1.X();
    long my=rRef2.Y()-rRef1.Y();
    if (mx==0) { // vertical axis
        long dx=rRef1.X()-rPnt.X();
        rPnt.X()+=2*dx;
    } else if (my==0) { // horizontal axis
        long dy=rRef1.Y()-rPnt.Y();
        rPnt.Y()+=2*dy;
    } else if (mx==my) { // diagonal axis '\'
        long dx1=rPnt.X()-rRef1.X();
        long dy1=rPnt.Y()-rRef1.Y();
        rPnt.X()=rRef1.X()+dy1;
        rPnt.Y()=rRef1.Y()+dx1;
    } else if (mx==-my) { // diagonal axis '/'
        long dx1=rPnt.X()-rRef1.X();
        long dy1=rPnt.Y()-rRef1.Y();
        rPnt.X()=rRef1.X()-dy1;
        rPnt.Y()=rRef1.Y()-dx1;
    } else { // arbitrary axis
        // TODO: Optimize this! Raise perpendicular on the mirroring axis..?
        long nRefWink=GetAngle(rRef2-rRef1);
        rPnt-=rRef1;
        long nPntWink=GetAngle(rPnt);
        long nWink=2*(nRefWink-nPntWink);
        double a=nWink*nPi180;
        double nSin=sin(a);
        double nCos=cos(a);
        RotatePoint(rPnt,Point(),nSin,nCos);
        rPnt+=rRef1;
    }
}

void MirrorPoly(Polygon& rPoly, const Point& rRef1, const Point& rRef2)
{
    sal_uInt16 nAnz=rPoly.GetSize();
    for (sal_uInt16 i=0; i<nAnz; i++) {
        MirrorPoint(rPoly[i],rRef1,rRef2);
    }
}

void MirrorXPoly(XPolygon& rPoly, const Point& rRef1, const Point& rRef2)
{
    sal_uInt16 nAnz=rPoly.GetPointCount();
    for (sal_uInt16 i=0; i<nAnz; i++) {
        MirrorPoint(rPoly[i],rRef1,rRef2);
    }
}

void ShearPoly(Polygon& rPoly, const Point& rRef, double tn, bool bVShear)
{
    sal_uInt16 nAnz=rPoly.GetSize();
    for (sal_uInt16 i=0; i<nAnz; i++) {
        ShearPoint(rPoly[i],rRef,tn,bVShear);
    }
}

void ShearXPoly(XPolygon& rPoly, const Point& rRef, double tn, bool bVShear)
{
    sal_uInt16 nAnz=rPoly.GetPointCount();
    for (sal_uInt16 i=0; i<nAnz; i++) {
        ShearPoint(rPoly[i],rRef,tn,bVShear);
    }
}

double CrookRotateXPoint(Point& rPnt, Point* pC1, Point* pC2, const Point& rCenter,
                         const Point& rRad, double& rSin, double& rCos, bool bVert)
{
    bool bC1=pC1!=NULL;
    bool bC2=pC2!=NULL;
    long x0=rPnt.X();
    long y0=rPnt.Y();
    long cx=rCenter.X();
    long cy=rCenter.Y();
    double nWink=GetCrookAngle(rPnt,rCenter,rRad,bVert);
    double sn=sin(nWink);
    double cs=cos(nWink);
    RotatePoint(rPnt,rCenter,sn,cs);
    if (bC1) {
        if (bVert) {
            // move into the direction of the center, as a basic position for the rotation
            pC1->Y()-=y0;
            // resize, account for the distance from the center
            pC1->Y()=Round(((double)pC1->Y()) /rRad.X()*(cx-pC1->X()));
            pC1->Y()+=cy;
        } else {
            // move into the direction of the center, as a basic position for the rotation
            pC1->X()-=x0;
            // resize, account for the distance from the center
            long nPntRad=cy-pC1->Y();
            double nFact=(double)nPntRad/(double)rRad.Y();
            pC1->X()=Round((double)pC1->X()*nFact);
            pC1->X()+=cx;
        }
        RotatePoint(*pC1,rCenter,sn,cs);
    }
    if (bC2) {
        if (bVert) {
            // move into the direction of the center, as a basic position for the rotation
            pC2->Y()-=y0;
            // resize, account for the distance from the center
            pC2->Y()=Round(((double)pC2->Y()) /rRad.X()*(rCenter.X()-pC2->X()));
            pC2->Y()+=cy;
        } else {
            // move into the direction of the center, as a basic position for the rotation
            pC2->X()-=x0;
            // resize, account for the distance from the center
            long nPntRad=rCenter.Y()-pC2->Y();
            double nFact=(double)nPntRad/(double)rRad.Y();
            pC2->X()=Round((double)pC2->X()*nFact);
            pC2->X()+=cx;
        }
        RotatePoint(*pC2,rCenter,sn,cs);
    }
    rSin=sn;
    rCos=cs;
    return nWink;
}

double CrookSlantXPoint(Point& rPnt, Point* pC1, Point* pC2, const Point& rCenter,
                        const Point& rRad, double& rSin, double& rCos, bool bVert)
{
    bool bC1=pC1!=NULL;
    bool bC2=pC2!=NULL;
    long x0=rPnt.X();
    long y0=rPnt.Y();
    long dx1=0,dy1=0;
    long dxC1=0,dyC1=0;
    long dxC2=0,dyC2=0;
    if (bVert) {
        long nStart=rCenter.X()-rRad.X();
        dx1=rPnt.X()-nStart;
        rPnt.X()=nStart;
        if (bC1) {
            dxC1=pC1->X()-nStart;
            pC1->X()=nStart;
        }
        if (bC2) {
            dxC2=pC2->X()-nStart;
            pC2->X()=nStart;
        }
    } else {
        long nStart=rCenter.Y()-rRad.Y();
        dy1=rPnt.Y()-nStart;
        rPnt.Y()=nStart;
        if (bC1) {
            dyC1=pC1->Y()-nStart;
            pC1->Y()=nStart;
        }
        if (bC2) {
            dyC2=pC2->Y()-nStart;
            pC2->Y()=nStart;
        }
    }
    double nWink=GetCrookAngle(rPnt,rCenter,rRad,bVert);
    double sn=sin(nWink);
    double cs=cos(nWink);
    RotatePoint(rPnt,rCenter,sn,cs);
    if (bC1) { if (bVert) pC1->Y()-=y0-rCenter.Y(); else pC1->X()-=x0-rCenter.X(); RotatePoint(*pC1,rCenter,sn,cs); }
    if (bC2) { if (bVert) pC2->Y()-=y0-rCenter.Y(); else pC2->X()-=x0-rCenter.X(); RotatePoint(*pC2,rCenter,sn,cs); }
    if (bVert) {
        rPnt.X()+=dx1;
        if (bC1) pC1->X()+=dxC1;
        if (bC2) pC2->X()+=dxC2;
    } else {
        rPnt.Y()+=dy1;
        if (bC1) pC1->Y()+=dyC1;
        if (bC2) pC2->Y()+=dyC2;
    }
    rSin=sn;
    rCos=cs;
    return nWink;
}

double CrookStretchXPoint(Point& rPnt, Point* pC1, Point* pC2, const Point& rCenter,
                          const Point& rRad, double& rSin, double& rCos, bool bVert,
                          const Rectangle rRefRect)
{
    long y0=rPnt.Y();
    CrookSlantXPoint(rPnt,pC1,pC2,rCenter,rRad,rSin,rCos,bVert);
    if (bVert) {
    } else {
        long nTop=rRefRect.Top();
        long nBtm=rRefRect.Bottom();
        long nHgt=nBtm-nTop;
        long dy=rPnt.Y()-y0;
        double a=((double)(y0-nTop))/nHgt;
        a*=dy;
        rPnt.Y()=y0+Round(a);
    } return 0.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CrookRotatePoly(XPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert)
{
    double nSin,nCos;
    sal_uInt16 nPointAnz=rPoly.GetPointCount();
    sal_uInt16 i=0;
    while (i<nPointAnz) {
        Point* pPnt=&rPoly[i];
        Point* pC1=NULL;
        Point* pC2=NULL;
        if (i+1<nPointAnz && rPoly.IsControl(i)) { // control point to the left
            pC1=pPnt;
            i++;
            pPnt=&rPoly[i];
        }
        i++;
        if (i<nPointAnz && rPoly.IsControl(i)) { // control point to the right
            pC2=&rPoly[i];
            i++;
        }
        CrookRotateXPoint(*pPnt,pC1,pC2,rCenter,rRad,nSin,nCos,bVert);
    }
}

void CrookSlantPoly(XPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert)
{
    double nSin,nCos;
    sal_uInt16 nPointAnz=rPoly.GetPointCount();
    sal_uInt16 i=0;
    while (i<nPointAnz) {
        Point* pPnt=&rPoly[i];
        Point* pC1=NULL;
        Point* pC2=NULL;
        if (i+1<nPointAnz && rPoly.IsControl(i)) { // control point to the left
            pC1=pPnt;
            i++;
            pPnt=&rPoly[i];
        }
        i++;
        if (i<nPointAnz && rPoly.IsControl(i)) { // control point to the right
            pC2=&rPoly[i];
            i++;
        }
        CrookSlantXPoint(*pPnt,pC1,pC2,rCenter,rRad,nSin,nCos,bVert);
    }
}

void CrookStretchPoly(XPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert, const Rectangle rRefRect)
{
    double nSin,nCos;
    sal_uInt16 nPointAnz=rPoly.GetPointCount();
    sal_uInt16 i=0;
    while (i<nPointAnz) {
        Point* pPnt=&rPoly[i];
        Point* pC1=NULL;
        Point* pC2=NULL;
        if (i+1<nPointAnz && rPoly.IsControl(i)) { //  control point to the left
            pC1=pPnt;
            i++;
            pPnt=&rPoly[i];
        }
        i++;
        if (i<nPointAnz && rPoly.IsControl(i)) { // control point to the right
            pC2=&rPoly[i];
            i++;
        }
        CrookStretchXPoint(*pPnt,pC1,pC2,rCenter,rRad,nSin,nCos,bVert,rRefRect);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CrookRotatePoly(XPolyPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert)
{
    sal_uInt16 nPolyAnz=rPoly.Count();
    for (sal_uInt16 nPolyNum=0; nPolyNum<nPolyAnz; nPolyNum++) {
        CrookRotatePoly(rPoly[nPolyNum],rCenter,rRad,bVert);
    }
}

void CrookSlantPoly(XPolyPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert)
{
    sal_uInt16 nPolyAnz=rPoly.Count();
    for (sal_uInt16 nPolyNum=0; nPolyNum<nPolyAnz; nPolyNum++) {
        CrookSlantPoly(rPoly[nPolyNum],rCenter,rRad,bVert);
    }
}

void CrookStretchPoly(XPolyPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert, const Rectangle rRefRect)
{
    sal_uInt16 nPolyAnz=rPoly.Count();
    for (sal_uInt16 nPolyNum=0; nPolyNum<nPolyAnz; nPolyNum++) {
        CrookStretchPoly(rPoly[nPolyNum],rCenter,rRad,bVert,rRefRect);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

long GetAngle(const Point& rPnt)
{
    long a=0;
    if (rPnt.Y()==0) {
        if (rPnt.X()<0) a=-18000;
    } else if (rPnt.X()==0) {
        if (rPnt.Y()>0) a=-9000;
        else a=9000;
    } else {
        a=Round((atan2((double)-rPnt.Y(),(double)rPnt.X())/nPi180));
    }
    return a;
}

long NormAngle180(long a)
{
    while (a<18000) a+=36000;
    while (a>=18000) a-=36000;
    return a;
}

long NormAngle360(long a)
{
    while (a<0) a+=36000;
    while (a>=36000) a-=36000;
    return a;
}

sal_uInt16 GetAngleSector(long nWink)
{
    while (nWink<0) nWink+=36000;
    while (nWink>=36000) nWink-=36000;
    if (nWink< 9000) return 0;
    if (nWink<18000) return 1;
    if (nWink<27000) return 2;
    return 3;
}

long GetLen(const Point& rPnt)
{
    long x=std::abs(rPnt.X());
    long y=std::abs(rPnt.Y());
    if (x+y<0x8000) { // because 7FFF * 7FFF * 2 = 7FFE0002
        x*=x;
        y*=y;
        x+=y;
        x=Round(sqrt((double)x));
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
            return Round(nx);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void GeoStat::RecalcSinCos()
{
    if (nDrehWink==0) {
        nSin=0.0;
        nCos=1.0;
    } else {
        double a=nDrehWink*nPi180;
        nSin=sin(a);
        nCos=cos(a);
    }
}

void GeoStat::RecalcTan()
{
    if (nShearWink==0) {
        nTan=0.0;
    } else {
        double a=nShearWink*nPi180;
        nTan=tan(a);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

Polygon Rect2Poly(const Rectangle& rRect, const GeoStat& rGeo)
{
    Polygon aPol(5);
    aPol[0]=rRect.TopLeft();
    aPol[1]=rRect.TopRight();
    aPol[2]=rRect.BottomRight();
    aPol[3]=rRect.BottomLeft();
    aPol[4]=rRect.TopLeft();
    if (rGeo.nShearWink!=0) ShearPoly(aPol,rRect.TopLeft(),rGeo.nTan);
    if (rGeo.nDrehWink!=0) RotatePoly(aPol,rRect.TopLeft(),rGeo.nSin,rGeo.nCos);
    return aPol;
}

void Poly2Rect(const Polygon& rPol, Rectangle& rRect, GeoStat& rGeo)
{
    rGeo.nDrehWink=GetAngle(rPol[1]-rPol[0]);
    rGeo.nDrehWink=NormAngle360(rGeo.nDrehWink);
    // rotation successful
    rGeo.RecalcSinCos();

    Point aPt1(rPol[1]-rPol[0]);
    if (rGeo.nDrehWink!=0) RotatePoint(aPt1,Point(0,0),-rGeo.nSin,rGeo.nCos); // -Sin to reverse rotation
    long nWdt=aPt1.X();

    Point aPt0(rPol[0]);
    Point aPt3(rPol[3]-rPol[0]);
    if (rGeo.nDrehWink!=0) RotatePoint(aPt3,Point(0,0),-rGeo.nSin,rGeo.nCos); // -Sin to reverse rotation
    long nHgt=aPt3.Y();


    long nShW=GetAngle(aPt3);
    nShW-=27000; // ShearWink is measured against a vertical line
    nShW=-nShW;  // Negieren, denn '+' ist Rechtskursivierung

    bool bMirr=aPt3.Y()<0;
    if (bMirr) { // "exchange of points" when mirroring
        nHgt=-nHgt;
        nShW+=18000;
        aPt0=rPol[3];
    }
    nShW=NormAngle180(nShW);
    if (nShW<-9000 || nShW>9000) {
        nShW=NormAngle180(nShW+18000);
    }
    if (nShW<-SDRMAXSHEAR) nShW=-SDRMAXSHEAR; // limit ShearWinkel (shear angle) to +/- 89.00 deg
    if (nShW>SDRMAXSHEAR)  nShW=SDRMAXSHEAR;
    rGeo.nShearWink=nShW;
    rGeo.RecalcTan();
    Point aRU(aPt0);
    aRU.X()+=nWdt;
    aRU.Y()+=nHgt;
    rRect=Rectangle(aPt0,aRU);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void OrthoDistance8(const Point& rPt0, Point& rPt, bool bBigOrtho)
{
    long dx=rPt.X()-rPt0.X();
    long dy=rPt.Y()-rPt0.Y();
    long dxa=std::abs(dx);
    long dya=std::abs(dy);
    if (dx==0 || dy==0 || dxa==dya) return;
    if (dxa>=dya*2) { rPt.Y()=rPt0.Y(); return; }
    if (dya>=dxa*2) { rPt.X()=rPt0.X(); return; }
    if ((dxa<dya) != bBigOrtho) {
        rPt.Y()=rPt0.Y()+(dxa* (dy>=0 ? 1 : -1) );
    } else {
        rPt.X()=rPt0.X()+(dya* (dx>=0 ? 1 : -1) );
    }
}

void OrthoDistance4(const Point& rPt0, Point& rPt, bool bBigOrtho)
{
    long dx=rPt.X()-rPt0.X();
    long dy=rPt.Y()-rPt0.Y();
    long dxa=std::abs(dx);
    long dya=std::abs(dy);
    if ((dxa<dya) != bBigOrtho) {
        rPt.Y()=rPt0.Y()+(dxa* (dy>=0 ? 1 : -1) );
    } else {
        rPt.X()=rPt0.X()+(dya* (dx>=0 ? 1 : -1) );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

long BigMulDiv(long nVal, long nMul, long nDiv)
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
        return long(aVal);
    }
    return 0x7fffffff;
}

void Kuerzen(Fraction& rF, unsigned nDigits)
{
    sal_Int32 nMul=rF.GetNumerator();
    sal_Int32 nDiv=rF.GetDenominator();
    bool bNeg = false;
    if (nMul<0) { nMul=-nMul; bNeg=!bNeg; }
    if (nDiv<0) { nDiv=-nDiv; bNeg=!bNeg; }
    if (nMul==0 || nDiv==0) return;
    sal_uInt32 a;
    a=sal_uInt32(nMul); unsigned nMulZ=0; // count leading zeros
    while (a<0x00800000) { nMulZ+=8; a<<=8; }
    while (a<0x80000000) { nMulZ++; a<<=1; }
    a=sal_uInt32(nDiv); unsigned nDivZ=0; // count leading zeros
    while (a<0x00800000) { nDivZ+=8; a<<=8; }
    while (a<0x80000000) { nDivZ++; a<<=1; }
    // count the number of digits
    int nMulDigits=32-nMulZ;
    int nDivDigits=32-nDivZ;
    // count how many decimal places can be removed
    int nMulWeg=nMulDigits-nDigits; if (nMulWeg<0) nMulWeg=0;
    int nDivWeg=nDivDigits-nDigits; if (nDivWeg<0) nDivWeg=0;
    int nWeg=std::min(nMulWeg,nDivWeg);
    nMul>>=nWeg;
    nDiv>>=nWeg;
    if (nMul==0 || nDiv==0) {
        DBG_WARNING("Math error after canceling decimal places.");
        return;
    }
    if (bNeg) nMul=-nMul;
    rF=Fraction(nMul,nDiv);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// How many eU units fit into a mm, respectively an inch?
// Or: How many mm, respectively inches, are there in an eU (and then give me the inverse)

FrPair GetInchOrMM(MapUnit eU)
{
    switch (eU) {
        case MAP_1000TH_INCH: return FrPair(1000,1);
        case MAP_100TH_INCH : return FrPair( 100,1);
        case MAP_10TH_INCH  : return FrPair(  10,1);
        case MAP_INCH       : return FrPair(   1,1);
        case MAP_POINT      : return FrPair(  72,1);
        case MAP_TWIP       : return FrPair(1440,1);
        case MAP_100TH_MM   : return FrPair( 100,1);
        case MAP_10TH_MM    : return FrPair(  10,1);
        case MAP_MM         : return FrPair(   1,1);
        case MAP_CM         : return FrPair(   1,10);
        case MAP_PIXEL      : {
            VirtualDevice aVD;
            aVD.SetMapMode(MapMode(MAP_100TH_MM));
            Point aP(aVD.PixelToLogic(Point(64,64))); // 64 pixels for more accuracy
            return FrPair(6400,aP.X(),6400,aP.Y());
        }
        case MAP_APPFONT: case MAP_SYSFONT: {
            VirtualDevice aVD;
            aVD.SetMapMode(MapMode(eU));
            Point aP(aVD.LogicToPixel(Point(32,32))); // 32 units for more accuracy
            aVD.SetMapMode(MapMode(MAP_100TH_MM));
            aP=aVD.PixelToLogic(aP);
            return FrPair(3200,aP.X(),3200,aP.Y());
        }
        default: break;
    }
    return Fraction(1,1);
}

FrPair GetInchOrMM(FieldUnit eU)
{
    switch (eU) {
        case FUNIT_INCH       : return FrPair(   1,1);
        case FUNIT_POINT      : return FrPair(  72,1);
        case FUNIT_TWIP       : return FrPair(1440,1);
        case FUNIT_100TH_MM   : return FrPair( 100,1);
        case FUNIT_MM         : return FrPair(   1,1);
        case FUNIT_CM         : return FrPair(   1,10);
        case FUNIT_M          : return FrPair(   1,1000);
        case FUNIT_KM         : return FrPair(   1,1000000);
        case FUNIT_PICA       : return FrPair(   6,1);
        case FUNIT_FOOT       : return FrPair(   1,12);
        case FUNIT_MILE       : return FrPair(   1,63360);
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

////////////////////////////////////////////////////////////////////////////////////////////////////

    // 1 mile    =  8 furlong = 63.360" = 1.609.344,0mm
    // 1 furlong = 10 chains  =  7.920" =   201.168,0mm
    // 1 chain   =  4 poles   =    792" =    20.116,8mm
    // 1 pole    =  5 1/2 yd  =    198" =     5.029,2mm
    // 1 yd      =  3 ft      =     36" =       914,4mm
    // 1 ft      = 12 "       =      1" =       304,8mm

void GetMeterOrInch(MapUnit eMU, short& rnKomma, long& rnMul, long& rnDiv, bool& rbMetr, bool& rbInch)
{
    rnMul=1; rnDiv=1;
    short nKomma=0;
    bool bMetr = false, bInch = false;
    switch (eMU) {
        // Metrisch
        case MAP_100TH_MM   : bMetr = true; nKomma=5; break;
        case MAP_10TH_MM    : bMetr = true; nKomma=4; break;
        case MAP_MM         : bMetr = true; nKomma=3; break;
        case MAP_CM         : bMetr = true; nKomma=2; break;
        // Inch
        case MAP_1000TH_INCH: bInch = true; nKomma=3; break;
        case MAP_100TH_INCH : bInch = true; nKomma=2; break;
        case MAP_10TH_INCH  : bInch = true; nKomma=1; break;
        case MAP_INCH       : bInch = true; nKomma=0; break;
        case MAP_POINT      : bInch = true; rnDiv=72;  break;          // 1Pt   = 1/72"
        case MAP_TWIP       : bInch = true; rnDiv=144; nKomma=1; break; // 1Twip = 1/1440"
        // Sonstiges
        case MAP_PIXEL      : break;
        case MAP_SYSFONT    : break;
        case MAP_APPFONT    : break;
        case MAP_RELATIVE   : break;
        default: break;
    } // switch
    rnKomma=nKomma;
    rbMetr=bMetr;
    rbInch=bInch;
}

void GetMeterOrInch(FieldUnit eFU, short& rnKomma, long& rnMul, long& rnDiv, bool& rbMetr, bool& rbInch)
{
    rnMul=1; rnDiv=1;
    short nKomma=0;
    bool bMetr = false, bInch = false;
    switch (eFU) {
        case FUNIT_NONE     : break;
        // metrically
        case FUNIT_100TH_MM : bMetr = true; nKomma=5; break;
        case FUNIT_MM       : bMetr = true; nKomma=3; break;
        case FUNIT_CM       : bMetr = true; nKomma=2; break;
        case FUNIT_M        : bMetr = true; nKomma=0; break;
        case FUNIT_KM       : bMetr = true; nKomma=-3; break;
        // Inch
        case FUNIT_TWIP     : bInch = true; rnDiv=144; nKomma=1; break;  // 1Twip = 1/1440"
        case FUNIT_POINT    : bInch = true; rnDiv=72; break;   // 1Pt   = 1/72"
        case FUNIT_PICA     : bInch = true; rnDiv=6; break;    // 1Pica = 1/6"  ?
        case FUNIT_INCH     : bInch = true; break;             // 1"    = 1"
        case FUNIT_FOOT     : bInch = true; rnMul=12; break;   // 1Ft   = 12"
        case FUNIT_MILE     : bInch = true; rnMul=6336; nKomma=-1; break; // 1mile = 63360"
        // others
        case FUNIT_CUSTOM   : break;
        case FUNIT_PERCENT  : nKomma=2; break;
        // TODO: Add code to handle the following (added to remove warning)
        case FUNIT_CHAR     : break;
        case FUNIT_LINE     : break;
        case FUNIT_PIXEL    : break;
        case FUNIT_DEGREE   : break;
        case FUNIT_SECOND   : break;
        case FUNIT_MILLISECOND   : break;
    } // switch
    rnKomma=nKomma;
    rbMetr=bMetr;
    rbInch=bInch;
}

void SdrFormatter::Undirty()
{
    if (aScale.GetNumerator()==0 || aScale.GetDenominator()==0) aScale=Fraction(1,1);
    bool bSrcMetr,bSrcInch,bDstMetr,bDstInch;
    long nMul1,nDiv1,nMul2,nDiv2;
    short nKomma1,nKomma2;
    // first: normalize to m or in
    if (!bSrcFU) {
        GetMeterOrInch(eSrcMU,nKomma1,nMul1,nDiv1,bSrcMetr,bSrcInch);
    } else {
        GetMeterOrInch(eSrcFU,nKomma1,nMul1,nDiv1,bSrcMetr,bSrcInch);
    }
    if (!bDstFU) {
        GetMeterOrInch(eDstMU,nKomma2,nMul2,nDiv2,bDstMetr,bDstInch);
    } else {
        GetMeterOrInch(eDstFU,nKomma2,nMul2,nDiv2,bDstMetr,bDstInch);
    }
    nMul1*=nDiv2;
    nDiv1*=nMul2;
    nKomma1=nKomma1-nKomma2;

    if (bSrcInch && bDstMetr) {
        nKomma1+=4;
        nMul1*=254;
    }
    if (bSrcMetr && bDstInch) {
        nKomma1-=4;
        nDiv1*=254;
    }

    // temporary fraction for canceling
    Fraction aTempFract(nMul1,nDiv1);
    nMul1=aTempFract.GetNumerator();
    nDiv1=aTempFract.GetDenominator();

    nMul_=nMul1;
    nDiv_=nDiv1;
    nKomma_=nKomma1;
    bDirty=sal_False;
}


void SdrFormatter::TakeStr(long nVal, OUString& rStr) const
{
    OUString aNullCode("0");

    if(!nVal)
    {
        rStr = aNullCode;
        return;
    }

    // we may lose some decimal places here, because of MulDiv instead of Real
    bool bNeg(nVal < 0);
    SvtSysLocale aSysLoc;
    const LocaleDataWrapper& rLoc = aSysLoc.GetLocaleData();

    ForceUndirty();

    sal_Int16 nC(nKomma_);

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

    OUStringBuffer aStr = OUString::valueOf(nVal);

    if(nC > 0 && aStr.getLength() <= nC )
    {
        // decimal separator necessary
        sal_Int32 nAnz(nC - aStr.getLength());

        if(nAnz >= 0 && rLoc.isNumLeadingZero())
            nAnz++;

        for(sal_Int32  i=0; i<nAnz; i++)
            aStr.insert(0, aNullCode);

        // remove superfluous decimal points
        sal_Int32 nNumDigits(rLoc.getNumDigits());
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
        OUString aThoSep( rLoc.getNumThousandSep() );
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
        aStr.insert(aStr.getLength(), aNullCode);

    if(bNeg && (aStr.getLength() > 1 || aStr[0] != aNullCode[0]))
    {
        aStr.insert(0, "-");
    }

    rStr = aStr.makeStringAndClear();
}

void SdrFormatter::TakeUnitStr(MapUnit eUnit, OUString& rStr)
{
    switch(eUnit)
    {
        // metrically
        case MAP_100TH_MM   :
        {
            rStr = "/100mm";
            break;
        }
        case MAP_10TH_MM    :
        {
            rStr = "/10mm";
            break;
        }
        case MAP_MM         :
        {
            rStr = "mm";
            break;
        }
        case MAP_CM         :
        {
            rStr = "cm";
            break;
        }

        // Inch
        case MAP_1000TH_INCH:
        {
            rStr = "/1000\"";
            break;
        }
        case MAP_100TH_INCH :
        {
            rStr = "/100\"";
            break;
        }
        case MAP_10TH_INCH  :
        {
            rStr = "/10\"";
            break;
        }
        case MAP_INCH       :
        {
            rStr = "\"";
            break;
        }
        case MAP_POINT      :
        {
            rStr = "pt";
            break;
        }
        case MAP_TWIP       :
        {
            rStr = "twip";
            break;
        }

        // others
        case MAP_PIXEL      :
        {
            rStr = "pixel";
            break;
        }
        case MAP_SYSFONT    :
        {
            rStr = "sysfont";
            break;
        }
        case MAP_APPFONT    :
        {
            rStr = "appfont";
            break;
        }
        case MAP_RELATIVE   :
        {
            rStr = "%";
            break;
        }
        default: break;
    }
}

void SdrFormatter::TakeUnitStr(FieldUnit eUnit, OUString& rStr)
{
    switch(eUnit)
    {
        default             :
        case FUNIT_NONE     :
        case FUNIT_CUSTOM   :
        {
            rStr = OUString();
            break;
        }

        // metrically
        case FUNIT_100TH_MM:
        {
            rStr = "/100mm";
            break;
        }
        case FUNIT_MM     :
        {
            rStr = "mm";
            break;
        }
        case FUNIT_CM     :
        {
            rStr = "cm";
            break;
        }
        case FUNIT_M      :
        {
            rStr = "m";
            break;
        }
        case FUNIT_KM     :
        {
            rStr = "km";
            break;
        }

        // Inch
        case FUNIT_TWIP   :
        {
            rStr = "twip";
            break;
        }
        case FUNIT_POINT  :
        {
            rStr = "pt";
            break;
        }
        case FUNIT_PICA   :
        {
            rStr = "pica";
            break;
        }
        case FUNIT_INCH   :
        {
            rStr = "\"";
            break;
        }
        case FUNIT_FOOT   :
        {
            rStr = "ft";
            break;
        }
        case FUNIT_MILE   :
        {
            rStr = "mile(s)";
            break;
        }

        // others
        case FUNIT_PERCENT:
        {
            rStr = "%";
            break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
