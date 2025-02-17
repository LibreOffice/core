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

#include <tools/debug.hxx>
#include <tools/poly.hxx>
#include <tools/helpers.hxx>
#include <tools/gen.hxx>

#include <svx/xpoly.hxx>
#include <xpolyimp.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drange.hxx>


ImpXPolygon::ImpXPolygon(sal_uInt16 nInitSize, sal_uInt16 _nResize)
    : pOldPointAry(nullptr)
    , bDeleteOldPoints(false)
    , nSize(0)
    , nResize(_nResize)
    , nPoints(0)
{
    Resize(nInitSize);
}

ImpXPolygon::ImpXPolygon( const ImpXPolygon& rImpXPoly )
    : pOldPointAry(nullptr)
    , bDeleteOldPoints(false)
    , nSize(0)
    , nResize(rImpXPoly.nResize)
    , nPoints(0)
{
    rImpXPoly.CheckPointDelete();

    Resize( rImpXPoly.nSize );

    // copy
    nPoints = rImpXPoly.nPoints;
    memcpy( pPointAry.get(), rImpXPoly.pPointAry.get(), nSize*sizeof( Point ) );
    memcpy( pFlagAry.get(), rImpXPoly.pFlagAry.get(), nSize );
}

ImpXPolygon::~ImpXPolygon()
{
    pPointAry.reset();
    if ( bDeleteOldPoints )
    {
        delete[] pOldPointAry;
        pOldPointAry = nullptr;
    }
}

bool ImpXPolygon::operator==(const ImpXPolygon& rImpXPoly) const
{
    return nPoints==rImpXPoly.nPoints &&
           (nPoints==0 ||
            (memcmp(pPointAry.get(), rImpXPoly.pPointAry.get(), nPoints*sizeof(Point))==0 &&
             memcmp(pFlagAry.get(), rImpXPoly.pFlagAry.get(), nPoints)==0));
}

/** Change polygon size
 *
 * @param nNewSize      the new size of the polygon
 * @param bDeletePoints if FALSE, do not delete the point array directly but
 *                      wait for the next call before doing so. This prevents
 *                      errors with XPoly[n] = XPoly[0] where a resize might
 *                      destroy the right side point array too early.
 */
void ImpXPolygon::Resize( sal_uInt16 nNewSize, bool bDeletePoints )
{
    if( nNewSize == nSize )
        return;

    PolyFlags*  pOldFlagAry  = pFlagAry.release();
    sal_uInt16  nOldSize     = nSize;

    CheckPointDelete();
    pOldPointAry = pPointAry.release();

    // Round the new size to a multiple of nResize, if
    // the object was not newly created (nSize != 0)
    if ( nSize != 0 && nNewSize > nSize )
    {
        DBG_ASSERT(nResize, "Trying to resize but nResize = 0 !");
        nNewSize = nSize + ((nNewSize-nSize-1) / nResize + 1) * nResize;
    }
    // create point array
    nSize     = nNewSize;
    pPointAry.reset( new Point[ nSize ] );

    // create flag array
    pFlagAry.reset( new PolyFlags[ nSize ] );
    memset( pFlagAry.get(), 0, nSize );

    // copy if needed
    if (nOldSize)
    {
        if( nOldSize < nSize )
        {
            memcpy( pPointAry.get(), pOldPointAry, nOldSize*sizeof( Point ) );
            memcpy( pFlagAry.get(),  pOldFlagAry, nOldSize );
        }
        else
        {
            memcpy( pPointAry.get(), pOldPointAry, nSize*sizeof( Point ) );
            memcpy( pFlagAry.get(), pOldFlagAry, nSize );

            // adjust number of valid points
            if( nPoints > nSize )
                nPoints = nSize;
        }
    }
    if ( bDeletePoints )
    {
        delete[] pOldPointAry;
        pOldPointAry = nullptr;
    }
    else
        bDeleteOldPoints = true;
    delete[] pOldFlagAry;
}

