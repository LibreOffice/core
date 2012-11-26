/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svdtrans.hxx>
#include <math.h>
#include <svx/xpoly.hxx>
#include <vcl/virdev.hxx>
#include <tools/bigint.hxx>
#include <tools/debug.hxx>
#include <unotools/syslocale.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

void RotatePoly(Polygon& rPoly, const Point& rRef, double sn, double cs)
{
    sal_uInt16 nAnz=rPoly.GetSize();
    for (sal_uInt16 i=0; i<nAnz; i++) {
        RotatePoint(rPoly[i],rRef,sn,cs);
    }
}

void RotatePoly(PolyPolygon& rPoly, const Point& rRef, double sn, double cs)
{
    sal_uInt16 nAnz=rPoly.Count();
    for (sal_uInt16 i=0; i<nAnz; i++) {
        RotatePoly(rPoly[i],rRef,sn,cs);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

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
        double fSin=sin(a);
        double fCos=cos(a);
        RotatePoint(rPnt,Point(),fSin,fCos);
        rPnt+=rRef1;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ShearPoly(Polygon& rPoly, const Point& rRef, double tn, bool bVShear)
{
    sal_uInt16 nAnz=rPoly.GetSize();
    for (sal_uInt16 i=0; i<nAnz; i++) {
        ShearPoint(rPoly[i],rRef,tn,bVShear);
    }
}

void ShearPoly(PolyPolygon& rPoly, const Point& rRef, double tn, bool bVShear)
{
    sal_uInt16 nAnz=rPoly.Count();
    for (sal_uInt16 i=0; i<nAnz; i++) {
        ShearPoly(rPoly[i],rRef,tn,bVShear);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double CrookRotateXPoint(basegfx::B2DPoint& rPnt, basegfx::B2DPoint* pC1, basegfx::B2DPoint* pC2, const basegfx::B2DPoint& rCenter,
    const basegfx::B2DPoint& rRad, double& rSin, double& rCos, bool bVert)
{
    basegfx::B2DPoint aPoint(rPnt);
    basegfx::B2DPoint aCenter(rCenter);
    double nWink(GetCrookAngle(rPnt, rCenter, rRad, bVert));
    double sn(sin(nWink));
    double cs(cos(nWink));

    RotateB2DPointAroundRef(rPnt, rCenter, sn, cs);

    if(pC1)
    {
        if(bVert)
        {
            // Richtung Zentrum verschieben, als Ausgangsposition fuer Rotate
            pC1->setY(pC1->getY() - aPoint.getY());

            // Resize, entsprechend der Entfernung vom Zentrum
            pC1->setY((pC1->getY() / rRad.getX() * (aCenter.getX() - pC1->getX())) + aCenter.getY());
        }
        else
        {
            // Richtung Zentrum verschieben, als Ausgangsposition fuer Rotate
            pC1->setX(pC1->getX() - aPoint.getX());

            // Resize, entsprechend der Entfernung vom Zentrum
            const double fPntRad(aCenter.getY() - pC1->getY());
            const double fFact(fPntRad / rRad.getY());

            pC1->setX((pC1->getX() * fFact) + aCenter.getX());
        }

        RotateB2DPointAroundRef(*pC1, rCenter, sn, cs);
    }

    if(pC2)
    {
        if(bVert)
        {
            // Richtung Zentrum verschieben, als Ausgangsposition fuer Rotate
            pC2->setY(pC2->getY() - aPoint.getY());

            // Resize, entsprechend der Entfernung vom Zentrum
            pC2->setY((pC2->getY() / rRad.getX() * (rCenter.getX() - pC2->getX())) + aCenter.getY());
        }
        else
        {
            // Richtung Zentrum verschieben, als Ausgangsposition fuer Rotate
            pC2->setX(pC2->getX() - aPoint.getX());

            // Resize, entsprechend der Entfernung vom Zentrum
            const double fPntRad(rCenter.getY() - pC2->getY());
            const double fFact(fPntRad / rRad.getY());

            pC2->setX((pC2->getX() * fFact) + aCenter.getX());
        }

        RotateB2DPointAroundRef(*pC2, rCenter, sn, cs);
    }

    rSin = sn;
    rCos = cs;

    return nWink;
}

double CrookSlantXPoint(basegfx::B2DPoint& rPnt, basegfx::B2DPoint* pC1, basegfx::B2DPoint* pC2, const basegfx::B2DPoint& rCenter,
    const basegfx::B2DPoint& rRad, double& rSin, double& rCos, bool bVert)
{
    basegfx::B2DPoint aPoint(rPnt);
    basegfx::B2DPoint aDelta1(0.0, 0.0);
    basegfx::B2DPoint aDeltaC1(0.0, 0.0);
    basegfx::B2DPoint aDeltaC2(0.0, 0.0);

    if(bVert)
    {
        const double fStart(rCenter.getX() - rRad.getX());

        aDelta1.setX(rPnt.getX() - fStart);
        rPnt.setX(fStart);

        if(pC1)
        {
            aDeltaC1.setX(pC1->getX() - fStart);
            pC1->setX(fStart);
        }

        if(pC2)
        {
            aDeltaC2.setX(pC2->getX() - fStart);
            pC2->setX(fStart);
        }
    }
    else
    {
        const double fStart(rCenter.getY() - rRad.getY());

        aDelta1.setY(rPnt.getY() - fStart);
        rPnt.setY(fStart);

        if(pC1)
        {
            aDeltaC1.setY(pC1->getY() - fStart);
            pC1->setY(fStart);
        }

        if(pC2)
        {
            aDeltaC2.setY(pC2->getY() - fStart);
            pC2->setY(fStart);
        }
    }

    double nWink(GetCrookAngle(rPnt, rCenter, rRad, bVert));
    double sn(sin(nWink));
    double cs(cos(nWink));

    RotateB2DPointAroundRef(rPnt, rCenter, sn, cs);

    if(pC1)
    {
        if(bVert)
        {
            pC1->setY(pC1->getY() - (aPoint.getY() - rCenter.getY()));
        }
        else
        {
            pC1->setX(pC1->getX() - (aPoint.getX() - rCenter.getX()));
        }

        RotateB2DPointAroundRef(*pC1, rCenter, sn, cs);
    }

    if(pC2)
    {
        if(bVert)
        {
            pC2->setY(pC2->getY() - (aPoint.getY() - rCenter.getY()));
        }
        else
        {
            pC2->setX(pC2->getX() - (aPoint.getX() - rCenter.getX()));
        }

        RotateB2DPointAroundRef(*pC2, rCenter, sn, cs);
    }

    if(bVert)
    {
        rPnt.setX(rPnt.getX() + aDelta1.getX());

        if(pC1)
        {
            pC1->setX(pC1->getX() + aDeltaC1.getX());
        }

        if(pC2)
        {
            pC2->setX(pC2->getX() + aDeltaC2.getX());
        }
    }
    else
    {
        rPnt.setY(rPnt.getY() + aDelta1.getY());

        if(pC1)
        {
            pC1->setY(pC1->getY() + aDeltaC1.getY());
        }

        if(pC2)
        {
            pC2->setY(pC2->getY() + aDeltaC2.getY());
        }
    }

    rSin=sn;
    rCos=cs;

    return nWink;
}

double CrookStretchXPoint(basegfx::B2DPoint& rPnt, basegfx::B2DPoint* pC1, basegfx::B2DPoint* pC2, const basegfx::B2DPoint& rCenter,
                          const basegfx::B2DPoint& rRad, double& rSin, double& rCos, bool bVert,
                          const basegfx::B2DRange& rRefRange)
{
    const double y0(rPnt.getY());

    CrookSlantXPoint(rPnt,pC1,pC2,rCenter,rRad,rSin,rCos,bVert);

    if(!bVert)
    {
        const double fTop(rRefRange.getMinY());
        const double fBtm(rRefRange.getMaxY());
        const double fHgt(fBtm - fTop);
        const double dy(rPnt.getY() - y0);
        const double a(((y0 - fTop) / fHgt) * dy);

        rPnt.setY(y0 + a);
    }

    return 0.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CrookRotatePoly(basegfx::B2DPolygon& rPoly, const basegfx::B2DPoint& rCenter, const basegfx::B2DPoint& rRad, bool bVert)
{
    double fSin(0.0);
    double fCos(0.0);

    for(sal_uInt32 a(0); a < rPoly.count(); a++)
    {
        basegfx::B2DPoint aPoint(rPoly.getB2DPoint(a));

        if(rPoly.areControlPointsUsed() && (rPoly.isPrevControlPointUsed(a) || rPoly.isNextControlPointUsed(a)))
        {
            basegfx::B2DPoint aPrev(rPoly.getPrevControlPoint(a));
            basegfx::B2DPoint aNext(rPoly.getNextControlPoint(a));

            CrookRotateXPoint(aPoint, &aPrev, &aNext, rCenter, rRad, fSin, fCos, bVert);
            rPoly.setB2DPoint(a, aPoint);
            rPoly.setControlPoints(a, aPrev, aNext);
        }
        else
        {
            CrookRotateXPoint(aPoint, 0, 0, rCenter, rRad, fSin, fCos, bVert);
            rPoly.setB2DPoint(a, aPoint);
        }
    }
}

void CrookSlantPoly(basegfx::B2DPolygon& rPoly, const basegfx::B2DPoint& rCenter, const basegfx::B2DPoint& rRad, bool bVert)
{
    double fSin(0.0);
    double fCos(0.0);

    for(sal_uInt32 a(0); a < rPoly.count(); a++)
    {
        basegfx::B2DPoint aPoint(rPoly.getB2DPoint(a));

        if(rPoly.areControlPointsUsed() && (rPoly.isPrevControlPointUsed(a) || rPoly.isNextControlPointUsed(a)))
        {
            basegfx::B2DPoint aPrev(rPoly.getPrevControlPoint(a));
            basegfx::B2DPoint aNext(rPoly.getNextControlPoint(a));

            CrookSlantXPoint(aPoint, &aPrev, &aNext, rCenter, rRad, fSin, fCos, bVert);
            rPoly.setB2DPoint(a, aPoint);
            rPoly.setControlPoints(a, aPrev, aNext);
        }
        else
        {
            CrookSlantXPoint(aPoint, 0, 0, rCenter, rRad, fSin, fCos, bVert);
            rPoly.setB2DPoint(a, aPoint);
        }
    }
}

void CrookStretchPoly(basegfx::B2DPolygon& rPoly, const basegfx::B2DPoint& rCenter, const basegfx::B2DPoint& rRad, bool bVert, const basegfx::B2DRange& rRefRange)
{
    double fSin(0.0);
    double fCos(0.0);

    for(sal_uInt32 a(0); a < rPoly.count(); a++)
    {
        basegfx::B2DPoint aPoint(rPoly.getB2DPoint(a));

        if(rPoly.areControlPointsUsed() && (rPoly.isPrevControlPointUsed(a) || rPoly.isNextControlPointUsed(a)))
        {
            basegfx::B2DPoint aPrev(rPoly.getPrevControlPoint(a));
            basegfx::B2DPoint aNext(rPoly.getNextControlPoint(a));

            CrookStretchXPoint(aPoint, &aPrev, &aNext, rCenter, rRad, fSin, fCos, bVert, rRefRange);
            rPoly.setB2DPoint(a, aPoint);
            rPoly.setControlPoints(a, aPrev, aNext);
        }
        else
        {
            CrookStretchXPoint(aPoint, 0, 0, rCenter, rRad, fSin, fCos, bVert, rRefRange);
            rPoly.setB2DPoint(a, aPoint);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void CrookRotatePoly(basegfx::B2DPolyPolygon& rPoly, const basegfx::B2DPoint& rCenter, const basegfx::B2DPoint& rRad, bool bVert)
{
    for(sal_uInt32 a(0); a < rPoly.count(); a++)
    {
        basegfx::B2DPolygon aCandidate(rPoly.getB2DPolygon(a));

        CrookRotatePoly(aCandidate, rCenter, rRad, bVert);
        rPoly.setB2DPolygon(a, aCandidate);
    }
}

void CrookSlantPoly(basegfx::B2DPolyPolygon& rPoly, const basegfx::B2DPoint& rCenter, const basegfx::B2DPoint& rRad, bool bVert)
{
    for(sal_uInt32 a(0); a < rPoly.count(); a++)
    {
        basegfx::B2DPolygon aCandidate(rPoly.getB2DPolygon(a));

        CrookSlantPoly(aCandidate, rCenter, rRad, bVert);
        rPoly.setB2DPolygon(a, aCandidate);
    }
}

void CrookStretchPoly(basegfx::B2DPolyPolygon& rPoly, const basegfx::B2DPoint& rCenter, const basegfx::B2DPoint& rRad, bool bVert, const basegfx::B2DRange& rRefRange)
{
    for(sal_uInt32 a(0); a < rPoly.count(); a++)
    {
        basegfx::B2DPolygon aCandidate(rPoly.getB2DPolygon(a));

        CrookStretchPoly(aCandidate, rCenter, rRad, bVert, rRefRange);
        rPoly.setB2DPolygon(a, aCandidate);
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

long BigMulDiv(long nVal, long nMul, long nDiv)
{
    BigInt aVal(nVal);
    aVal*=nMul;
    if (aVal.IsNeg()!=(nDiv<0)) {
        aVal-=nDiv/2; // fuer korrektes Runden
    } else {
        aVal+=nDiv/2; // fuer korrektes Runden
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
    bool bNeg=sal_False;
    if (nMul<0) { nMul=-nMul; bNeg=!bNeg; }
    if (nDiv<0) { nDiv=-nDiv; bNeg=!bNeg; }
    if (nMul==0 || nDiv==0) return;
    sal_uInt32 a;
    a=sal_uInt32(nMul); unsigned nMulZ=0; // Fuehrende Nullen zaehlen
    while (a<0x00800000) { nMulZ+=8; a<<=8; }
    while (a<0x80000000) { nMulZ++; a<<=1; }
    a=sal_uInt32(nDiv); unsigned nDivZ=0; // Fuehrende Nullen zaehlen
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

// Den Faktor berechnen, der anzuwenden ist um n Einheiten von eS nach
// eD umzurechnen. Z.B. GetMapFactor(UNIT_MM,UNIT_100TH_MM) => 100.

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

FrPair GetMapFactor(MapUnit eS, FieldUnit eD)
{
    FrPair aS(GetInchOrMM(eS));
    FrPair aD(GetInchOrMM(eD));
    bool bSInch=IsInch(eS);
    bool bDInch=IsInch(eD);
    FrPair aRet(aD.X()/aS.X(),aD.Y()/aS.Y());
    if (bSInch && !bDInch) { aRet.X()*=Fraction(127,5); aRet.Y()*=Fraction(127,5); }
    if (!bSInch && bDInch) { aRet.X()*=Fraction(5,127); aRet.Y()*=Fraction(5,127); }
    return aRet;
};

FrPair GetMapFactor(FieldUnit eS, MapUnit eD)
{
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
    bool bMetr=false,bInch=false;
    switch (eMU) {
        // Metrisch
        case MAP_100TH_MM   : bMetr=true; nKomma=5; break;
        case MAP_10TH_MM    : bMetr=true; nKomma=4; break;
        case MAP_MM         : bMetr=true; nKomma=3; break;
        case MAP_CM         : bMetr=true; nKomma=2; break;
        // Inch
        case MAP_1000TH_INCH: bInch=true; nKomma=3; break;
        case MAP_100TH_INCH : bInch=true; nKomma=2; break;
        case MAP_10TH_INCH  : bInch=true; nKomma=1; break;
        case MAP_INCH       : bInch=true; nKomma=0; break;
        case MAP_POINT      : bInch=true; rnDiv=72;  break;          // 1Pt   = 1/72"
        case MAP_TWIP       : bInch=true; rnDiv=144; nKomma=1; break; // 1Twip = 1/1440"
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
    bool bMetr=false,bInch=false;
    switch (eFU) {
        case FUNIT_NONE     : break;
        // Metrisch
        case FUNIT_100TH_MM : bMetr=true; nKomma=5; break;
        case FUNIT_MM       : bMetr=true; nKomma=3; break;
        case FUNIT_CM       : bMetr=true; nKomma=2; break;
        case FUNIT_M        : bMetr=true; nKomma=0; break;
        case FUNIT_KM       : bMetr=true; nKomma=-3; break;
        // Inch
        case FUNIT_TWIP     : bInch=true; rnDiv=144; nKomma=1; break;  // 1Twip = 1/1440"
        case FUNIT_POINT    : bInch=true; rnDiv=72; break;   // 1Pt   = 1/72"
        case FUNIT_PICA     : bInch=true; rnDiv=6; break;    // 1Pica = 1/6"  ?
        case FUNIT_INCH     : bInch=true; break;             // 1"    = 1"
        case FUNIT_FOOT     : bInch=true; rnMul=12; break;   // 1Ft   = 12"
        case FUNIT_MILE     : bInch=true; rnMul=6336; nKomma=-1; break; // 1mile = 63360"
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
    bool bSrcMetr,bSrcInch,bDstMetr,bDstInch;
    long nMul1,nDiv1,nMul2,nDiv2;
    short nKomma1,nKomma2;

    // Zunaechst normalisieren auf m bzw. "
    if (!bSrcFU)
    {
        GetMeterOrInch(eSrcMU,nKomma1,nMul1,nDiv1,bSrcMetr,bSrcInch);
    }
    else
    {
        GetMeterOrInch(eSrcFU,nKomma1,nMul1,nDiv1,bSrcMetr,bSrcInch);
    }

    if (!bDstFU)
    {
        GetMeterOrInch(eDstMU,nKomma2,nMul2,nDiv2,bDstMetr,bDstInch);
    }
    else
    {
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

    // Temporaere Fraction zum Kuerzen
    Fraction aTempFract(nMul1,nDiv1);
    nMul1=aTempFract.GetNumerator();
    nDiv1=aTempFract.GetDenominator();

    nMul_=nMul1;
    nDiv_=nDiv1;
    nKomma_=nKomma1;
    bDirty=sal_False;
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
    sal_Bool bNeg(nVal < 0);
    SvtSysLocale aSysLoc;
    const LocaleDataWrapper& rLoc = aSysLoc.GetLocaleData();

    ForceUndirty();

    sal_Int16 nK(nKomma_);
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

    if(nK > 0 && aStr.Len() <= nK )
    {
        // Komma erforderlich
        sal_Int16 nAnz(nK - aStr.Len());

        if(nAnz >= 0 && rLoc.isNumLeadingZero())
            nAnz++;

        for(xub_StrLen  i=0; i<nAnz; i++)
            aStr.Insert(aNullCode, 0);

        // zuviele Nachkommastellen abhacken
        xub_StrLen nNumDigits(rLoc.getNumDigits());
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
            sal_Unicode cDec(rLoc.getNumDecimalSep().GetChar(0));
            aStr.Insert(cDec, nVorKomma);
        }
    }

    // ggf. Trennpunkte bei jedem Tausender einfuegen
    if( nVorKomma > 3 )
    {
        String aThoSep( rLoc.getNumThousandSep() );
        if ( aThoSep.Len() > 0 )
        {
            sal_Unicode cTho( aThoSep.GetChar(0) );
            sal_Int32 i(nVorKomma - 3);

            while(i > 0)
            {
                rStr.Insert(cTho, (xub_StrLen)i);
                i -= 3;
            }
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
    const sal_Char* pText;

    switch(eUnit)
    {
        // metric units
        case MAP_100TH_MM   : pText = "/100mm"; break;
        case MAP_10TH_MM    : pText = "/10mm"; break;
        case MAP_MM         : pText = "mm"; break;
        case MAP_CM         : pText = "cm"; break;

        // imperial units
        case MAP_1000TH_INCH: pText = "/1000\""; break;
        case MAP_100TH_INCH : pText = "/100\""; break;
        case MAP_10TH_INCH  : pText = "/10\""; break;
        case MAP_INCH       : pText = "\""; break;
        case MAP_POINT      : pText = "pt"; break;
        case MAP_TWIP       : pText = "twip"; break;

        // other units
        case MAP_PIXEL      : pText = "pixel"; break;
        case MAP_SYSFONT    : pText = "sysfont"; break;
        case MAP_APPFONT    : pText = "appfont"; break;
        case MAP_RELATIVE   : pText = "%"; break;

        default             : pText = ""; break;
    }

    rStr = XubString::CreateFromAscii( pText );
}

void SdrFormatter::TakeUnitStr(FieldUnit eUnit, XubString& rStr)
{
    const sal_Char* pText;

    switch(eUnit)
    {
        // metric units
        case FUNIT_100TH_MM : pText = "/100mm"; break;
        case FUNIT_MM       : pText = "mm"; break;
        case FUNIT_CM       : pText = "cm"; break;
        case FUNIT_M        : pText = "m"; break;
        case FUNIT_KM       : pText = "km"; break;

        // imperial units
        case FUNIT_TWIP     : pText = "twip"; break;
        case FUNIT_POINT    : pText = "pt"; break;
        case FUNIT_PICA     : pText = "pica"; break;
        case FUNIT_INCH     : pText = "\""; break;
        case FUNIT_FOOT     : pText = "ft"; break;
        case FUNIT_MILE     : pText = "mile(s)"; break;

        // other units
        case FUNIT_PERCENT: pText = "%"; break;

//      case FUNIT_NONE     :
//      case FUNIT_CUSTOM   :
        default             : pText = ""; break;
    }

    rStr = XubString::CreateFromAscii( pText );
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof
