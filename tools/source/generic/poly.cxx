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
#include <osl/diagnose.h>
#include <sal/log.hxx>
#include <tools/bigint.hxx>
#include <tools/debug.hxx>
#include <tools/helpers.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <tools/gen.hxx>
#include <poly.h>
#include <tools/line.hxx>
#include <tools/vector2d.hxx>
#include <tools/poly.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>

#include <memory>
#include <vector>
#include <iterator>
#include <algorithm>
#include <cstring>
#include <limits.h>
#include <cmath>

#define EDGE_LEFT       1
#define EDGE_TOP        2
#define EDGE_RIGHT      4
#define EDGE_BOTTOM     8
#define EDGE_HORZ       (EDGE_RIGHT | EDGE_LEFT)
#define EDGE_VERT       (EDGE_TOP | EDGE_BOTTOM)
#define SMALL_DVALUE    0.0000001
#define FSQRT2          1.4142135623730950488016887242097

static ImplPolygonData aStaticImplPolygon =
{
    nullptr, nullptr, 0, 0
};

ImplPolygon::ImplPolygon( sal_uInt16 nInitSize, bool bFlags  )
{
    if ( nInitSize )
    {
        const std::size_t nSz(static_cast<std::size_t>(nInitSize)*sizeof(Point));
        mpPointAry = reinterpret_cast<Point*>(new char[nSz]);
        memset( mpPointAry, 0, nSz );
    }
    else
        mpPointAry = nullptr;

    if( bFlags )
    {
        mpFlagAry = new sal_uInt8[ nInitSize ];
        memset( mpFlagAry, 0, nInitSize );
    }
    else
        mpFlagAry = nullptr;

    mnRefCount = 1;
    mnPoints = nInitSize;
}

ImplPolygon::ImplPolygon( const ImplPolygon& rImpPoly )
{
    if ( rImpPoly.mnPoints )
    {
        const std::size_t nSz(static_cast<std::size_t>(rImpPoly.mnPoints)*sizeof(Point));
        mpPointAry = reinterpret_cast<Point*>(new char[nSz]);
        memcpy( mpPointAry, rImpPoly.mpPointAry, nSz );

        if( rImpPoly.mpFlagAry )
        {
            mpFlagAry = new sal_uInt8[ rImpPoly.mnPoints ];
            memcpy( mpFlagAry, rImpPoly.mpFlagAry, rImpPoly.mnPoints );
        }
        else
            mpFlagAry = nullptr;
    }
    else
    {
        mpPointAry = nullptr;
        mpFlagAry = nullptr;
    }

    mnRefCount = 1;
    mnPoints   = rImpPoly.mnPoints;
}

ImplPolygon::ImplPolygon( sal_uInt16 nInitSize, const Point* pInitAry, const sal_uInt8* pInitFlags )
{
    if ( nInitSize )
    {
        const std::size_t nSz(static_cast<std::size_t>(nInitSize)*sizeof(Point));
        mpPointAry = reinterpret_cast<Point*>(new char[nSz]);
        memcpy( mpPointAry, pInitAry, nSz );

        if( pInitFlags )
        {
            mpFlagAry = new sal_uInt8[ nInitSize ];
            memcpy( mpFlagAry, pInitFlags, nInitSize );
        }
        else
            mpFlagAry = nullptr;
    }
    else
    {
        mpPointAry = nullptr;
        mpFlagAry  = nullptr;
    }

    mnRefCount = 1;
    mnPoints   = nInitSize;
}

ImplPolygon::~ImplPolygon()
{
    if ( mpPointAry )
    {
        delete[] reinterpret_cast<char*>(mpPointAry);
    }

    if( mpFlagAry )
        delete[] mpFlagAry;
}

void ImplPolygon::ImplSetSize( sal_uInt16 nNewSize, bool bResize )
{
    if( mnPoints == nNewSize )
        return;

    Point* pNewAry;

    if ( nNewSize )
    {
        const std::size_t nNewSz(static_cast<std::size_t>(nNewSize)*sizeof(Point));
        pNewAry = reinterpret_cast<Point*>(new char[nNewSz]);

        if ( bResize )
        {
            // Copy the old points
            if ( mnPoints < nNewSize )
            {
                // New points initialized to zero
                const std::size_t nOldSz(static_cast<std::size_t>(mnPoints)*sizeof(Point));
                memset( pNewAry+mnPoints, 0, nNewSz-nOldSz );
                if ( mpPointAry )
                    memcpy( pNewAry, mpPointAry, nOldSz );
            }
            else
            {
                if ( mpPointAry )
                    memcpy( pNewAry, mpPointAry, nNewSz );
            }
        }
    }
    else
        pNewAry = nullptr;

    if ( mpPointAry )
        delete[] reinterpret_cast<char*>(mpPointAry);

    // ggf. FlagArray beruecksichtigen
    if( mpFlagAry )
    {
        sal_uInt8* pNewFlagAry;

        if( nNewSize )
        {
            pNewFlagAry = new sal_uInt8[ nNewSize ];

            if( bResize )
            {
                // copy the old flags
                if ( mnPoints < nNewSize )
                {
                    // initialize new flags to zero
                    memset( pNewFlagAry+mnPoints, 0, nNewSize-mnPoints );
                    memcpy( pNewFlagAry, mpFlagAry, mnPoints );
                }
                else
                    memcpy( pNewFlagAry, mpFlagAry, nNewSize );
            }
        }
        else
            pNewFlagAry = nullptr;

        delete[] mpFlagAry;
        mpFlagAry  = pNewFlagAry;
    }

    mpPointAry = pNewAry;
    mnPoints   = nNewSize;
}

void ImplPolygon::ImplSplit( sal_uInt16 nPos, sal_uInt16 nSpace, ImplPolygon* pInitPoly )
{
    //Can't fit this in :-(, throw ?
    if (mnPoints + nSpace > USHRT_MAX)
        return;

    const sal_uInt16    nNewSize = mnPoints + nSpace;
    const std::size_t   nSpaceSize = static_cast<std::size_t>(nSpace) * sizeof(Point);

    if( nPos >= mnPoints )
    {
        // Append at the back
        nPos = mnPoints;
        ImplSetSize( nNewSize );

        if( pInitPoly )
        {
            memcpy( mpPointAry + nPos, pInitPoly->mpPointAry, nSpaceSize );

            if( pInitPoly->mpFlagAry )
                memcpy( mpFlagAry + nPos, pInitPoly->mpFlagAry, nSpace );
        }
    }
    else
    {
        const sal_uInt16    nSecPos = nPos + nSpace;
        const sal_uInt16    nRest = mnPoints - nPos;

        Point* pNewAry = reinterpret_cast<Point*>(new char[ static_cast<std::size_t>(nNewSize) * sizeof(Point) ]);

        memcpy( pNewAry, mpPointAry, nPos * sizeof( Point ) );

        if( pInitPoly )
            memcpy( pNewAry + nPos, pInitPoly->mpPointAry, nSpaceSize );
        else
            memset( pNewAry + nPos, 0, nSpaceSize );

        memcpy( pNewAry + nSecPos, mpPointAry + nPos, nRest * sizeof( Point ) );
        delete[] reinterpret_cast<char*>(mpPointAry);

        // consider FlagArray
        if( mpFlagAry )
        {
            sal_uInt8* pNewFlagAry = new sal_uInt8[ nNewSize ];

            memcpy( pNewFlagAry, mpFlagAry, nPos );

            if( pInitPoly && pInitPoly->mpFlagAry )
                memcpy( pNewFlagAry + nPos, pInitPoly->mpFlagAry, nSpace );
            else
                memset( pNewFlagAry + nPos, 0, nSpace );

            memcpy( pNewFlagAry + nSecPos, mpFlagAry + nPos, nRest );
            delete[] mpFlagAry;
            mpFlagAry = pNewFlagAry;
        }

        mpPointAry = pNewAry;
        mnPoints   = nNewSize;
    }
}

void ImplPolygon::ImplCreateFlagArray()
{
    if( !mpFlagAry )
    {
        mpFlagAry = new sal_uInt8[ mnPoints ];
        memset( mpFlagAry, 0, mnPoints );
    }
}

inline double ImplGetParameter( const Point& rCenter, const Point& rPt, double fWR, double fHR )
{
    const long nDX = rPt.X() - rCenter.X();
    double fAngle = atan2( -rPt.Y() + rCenter.Y(), ( ( nDX == 0L ) ? 0.000000001 : nDX ) );

    return atan2(fWR*sin(fAngle), fHR*cos(fAngle));
}

class ImplPointFilter
{
public:
    virtual void LastPoint() = 0;
    virtual void Input( const Point& rPoint ) = 0;

protected:
    ~ImplPointFilter() {}
};

class ImplPolygonPointFilter : public ImplPointFilter
{
    std::unique_ptr<ImplPolygon> mxPoly;
    sal_uInt16      mnSize;
public:
    explicit ImplPolygonPointFilter(sal_uInt16 nDestSize)
        : mxPoly(new ImplPolygon(nDestSize))
        , mnSize(0)
    {
    }

    virtual ~ImplPolygonPointFilter()
    {
    }

    virtual void    LastPoint() override;
    virtual void    Input( const Point& rPoint ) override;

    ImplPolygon*    release() { return mxPoly.release(); }
};

