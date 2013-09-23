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

#ifndef _SVDTRANS_HXX
#define _SVDTRANS_HXX

#include <rtl/ustring.hxx>
#include <svx/svxdllapi.h>
#include <tools/fract.hxx>
#include <tools/gen.hxx>
#include <tools/poly.hxx>
#include <vcl/field.hxx>
#include <vcl/mapmod.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

// Winkelangaben der DrawingEngine sind 1/100 Degree
// #i19054# nowhere used, removed // const int nWinkDiv=100;
// Um Winkel der DrawingEngine mit den Trigonometrischen Funktionen
// verarbeiten zu koennen, muessen sie zunaest ins Bogenmass umgerechnet
// werden. Dies gestaltet sich recht einfach mit der folgenden Konstanten
// nPi180. Sei nWink ein Winkel in 1/100 Deg so schreibt man z.B.:
//   double nSin=sin(nWink*nPi180);
// Rueckwandlung entsprechend durch Teilen.
const double nPi=3.14159265358979323846;
const double nPi180=0.000174532925199432957692222; // Bei zuweing Stellen ist tan(4500*nPi180)!=1.0

// Der maximale Shearwinkel
#define SDRMAXSHEAR 8900

class XPolygon;
class XPolyPolygon;

inline long Round(double a) { return a>0.0 ? (long)(a+0.5) : -(long)((-a)+0.5); }

inline void MoveRect(Rectangle& rRect, const Size& S)    { rRect.Move(S.Width(),S.Height()); }
inline void MovePoint(Point& rPnt, const Size& S)        { rPnt.X()+=S.Width(); rPnt.Y()+=S.Height(); }
inline void MovePoly(Polygon& rPoly, const Size& S)      { rPoly.Move(S.Width(),S.Height()); }
inline void MovePoly(PolyPolygon& rPoly, const Size& S)  { rPoly.Move(S.Width(),S.Height()); }
void MoveXPoly(XPolygon& rPoly, const Size& S);

SVX_DLLPUBLIC void ResizeRect(Rectangle& rRect, const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bNoJustify = false);
inline void ResizePoint(Point& rPnt, const Point& rRef, Fraction xFact, Fraction yFact);
void ResizePoly(Polygon& rPoly, const Point& rRef, const Fraction& xFact, const Fraction& yFact);
void ResizeXPoly(XPolygon& rPoly, const Point& rRef, const Fraction& xFact, const Fraction& yFact);

inline void RotatePoint(Point& rPnt, const Point& rRef, double sn, double cs);
SVX_DLLPUBLIC void RotatePoly(Polygon& rPoly, const Point& rRef, double sn, double cs);
void RotateXPoly(XPolygon& rPoly, const Point& rRef, double sn, double cs);
void RotateXPoly(XPolyPolygon& rPoly, const Point& rRef, double sn, double cs);

void MirrorPoint(Point& rPnt, const Point& rRef1, const Point& rRef2);
void MirrorPoly(Polygon& rPoly, const Point& rRef1, const Point& rRef2);
void MirrorXPoly(XPolygon& rPoly, const Point& rRef1, const Point& rRef2);

inline void ShearPoint(Point& rPnt, const Point& rRef, double tn, bool bVShear = false);
SVX_DLLPUBLIC void ShearPoly(Polygon& rPoly, const Point& rRef, double tn, bool bVShear = false);
void ShearXPoly(XPolygon& rPoly, const Point& rRef, double tn, bool bVShear = false);

// rPnt.X bzw rPnt.Y wird auf rCenter.X bzw. rCenter.Y gesetzt!
// anschliessend muss rPnt nur noch um rCenter gedreht werden.
// Der Rueckgabewinkel ist ausnahmsweise in Rad.
inline double GetCrookAngle(Point& rPnt, const Point& rCenter, const Point& rRad, bool bVertical);
// Die folgenden Methoden behandeln einen Punkt eines XPolygons, wobei die
// benachbarten Kontrollpunkte des eigentlichen Punktes ggf. in pC1/pC2
// uebergeben werden. Ueber rSin/rCos wird gleichzeitig sin(nWink) und cos(nWink)
// zurueckgegeben.
// Der Rueckgabewinkel ist hier ebenfalls in Rad.
double CrookRotateXPoint(Point& rPnt, Point* pC1, Point* pC2, const Point& rCenter,
                         const Point& rRad, double& rSin, double& rCos, bool bVert);
