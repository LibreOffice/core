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

#pragma once

#include <rtl/ustring.hxx>
#include <svx/svxdllapi.h>
#include <tools/degree.hxx>
#include <tools/fldunit.hxx>
#include <tools/fract.hxx>
#include <tools/gen.hxx>
#include <tools/helpers.hxx>
#include <tools/mapunit.hxx>
#include <tools/poly.hxx>

// That maximum shear angle
constexpr Degree100 SDRMAXSHEAR(8900);

class XPolygon;
class XPolyPolygon;

inline void MovePoly(tools::Polygon& rPoly, const Size& S)      { rPoly.Move(S.Width(),S.Height()); }
void MoveXPoly(XPolygon& rPoly, const Size& S);

SVXCORE_DLLPUBLIC void ResizeRect(tools::Rectangle& rRect, const Point& rRef, const Fraction& xFact, const Fraction& yFact);
inline void ResizePoint(Point& rPnt, const Point& rRef, const Fraction& xFract, const Fraction& yFract);
void ResizePoly(tools::Polygon& rPoly, const Point& rRef, const Fraction& xFact, const Fraction& yFact);
void ResizeXPoly(XPolygon& rPoly, const Point& rRef, const Fraction& xFact, const Fraction& yFact);

inline void RotatePoint(Point& rPnt, const Point& rRef, double sn, double cs);
SVXCORE_DLLPUBLIC void RotatePoly(tools::Polygon& rPoly, const Point& rRef, double sn, double cs);
void RotateXPoly(XPolygon& rPoly, const Point& rRef, double sn, double cs);
void RotateXPoly(XPolyPolygon& rPoly, const Point& rRef, double sn, double cs);

void MirrorPoint(Point& rPnt, const Point& rRef1, const Point& rRef2);
void MirrorXPoly(XPolygon& rPoly, const Point& rRef1, const Point& rRef2);

inline void ShearPoint(Point& rPnt, const Point& rRef, double tn, bool bVShear = false);
SVXCORE_DLLPUBLIC void ShearPoly(tools::Polygon& rPoly, const Point& rRef, double tn);
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
                          const tools::Rectangle& rRefRect);

void CrookRotatePoly(XPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert);
void CrookSlantPoly(XPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert);
void CrookStretchPoly(XPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert, const tools::Rectangle& rRefRect);

void CrookRotatePoly(XPolyPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert);
void CrookSlantPoly(XPolyPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert);
void CrookStretchPoly(XPolyPolygon& rPoly, const Point& rCenter, const Point& rRad, bool bVert, const tools::Rectangle& rRefRect);

/**************************************************************************************************/
/*  Inline                                                                                        */
/**************************************************************************************************/

inline void ResizePoint(Point& rPnt, const Point& rRef, const Fraction& xFract, const Fraction& yFract)
{
    double nxFract = xFract.IsValid() ? static_cast<double>(xFract) : 1.0;
    double nyFract = yFract.IsValid() ? static_cast<double>(yFract) : 1.0;
    rPnt.setX(rRef.X() + basegfx::fround<tools::Long>((rPnt.X() - rRef.X()) * nxFract));
    rPnt.setY(rRef.Y() + basegfx::fround<tools::Long>((rPnt.Y() - rRef.Y()) * nyFract));
}

inline void RotatePoint(Point& rPnt, const Point& rRef, double sn, double cs)
{
    tools::Long dx=rPnt.X()-rRef.X();
    tools::Long dy=rPnt.Y()-rRef.Y();
    rPnt.setX(basegfx::fround<tools::Long>(rRef.X() + dx * cs + dy * sn));
    rPnt.setY(basegfx::fround<tools::Long>(rRef.Y() + dy * cs - dx * sn));
}

inline void ShearPoint(Point& rPnt, const Point& rRef, double tn, bool bVShear)
{
    if (!bVShear) { // Horizontal
        if (rPnt.Y()!=rRef.Y()) { // else not needed
            rPnt.AdjustX(basegfx::fround<tools::Long>((rRef.Y() - rPnt.Y()) * tn));
        }
    } else { // or else vertical
        if (rPnt.X()!=rRef.X()) { // else not needed
            rPnt.AdjustY(basegfx::fround<tools::Long>((rRef.X() - rPnt.X()) * tn));
        }
    }
}

inline double GetCrookAngle(Point& rPnt, const Point& rCenter, const Point& rRad, bool bVertical)
{
    double nAngle;
    if (bVertical) {
        tools::Long dy=rPnt.Y()-rCenter.Y();
        nAngle=static_cast<double>(dy)/static_cast<double>(rRad.Y());
        rPnt.setY(rCenter.Y());
    } else {
        tools::Long dx=rCenter.X()-rPnt.X();
        nAngle=static_cast<double>(dx)/static_cast<double>(rRad.X());
        rPnt.setX(rCenter.X());
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
SVXCORE_DLLPUBLIC Degree100 GetAngle(const Point& rPnt);

SVXCORE_DLLPUBLIC Degree100 NormAngle18000(Degree100 a); /// Normalize angle to -180.00..179.99

SVXCORE_DLLPUBLIC Degree100 NormAngle36000(Degree100 a); /// Normalize angle to 0.00..359.99

sal_uInt16 GetAngleSector(Degree100 nAngle); /// Determine sector within the cartesian coordinate system

/**
 * Calculates the length of (0,0) via a^2 + b^2 = c^2
 * In order to avoid overflows, we ignore some decimal places.
 */
tools::Long GetLen(const Point& rPnt);

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
 * transformed in the meantime (e.g. mirror or resize with negative factor).
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
    Degree100 m_nRotationAngle;
    Degree100 m_nShearAngle;
    double   mfTanShearAngle;      // tan(nShearAngle)
    double   mfSinRotationAngle;   // sin(nRotationAngle)
    double   mfCosRotationAngle;   // cos(nRotationAngle)

    GeoStat(): m_nRotationAngle(0),m_nShearAngle(0),mfTanShearAngle(0.0),mfSinRotationAngle(0.0),mfCosRotationAngle(1.0) {}
    void RecalcSinCos();
    void RecalcTan();
};

tools::Polygon Rect2Poly(const tools::Rectangle& rRect, const GeoStat& rGeo);

namespace svx
{
tools::Rectangle polygonToRectangle(const tools::Polygon& rPolygon, GeoStat& rGeo);
}

void OrthoDistance8(const Point& rPt0, Point& rPt, bool bBigOrtho);
void OrthoDistance4(const Point& rPt0, Point& rPt, bool bBigOrtho);

// Multiplication and subsequent division
// Calculation and intermediate values are in BigInt
SVXCORE_DLLPUBLIC tools::Long BigMulDiv(tools::Long nVal, tools::Long nMul, tools::Long nDiv);

class FrPair {
    Fraction m_aX;
    Fraction m_aY;
public:
    FrPair(const Fraction& rBoth)                     : m_aX(rBoth),m_aY(rBoth)         {}
    FrPair(const Fraction& rX, const Fraction& rY)    : m_aX(rX),m_aY(rY)               {}
    FrPair(tools::Long nMul, tools::Long nDiv)                      : m_aX(nMul,nDiv),m_aY(nMul,nDiv) {}
    FrPair(tools::Long xMul, tools::Long xDiv, tools::Long yMul, tools::Long yDiv): m_aX(xMul,xDiv),m_aY(yMul,yDiv) {}
    const Fraction& X() const { return m_aX; }
    const Fraction& Y() const { return m_aY; }
    Fraction& X()             { return m_aX; }
    Fraction& Y()             { return m_aY; }
};

// To convert units of measurement
SVXCORE_DLLPUBLIC FrPair GetMapFactor(MapUnit eS, MapUnit eD);
FrPair GetMapFactor(FieldUnit eS, FieldUnit eD);

inline bool IsMetric(MapUnit eU) {
    return (eU==MapUnit::Map100thMM || eU==MapUnit::Map10thMM || eU==MapUnit::MapMM || eU==MapUnit::MapCM);
}

inline bool IsInch(MapUnit eU) {
    return (eU==MapUnit::Map1000thInch || eU==MapUnit::Map100thInch || eU==MapUnit::Map10thInch || eU==MapUnit::MapInch ||
            eU==MapUnit::MapPoint       || eU==MapUnit::MapTwip);
}

inline bool IsMetric(FieldUnit eU) {
    return (eU == FieldUnit::MM || eU == FieldUnit::CM || eU == FieldUnit::M
            || eU == FieldUnit::KM || eU == FieldUnit::MM_100TH);
}

inline bool IsInch(FieldUnit eU) {
    return (eU == FieldUnit::TWIP || eU == FieldUnit::POINT
            || eU == FieldUnit::PICA || eU == FieldUnit::INCH
            || eU == FieldUnit::FOOT || eU == FieldUnit::MILE);
}

class SVXCORE_DLLPUBLIC SdrFormatter {
    tools::Long      m_nMul;
    tools::Long      m_nDiv;
    short     m_nComma;
    bool      m_bDirty;
    MapUnit   m_eSrcMU;
    MapUnit   m_eDstMU;
private:
    SVX_DLLPRIVATE void Undirty();
public:
    SdrFormatter(MapUnit eSrc, MapUnit eDst)
        : m_nMul(0)
        , m_nDiv(0)
        , m_nComma(0)
        , m_bDirty(true)
        , m_eSrcMU(eSrc)
        , m_eDstMU(eDst)
    {
    }
    OUString GetStr(tools::Long nVal) const;
    static OUString GetUnitStr(MapUnit eUnit);
    static OUString GetUnitStr(FieldUnit eUnit);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