void ImplPolygonPointFilter::Input( const Point& rPoint )
{
    if ( !mnSize || (rPoint != mxPoly->mpPointAry[mnSize-1]) )
    {
        mnSize++;
        if ( mnSize > mxPoly->mnPoints )
            mxPoly->ImplSetSize( mnSize );
        mxPoly->mpPointAry[mnSize-1] = rPoint;
    }
}

void ImplPolygonPointFilter::LastPoint()
{
    if ( mnSize < mxPoly->mnPoints )
        mxPoly->ImplSetSize( mnSize );
};

class ImplEdgePointFilter : public ImplPointFilter
{
    Point               maFirstPoint;
    Point               maLastPoint;
    ImplPointFilter&    mrNextFilter;
    const long          mnLow;
    const long          mnHigh;
    const int           mnEdge;
    int                 mnLastOutside;
    bool                mbFirst;

public:
                        ImplEdgePointFilter( int nEdge, long nLow, long nHigh,
                                             ImplPointFilter& rNextFilter ) :
                            mrNextFilter( rNextFilter ),
                            mnLow( nLow ),
                            mnHigh( nHigh ),
                            mnEdge( nEdge ),
                            mnLastOutside( 0 ),
                            mbFirst( true )
                        {
                        }

    virtual             ~ImplEdgePointFilter() {}

    Point               EdgeSection( const Point& rPoint, int nEdge ) const;
    int                 VisibleSide( const Point& rPoint ) const;
    bool                IsPolygon() const
                            { return maFirstPoint == maLastPoint; }

    virtual void        Input( const Point& rPoint ) override;
    virtual void        LastPoint() override;
};

inline int ImplEdgePointFilter::VisibleSide( const Point& rPoint ) const
{
    if ( mnEdge & EDGE_HORZ )
    {
        return rPoint.X() < mnLow ? EDGE_LEFT :
                                     rPoint.X() > mnHigh ? EDGE_RIGHT : 0;
    }
    else
    {
        return rPoint.Y() < mnLow ? EDGE_TOP :
                                     rPoint.Y() > mnHigh ? EDGE_BOTTOM : 0;
    }
}

Point ImplEdgePointFilter::EdgeSection( const Point& rPoint, int nEdge ) const
{
    long lx = maLastPoint.X();
    long ly = maLastPoint.Y();
    long md = rPoint.X() - lx;
    long mn = rPoint.Y() - ly;
    long nNewX;
    long nNewY;

    if ( nEdge & EDGE_VERT )
    {
        nNewY = (nEdge == EDGE_TOP) ? mnLow : mnHigh;
        long dy = nNewY - ly;
        if ( !md )
            nNewX = lx;
        else if ( (LONG_MAX / std::abs(md)) >= std::abs(dy) )
            nNewX = (dy * md) / mn + lx;
        else
        {
            BigInt ady = dy;
            ady *= md;
            if( ady.IsNeg() )
                if( mn < 0 )
                    ady += mn/2;
                else
                    ady -= (mn-1)/2;
            else
                if( mn < 0 )
                    ady -= (mn+1)/2;
                else
                    ady += mn/2;
            ady /= mn;
            nNewX = (long)ady + lx;
        }
    }
    else
    {
        nNewX = (nEdge == EDGE_LEFT) ? mnLow : mnHigh;
        long dx = nNewX - lx;
        if ( !mn )
            nNewY = ly;
        else if ( (LONG_MAX / std::abs(mn)) >= std::abs(dx) )
            nNewY = (dx * mn) / md + ly;
        else
        {
            BigInt adx = dx;
            adx *= mn;
            if( adx.IsNeg() )
                if( md < 0 )
                    adx += md/2;
                else
                    adx -= (md-1)/2;
            else
                if( md < 0 )
                    adx -= (md+1)/2;
                else
                    adx += md/2;
            adx /= md;
            nNewY = (long)adx + ly;
        }
    }

    return Point( nNewX, nNewY );
}

void ImplEdgePointFilter::Input( const Point& rPoint )
{
    int nOutside = VisibleSide( rPoint );

    if ( mbFirst )
    {
        maFirstPoint = rPoint;
        mbFirst      = false;
        if ( !nOutside )
            mrNextFilter.Input( rPoint );
    }
    else if ( rPoint == maLastPoint )
        return;
    else if ( !nOutside )
    {
        if ( mnLastOutside )
            mrNextFilter.Input( EdgeSection( rPoint, mnLastOutside ) );
        mrNextFilter.Input( rPoint );
    }
    else if ( !mnLastOutside )
        mrNextFilter.Input( EdgeSection( rPoint, nOutside ) );
    else if ( nOutside != mnLastOutside )
    {
        mrNextFilter.Input( EdgeSection( rPoint, mnLastOutside ) );
        mrNextFilter.Input( EdgeSection( rPoint, nOutside ) );
    }

    maLastPoint    = rPoint;
    mnLastOutside  = nOutside;
}

void ImplEdgePointFilter::LastPoint()
{
    if ( !mbFirst )
    {
        int nOutside = VisibleSide( maFirstPoint );

        if ( nOutside != mnLastOutside )
            Input( maFirstPoint );
        mrNextFilter.LastPoint();
    }
}

