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

#ifndef INCLUDED_SVX_SVDTRANS_HXX
#define INCLUDED_SVX_SVDTRANS_HXX

#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <svx/svxdllapi.h>
#include <tools/fract.hxx>
#include <tools/gen.hxx>
#include <tools/poly.hxx>
#include <vcl/field.hxx>
#include <vcl/mapmod.hxx>

// The DrawingEngine's angles are specified in 1/100th degrees
// We need to convert these angles to radians, in order to be able
// to process them with trigonometric functions.
// This is done, using the constant nPi180.
//
// Example usage:
// nAngle ... is an angle in 1/100 Deg
//
// Which is converted, by this:
//   double nSin=sin(nAngle*nPi180);
//
// To convert it back, we use division.
const double nPi=3.14159265358979323846;
const double nPi180=0.000174532925199432957692222; // If we have too few digits, we get tan(4500*nPi180)!=1.0

// That maximum shear angle
#define SDRMAXSHEAR 8900

class XPolygon;
class XPolyPolygon;

inline long Round(double a) { return a>0.0 ? (long)(a+0.5) : -(long)((-a)+0.5); }

inline void MoveRect(Rectangle& rRect, const Size& S)    { rRect.Move(S.Width(),S.Height()); }
inline void MovePoint(Point& rPnt, const Size& S)        { rPnt.X()+=S.Width(); rPnt.Y()+=S.Height(); }
inline void MovePoly(tools::Polygon& rPoly, const Size& S)      { rPoly.Move(S.Width(),S.Height()); }
void MoveXPoly(XPolygon& rPoly, const Size& S);

SVX_DLLPUBLIC void ResizeRect(Rectangle& rRect, const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bNoJustify = false);
inline void ResizePoint(Point& rPnt, const Point& rRef, Fraction xFact, Fraction yFact);
void ResizePoly(tools::Polygon& rPoly, const Point& rRef, const Fraction& xFact, const Fraction& yFact);
void ResizeXPoly(XPolygon& rPoly, const Point& rRef, const Fraction& xFact, const Fraction& yFact);

inline void RotatePoint(Point& rPnt, const Point& rRef, double sn, double cs);
SVX_DLLPUBLIC void RotatePoly(tools::Polygon& rPoly, const Point& rRef, double sn, double cs);
void RotateXPoly(XPolygon& rPoly, const Point& rRef, double sn, double cs);
void RotateXPoly(XPolyPolygon& rPoly, const Point& rRef, double sn, double cs);

void MirrorPoint(Point& rPnt, const Point& rRef1, const Point& rRef2);
void MirrorXPoly(XPolygon& rPoly, const Point& rRef1, const Point& rRef2);

inline void ShearPoint(Point& rPnt, const Point& rRef, double tn, bool bVShear = false);
SVX_DLLPUBLIC void ShearPoly(tools::Polygon& rPoly, const Point& rRef, double tn, bool bVShear = false);
void ShearXPoly(XPolygon& rPoly, const Point& rRef, double tn, bool bVShear = false);

/**
 * rPnt.X/rPnt.Y is set to rCenter.X or rCenter.Y!
 * We then only need to rotate rPnt by rCenter.
 *
 * @return the returned angle is in rad
 */
inline double GetCrookAngle(Point& rPnt, const Point& rCenter, const Point& rRad, bool bVertical);

/**
 * The following methods accept a point of an XPolygon, whereas the neighbouring
 * control points of the actual point are passed in pC1/pC2.
 * Via rSin/rCos, sin(nAngle) and cos(nAngle) are returned.
 *
 * @return the returned angle is in rad
 */
double CrookRotateXPoint(Point& rPnt, Point* pC1, Point* pC2, const Point& rCenter,
                         const Point& rRad, double& rSin, double& rCos, bool bVert);
double CrookSlantXPoint(Point& rPnt, Point* pC1, Point* pC2, const Point& rCenter,
                        const Point& rRad, double& rSin, double& rCos, bool bVert);
