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


ImpXPolygon::ImpXPolygon(sal_uInt16 nInitSize, sal_uInt16 _nResize)
    : pPointAry(nullptr)
    , pFlagAry(nullptr)
    , pOldPointAry(nullptr)
    , bDeleteOldPoints(false)
    , nSize(0)
    , nResize(_nResize)
    , nPoints(0)
{
    Resize(nInitSize);
}

ImpXPolygon::ImpXPolygon( const ImpXPolygon& rImpXPoly )
    : pPointAry(nullptr)
    , pFlagAry(nullptr)
    , pOldPointAry(nullptr)
    , bDeleteOldPoints(false)
    , nSize(0)
    , nResize(rImpXPoly.nResize)
    , nPoints(0)
{
    rImpXPoly.CheckPointDelete();

    Resize( rImpXPoly.nSize );

    // copy
    nPoints = rImpXPoly.nPoints;
    memcpy( pPointAry, rImpXPoly.pPointAry, nSize*sizeof( Point ) );
    memcpy( pFlagAry.get(), rImpXPoly.pFlagAry.get(), nSize );
}

ImpXPolygon::~ImpXPolygon()
{
    delete[] reinterpret_cast<char*>(pPointAry);
    if ( bDeleteOldPoints )
    {
        delete[] reinterpret_cast<char*>(pOldPointAry);
        pOldPointAry = nullptr;
    }
}