namespace tools
{

tools::Polygon Polygon::SubdivideBezier( const tools::Polygon& rPoly )
{
    tools::Polygon aPoly;

    // #100127# Use adaptive subdivide instead of fixed 25 segments
    rPoly.AdaptiveSubdivide( aPoly );

    return aPoly;
}


inline void Polygon::ImplMakeUnique()
{
    // copy references if any exist
    if ( mpImplPolygon->mnRefCount != 1 )
    {
        if ( mpImplPolygon->mnRefCount )
            mpImplPolygon->mnRefCount--;
        mpImplPolygon = new ImplPolygon( *mpImplPolygon );
    }
}

Polygon::Polygon()
{
    mpImplPolygon = static_cast<ImplPolygon*>(&aStaticImplPolygon);
}

Polygon::Polygon( sal_uInt16 nSize )
{

    if ( nSize )
        mpImplPolygon = new ImplPolygon( nSize );
    else
        mpImplPolygon = static_cast<ImplPolygon*>(&aStaticImplPolygon);
}

Polygon::Polygon( sal_uInt16 nPoints, const Point* pPtAry, const sal_uInt8* pFlagAry )
{

    if( nPoints )
        mpImplPolygon = new ImplPolygon( nPoints, pPtAry, pFlagAry );
    else
        mpImplPolygon = static_cast<ImplPolygon*>(&aStaticImplPolygon);
}

Polygon::Polygon( const tools::Polygon& rPoly )
{
    DBG_ASSERT( rPoly.mpImplPolygon->mnRefCount < (SAL_MAX_UINT32-1), "Polygon: RefCount overflow" );

    mpImplPolygon = rPoly.mpImplPolygon;
    if ( mpImplPolygon->mnRefCount )
        mpImplPolygon->mnRefCount++;
}

Polygon::Polygon( const Rectangle& rRect )
{

    if ( rRect.IsEmpty() )
        mpImplPolygon = static_cast<ImplPolygon*>(&aStaticImplPolygon);
    else
    {
        mpImplPolygon = new ImplPolygon( 5 );
        mpImplPolygon->mpPointAry[0] = rRect.TopLeft();
        mpImplPolygon->mpPointAry[1] = rRect.TopRight();
        mpImplPolygon->mpPointAry[2] = rRect.BottomRight();
        mpImplPolygon->mpPointAry[3] = rRect.BottomLeft();
        mpImplPolygon->mpPointAry[4] = rRect.TopLeft();
    }
}

Polygon::Polygon( const Rectangle& rRect, sal_uInt32 nHorzRound, sal_uInt32 nVertRound )
{
    if ( rRect.IsEmpty() )
        mpImplPolygon = static_cast<ImplPolygon*>(&aStaticImplPolygon);
    else
    {
        Rectangle aRect( rRect );
        aRect.Justify();            // SJ: i9140

        nHorzRound = std::min( nHorzRound, static_cast<sal_uInt32>(labs( aRect.GetWidth() >> 1 )) );
        nVertRound = std::min( nVertRound, static_cast<sal_uInt32>(labs( aRect.GetHeight() >> 1 )) );

        if( !nHorzRound && !nVertRound )
        {
            mpImplPolygon = new ImplPolygon( 5 );
            mpImplPolygon->mpPointAry[0] = aRect.TopLeft();
            mpImplPolygon->mpPointAry[1] = aRect.TopRight();
            mpImplPolygon->mpPointAry[2] = aRect.BottomRight();
            mpImplPolygon->mpPointAry[3] = aRect.BottomLeft();
            mpImplPolygon->mpPointAry[4] = aRect.TopLeft();
        }
        else
        {
            const Point     aTL( aRect.Left() + nHorzRound, aRect.Top() + nVertRound );
            const Point     aTR( aRect.Right() - nHorzRound, aRect.Top() + nVertRound );
            const Point     aBR( aRect.Right() - nHorzRound, aRect.Bottom() - nVertRound );
            const Point     aBL( aRect.Left() + nHorzRound, aRect.Bottom() - nVertRound );
            tools::Polygon* pEllipsePoly = new tools::Polygon( Point(), nHorzRound, nVertRound );
            sal_uInt16 i, nEnd, nSize4 = pEllipsePoly->GetSize() >> 2;

            mpImplPolygon = new ImplPolygon( pEllipsePoly->GetSize() + 1 );

            const Point* pSrcAry = pEllipsePoly->GetConstPointAry();
            Point* pDstAry = mpImplPolygon->mpPointAry;

            for( i = 0, nEnd = nSize4; i < nEnd; i++ )
                ( pDstAry[ i ] = pSrcAry[ i ] ) += aTR;

            for( nEnd = nEnd + nSize4; i < nEnd; i++ )
                ( pDstAry[ i ] = pSrcAry[ i ] ) += aTL;

            for( nEnd = nEnd + nSize4; i < nEnd; i++ )
                ( pDstAry[ i ] = pSrcAry[ i ] ) += aBL;

            for( nEnd = nEnd + nSize4; i < nEnd; i++ )
                ( pDstAry[ i ] = pSrcAry[ i ] ) += aBR;

            pDstAry[ nEnd ] = pDstAry[ 0 ];
            delete pEllipsePoly;
        }
    }
}

Polygon::Polygon( const Point& rCenter, long nRadX, long nRadY, sal_uInt16 nPoints )
{
    if( nRadX && nRadY )
    {
        // Compute default (depends on size)
        if( !nPoints )
        {
            nPoints = (sal_uInt16) MinMax(
                ( F_PI * ( 1.5 * ( nRadX + nRadY ) -
                           sqrt( (double) labs( nRadX * nRadY ) ) ) ),
                32, 256 );

            if( ( nRadX > 32 ) && ( nRadY > 32 ) && ( nRadX + nRadY ) < 8192 )
                nPoints >>= 1;
        }

        // Ceil number of points until divisible by four
        mpImplPolygon = new ImplPolygon( nPoints = (nPoints + 3) & ~3 );

        sal_uInt16 i;
        sal_uInt16 nPoints2 = nPoints >> 1;
        sal_uInt16 nPoints4 = nPoints >> 2;
        double nAngle;
        double nAngleStep = F_PI2 / ( nPoints4 - 1 );

        for( i=0, nAngle = 0.0; i < nPoints4; i++, nAngle += nAngleStep )
        {
            long nX = FRound( nRadX * cos( nAngle ) );
            long nY = FRound( -nRadY * sin( nAngle ) );

            Point* pPt = &(mpImplPolygon->mpPointAry[i]);
            pPt->X() =  nX + rCenter.X();
            pPt->Y() =  nY + rCenter.Y();
            pPt = &(mpImplPolygon->mpPointAry[nPoints2-i-1]);
            pPt->X() = -nX + rCenter.X();
            pPt->Y() =  nY + rCenter.Y();
            pPt = &(mpImplPolygon->mpPointAry[i+nPoints2]);
            pPt->X() = -nX + rCenter.X();
            pPt->Y() = -nY + rCenter.Y();
            pPt = &(mpImplPolygon->mpPointAry[nPoints-i-1]);
            pPt->X() =  nX + rCenter.X();
            pPt->Y() = -nY + rCenter.Y();
        }
    }
    else
        mpImplPolygon = static_cast<ImplPolygon*>(&aStaticImplPolygon);
}

Polygon::Polygon( const Rectangle& rBound, const Point& rStart, const Point& rEnd,
                  PolyStyle eStyle, bool bFullCircle )
{
    const long  nWidth = rBound.GetWidth();
    const long  nHeight = rBound.GetHeight();

    if( ( nWidth > 1 ) && ( nHeight > 1 ) )
    {
        const Point aCenter( rBound.Center() );
        const long  nRadX = aCenter.X() - rBound.Left();
        const long  nRadY = aCenter.Y() - rBound.Top();
        sal_uInt16  nPoints;

        nPoints = (sal_uInt16) MinMax(
            ( F_PI * ( 1.5 * ( nRadX + nRadY ) -
                       sqrt( (double) labs( nRadX * nRadY ) ) ) ),
            32, 256 );

        if( ( nRadX > 32 ) && ( nRadY > 32 ) && ( nRadX + nRadY ) < 8192 )
            nPoints >>= 1;

        // compute threshold
        const double    fRadX = nRadX;
        const double    fRadY = nRadY;
        const double    fCenterX = aCenter.X();
        const double    fCenterY = aCenter.Y();
        double          fStart = ImplGetParameter( aCenter, rStart, fRadX, fRadY );
        double          fEnd = ImplGetParameter( aCenter, rEnd, fRadX, fRadY );
        double          fDiff = fEnd - fStart;
        double          fStep;
        sal_uInt16      nStart;
        sal_uInt16      nEnd;

        if( fDiff < 0. )
            fDiff += F_2PI;

        if ( bFullCircle )
            fDiff = F_2PI;

        // Proportionally shrink number of points( fDiff / (2PI) );
        nPoints = std::max( (sal_uInt16) ( ( fDiff * 0.1591549 ) * nPoints ), (sal_uInt16) 16 );
        fStep = fDiff / ( nPoints - 1 );

        if( POLY_PIE == eStyle )
        {
            const Point aCenter2( FRound( fCenterX ), FRound( fCenterY ) );

            nStart = 1;
            nEnd = nPoints + 1;
            mpImplPolygon = new ImplPolygon( nPoints + 2 );
            mpImplPolygon->mpPointAry[ 0 ] = aCenter2;
            mpImplPolygon->mpPointAry[ nEnd ] = aCenter2;
        }
        else
        {
            mpImplPolygon = new ImplPolygon( ( POLY_CHORD == eStyle ) ? ( nPoints + 1 ) : nPoints );
            nStart = 0;
            nEnd = nPoints;
        }

        for(; nStart < nEnd; nStart++, fStart += fStep )
        {
            Point& rPt = mpImplPolygon->mpPointAry[ nStart ];

            rPt.X() = FRound( fCenterX + fRadX * cos( fStart ) );
            rPt.Y() = FRound( fCenterY - fRadY * sin( fStart ) );
        }

        if( POLY_CHORD == eStyle )
            mpImplPolygon->mpPointAry[ nPoints ] = mpImplPolygon->mpPointAry[ 0 ];
    }
    else
        mpImplPolygon = static_cast<ImplPolygon*>( &aStaticImplPolygon );
}

Polygon::Polygon( const Point& rBezPt1, const Point& rCtrlPt1,
                  const Point& rBezPt2, const Point& rCtrlPt2,
                  sal_uInt16 nPoints )
{
    nPoints = ( 0 == nPoints ) ? 25 : ( ( nPoints < 2 ) ? 2 : nPoints );

    const double    fInc = 1.0 / ( nPoints - 1 );
    double          fK_1 = 0.0, fK1_1 = 1.0;
    double          fK_2, fK_3, fK1_2, fK1_3;
    const double    fX0 = rBezPt1.X();
    const double    fY0 = rBezPt1.Y();
    const double    fX1 = 3.0 * rCtrlPt1.X();
    const double    fY1 = 3.0 * rCtrlPt1.Y();
    const double    fX2 = 3.0 * rCtrlPt2.X();
    const double    fY2 = 3.0 * rCtrlPt2.Y();
    const double    fX3 = rBezPt2.X();
    const double    fY3 = rBezPt2.Y();

    mpImplPolygon = new ImplPolygon( nPoints );

    for( sal_uInt16 i = 0; i < nPoints; i++, fK_1 += fInc, fK1_1 -= fInc )
    {
        Point& rPt = mpImplPolygon->mpPointAry[ i ];

        fK_2 = fK_1, fK_3 = ( fK_2 *= fK_1 ), fK_3 *= fK_1;
        fK1_2 = fK1_1, fK1_3 = ( fK1_2 *= fK1_1 ), fK1_3 *= fK1_1;
        double fK12 = fK_1 * fK1_2;
        double fK21 = fK_2 * fK1_1;

        rPt.X() = FRound( fK1_3 * fX0 + fK12 * fX1 + fK21 * fX2 + fK_3 * fX3 );
        rPt.Y() = FRound( fK1_3 * fY0 + fK12 * fY1 + fK21 * fY2 + fK_3 * fY3 );
    }
}

Polygon::~Polygon()
{

    // Remove if refcount == 0, otherwise decrement refcount
    if ( mpImplPolygon->mnRefCount )
    {
        if ( mpImplPolygon->mnRefCount > 1 )
            mpImplPolygon->mnRefCount--;
        else
            delete mpImplPolygon;
    }
}

const Point* Polygon::GetConstPointAry() const
{
    return mpImplPolygon->mpPointAry;
}

const sal_uInt8* Polygon::GetConstFlagAry() const
{
    return mpImplPolygon->mpFlagAry;
}

void Polygon::SetPoint( const Point& rPt, sal_uInt16 nPos )
{
    DBG_ASSERT( nPos < mpImplPolygon->mnPoints,
                "Polygon::SetPoint(): nPos >= nPoints" );

    ImplMakeUnique();
    mpImplPolygon->mpPointAry[nPos] = rPt;
}

void Polygon::SetFlags( sal_uInt16 nPos, PolyFlags eFlags )
{
    DBG_ASSERT( nPos < mpImplPolygon->mnPoints,
                "Polygon::SetFlags(): nPos >= nPoints" );

    // we do only want to create the flag array if there
    // is at least one flag different to POLY_NORMAL
    if ( eFlags != POLY_NORMAL )
    {
        ImplMakeUnique();
        mpImplPolygon->ImplCreateFlagArray();
        mpImplPolygon->mpFlagAry[ nPos ] = (sal_uInt8) eFlags;
    }
}

const Point& Polygon::GetPoint( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos < mpImplPolygon->mnPoints,
                "Polygon::GetPoint(): nPos >= nPoints" );