void ImpXPolygon::InsertSpace( sal_uInt16 nPos, sal_uInt16 nCount )
{
    CheckPointDelete();

    if ( nPos > nPoints )
        nPos = nPoints;

    // if the polygon is too small then enlarge it
    if( (nPoints + nCount) > nSize )
        Resize( nPoints + nCount );

    // If the insert is not at the last position, move everything after backwards
    if( nPos < nPoints )
    {
        sal_uInt16 nMove = nPoints - nPos;
        memmove( &pPointAry[nPos+nCount], &pPointAry[nPos],
                 nMove * sizeof(Point) );
        memmove( &pFlagAry[nPos+nCount], &pFlagAry[nPos], nMove );
    }
    std::fill(pPointAry.get() + nPos, pPointAry.get() + nPos + nCount, Point());
    memset( &pFlagAry [nPos], 0, nCount );

    nPoints = nPoints + nCount;
}

void ImpXPolygon::Remove( sal_uInt16 nPos, sal_uInt16 nCount )
{
    CheckPointDelete();

    if( (nPos + nCount) > nPoints )
        return;

    sal_uInt16 nMove = nPoints - nPos - nCount;

    if( nMove )
    {
        memmove( &pPointAry[nPos], &pPointAry[nPos+nCount],
                 nMove * sizeof(Point) );
        memmove( &pFlagAry[nPos], &pFlagAry[nPos+nCount], nMove );
    }
    std::fill(pPointAry.get() + (nPoints - nCount), pPointAry.get() + nPoints, Point());
    memset( &pFlagAry [nPoints - nCount], 0, nCount );
    nPoints = nPoints - nCount;
}

void ImpXPolygon::CheckPointDelete() const
{
    if ( bDeleteOldPoints )
    {
        delete[] pOldPointAry;
        const_cast< ImpXPolygon* >(this)->pOldPointAry = nullptr;
        const_cast< ImpXPolygon* >(this)->bDeleteOldPoints = false;
    }
}

XPolygon::XPolygon( sal_uInt16 nSize )
    : m_pImpXPolygon( ImpXPolygon( nSize, 16 ) )
{
}

XPolygon::XPolygon( const XPolygon& ) = default;

XPolygon::XPolygon( XPolygon&& ) = default;

/// create a XPolygon out of a standard polygon
XPolygon::XPolygon( const tools::Polygon& rPoly )
    : m_pImpXPolygon( rPoly.GetSize() )
{
    sal_uInt16 nSize = rPoly.GetSize();
    m_pImpXPolygon->nPoints = nSize;

    for( sal_uInt16 i = 0; i < nSize;  i++ )
    {
        m_pImpXPolygon->pPointAry[i] = rPoly[i];
        m_pImpXPolygon->pFlagAry[i] = rPoly.GetFlags( i );
    }
}

/// create a rectangle (also with rounded corners) as a Bézier polygon
XPolygon::XPolygon(const tools::Rectangle& rRect, tools::Long nRx, tools::Long nRy)
    : m_pImpXPolygon( 17 )
{
    tools::Long nWh = (rRect.GetWidth()  - 1) / 2;
    tools::Long nHh = (rRect.GetHeight() - 1) / 2;

    if ( nRx > nWh )    nRx = nWh;
    if ( nRy > nHh )    nRy = nHh;

    // negate Rx => circle clockwise
    nRx = -nRx;

    // factor for control points of the Bézier curve: 8/3 * (sin(45g) - 0.5)
    tools::Long    nXHdl = static_cast<tools::Long>(0.552284749 * nRx);
    tools::Long    nYHdl = static_cast<tools::Long>(0.552284749 * nRy);
    sal_uInt16  nPos = 0;

    if ( nRx && nRy )
    {
        Point aCenter;

        for (sal_uInt16 nQuad = 0; nQuad < 4; nQuad++)
        {
            switch ( nQuad )
            {
                case 0: aCenter = rRect.TopLeft();
                        aCenter.AdjustX( -nRx );
                        aCenter.AdjustY(nRy );
                        break;
                case 1: aCenter = rRect.TopRight();
                        aCenter.AdjustX(nRx );
                        aCenter.AdjustY(nRy );
                        break;
                case 2: aCenter = rRect.BottomRight();
                        aCenter.AdjustX(nRx );
                        aCenter.AdjustY( -nRy );
                        break;
                case 3: aCenter = rRect.BottomLeft();
                        aCenter.AdjustX( -nRx );
                        aCenter.AdjustY( -nRy );
                        break;
            }
            GenBezArc(aCenter, nRx, nRy, nXHdl, nYHdl, 0_deg100, 9000_deg100, nQuad, nPos);
            m_pImpXPolygon->pFlagAry[nPos  ] = PolyFlags::Smooth;
            m_pImpXPolygon->pFlagAry[nPos+3] = PolyFlags::Smooth;
            nPos += 4;
        }
    }
    else
    {
        m_pImpXPolygon->pPointAry[nPos++] = rRect.TopLeft();
        m_pImpXPolygon->pPointAry[nPos++] = rRect.TopRight();
        m_pImpXPolygon->pPointAry[nPos++] = rRect.BottomRight();
        m_pImpXPolygon->pPointAry[nPos++] = rRect.BottomLeft();
    }
    m_pImpXPolygon->pPointAry[nPos] = m_pImpXPolygon->pPointAry[0];
    m_pImpXPolygon->nPoints = nPos + 1;
}

