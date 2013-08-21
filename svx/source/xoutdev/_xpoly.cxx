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
#include "xpolyimp.hxx"
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/numeric/ftools.hxx>

DBG_NAME(XPolygon);
DBG_NAME(XPolyPolygon);

ImpXPolygon::ImpXPolygon( sal_uInt16 nInitSize, sal_uInt16 _nResize )
{
    pPointAry               = NULL;
    pFlagAry                = NULL;
    bDeleteOldPoints        = sal_False;
    nSize                   = 0;
    nResize                 = _nResize;
    nPoints                 = 0;
    nRefCount               = 1;

    Resize( nInitSize );
}

ImpXPolygon::ImpXPolygon( const ImpXPolygon& rImpXPoly )
{
    ( (ImpXPolygon&) rImpXPoly ).CheckPointDelete();

    pPointAry               = NULL;
    pFlagAry                = NULL;
    bDeleteOldPoints        = sal_False;
    nSize                   = 0;
    ImpXPolygon::nResize    = rImpXPoly.nResize;
    nPoints                 = 0;
    nRefCount               = 1;

    Resize( rImpXPoly.nSize );

    // copy
    nPoints = rImpXPoly.nPoints;
    memcpy( pPointAry, rImpXPoly.pPointAry, nSize*sizeof( Point ) );
    memcpy( pFlagAry, rImpXPoly.pFlagAry, nSize );
}

ImpXPolygon::~ImpXPolygon()
{
    delete[] (char*) pPointAry;
    delete[] pFlagAry;
    if ( bDeleteOldPoints )
        delete[] (char*) pOldPointAry;
}

bool ImpXPolygon::operator==(const ImpXPolygon& rImpXPoly) const
{
    return nPoints==rImpXPoly.nPoints &&
           (nPoints==0 ||
            (memcmp(pPointAry,rImpXPoly.pPointAry,nPoints*sizeof(Point))==0 &&
             memcmp(pFlagAry,rImpXPoly.pFlagAry,nPoints)==0));
}

/** Change polygon size
 *
 * @param nNewSize      the new size of the polygon
 * @param bDeletePoints if FALSE, do not delete the point array directly but
 *                      wait for the next call before doing so. This prevents
 *                      errors with XPoly[n] = XPoly[0] where a resize might
 *                      destroy the right side point array too early.
 */