    return mpImplPolygon->mpPointAry[nPos];
}

PolyFlags Polygon::GetFlags( sal_uInt16 nPos ) const
{
    DBG_ASSERT( nPos < mpImplPolygon->mnPoints,
                "Polygon::GetFlags(): nPos >= nPoints" );
    return( mpImplPolygon->mpFlagAry ?
            (PolyFlags) mpImplPolygon->mpFlagAry[ nPos ] :
            POLY_NORMAL );
}

bool Polygon::HasFlags() const
{
    return mpImplPolygon->mpFlagAry != nullptr;
}

bool Polygon::IsRect() const
{
    bool bIsRect = false;
    if ( mpImplPolygon->mpFlagAry == nullptr )
    {
        if ( ( ( mpImplPolygon->mnPoints == 5 ) && ( mpImplPolygon->mpPointAry[ 0 ] == mpImplPolygon->mpPointAry[ 4 ] ) ) ||
             ( mpImplPolygon->mnPoints == 4 ) )
        {
            if ( ( mpImplPolygon->mpPointAry[ 0 ].X() == mpImplPolygon->mpPointAry[ 3 ].X() ) &&
                 ( mpImplPolygon->mpPointAry[ 0 ].Y() == mpImplPolygon->mpPointAry[ 1 ].Y() ) &&
                 ( mpImplPolygon->mpPointAry[ 1 ].X() == mpImplPolygon->mpPointAry[ 2 ].X() ) &&
                 ( mpImplPolygon->mpPointAry[ 2 ].Y() == mpImplPolygon->mpPointAry[ 3 ].Y() ) )
                bIsRect = true;
        }
    }
    return bIsRect;
}

void Polygon::SetSize( sal_uInt16 nNewSize )
{
    if( nNewSize != mpImplPolygon->mnPoints )
    {
        ImplMakeUnique();
        mpImplPolygon->ImplSetSize( nNewSize );
    }
}

sal_uInt16 Polygon::GetSize() const
{
    return mpImplPolygon->mnPoints;
}

void Polygon::Clear()
{
    if ( mpImplPolygon->mnRefCount )
    {
        if ( mpImplPolygon->mnRefCount > 1 )
            mpImplPolygon->mnRefCount--;
        else
            delete mpImplPolygon;
    }

    mpImplPolygon = static_cast<ImplPolygon*>(&aStaticImplPolygon);
}

double Polygon::CalcDistance( sal_uInt16 nP1, sal_uInt16 nP2 )
{
    DBG_ASSERT( nP1 < mpImplPolygon->mnPoints,
                "Polygon::CalcDistance(): nPos1 >= nPoints" );
    DBG_ASSERT( nP2 < mpImplPolygon->mnPoints,
                "Polygon::CalcDistance(): nPos2 >= nPoints" );

    const Point& rP1 = mpImplPolygon->mpPointAry[ nP1 ];
    const Point& rP2 = mpImplPolygon->mpPointAry[ nP2 ];
    const double fDx = rP2.X() - rP1.X();
    const double fDy = rP2.Y() - rP1.Y();

    return sqrt( fDx * fDx + fDy * fDy );
}

void Polygon::Optimize( PolyOptimizeFlags nOptimizeFlags )
{
    DBG_ASSERT( !mpImplPolygon->mpFlagAry, "Optimizing could fail with beziers!" );

    sal_uInt16 nSize = mpImplPolygon->mnPoints;

    if( bool(nOptimizeFlags) && nSize )
    {
        if( nOptimizeFlags & PolyOptimizeFlags::EDGES )
        {
            const Rectangle aBound( GetBoundRect() );
            const double    fArea = ( aBound.GetWidth() + aBound.GetHeight() ) * 0.5;
            const sal_uInt16 nPercent = 50;

            Optimize( PolyOptimizeFlags::NO_SAME );
            ImplReduceEdges( *this, fArea, nPercent );
        }
        else if( nOptimizeFlags & ( PolyOptimizeFlags::REDUCE | PolyOptimizeFlags::NO_SAME ) )
        {
            tools::Polygon aNewPoly;
            const Point& rFirst = mpImplPolygon->mpPointAry[ 0 ];
            const long nReduce = ( nOptimizeFlags & PolyOptimizeFlags::REDUCE ) ? 4 : 0;

            while( nSize && ( mpImplPolygon->mpPointAry[ nSize - 1 ] == rFirst ) )
                nSize--;

            if( nSize > 1 )
            {
                sal_uInt16 nLast = 0, nNewCount = 1;

                aNewPoly.SetSize( nSize );
                aNewPoly[ 0 ] = rFirst;

                for( sal_uInt16 i = 1; i < nSize; i++ )
                {
                    if( ( mpImplPolygon->mpPointAry[ i ] != mpImplPolygon->mpPointAry[ nLast ] ) &&
                        ( !nReduce || ( nReduce < FRound( CalcDistance( nLast, i ) ) ) ) )
                    {
                        aNewPoly[ nNewCount++ ] = mpImplPolygon->mpPointAry[ nLast = i ];
                    }
                }

                if( nNewCount == 1 )
                    aNewPoly.Clear();
                else
                    aNewPoly.SetSize( nNewCount );
            }

            *this = aNewPoly;
        }

        nSize = mpImplPolygon->mnPoints;

        if( nSize > 1 )
        {
            if( ( nOptimizeFlags & PolyOptimizeFlags::CLOSE ) &&
                ( mpImplPolygon->mpPointAry[ 0 ] != mpImplPolygon->mpPointAry[ nSize - 1 ] ) )
            {
                SetSize( mpImplPolygon->mnPoints + 1 );
                mpImplPolygon->mpPointAry[ mpImplPolygon->mnPoints - 1 ] = mpImplPolygon->mpPointAry[ 0 ];
            }
            else if( ( nOptimizeFlags & PolyOptimizeFlags::OPEN ) &&
                     ( mpImplPolygon->mpPointAry[ 0 ] == mpImplPolygon->mpPointAry[ nSize - 1 ] ) )
            {
                const Point& rFirst = mpImplPolygon->mpPointAry[ 0 ];

                while( nSize && ( mpImplPolygon->mpPointAry[ nSize - 1 ] == rFirst ) )
                    nSize--;

                SetSize( nSize );
            }
        }
    }
}


/** Recursively subdivide cubic bezier curve via deCasteljau.

   @param rPointIter
   Output iterator, where the subdivided polylines are written to.

   @param d
   Squared difference of curve to a straight line

   @param P*
   Exactly four points, interpreted as support and control points of
   a cubic bezier curve. Must be in device coordinates, since stop
   criterion is based on the following assumption: the device has a
   finite resolution, it is thus sufficient to stop subdivision if the
   curve does not deviate more than one pixel from a straight line.

*/
static void ImplAdaptiveSubdivide( ::std::back_insert_iterator< ::std::vector< Point > >& rPointIter,
                                   const double old_d2,
                                   int recursionDepth,
                                   const double d2,
                                   const double P1x, const double P1y,
                                   const double P2x, const double P2y,
                                   const double P3x, const double P3y,
                                   const double P4x, const double P4y )
{
    // Hard limit on recursion depth, empiric number.
    enum {maxRecursionDepth=128};

    // Perform bezier flatness test (lecture notes from R. Schaback,
    // Mathematics of Computer-Aided Design, Uni Goettingen, 2000)

    // ||P(t) - L(t)|| <= max     ||b_j - b_0 - j/n(b_n - b_0)||
    //                    0<=j<=n

    // What is calculated here is an upper bound to the distance from
    // a line through b_0 and b_3 (P1 and P4 in our notation) and the
    // curve. We can drop 0 and n from the running indices, since the
    // argument of max becomes zero for those cases.
    const double fJ1x( P2x - P1x - 1.0/3.0*(P4x - P1x) );
    const double fJ1y( P2y - P1y - 1.0/3.0*(P4y - P1y) );
    const double fJ2x( P3x - P1x - 2.0/3.0*(P4x - P1x) );
    const double fJ2y( P3y - P1y - 2.0/3.0*(P4y - P1y) );
    const double distance2( ::std::max( fJ1x*fJ1x + fJ1y*fJ1y,
                                        fJ2x*fJ2x + fJ2y*fJ2y) );

    // stop if error measure does not improve anymore. This is a
    // safety guard against floating point inaccuracies.
    // stop at recursion level 128. This is a safety guard against
    // floating point inaccuracies.
    // stop if distance from line is guaranteed to be bounded by d
    if( old_d2 > d2 &&
        recursionDepth < maxRecursionDepth &&
        distance2 >= d2 )
    {
        // deCasteljau bezier arc, split at t=0.5
        // Foley/vanDam, p. 508
        const double L1x( P1x ),             L1y( P1y );
        const double L2x( (P1x + P2x)*0.5 ), L2y( (P1y + P2y)*0.5 );
        const double Hx ( (P2x + P3x)*0.5 ), Hy ( (P2y + P3y)*0.5 );
        const double L3x( (L2x + Hx)*0.5 ),  L3y( (L2y + Hy)*0.5 );
        const double R4x( P4x ),             R4y( P4y );
        const double R3x( (P3x + P4x)*0.5 ), R3y( (P3y + P4y)*0.5 );
        const double R2x( (Hx + R3x)*0.5 ),  R2y( (Hy + R3y)*0.5 );
        const double R1x( (L3x + R2x)*0.5 ), R1y( (L3y + R2y)*0.5 );
        const double L4x( R1x ),             L4y( R1y );

        // subdivide further
        ++recursionDepth;
        ImplAdaptiveSubdivide(rPointIter, distance2, recursionDepth, d2, L1x, L1y, L2x, L2y, L3x, L3y, L4x, L4y);
        ImplAdaptiveSubdivide(rPointIter, distance2, recursionDepth, d2, R1x, R1y, R2x, R2y, R3x, R3y, R4x, R4y);
    }
    else
    {
        // requested resolution reached.
        // Add end points to output iterator.
        // order is preserved, since this is so to say depth first traversal.
        *rPointIter++ = Point( FRound(P1x), FRound(P1y) );
    }
}