bool ImpXPolygon::operator==(const ImpXPolygon& rImpXPoly) const
{
    return nPoints==rImpXPoly.nPoints &&
           (nPoints==0 ||
            (memcmp(pPointAry, rImpXPoly.pPointAry, nPoints*sizeof(Point))==0 &&
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
    pOldPointAry = pPointAry;

    // Round the new size to a multiple of nResize, if
    // the object was not newly created (nSize != 0)
    if ( nSize != 0 && nNewSize > nSize )
    {
        DBG_ASSERT(nResize, "Trying to resize but nResize = 0 !");
        nNewSize = nSize + ((nNewSize-nSize-1) / nResize + 1) * nResize;
    }
    // create point array
    nSize     = nNewSize;
    pPointAry = reinterpret_cast<Point*>(new char[ nSize*sizeof( Point ) ]);
    memset( pPointAry, 0, nSize*sizeof( Point ) );

    // create flag array
    pFlagAry.reset( new PolyFlags[ nSize ] );
    memset( pFlagAry.get(), 0, nSize );

    // copy if needed
    if( nOldSize )
    {
        if( nOldSize < nSize )
        {
            memcpy( pPointAry, pOldPointAry, nOldSize*sizeof( Point ) );
            memcpy( pFlagAry.get(),  pOldFlagAry, nOldSize );
        }
        else
        {
            memcpy( pPointAry, pOldPointAry, nSize*sizeof( Point ) );
            memcpy( pFlagAry.get(), pOldFlagAry, nSize );

            // adjust number of valid points
            if( nPoints > nSize )
                nPoints = nSize;
        }
        if ( bDeletePoints )
        {
            delete[] reinterpret_cast<char*>(pOldPointAry);
            pOldPointAry = nullptr;
        }
        else
            bDeleteOldPoints = true;
        delete[] pOldFlagAry;
    }
}

void ImpXPolygon::InsertSpace( sal_uInt16 nPos, sal_uInt16 nCount )
{
    CheckPointDelete();

    if ( nPos > nPoints )
        nPos = nPoints;

    // if the polygon is too small than enlarge it
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
    memset( &pPointAry[nPos], 0, nCount * sizeof( Point ) );
    memset( &pFlagAry [nPos], 0, nCount );

    nPoints = nPoints + nCount;
}

void ImpXPolygon::Remove( sal_uInt16 nPos, sal_uInt16 nCount )
{
    CheckPointDelete();

    if( (nPos + nCount) <= nPoints )
    {
        sal_uInt16 nMove = nPoints - nPos - nCount;

        if( nMove )
        {
            memmove( &pPointAry[nPos], &pPointAry[nPos+nCount],
                     nMove * sizeof(Point) );
            memmove( &pFlagAry[nPos], &pFlagAry[nPos+nCount], nMove );
        }
        memset( &pPointAry[nPoints - nCount], 0, nCount * sizeof( Point ) );
        memset( &pFlagAry [nPoints - nCount], 0, nCount );
        nPoints = nPoints - nCount;
    }
}

void ImpXPolygon::CheckPointDelete() const
{
    if ( bDeleteOldPoints )
    {
        delete[] reinterpret_cast<char*>(pOldPointAry);
        const_cast< ImpXPolygon* >(this)->pOldPointAry = nullptr;
        const_cast< ImpXPolygon* >(this)->bDeleteOldPoints = false;
    }
}

XPolygon::XPolygon( sal_uInt16 nSize )
    : pImpXPolygon( ImpXPolygon( nSize, 16 ) )
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
    pImpXPolygon->nPoints = nSize;

    for( sal_uInt16 i = 0; i < nSize;  i++ )
    {
        pImpXPolygon->pPointAry[i] = rPoly[i];
        pImpXPolygon->pFlagAry[i] = rPoly.GetFlags( i );
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
    long    nXHdl = (long)(0.552284749 * nRx);
    long    nYHdl = (long)(0.552284749 * nRy);
    sal_uInt16  nPos = 0;

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
            pImpXPolygon->pFlagAry[nPos  ] = PolyFlags::Smooth;
            pImpXPolygon->pFlagAry[nPos+3] = PolyFlags::Smooth;
            nPos += 4;
        }
    }
    else
    {
        pImpXPolygon->pPointAry[nPos++] = rRect.TopLeft();
        pImpXPolygon->pPointAry[nPos++] = rRect.TopRight();
        pImpXPolygon->pPointAry[nPos++] = rRect.BottomRight();
        pImpXPolygon->pPointAry[nPos++] = rRect.BottomLeft();
    }
    pImpXPolygon->pPointAry[nPos] = pImpXPolygon->pPointAry[0];
    pImpXPolygon->nPoints = nPos + 1;
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
    long    nXHdl = (long)(0.552284749 * nRx);
    long    nYHdl = (long)(0.552284749 * nRy);
    sal_uInt16  nPos = 0;
    bool    bLoopEnd = false;

    do
    {
        sal_uInt16 nA1, nA2;
        sal_uInt16 nQuad = nStartAngle / 900;
        if ( nQuad == 4 ) nQuad = 0;
        bLoopEnd = CheckAngles(nStartAngle, nEndAngle, nA1, nA2);
        GenBezArc(rCenter, nRx, nRy, nXHdl, nYHdl, nA1, nA2, nQuad, nPos);
        nPos += 3;
        if ( !bLoopEnd )
            pImpXPolygon->pFlagAry[nPos] = PolyFlags::Smooth;

    } while ( !bLoopEnd );

    // if not a full circle than connect edges with center point if necessary
    if ( !bFull && bClose )
        pImpXPolygon->pPointAry[++nPos] = rCenter;

    if ( bFull )
    {
        pImpXPolygon->pFlagAry[0   ] = PolyFlags::Smooth;
        pImpXPolygon->pFlagAry[nPos] = PolyFlags::Smooth;
    }
    pImpXPolygon->nPoints = nPos + 1;
}

XPolygon::~XPolygon()
{
}

void XPolygon::SetPointCount( sal_uInt16 nPoints )
{
    pImpXPolygon->CheckPointDelete();

    if( pImpXPolygon->nSize < nPoints )
        pImpXPolygon->Resize( nPoints );

    if ( nPoints < pImpXPolygon->nPoints )
    {
        sal_uInt16 nSize = pImpXPolygon->nPoints - nPoints;
        memset( &pImpXPolygon->pPointAry[nPoints], 0, nSize * sizeof( Point ) );
        memset( &pImpXPolygon->pFlagAry [nPoints], 0, nSize );
    }
    pImpXPolygon->nPoints = nPoints;
}

sal_uInt16 XPolygon::GetSize() const
{
    pImpXPolygon->CheckPointDelete();
    return pImpXPolygon->nSize;
}

sal_uInt16 XPolygon::GetPointCount() const
{
    pImpXPolygon->CheckPointDelete();
    return pImpXPolygon->nPoints;
}