void ImpXPolygon::Resize( sal_uInt16 nNewSize, sal_Bool bDeletePoints )
{
    if( nNewSize == nSize )
        return;

    sal_uInt8*   pOldFlagAry  = pFlagAry;
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
    pPointAry = (Point*)new char[ nSize*sizeof( Point ) ];
    memset( pPointAry, 0, nSize*sizeof( Point ) );

    // create flag array
    pFlagAry = new sal_uInt8[ nSize ];
    memset( pFlagAry, 0, nSize );

    // copy if needed
    if( nOldSize )
    {
        if( nOldSize < nSize )
        {
            memcpy( pPointAry, pOldPointAry, nOldSize*sizeof( Point ) );
            memcpy( pFlagAry,  pOldFlagAry, nOldSize );
        }
        else
        {
            memcpy( pPointAry, pOldPointAry, nSize*sizeof( Point ) );
            memcpy( pFlagAry, pOldFlagAry, nSize );

            // adjust number of valid points
            if( nPoints > nSize )
                nPoints = nSize;
        }
        if ( bDeletePoints )    delete[] (char*) pOldPointAry;
        else                    bDeleteOldPoints = sal_True;
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

XPolygon::XPolygon( sal_uInt16 nSize, sal_uInt16 nResize )
{
    DBG_CTOR(XPolygon,NULL);
    pImpXPolygon = new ImpXPolygon( nSize, nResize );
}

XPolygon::XPolygon( const XPolygon& rXPoly )
{
    DBG_CTOR(XPolygon,NULL);
    pImpXPolygon = rXPoly.pImpXPolygon;
    pImpXPolygon->nRefCount++;
}

/// create a XPolygon out of a standard polygon
XPolygon::XPolygon( const Polygon& rPoly )
{
    DBG_CTOR(XPolygon,NULL);

    sal_uInt16 nSize = rPoly.GetSize();
    pImpXPolygon = new ImpXPolygon( nSize );
    pImpXPolygon->nPoints = nSize;

    for( sal_uInt16 i = 0; i < nSize;  i++ )
    {
        pImpXPolygon->pPointAry[i] = rPoly[i];
        pImpXPolygon->pFlagAry[i] = (sal_uInt8) rPoly.GetFlags( i );
    }
}

/// create a rectangle (also with rounded corners) as a Bézier polygon
XPolygon::XPolygon(const Rectangle& rRect, long nRx, long nRy)
{
    DBG_CTOR(XPolygon,NULL);
    pImpXPolygon = new ImpXPolygon(17);
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
            pImpXPolygon->pFlagAry[nPos  ] = (sal_uInt8) XPOLY_SMOOTH;
            pImpXPolygon->pFlagAry[nPos+3] = (sal_uInt8) XPOLY_SMOOTH;
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
                   sal_uInt16 nStartAngle, sal_uInt16 nEndAngle, sal_Bool bClose)
{
    DBG_CTOR(XPolygon,NULL);
    pImpXPolygon = new ImpXPolygon(17);

    nStartAngle %= 3600;
    if ( nEndAngle > 3600 ) nEndAngle %= 3600;
    bool bFull = (nStartAngle == 0 && nEndAngle == 3600);

    // factor for control points of the Bézier curve: 8/3 * (sin(45g) - 0.5)
    long    nXHdl = (long)(0.552284749 * nRx);
    long    nYHdl = (long)(0.552284749 * nRy);
    sal_uInt16  nPos = 0;
    sal_Bool    bLoopEnd = sal_False;

    do
    {
        sal_uInt16 nA1, nA2;
        sal_uInt16 nQuad = nStartAngle / 900;
        if ( nQuad == 4 ) nQuad = 0;
        bLoopEnd = CheckAngles(nStartAngle, nEndAngle, nA1, nA2);
        GenBezArc(rCenter, nRx, nRy, nXHdl, nYHdl, nA1, nA2, nQuad, nPos);
        nPos += 3;
        if ( !bLoopEnd )
            pImpXPolygon->pFlagAry[nPos] = (sal_uInt8) XPOLY_SMOOTH;

    } while ( !bLoopEnd );

    // if not a full circle than connect edges with center point if necessary
    if ( !bFull && bClose )
        pImpXPolygon->pPointAry[++nPos] = rCenter;

    if ( bFull )
    {
        pImpXPolygon->pFlagAry[0   ] = (sal_uInt8) XPOLY_SMOOTH;
        pImpXPolygon->pFlagAry[nPos] = (sal_uInt8) XPOLY_SMOOTH;
    }
    pImpXPolygon->nPoints = nPos + 1;
}

XPolygon::~XPolygon()
{
    DBG_DTOR(XPolygon,NULL);
    if( pImpXPolygon->nRefCount > 1 )
        pImpXPolygon->nRefCount--;
    else
        delete pImpXPolygon;
}

/// check reference counter and decouple if > 1
void XPolygon::CheckReference()
{
    if( pImpXPolygon->nRefCount > 1 )
    {
        pImpXPolygon->nRefCount--;
        pImpXPolygon = new ImpXPolygon( *pImpXPolygon );
    }
}

void XPolygon::SetPointCount( sal_uInt16 nPoints )
{
    pImpXPolygon->CheckPointDelete();
    CheckReference();

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

void XPolygon::Insert( sal_uInt16 nPos, const Point& rPt, XPolyFlags eFlags )
{
    CheckReference();
    if (nPos>pImpXPolygon->nPoints) nPos=pImpXPolygon->nPoints;
    pImpXPolygon->InsertSpace( nPos, 1 );
    pImpXPolygon->pPointAry[nPos] = rPt;
    pImpXPolygon->pFlagAry[nPos]  = (sal_uInt8)eFlags;
}

void XPolygon::Insert( sal_uInt16 nPos, const XPolygon& rXPoly )
{
    CheckReference();
    if (nPos>pImpXPolygon->nPoints) nPos=pImpXPolygon->nPoints;

    sal_uInt16 nPoints = rXPoly.GetPointCount();

    pImpXPolygon->InsertSpace( nPos, nPoints );

    memcpy( &(pImpXPolygon->pPointAry[nPos]),
            rXPoly.pImpXPolygon->pPointAry,
            nPoints*sizeof( Point ) );
    memcpy( &(pImpXPolygon->pFlagAry[nPos]),
            rXPoly.pImpXPolygon->pFlagAry,
            nPoints );
}

void XPolygon::Remove( sal_uInt16 nPos, sal_uInt16 nCount )
{
    CheckReference();
    pImpXPolygon->Remove( nPos, nCount );
}

void XPolygon::Move( long nHorzMove, long nVertMove )
{
    if ( !nHorzMove && !nVertMove )
        return;

    CheckReference();

    // move points
    sal_uInt16 nCount = pImpXPolygon->nPoints;
    for ( sal_uInt16 i = 0; i < nCount; i++ )
    {
        Point* pPt = &(pImpXPolygon->pPointAry[i]);
        pPt->X() += nHorzMove;
        pPt->Y() += nVertMove;
    }
}

Rectangle XPolygon::GetBoundRect() const
{
    pImpXPolygon->CheckPointDelete();
    Rectangle aRetval;

    if(pImpXPolygon->nPoints)
    {
        // #i37709#
        // For historical reasons the control points are not part of the
        // BoundRect. This makes it necessary to subdivide the polygon to
        // get a relatively correct BoundRect. Numerically, this is not
        // correct and never was.

        const basegfx::B2DRange aPolygonRange(basegfx::tools::getRange(getB2DPolygon()));
        aRetval = Rectangle(
            FRound(aPolygonRange.getMinX()), FRound(aPolygonRange.getMinY()),
            FRound(aPolygonRange.getMaxX()), FRound(aPolygonRange.getMaxY()));
    }

    return aRetval;
}

const Point& XPolygon::operator[]( sal_uInt16 nPos ) const
{
    DBG_ASSERT(nPos < pImpXPolygon->nPoints, "Ungueltiger Index bei const-Arrayzugriff auf XPolygon");

    pImpXPolygon->CheckPointDelete();
    return pImpXPolygon->pPointAry[nPos];
}

Point& XPolygon::operator[]( sal_uInt16 nPos )
{
    pImpXPolygon->CheckPointDelete();
    CheckReference();

    if( nPos >= pImpXPolygon->nSize )
    {
        DBG_ASSERT(pImpXPolygon->nResize, "Ungueltiger Index bei Arrayzugriff auf XPolygon");
        pImpXPolygon->Resize(nPos + 1, sal_False);
    }
    if( nPos >= pImpXPolygon->nPoints )
        pImpXPolygon->nPoints = nPos + 1;

    return pImpXPolygon->pPointAry[nPos];
}

XPolygon& XPolygon::operator=( const XPolygon& rXPoly )
{
    pImpXPolygon->CheckPointDelete();

    rXPoly.pImpXPolygon->nRefCount++;

    if( pImpXPolygon->nRefCount > 1 )
        pImpXPolygon->nRefCount--;
    else
        delete pImpXPolygon;

    pImpXPolygon = rXPoly.pImpXPolygon;
    return *this;
}

sal_Bool XPolygon::operator==( const XPolygon& rXPoly ) const
{
    pImpXPolygon->CheckPointDelete();
    if (rXPoly.pImpXPolygon==pImpXPolygon) return sal_True;
    return *rXPoly.pImpXPolygon == *pImpXPolygon;
}

sal_Bool XPolygon::operator!=( const XPolygon& rXPoly ) const
{
    pImpXPolygon->CheckPointDelete();
    if (rXPoly.pImpXPolygon==pImpXPolygon) return sal_False;
    return *rXPoly.pImpXPolygon != *pImpXPolygon;
}

/// get the flags for the point at the given position
XPolyFlags XPolygon::GetFlags( sal_uInt16 nPos ) const
{
    pImpXPolygon->CheckPointDelete();
    return (XPolyFlags) pImpXPolygon->pFlagAry[nPos];
}

/// set the flags for the point at the given position
void XPolygon::SetFlags( sal_uInt16 nPos, XPolyFlags eFlags )
{
    pImpXPolygon->CheckPointDelete();
    CheckReference();
    pImpXPolygon->pFlagAry[nPos] = (sal_uInt8) eFlags;
}

/// short path to read the CONTROL flag directly (TODO: better explain what the sense behind this flag is!)
sal_Bool XPolygon::IsControl(sal_uInt16 nPos) const
{
    return ( (XPolyFlags) pImpXPolygon->pFlagAry[nPos] == XPOLY_CONTROL );
}

/// short path to read the SMOOTH and SYMMTR flag directly (TODO: better explain what the sense behind these flags is!)
sal_Bool XPolygon::IsSmooth(sal_uInt16 nPos) const
{
    XPolyFlags eFlag = (XPolyFlags) pImpXPolygon->pFlagAry[nPos];
    return ( eFlag == XPOLY_SMOOTH || eFlag == XPOLY_SYMMTR );
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

void XPolygon::SubdivideBezier(sal_uInt16 nPos, sal_Bool bCalcFirst, double fT)
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
        SubdivideBezier(nFirst, sal_False, (double)nStart / 900);
    if ( nEnd < 900 )
        SubdivideBezier(nFirst, sal_True, (double)(nEnd-nStart) / (900-nStart));
    SetFlags(nFirst+1, XPOLY_CONTROL);
    SetFlags(nFirst+2, XPOLY_CONTROL);
}

sal_Bool XPolygon::CheckAngles(sal_uInt16& nStart, sal_uInt16 nEnd, sal_uInt16& nA1, sal_uInt16& nA2)
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
    CheckReference();

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
        if ( GetFlags(nCenter) == XPOLY_SMOOTH || !IsControl(nDrag) )
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
    CheckReference();

    double fAbsLen = CalcDistance(nNext, nPrev);

    if ( fAbsLen )
    {
        const Point& rCenter = pImpXPolygon->pPointAry[nCenter];
        Point&  rNext = pImpXPolygon->pPointAry[nNext];
        Point&  rPrev = pImpXPolygon->pPointAry[nPrev];
        Point   aDiff = rNext - rPrev;
        double  fNextLen = CalcDistance(nCenter, nNext) / fAbsLen;
        double  fPrevLen = CalcDistance(nCenter, nPrev) / fAbsLen;

        // same length for both sides if SYMMTR
        if ( GetFlags(nCenter) == XPOLY_SYMMTR )
        {
            fPrevLen = (fNextLen + fPrevLen) / 2;
            fNextLen = fPrevLen;
        }
        rNext.X() = rCenter.X() + (long) (fNextLen * aDiff.X());
        rNext.Y() = rCenter.Y() + (long) (fNextLen * aDiff.Y());
        rPrev.X() = rCenter.X() - (long) (fPrevLen * aDiff.X());
        rPrev.Y() = rCenter.Y() - (long) (fPrevLen * aDiff.Y());
    }
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

    CheckReference();

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
    SetFlags(nFirst+1, XPOLY_CONTROL);
    SetFlags(nFirst+2, XPOLY_CONTROL);
}

/// scale in X- and/or Y-direction
void XPolygon::Scale(double fSx, double fSy)
{
    pImpXPolygon->CheckPointDelete();
    CheckReference();

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
void XPolygon::Distort(const Rectangle& rRefRect,
                       const XPolygon& rDistortedRect)
{
    pImpXPolygon->CheckPointDelete();
    CheckReference();

    long    Xr, Wr;
    long    Yr, Hr;

    Xr = rRefRect.Left();
    Yr = rRefRect.Top();
    Wr = rRefRect.GetWidth();
    Hr = rRefRect.GetHeight();

    if ( Wr && Hr )
    {
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
}

basegfx::B2DPolygon XPolygon::getB2DPolygon() const
{
    // #i74631# use tools Polygon class for conversion to not have the code doubled
    // here. This needs one more conversion but avoids different convertors in
    // the long run
    DBG_ASSERT(pImpXPolygon != 0, "XPolygon::getB2DPolygon(): XPolygon has no implementation incarnated (!)");
    const Polygon aSource(GetPointCount(), pImpXPolygon->pPointAry, pImpXPolygon->pFlagAry);

    return aSource.getB2DPolygon();
}

XPolygon::XPolygon(const basegfx::B2DPolygon& rPolygon)
{
    // #i74631# use tools Polygon class for conversion to not have the code doubled
    // here. This needs one more conversion but avoids different convertors in
    // the long run
    DBG_CTOR(XPolygon,NULL);

    const Polygon aSource(rPolygon);
    sal_uInt16 nSize = aSource.GetSize();
    pImpXPolygon = new ImpXPolygon( nSize );
    pImpXPolygon->nPoints = nSize;

    for( sal_uInt16 i = 0; i < nSize;  i++ )
    {
        pImpXPolygon->pPointAry[i] = aSource[i];
        pImpXPolygon->pFlagAry[i] = (sal_uInt8) aSource.GetFlags( i );
    }
}

// XPolyPolygon

ImpXPolyPolygon::ImpXPolyPolygon( const ImpXPolyPolygon& rImpXPolyPoly ) :
                     aXPolyList( rImpXPolyPoly.aXPolyList )
{
    nRefCount = 1;

    // duplicate elements
    for ( size_t i = 0, n = aXPolyList.size(); i < n; ++i )
        aXPolyList[ i ] = new XPolygon( *aXPolyList[ i ] );
}

ImpXPolyPolygon::~ImpXPolyPolygon()
{
    for ( size_t i = 0, n = aXPolyList.size(); i < n; ++i )
        delete aXPolyList[ i ];
    aXPolyList.clear();
}

bool ImpXPolyPolygon::operator==(const ImpXPolyPolygon& rImpXPolyPoly) const
{
    size_t nAnz = aXPolyList.size();
    const XPolygonList& rCmpList = rImpXPolyPoly.aXPolyList;
    if ( nAnz != rCmpList.size() ) return false;
    bool bEq=true;
    for ( size_t i = nAnz; i > 0 && bEq; )
    {
        i--;
        bEq = ( *aXPolyList[ i ] == *rCmpList[ i ] );
    }
    return bEq;
}

XPolyPolygon::XPolyPolygon( sal_uInt16 /*nInitSize*/, sal_uInt16 /*nResize*/ )
{
    DBG_CTOR(XPolyPolygon,NULL);
    pImpXPolyPolygon = new ImpXPolyPolygon();
}

XPolyPolygon::XPolyPolygon( const XPolyPolygon& rXPolyPoly )
{
    DBG_CTOR(XPolyPolygon,NULL);
    pImpXPolyPolygon = rXPolyPoly.pImpXPolyPolygon;
    pImpXPolyPolygon->nRefCount++;
}

XPolyPolygon::~XPolyPolygon()
{
    DBG_DTOR(XPolyPolygon,NULL);
    if( pImpXPolyPolygon->nRefCount > 1 )
        pImpXPolyPolygon->nRefCount--;
    else
        delete pImpXPolyPolygon;
}

/// check reference counter and decouple if > 1
void XPolyPolygon::CheckReference()
{
    if( pImpXPolyPolygon->nRefCount > 1 )
    {
        pImpXPolyPolygon->nRefCount--;
        pImpXPolyPolygon = new ImpXPolyPolygon( *pImpXPolyPolygon );
    }
}

void XPolyPolygon::Insert( const XPolygon& rXPoly, sal_uInt16 nPos )
{
    CheckReference();
    XPolygon* pXPoly = new XPolygon( rXPoly );
    if ( nPos < pImpXPolyPolygon->aXPolyList.size() )
    {
        XPolygonList::iterator it = pImpXPolyPolygon->aXPolyList.begin();
        ::std::advance( it, nPos );
        pImpXPolyPolygon->aXPolyList.insert( it, pXPoly );
    }
    else
        pImpXPolyPolygon->aXPolyList.push_back( pXPoly );
}

/// insert all XPolygons of a XPolyPolygon
void XPolyPolygon::Insert( const XPolyPolygon& rXPolyPoly, sal_uInt16 nPos )
{
    CheckReference();

    for ( size_t i = 0; i < rXPolyPoly.Count(); i++)
    {
        XPolygon* pXPoly = new XPolygon( rXPolyPoly[i] );

        if ( nPos < pImpXPolyPolygon->aXPolyList.size() )
        {
            XPolygonList::iterator it = pImpXPolyPolygon->aXPolyList.begin();
            ::std::advance( it, nPos );
            pImpXPolyPolygon->aXPolyList.insert( it, pXPoly );
            nPos++;
        }
        else
            pImpXPolyPolygon->aXPolyList.push_back( pXPoly );
    }
}

XPolygon XPolyPolygon::Remove( sal_uInt16 nPos )
{
    CheckReference();
    XPolygonList::iterator it = pImpXPolyPolygon->aXPolyList.begin();
    ::std::advance( it, nPos );
    XPolygon* pTmpXPoly = *it;
    pImpXPolyPolygon->aXPolyList.erase( it );
    XPolygon  aXPoly( *pTmpXPoly );
    delete pTmpXPoly;
    return aXPoly;
}

const XPolygon& XPolyPolygon::GetObject( sal_uInt16 nPos ) const
{
    return *(pImpXPolyPolygon->aXPolyList[ nPos ]);
}

void XPolyPolygon::Clear()
{
    if ( pImpXPolyPolygon->nRefCount > 1 )
    {
        pImpXPolyPolygon->nRefCount--;
        pImpXPolyPolygon = new ImpXPolyPolygon();
    }
    else
    {
        for( size_t i = 0, n = pImpXPolyPolygon->aXPolyList.size(); i < n; ++i )
            delete pImpXPolyPolygon->aXPolyList[ i ];
        pImpXPolyPolygon->aXPolyList.clear();
    }
}

sal_uInt16 XPolyPolygon::Count() const
{
    return (sal_uInt16)(pImpXPolyPolygon->aXPolyList.size());
}

Rectangle XPolyPolygon::GetBoundRect() const
{
    size_t nXPoly = pImpXPolyPolygon->aXPolyList.size();
    Rectangle aRect;

    for ( size_t n = 0; n < nXPoly; n++ )
    {
        const XPolygon* pXPoly = pImpXPolyPolygon->aXPolyList[ n ];
        aRect.Union( pXPoly->GetBoundRect() );
    }

    return aRect;
}

XPolygon& XPolyPolygon::operator[]( sal_uInt16 nPos )
{
    CheckReference();
    return *( pImpXPolyPolygon->aXPolyList[ nPos ] );
}

XPolyPolygon& XPolyPolygon::operator=( const XPolyPolygon& rXPolyPoly )
{
    rXPolyPoly.pImpXPolyPolygon->nRefCount++;

    if( pImpXPolyPolygon->nRefCount > 1 )
        pImpXPolyPolygon->nRefCount--;
    else
        delete pImpXPolyPolygon;

    pImpXPolyPolygon = rXPolyPoly.pImpXPolyPolygon;
    return *this;
}

sal_Bool XPolyPolygon::operator==( const XPolyPolygon& rXPolyPoly ) const
{
    if (pImpXPolyPolygon==rXPolyPoly.pImpXPolyPolygon) return sal_True;
    return *pImpXPolyPolygon == *rXPolyPoly.pImpXPolyPolygon;
}

sal_Bool XPolyPolygon::operator!=( const XPolyPolygon& rXPolyPoly ) const
{
    if (pImpXPolyPolygon==rXPolyPoly.pImpXPolyPolygon) return sal_False;
    return *pImpXPolyPolygon != *rXPolyPoly.pImpXPolyPolygon;
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
void XPolyPolygon::Distort(const Rectangle& rRefRect,
                           const XPolygon& rDistortedRect)
{
    CheckReference();

    for (size_t i = 0; i < Count(); i++)
        pImpXPolyPolygon->aXPolyList[ i ]->Distort(rRefRect, rDistortedRect);
}

basegfx::B2DPolyPolygon XPolyPolygon::getB2DPolyPolygon() const
{
    basegfx::B2DPolyPolygon aRetval;

    for(sal_uInt16 a(0L); a < Count(); a++)
    {
        const XPolygon& rPoly = (*this)[a];
        aRetval.append(rPoly.getB2DPolygon());
    }

    return aRetval;
}

XPolyPolygon::XPolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    DBG_CTOR(XPolyPolygon,NULL);
    pImpXPolyPolygon = new ImpXPolyPolygon();

    for(sal_uInt32 a(0L); a < rPolyPolygon.count(); a++)
    {
        basegfx::B2DPolygon aCandidate = rPolyPolygon.getB2DPolygon(a);
        XPolygon aNewPoly(aCandidate);
        Insert(aNewPoly);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