void Polygon::AdaptiveSubdivide( Polygon& rResult, const double d ) const
{
    if( !mpImplPolygon->mpFlagAry )
    {
        rResult = *this;
    }
    else
    {
        sal_uInt16 i;
        sal_uInt16 nPts( GetSize() );
        ::std::vector< Point > aPoints;
        aPoints.reserve( nPts );
        ::std::back_insert_iterator< ::std::vector< Point > > aPointIter( aPoints );

        for(i=0; i<nPts;)
        {
            if( ( i + 3 ) < nPts )
            {
                sal_uInt8 P1( mpImplPolygon->mpFlagAry[ i ] );
                sal_uInt8 P4( mpImplPolygon->mpFlagAry[ i + 3 ] );

                if( ( POLY_NORMAL == P1 || POLY_SMOOTH == P1 || POLY_SYMMTR == P1 ) &&
                    ( POLY_CONTROL == mpImplPolygon->mpFlagAry[ i + 1 ] ) &&
                    ( POLY_CONTROL == mpImplPolygon->mpFlagAry[ i + 2 ] ) &&
                    ( POLY_NORMAL == P4 || POLY_SMOOTH == P4 || POLY_SYMMTR == P4 ) )
                {
                    ImplAdaptiveSubdivide( aPointIter, d*d+1.0, 0, d*d,
                                           mpImplPolygon->mpPointAry[ i ].X(),   mpImplPolygon->mpPointAry[ i ].Y(),
                                           mpImplPolygon->mpPointAry[ i+1 ].X(), mpImplPolygon->mpPointAry[ i+1 ].Y(),
                                           mpImplPolygon->mpPointAry[ i+2 ].X(), mpImplPolygon->mpPointAry[ i+2 ].Y(),
                                           mpImplPolygon->mpPointAry[ i+3 ].X(), mpImplPolygon->mpPointAry[ i+3 ].Y() );
                    i += 3;
                    continue;
                }
            }

            *aPointIter++ = mpImplPolygon->mpPointAry[ i++ ];

            if (aPoints.size() >= SAL_MAX_UINT16)
            {
                OSL_ENSURE(aPoints.size() < SAL_MAX_UINT16,
                    "Polygon::AdapativeSubdivision created polygon too many points;"
                    " using original polygon instead");

                // The resulting polygon can not hold all the points
                // that we have created so far.  Stop the subdivision
                // and return a copy of the unmodified polygon.
                rResult = *this;
                return;
            }
        }

        // fill result polygon
        rResult = tools::Polygon( (sal_uInt16)aPoints.size() ); // ensure sufficient size for copy
        ::std::copy(aPoints.begin(), aPoints.end(), rResult.mpImplPolygon->mpPointAry);
    }
}

void Polygon::ImplReduceEdges( tools::Polygon& rPoly, const double& rArea, sal_uInt16 nPercent )
{
    const double    fBound = 2000.0 * ( 100 - nPercent ) * 0.01;
    sal_uInt16      nNumNoChange = 0,
                    nNumRuns = 0;

    while( nNumNoChange < 2 )
    {
        sal_uInt16  nPntCnt = rPoly.GetSize(), nNewPos = 0;
        tools::Polygon aNewPoly( nPntCnt );
        bool bChangeInThisRun = false;

        for( sal_uInt16 n = 0; n < nPntCnt; n++ )
        {
            bool bDeletePoint = false;

            if( ( n + nNumRuns ) % 2 )
            {
                sal_uInt16      nIndPrev = !n ? nPntCnt - 1 : n - 1;
                sal_uInt16      nIndPrevPrev = !nIndPrev ? nPntCnt - 1 : nIndPrev - 1;
                sal_uInt16      nIndNext = ( n == nPntCnt-1 ) ? 0 : n + 1;
                sal_uInt16      nIndNextNext = ( nIndNext == nPntCnt - 1 ) ? 0 : nIndNext + 1;
                Vector2D    aVec1( rPoly[ nIndPrev ] ); aVec1 -= rPoly[ nIndPrevPrev ];
                Vector2D    aVec2( rPoly[ n ] ); aVec2 -= rPoly[ nIndPrev ];
                Vector2D    aVec3( rPoly[ nIndNext ] ); aVec3 -= rPoly[ n ];
                Vector2D    aVec4( rPoly[ nIndNextNext ] ); aVec4 -= rPoly[ nIndNext ];
                double      fDist1 = aVec1.GetLength(), fDist2 = aVec2.GetLength();
                double      fDist3 = aVec3.GetLength(), fDist4 = aVec4.GetLength();
                double      fTurnB = aVec2.Normalize().Scalar( aVec3.Normalize() );

                if( fabs( fTurnB ) < ( 1.0 + SMALL_DVALUE ) && fabs( fTurnB ) > ( 1.0 - SMALL_DVALUE ) )
                    bDeletePoint = true;
                else
                {
                    Vector2D    aVecB( rPoly[ nIndNext ] );
                    double      fDistB = ( aVecB -= rPoly[ nIndPrev ] ).GetLength();
                    double      fLenWithB = fDist2 + fDist3;
                    double      fLenFact = ( fDistB != 0.0 ) ? fLenWithB / fDistB : 1.0;
                    double      fTurnPrev = aVec1.Normalize().Scalar( aVec2 );
                    double      fTurnNext = aVec3.Scalar( aVec4.Normalize() );
                    double      fGradPrev, fGradB, fGradNext;

                    if( fabs( fTurnPrev ) < ( 1.0 + SMALL_DVALUE ) && fabs( fTurnPrev ) > ( 1.0 - SMALL_DVALUE ) )
                        fGradPrev = 0.0;
                    else
                        fGradPrev = acos( fTurnPrev ) / ( aVec1.IsNegative( aVec2 ) ? -F_PI180 : F_PI180 );

                    fGradB = acos( fTurnB ) / ( aVec2.IsNegative( aVec3 ) ? -F_PI180 : F_PI180 );

                    if( fabs( fTurnNext ) < ( 1.0 + SMALL_DVALUE ) && fabs( fTurnNext ) > ( 1.0 - SMALL_DVALUE ) )
                        fGradNext = 0.0;
                    else
                        fGradNext = acos( fTurnNext ) / ( aVec3.IsNegative( aVec4 ) ? -F_PI180 : F_PI180 );

                    if( ( fGradPrev > 0.0 && fGradB < 0.0 && fGradNext > 0.0 ) ||
                        ( fGradPrev < 0.0 && fGradB > 0.0 && fGradNext < 0.0 ) )
                    {
                        if( ( fLenFact < ( FSQRT2 + SMALL_DVALUE ) ) &&
                            ( ( ( fDist1 + fDist4 ) / ( fDist2 + fDist3 ) ) * 2000.0 ) > fBound )
                        {
                            bDeletePoint = true;
                        }
                    }
                    else
                    {
                        double fRelLen = 1.0 - sqrt( fDistB / rArea );

                        if( fRelLen < 0.0 )
                            fRelLen = 0.0;
                        else if( fRelLen > 1.0 )
                            fRelLen = 1.0;

                        if( ( (sal_uInt32) ( ( ( fLenFact - 1.0 ) * 1000000.0 ) + 0.5 ) < fBound ) &&
                            ( fabs( fGradB ) <= ( fRelLen * fBound * 0.01 ) ) )
                        {
                            bDeletePoint = true;
                        }
                    }
                }
            }

            if( !bDeletePoint )
                aNewPoly[ nNewPos++ ] = rPoly[ n ];
            else
                bChangeInThisRun = true;
        }

        if( bChangeInThisRun && nNewPos )
        {
            aNewPoly.SetSize( nNewPos );
            rPoly = aNewPoly;
            nNumNoChange = 0;
        }
        else
            nNumNoChange++;

        nNumRuns++;
    }
}