void XPolygon::Insert( sal_uInt16 nPos, const Point& rPt, PolyFlags eFlags )
{
    if (nPos>pImpXPolygon->nPoints) nPos=pImpXPolygon->nPoints;
    pImpXPolygon->InsertSpace( nPos, 1 );
    pImpXPolygon->pPointAry[nPos] = rPt;
    pImpXPolygon->pFlagAry[nPos]  = eFlags;
}

void XPolygon::Insert( sal_uInt16 nPos, const XPolygon& rXPoly )
{
    if (nPos>pImpXPolygon->nPoints) nPos=pImpXPolygon->nPoints;

    sal_uInt16 nPoints = rXPoly.GetPointCount();

    pImpXPolygon->InsertSpace( nPos, nPoints );

    memcpy( &(pImpXPolygon->pPointAry[nPos]),
            rXPoly.pImpXPolygon->pPointAry,
            nPoints*sizeof( Point ) );
    memcpy( &(pImpXPolygon->pFlagAry[nPos]),
            rXPoly.pImpXPolygon->pFlagAry.get(),
            nPoints );
}

void XPolygon::Remove( sal_uInt16 nPos, sal_uInt16 nCount )
{
    pImpXPolygon->Remove( nPos, nCount );
}

void XPolygon::Move( long nHorzMove, long nVertMove )
{
    if ( !nHorzMove && !nVertMove )
        return;

    // move points
    sal_uInt16 nCount = pImpXPolygon->nPoints;
    for ( sal_uInt16 i = 0; i < nCount; i++ )
    {
        Point* pPt = &(pImpXPolygon->pPointAry[i]);
        pPt->X() += nHorzMove;
        pPt->Y() += nVertMove;
    }
}

tools::Rectangle XPolygon::GetBoundRect() const
{
    pImpXPolygon->CheckPointDelete();
    tools::Rectangle aRetval;

    if(pImpXPolygon->nPoints)
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
    DBG_ASSERT(nPos < pImpXPolygon->nPoints, "Invalid index at const array access to XPolygon");

    pImpXPolygon->CheckPointDelete();
    return pImpXPolygon->pPointAry[nPos];
}

Point& XPolygon::operator[]( sal_uInt16 nPos )
{
    pImpXPolygon->CheckPointDelete();

    if( nPos >= pImpXPolygon->nSize )
    {
        DBG_ASSERT(pImpXPolygon->nResize, "Invalid index at array access to XPolygon");
        pImpXPolygon->Resize(nPos + 1, false);
    }
    if( nPos >= pImpXPolygon->nPoints )
        pImpXPolygon->nPoints = nPos + 1;

    return pImpXPolygon->pPointAry[nPos];
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
    pImpXPolygon->CheckPointDelete();
    return rXPoly.pImpXPolygon == pImpXPolygon;
}

/// get the flags for the point at the given position
PolyFlags XPolygon::GetFlags( sal_uInt16 nPos ) const
{
    pImpXPolygon->CheckPointDelete();
    return pImpXPolygon->pFlagAry[nPos];
}

/// set the flags for the point at the given position
void XPolygon::SetFlags( sal_uInt16 nPos, PolyFlags eFlags )
{
    pImpXPolygon->CheckPointDelete();
    pImpXPolygon->pFlagAry[nPos] = eFlags;
}

/// short path to read the CONTROL flag directly (TODO: better explain what the sense behind this flag is!)
bool XPolygon::IsControl(sal_uInt16 nPos) const
{
    return pImpXPolygon->pFlagAry[nPos] == PolyFlags::Control;
}

/// short path to read the SMOOTH and SYMMTR flag directly (TODO: better explain what the sense behind these flags is!)
bool XPolygon::IsSmooth(sal_uInt16 nPos) const
{
    PolyFlags eFlag = pImpXPolygon->pFlagAry[nPos];
    return ( eFlag == PolyFlags::Smooth || eFlag == PolyFlags::Symmetric );
}

/** calculate the euclidean distance between two points
 *
 * @param nP1 The first point
 * @param nP2 The second point
 */
double XPolygon::CalcDistance(sal_uInt16 nP1, sal_uInt16 nP2)
{
    const Point& rP1 = pImpXPolygon->pPointAry[nP1];
    const Point& rP2 = pImpXPolygon->pPointAry[nP2];
    double fDx = rP2.X() - rP1.X();
    double fDy = rP2.Y() - rP1.Y();
    return sqrt(fDx * fDx + fDy * fDy);
}

