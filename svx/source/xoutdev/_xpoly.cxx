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

#include <sal/config.h>

#include <algorithm>

#include <osl/endian.h>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/poly.hxx>
#include <tools/helpers.hxx>
#include <tools/gen.hxx>

#include <svx/xpoly.hxx>
#include <xpolyimp.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/numeric/ftools.hxx>



XPolygon::XPolygon()
    : pImpXPolygon( ImpXPolygon() )
{
}

XPolygon::XPolygon( sal_uInt16 nSize )
    : pImpXPolygon( ImpXPolygon( nSize ) )
{
}

XPolygon::XPolygon( const XPolygon& rXPoly )
    : pImpXPolygon(rXPoly.pImpXPolygon)
{
}

XPolygon::XPolygon( XPolygon&& rXPoly )
    : pImpXPolygon(std::move(rXPoly.pImpXPolygon))
{
}

/// create a XPolygon out of a standard polygon
XPolygon::XPolygon( const tools::Polygon& rPoly )
    : pImpXPolygon( rPoly.GetSize() )
{
    sal_uInt16 nSize = rPoly.GetSize();
    pImpXPolygon->mvPointsAndFlags.resize(nSize);

    for( sal_uInt16 i = 0; i < nSize;  i++ )
    {
        pImpXPolygon->mvPointsAndFlags[i].first = rPoly[i];
        pImpXPolygon->mvPointsAndFlags[i].second = rPoly.GetFlags( i );
    }
}

/// create a rectangle (also with rounded corners) as a Bézier polygon
XPolygon::XPolygon(const tools::Rectangle& rRect, long nRx, long nRy)
    : pImpXPolygon( 17 )
{
    long nWh = (rRect.GetWidth()  - 1) / 2;
    long nHh = (rRect.GetHeight() - 1) / 2;

    if ( nRx > nWh )    nRx = nWh;
    if ( nRy > nHh )    nRy = nHh;

    // negate Rx => circle clockwise
    nRx = -nRx;

    // factor for control points of the Bézier curve: 8/3 * (sin(45g) - 0.5)
    long    nXHdl = static_cast<long>(0.552284749 * nRx);
    long    nYHdl = static_cast<long>(0.552284749 * nRy);
    sal_uInt16  nPos = 0;

    pImpXPolygon->mvPointsAndFlags.resize(17);

    if ( nRx && nRy )
    {
        Point aCenter;

        for (sal_uInt16 nQuad = 0; nQuad < 4; nQuad++)
        {
            switch ( nQuad )
            {
                case 0: aCenter = rRect.TopLeft();
                        aCenter.X() -= nRx;
                        aCenter.Y() += nRy;
                        break;
                case 1: aCenter = rRect.TopRight();
                        aCenter.X() += nRx;
                        aCenter.Y() += nRy;
                        break;
                case 2: aCenter = rRect.BottomRight();
                        aCenter.X() += nRx;
                        aCenter.Y() -= nRy;
                        break;
                case 3: aCenter = rRect.BottomLeft();
                        aCenter.X() -= nRx;
                        aCenter.Y() -= nRy;
                        break;
            }
            GenBezArc(aCenter, nRx, nRy, nXHdl, nYHdl, 0, 900, nQuad, nPos);
            pImpXPolygon->mvPointsAndFlags[nPos  ].second = PolyFlags::Smooth;
            pImpXPolygon->mvPointsAndFlags[nPos+3].second = PolyFlags::Smooth;
            nPos += 4;
        }
    }
    else
    {
        pImpXPolygon->mvPointsAndFlags[nPos++].first = rRect.TopLeft();
        pImpXPolygon->mvPointsAndFlags[nPos++].first = rRect.TopRight();
        pImpXPolygon->mvPointsAndFlags[nPos++].first = rRect.BottomRight();
        pImpXPolygon->mvPointsAndFlags[nPos++].first = rRect.BottomLeft();
    }
    pImpXPolygon->mvPointsAndFlags[nPos].first = pImpXPolygon->mvPointsAndFlags[0].first;
    pImpXPolygon->mvPointsAndFlags.resize(nPos + 1);
}