/// create an ellipse (curve) as Bézier polygon
XPolygon::XPolygon(const Point& rCenter, tools::Long nRx, tools::Long nRy,
                   Degree100 nStartAngle, Degree100 nEndAngle, bool bClose)
    : m_pImpXPolygon( 17 )
{
    nStartAngle %= 36000_deg100;
    if ( nEndAngle > 36000_deg100 ) nEndAngle %= 36000_deg100;
    bool bFull = (nStartAngle == 0_deg100 && nEndAngle == 36000_deg100);

    // factor for control points of the Bézier curve: 8/3 * (sin(45g) - 0.5)
    tools::Long    nXHdl = static_cast<tools::Long>(0.552284749 * nRx);
    tools::Long    nYHdl = static_cast<tools::Long>(0.552284749 * nRy);
    sal_uInt16  nPos = 0;
    bool    bLoopEnd = false;

    do
    {
        Degree100 nA1, nA2;
        sal_uInt16 nQuad = nStartAngle.get() / 9000;
        if ( nQuad == 4 ) nQuad = 0;
        bLoopEnd = CheckAngles(nStartAngle, nEndAngle, nA1, nA2);
        GenBezArc(rCenter, nRx, nRy, nXHdl, nYHdl, nA1, nA2, nQuad, nPos);
        nPos += 3;
        if ( !bLoopEnd )
            m_pImpXPolygon->pFlagAry[nPos] = PolyFlags::Smooth;

    } while ( !bLoopEnd );

    // if not a full circle then connect edges with center point if necessary
    if ( !bFull && bClose )
        m_pImpXPolygon->pPointAry[++nPos] = rCenter;

    if ( bFull )
    {
        m_pImpXPolygon->pFlagAry[0   ] = PolyFlags::Smooth;
        m_pImpXPolygon->pFlagAry[nPos] = PolyFlags::Smooth;
    }
    m_pImpXPolygon->nPoints = nPos + 1;
}

XPolygon::~XPolygon() = default;

void XPolygon::SetPointCount( sal_uInt16 nPoints )
{
    std::as_const(m_pImpXPolygon)->CheckPointDelete();

    if( m_pImpXPolygon->nSize < nPoints )
        m_pImpXPolygon->Resize( nPoints );

    if ( nPoints < m_pImpXPolygon->nPoints )
    {
        sal_uInt16 nSize = m_pImpXPolygon->nPoints - nPoints;
        std::fill(
            m_pImpXPolygon->pPointAry.get() + nPoints, m_pImpXPolygon->pPointAry.get() + nPoints + nSize, Point());
        memset( &m_pImpXPolygon->pFlagAry [nPoints], 0, nSize );
    }
    m_pImpXPolygon->nPoints = nPoints;
}

sal_uInt16 XPolygon::GetSize() const
{
    m_pImpXPolygon->CheckPointDelete();
    return m_pImpXPolygon->nSize;
}

sal_uInt16 XPolygon::GetPointCount() const
{
    m_pImpXPolygon->CheckPointDelete();
    return m_pImpXPolygon->nPoints;
}

