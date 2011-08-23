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

#include "svdtrans.hxx"
#include <math.h>
#include "xpoly.hxx"


#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif


namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void MoveXPoly(XPolygon& rPoly, const Size& S)
/*N*/ {
/*N*/ 	rPoly.Move(S.Width(),S.Height());
/*N*/ }

/*N*/ void MoveXPoly(XPolyPolygon& rPoly, const Size& S)
/*N*/ {
/*N*/ 	rPoly.Move(S.Width(),S.Height());
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void ResizeRect(Rectangle& rRect, const Point& rRef, const Fraction& rxFact, const Fraction& ryFact, FASTBOOL bNoJustify)
/*N*/ {
/*N*/ 	Fraction xFact(rxFact);
/*N*/ 	Fraction yFact(ryFact);
/*N*/ 	long nHgt=rRect.Bottom()-rRect.Top();
/*N*/ 
/*N*/ 	{
/*N*/ 		if (xFact.GetDenominator()==0) {
/*?*/ 			long nWdt=rRect.Right()-rRect.Left();
/*?*/ 			if (xFact.GetNumerator()>=0) { // DivZero abfangen
/*?*/ 				xFact=Fraction(xFact.GetNumerator(),1);
/*?*/ 				if (nWdt==0) rRect.Right()++;
/*?*/ 			} else {
/*?*/ 				xFact=Fraction(xFact.GetNumerator(),-1);
/*?*/ 				if (nWdt==0) rRect.Left()--;
/*N*/ 			}
/*N*/ 		}
/*N*/ 		rRect.Left()  =rRef.X()+Round(((double)(rRect.Left()  -rRef.X())*xFact.GetNumerator())/xFact.GetDenominator());
/*N*/ 		rRect.Right() =rRef.X()+Round(((double)(rRect.Right() -rRef.X())*xFact.GetNumerator())/xFact.GetDenominator());
/*N*/ 	}
/*N*/ 	{
/*N*/ 		if (yFact.GetDenominator()==0) {
/*?*/ 			long nHgt=rRect.Bottom()-rRect.Top();
/*?*/ 			if (yFact.GetNumerator()>=0) { // DivZero abfangen
/*?*/ 				yFact=Fraction(yFact.GetNumerator(),1);
/*?*/ 				if (nHgt==0) rRect.Bottom()++;
/*?*/ 			} else {
/*?*/ 				yFact=Fraction(yFact.GetNumerator(),-1);
/*?*/ 				if (nHgt==0) rRect.Top()--;
/*?*/ 			}
/*?*/ 
/*?*/ 			yFact=Fraction(yFact.GetNumerator(),1); // DivZero abfangen
/*N*/ 		}
/*N*/ 		rRect.Top()   =rRef.Y()+Round(((double)(rRect.Top()   -rRef.Y())*yFact.GetNumerator())/yFact.GetDenominator());
/*N*/ 		rRect.Bottom()=rRef.Y()+Round(((double)(rRect.Bottom()-rRef.Y())*yFact.GetNumerator())/yFact.GetDenominator());
/*N*/ 	}
/*N*/ 	if (!bNoJustify) rRect.Justify();
/*N*/ }


/*?*/ void ResizePoly(Polygon& rPoly, const Point& rRef, const Fraction& xFact, const Fraction& yFact)
/*?*/ {
/*?*/ 	USHORT nAnz=rPoly.GetSize();
/*?*/ 	for (USHORT i=0; i<nAnz; i++) {
/*?*/ 		ResizePoint(rPoly[i],rRef,xFact,yFact);
/*?*/ 	}
/*?*/ }

/*N*/ void ResizeXPoly(XPolygon& rPoly, const Point& rRef, const Fraction& xFact, const Fraction& yFact)
/*N*/ {
/*N*/ 	USHORT nAnz=rPoly.GetPointCount();
/*N*/ 	for (USHORT i=0; i<nAnz; i++) {
/*N*/ 		ResizePoint(rPoly[i],rRef,xFact,yFact);
/*N*/ 	}
/*N*/ }

/*N*/ void ResizeXPoly(XPolyPolygon& rPoly, const Point& rRef, const Fraction& xFact, const Fraction& yFact)
/*N*/ {
/*N*/ 	USHORT nAnz=rPoly.Count();
/*N*/ 	for (USHORT i=0; i<nAnz; i++) {
/*N*/ 		ResizeXPoly(rPoly[i],rRef,xFact,yFact);
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void RotatePoly(Polygon& rPoly, const Point& rRef, double sn, double cs)
/*N*/ {
/*N*/ 	USHORT nAnz=rPoly.GetSize();
/*N*/ 	for (USHORT i=0; i<nAnz; i++) {
/*N*/ 		RotatePoint(rPoly[i],rRef,sn,cs);
/*N*/ 	}
/*N*/ }

/*N*/ void RotateXPoly(XPolygon& rPoly, const Point& rRef, double sn, double cs)
/*N*/ {
/*N*/ 	USHORT nAnz=rPoly.GetPointCount();
/*N*/ 	for (USHORT i=0; i<nAnz; i++) {
/*N*/ 		RotatePoint(rPoly[i],rRef,sn,cs);
/*N*/ 	}
/*N*/ }

/*N*/ void RotateXPoly(XPolyPolygon& rPoly, const Point& rRef, double sn, double cs)
/*N*/ {
/*N*/ 	USHORT nAnz=rPoly.Count();
/*N*/ 	for (USHORT i=0; i<nAnz; i++) {
/*N*/ 		RotateXPoly(rPoly[i],rRef,sn,cs);
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////







////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void ShearPoly(Polygon& rPoly, const Point& rRef, double tn, FASTBOOL bVShear)
/*N*/ {
/*N*/ 	USHORT nAnz=rPoly.GetSize();
/*N*/ 	for (USHORT i=0; i<nAnz; i++) {
/*N*/ 		ShearPoint(rPoly[i],rRef,tn,bVShear);
/*N*/ 	}
/*N*/ }

/*N*/ void ShearXPoly(XPolygon& rPoly, const Point& rRef, double tn, FASTBOOL bVShear)
/*N*/ {
/*N*/ 	USHORT nAnz=rPoly.GetPointCount();
/*N*/ 	for (USHORT i=0; i<nAnz; i++) {
/*N*/ 		ShearPoint(rPoly[i],rRef,tn,bVShear);
/*N*/ 	}
/*N*/ }

/*N*/ void ShearXPoly(XPolyPolygon& rPoly, const Point& rRef, double tn, FASTBOOL bVShear)
/*N*/ {
/*N*/ 	USHORT nAnz=rPoly.Count();
/*N*/ 	for (USHORT i=0; i<nAnz; i++) {
/*N*/ 		ShearXPoly(rPoly[i],rRef,tn,bVShear);
/*N*/ 	}
/*N*/ }

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




////////////////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ long GetAngle(const Point& rPnt)
/*N*/ {
/*N*/ 	long a=0;
/*N*/ 	if (rPnt.Y()==0) {
/*N*/ 		if (rPnt.X()<0) a=-18000;
/*N*/ 	} else if (rPnt.X()==0) {
/*N*/ 		if (rPnt.Y()>0) a=-9000;
/*N*/ 		else a=9000;
/*N*/ 	} else {
/*N*/ 		a=Round((atan2((double)-rPnt.Y(),(double)rPnt.X())/nPi180));
/*N*/ 	}
/*N*/ 	return a;
/*N*/ }

/*N*/ long NormAngle180(long a)
/*N*/ {
/*N*/ 	while (a<18000) a+=36000;
/*N*/ 	while (a>=18000) a-=36000;
/*N*/ 	return a;
/*N*/ }

/*N*/ long NormAngle360(long a)
/*N*/ {
/*N*/ 	while (a<0) a+=36000;
/*N*/ 	while (a>=36000) a-=36000;
/*N*/ 	return a;
/*N*/ }

/*N*/ long GetLen(const Point& rPnt)
/*N*/ {
/*N*/ 	long x=Abs(rPnt.X());
/*N*/ 	long y=Abs(rPnt.Y());
/*N*/ 	if (x+y<0x8000) { // weil 7FFF * 7FFF * 2 = 7FFE0002
/*N*/ 		x*=x;
/*N*/ 		y*=y;
/*N*/ 		x+=y;
/*N*/ 		x=Round(sqrt((double)x));
/*N*/ 		return x;
/*N*/ 	} else {
/*?*/ 		double nx=x;
/*?*/ 		double ny=y;
/*?*/ 		nx*=nx;
/*?*/ 		ny*=ny;
/*?*/ 		nx+=ny;
/*?*/ 		nx=sqrt(nx);
/*?*/ 		if (nx>0x7FFFFFFF) {
/*?*/ 			return 0x7FFFFFFF; // Ueberlauf, mehr is nich!
/*?*/ 		} else {
/*?*/ 			return Round(nx);
/*?*/ 		}
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ void GeoStat::RecalcSinCos()
/*N*/ {
/*N*/ 	if (nDrehWink==0) {
/*N*/ 		nSin=0.0;
/*N*/ 		nCos=1.0;
/*N*/ 	} else {
/*N*/ 		double a=nDrehWink*nPi180;
/*N*/ 		nSin=sin(a);
/*N*/ 		nCos=cos(a);
/*N*/ 	}
/*N*/ }
/*N*/ 
/*N*/ void GeoStat::RecalcTan()
/*N*/ {
/*N*/ 	if (nShearWink==0) {
/*N*/ 		nTan=0.0;
/*N*/ 	} else {
/*N*/ 		double a=nShearWink*nPi180;
/*N*/ 		nTan=tan(a);
/*N*/ 	}
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ Polygon Rect2Poly(const Rectangle& rRect, const GeoStat& rGeo)
/*N*/ {
/*N*/ 	Polygon aPol(5);
/*N*/ 	aPol[0]=rRect.TopLeft();
/*N*/ 	aPol[1]=rRect.TopRight();
/*N*/ 	aPol[2]=rRect.BottomRight();
/*N*/ 	aPol[3]=rRect.BottomLeft();
/*N*/ 	aPol[4]=rRect.TopLeft();
/*N*/ 	if (rGeo.nShearWink!=0) ShearPoly(aPol,rRect.TopLeft(),rGeo.nTan);
/*N*/ 	if (rGeo.nDrehWink!=0) RotatePoly(aPol,rRect.TopLeft(),rGeo.nSin,rGeo.nCos);
/*N*/ 	return aPol;
/*N*/ }

/*N*/ void Poly2Rect(const Polygon& rPol, Rectangle& rRect, GeoStat& rGeo)
/*N*/ {
/*N*/ 	rGeo.nDrehWink=GetAngle(rPol[1]-rPol[0]);
/*N*/ 	rGeo.nDrehWink=NormAngle360(rGeo.nDrehWink);
/*N*/ 	// Drehung ist damit im Kasten
/*N*/ 	rGeo.RecalcSinCos();
/*N*/ 
/*N*/ 	Point aPt1(rPol[1]-rPol[0]);
/*N*/ 	if (rGeo.nDrehWink!=0) RotatePoint(aPt1,Point(0,0),-rGeo.nSin,rGeo.nCos); // -Sin fuer Rueckdrehung
/*N*/ 	long nWdt=aPt1.X();
/*N*/ 
/*N*/ 	Point aPt0(rPol[0]);
/*N*/ 	Point aPt3(rPol[3]-rPol[0]);
/*N*/ 	if (rGeo.nDrehWink!=0) RotatePoint(aPt3,Point(0,0),-rGeo.nSin,rGeo.nCos); // -Sin fuer Rueckdrehung
/*N*/ 	long nHgt=aPt3.Y();
/*N*/ 	long nShW=GetAngle(aPt3);
/*N*/ 	nShW-=27000; // ShearWink wird zur Senkrechten gemessen
/*N*/ 	nShW=-nShW;  // Negieren, denn '+' ist Rechtskursivierung
/*N*/ 
/*N*/ 	FASTBOOL bMirr=aPt3.Y()<0;
/*N*/ 	if (bMirr) { // "Punktetausch" bei Spiegelung
/*N*/ 		nHgt=-nHgt;
/*N*/ 		nShW+=18000;
/*N*/ 		aPt0=rPol[3];
/*N*/ 	}
/*N*/ 	nShW=NormAngle180(nShW);
/*N*/ 	if (nShW<-9000 || nShW>9000) {
/*N*/ 		nShW=NormAngle180(nShW+18000);
/*N*/ 	}
/*N*/ 	if (nShW<-SDRMAXSHEAR) nShW=-SDRMAXSHEAR; // ShearWinkel begrenzen auf +/- 89.00 deg
/*N*/ 	if (nShW>SDRMAXSHEAR)  nShW=SDRMAXSHEAR;
/*N*/ 	rGeo.nShearWink=nShW;
/*N*/ 	rGeo.RecalcTan();
/*N*/ 	Point aRU(aPt0);
/*N*/ 	aRU.X()+=nWdt;
/*N*/ 	aRU.Y()+=nHgt;
/*N*/ 	rRect=Rectangle(aPt0,aRU);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////

/*N*/ long BigMulDiv(long nVal, long nMul, long nDiv)
/*N*/ {
/*N*/ 	BigInt aVal(nVal);
/*N*/ 	aVal*=nMul;
/*N*/ 	if (aVal.IsNeg()!=(nDiv<0)) {
/*N*/ 		aVal-=nDiv/2; // fuer korrektes Runden
/*N*/ 	} else {
/*N*/ 		aVal+=nDiv/2; // fuer korrektes Runden
/*N*/ 	}
/*N*/ 	if(nDiv)
/*N*/ 	{
/*N*/ 		aVal/=nDiv;
/*N*/ 		return long(aVal);
/*N*/ 	}
/*N*/ 	return 0x7fffffff;
/*N*/ }

/*NBFF*/ void Kuerzen(Fraction& rF, unsigned nDigits)
/*NBFF*/ {
/*NBFF*/ 	INT32 nMul=rF.GetNumerator();
/*NBFF*/ 	INT32 nDiv=rF.GetDenominator();
/*NBFF*/ 	FASTBOOL bNeg=FALSE;
/*NBFF*/ 	if (nMul<0) { nMul=-nMul; bNeg=!bNeg; }
/*NBFF*/ 	if (nDiv<0) { nDiv=-nDiv; bNeg=!bNeg; }
/*NBFF*/ 	if (nMul==0 || nDiv==0) return;
/*NBFF*/ 	UINT32 a;
/*NBFF*/ 	a=UINT32(nMul); unsigned nMulZ=0; // Fuehrende Nullen zaehlen
/*NBFF*/ 	while (a<0x00800000) { nMulZ+=8; a<<=8; }
/*NBFF*/ 	while (a<0x80000000) { nMulZ++; a<<=1; }
/*NBFF*/ 	a=UINT32(nDiv); unsigned nDivZ=0; // Fuehrende Nullen zaehlen
/*NBFF*/ 	while (a<0x00800000) { nDivZ+=8; a<<=8; }
/*NBFF*/ 	while (a<0x80000000) { nDivZ++; a<<=1; }
/*NBFF*/ 	// Anzahl der verwendeten Digits bestimmen
/*NBFF*/ 	int nMulDigits=32-nMulZ;
/*NBFF*/ 	int nDivDigits=32-nDivZ;
/*NBFF*/ 	// Nun bestimmen, wieviele Stellen hinten weg koennen
/*NBFF*/ 	int nMulWeg=nMulDigits-nDigits; if (nMulWeg<0) nMulWeg=0;
/*NBFF*/ 	int nDivWeg=nDivDigits-nDigits; if (nDivWeg<0) nDivWeg=0;
/*NBFF*/ 	int nWeg=Min(nMulWeg,nDivWeg);
/*NBFF*/ 	nMul>>=nWeg;
/*NBFF*/ 	nDiv>>=nWeg;
/*NBFF*/ 	if (nMul==0 || nDiv==0) {
/*NBFF*/ 		DBG_WARNING("Oups, beim kuerzen einer Fraction hat sich Joe verrechnet.");
/*NBFF*/ 		return;
/*NBFF*/ 	}
/*NBFF*/ 	if (bNeg) nMul=-nMul;
/*NBFF*/ 	rF=Fraction(nMul,nDiv);
/*NBFF*/ }

////////////////////////////////////////////////////////////////////////////////////////////////////
// Wieviele eU-Einheiten passen in einen mm bzw. Inch?
// Oder wie gross ist ein eU in mm bzw. Inch, und davon der Kehrwert



// Den Faktor berechnen, der anzuwenden ist um n Einheiten von eS nach
// eD umzurechnen. Z.B. GetMapFactor(UNIT_MM,UNIT_100TH_MM) => 100.




/*?*/ FrPair GetMapFactor(FieldUnit eS, FieldUnit eD)
/*?*/ {
/*?*/ 	if (eS==eD) return FrPair(1,1,1,1);
/*?*/ {DBG_BF_ASSERT(0, "STRIP");}return FrPair(1,1,1,1);//STRIP001 //STRIP001 /*?*/ 	FrPair aS(GetInchOrMM(eS));
/*?*/ };

////////////////////////////////////////////////////////////////////////////////////////////////////

    // 1 mile    =  8 furlong = 63.360" = 1.609.344,0mm
    // 1 furlong = 10 chains  =  7.920" =   201.168,0mm
    // 1 chain   =  4 poles   =    792" =    20.116,8mm
    // 1 pole    =  5 1/2 yd  =    198" =     5.029,2mm
    // 1 yd      =  3 ft      =     36" =       914,4mm
    // 1 ft      = 12 "       =      1" =       304,8mm








////////////////////////////////////////////////////////////////////////////////////////////////////


}
