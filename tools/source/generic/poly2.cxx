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

#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <poly.h>
#include <tools/poly.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <tools/gen.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>

namespace tools {

PolyPolygon::PolyPolygon( sal_uInt16 nInitSize )
    : mpImplPolyPolygon( ImplPolyPolygon( nInitSize ) )
{
}

PolyPolygon::PolyPolygon( const tools::Polygon& rPoly )
    : mpImplPolyPolygon( rPoly )
{
}

PolyPolygon::PolyPolygon( const tools::PolyPolygon& rPolyPoly )
    : mpImplPolyPolygon( rPolyPoly.mpImplPolyPolygon )
{
}

PolyPolygon::PolyPolygon( tools::PolyPolygon&& rPolyPoly ) noexcept
    : mpImplPolyPolygon( std::move(rPolyPoly.mpImplPolyPolygon) )
{
}

PolyPolygon::~PolyPolygon()
{
}

void PolyPolygon::Insert( const tools::Polygon& rPoly, sal_uInt16 nPos )
{
    assert ( mpImplPolyPolygon->mvPolyAry.size() < MAX_POLYGONS );

    if ( nPos > mpImplPolyPolygon->mvPolyAry.size() )
        nPos = mpImplPolyPolygon->mvPolyAry.size();

    mpImplPolyPolygon->mvPolyAry.insert(mpImplPolyPolygon->mvPolyAry.begin() + nPos, rPoly);
}

void PolyPolygon::Remove( sal_uInt16 nPos )
{
    assert(nPos < Count() && "PolyPolygon::Remove(): nPos >= nSize");

    mpImplPolyPolygon->mvPolyAry.erase(mpImplPolyPolygon->mvPolyAry.begin() + nPos);
}

void PolyPolygon::Replace( const tools::Polygon& rPoly, sal_uInt16 nPos )
{
    assert(nPos < Count() && "PolyPolygon::Replace(): nPos >= nSize");

    mpImplPolyPolygon->mvPolyAry[nPos] = rPoly;
}

const tools::Polygon& PolyPolygon::GetObject( sal_uInt16 nPos ) const
{
    assert(nPos < Count() && "PolyPolygon::GetObject(): nPos >= nSize");

    return mpImplPolyPolygon->mvPolyAry[nPos];
}

bool PolyPolygon::IsRect() const
{
    bool bIsRect = false;
    if ( Count() == 1 )
        bIsRect = mpImplPolyPolygon->mvPolyAry[ 0 ].IsRect();
    return bIsRect;
}

void PolyPolygon::Clear()
{
    mpImplPolyPolygon->mvPolyAry.clear();
}

void PolyPolygon::Optimize( PolyOptimizeFlags nOptimizeFlags )
{
    if(!(bool(nOptimizeFlags) && Count()))
        return;

    // #115630# ImplDrawHatch does not work with beziers included in the polypolygon, take care of that
    bool bIsCurve(false);

    for(sal_uInt16 a(0); !bIsCurve && a < Count(); a++)
    {
        if((*this)[a].HasFlags())
        {
            bIsCurve = true;
        }
    }

    if(bIsCurve)
    {
        OSL_ENSURE(false, "Optimize does *not* support curves, falling back to AdaptiveSubdivide()...");
        tools::PolyPolygon aPolyPoly;

        AdaptiveSubdivide(aPolyPoly);
        aPolyPoly.Optimize(nOptimizeFlags);
        *this = aPolyPoly;
    }
    else
    {
        double      fArea;
        const bool  bEdges = ( nOptimizeFlags & PolyOptimizeFlags::EDGES ) == PolyOptimizeFlags::EDGES;
        sal_uInt16      nPercent = 0;

        if( bEdges )
        {
            const tools::Rectangle aBound( GetBoundRect() );

            fArea = ( aBound.GetWidth() + aBound.GetHeight() ) * 0.5;
            nPercent = 50;
            nOptimizeFlags &= ~PolyOptimizeFlags::EDGES;
        }

        // Optimize polygons
        for( sal_uInt16 i = 0, nPolyCount = mpImplPolyPolygon->mvPolyAry.size(); i < nPolyCount; i++ )
        {
            if( bEdges )
            {
                mpImplPolyPolygon->mvPolyAry[ i ].Optimize( PolyOptimizeFlags::NO_SAME );
                tools::Polygon::ImplReduceEdges( mpImplPolyPolygon->mvPolyAry[ i ], fArea, nPercent );
            }

            if( bool(nOptimizeFlags) )
                mpImplPolyPolygon->mvPolyAry[ i ].Optimize( nOptimizeFlags );
        }
    }
}

void PolyPolygon::AdaptiveSubdivide( tools::PolyPolygon& rResult ) const
{
    rResult.Clear();

    tools::Polygon aPolygon;

    for( size_t i = 0; i < mpImplPolyPolygon->mvPolyAry.size(); i++ )
    {
        mpImplPolyPolygon->mvPolyAry[ i ].AdaptiveSubdivide( aPolygon, 1.0 );
        rResult.Insert( aPolygon );
    }
}

tools::PolyPolygon PolyPolygon::SubdivideBezier( const tools::PolyPolygon& rPolyPoly )
{
    sal_uInt16 i, nPolys = rPolyPoly.Count();
    tools::PolyPolygon aPolyPoly( nPolys );
    for( i=0; i<nPolys; ++i )
        aPolyPoly.Insert( tools::Polygon::SubdivideBezier( rPolyPoly.GetObject(i) ) );

    return aPolyPoly;
}


void PolyPolygon::GetIntersection( const tools::PolyPolygon& rPolyPoly, tools::PolyPolygon& rResult ) const
{
    ImplDoOperation( rPolyPoly, rResult, PolyClipOp::INTERSECT );
}

void PolyPolygon::GetUnion( const tools::PolyPolygon& rPolyPoly, tools::PolyPolygon& rResult ) const
{
    ImplDoOperation( rPolyPoly, rResult, PolyClipOp::UNION );
}

void PolyPolygon::ImplDoOperation( const tools::PolyPolygon& rPolyPoly, tools::PolyPolygon& rResult, PolyClipOp nOperation ) const
{
    // Convert to B2DPolyPolygon, temporarily. It might be
    // advantageous in the future, to have a tools::PolyPolygon adaptor that
    // just simulates a B2DPolyPolygon here...
    basegfx::B2DPolyPolygon aMergePolyPolygonA( getB2DPolyPolygon() );
    basegfx::B2DPolyPolygon aMergePolyPolygonB( rPolyPoly.getB2DPolyPolygon() );

    // normalize the two polypolygons before. Force properly oriented
    // polygons.
    aMergePolyPolygonA = basegfx::utils::prepareForPolygonOperation( aMergePolyPolygonA );
    aMergePolyPolygonB = basegfx::utils::prepareForPolygonOperation( aMergePolyPolygonB );

    switch( nOperation )
    {
        // All code extracted from svx/source/svdraw/svedtv2.cxx

        case PolyClipOp::UNION:
        {
            // merge A and B (OR)
            aMergePolyPolygonA = basegfx::utils::solvePolygonOperationOr(aMergePolyPolygonA, aMergePolyPolygonB);
            break;
        }

        default:
        case PolyClipOp::INTERSECT:
        {
            // cut poly 1 against polys 2..n (AND)
            aMergePolyPolygonA = basegfx::utils::solvePolygonOperationAnd(aMergePolyPolygonA, aMergePolyPolygonB);
            break;
        }
    }

    rResult = tools::PolyPolygon( aMergePolyPolygonA );
}

sal_uInt16 PolyPolygon::Count() const
{
    return mpImplPolyPolygon->mvPolyAry.size();
}

void PolyPolygon::Move( long nHorzMove, long nVertMove )
{
    // Required for DrawEngine
    if( nHorzMove || nVertMove )
    {
        // move points
        sal_uInt16 nPolyCount = mpImplPolyPolygon->mvPolyAry.size();
        for ( sal_uInt16 i = 0; i < nPolyCount; i++ )
            mpImplPolyPolygon->mvPolyAry[i].Move( nHorzMove, nVertMove );
    }
}

void PolyPolygon::Translate( const Point& rTrans )
{
    // move points
    for ( sal_uInt16 i = 0, nCount = mpImplPolyPolygon->mvPolyAry.size(); i < nCount; i++ )
        mpImplPolyPolygon->mvPolyAry[ i ].Translate( rTrans );
}

void PolyPolygon::Scale( double fScaleX, double fScaleY )
{
    // Move points
    for ( sal_uInt16 i = 0, nCount = mpImplPolyPolygon->mvPolyAry.size(); i < nCount; i++ )
        mpImplPolyPolygon->mvPolyAry[ i ].Scale( fScaleX, fScaleY );
}

void PolyPolygon::Rotate( const Point& rCenter, sal_uInt16 nAngle10 )
{
    nAngle10 %= 3600;

    if( nAngle10 )
    {
        const double fAngle = F_PI1800 * nAngle10;
        Rotate( rCenter, sin( fAngle ), cos( fAngle ) );
    }
}

void PolyPolygon::Rotate( const Point& rCenter, double fSin, double fCos )
{
    // move points
    for ( sal_uInt16 i = 0, nCount = mpImplPolyPolygon->mvPolyAry.size(); i < nCount; i++ )
        mpImplPolyPolygon->mvPolyAry[ i ].Rotate( rCenter, fSin, fCos );
}

void PolyPolygon::Clip( const tools::Rectangle& rRect )
{
    sal_uInt16 nPolyCount = mpImplPolyPolygon->mvPolyAry.size();
    sal_uInt16 i;

    if ( !nPolyCount )
        return;

    // Clip every polygon, deleting the empty ones
    for ( i = 0; i < nPolyCount; i++ )
        mpImplPolyPolygon->mvPolyAry[i].Clip( rRect );
    while ( nPolyCount )
    {
        if ( GetObject( nPolyCount-1 ).GetSize() <= 2 )
            Remove( nPolyCount-1 );
        nPolyCount--;
    }
}

tools::Rectangle PolyPolygon::GetBoundRect() const
{
    long    nXMin=0, nXMax=0, nYMin=0, nYMax=0;
    bool    bFirst = true;
    sal_uInt16  nPolyCount = mpImplPolyPolygon->mvPolyAry.size();

    for ( sal_uInt16 n = 0; n < nPolyCount; n++ )
    {
        const tools::Polygon*  pPoly = &mpImplPolyPolygon->mvPolyAry[n];
        const Point*    pAry = pPoly->GetConstPointAry();
        sal_uInt16          nPointCount = pPoly->GetSize();

        for ( sal_uInt16 i = 0; i < nPointCount; i++ )
        {
            const Point* pPt = &pAry[ i ];

            if ( bFirst )
            {
                nXMin = nXMax = pPt->X();
                nYMin = nYMax = pPt->Y();
                bFirst = false;
            }
            else
            {
                if ( pPt->X() < nXMin )
                    nXMin = pPt->X();
                if ( pPt->X() > nXMax )
                    nXMax = pPt->X();
                if ( pPt->Y() < nYMin )
                    nYMin = pPt->Y();
                if ( pPt->Y() > nYMax )
                    nYMax = pPt->Y();
            }
        }
    }

    if ( !bFirst )
        return tools::Rectangle( nXMin, nYMin, nXMax, nYMax );
    else
        return tools::Rectangle();
}

Polygon& PolyPolygon::operator[]( sal_uInt16 nPos )
{
    assert(nPos < Count() && "PolyPolygon::[](): nPos >= nSize");

    return mpImplPolyPolygon->mvPolyAry[nPos];
}

PolyPolygon& PolyPolygon::operator=( const tools::PolyPolygon& rPolyPoly )
{
    mpImplPolyPolygon = rPolyPoly.mpImplPolyPolygon;
    return *this;
}

PolyPolygon& PolyPolygon::operator=( tools::PolyPolygon&& rPolyPoly ) noexcept
{
    mpImplPolyPolygon = std::move(rPolyPoly.mpImplPolyPolygon);
    return *this;
}

bool PolyPolygon::operator==( const tools::PolyPolygon& rPolyPoly ) const
{
    return rPolyPoly.mpImplPolyPolygon == mpImplPolyPolygon;
}

SvStream& ReadPolyPolygon( SvStream& rIStream, tools::PolyPolygon& rPolyPoly )
{
    sal_uInt16 nPolyCount(0);

    // Read number of polygons
    rIStream.ReadUInt16( nPolyCount );

    const size_t nMinRecordSize = sizeof(sal_uInt16);
    const size_t nMaxRecords = rIStream.remainingSize() / nMinRecordSize;
    if (nPolyCount > nMaxRecords)
    {
        SAL_WARN("tools", "Parsing error: " << nMaxRecords <<
                 " max possible entries, but " << nPolyCount << " claimed, truncating");
        nPolyCount = nMaxRecords;
    }

    if( nPolyCount )
    {
        rPolyPoly.mpImplPolyPolygon->mvPolyAry.resize(nPolyCount);

        tools::Polygon aTempPoly;
        for ( sal_uInt16 i = 0; i < nPolyCount; i++ )
        {
            ReadPolygon( rIStream, aTempPoly );
            rPolyPoly.mpImplPolyPolygon->mvPolyAry[i] = aTempPoly;
        }
    }
    else
        rPolyPoly = tools::PolyPolygon();

    return rIStream;
}

SvStream& WritePolyPolygon( SvStream& rOStream, const tools::PolyPolygon& rPolyPoly )
{
    // Write number of polygons
    sal_uInt16 nPolyCount = rPolyPoly.mpImplPolyPolygon->mvPolyAry.size();
    rOStream.WriteUInt16( nPolyCount );

    // output polygons
    for ( sal_uInt16 i = 0; i < nPolyCount; i++ )
        WritePolygon( rOStream, rPolyPoly.mpImplPolyPolygon->mvPolyAry[i] );

    return rOStream;
}

void PolyPolygon::Read( SvStream& rIStream )
{
    VersionCompat aCompat( rIStream, StreamMode::READ );

    sal_uInt16 nPolyCount(0);

    // Read number of polygons
    rIStream.ReadUInt16( nPolyCount );

    const size_t nMinRecordSize = sizeof(sal_uInt16);
    const size_t nMaxRecords = rIStream.remainingSize() / nMinRecordSize;
    if (nPolyCount > nMaxRecords)
    {
        SAL_WARN("tools", "Parsing error: " << nMaxRecords <<
                 " max possible entries, but " << nPolyCount << " claimed, truncating");
        nPolyCount = nMaxRecords;
    }

    if( nPolyCount )
    {
        mpImplPolyPolygon->mvPolyAry.clear();

        for ( sal_uInt16 i = 0; i < nPolyCount; i++ )
        {
            tools::Polygon aTempPoly;
            aTempPoly.ImplRead( rIStream );
            mpImplPolyPolygon->mvPolyAry.emplace_back( aTempPoly );
        }
    }
    else
        *this = tools::PolyPolygon();
}

void PolyPolygon::Write( SvStream& rOStream ) const
{
    VersionCompat aCompat( rOStream, StreamMode::WRITE, 1 );

    // Write number of polygons
    sal_uInt16 nPolyCount = mpImplPolyPolygon->mvPolyAry.size();
    rOStream.WriteUInt16( nPolyCount );

    // Output polygons
    for ( sal_uInt16 i = 0; i < nPolyCount; i++ )
        mpImplPolyPolygon->mvPolyAry[i].ImplWrite( rOStream );
}

// convert to basegfx::B2DPolyPolygon and return
basegfx::B2DPolyPolygon PolyPolygon::getB2DPolyPolygon() const
{
    basegfx::B2DPolyPolygon aRetval;

    for(size_t a(0); a < mpImplPolyPolygon->mvPolyAry.size(); a++)
    {
        tools::Polygon const & rCandidate = mpImplPolyPolygon->mvPolyAry[a];
        aRetval.append(rCandidate.getB2DPolygon());
    }

    return aRetval;
}

// constructor to convert from basegfx::B2DPolyPolygon
PolyPolygon::PolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon)
    : mpImplPolyPolygon(rPolyPolygon)
{
}

} /* namespace tools */

ImplPolyPolygon::ImplPolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    const sal_uInt16 nCount(sal_uInt16(rPolyPolygon.count()));
    DBG_ASSERT(sal_uInt32(nCount) == rPolyPolygon.count(),
        "PolyPolygon::PolyPolygon: Too many sub-polygons in given basegfx::B2DPolyPolygon (!)");

    if ( nCount )
    {
        mvPolyAry.resize( nCount );

        for(sal_uInt16 a(0); a < nCount; a++)
        {
            const basegfx::B2DPolygon& aCandidate(rPolyPolygon.getB2DPolygon(sal_uInt32(a)));
            mvPolyAry[a] = tools::Polygon( aCandidate );
        }
    }
    else
       mvPolyAry.reserve(16);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