double CrookStretchXPoint(Point& rPnt, Point* pC1, Point* pC2, const Point& rCenter,
                          const Point& rRad, double& rSin, double& rCos, bool bVert,
                          const Rectangle& rRefRect);

void CrookRotatePoly(XPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert);
void CrookSlantPoly(XPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert);
void CrookStretchPoly(XPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert, const Rectangle& rRefRect);

void CrookRotatePoly(XPolyPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert);
void CrookSlantPoly(XPolyPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert);
void CrookStretchPoly(XPolyPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert, const Rectangle& rRefRect);

/**************************************************************************************************/
/*  Inline                                                                                        */
/**************************************************************************************************/

inline void ResizePoint(Point& rPnt, const Point& rRef, Fraction xFact, Fraction yFact)
{
    if (!xFact.IsValid()) {
        SAL_WARN( "svx.svdraw", "invalid fraction xFact, using Fraction(1,1)" );
        xFact = Fraction(1,1);
    }
    if (!yFact.IsValid()) {
        SAL_WARN( "svx.svdraw", "invalid fraction yFact, using Fraction(1,1)" );
        yFact = Fraction(1,1);
    }
    rPnt.X() = rRef.X() + Round( (rPnt.X() - rRef.X()) * double(xFact) );
    rPnt.Y() = rRef.Y() + Round( (rPnt.Y() - rRef.Y()) * double(yFact) );
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
        if (rPnt.Y()!=rRef.Y()) { // else not needed
            rPnt.X()-=Round((rPnt.Y()-rRef.Y())*tn);
        }
    } else { // or else vertical
        if (rPnt.X()!=rRef.X()) { // else not needed
            rPnt.Y()-=Round((rPnt.X()-rRef.X())*tn);
        }
    }
}

inline double GetCrookAngle(Point& rPnt, const Point& rCenter, const Point& rRad, bool bVertical)
{
    double nAngle;
    if (bVertical) {
        long dy=rPnt.Y()-rCenter.Y();
        nAngle=(double)dy/(double)rRad.Y();
        rPnt.Y()=rCenter.Y();
    } else {
        long dx=rCenter.X()-rPnt.X();
        nAngle=(double)dx/(double)rRad.X();
        rPnt.X()=rCenter.X();
    }
    return nAngle;
}

/**************************************************************************************************/
/**************************************************************************************************/

/**
 * The Y axis points down!
 * The function negates the Y axis, when calculating the angle, such
 * that GetAngle(Point(0,-1))=90 deg.
 * GetAngle(Point(0,0)) returns 0.
 *
 * @return the returned value is in the range of -180.00..179.99 deg
 * and is in 1/100 deg units
 */
SVX_DLLPUBLIC long GetAngle(const Point& rPnt);

long NormAngle180(long a); /// Normalize angle to -180.00..179.99

SVX_DLLPUBLIC long NormAngle360(long a); /// Normalize angle to 0.00..359.99

sal_uInt16 GetAngleSector(long nAngle); /// Determine sector within the cartesian coordinate system

/**
 * Calculates the length of (0,0) via a^2 + b^2 = c^2
 * In order to avoid overflows, we ignore some decimal places.
 */
long GetLen(const Point& rPnt);