void Polygon::Move( long nHorzMove, long nVertMove )
{
    // This check is required for DrawEngine
    if ( !nHorzMove && !nVertMove )
        return;

    ImplMakeUnique();

    // Move points
    sal_uInt16 nCount = mpImplPolygon->mnPoints;
    for ( sal_uInt16 i = 0; i < nCount; i++ )
    {
        Point* pPt = &(mpImplPolygon->mpPointAry[i]);
        pPt->X() += nHorzMove;
        pPt->Y() += nVertMove;
    }
}

void Polygon::Translate(const Point& rTrans)
{
    ImplMakeUnique();

    for ( sal_uInt16 i = 0, nCount = mpImplPolygon->mnPoints; i < nCount; i++ )
        mpImplPolygon->mpPointAry[ i ] += rTrans;
}

void Polygon::Scale( double fScaleX, double fScaleY )
{
    ImplMakeUnique();

    for ( sal_uInt16 i = 0, nCount = mpImplPolygon->mnPoints; i < nCount; i++ )
    {
        Point& rPnt = mpImplPolygon->mpPointAry[i];
        rPnt.X() = (long) ( fScaleX * rPnt.X() );
        rPnt.Y() = (long) ( fScaleY * rPnt.Y() );
    }
}

void Polygon::Rotate( const Point& rCenter, sal_uInt16 nAngle10 )
{
    nAngle10 %= 3600;

    if( nAngle10 )
    {
        const double fAngle = F_PI1800 * nAngle10;
        Rotate( rCenter, sin( fAngle ), cos( fAngle ) );
    }
}

void Polygon::Rotate( const Point& rCenter, double fSin, double fCos )
{
    ImplMakeUnique();

    long nCenterX = rCenter.X();
    long nCenterY = rCenter.Y();

    for( sal_uInt16 i = 0, nCount = mpImplPolygon->mnPoints; i < nCount; i++ )
    {
        Point& rPt = mpImplPolygon->mpPointAry[ i ];

        const long nX = rPt.X() - nCenterX;
        const long nY = rPt.Y() - nCenterY;
        rPt.X() = (long) FRound( fCos * nX + fSin * nY ) + nCenterX;
        rPt.Y() = -(long) FRound( fSin * nX - fCos * nY ) + nCenterY;
    }
}

void Polygon::Clip( const Rectangle& rRect, bool bPolygon )
{
    // #105251# Justify rect before edge filtering
    Rectangle               aJustifiedRect( rRect );
    aJustifiedRect.Justify();

    sal_uInt16                  nSourceSize = mpImplPolygon->mnPoints;
    ImplPolygonPointFilter  aPolygon( nSourceSize );
    ImplEdgePointFilter     aHorzFilter( EDGE_HORZ, aJustifiedRect.Left(), aJustifiedRect.Right(),
                                         aPolygon );
    ImplEdgePointFilter     aVertFilter( EDGE_VERT, aJustifiedRect.Top(), aJustifiedRect.Bottom(),
                                         aHorzFilter );

    for ( sal_uInt16 i = 0; i < nSourceSize; i++ )
        aVertFilter.Input( mpImplPolygon->mpPointAry[i] );
    if ( bPolygon || aVertFilter.IsPolygon() )
        aVertFilter.LastPoint();
    else
        aPolygon.LastPoint();

    // Delete old ImpPolygon-data and assign from ImpPolygonPointFilter
    if ( mpImplPolygon->mnRefCount )
    {
        if ( mpImplPolygon->mnRefCount > 1 )
            mpImplPolygon->mnRefCount--;
        else
            delete mpImplPolygon;
    }
    mpImplPolygon = aPolygon.release();
}

Rectangle Polygon::GetBoundRect() const
{
    // Removing the assert. Bezier curves have the attribute that each single
    // curve segment defined by four points can not exit the four-point polygon
    // defined by that points. This allows to say that the curve segment can also
    // never leave the Range of its defining points.
    // The result is that Polygon::GetBoundRect() may not create the minimal
    // BoundRect of the Polygon (to get that, use basegfx::B2DPolygon classes),
    // but will always create a valid BoundRect, at least as long as this method
    // 'blindly' travels over all points, including control points.

    // DBG_ASSERT( !mpImplPolygon->mpFlagAry, "GetBoundRect could fail with beziers!" );

    sal_uInt16  nCount = mpImplPolygon->mnPoints;
    if( ! nCount )
        return Rectangle();

    long    nXMin, nXMax, nYMin, nYMax;

    const Point* pPt = &(mpImplPolygon->mpPointAry[0]);
    nXMin = nXMax = pPt->X();
    nYMin = nYMax = pPt->Y();

    for ( sal_uInt16 i = 0; i < nCount; i++ )
    {
        pPt = &(mpImplPolygon->mpPointAry[i]);

        if ( pPt->X() < nXMin )
            nXMin = pPt->X();
        if ( pPt->X() > nXMax )
            nXMax = pPt->X();
        if ( pPt->Y() < nYMin )
            nYMin = pPt->Y();
        if ( pPt->Y() > nYMax )
            nYMax = pPt->Y();
    }

    return Rectangle( nXMin, nYMin, nXMax, nYMax );
}

double Polygon::GetSignedArea() const
{
    DBG_ASSERT( !mpImplPolygon->mpFlagAry, "GetArea could fail with beziers!" );

    double fArea = 0.0;

    if( mpImplPolygon->mnPoints > 2 )
    {
        const sal_uInt16 nCount1 = mpImplPolygon->mnPoints - 1;

        for( sal_uInt16 i = 0; i < nCount1; )
        {
            const Point& rPt = mpImplPolygon->mpPointAry[ i ];
            const Point& rPt1 = mpImplPolygon->mpPointAry[ ++i ];
            fArea += ( rPt.X() - rPt1.X() ) * ( rPt.Y() + rPt1.Y() );
        }

        const Point& rPt = mpImplPolygon->mpPointAry[ nCount1 ];
        const Point& rPt0 = mpImplPolygon->mpPointAry[ 0 ];
        fArea += ( rPt.X() - rPt0.X() ) * ( rPt.Y() + rPt0.Y() );
    }

    return fArea;
}

bool Polygon::IsInside( const Point& rPoint ) const
{
    DBG_ASSERT( !mpImplPolygon->mpFlagAry, "IsInside could fail with beziers!" );

    const Rectangle aBound( GetBoundRect() );
    const Line      aLine( rPoint, Point( aBound.Right() + 100L, rPoint.Y() ) );
    sal_uInt16          nCount = mpImplPolygon->mnPoints;
    sal_uInt16          nPCounter = 0;

    if ( ( nCount > 2 ) && aBound.IsInside( rPoint ) )
    {
        Point   aPt1( mpImplPolygon->mpPointAry[ 0 ] );
        Point   aIntersection;
        Point   aLastIntersection;

        while ( ( aPt1 == mpImplPolygon->mpPointAry[ nCount - 1 ] ) && ( nCount > 3 ) )
            nCount--;

        for ( sal_uInt16 i = 1; i <= nCount; i++ )
        {
            const Point& rPt2 = mpImplPolygon->mpPointAry[ ( i < nCount ) ? i : 0 ];

            if ( aLine.Intersection( Line( aPt1, rPt2 ), aIntersection ) )
            {
                // This avoids insertion of double intersections
                if ( nPCounter )
                {
                    if ( aIntersection != aLastIntersection )
                    {
                        aLastIntersection = aIntersection;
                        nPCounter++;
                    }
                }
                else
                {
                    aLastIntersection = aIntersection;
                    nPCounter++;
                }
            }

            aPt1 = rPt2;
        }
    }

    // is inside, if number of intersection points is odd
    return ( ( nPCounter & 1 ) == 1 );
}

bool Polygon::IsRightOrientated() const
{
    return GetSignedArea() >= 0.0;
}

void Polygon::Insert( sal_uInt16 nPos, const Point& rPt, PolyFlags eFlags )
{
    ImplMakeUnique();

    if( nPos >= mpImplPolygon->mnPoints )
        nPos = mpImplPolygon->mnPoints;

    mpImplPolygon->ImplSplit( nPos, 1 );
    mpImplPolygon->mpPointAry[ nPos ] = rPt;

    if( POLY_NORMAL != eFlags )
    {
        mpImplPolygon->ImplCreateFlagArray();
        mpImplPolygon->mpFlagAry[ nPos ] = (sal_uInt8) eFlags;
    }
}

void Polygon::Insert( sal_uInt16 nPos, const tools::Polygon& rPoly )
{
    const sal_uInt16 nInsertCount = rPoly.mpImplPolygon->mnPoints;

    if( nInsertCount )
    {
        ImplMakeUnique();

        if( nPos >= mpImplPolygon->mnPoints )
            nPos = mpImplPolygon->mnPoints;

        if( rPoly.mpImplPolygon->mpFlagAry )
            mpImplPolygon->ImplCreateFlagArray();

        mpImplPolygon->ImplSplit( nPos, nInsertCount, rPoly.mpImplPolygon );
    }
}

Point& Polygon::operator[]( sal_uInt16 nPos )
{
    DBG_ASSERT( nPos < mpImplPolygon->mnPoints, "Polygon::[]: nPos >= nPoints" );

    ImplMakeUnique();
    return mpImplPolygon->mpPointAry[nPos];
}

