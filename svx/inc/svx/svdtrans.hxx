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



#ifndef _SVDTRANS_HXX
#define _SVDTRANS_HXX

#include <tools/gen.hxx>
#include <tools/poly.hxx>
#include <tools/fract.hxx>

#ifndef _MAPMOD_HXX //autogen
#include <vcl/mapmod.hxx>
#endif
#include <tools/string.hxx>
#include "svx/svxdllapi.h"

#include <vcl/field.hxx>

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

inline long Round(double a) { return a>0.0 ? (long)(a+0.5) : -(long)((-a)+0.5); }

inline void RotatePoint(basegfx::B2DPoint& rPnt, const basegfx::B2DPoint& rRef, double sn, double cs);
SVX_DLLPUBLIC void RotatePoly(Polygon& rPoly, const Point& rRef, double sn, double cs);
void RotatePoly(PolyPolygon& rPoly, const Point& rRef, double sn, double cs);
void MirrorPoint(Point& rPnt, const Point& rRef1, const Point& rRef2);
inline void ShearPoint(Point& rPnt, const Point& rRef, double tn, bool bVShear = false);
SVX_DLLPUBLIC void ShearPoly(Polygon& rPoly, const Point& rRef, double tn, bool bVShear = false);
void ShearPoly(PolyPolygon& rPoly, const Point& rRef, double tn, bool bVShear = false);

// Die folgenden Methoden behandeln einen Punkt eines XPolygons, wobei die
// benachbarten Kontrollpunkte des eigentlichen Punktes ggf. in pC1/pC2
// uebergeben werden. Ueber rSin/rCos wird gleichzeitig sin(nWink) und cos(nWink)
// zurueckgegeben.
// Der Rueckgabewinkel ist hier ebenfalls in Rad.
double CrookRotateXPoint(basegfx::B2DPoint& rPnt, basegfx::B2DPoint* pC1, basegfx::B2DPoint* pC2, const basegfx::B2DPoint& rCenter, const basegfx::B2DPoint& rRad, double& rSin, double& rCos, bool bVert);
double CrookSlantXPoint(basegfx::B2DPoint& rPnt, basegfx::B2DPoint* pC1, basegfx::B2DPoint* pC2, const basegfx::B2DPoint& rCenter, const basegfx::B2DPoint& rRad, double& rSin, double& rCos, bool bVert);
double CrookStretchXPoint(basegfx::B2DPoint& rPnt, basegfx::B2DPoint* pC1, basegfx::B2DPoint* pC2, const basegfx::B2DPoint& rCenter, const basegfx::B2DPoint& rRad, double& rSin, double& rCos, bool bVert, const basegfx::B2DRange& rRefRect);

void CrookRotatePoly(basegfx::B2DPolygon& rPoly, const basegfx::B2DPoint& rCenter, const basegfx::B2DPoint& rRad, bool bVert);
void CrookSlantPoly(basegfx::B2DPolygon& rPoly, const basegfx::B2DPoint& rCenter, const basegfx::B2DPoint& rRad, bool bVert);
void CrookStretchPoly(basegfx::B2DPolygon& rPoly, const basegfx::B2DPoint& rCenter, const basegfx::B2DPoint& rRad, bool bVert, const basegfx::B2DRange& rRefRange);

void CrookRotatePoly(basegfx::B2DPolyPolygon& rPoly, const basegfx::B2DPoint& rCenter, const basegfx::B2DPoint& rRad, bool bVert);
void CrookSlantPoly(basegfx::B2DPolyPolygon& rPoly, const basegfx::B2DPoint& rCenter, const basegfx::B2DPoint& rRad, bool bVert);
void CrookStretchPoly(basegfx::B2DPolyPolygon& rPoly, const basegfx::B2DPoint& rCenter, const basegfx::B2DPoint& rRad, bool bVert, const basegfx::B2DRange& rRefRange);

/**************************************************************************************************/
/*  Inline                                                                                        */
/**************************************************************************************************/

inline void RotatePoint(Point& rPnt, const Point& rRef, double sn, double cs)
{
    long dx=rPnt.X()-rRef.X();
    long dy=rPnt.Y()-rRef.Y();
    rPnt.X()=Round(rRef.X()+dx*cs+dy*sn);
    rPnt.Y()=Round(rRef.Y()+dy*cs-dx*sn);
}

inline void RotateB2DPointAroundRef(basegfx::B2DPoint& rPnt, const basegfx::B2DPoint& rRef, double sn, double cs)
{
    const basegfx::B2DTuple aDelta(rPnt - rRef);
    rPnt.setX(rRef.getX() + aDelta.getX() * cs + aDelta.getY() * sn);
    rPnt.setY(rRef.getY() + aDelta.getY() * cs - aDelta.getX() * sn);
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

inline double GetCrookAngle(basegfx::B2DPoint& rPnt, const basegfx::B2DPoint& rCenter, const basegfx::B2DPoint& rRad, bool bVertical)
{
    double fAngle(0.0);

    if(bVertical)
    {
        const double dy(rPnt.getY() - rCenter.getY());

        fAngle = dy/rRad.getY();
        rPnt.setY(rCenter.getY());
    }
    else
    {
        const double dx(rCenter.getX() - rPnt.getX());

        fAngle = dx / rRad.getX();
        rPnt.setX(rCenter.getX());
    }

    return fAngle;
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
FrPair GetMapFactor(MapUnit eS, FieldUnit eD);
FrPair GetMapFactor(FieldUnit eS, MapUnit eD);
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
    bool  bSrcFU;
    bool  bDstFU;
    bool  bDirty;
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
    void TakeStr(long nVal, XubString& rStr) const;
    static void TakeUnitStr(MapUnit eUnit, XubString& rStr);
    static void TakeUnitStr(FieldUnit eUnit, XubString& rStr);
    static XubString GetUnitStr(MapUnit eUnit)   { XubString aStr; TakeUnitStr(eUnit,aStr); return aStr; }
    static XubString GetUnitStr(FieldUnit eUnit) { XubString aStr; TakeUnitStr(eUnit,aStr); return aStr; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDTRANS_HXX