void XPolygon::Insert( sal_uInt16 nPos, const Point& rPt, PolyFlags eFlags )
{
    if (nPos>m_pImpXPolygon->nPoints) nPos=m_pImpXPolygon->nPoints;
    m_pImpXPolygon->InsertSpace( nPos, 1 );
    m_pImpXPolygon->pPointAry[nPos] = rPt;
    m_pImpXPolygon->pFlagAry[nPos]  = eFlags;
}

void XPolygon::Insert( sal_uInt16 nPos, const XPolygon& rXPoly )
{
    if (nPos>m_pImpXPolygon->nPoints) nPos=m_pImpXPolygon->nPoints;

    sal_uInt16 nPoints = rXPoly.GetPointCount();

    m_pImpXPolygon->InsertSpace( nPos, nPoints );

    memcpy( &(m_pImpXPolygon->pPointAry[nPos]),
            rXPoly.m_pImpXPolygon->pPointAry.get(),
            nPoints*sizeof( Point ) );
    memcpy( &(m_pImpXPolygon->pFlagAry[nPos]),
            rXPoly.m_pImpXPolygon->pFlagAry.get(),
            nPoints );
}

void XPolygon::Remove( sal_uInt16 nPos, sal_uInt16 nCount )
{
    m_pImpXPolygon->Remove( nPos, nCount );
}

void XPolygon::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    if ( !nHorzMove && !nVertMove )
        return;

    // move points
    sal_uInt16 nCount = m_pImpXPolygon->nPoints;
    for ( sal_uInt16 i = 0; i < nCount; i++ )
    {
        Point* pPt = &(m_pImpXPolygon->pPointAry[i]);
        pPt->AdjustX( nHorzMove );
        pPt->AdjustY( nVertMove );
    }
}

tools::Rectangle XPolygon::GetBoundRect() const
{
    m_pImpXPolygon->CheckPointDelete();
    tools::Rectangle aRetval;

    if(m_pImpXPolygon->nPoints)
    {
        // #i37709#
        // For historical reasons the control points are not part of the
        // BoundRect. This makes it necessary to subdivide the polygon to
        // get a relatively correct BoundRect. Numerically, this is not
        // correct and never was.

        const basegfx::B2DRange aPolygonRange(basegfx::utils::getRange(getB2DPolygon()));
        aRetval = tools::Rectangle(basegfx::fround<tools::Long>(aPolygonRange.getMinX()),
                                   basegfx::fround<tools::Long>(aPolygonRange.getMinY()),
                                   basegfx::fround<tools::Long>(aPolygonRange.getMaxX()),
                                   basegfx::fround<tools::Long>(aPolygonRange.getMaxY()));
    }

    return aRetval;
}

const Point& XPolygon::operator[]( sal_uInt16 nPos ) const
{
    DBG_ASSERT(nPos < m_pImpXPolygon->nPoints, "Invalid index at const array access to XPolygon");

    m_pImpXPolygon->CheckPointDelete();
    return m_pImpXPolygon->pPointAry[nPos];
}

Point& XPolygon::operator[]( sal_uInt16 nPos )
{
    std::as_const(m_pImpXPolygon)->CheckPointDelete();

    if( nPos >= m_pImpXPolygon->nSize )
    {
        DBG_ASSERT(m_pImpXPolygon->nResize, "Invalid index at array access to XPolygon");
        m_pImpXPolygon->Resize(nPos + 1, false);
    }
    if( nPos >= m_pImpXPolygon->nPoints )
        m_pImpXPolygon->nPoints = nPos + 1;

    return m_pImpXPolygon->pPointAry[nPos];
}

XPolygon& XPolygon::operator=( const XPolygon& ) = default;

XPolygon& XPolygon::operator=( XPolygon&& ) = default;

bool XPolygon::operator==( const XPolygon& rXPoly ) const
{
    m_pImpXPolygon->CheckPointDelete();
    return rXPoly.m_pImpXPolygon == m_pImpXPolygon;
}