void XPolygon::SubdivideBezier(sal_uInt16 nPos, bool bCalcFirst, double fT)
{
    Point*  pPoints = pImpXPolygon->pPointAry;
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
    pPoints[nPos].X() = (long) (fU3 *       pPoints[nIdx  ].X() +
                                fT  * fU2 * pPoints[nIdx+1].X() * 3 +
                                fT2 * fU  * pPoints[nIdx+2].X() * 3 +
                                fT3 *       pPoints[nIdx+3].X());
    pPoints[nPos].Y() = (long) (fU3 *       pPoints[nIdx  ].Y() +
                                fT  * fU2 * pPoints[nIdx+1].Y() * 3 +
                                fT2 * fU  * pPoints[nIdx+2].Y() * 3 +
                                fT3 *       pPoints[nIdx+3].Y());
    nPos = nPos + nPosInc;
    nIdx = nIdx + nIdxInc;
    pPoints[nPos].X() = (long) (fU2 *       pPoints[nIdx  ].X() +
                                fT  * fU *  pPoints[nIdx+1].X() * 2 +
                                fT2 *       pPoints[nIdx+2].X());
    pPoints[nPos].Y() = (long) (fU2 *       pPoints[nIdx  ].Y() +
                                fT  * fU *  pPoints[nIdx+1].Y() * 2 +
                                fT2 *       pPoints[nIdx+2].Y());
    nPos = nPos + nPosInc;
    nIdx = nIdx + nIdxInc;
    pPoints[nPos].X() = (long) (fU * pPoints[nIdx  ].X() +
                                fT * pPoints[nIdx+1].X());
    pPoints[nPos].Y() = (long) (fU * pPoints[nIdx  ].Y() +
                                fT * pPoints[nIdx+1].Y());
}

/// Generate a Bézier arc
void XPolygon::GenBezArc(const Point& rCenter, long nRx, long nRy,
                         long nXHdl, long nYHdl, sal_uInt16 nStart, sal_uInt16 nEnd,
                         sal_uInt16 nQuad, sal_uInt16 nFirst)
{
    Point* pPoints = pImpXPolygon->pPointAry;
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
        pPoints[nFirst].X() += nRx; pPoints[nFirst+3].Y() += nRy;
    }
    else
    {
        pPoints[nFirst].Y() += nRy; pPoints[nFirst+3].X() += nRx;
    }
    pPoints[nFirst+1] = pPoints[nFirst];
    pPoints[nFirst+2] = pPoints[nFirst+3];

    if ( nQuad == 0 || nQuad == 2 )
    {
        pPoints[nFirst+1].Y() += nYHdl; pPoints[nFirst+2].X() += nXHdl;
    }
    else
    {
        pPoints[nFirst+1].X() += nXHdl; pPoints[nFirst+2].Y() += nYHdl;
    }
    if ( nStart > 0 )
        SubdivideBezier(nFirst, false, (double)nStart / 900);
    if ( nEnd < 900 )
        SubdivideBezier(nFirst, true, (double)(nEnd-nStart) / (900-nStart));
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
    Point*  pPoints = pImpXPolygon->pPointAry;
    Point   aDiff   = pPoints[nDrag] - pPoints[nCenter];
    double  fDiv    = CalcDistance(nCenter, nDrag);

    if ( fDiv )
    {
        double fRatio = CalcDistance(nCenter, nPnt) / fDiv;
        // keep the length if SMOOTH
        if ( GetFlags(nCenter) == PolyFlags::Smooth || !IsControl(nDrag) )
        {
            aDiff.X() = (long) (fRatio * aDiff.X());
            aDiff.Y() = (long) (fRatio * aDiff.Y());
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

    const Point& rCenter = pImpXPolygon->pPointAry[nCenter];
    Point&  rNext = pImpXPolygon->pPointAry[nNext];
    Point&  rPrev = pImpXPolygon->pPointAry[nPrev];
    Point   aDiff = rNext - rPrev;
    double  fNextLen = CalcDistance(nCenter, nNext) / fAbsLen;
    double  fPrevLen = CalcDistance(nCenter, nPrev) / fAbsLen;

    // same length for both sides if SYMMTR
    if ( GetFlags(nCenter) == PolyFlags::Symmetric )
    {
        fPrevLen = (fNextLen + fPrevLen) / 2;
        fNextLen = fPrevLen;
    }
    rNext.X() = rCenter.X() + (long) (fNextLen * aDiff.X());
    rNext.Y() = rCenter.Y() + (long) (fNextLen * aDiff.Y());
    rPrev.X() = rCenter.X() - (long) (fPrevLen * aDiff.X());
    rPrev.Y() = rCenter.Y() - (long) (fPrevLen * aDiff.Y());
}

/// convert four polygon points into a Bézier curve
void XPolygon::PointsToBezier(sal_uInt16 nFirst)
{
    double  nFullLength, nPart1Length, nPart2Length;
    double  fX0, fY0, fX1, fY1, fX2, fY2, fX3, fY3;
    double  fTx1, fTx2, fTy1, fTy2;
    double  fT1, fU1, fT2, fU2, fV;
    Point*  pPoints = pImpXPolygon->pPointAry;

    if ( nFirst > pImpXPolygon->nPoints - 4 || IsControl(nFirst) ||
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

    pPoints[nFirst+1] = Point((long) fX1, (long) fY1);
    pPoints[nFirst+2] = Point((long) fX2, (long) fY2);
    SetFlags(nFirst+1, PolyFlags::Control);
    SetFlags(nFirst+2, PolyFlags::Control);
}

/// scale in X- and/or Y-direction
void XPolygon::Scale(double fSx, double fSy)
{
    pImpXPolygon->CheckPointDelete();

    sal_uInt16 nPntCnt = pImpXPolygon->nPoints;

    for (sal_uInt16 i = 0; i < nPntCnt; i++)
    {
        Point& rPnt = pImpXPolygon->pPointAry[i];
        rPnt.X() = (long)(fSx * rPnt.X());
        rPnt.Y() = (long)(fSy * rPnt.Y());
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
    pImpXPolygon->CheckPointDelete();

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
    DBG_ASSERT(rDistortedRect.pImpXPolygon->nPoints >= 4,
               "Distort: rectangle to small");

    X1 = rDistortedRect[0].X();
    Y1 = rDistortedRect[0].Y();
    X2 = rDistortedRect[1].X();
    Y2 = rDistortedRect[1].Y();
    X3 = rDistortedRect[3].X();
    Y3 = rDistortedRect[3].Y();
    X4 = rDistortedRect[2].X();
    Y4 = rDistortedRect[2].Y();

    sal_uInt16 nPntCnt = pImpXPolygon->nPoints;

    for (sal_uInt16 i = 0; i < nPntCnt; i++)
    {
        double  fTx, fTy, fUx, fUy;
        Point& rPnt = pImpXPolygon->pPointAry[i];

        fTx = (double)(rPnt.X() - Xr) / Wr;
        fTy = (double)(rPnt.Y() - Yr) / Hr;
        fUx = 1.0 - fTx;
        fUy = 1.0 - fTy;

        rPnt.X() = (long) ( fUy * (fUx * X1 + fTx * X2) +
                            fTy * (fUx * X3 + fTx * X4) );
        rPnt.Y() = (long) ( fUx * (fUy * Y1 + fTy * Y3) +
                            fTx * (fUy * Y2 + fTy * Y4) );
    }
}

basegfx::B2DPolygon XPolygon::getB2DPolygon() const
{
    // #i74631# use tools Polygon class for conversion to not have the code doubled
    // here. This needs one more conversion but avoids different convertors in
    // the long run
    const tools::Polygon aSource(GetPointCount(), pImpXPolygon->pPointAry, pImpXPolygon->pFlagAry.get());

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
    pImpXPolygon->nPoints = nSize;

    for( sal_uInt16 i = 0; i < nSize;  i++ )
    {
        pImpXPolygon->pPointAry[i] = aSource[i];
        pImpXPolygon->pFlagAry[i] = aSource.GetFlags( i );
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
    return (sal_uInt16)(pImpXPolyPolygon->aXPolyList.size());
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