double CrookSlantXPoint(Point& rPnt, Point* pC1, Point* pC2, const Point& rCenter,
                        const Point& rRad, double& rSin, double& rCos, bool bVert);
double CrookStretchXPoint(Point& rPnt, Point* pC1, Point* pC2, const Point& rCenter,
                          const Point& rRad, double& rSin, double& rCos, bool bVert,
                          const Rectangle rRefRect);

void CrookRotatePoly(XPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert);
void CrookSlantPoly(XPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert);
void CrookStretchPoly(XPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert, const Rectangle rRefRect);

void CrookRotatePoly(XPolyPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert);
void CrookSlantPoly(XPolyPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert);
void CrookStretchPoly(XPolyPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert, const Rectangle rRefRect);

/**************************************************************************************************/
/*  Inline                                                                                        */
/**************************************************************************************************/

inline void ResizePoint(Point& rPnt, const Point& rRef, Fraction xFact, Fraction yFact)
{
    if (xFact.GetDenominator()==0) xFact=Fraction(xFact.GetNumerator(),1); // DivZero abfangen
    if (yFact.GetDenominator()==0) yFact=Fraction(yFact.GetNumerator(),1); // DivZero abfangen
    rPnt.X()=rRef.X()+ Round(((double)(rPnt.X()-rRef.X())*xFact.GetNumerator())/xFact.GetDenominator());
    rPnt.Y()=rRef.Y()+ Round(((double)(rPnt.Y()-rRef.Y())*yFact.GetNumerator())/yFact.GetDenominator());
}

inline void RotatePoint(Point& rPnt, const Point& rRef, double sn, double cs)
{
    long dx=rPnt.X()-rRef.X();
    long dy=rPnt.Y()-rRef.Y();
    rPnt.X()=Round(rRef.X()+dx*cs+dy*sn);
    rPnt.Y()=Round(rRef.Y()+dy*cs-dx*sn);
}

inline void ShearPoint(Point& rPnt, const Point& rRef, double tn, bool bVShear)
{
    if (!bVShear) { // Horizontal
        if (rPnt.Y()!=rRef.Y()) { // sonst nicht noetig
            rPnt.X()-=Round((rPnt.Y()-rRef.Y())*tn);
        }
    } else { // ansonsten vertikal
        if (rPnt.X()!=rRef.X()) { // sonst nicht noetig
            rPnt.Y()-=Round((rPnt.X()-rRef.X())*tn);
        }
    }
}

inline double GetCrookAngle(Point& rPnt, const Point& rCenter, const Point& rRad, bool bVertical)
{
    double nWink;
    if (bVertical) {
        long dy=rPnt.Y()-rCenter.Y();
        nWink=(double)dy/(double)rRad.Y();
        rPnt.Y()=rCenter.Y();
    } else {
        long dx=rCenter.X()-rPnt.X();
        nWink=(double)dx/(double)rRad.X();
        rPnt.X()=rCenter.X();
    }
    return nWink;
}

/**************************************************************************************************/
/**************************************************************************************************/

// Y-Achse zeigt nach unten! Die Funktion negiert bei der
// Winkelberechnung die Y-Achse, sodass GetAngle(Point(0,-1))=90.00deg.
// GetAngle(Point(0,0)) liefert 0.
// Der Rueckgabewert liegt im Bereich -180.00..179.99 Degree und
// ist in 1/100 Degree angegeben.
SVX_DLLPUBLIC long GetAngle(const Point& rPnt);
long NormAngle180(long a); // Winkel normalisieren auf -180.00..179.99
SVX_DLLPUBLIC long NormAngle360(long a); // Winkel normalisieren auf    0.00..359.99
sal_uInt16 GetAngleSector(long nWink); // Sektor im kartesischen Koordinatensystem bestimmen
// Berechnet die Laenge von (0,0) via a^2 + b^2 = c^2
// Zur Vermeidung von Ueberlaeufen werden ggf. einige Stellen ignoriert.
long GetLen(const Point& rPnt);