/// get the flags for the point at the given position
PolyFlags XPolygon::GetFlags( sal_uInt16 nPos ) const
{
    m_pImpXPolygon->CheckPointDelete();
    return m_pImpXPolygon->pFlagAry[nPos];
}

/// set the flags for the point at the given position
void XPolygon::SetFlags( sal_uInt16 nPos, PolyFlags eFlags )
{
    std::as_const(m_pImpXPolygon)->CheckPointDelete();
    m_pImpXPolygon->pFlagAry[nPos] = eFlags;
}

/// short path to read the CONTROL flag directly (TODO: better explain what the sense behind this flag is!)
bool XPolygon::IsControl(sal_uInt16 nPos) const
{
    return m_pImpXPolygon->pFlagAry[nPos] == PolyFlags::Control;
}

/// short path to read the SMOOTH and SYMMTR flag directly (TODO: better explain what the sense behind these flags is!)
bool XPolygon::IsSmooth(sal_uInt16 nPos) const
{
    PolyFlags eFlag = m_pImpXPolygon->pFlagAry[nPos];
    return ( eFlag == PolyFlags::Smooth || eFlag == PolyFlags::Symmetric );
}

/** calculate the euclidean distance between two points
 *
 * @param nP1 The first point
 * @param nP2 The second point
 */
double XPolygon::CalcDistance(sal_uInt16 nP1, sal_uInt16 nP2)
{
    const Point& rP1 = m_pImpXPolygon->pPointAry[nP1];
    const Point& rP2 = m_pImpXPolygon->pPointAry[nP2];
    double fDx = rP2.X() - rP1.X();
    double fDy = rP2.Y() - rP1.Y();
    return std::hypot(fDx, fDy);
}

void XPolygon::SubdivideBezier(sal_uInt16 nPos, bool bCalcFirst, double fT)
{
    Point*  pPoints = m_pImpXPolygon->pPointAry.get();
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
    pPoints[nPos].setX( static_cast<tools::Long>(fU3 *       pPoints[nIdx  ].X() +
                                fT  * fU2 * pPoints[nIdx+1].X() * 3 +
                                fT2 * fU  * pPoints[nIdx+2].X() * 3 +
                                fT3 *       pPoints[nIdx+3].X()) );
    pPoints[nPos].setY( static_cast<tools::Long>(fU3 *       pPoints[nIdx  ].Y() +
                                fT  * fU2 * pPoints[nIdx+1].Y() * 3 +
                                fT2 * fU  * pPoints[nIdx+2].Y() * 3 +
                                fT3 *       pPoints[nIdx+3].Y()) );
    nPos = nPos + nPosInc;
    nIdx = nIdx + nIdxInc;
    pPoints[nPos].setX( static_cast<tools::Long>(fU2 *       pPoints[nIdx  ].X() +
                                fT  * fU *  pPoints[nIdx+1].X() * 2 +
                                fT2 *       pPoints[nIdx+2].X()) );
    pPoints[nPos].setY( static_cast<tools::Long>(fU2 *       pPoints[nIdx  ].Y() +
                                fT  * fU *  pPoints[nIdx+1].Y() * 2 +
                                fT2 *       pPoints[nIdx+2].Y()) );
    nPos = nPos + nPosInc;
    nIdx = nIdx + nIdxInc;
    pPoints[nPos].setX( static_cast<tools::Long>(fU * pPoints[nIdx  ].X() +
                                fT * pPoints[nIdx+1].X()) );
    pPoints[nPos].setY( static_cast<tools::Long>(fU * pPoints[nIdx  ].Y() +
                                fT * pPoints[nIdx+1].Y()) );
}

