/*************************************************************************
 *
 *  $RCSfile: svdtrans.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dl $ $Date: 2000-09-29 08:57:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "svdtrans.hxx"
#include <math.h>
#include "xpoly.hxx"

#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif

#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX //autogen
#include <tools/debug.hxx>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

void MoveXPoly(XPolygon& rPoly, const Size& S)
{
    rPoly.Move(S.Width(),S.Height());
}

void MoveXPoly(XPolyPolygon& rPoly, const Size& S)
{
    rPoly.Move(S.Width(),S.Height());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ResizeRect(Rectangle& rRect, const Point& rRef, const Fraction& rxFact, const Fraction& ryFact, FASTBOOL bNoJustify)
{
    Fraction xFact(rxFact);
    Fraction yFact(ryFact);
    long nHgt=rRect.Bottom()-rRect.Top();

    {
        if (xFact.GetDenominator()==0) {
            long nWdt=rRect.Right()-rRect.Left();
            if (xFact.GetNumerator()>=0) { // DivZero abfangen
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
            if (yFact.GetNumerator()>=0) { // DivZero abfangen
                yFact=Fraction(yFact.GetNumerator(),1);
                if (nHgt==0) rRect.Bottom()++;
            } else {
                yFact=Fraction(yFact.GetNumerator(),-1);
                if (nHgt==0) rRect.Top()--;
            }

            yFact=Fraction(yFact.GetNumerator(),1); // DivZero abfangen
        }
        rRect.Top()   =rRef.Y()+Round(((double)(rRect.Top()   -rRef.Y())*yFact.GetNumerator())/yFact.GetDenominator());
        rRect.Bottom()=rRef.Y()+Round(((double)(rRect.Bottom()-rRef.Y())*yFact.GetNumerator())/yFact.GetDenominator());
    }
    if (!bNoJustify) rRect.Justify();
}


void ResizePoly(Polygon& rPoly, const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    USHORT nAnz=rPoly.GetSize();
    for (USHORT i=0; i<nAnz; i++) {
        ResizePoint(rPoly[i],rRef,xFact,yFact);
    }
}

void ResizeXPoly(XPolygon& rPoly, const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    USHORT nAnz=rPoly.GetPointCount();
    for (USHORT i=0; i<nAnz; i++) {
        ResizePoint(rPoly[i],rRef,xFact,yFact);
    }
}

void ResizePoly(PolyPolygon& rPoly, const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    USHORT nAnz=rPoly.Count();
    for (USHORT i=0; i<nAnz; i++) {
        ResizePoly(rPoly[i],rRef,xFact,yFact);
    }
}

void ResizeXPoly(XPolyPolygon& rPoly, const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    USHORT nAnz=rPoly.Count();
    for (USHORT i=0; i<nAnz; i++) {
        ResizeXPoly(rPoly[i],rRef,xFact,yFact);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RotatePoly(Polygon& rPoly, const Point& rRef, double sn, double cs)
{
    USHORT nAnz=rPoly.GetSize();
    for (USHORT i=0; i<nAnz; i++) {
        RotatePoint(rPoly[i],rRef,sn,cs);
    }
}

void RotateXPoly(XPolygon& rPoly, const Point& rRef, double sn, double cs)
{
    USHORT nAnz=rPoly.GetPointCount();
    for (USHORT i=0; i<nAnz; i++) {
        RotatePoint(rPoly[i],rRef,sn,cs);
    }
}

void RotatePoly(PolyPolygon& rPoly, const Point& rRef, double sn, double cs)
{
    USHORT nAnz=rPoly.Count();
    for (USHORT i=0; i<nAnz; i++) {
        RotatePoly(rPoly[i],rRef,sn,cs);
    }
}

void RotateXPoly(XPolyPolygon& rPoly, const Point& rRef, double sn, double cs)
{
    USHORT nAnz=rPoly.Count();
    for (USHORT i=0; i<nAnz; i++) {
        RotateXPoly(rPoly[i],rRef,sn,cs);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void MirrorRect(Rectangle& rRect, const Point& rRef1, const Point& rRef2, FASTBOOL bNoJustify)
{
    // !!! fehlende Implementation !!!
    if (!bNoJustify) rRect.Justify();
}

void MirrorPoint(Point& rPnt, const Point& rRef1, const Point& rRef2)
{
    long mx=rRef2.X()-rRef1.X();
    long my=rRef2.Y()-rRef1.Y();
    if (mx==0) { // Achse senkrecht
        long dx=rRef1.X()-rPnt.X();
        rPnt.X()+=2*dx;
    } else if (my==0) { // Achse waagerecht
        long dy=rRef1.Y()-rPnt.Y();
        rPnt.Y()+=2*dy;
    } else if (mx==my) { // Achse diagonal '\'
        long dx1=rPnt.X()-rRef1.X();
        long dy1=rPnt.Y()-rRef1.Y();
        rPnt.X()=rRef1.X()+dy1;
        rPnt.Y()=rRef1.Y()+dx1;
    } else if (mx==-my) { // Achse diagonal '/'
        long dx1=rPnt.X()-rRef1.X();
        long dy1=rPnt.Y()-rRef1.Y();
        rPnt.X()=rRef1.X()-dy1;
        rPnt.Y()=rRef1.Y()-dx1;
    } else { // beliebige Achse
        // mal optimieren !!!
        // Lot auf der Spiegelachse faellen oder so
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
    USHORT nAnz=rPoly.GetSize();
    for (USHORT i=0; i<nAnz; i++) {
        MirrorPoint(rPoly[i],rRef1,rRef2);
    }
}

void MirrorXPoly(XPolygon& rPoly, const Point& rRef1, const Point& rRef2)
{
    USHORT nAnz=rPoly.GetPointCount();
    for (USHORT i=0; i<nAnz; i++) {
        MirrorPoint(rPoly[i],rRef1,rRef2);
    }
}

void MirrorPoly(PolyPolygon& rPoly, const Point& rRef1, const Point& rRef2)
{
    USHORT nAnz=rPoly.Count();
    for (USHORT i=0; i<nAnz; i++) {
        MirrorPoly(rPoly[i],rRef1,rRef2);
    }
}

void MirrorXPoly(XPolyPolygon& rPoly, const Point& rRef1, const Point& rRef2)
{
    USHORT nAnz=rPoly.Count();
    for (USHORT i=0; i<nAnz; i++) {
        MirrorXPoly(rPoly[i],rRef1,rRef2);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ShearPoly(Polygon& rPoly, const Point& rRef, double tn, FASTBOOL bVShear)
{
    USHORT nAnz=rPoly.GetSize();
    for (USHORT i=0; i<nAnz; i++) {
        ShearPoint(rPoly[i],rRef,tn,bVShear);
    }
}

void ShearXPoly(XPolygon& rPoly, const Point& rRef, double tn, FASTBOOL bVShear)
{
    USHORT nAnz=rPoly.GetPointCount();
    for (USHORT i=0; i<nAnz; i++) {
        ShearPoint(rPoly[i],rRef,tn,bVShear);
    }
}

void ShearPoly(PolyPolygon& rPoly, const Point& rRef, double tn, FASTBOOL bVShear)
{
    USHORT nAnz=rPoly.Count();
    for (USHORT i=0; i<nAnz; i++) {
        ShearPoly(rPoly[i],rRef,tn,bVShear);
    }
}

void ShearXPoly(XPolyPolygon& rPoly, const Point& rRef, double tn, FASTBOOL bVShear)
{
    USHORT nAnz=rPoly.Count();
    for (USHORT i=0; i<nAnz; i++) {
        ShearXPoly(rPoly[i],rRef,tn,bVShear);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//   @@@@  @@@@@   @@@@   @@@@  @@  @@
//  @@  @@ @@  @@ @@  @@ @@  @@ @@  @@
//  @@     @@  @@ @@  @@ @@  @@ @@ @@
//  @@     @@@@@  @@  @@ @@  @@ @@@@
//  @@     @@  @@ @@  @@ @@  @@ @@ @@
//  @@  @@ @@  @@ @@  @@ @@  @@ @@  @@
//   @@@@  @@  @@  @@@@   @@@@  @@  @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

double CrookRotateXPoint(Point& rPnt, Point* pC1, Point* pC2, const Point& rCenter,
                         const Point& rRad, double& rSin, double& rCos, FASTBOOL bVert)
{
    FASTBOOL bC1=pC1!=NULL;
    FASTBOOL bC2=pC2!=NULL;
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
            // Richtung Zentrum verschieben, als Ausgangsposition fuer Rotate
            pC1->Y()-=y0;
            // Resize, entsprechend der Entfernung vom Zentrum
            pC1->Y()=Round(((double)pC1->Y()) /rRad.X()*(cx-pC1->X()));
            pC1->Y()+=cy;
        } else {
            // Richtung Zentrum verschieben, als Ausgangsposition fuer Rotate
            pC1->X()-=x0;
            // Resize, entsprechend der Entfernung vom Zentrum
            long nPntRad=cy-pC1->Y();
            double nFact=(double)nPntRad/(double)rRad.Y();
            pC1->X()=Round((double)pC1->X()*nFact);
            pC1->X()+=cx;
        }
        RotatePoint(*pC1,rCenter,sn,cs);
    }
    if (bC2) {
        if (bVert) {
            // Richtung Zentrum verschieben, als Ausgangsposition fuer Rotate
            pC2->Y()-=y0;
            // Resize, entsprechend der Entfernung vom Zentrum
            pC2->Y()=Round(((double)pC2->Y()) /rRad.X()*(rCenter.X()-pC2->X()));
            pC2->Y()+=cy;
        } else {
            // Richtung Zentrum verschieben, als Ausgangsposition fuer Rotate
            pC2->X()-=x0;
            // Resize, entsprechend der Entfernung vom Zentrum
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
                        const Point& rRad, double& rSin, double& rCos, FASTBOOL bVert)
{
    FASTBOOL bC1=pC1!=NULL;
    FASTBOOL bC2=pC2!=NULL;
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
                          const Point& rRad, double& rSin, double& rCos, FASTBOOL bVert,
                          const Rectangle rRefRect)
{
    FASTBOOL bC1=pC1!=NULL;
    FASTBOOL bC2=pC2!=NULL;
    long x0=rPnt.X();
    long y0=rPnt.Y();
    CrookSlantXPoint(rPnt,pC1,pC2,rCenter,rRad,rSin,rCos,bVert);
    if (bVert) {
    } else {
        long nBase=rCenter.Y()-rRad.Y();
        long nTop=rRefRect.Top();
        long nBtm=rRefRect.Bottom();
        long nHgt=nBtm-nTop;
        long dy=rPnt.Y()-y0;
        FASTBOOL bOben=rRad.Y()<0;
        double a=((double)(y0-nTop))/nHgt;
        a*=dy;
        rPnt.Y()=y0+Round(a);
    } return 0.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CrookRotatePoly(XPolygon& rPoly, const Point& rCenter, const Point& rRad, FASTBOOL bVert)
{
    double nSin,nCos;
    USHORT nPointAnz=rPoly.GetPointCount();
    USHORT i=0;
    while (i<nPointAnz) {
        Point* pPnt=&rPoly[i];
        Point* pC1=NULL;
        Point* pC2=NULL;
        if (i+1<nPointAnz && rPoly.IsControl(i)) { // Kontrollpunkt links
            pC1=pPnt;
            i++;
            pPnt=&rPoly[i];
        }
        i++;
        if (i<nPointAnz && rPoly.IsControl(i)) { // Kontrollpunkt rechts
            pC2=&rPoly[i];
            i++;
        }
        CrookRotateXPoint(*pPnt,pC1,pC2,rCenter,rRad,nSin,nCos,bVert);
    }
}

void CrookSlantPoly(XPolygon& rPoly, const Point& rCenter, const Point& rRad, FASTBOOL bVert)
{
    double nSin,nCos;
    USHORT nPointAnz=rPoly.GetPointCount();
    USHORT i=0;
    while (i<nPointAnz) {
        Point* pPnt=&rPoly[i];
        Point* pC1=NULL;
        Point* pC2=NULL;
        if (i+1<nPointAnz && rPoly.IsControl(i)) { // Kontrollpunkt links
            pC1=pPnt;
            i++;
            pPnt=&rPoly[i];
        }
        i++;
        if (i<nPointAnz && rPoly.IsControl(i)) { // Kontrollpunkt rechts
            pC2=&rPoly[i];
            i++;
        }
        CrookSlantXPoint(*pPnt,pC1,pC2,rCenter,rRad,nSin,nCos,bVert);
    }
}

void CrookStretchPoly(XPolygon& rPoly, const Point& rCenter, const Point& rRad, FASTBOOL bVert, const Rectangle rRefRect)
{
    double nSin,nCos;
    USHORT nPointAnz=rPoly.GetPointCount();
    USHORT i=0;
    while (i<nPointAnz) {
        Point* pPnt=&rPoly[i];
        Point* pC1=NULL;
        Point* pC2=NULL;
        if (i+1<nPointAnz && rPoly.IsControl(i)) { // Kontrollpunkt links
            pC1=pPnt;
            i++;
            pPnt=&rPoly[i];
        }
        i++;
        if (i<nPointAnz && rPoly.IsControl(i)) { // Kontrollpunkt rechts
            pC2=&rPoly[i];
            i++;
        }
        CrookStretchXPoint(*pPnt,pC1,pC2,rCenter,rRad,nSin,nCos,bVert,rRefRect);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CrookRotatePoly(XPolyPolygon& rPoly, const Point& rCenter, const Point& rRad, FASTBOOL bVert)
{
    USHORT nPolyAnz=rPoly.Count();
    for (USHORT nPolyNum=0; nPolyNum<nPolyAnz; nPolyNum++) {
        CrookRotatePoly(rPoly[nPolyNum],rCenter,rRad,bVert);
    }
}

void CrookSlantPoly(XPolyPolygon& rPoly, const Point& rCenter, const Point& rRad, FASTBOOL bVert)
{
    USHORT nPolyAnz=rPoly.Count();
    for (USHORT nPolyNum=0; nPolyNum<nPolyAnz; nPolyNum++) {
        CrookSlantPoly(rPoly[nPolyNum],rCenter,rRad,bVert);
    }
}

void CrookStretchPoly(XPolyPolygon& rPoly, const Point& rCenter, const Point& rRad, FASTBOOL bVert, const Rectangle rRefRect)
{
    USHORT nPolyAnz=rPoly.Count();
    for (USHORT nPolyNum=0; nPolyNum<nPolyAnz; nPolyNum++) {
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

USHORT GetAngleSector(long nWink)
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
    long x=Abs(rPnt.X());
    long y=Abs(rPnt.Y());
    if (x+y<0x8000) { // weil 7FFF * 7FFF * 2 = 7FFE0002
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
            return 0x7FFFFFFF; // Ueberlauf, mehr is nich!
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
    // Drehung ist damit im Kasten
    rGeo.RecalcSinCos();

    Point aPt1(rPol[1]-rPol[0]);
    if (rGeo.nDrehWink!=0) RotatePoint(aPt1,Point(0,0),-rGeo.nSin,rGeo.nCos); // -Sin fuer Rueckdrehung
    long nWdt=aPt1.X();

    Point aPt0(rPol[0]);
    Point aPt3(rPol[3]-rPol[0]);
    if (rGeo.nDrehWink!=0) RotatePoint(aPt3,Point(0,0),-rGeo.nSin,rGeo.nCos); // -Sin fuer Rueckdrehung
    long nHgt=aPt3.Y();
    long nShW=GetAngle(aPt3);
    nShW-=27000; // ShearWink wird zur Senkrechten gemessen
    nShW=-nShW;  // Negieren, denn '+' ist Rechtskursivierung

    FASTBOOL bMirr=aPt3.Y()<0;
    if (bMirr) { // "Punktetausch" bei Spiegelung
        nHgt=-nHgt;
        nShW+=18000;
        aPt0=rPol[3];
    }
    nShW=NormAngle180(nShW);
    if (nShW<-9000 || nShW>9000) {
        nShW=NormAngle180(nShW+18000);
    }
    if (nShW<-SDRMAXSHEAR) nShW=-SDRMAXSHEAR; // ShearWinkel begrenzen auf +/- 89.00 deg
    if (nShW>SDRMAXSHEAR)  nShW=SDRMAXSHEAR;
    rGeo.nShearWink=nShW;
    rGeo.RecalcTan();
    Point aRU(aPt0);
    aRU.X()+=nWdt;
    aRU.Y()+=nHgt;
    rRect=Rectangle(aPt0,aRU);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void OrthoDistance8(const Point& rPt0, Point& rPt, FASTBOOL bBigOrtho)
{
    long dx=rPt.X()-rPt0.X();
    long dy=rPt.Y()-rPt0.Y();
    long dxa=Abs(dx);
    long dya=Abs(dy);
    if (dx==0 || dy==0 || dxa==dya) return;
    if (dxa>=dya*2) { rPt.Y()=rPt0.Y(); return; }
    if (dya>=dxa*2) { rPt.X()=rPt0.X(); return; }
    if ((dxa<dya) != bBigOrtho) {
        rPt.Y()=rPt0.Y()+(dxa* (dy>=0 ? 1 : -1) );
    } else {
        rPt.X()=rPt0.X()+(dya* (dx>=0 ? 1 : -1) );
    }
}

void OrthoDistance4(const Point& rPt0, Point& rPt, FASTBOOL bBigOrtho)
{
    long dx=rPt.X()-rPt0.X();
    long dy=rPt.Y()-rPt0.Y();
    long dxa=Abs(dx);
    long dya=Abs(dy);
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
        aVal-=nDiv/2; // fuer korrektes Runden
    } else {
        aVal+=nDiv/2; // fuer korrektes Runden
    }
    aVal/=nDiv;
    return long(aVal);
}

void Kuerzen(Fraction& rF, unsigned nDigits)
{
    INT32 nMul=rF.GetNumerator();
    INT32 nDiv=rF.GetDenominator();
    FASTBOOL bNeg=FALSE;
    if (nMul<0) { nMul=-nMul; bNeg=!bNeg; }
    if (nDiv<0) { nDiv=-nDiv; bNeg=!bNeg; }
    if (nMul==0 || nDiv==0) return;
    UINT32 a;
    a=UINT32(nMul); unsigned nMulZ=0; // Fuehrende Nullen zaehlen
    while (a<0x00800000) { nMulZ+=8; a<<=8; }
    while (a<0x80000000) { nMulZ++; a<<=1; }
    a=UINT32(nDiv); unsigned nDivZ=0; // Fuehrende Nullen zaehlen
    while (a<0x00800000) { nDivZ+=8; a<<=8; }
    while (a<0x80000000) { nDivZ++; a<<=1; }
    // Anzahl der verwendeten Digits bestimmen
    int nMulDigits=32-nMulZ;
    int nDivDigits=32-nDivZ;
    // Nun bestimmen, wieviele Stellen hinten weg koennen
    int nMulWeg=nMulDigits-nDigits; if (nMulWeg<0) nMulWeg=0;
    int nDivWeg=nDivDigits-nDigits; if (nDivWeg<0) nDivWeg=0;
    int nWeg=Min(nMulWeg,nDivWeg);
    nMul>>=nWeg;
    nDiv>>=nWeg;
    if (nMul==0 || nDiv==0) {
        DBG_WARNING("Oups, beim kuerzen einer Fraction hat sich Joe verrechnet.");
        return;
    }
    if (bNeg) nMul=-nMul;
    rF=Fraction(nMul,nDiv);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Wieviele eU-Einheiten passen in einen mm bzw. Inch?
// Oder wie gross ist ein eU in mm bzw. Inch, und davon der Kehrwert

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
            Point aP(aVD.PixelToLogic(Point(64,64))); // 64 Pixel fuer bessere Genauigkeit
            return FrPair(6400,aP.X(),6400,aP.Y());
        }
        case MAP_APPFONT: case MAP_SYSFONT: {
            VirtualDevice aVD;
            aVD.SetMapMode(MapMode(eU));
            Point aP(aVD.LogicToPixel(Point(32,32))); // 32 Einheiten fuer bessere Genauigkeit
            aVD.SetMapMode(MapMode(MAP_100TH_MM));
            aP=aVD.PixelToLogic(aP);
            return FrPair(3200,aP.X(),3200,aP.Y());
        }
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
    }
    return Fraction(1,1);
}

// Den Faktor berechnen, der anzuwenden ist um n Einheiten von eS nach
// eD umzurechnen. Z.B. GetMapFactor(UNIT_MM,UNIT_100TH_MM) => 100.

FrPair GetMapFactor(MapUnit eS, MapUnit eD)
{
    if (eS==eD) return FrPair(1,1,1,1);
    FrPair aS(GetInchOrMM(eS));
    FrPair aD(GetInchOrMM(eD));
    FASTBOOL bSInch=IsInch(eS);
    FASTBOOL bDInch=IsInch(eD);
    FrPair aRet(aD.X()/aS.X(),aD.Y()/aS.Y());
    if (bSInch && !bDInch) { aRet.X()*=Fraction(127,5); aRet.Y()*=Fraction(127,5); }
    if (!bSInch && bDInch) { aRet.X()*=Fraction(5,127); aRet.Y()*=Fraction(5,127); }
    return aRet;
};

FrPair GetMapFactor(MapUnit eS, FieldUnit eD)
{
    FrPair aS(GetInchOrMM(eS));
    FrPair aD(GetInchOrMM(eD));
    FASTBOOL bSInch=IsInch(eS);
    FASTBOOL bDInch=IsInch(eD);
    FrPair aRet(aD.X()/aS.X(),aD.Y()/aS.Y());
    if (bSInch && !bDInch) { aRet.X()*=Fraction(127,5); aRet.Y()*=Fraction(127,5); }
    if (!bSInch && bDInch) { aRet.X()*=Fraction(5,127); aRet.Y()*=Fraction(5,127); }
    return aRet;
};

FrPair GetMapFactor(FieldUnit eS, MapUnit eD)
{
    FrPair aS(GetInchOrMM(eS));
    FrPair aD(GetInchOrMM(eD));
    FASTBOOL bSInch=IsInch(eS);
    FASTBOOL bDInch=IsInch(eD);
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
    FASTBOOL bSInch=IsInch(eS);
    FASTBOOL bDInch=IsInch(eD);
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

void GetMeterOrInch(MapUnit eMU, short& rnKomma, long& rnMul, long& rnDiv, FASTBOOL& rbMetr, FASTBOOL& rbInch)
{
    rnMul=1; rnDiv=1;
    short nKomma=0;
    FASTBOOL bMetr=FALSE,bInch=FALSE;
    switch (eMU) {
        // Metrisch
        case MAP_100TH_MM   : bMetr=TRUE; nKomma=5; break;
        case MAP_10TH_MM    : bMetr=TRUE; nKomma=4; break;
        case MAP_MM         : bMetr=TRUE; nKomma=3; break;
        case MAP_CM         : bMetr=TRUE; nKomma=2; break;
        // Inch
        case MAP_1000TH_INCH: bInch=TRUE; nKomma=3; break;
        case MAP_100TH_INCH : bInch=TRUE; nKomma=2; break;
        case MAP_10TH_INCH  : bInch=TRUE; nKomma=1; break;
        case MAP_INCH       : bInch=TRUE; nKomma=0; break;
        case MAP_POINT      : bInch=TRUE; rnDiv=72;  break;          // 1Pt   = 1/72"
        case MAP_TWIP       : bInch=TRUE; rnDiv=144; nKomma=1; break; // 1Twip = 1/1440"
        // Sonstiges
        case MAP_PIXEL      : break;
        case MAP_SYSFONT    : break;
        case MAP_APPFONT    : break;
        case MAP_RELATIVE   : break;
    } // switch
    rnKomma=nKomma;
    rbMetr=bMetr;
    rbInch=bInch;
}

void GetMeterOrInch(FieldUnit eFU, short& rnKomma, long& rnMul, long& rnDiv, FASTBOOL& rbMetr, FASTBOOL& rbInch)
{
    rnMul=1; rnDiv=1;
    short nKomma=0;
    FASTBOOL bMetr=FALSE,bInch=FALSE;
    switch (eFU) {
        case FUNIT_NONE     : break;
        // Metrisch
        case FUNIT_100TH_MM : bMetr=TRUE; nKomma=5; break;
        case FUNIT_MM       : bMetr=TRUE; nKomma=3; break;
        case FUNIT_CM       : bMetr=TRUE; nKomma=2; break;
        case FUNIT_M        : bMetr=TRUE; nKomma=0; break;
        case FUNIT_KM       : bMetr=TRUE; nKomma=-3; break;
        // Inch
        case FUNIT_TWIP     : bInch=TRUE; rnDiv=144; nKomma=1; break;  // 1Twip = 1/1440"
        case FUNIT_POINT    : bInch=TRUE; rnDiv=72; break;   // 1Pt   = 1/72"
        case FUNIT_PICA     : bInch=TRUE; rnDiv=6; break;    // 1Pica = 1/6"  ?
        case FUNIT_INCH     : bInch=TRUE; break;             // 1"    = 1"
        case FUNIT_FOOT     : bInch=TRUE; rnMul=12; break;   // 1Ft   = 12"
        case FUNIT_MILE     : bInch=TRUE; rnMul=6336; nKomma=-1; break; // 1mile = 63360"
        // sonstiges
        case FUNIT_CUSTOM   : break;
        case FUNIT_PERCENT  : nKomma=2; break;
    } // switch
    rnKomma=nKomma;
    rbMetr=bMetr;
    rbInch=bInch;
}

void SdrFormatter::Undirty()
{
    if (aScale.GetNumerator()==0 || aScale.GetDenominator()==0) aScale=Fraction(1,1);
    FASTBOOL bSrcMetr,bSrcInch,bDstMetr,bDstInch;
    long nMul1,nDiv1,nMul2,nDiv2;
    short nKomma1,nKomma2;
    // Zunaechst normalisieren auf m bzw. "
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
    nKomma1-=nKomma2;

    if (bSrcInch && bDstMetr) {
        nKomma1+=4;
        nMul1*=254;
    }
    if (bSrcMetr && bDstInch) {
        nKomma1-=4;
        nDiv1*=254;
    }

    // Temporaere Fraction zum Kuerzen
    Fraction aTempFract(nMul1,nDiv1);
    nMul1=aTempFract.GetNumerator();
    nDiv1=aTempFract.GetDenominator();

    nMul_=nMul1;
    nDiv_=nDiv1;
    nKomma_=nKomma1;
    bDirty=FALSE;
}


void SdrFormatter::TakeStr(long nVal, XubString& rStr) const
{
    sal_Unicode aNullCode('0');

    if(!nVal)
    {
        rStr = UniString();
        rStr += aNullCode;
        return;
    }

    // Hier fallen trotzdem evtl. Nachkommastellen weg, wg. MulDiv statt Real
    BOOL bNeg(nVal < 0);
    International aInter(GetpApp()->GetAppInternational());

    ForceUndirty();

    xub_StrLen nK(nKomma_);
    XubString aStr;

    if(bNeg)
        nVal = -nVal;

    while(nK <= -3)
    {
        nVal *= 1000;
        nK += 3;
    }

    while(nK <= -1)
    {
        nVal *= 10;
        nK++;
    }

    if(nMul_ != nDiv_)
        nVal = BigMulDiv(nVal, nMul_, nDiv_);

    aStr = UniString::CreateFromInt32(nVal);

    if(nK > 0)
    {
        // Komma erforderlich
        xub_StrLen nAnz(nK - aStr.Len());

        if(nAnz >= 0 && aInter.IsNumLeadingZero())
            nAnz++;

        for(xub_StrLen  i=0; i<nAnz; i++)
            aStr.Insert(aNullCode, 0);

        // zuviele Nachkommastellen abhacken
        xub_StrLen nNumDigits(aInter.GetNumDigits());
        xub_StrLen nWeg(nK - nNumDigits);

        if(nWeg > 0)
        {
            // hier muesste eigentlich noch gerundet werden!
            aStr.Erase(aStr.Len() - nWeg);
            nK = nNumDigits;
        }
    }

    // Vorkommastellen fuer spaeter merken
    xub_StrLen nVorKomma(aStr.Len() - nK);

    if(nK > 0)
    {
        // KommaChar einfuegen
        // erstmal trailing Zeros abhacken
        while(nK > 0 && aStr.GetChar(aStr.Len() - 1) == aNullCode)
        {
            aStr.Erase(aStr.Len() - 1);
            nK--;
        }

        if(nK > 0)
        {
            // na, noch Nachkommastellen da?
            sal_Unicode cDec(aInter.GetNumDecimalSep());
            aStr.Insert(cDec, nVorKomma);
        }
    }

    // ggf. Trennpunkte bei jedem Tausender einfuegen
    if(nVorKomma > 3 && aInter.IsNumThousandSep())
    {
        sal_Unicode cDot(aInter.GetNumThousandSep());
        xub_StrLen i(nVorKomma - 3);

        while(i > 0)
        {
            aStr.Insert(cDot, i);
            i -= 3;
        }
    }

    if(!aStr.Len())
        aStr += aNullCode;

    if(bNeg && (aStr.Len() > 1 || aStr.GetChar(0) != aNullCode))
    {
        rStr.Insert(sal_Unicode('-'), 0);
    }

    rStr = aStr;
}

void SdrFormatter::TakeUnitStr(MapUnit eUnit, XubString& rStr)
{
    switch(eUnit)
    {
        // Metrisch
        case MAP_100TH_MM   :
        {
            sal_Char aText[] = "/100mm";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case MAP_10TH_MM    :
        {
            sal_Char aText[] = "/10mm";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case MAP_MM         :
        {
            sal_Char aText[] = "mm";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case MAP_CM         :
        {
            sal_Char aText[] = "cm";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }

        // Inch
        case MAP_1000TH_INCH:
        {
            sal_Char aText[] = "/1000\"";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case MAP_100TH_INCH :
        {
            sal_Char aText[] = "/100\"";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case MAP_10TH_INCH  :
        {
            sal_Char aText[] = "/10\"";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case MAP_INCH       :
        {
            rStr = UniString();
            rStr += sal_Unicode('"');
            break;
        }
        case MAP_POINT      :
        {
            sal_Char aText[] = "pt";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case MAP_TWIP       :
        {
            sal_Char aText[] = "twip";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }

        // Sonstiges
        case MAP_PIXEL      :
        {
            sal_Char aText[] = "pixel";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case MAP_SYSFONT    :
        {
            sal_Char aText[] = "sysfont";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case MAP_APPFONT    :
        {
            sal_Char aText[] = "appfont";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case MAP_RELATIVE   :
        {
            rStr = UniString();
            rStr += sal_Unicode('%');
            break;
        }
    }
}

void SdrFormatter::TakeUnitStr(FieldUnit eUnit, XubString& rStr)
{
    switch(eUnit)
    {
        default             :
        case FUNIT_NONE     :
        case FUNIT_CUSTOM   :
        {
            rStr = UniString();
            break;
        }

        // Metrisch
        case FUNIT_100TH_MM:
        {
            sal_Char aText[] = "/100mm";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case FUNIT_MM     :
        {
            sal_Char aText[] = "mm";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case FUNIT_CM     :
        {
            sal_Char aText[] = "cm";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case FUNIT_M      :
        {
            rStr = UniString();
            rStr += sal_Unicode('m');
            break;
        }
        case FUNIT_KM     :
        {
            sal_Char aText[] = "km";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }

        // Inch
        case FUNIT_TWIP   :
        {
            sal_Char aText[] = "twip";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case FUNIT_POINT  :
        {
            sal_Char aText[] = "pt";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case FUNIT_PICA   :
        {
            sal_Char aText[] = "pica";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case FUNIT_INCH   :
        {
            rStr = UniString();
            rStr += sal_Unicode('"');
            break;
        }
        case FUNIT_FOOT   :
        {
            sal_Char aText[] = "ft";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }
        case FUNIT_MILE   :
        {
            sal_Char aText[] = "mile(s)";
            rStr = UniString(aText, sizeof(aText-1));
            break;
        }

        // sonstiges
        case FUNIT_PERCENT:
        {
            rStr = UniString();
            rStr += sal_Unicode('%');
            break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////