/*
  Transformation eines Rechtecks in ein Polygon unter            ------------
  Anwendung der Winkelparameter aus GeoStat.                    /1        2/
  Referenzpunkt ist stets der Punkt 0, also die linke          /          /
  obere Ecke des Ausgangsrects.                               /          /
  Bei der Berechnung des Polygons ist die Reihenfolge        /          /
  (erst Shear, dann Rotation vorgegeben).                   /          / \
                                                           /          /   |
  A) Ausgangsrechteck aRect  B) Nach Anwendung von Shear  /0        3/  Rot|
  +------------------+       --------------------        ------------  ------
  |0                1|        \0                1\       C) Nach Anwendung
  |                  |         \                  \      von Rotate
  |                  |       |  \                  \
  |3                2|       |   \3                2\
  +------------------+       |    --------------------
                             |Shr |
  Bei Rueckkonvertierung des        Polygons in ein Rect ist die Reihenfolge
  zwangslaeufig umgekehrt:
  - Berechnung des Drehwinkels: Winkel der Strecke 0-1 aus Abb. C) zum Horizont
  - Rueckdrehung des geshearten Rects (man erhaelt Abb B))
  - Bestimmung der Breite des Rects=Laenge der Strecke 0-1 aus Abb. B)
  - Bestimmung der Hoehe des Rects=vertikaler Abstand zwischen den Punkten
    0 und 3 aus Abb. B)
  - Bestimmung des Shear-Winkels aus der Strecke 0-3 zur Senkrechten.
  Es ist darauf zu achten, dass das Polygon bei einer zwischenzeitlichen
  Transformation evtl. gespiegelt wurde (Mirror oder Resize mit neg. Faktor).
  In diesem Fall muss zunaecht eine Normalisierung durch Vertauschung der
  Punkte (z.B. 0 mit 3 und 1 mit 2) durchgefuehrt werden, damit der
  Richtungssinn im Polygon wieder stimmig ist.
  Hinweis: Positiver Shear-Winkel bedeutet Shear mit auf dem Bildschirm
  sichtbarer positiver Kursivierung. Mathematisch waere dass eine negative
  Kursivierung, da die Y-Achse auf dem Bildschirm von oben nach unten verlaeuft.
  Drehwinkel: Positiv bedeutet auf dem Bildschirm sichtbare Linksdrehung.
*/

class GeoStat { // Geometrischer Status fuer ein Rect
public:
    long     nDrehWink;
    long     nShearWink;
    double   nTan;      // tan(nShearWink)
    double   nSin;      // sin(nDrehWink)
    double   nCos;      // cos(nDrehWink)
    bool     bMirrored; // Horizontal gespiegelt? (ni)
public:
    GeoStat(): nDrehWink(0),nShearWink(0),nTan(0.0),nSin(0.0),nCos(1.0),bMirrored(false) {}
    void RecalcSinCos();
    void RecalcTan();
};

Polygon Rect2Poly(const Rectangle& rRect, const GeoStat& rGeo);
void Poly2Rect(const Polygon& rPol, Rectangle& rRect, GeoStat& rGeo);

SVX_DLLPUBLIC void OrthoDistance8(const Point& rPt0, Point& rPt, bool bBigOrtho);
SVX_DLLPUBLIC void OrthoDistance4(const Point& rPt0, Point& rPt, bool bBigOrtho);

// Multiplikation und anschliessende Division.
// Rechnung und Zwischenergebnis sind BigInt.
SVX_DLLPUBLIC long BigMulDiv(long nVal, long nMul, long nDiv);

// Fehlerbehaftetes Kuerzen einer Fraction.
// nDigits gibt an, wieviele signifikante Stellen in
// Zaehler/Nenner mindestens erhalten bleiben sollen.
void Kuerzen(Fraction& rF, unsigned nDigits);


class FrPair {
    Fraction aX;
    Fraction aY;
public:
    FrPair()                                          : aX(0,1),aY(0,1)             {}
    FrPair(const Fraction& rBoth)                     : aX(rBoth),aY(rBoth)         {}
    FrPair(const Fraction& rX, const Fraction& rY)    : aX(rX),aY(rY)               {}
    FrPair(long nMul, long nDiv)                      : aX(nMul,nDiv),aY(nMul,nDiv) {}
    FrPair(long xMul, long xDiv, long yMul, long yDiv): aX(xMul,xDiv),aY(yMul,yDiv) {}
    const Fraction& X() const { return aX; }
    const Fraction& Y() const { return aY; }
    Fraction& X()             { return aX; }
    Fraction& Y()             { return aY; }
};

// Fuer die Umrechnung von Masseinheiten
SVX_DLLPUBLIC FrPair GetMapFactor(MapUnit eS, MapUnit eD);
FrPair GetMapFactor(FieldUnit eS, FieldUnit eD);

inline bool IsMetric(MapUnit eU) {
    return (eU==MAP_100TH_MM || eU==MAP_10TH_MM || eU==MAP_MM || eU==MAP_CM);
}

inline bool IsInch(MapUnit eU) {
    return (eU==MAP_1000TH_INCH || eU==MAP_100TH_INCH || eU==MAP_10TH_INCH || eU==MAP_INCH ||
            eU==MAP_POINT       || eU==MAP_TWIP);
}

inline bool IsMetric(FieldUnit eU) {
    return (eU==FUNIT_MM || eU==FUNIT_CM || eU==FUNIT_M || eU==FUNIT_KM || eU==FUNIT_100TH_MM);
}

inline bool IsInch(FieldUnit eU) {
    return (eU==FUNIT_TWIP || eU==FUNIT_POINT || eU==FUNIT_PICA ||
            eU==FUNIT_INCH || eU==FUNIT_FOOT || eU==FUNIT_MILE);
}

class SVX_DLLPUBLIC SdrFormatter {
    Fraction  aScale;
    long      nMul_;
    long      nDiv_;
    short     nKomma_;
    bool      bSrcFU;
    bool      bDstFU;
    bool      bDirty;
    MapUnit   eSrcMU;
    MapUnit   eDstMU;
    FieldUnit eSrcFU;
    FieldUnit eDstFU;
private:
    SVX_DLLPRIVATE void Undirty();
    SVX_DLLPRIVATE void ForceUndirty() const { if (bDirty) ((SdrFormatter*)this)->Undirty(); }
public:
    SdrFormatter(MapUnit eSrc, MapUnit eDst)     { eSrcMU=eSrc; bSrcFU=sal_False; eDstMU=eDst; bDstFU=sal_False; bDirty=sal_True; }
    SdrFormatter(MapUnit eSrc, FieldUnit eDst)   { eSrcMU=eSrc; bSrcFU=sal_False; eDstFU=eDst; bDstFU=sal_True;  bDirty=sal_True; }
    SdrFormatter(FieldUnit eSrc, MapUnit eDst)   { eSrcFU=eSrc; bSrcFU=sal_True;  eDstMU=eDst; bDstFU=sal_False; bDirty=sal_True; }
    SdrFormatter(FieldUnit eSrc, FieldUnit eDst) { eSrcFU=eSrc; bSrcFU=sal_True;  eDstFU=eDst; bDstFU=sal_True;  bDirty=sal_True; }
    void SetSourceUnit(MapUnit eSrc)        { eSrcMU=eSrc; bSrcFU=sal_False; bDirty=sal_True; }
    void SetSourceUnit(FieldUnit eSrc)      { eSrcFU=eSrc; bSrcFU=sal_True;  bDirty=sal_True; }
    void SetDestinationUnit(MapUnit eDst)   { eDstMU=eDst; bDstFU=sal_False; bDirty=sal_True; }
    void SetDestinationUnit(FieldUnit eDst) { eDstFU=eDst; bDstFU=sal_True;  bDirty=sal_True; }
    void TakeStr(long nVal, OUString& rStr) const;
    static void TakeUnitStr(MapUnit eUnit, OUString& rStr);
    static void TakeUnitStr(FieldUnit eUnit, OUString& rStr);
    static OUString GetUnitStr(MapUnit eUnit)   { OUString aStr; TakeUnitStr(eUnit,aStr); return aStr; }
    static OUString GetUnitStr(FieldUnit eUnit) { OUString aStr; TakeUnitStr(eUnit,aStr); return aStr; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDTRANS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