/// Generate a Bézier arc
void XPolygon::GenBezArc(const Point& rCenter, tools::Long nRx, tools::Long nRy,
                         tools::Long nXHdl, tools::Long nYHdl, Degree100 nStart, Degree100 nEnd,
                         sal_uInt16 nQuad, sal_uInt16 nFirst)
{
    Point* pPoints = m_pImpXPolygon->pPointAry.get();
    pPoints[nFirst  ] = rCenter;
    pPoints[nFirst+3] = rCenter;

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
        pPoints[nFirst].AdjustX( nRx );
        pPoints[nFirst+3].AdjustY( nRy );
    }
    else
    {
        pPoints[nFirst].AdjustY( nRy );
        pPoints[nFirst+3].AdjustX( nRx );
    }
    pPoints[nFirst+1] = pPoints[nFirst];
    pPoints[nFirst+2] = pPoints[nFirst+3];

    if ( nQuad == 0 || nQuad == 2 )
    {
        pPoints[nFirst+1].AdjustY( nYHdl );
        pPoints[nFirst+2].AdjustX( nXHdl );
    }
    else
    {
        pPoints[nFirst+1].AdjustX( nXHdl );
        pPoints[nFirst+2].AdjustY( nYHdl );
    }
    if ( nStart > 0_deg100 )
        SubdivideBezier(nFirst, false, static_cast<double>(nStart.get()) / 9000);
    if ( nEnd < 9000_deg100 )
        SubdivideBezier(nFirst, true, static_cast<double>((nEnd-nStart).get()) / (9000_deg100-nStart).get());
    SetFlags(nFirst+1, PolyFlags::Control);
    SetFlags(nFirst+2, PolyFlags::Control);
}

bool XPolygon::CheckAngles(Degree100& nStart, Degree100 nEnd, Degree100& nA1, Degree100& nA2)
{
    if ( nStart == 36000_deg100 ) nStart = 0_deg100;
    if ( nEnd == 0_deg100 ) nEnd = 36000_deg100;
    Degree100 nStPrev = nStart;
    Degree100 nMax((nStart.get() / 9000 + 1) * 9000);
    Degree100 nMin = nMax - 9000_deg100;

    if ( nEnd >= nMax || nEnd <= nStart )   nA2 = 9000_deg100;
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
    // If nPoint is no control point, i.e. cannot be moved, then
    // move nDrag instead on the line between nCenter and nPnt
    if ( !IsControl(nPnt) )
        std::swap( nDrag, nPnt );
    Point*  pPoints = m_pImpXPolygon->pPointAry.get();
    Point   aDiff   = pPoints[nDrag] - pPoints[nCenter];
    double  fDiv    = CalcDistance(nCenter, nDrag);

    if ( fDiv )
    {
        double fRatio = CalcDistance(nCenter, nPnt) / fDiv;
        // keep the length if SMOOTH
        if ( GetFlags(nCenter) == PolyFlags::Smooth || !IsControl(nDrag) )
        {
            aDiff.setX( static_cast<tools::Long>(fRatio * aDiff.X()) );
            aDiff.setY( static_cast<tools::Long>(fRatio * aDiff.Y()) );
        }
        pPoints[nPnt] = pPoints[nCenter] - aDiff;
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

    const Point& rCenter = m_pImpXPolygon->pPointAry[nCenter];
    Point&  rNext = m_pImpXPolygon->pPointAry[nNext];
    Point&  rPrev = m_pImpXPolygon->pPointAry[nPrev];
    Point   aDiff = rNext - rPrev;
    double  fNextLen = CalcDistance(nCenter, nNext) / fAbsLen;
    double  fPrevLen = CalcDistance(nCenter, nPrev) / fAbsLen;

    // same length for both sides if SYMMTR
    if ( GetFlags(nCenter) == PolyFlags::Symmetric )
    {
        fPrevLen = (fNextLen + fPrevLen) / 2;
        fNextLen = fPrevLen;
    }
    rNext.setX( rCenter.X() + static_cast<tools::Long>(fNextLen * aDiff.X()) );
    rNext.setY( rCenter.Y() + static_cast<tools::Long>(fNextLen * aDiff.Y()) );
    rPrev.setX( rCenter.X() - static_cast<tools::Long>(fPrevLen * aDiff.X()) );
    rPrev.setY( rCenter.Y() - static_cast<tools::Long>(fPrevLen * aDiff.Y()) );
}

/// convert four polygon points into a Bézier curve
void XPolygon::PointsToBezier(sal_uInt16 nFirst)
{
    double  nFullLength, nPart1Length, nPart2Length;
    double  fX0, fY0, fX1, fY1, fX2, fY2, fX3, fY3;
    double  fTx1, fTx2, fTy1, fTy2;
    double  fT1, fU1, fT2, fU2, fV;
    Point*  pPoints = m_pImpXPolygon->pPointAry.get();

    if ( nFirst > m_pImpXPolygon->nPoints - 4 || IsControl(nFirst) ||
         IsControl(nFirst+1) || IsControl(nFirst+2) || IsControl(nFirst+3) )
        return;

    fTx1 = pPoints[nFirst+1].X();
    fTy1 = pPoints[nFirst+1].Y();
    fTx2 = pPoints[nFirst+2].X();
    fTy2 = pPoints[nFirst+2].Y();
    fX0  = pPoints[nFirst  ].X();
    fY0  = pPoints[nFirst  ].Y();
    fX3  = pPoints[nFirst+3].X();
    fY3  = pPoints[nFirst+3].Y();

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

    Point aControlPoint1(static_cast<tools::Long>(fX1), static_cast<tools::Long>(fY1));
    Point aControlPoint2(static_cast<tools::Long>(fX2), static_cast<tools::Long>(fY2));

    auto fPointOffset1 = std::hypot(aControlPoint1.X() - pPoints[nFirst + 1].X(), aControlPoint1.Y() - pPoints[nFirst + 1].Y());
    auto fPointOffset2 = std::hypot(aControlPoint2.X() - pPoints[nFirst + 2].X(), aControlPoint2.Y() - pPoints[nFirst + 2].Y());

    // To prevent the curve from overshooting due to sharp direction changes in the given sequence of points,
    // compare the control point offsets against the full segment length.
    // Apply the calculated ControlPoints only if their offsets are within a reasonable range.
    if( fPointOffset1 < nFullLength && fPointOffset2 < nFullLength )
    {
        pPoints[nFirst + 1] = aControlPoint1;
        pPoints[nFirst + 2] = aControlPoint2;
    }

    SetFlags(nFirst+1, PolyFlags::Control);
    SetFlags(nFirst+2, PolyFlags::Control);
}

/// scale in X- and/or Y-direction
void XPolygon::Scale(double fSx, double fSy)
{
    std::as_const(m_pImpXPolygon)->CheckPointDelete();

    sal_uInt16 nPntCnt = m_pImpXPolygon->nPoints;

    for (sal_uInt16 i = 0; i < nPntCnt; i++)
    {
        Point& rPnt = m_pImpXPolygon->pPointAry[i];
        rPnt.setX( static_cast<tools::Long>(fSx * rPnt.X()) );
        rPnt.setY( static_cast<tools::Long>(fSy * rPnt.Y()) );
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
    std::as_const(m_pImpXPolygon)->CheckPointDelete();

    tools::Long    Xr, Wr;
    tools::Long    Yr, Hr;

    Xr = rRefRect.Left();
    Yr = rRefRect.Top();
    Wr = rRefRect.GetWidth();
    Hr = rRefRect.GetHeight();

    if ( !Wr || !Hr )
        return;

    tools::Long    X1, X2, X3, X4;
    tools::Long    Y1, Y2, Y3, Y4;
    DBG_ASSERT(rDistortedRect.m_pImpXPolygon->nPoints >= 4,
               "Distort: rectangle too small");

    X1 = rDistortedRect[0].X();
    Y1 = rDistortedRect[0].Y();
    X2 = rDistortedRect[1].X();
    Y2 = rDistortedRect[1].Y();
    X3 = rDistortedRect[3].X();
    Y3 = rDistortedRect[3].Y();
    X4 = rDistortedRect[2].X();
    Y4 = rDistortedRect[2].Y();

    sal_uInt16 nPntCnt = m_pImpXPolygon->nPoints;

    for (sal_uInt16 i = 0; i < nPntCnt; i++)
    {
        double  fTx, fTy, fUx, fUy;
        Point& rPnt = m_pImpXPolygon->pPointAry[i];

        fTx = static_cast<double>(rPnt.X() - Xr) / Wr;
        fTy = static_cast<double>(rPnt.Y() - Yr) / Hr;
        fUx = 1.0 - fTx;
        fUy = 1.0 - fTy;

        rPnt.setX( static_cast<tools::Long>( fUy * (fUx * X1 + fTx * X2) +
                            fTy * (fUx * X3 + fTx * X4) ) );
        rPnt.setY( static_cast<tools::Long>( fUx * (fUy * Y1 + fTy * Y3) +
                            fTx * (fUy * Y2 + fTy * Y4) ) );
    }
}

basegfx::B2DPolygon XPolygon::getB2DPolygon() const
{
    // #i74631# use tools Polygon class for conversion to not have the code doubled
    // here. This needs one more conversion but avoids different converters in
    // the long run
    const tools::Polygon aSource(GetPointCount(), m_pImpXPolygon->pPointAry.get(), m_pImpXPolygon->pFlagAry.get());

    return aSource.getB2DPolygon();
}

XPolygon::XPolygon(const basegfx::B2DPolygon& rPolygon)
    : m_pImpXPolygon( tools::Polygon( rPolygon ).GetSize() )
{
    // #i74631# use tools Polygon class for conversion to not have the code doubled
    // here. This needs one more conversion but avoids different converters in
    // the long run

    const tools::Polygon aSource(rPolygon);
    sal_uInt16 nSize = aSource.GetSize();
    m_pImpXPolygon->nPoints = nSize;

    for( sal_uInt16 i = 0; i < nSize;  i++ )
    {
        m_pImpXPolygon->pPointAry[i] = aSource[i];
        m_pImpXPolygon->pFlagAry[i] = aSource.GetFlags( i );
    }
}

// XPolyPolygon
XPolyPolygon::XPolyPolygon() = default;

XPolyPolygon::XPolyPolygon( const XPolyPolygon& ) = default;

XPolyPolygon::XPolyPolygon( XPolyPolygon&& ) = default;

XPolyPolygon::XPolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    for(auto const& rCandidate : rPolyPolygon)
    {
        Insert(XPolygon(rCandidate));
    }
}

XPolyPolygon::~XPolyPolygon() = default;

void XPolyPolygon::Insert( XPolygon&& rXPoly )
{
    m_pImpXPolyPolygon->aXPolyList.emplace_back( std::move(rXPoly) );
}

/// insert all XPolygons of a XPolyPolygon
void XPolyPolygon::Insert( const XPolyPolygon& rXPolyPoly )
{
    for ( size_t i = 0; i < rXPolyPoly.Count(); i++)
    {
        m_pImpXPolyPolygon->aXPolyList.emplace_back( rXPolyPoly[i] );
    }
}

void XPolyPolygon::Remove( sal_uInt16 nPos )
{
    m_pImpXPolyPolygon->aXPolyList.erase( m_pImpXPolyPolygon->aXPolyList.begin() + nPos );
}

const XPolygon& XPolyPolygon::GetObject( sal_uInt16 nPos ) const
{
    return m_pImpXPolyPolygon->aXPolyList[ nPos ];
}

void XPolyPolygon::Clear()
{
    m_pImpXPolyPolygon->aXPolyList.clear();
}

sal_uInt16 XPolyPolygon::Count() const
{
    return static_cast<sal_uInt16>(m_pImpXPolyPolygon->aXPolyList.size());
}

tools::Rectangle XPolyPolygon::GetBoundRect() const
{
    size_t nXPoly = m_pImpXPolyPolygon->aXPolyList.size();
    tools::Rectangle aRect;

    for ( size_t n = 0; n < nXPoly; n++ )
    {
        XPolygon const & rXPoly = m_pImpXPolyPolygon->aXPolyList[ n ];
        aRect.Union( rXPoly.GetBoundRect() );
    }

    return aRect;
}

XPolygon& XPolyPolygon::operator[]( sal_uInt16 nPos )
{
    return m_pImpXPolyPolygon->aXPolyList[ nPos ];
}

XPolyPolygon& XPolyPolygon::operator=( const XPolyPolygon& ) = default;

XPolyPolygon& XPolyPolygon::operator=( XPolyPolygon&& ) = default;

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
        m_pImpXPolyPolygon->aXPolyList[ i ].Distort(rRefRect, rDistortedRect);
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