tools::Polygon& Polygon::operator=( const tools::Polygon& rPoly )
{
    DBG_ASSERT( rPoly.mpImplPolygon->mnRefCount < 0xFFFFFFFE, "Polygon: RefCount overflow" );

    // Increase refcounter before assigning
    // Note: RefCount == 0 for static objects
    if ( rPoly.mpImplPolygon->mnRefCount )
        rPoly.mpImplPolygon->mnRefCount++;

    // Delete if recount == 0, otherwise decrement
    if ( mpImplPolygon->mnRefCount )
    {
        if ( mpImplPolygon->mnRefCount > 1 )
            mpImplPolygon->mnRefCount--;
        else
            delete mpImplPolygon;
    }

    mpImplPolygon = rPoly.mpImplPolygon;
    return *this;
}

bool Polygon::operator==( const tools::Polygon& rPoly ) const
{

    if ( (rPoly.mpImplPolygon == mpImplPolygon) )
        return true;
    else
        return false;
}

bool Polygon::IsEqual( const tools::Polygon& rPoly ) const
{
    bool bIsEqual = true;
    sal_uInt16 i;
    if ( GetSize() != rPoly.GetSize() )
        bIsEqual = false;
    else
    {
        for ( i = 0; i < GetSize(); i++ )
        {
            if ( ( GetPoint( i ) != rPoly.GetPoint( i ) ) ||
                ( GetFlags( i ) != rPoly.GetFlags( i ) ) )
            {
                bIsEqual = false;
                break;
            }
        }
    }
    return bIsEqual;
}

SvStream& ReadPolygon( SvStream& rIStream, tools::Polygon& rPoly )
{
    SAL_WARN_IF( !rIStream.GetVersion(), "tools", "Polygon::>> - Solar-Version not set on rIStream" );

    sal_uInt16          i;
    sal_uInt16          nPoints(0);

    // read all points and create array
    rIStream.ReadUInt16( nPoints );

    const size_t nMaxRecordsPossible = rIStream.remainingSize() / (2 * sizeof(sal_Int32));
    if (nPoints > nMaxRecordsPossible)
    {
        SAL_WARN("tools", "Polygon claims " << nPoints << " records, but only " << nMaxRecordsPossible << " possible");
        nPoints = nMaxRecordsPossible;
    }

    if ( rPoly.mpImplPolygon->mnRefCount != 1 )
    {
        if ( rPoly.mpImplPolygon->mnRefCount )
            rPoly.mpImplPolygon->mnRefCount--;
        rPoly.mpImplPolygon = new ImplPolygon( nPoints );
    }
    else
        rPoly.mpImplPolygon->ImplSetSize( nPoints, false );

    {
        // Determine whether we need to write through operators
#if (SAL_TYPES_SIZEOFLONG) == 4
#ifdef OSL_BIGENDIAN
        if ( rIStream.GetEndian() == SvStreamEndian::BIG )
#else
        if ( rIStream.GetEndian() == SvStreamEndian::LITTLE )
#endif
            rIStream.Read( rPoly.mpImplPolygon->mpPointAry, nPoints*sizeof(Point) );
        else
#endif
        {
            for( i = 0; i < nPoints; i++ )
            {
                sal_Int32 nTmpX(0), nTmpY(0);
                rIStream.ReadInt32( nTmpX ).ReadInt32( nTmpY );
                rPoly.mpImplPolygon->mpPointAry[i].X() = nTmpX;
                rPoly.mpImplPolygon->mpPointAry[i].Y() = nTmpY;
            }
        }
    }

    return rIStream;
}

SvStream& WritePolygon( SvStream& rOStream, const tools::Polygon& rPoly )
{
    sal_uInt16          i;
    sal_uInt16          nPoints = rPoly.GetSize();

    // Write number of points
    rOStream.WriteUInt16( nPoints );

    {
        // Determine whether we need to write through operators
#if (SAL_TYPES_SIZEOFLONG) == 4
#ifdef OSL_BIGENDIAN
        if ( rOStream.GetEndian() == SvStreamEndian::BIG )
#else
        if ( rOStream.GetEndian() == SvStreamEndian::LITTLE )
#endif
        {
            if ( nPoints )
                rOStream.Write( rPoly.mpImplPolygon->mpPointAry, nPoints*sizeof(Point) );
        }
        else
#endif
        {
            for( i = 0; i < nPoints; i++ )
            {
                rOStream.WriteInt32( rPoly.mpImplPolygon->mpPointAry[i].X() )
                        .WriteInt32( rPoly.mpImplPolygon->mpPointAry[i].Y() );
            }
        }
    }

    return rOStream;
}

void Polygon::ImplRead( SvStream& rIStream )
{
    sal_uInt8 bHasPolyFlags(0);

    ReadPolygon( rIStream, *this );
    rIStream.ReadUChar( bHasPolyFlags );

    if ( bHasPolyFlags )
    {
        mpImplPolygon->mpFlagAry = new sal_uInt8[ mpImplPolygon->mnPoints ];
        rIStream.Read( mpImplPolygon->mpFlagAry, mpImplPolygon->mnPoints );
    }
}

void Polygon::Read( SvStream& rIStream )
{
    VersionCompat aCompat( rIStream, StreamMode::READ );

    ImplRead( rIStream );
}

void Polygon::ImplWrite( SvStream& rOStream ) const
{
    bool bHasPolyFlags = mpImplPolygon->mpFlagAry != nullptr;
    WritePolygon( rOStream, *this );
    rOStream.WriteBool(bHasPolyFlags);

    if ( bHasPolyFlags )
        rOStream.Write( mpImplPolygon->mpFlagAry, mpImplPolygon->mnPoints );
}

void Polygon::Write( SvStream& rOStream ) const
{
    VersionCompat aCompat( rOStream, StreamMode::WRITE, 1 );

    ImplWrite( rOStream );
}

// #i74631#/#i115917# numerical correction method for B2DPolygon
void impCorrectContinuity(basegfx::B2DPolygon& roPolygon, sal_uInt32 nIndex, sal_uInt8 nCFlag)
{
    const sal_uInt32 nPointCount(roPolygon.count());
    OSL_ENSURE(nIndex < nPointCount, "impCorrectContinuity: index access out of range (!)");

    if(nIndex < nPointCount && (POLY_SMOOTH == nCFlag || POLY_SYMMTR == nCFlag))
    {
        if(roPolygon.isPrevControlPointUsed(nIndex) && roPolygon.isNextControlPointUsed(nIndex))
        {
            // #i115917# Patch from osnola (modified, thanks for showing the porblem)

            // The correction is needed because an integer polygon with control points
            // is converted to double precision. When C1 or C2 is used the involved vectors
            // may not have the same directions/lengths since these come from integer coordinates
            //  and may have been snapped to different nearest integer coordinates. The snap error
            // is in the range of +-1 in y and y, thus 0.0 <= error <= sqrt(2.0). Nonetheless,
            // it needs to be corrected to be able to detect the continuity in this points
            // correctly.

            // We only have the integer data here (already in double precision form, but no mantisses
            // used), so the best correction is to use:

            // for C1: The longest vector since it potentially has best preserved the original vector.
            //         Even better the sum of the vectors, weighted by their length. This gives the
            //         normal vector addition to get the vector itself, lengths need to be preserved.
            // for C2: The mediated vector(s) since both should be the same, but mirrored

            // extract the point and vectors
            const basegfx::B2DPoint aPoint(roPolygon.getB2DPoint(nIndex));
            const basegfx::B2DVector aNext(roPolygon.getNextControlPoint(nIndex) - aPoint);
            const basegfx::B2DVector aPrev(aPoint - roPolygon.getPrevControlPoint(nIndex));

            // calculate common direction vector, normalize
            const basegfx::B2DVector aDirection(aNext + aPrev);

            if(POLY_SMOOTH == nCFlag)
            {
                // C1: apply common direction vector, preserve individual lengths
                const double fInvDirectionLen(1.0 / aDirection.getLength());
                roPolygon.setNextControlPoint(nIndex, basegfx::B2DPoint(aPoint + (aDirection * (aNext.getLength() * fInvDirectionLen))));
                roPolygon.setPrevControlPoint(nIndex, basegfx::B2DPoint(aPoint - (aDirection * (aPrev.getLength() * fInvDirectionLen))));
            }
            else // POLY_SYMMTR
            {
                // C2: get mediated length. Taking half of the unnormalized direction would be
                // an approximation, but not correct.
                const double fMedLength((aNext.getLength() + aPrev.getLength()) * (0.5 / aDirection.getLength()));
                const basegfx::B2DVector aScaledDirection(aDirection * fMedLength);

                // Bring Direction to correct length and apply
                roPolygon.setNextControlPoint(nIndex, basegfx::B2DPoint(aPoint + aScaledDirection));
                roPolygon.setPrevControlPoint(nIndex, basegfx::B2DPoint(aPoint - aScaledDirection));
            }
        }
    }
}

// convert to basegfx::B2DPolygon and return
basegfx::B2DPolygon Polygon::getB2DPolygon() const
{
    basegfx::B2DPolygon aRetval;
    const sal_uInt16 nCount(mpImplPolygon->mnPoints);

    if(nCount)
    {
        if(mpImplPolygon->mpFlagAry)
        {
            // handling for curves. Add start point
            const Point aStartPoint(mpImplPolygon->mpPointAry[0]);
            sal_uInt8 nPointFlag(mpImplPolygon->mpFlagAry[0]);
            aRetval.append(basegfx::B2DPoint(aStartPoint.X(), aStartPoint.Y()));
            Point aControlA, aControlB;

            for(sal_uInt16 a(1); a < nCount;)
            {
                bool bControlA(false);
                bool bControlB(false);

                if(POLY_CONTROL == mpImplPolygon->mpFlagAry[a])
                {
                    aControlA = mpImplPolygon->mpPointAry[a++];
                    bControlA = true;
                }

                if(a < nCount && POLY_CONTROL == mpImplPolygon->mpFlagAry[a])
                {
                    aControlB = mpImplPolygon->mpPointAry[a++];
                    bControlB = true;
                }

                // assert invalid polygons
                OSL_ENSURE(bControlA == bControlB, "Polygon::getB2DPolygon: Invalid source polygon (!)");
                (void)bControlB;

                if(a < nCount)
                {
                    const Point aEndPoint(mpImplPolygon->mpPointAry[a]);

                    if(bControlA)
                    {
                        // bezier edge, add
                        aRetval.appendBezierSegment(
                            basegfx::B2DPoint(aControlA.X(), aControlA.Y()),
                            basegfx::B2DPoint(aControlB.X(), aControlB.Y()),
                            basegfx::B2DPoint(aEndPoint.X(), aEndPoint.Y()));

                        impCorrectContinuity(aRetval, aRetval.count() - 2, nPointFlag);
                    }
                    else
                    {
                        // no bezier edge, add end point
                        aRetval.append(basegfx::B2DPoint(aEndPoint.X(), aEndPoint.Y()));
                    }

                    nPointFlag = mpImplPolygon->mpFlagAry[a++];
                }
            }

            // if exist, remove double first/last points, set closed and correct control points
            basegfx::tools::checkClosed(aRetval);

            if(aRetval.isClosed())
            {
                // closeWithGeometryChange did really close, so last point(s) were removed.
                // Correct the continuity in the changed point
                impCorrectContinuity(aRetval, 0, mpImplPolygon->mpFlagAry[0]);
            }
        }
        else
        {
            // extra handling for non-curves (most-used case) for speedup
            for(sal_uInt16 a(0); a < nCount; a++)
            {
                // get point and add
                const Point aPoint(mpImplPolygon->mpPointAry[a]);
                aRetval.append(basegfx::B2DPoint(aPoint.X(), aPoint.Y()));
            }

            // set closed flag
            basegfx::tools::checkClosed(aRetval);
        }
    }

    return aRetval;
}

// constructor to convert from basegfx::B2DPolygon
// #i76891# Needed to change from adding all control points (even for unused
// edges) and creating a fixed-size Polygon in the first run to creating the
// minimal Polygon. This requires a temporary Point- and Flag-Array for curves
// and a memcopy at ImplPolygon creation, but contains no zero-controlpoints
// for straight edges.
Polygon::Polygon(const basegfx::B2DPolygon& rPolygon)
:   mpImplPolygon(nullptr)
{
    const bool bCurve(rPolygon.areControlPointsUsed());
    const bool bClosed(rPolygon.isClosed());
    sal_uInt32 nB2DLocalCount(rPolygon.count());

    if(bCurve)
    {
        // #127979# Reduce source point count hard to the limit of the tools Polygon
        if(nB2DLocalCount > ((0x0000ffff / 3L) - 1L))
        {
            OSL_FAIL("Polygon::Polygon: Too many points in given B2DPolygon, need to reduce hard to maximum of tools Polygon (!)");
            nB2DLocalCount = ((0x0000ffff / 3L) - 1L);
        }

        // calculate target point count
        const sal_uInt32 nLoopCount(bClosed ? nB2DLocalCount : (nB2DLocalCount ? nB2DLocalCount - 1L : 0L ));

        if(nLoopCount)
        {
            // calculate maximum array size and allocate; prepare insert index
            const sal_uInt32 nMaxTargetCount((nLoopCount * 3) + 1);
            mpImplPolygon = new ImplPolygon(static_cast< sal_uInt16 >(nMaxTargetCount), true);

            // prepare insert index and current point
            sal_uInt32 nArrayInsert(0);
            basegfx::B2DCubicBezier aBezier;
            aBezier.setStartPoint(rPolygon.getB2DPoint(0));

            for(sal_uInt32 a(0L); a < nLoopCount; a++)
            {
                // add current point (always) and remember StartPointIndex for evtl. later corrections
                const Point aStartPoint(FRound(aBezier.getStartPoint().getX()), FRound(aBezier.getStartPoint().getY()));
                const sal_uInt32 nStartPointIndex(nArrayInsert);
                mpImplPolygon->mpPointAry[nStartPointIndex] = aStartPoint;
                mpImplPolygon->mpFlagAry[nStartPointIndex] = (sal_uInt8)POLY_NORMAL;
                nArrayInsert++;

                // prepare next segment
                const sal_uInt32 nNextIndex((a + 1) % nB2DLocalCount);
                aBezier.setEndPoint(rPolygon.getB2DPoint(nNextIndex));
                aBezier.setControlPointA(rPolygon.getNextControlPoint(a));
                aBezier.setControlPointB(rPolygon.getPrevControlPoint(nNextIndex));

                if(aBezier.isBezier())
                {
                    // if one is used, add always two control points due to the old schema
                    mpImplPolygon->mpPointAry[nArrayInsert] = Point(FRound(aBezier.getControlPointA().getX()), FRound(aBezier.getControlPointA().getY()));
                    mpImplPolygon->mpFlagAry[nArrayInsert] = (sal_uInt8)POLY_CONTROL;
                    nArrayInsert++;

                    mpImplPolygon->mpPointAry[nArrayInsert] = Point(FRound(aBezier.getControlPointB().getX()), FRound(aBezier.getControlPointB().getY()));
                    mpImplPolygon->mpFlagAry[nArrayInsert] = (sal_uInt8)POLY_CONTROL;
                    nArrayInsert++;
                }

                // test continuity with previous control point to set flag value
                if(aBezier.getControlPointA() != aBezier.getStartPoint() && (bClosed || a))
                {
                    const basegfx::B2VectorContinuity eCont(rPolygon.getContinuityInPoint(a));

                    if(basegfx::B2VectorContinuity::C1 == eCont)
                    {
                        mpImplPolygon->mpFlagAry[nStartPointIndex] = (sal_uInt8)POLY_SMOOTH;
                    }
                    else if(basegfx::B2VectorContinuity::C2 == eCont)
                    {
                        mpImplPolygon->mpFlagAry[nStartPointIndex] = (sal_uInt8)POLY_SYMMTR;
                    }
                }

                // prepare next polygon step
                aBezier.setStartPoint(aBezier.getEndPoint());
            }

            if(bClosed)
            {
                // add first point again as closing point due to old definition
                mpImplPolygon->mpPointAry[nArrayInsert] = mpImplPolygon->mpPointAry[0];
                mpImplPolygon->mpFlagAry[nArrayInsert] = (sal_uInt8)POLY_NORMAL;
                nArrayInsert++;
            }
            else
            {
                // add last point as closing point
                const basegfx::B2DPoint aClosingPoint(rPolygon.getB2DPoint(nB2DLocalCount - 1L));
                const Point aEnd(FRound(aClosingPoint.getX()), FRound(aClosingPoint.getY()));
                mpImplPolygon->mpPointAry[nArrayInsert] = aEnd;
                mpImplPolygon->mpFlagAry[nArrayInsert] = (sal_uInt8)POLY_NORMAL;
                nArrayInsert++;
            }

            DBG_ASSERT(nArrayInsert <= nMaxTargetCount, "Polygon::Polygon from basegfx::B2DPolygon: wrong max point count estimation (!)");

            if(nArrayInsert != nMaxTargetCount)
            {
                mpImplPolygon->ImplSetSize(static_cast< sal_uInt16 >(nArrayInsert));
            }
        }
    }
    else
    {
        // #127979# Reduce source point count hard to the limit of the tools Polygon
        if(nB2DLocalCount > (0x0000ffff - 1L))
        {
            OSL_FAIL("Polygon::Polygon: Too many points in given B2DPolygon, need to reduce hard to maximum of tools Polygon (!)");
            nB2DLocalCount = (0x0000ffff - 1L);
        }

        if(nB2DLocalCount)
        {
            // point list creation
            const sal_uInt32 nTargetCount(nB2DLocalCount + (bClosed ? 1L : 0L));
            mpImplPolygon = new ImplPolygon( static_cast< sal_uInt16 >(nTargetCount) );
            sal_uInt16 nIndex(0);

            for(sal_uInt32 a(0L); a < nB2DLocalCount; a++)
            {
                basegfx::B2DPoint aB2DPoint(rPolygon.getB2DPoint(a));
                Point aPoint(FRound(aB2DPoint.getX()), FRound(aB2DPoint.getY()));
                mpImplPolygon->mpPointAry[nIndex++] = aPoint;
            }

            if(bClosed)
            {
                // add first point as closing point
                mpImplPolygon->mpPointAry[nIndex] = mpImplPolygon->mpPointAry[0];
            }
        }
    }

    if(!mpImplPolygon)
    {
        // no content yet, create empty polygon
        mpImplPolygon = static_cast<ImplPolygon*>(&aStaticImplPolygon);
    }
}

} // namespace tools

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