/// create a ellipse (curve) as Bézier polygon
XPolygon::XPolygon(const Point& rCenter, long nRx, long nRy,
                   sal_uInt16 nStartAngle, sal_uInt16 nEndAngle, bool bClose)
    : pImpXPolygon( 17 )
{
    nStartAngle %= 3600;
    if ( nEndAngle > 3600 ) nEndAngle %= 3600;
    bool bFull = (nStartAngle == 0 && nEndAngle == 3600);

    // factor for control points of the Bézier curve: 8/3 * (sin(45g) - 0.5)
    long    nXHdl = static_cast<long>(0.552284749 * nRx);
    long    nYHdl = static_cast<long>(0.552284749 * nRy);
    sal_uInt16  nPos = 0;
    bool    bLoopEnd = false;

    pImpXPolygon->mvPointsAndFlags.resize(17);
    do
    {
        sal_uInt16 nA1, nA2;
        sal_uInt16 nQuad = nStartAngle / 900;
        if ( nQuad == 4 ) nQuad = 0;
        bLoopEnd = CheckAngles(nStartAngle, nEndAngle, nA1, nA2);
        GenBezArc(rCenter, nRx, nRy, nXHdl, nYHdl, nA1, nA2, nQuad, nPos);
        nPos += 3;
        if ( !bLoopEnd )
            pImpXPolygon->mvPointsAndFlags[nPos].second = PolyFlags::Smooth;

    } while ( !bLoopEnd );

    // if not a full circle than connect edges with center point if necessary
    if ( !bFull && bClose )
        pImpXPolygon->mvPointsAndFlags[++nPos].first = rCenter;

    if ( bFull )
    {
        pImpXPolygon->mvPointsAndFlags[0   ].second = PolyFlags::Smooth;
        pImpXPolygon->mvPointsAndFlags[nPos].second = PolyFlags::Smooth;
    }
    pImpXPolygon->mvPointsAndFlags.resize( nPos + 1 );
}

XPolygon::~XPolygon()
{
}

void XPolygon::SetPointCount( sal_uInt16 nPoints )
{
    pImpXPolygon->mvPointsAndFlags.resize( nPoints );
}

sal_uInt16 XPolygon::GetPointCount() const
{
    return pImpXPolygon->mvPointsAndFlags.size();
}

void XPolygon::Insert( sal_uInt16 nPos, const Point& rPt, PolyFlags eFlags )
{
    if (nPos>pImpXPolygon->mvPointsAndFlags.size()) nPos=pImpXPolygon->mvPointsAndFlags.size();
    pImpXPolygon->mvPointsAndFlags.emplace(pImpXPolygon->mvPointsAndFlags.begin() + nPos, rPt, eFlags);
}

void XPolygon::Insert( sal_uInt16 nPos, const XPolygon& rXPoly )
{
    if (nPos>pImpXPolygon->mvPointsAndFlags.size()) nPos=pImpXPolygon->mvPointsAndFlags.size();

    std::copy( rXPoly.pImpXPolygon->mvPointsAndFlags.begin(),
               rXPoly.pImpXPolygon->mvPointsAndFlags.end(),
               pImpXPolygon->mvPointsAndFlags.begin() + nPos );
}

void XPolygon::Remove( sal_uInt16 nPos, sal_uInt16 nCount )
{
    pImpXPolygon->mvPointsAndFlags.erase( pImpXPolygon->mvPointsAndFlags.begin() + nPos,
                                          pImpXPolygon->mvPointsAndFlags.begin() + nPos + nCount );
}

void XPolygon::Move( long nHorzMove, long nVertMove )
{
    if ( !nHorzMove && !nVertMove )
        return;

    // move points
    sal_uInt16 nCount = pImpXPolygon->mvPointsAndFlags.size();
    for ( sal_uInt16 i = 0; i < nCount; i++ )
    {
        Point* pPt = &(pImpXPolygon->mvPointsAndFlags[i].first);
        pPt->X() += nHorzMove;
        pPt->Y() += nVertMove;
    }
}

tools::Rectangle XPolygon::GetBoundRect() const
{
    tools::Rectangle aRetval;

    if(pImpXPolygon->mvPointsAndFlags.size())
    {
        // #i37709#
        // For historical reasons the control points are not part of the
        // BoundRect. This makes it necessary to subdivide the polygon to
        // get a relatively correct BoundRect. Numerically, this is not
        // correct and never was.

        const basegfx::B2DRange aPolygonRange(basegfx::utils::getRange(getB2DPolygon()));
        aRetval = tools::Rectangle(
            FRound(aPolygonRange.getMinX()), FRound(aPolygonRange.getMinY()),
            FRound(aPolygonRange.getMaxX()), FRound(aPolygonRange.getMaxY()));
    }

    return aRetval;
}

const Point& XPolygon::operator[]( sal_uInt16 nPos ) const
{
    return pImpXPolygon->mvPointsAndFlags[nPos].first;
}

Point& XPolygon::operator[]( sal_uInt16 nPos )
{
    return pImpXPolygon->mvPointsAndFlags[nPos].first;
}

XPolygon& XPolygon::operator=( const XPolygon& rXPoly )
{
    pImpXPolygon = rXPoly.pImpXPolygon;
    return *this;
}

XPolygon& XPolygon::operator=( XPolygon&& rXPoly )
{
    pImpXPolygon = std::move(rXPoly.pImpXPolygon);
    return *this;
}

bool XPolygon::operator==( const XPolygon& rXPoly ) const
{
    return rXPoly.pImpXPolygon == pImpXPolygon;
}

/// get the flags for the point at the given position
PolyFlags XPolygon::GetFlags( sal_uInt16 nPos ) const
{
    return pImpXPolygon->mvPointsAndFlags[nPos].second;
}

/// set the flags for the point at the given position
void XPolygon::SetFlags( sal_uInt16 nPos, PolyFlags eFlags )
{
    pImpXPolygon->mvPointsAndFlags[nPos].second = eFlags;
}

/// short path to read the CONTROL flag directly (TODO: better explain what the sense behind this flag is!)
bool XPolygon::IsControl(sal_uInt16 nPos) const
{
    return pImpXPolygon->mvPointsAndFlags[nPos].second == PolyFlags::Control;
}

/// short path to read the SMOOTH and SYMMTR flag directly (TODO: better explain what the sense behind these flags is!)
bool XPolygon::IsSmooth(sal_uInt16 nPos) const
{
    PolyFlags eFlag = pImpXPolygon->mvPointsAndFlags[nPos].second;
    return ( eFlag == PolyFlags::Smooth || eFlag == PolyFlags::Symmetric );
}

/** calculate the euclidean distance between two points
 *
 * @param nP1 The first point
 * @param nP2 The second point
 */
double XPolygon::CalcDistance(sal_uInt16 nP1, sal_uInt16 nP2)
{
    const Point& rP1 = pImpXPolygon->mvPointsAndFlags[nP1].first;
    const Point& rP2 = pImpXPolygon->mvPointsAndFlags[nP2].first;
    double fDx = rP2.X() - rP1.X();
    double fDy = rP2.Y() - rP1.Y();
    return sqrt(fDx * fDx + fDy * fDy);
}

void XPolygon::SubdivideBezier(sal_uInt16 nPos, bool bCalcFirst, double fT)
{
    auto &  rPoints = pImpXPolygon->mvPointsAndFlags;
    double  fT2 = fT * fT;
    double  fT3 = fT * fT2;
    double  fU = 1.0 - fT;
    double  fU2 = fU * fU;
    double  fU3 = fU * fU2;
    sal_uInt16  nIdx = nPos;
    short   nPosInc, nIdxInc;

    if ( bCalcFirst )
    {
        nPos += 3;
        nPosInc = -1;
        nIdxInc = 0;
    }
    else
    {
        nPosInc = 1;
        nIdxInc = 1;
    }
    rPoints[nPos].first.X() = static_cast<long>(fU3 *       rPoints[nIdx  ].first.X() +
                                fT  * fU2 * rPoints[nIdx+1].first.X() * 3 +
                                fT2 * fU  * rPoints[nIdx+2].first.X() * 3 +
                                fT3 *       rPoints[nIdx+3].first.X());
    rPoints[nPos].first.Y() = static_cast<long>(fU3 *       rPoints[nIdx  ].first.Y() +
                                fT  * fU2 * rPoints[nIdx+1].first.Y() * 3 +
                                fT2 * fU  * rPoints[nIdx+2].first.Y() * 3 +
                                fT3 *       rPoints[nIdx+3].first.Y());
    nPos = nPos + nPosInc;
    nIdx = nIdx + nIdxInc;
    rPoints[nPos].first.X() = static_cast<long>(fU2 *       rPoints[nIdx  ].first.X() +
                                fT  * fU *  rPoints[nIdx+1].first.X() * 2 +
                                fT2 *       rPoints[nIdx+2].first.X());
    rPoints[nPos].first.Y() = static_cast<long>(fU2 *       rPoints[nIdx  ].first.Y() +
                                fT  * fU *  rPoints[nIdx+1].first.Y() * 2 +
                                fT2 *       rPoints[nIdx+2].first.Y());
    nPos = nPos + nPosInc;
    nIdx = nIdx + nIdxInc;
    rPoints[nPos].first.X() = static_cast<long>(fU * rPoints[nIdx  ].first.X() +
                                fT * rPoints[nIdx+1].first.X());
    rPoints[nPos].first.Y() = static_cast<long>(fU * rPoints[nIdx  ].first.Y() +
                                fT * rPoints[nIdx+1].first.Y());
}

/// Generate a Bézier arc
void XPolygon::GenBezArc(const Point& rCenter, long nRx, long nRy,
                         long nXHdl, long nYHdl, sal_uInt16 nStart, sal_uInt16 nEnd,
                         sal_uInt16 nQuad, sal_uInt16 nFirst)
{
    auto& rPoints = pImpXPolygon->mvPointsAndFlags;
    rPoints[nFirst  ].first = rCenter;
    rPoints[nFirst+3].first = rCenter;

    if ( nQuad == 1 || nQuad == 2 )
    {
        nRx   = -nRx; nXHdl = -nXHdl;
    }
    if ( nQuad == 0 || nQuad == 1 )
    {
        nRy   = -nRy; nYHdl = -nYHdl;
    }

    if ( nQuad == 0 || nQuad == 2 )
    {
        rPoints[nFirst].first.X() += nRx; rPoints[nFirst+3].first.Y() += nRy;
    }
    else
    {
        rPoints[nFirst].first.Y() += nRy; rPoints[nFirst+3].first.X() += nRx;
    }
    rPoints[nFirst+1] = rPoints[nFirst];
    rPoints[nFirst+2] = rPoints[nFirst+3];

    if ( nQuad == 0 || nQuad == 2 )
    {
        rPoints[nFirst+1].first.Y() += nYHdl; rPoints[nFirst+2].first.X() += nXHdl;
    }
    else
    {
        rPoints[nFirst+1].first.X() += nXHdl; rPoints[nFirst+2].first.Y() += nYHdl;
    }
    if ( nStart > 0 )
        SubdivideBezier(nFirst, false, static_cast<double>(nStart) / 900);
    if ( nEnd < 900 )
        SubdivideBezier(nFirst, true, static_cast<double>(nEnd-nStart) / (900-nStart));
    SetFlags(nFirst+1, PolyFlags::Control);
    SetFlags(nFirst+2, PolyFlags::Control);
}

bool XPolygon::CheckAngles(sal_uInt16& nStart, sal_uInt16 nEnd, sal_uInt16& nA1, sal_uInt16& nA2)
{
    if ( nStart == 3600 ) nStart = 0;
    if ( nEnd == 0 ) nEnd = 3600;
    sal_uInt16 nStPrev = nStart;
    sal_uInt16 nMax = (nStart / 900 + 1) * 900;
    sal_uInt16 nMin = nMax - 900;

    if ( nEnd >= nMax || nEnd <= nStart )   nA2 = 900;
    else                                    nA2 = nEnd - nMin;
    nA1 = nStart - nMin;
    nStart = nMax;

    // returns true when the last segment was calculated
    return (nStPrev < nEnd && nStart >= nEnd);
}

/** Calculate a smooth transition to connect two Bézier curves
 *
 * This is done by projecting the corresponding point onto a line between
 * two other points.
 *
 * @param nCenter The point at the end or beginning of the curve.
 *                If nCenter is at the end of the polygon the point is moved
 *                to the opposite side.
 * @param nDrag The moved point that specifies the relocation.
 * @param nPnt The point to modify.
 */
void XPolygon::CalcSmoothJoin(sal_uInt16 nCenter, sal_uInt16 nDrag, sal_uInt16 nPnt)
{
    // If nPoint is no control point, i.e. cannot be moved, than
    // move nDrag instead on the line between nCenter and nPnt
    if ( !IsControl(nPnt) )
    {
        sal_uInt16 nTmp = nDrag;
        nDrag = nPnt;
        nPnt = nTmp;
    }
    auto&   rPoints = pImpXPolygon->mvPointsAndFlags;
    Point   aDiff   = rPoints[nDrag].first - rPoints[nCenter].first;
    double  fDiv    = CalcDistance(nCenter, nDrag);

    if ( fDiv )
    {
        double fRatio = CalcDistance(nCenter, nPnt) / fDiv;
        // keep the length if SMOOTH
        if ( GetFlags(nCenter) == PolyFlags::Smooth || !IsControl(nDrag) )
        {
            aDiff.X() = static_cast<long>(fRatio * aDiff.X());
            aDiff.Y() = static_cast<long>(fRatio * aDiff.Y());
        }
        rPoints[nPnt].first = rPoints[nCenter].first - aDiff;
    }
}

/** Calculate tangent between two Bézier curves
 *
 * @param nCenter start or end point of the curves
 * @param nPrev previous reference point
 * @param nNext next reference point
 */
void XPolygon::CalcTangent(sal_uInt16 nCenter, sal_uInt16 nPrev, sal_uInt16 nNext)
{
    double fAbsLen = CalcDistance(nNext, nPrev);

    if ( !fAbsLen )
        return;

    const Point& rCenter = pImpXPolygon->mvPointsAndFlags[nCenter].first;
    Point&  rNext = pImpXPolygon->mvPointsAndFlags[nNext].first;
    Point&  rPrev = pImpXPolygon->mvPointsAndFlags[nPrev].first;
    Point   aDiff = rNext - rPrev;
    double  fNextLen = CalcDistance(nCenter, nNext) / fAbsLen;
    double  fPrevLen = CalcDistance(nCenter, nPrev) / fAbsLen;

    // same length for both sides if SYMMTR
    if ( GetFlags(nCenter) == PolyFlags::Symmetric )
    {
        fPrevLen = (fNextLen + fPrevLen) / 2;
        fNextLen = fPrevLen;
    }
    rNext.X() = rCenter.X() + static_cast<long>(fNextLen * aDiff.X());
    rNext.Y() = rCenter.Y() + static_cast<long>(fNextLen * aDiff.Y());
    rPrev.X() = rCenter.X() - static_cast<long>(fPrevLen * aDiff.X());
    rPrev.Y() = rCenter.Y() - static_cast<long>(fPrevLen * aDiff.Y());
}

/// convert four polygon points into a Bézier curve
void XPolygon::PointsToBezier(sal_uInt16 nFirst)
{
    double  nFullLength, nPart1Length, nPart2Length;
    double  fX0, fY0, fX1, fY1, fX2, fY2, fX3, fY3;
    double  fTx1, fTx2, fTy1, fTy2;
    double  fT1, fU1, fT2, fU2, fV;
    auto&   rPoints = pImpXPolygon->mvPointsAndFlags;

    if ( nFirst > pImpXPolygon->mvPointsAndFlags.size() - 4 || IsControl(nFirst) ||
         IsControl(nFirst+1) || IsControl(nFirst+2) || IsControl(nFirst+3) )
        return;

    fTx1 = rPoints[nFirst+1].first.X();
    fTy1 = rPoints[nFirst+1].first.Y();
    fTx2 = rPoints[nFirst+2].first.X();
    fTy2 = rPoints[nFirst+2].first.Y();
    fX0  = rPoints[nFirst  ].first.X();
    fY0  = rPoints[nFirst  ].first.Y();
    fX3  = rPoints[nFirst+3].first.X();
    fY3  = rPoints[nFirst+3].first.Y();

    nPart1Length = CalcDistance(nFirst, nFirst+1);
    nPart2Length = nPart1Length + CalcDistance(nFirst+1, nFirst+2);
    nFullLength  = nPart2Length + CalcDistance(nFirst+2, nFirst+3);
    if ( nFullLength < 20 )
        return;

    if ( nPart2Length == nFullLength )
        nPart2Length -= 1;
    if ( nPart1Length == nFullLength )
        nPart1Length = nPart2Length - 1;
    if ( nPart1Length <= 0 )
        nPart1Length = 1;
    if ( nPart2Length <= 0 || nPart2Length == nPart1Length )
        nPart2Length = nPart1Length + 1;

    fT1 = nPart1Length / nFullLength;
    fU1 = 1.0 - fT1;
    fT2 = nPart2Length / nFullLength;
    fU2 = 1.0 - fT2;
    fV = 3 * (1.0 - (fT1 * fU2) / (fT2 * fU1));

    fX1 = fTx1 / (fT1 * fU1 * fU1) - fTx2 * fT1 / (fT2 * fT2 * fU1 * fU2);
    fX1 /= fV;
    fX1 -= fX0 * ( fU1 / fT1 + fU2 / fT2) / 3;
    fX1 += fX3 * ( fT1 * fT2 / (fU1 * fU2)) / 3;

    fY1 = fTy1 / (fT1 * fU1 * fU1) - fTy2 * fT1 / (fT2 * fT2 * fU1 * fU2);
    fY1 /= fV;
    fY1 -= fY0 * ( fU1 / fT1 + fU2 / fT2) / 3;
    fY1 += fY3 * ( fT1 * fT2 / (fU1 * fU2)) / 3;

    fX2 = fTx2 / (fT2 * fT2 * fU2 * 3) - fX0 * fU2 * fU2 / ( fT2 * fT2 * 3);
    fX2 -= fX1 * fU2 / fT2;
    fX2 -= fX3 * fT2 / (fU2 * 3);

    fY2 = fTy2 / (fT2 * fT2 * fU2 * 3) - fY0 * fU2 * fU2 / ( fT2 * fT2 * 3);
    fY2 -= fY1 * fU2 / fT2;
    fY2 -= fY3 * fT2 / (fU2 * 3);

    rPoints[nFirst+1].first = Point(static_cast<long>(fX1), static_cast<long>(fY1));
    rPoints[nFirst+2].first = Point(static_cast<long>(fX2), static_cast<long>(fY2));
    SetFlags(nFirst+1, PolyFlags::Control);
    SetFlags(nFirst+2, PolyFlags::Control);
}

/// scale in X- and/or Y-direction
void XPolygon::Scale(double fSx, double fSy)
{
    sal_uInt16 nPntCnt = pImpXPolygon->mvPointsAndFlags.size();

    for (sal_uInt16 i = 0; i < nPntCnt; i++)
    {
        Point& rPnt = pImpXPolygon->mvPointsAndFlags[i].first;
        rPnt.X() = static_cast<long>(fSx * rPnt.X());
        rPnt.Y() = static_cast<long>(fSy * rPnt.Y());
    }
}

/**
 * Distort a polygon by scaling its coordinates relative to a reference
 * rectangle into an arbitrary rectangle.
 *
 * Mapping between polygon corners and reference rectangle:
 *     0: top left     0----1
 *     1: top right    |    |
 *     2: bottom right 3----2
 *     3: bottom left
 */
void XPolygon::Distort(const tools::Rectangle& rRefRect,
                       const XPolygon& rDistortedRect)
{
    long    Xr, Wr;
    long    Yr, Hr;

    Xr = rRefRect.Left();
    Yr = rRefRect.Top();
    Wr = rRefRect.GetWidth();
    Hr = rRefRect.GetHeight();

    if ( !Wr || !Hr )
        return;

    long    X1, X2, X3, X4;
    long    Y1, Y2, Y3, Y4;
    DBG_ASSERT(rDistortedRect.pImpXPolygon->mvPointsAndFlags.size() >= 4,
               "Distort: rectangle to small");

    X1 = rDistortedRect[0].X();
    Y1 = rDistortedRect[0].Y();
    X2 = rDistortedRect[1].X();
    Y2 = rDistortedRect[1].Y();
    X3 = rDistortedRect[3].X();
    Y3 = rDistortedRect[3].Y();
    X4 = rDistortedRect[2].X();
    Y4 = rDistortedRect[2].Y();

    sal_uInt16 nPntCnt = pImpXPolygon->mvPointsAndFlags.size();

    for (sal_uInt16 i = 0; i < nPntCnt; i++)
    {
        double  fTx, fTy, fUx, fUy;
        Point& rPnt = pImpXPolygon->mvPointsAndFlags[i].first;

        fTx = static_cast<double>(rPnt.X() - Xr) / Wr;
        fTy = static_cast<double>(rPnt.Y() - Yr) / Hr;
        fUx = 1.0 - fTx;
        fUy = 1.0 - fTy;

        rPnt.X() = static_cast<long>( fUy * (fUx * X1 + fTx * X2) +
                            fTy * (fUx * X3 + fTx * X4) );
        rPnt.Y() = static_cast<long>( fUx * (fUy * Y1 + fTy * Y3) +
                            fTx * (fUy * Y2 + fTy * Y4) );
    }
}

basegfx::B2DPolygon XPolygon::getB2DPolygon() const
{
    // #i74631# use tools Polygon class for conversion to not have the code doubled
    // here. This needs one more conversion but avoids different convertors in
    // the long run
    const tools::Polygon aSource(pImpXPolygon->mvPointsAndFlags);

    return aSource.getB2DPolygon();
}

XPolygon::XPolygon(const basegfx::B2DPolygon& rPolygon)
    : pImpXPolygon( tools::Polygon( rPolygon ).GetSize() )
{
    // #i74631# use tools Polygon class for conversion to not have the code doubled
    // here. This needs one more conversion but avoids different convertors in
    // the long run

    const tools::Polygon aSource(rPolygon);
    sal_uInt16 nSize = aSource.GetSize();
    pImpXPolygon->mvPointsAndFlags.resize(nSize);

    for( sal_uInt16 i = 0; i < nSize;  i++ )
    {
        pImpXPolygon->mvPointsAndFlags[i].first = aSource[i];
        pImpXPolygon->mvPointsAndFlags[i].second = aSource.GetFlags( i );
    }
}

// XPolyPolygon

ImpXPolyPolygon::ImpXPolyPolygon( const ImpXPolyPolygon& rImpXPolyPoly )
    : aXPolyList( rImpXPolyPoly.aXPolyList )
{
}

ImpXPolyPolygon::~ImpXPolyPolygon()
{
}

XPolyPolygon::XPolyPolygon()
    : pImpXPolyPolygon()
{
}

XPolyPolygon::XPolyPolygon( const XPolyPolygon& rXPolyPoly )
    : pImpXPolyPolygon( rXPolyPoly.pImpXPolyPolygon )
{
}

XPolyPolygon::XPolyPolygon( XPolyPolygon&& rXPolyPoly )
    : pImpXPolyPolygon( std::move(rXPolyPoly.pImpXPolyPolygon) )
{
}

XPolyPolygon::XPolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon)
    : pImpXPolyPolygon()
{
    for(sal_uInt32 a(0); a < rPolyPolygon.count(); a++)
    {
        const basegfx::B2DPolygon aCandidate = rPolyPolygon.getB2DPolygon(a);
        Insert(XPolygon(aCandidate));
    }
}

XPolyPolygon::~XPolyPolygon()
{
}

void XPolyPolygon::Insert( XPolygon&& rXPoly )
{
    pImpXPolyPolygon->aXPolyList.emplace_back( std::move(rXPoly) );
}

/// insert all XPolygons of a XPolyPolygon
void XPolyPolygon::Insert( const XPolyPolygon& rXPolyPoly )
{
    for ( size_t i = 0; i < rXPolyPoly.Count(); i++)
    {
        pImpXPolyPolygon->aXPolyList.emplace_back( rXPolyPoly[i] );
    }
}

void XPolyPolygon::Remove( sal_uInt16 nPos )
{
    pImpXPolyPolygon->aXPolyList.erase( pImpXPolyPolygon->aXPolyList.begin() + nPos );
}

const XPolygon& XPolyPolygon::GetObject( sal_uInt16 nPos ) const
{
    return pImpXPolyPolygon->aXPolyList[ nPos ];
}

void XPolyPolygon::Clear()
{
    pImpXPolyPolygon->aXPolyList.clear();
}

sal_uInt16 XPolyPolygon::Count() const
{
    return static_cast<sal_uInt16>(pImpXPolyPolygon->aXPolyList.size());
}

tools::Rectangle XPolyPolygon::GetBoundRect() const
{
    size_t nXPoly = pImpXPolyPolygon->aXPolyList.size();
    tools::Rectangle aRect;

    for ( size_t n = 0; n < nXPoly; n++ )
    {
        XPolygon const & rXPoly = pImpXPolyPolygon->aXPolyList[ n ];
        aRect.Union( rXPoly.GetBoundRect() );
    }

    return aRect;
}

XPolygon& XPolyPolygon::operator[]( sal_uInt16 nPos )
{
    return pImpXPolyPolygon->aXPolyList[ nPos ];
}

XPolyPolygon& XPolyPolygon::operator=( const XPolyPolygon& rXPolyPoly )
{
    pImpXPolyPolygon = rXPolyPoly.pImpXPolyPolygon;
    return *this;
}

XPolyPolygon& XPolyPolygon::operator=( XPolyPolygon&& rXPolyPoly )
{
    pImpXPolyPolygon = std::move(rXPolyPoly.pImpXPolyPolygon);
    return *this;
}

/**
 * Distort a polygon by scaling its coordinates relative to a reference
 * rectangle into an arbitrary rectangle.
 *
 * Mapping between polygon corners and reference rectangle:
 *     0: top left     0----1
 *     1: top right    |    |
 *     2: bottom right 3----2
 *     3: bottom left
 */
void XPolyPolygon::Distort(const tools::Rectangle& rRefRect,
                           const XPolygon& rDistortedRect)
{
    for (size_t i = 0; i < Count(); i++)
        pImpXPolyPolygon->aXPolyList[ i ].Distort(rRefRect, rDistortedRect);
}

basegfx::B2DPolyPolygon XPolyPolygon::getB2DPolyPolygon() const
{
    basegfx::B2DPolyPolygon aRetval;

    for(sal_uInt16 a(0); a < Count(); a++)
    {
        const XPolygon& rPoly = (*this)[a];
        aRetval.append(rPoly.getB2DPolygon());
    }

    return aRetval;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