/**
 * The transformation of a rectangle into a polygon, by
 * using angle parameters from GeoStat.                          ------------
 * The point of reference is always the Point 0, meaning        /1        2/
 * the upper left corner of the initial rectangle.             /          /
 * When calculating the polygon, the order is first           /          /
 * shear and then the rotation.                              /          /
 *                                                          /          / \
 *                                                         /          /   |
 * A) Initial rectangle aRect  B) After applying Shear     /0        3/ Rot|
 * +------------------+       --------------------        ------------------
 * |0                1|        \0                1\       C) After applying Rotate
 * |                  |         \                  \
 * |                  |       |  \                  \
 * |3                2|       |   \3                2\
 * +------------------+       |    --------------------
 *                            |Shr
 *
 * When converting the polygon back into a rect, the order is necessarily the
 * other way around:
 *  - Calculating the rotation angle: angle of the line 0-1 in figure C) to the horizontal
 *  - Turning the sheared rect back (we get figure B)
 *  - Determining the width of the rect = length of the line 0-1 in figure B)
 *  - Determining the height of the rect = vertical distance between the points 0 and 3
 *    of figure B)
 *  - Determining the shear angle from the line 0-3 to the perpendicular line.
 *
 * We need to keep in mind that the polygon can be mirrored when it was
 * transformed in the mean time (e.g. mirror or resize with negative factor).
 * In that case, we first need to normalize, by swapping points (0 with 3 and 1
 * with 2), so that it has the right orientation.
 *
 * Note: a positive shear angle means a shear with a positive visible curvature
 * on the screen. Mathematically, that would be a negative curvature, as the
 * Y axis runs from top to bottom on the screen.
 * Rotation angle: positive means a visible left rotation.
 */

class GeoStat { // Geometric state for a rect
public:
    long     nRotationAngle;
    long     nShearAngle;
    double   nTan;      // tan(nShearAngle)
    double   nSin;      // sin(nRotationAngle)
    double   nCos;      // cos(nRotationAngle)
    bool     bMirrored; // Horizontally mirrored? (ni)
public:
    GeoStat(): nRotationAngle(0),nShearAngle(0),nTan(0.0),nSin(0.0),nCos(1.0),bMirrored(false) {}
    void RecalcSinCos();
    void RecalcTan();
};

tools::Polygon Rect2Poly(const Rectangle& rRect, const GeoStat& rGeo);
void Poly2Rect(const tools::Polygon& rPol, Rectangle& rRect, GeoStat& rGeo);

SVX_DLLPUBLIC void OrthoDistance8(const Point& rPt0, Point& rPt, bool bBigOrtho);
SVX_DLLPUBLIC void OrthoDistance4(const Point& rPt0, Point& rPt, bool bBigOrtho);

// Multiplication and subsequent division
// Calculation and intermediate values are in BigInt
SVX_DLLPUBLIC long BigMulDiv(long nVal, long nMul, long nDiv);

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

// To convert units of measurement
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
    SVX_DLLPRIVATE void ForceUndirty() const { if (bDirty) const_cast<SdrFormatter*>(this)->Undirty(); }
public:
    SdrFormatter(MapUnit eSrc, MapUnit eDst)
        : nMul_(0)
        , nDiv_(0)
        , nKomma_(0)
        , bSrcFU(false)
        , bDstFU(false)
        , bDirty(true)
        , eSrcMU(eSrc)
        , eDstMU(eDst)
        , eSrcFU(FUNIT_NONE)
        , eDstFU(FUNIT_NONE)
    {
    }
    SdrFormatter(MapUnit eSrc, FieldUnit eDst)   { eSrcMU=eSrc; bSrcFU=false; eDstFU=eDst; bDstFU=true;  bDirty=true; }
    SdrFormatter(FieldUnit eSrc, MapUnit eDst)   { eSrcFU=eSrc; bSrcFU=true;  eDstMU=eDst; bDstFU=false; bDirty=true; }
    SdrFormatter(FieldUnit eSrc, FieldUnit eDst) { eSrcFU=eSrc; bSrcFU=true;  eDstFU=eDst; bDstFU=true;  bDirty=true; }
    void TakeStr(long nVal, OUString& rStr) const;
    static void TakeUnitStr(MapUnit eUnit, OUString& rStr);
    static void TakeUnitStr(FieldUnit eUnit, OUString& rStr);
    static OUString GetUnitStr(FieldUnit eUnit) { OUString aStr; TakeUnitStr(eUnit,aStr); return aStr; }
};



#endif // INCLUDED_SVX_SVDTRANS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
