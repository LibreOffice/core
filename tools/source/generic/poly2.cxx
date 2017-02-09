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

#include <rtl/math.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <poly.h>
#include <tools/poly.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <tools/gen.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>

ImplPolyPolygon::ImplPolyPolygon( sal_uInt16 nInitSize )
{
    mnRefCount  = 1;
    mnCount     = nInitSize;
    mnSize      = nInitSize;
    mnResize    = 16;
    mpPolyAry   = new tools::Polygon*[ nInitSize ];
}

ImplPolyPolygon::ImplPolyPolygon( const ImplPolyPolygon& rImplPolyPoly )
{
    mnRefCount  = 1;
    mnCount     = rImplPolyPoly.mnCount;
    mnSize      = rImplPolyPoly.mnSize;
    mnResize    = rImplPolyPoly.mnResize;

    if ( rImplPolyPoly.mpPolyAry )
    {
        mpPolyAry = new tools::Polygon*[mnSize];
        for ( sal_uInt16 i = 0; i < mnCount; i++ )
            mpPolyAry[i] = new tools::Polygon( *rImplPolyPoly.mpPolyAry[i] );
    }
    else
        mpPolyAry = nullptr;
}

ImplPolyPolygon::~ImplPolyPolygon()
{
    if ( mpPolyAry )
    {
        for ( sal_uInt16 i = 0; i < mnCount; i++ )
            delete mpPolyAry[i];
        delete[] mpPolyAry;
    }
}

namespace tools {

PolyPolygon::PolyPolygon( sal_uInt16 nInitSize, sal_uInt16 nResize )
{
    if ( nInitSize > MAX_POLYGONS )
        nInitSize = MAX_POLYGONS;
    else if ( !nInitSize )
        nInitSize = 1;
    if ( nResize > MAX_POLYGONS )
        nResize = MAX_POLYGONS;
    else if ( !nResize )
        nResize = 1;
    mpImplPolyPolygon = new ImplPolyPolygon( nInitSize, nResize );
}

PolyPolygon::PolyPolygon( const tools::Polygon& rPoly )
{
    if ( rPoly.GetSize() )
    {
        mpImplPolyPolygon = new ImplPolyPolygon( 1 );
        mpImplPolyPolygon->mpPolyAry[0] = new tools::Polygon( rPoly );
    }
    else
        mpImplPolyPolygon = new ImplPolyPolygon( 16, 16 );
}

PolyPolygon::PolyPolygon( const tools::PolyPolygon& rPolyPoly )
{
    DBG_ASSERT( rPolyPoly.mpImplPolyPolygon->mnRefCount < (SAL_MAX_UINT32-1), "PolyPolygon: RefCount overflow" );

    mpImplPolyPolygon = rPolyPoly.mpImplPolyPolygon;
    mpImplPolyPolygon->mnRefCount++;
}

PolyPolygon::~PolyPolygon()
{
    if ( mpImplPolyPolygon->mnRefCount > 1 )
        mpImplPolyPolygon->mnRefCount--;
    else
        delete mpImplPolyPolygon;
}

void PolyPolygon::Insert( const tools::Polygon& rPoly, sal_uInt16 nPos )
{
    if ( mpImplPolyPolygon->mnCount >= MAX_POLYGONS )
        return;

    if ( mpImplPolyPolygon->mnRefCount > 1 )
    {
        mpImplPolyPolygon->mnRefCount--;
        mpImplPolyPolygon = new ImplPolyPolygon( *mpImplPolyPolygon );
    }

    if ( nPos > mpImplPolyPolygon->mnCount )
        nPos = mpImplPolyPolygon->mnCount;

    if ( !mpImplPolyPolygon->mpPolyAry )
        mpImplPolyPolygon->mpPolyAry = new tools::Polygon*[mpImplPolyPolygon->mnSize];
    else if ( mpImplPolyPolygon->mnCount == mpImplPolyPolygon->mnSize )
    {
        sal_uInt16      nOldSize = mpImplPolyPolygon->mnSize;
        sal_uInt16      nNewSize = nOldSize + mpImplPolyPolygon->mnResize;
        tools::Polygon** pNewAry;

        if ( nNewSize >= MAX_POLYGONS )
            nNewSize = MAX_POLYGONS;
        pNewAry = new tools::Polygon*[nNewSize];
        memcpy( pNewAry, mpImplPolyPolygon->mpPolyAry, nPos*sizeof(Polygon*) );
        memcpy( pNewAry+nPos+1, mpImplPolyPolygon->mpPolyAry+nPos,
                (nOldSize-nPos)*sizeof(Polygon*) );
        delete[] mpImplPolyPolygon->mpPolyAry;
        mpImplPolyPolygon->mpPolyAry = pNewAry;
        mpImplPolyPolygon->mnSize = nNewSize;
    }
    else if ( nPos < mpImplPolyPolygon->mnCount )
    {
        memmove( mpImplPolyPolygon->mpPolyAry+nPos+1,
                 mpImplPolyPolygon->mpPolyAry+nPos,
                 (mpImplPolyPolygon->mnCount-nPos)*sizeof(Polygon*) );
    }

    mpImplPolyPolygon->mpPolyAry[nPos] = new tools::Polygon( rPoly );
    mpImplPolyPolygon->mnCount++;
}

void PolyPolygon::Remove( sal_uInt16 nPos )
{
    assert(nPos < Count() && "PolyPolygon::Remove(): nPos >= nSize");

    if ( mpImplPolyPolygon->mnRefCount > 1 )
    {
        mpImplPolyPolygon->mnRefCount--;
        mpImplPolyPolygon = new ImplPolyPolygon( *mpImplPolyPolygon );
    }

    delete mpImplPolyPolygon->mpPolyAry[nPos];
    mpImplPolyPolygon->mnCount--;
    memmove( mpImplPolyPolygon->mpPolyAry+nPos,
             mpImplPolyPolygon->mpPolyAry+nPos+1,
             (mpImplPolyPolygon->mnCount-nPos)*sizeof(Polygon*) );
}

void PolyPolygon::Replace( const tools::Polygon& rPoly, sal_uInt16 nPos )
{
    assert(nPos < Count() && "PolyPolygon::Replace(): nPos >= nSize");

    if ( mpImplPolyPolygon->mnRefCount > 1 )
    {
        mpImplPolyPolygon->mnRefCount--;
        mpImplPolyPolygon = new ImplPolyPolygon( *mpImplPolyPolygon );
    }

    delete mpImplPolyPolygon->mpPolyAry[nPos];
    mpImplPolyPolygon->mpPolyAry[nPos] = new tools::Polygon( rPoly );
}

const tools::Polygon& PolyPolygon::GetObject( sal_uInt16 nPos ) const
{
    assert(nPos < Count() && "PolyPolygon::GetObject(): nPos >= nSize");

    return *(mpImplPolyPolygon->mpPolyAry[nPos]);
}

bool PolyPolygon::IsRect() const
{
    bool bIsRect = false;
    if ( Count() == 1 )
        bIsRect = mpImplPolyPolygon->mpPolyAry[ 0 ]->IsRect();
    return bIsRect;
}

void PolyPolygon::Clear()
{
    if ( mpImplPolyPolygon->mnRefCount > 1 )
    {
        mpImplPolyPolygon->mnRefCount--;
        mpImplPolyPolygon = new ImplPolyPolygon( mpImplPolyPolygon->mnResize,
                                                 mpImplPolyPolygon->mnResize );
    }
    else
    {
        if ( mpImplPolyPolygon->mpPolyAry )
        {
            for ( sal_uInt16 i = 0; i < mpImplPolyPolygon->mnCount; i++ )
                delete mpImplPolyPolygon->mpPolyAry[i];
            delete[] mpImplPolyPolygon->mpPolyAry;
            mpImplPolyPolygon->mpPolyAry = nullptr;
            mpImplPolyPolygon->mnCount   = 0;
            mpImplPolyPolygon->mnSize    = mpImplPolyPolygon->mnResize;
        }
    }
}

void PolyPolygon::Optimize( PolyOptimizeFlags nOptimizeFlags )
{
    if(bool(nOptimizeFlags) && Count())
    {
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
                const Rectangle aBound( GetBoundRect() );

                fArea = ( aBound.GetWidth() + aBound.GetHeight() ) * 0.5;
                nPercent = 50;
                nOptimizeFlags &= ~PolyOptimizeFlags::EDGES;
            }

            // watch for ref counter
            if( mpImplPolyPolygon->mnRefCount > 1 )
            {
                mpImplPolyPolygon->mnRefCount--;
                mpImplPolyPolygon = new ImplPolyPolygon( *mpImplPolyPolygon );
            }

            // Optimize polygons
            for( sal_uInt16 i = 0, nPolyCount = mpImplPolyPolygon->mnCount; i < nPolyCount; i++ )
            {
                if( bEdges )
                {
                    mpImplPolyPolygon->mpPolyAry[ i ]->Optimize( PolyOptimizeFlags::NO_SAME );
                    tools::Polygon::ImplReduceEdges( *( mpImplPolyPolygon->mpPolyAry[ i ] ), fArea, nPercent );
                }

                if( bool(nOptimizeFlags) )
                    mpImplPolyPolygon->mpPolyAry[ i ]->Optimize( nOptimizeFlags );
            }
        }
    }
}

void PolyPolygon::AdaptiveSubdivide( tools::PolyPolygon& rResult ) const
{
    rResult.Clear();

    tools::Polygon aPolygon;

    for( sal_uInt16 i = 0; i < mpImplPolyPolygon->mnCount; i++ )
    {
        mpImplPolyPolygon->mpPolyAry[ i ]->AdaptiveSubdivide( aPolygon, 1.0 );
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
    aMergePolyPolygonA = basegfx::tools::prepareForPolygonOperation( aMergePolyPolygonA );
    aMergePolyPolygonB = basegfx::tools::prepareForPolygonOperation( aMergePolyPolygonB );

    switch( nOperation )
    {
        // All code extracted from svx/source/svdraw/svedtv2.cxx

        case PolyClipOp::UNION:
        {
            // merge A and B (OR)
            aMergePolyPolygonA = basegfx::tools::solvePolygonOperationOr(aMergePolyPolygonA, aMergePolyPolygonB);
            break;
        }

        default:
        case PolyClipOp::INTERSECT:
        {
            // cut poly 1 against polys 2..n (AND)
            aMergePolyPolygonA = basegfx::tools::solvePolygonOperationAnd(aMergePolyPolygonA, aMergePolyPolygonB);
            break;
        }
    }

    rResult = tools::PolyPolygon( aMergePolyPolygonA );
}

sal_uInt16 PolyPolygon::Count() const
{
    return mpImplPolyPolygon->mnCount;
}

void PolyPolygon::Move( long nHorzMove, long nVertMove )
{
    // Required for DrawEngine
    if( nHorzMove || nVertMove )
    {
        if ( mpImplPolyPolygon->mnRefCount > 1 )
        {
            mpImplPolyPolygon->mnRefCount--;
            mpImplPolyPolygon = new ImplPolyPolygon( *mpImplPolyPolygon );
        }

        // move points
        sal_uInt16 nPolyCount = mpImplPolyPolygon->mnCount;
        for ( sal_uInt16 i = 0; i < nPolyCount; i++ )
            mpImplPolyPolygon->mpPolyAry[i]->Move( nHorzMove, nVertMove );
    }
}

void PolyPolygon::Translate( const Point& rTrans )
{
    if( mpImplPolyPolygon->mnRefCount > 1 )
    {
        mpImplPolyPolygon->mnRefCount--;
        mpImplPolyPolygon = new ImplPolyPolygon( *mpImplPolyPolygon );
    }

    // move points
    for ( sal_uInt16 i = 0, nCount = mpImplPolyPolygon->mnCount; i < nCount; i++ )
        mpImplPolyPolygon->mpPolyAry[ i ]->Translate( rTrans );
}

void PolyPolygon::Scale( double fScaleX, double fScaleY )
{
    if( mpImplPolyPolygon->mnRefCount > 1 )
    {
        mpImplPolyPolygon->mnRefCount--;
        mpImplPolyPolygon = new ImplPolyPolygon( *mpImplPolyPolygon );
    }

    // Move points
    for ( sal_uInt16 i = 0, nCount = mpImplPolyPolygon->mnCount; i < nCount; i++ )
        mpImplPolyPolygon->mpPolyAry[ i ]->Scale( fScaleX, fScaleY );
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
    if( mpImplPolyPolygon->mnRefCount > 1 )
    {
        mpImplPolyPolygon->mnRefCount--;
        mpImplPolyPolygon = new ImplPolyPolygon( *mpImplPolyPolygon );
    }

    // move points
    for ( sal_uInt16 i = 0, nCount = mpImplPolyPolygon->mnCount; i < nCount; i++ )
        mpImplPolyPolygon->mpPolyAry[ i ]->Rotate( rCenter, fSin, fCos );
}

void PolyPolygon::Clip( const Rectangle& rRect )
{
    sal_uInt16 nPolyCount = mpImplPolyPolygon->mnCount;
    sal_uInt16 i;

    if ( !nPolyCount )
        return;

    if ( mpImplPolyPolygon->mnRefCount > 1 )
    {
        mpImplPolyPolygon->mnRefCount--;
        mpImplPolyPolygon = new ImplPolyPolygon( *mpImplPolyPolygon );
    }

    // Clip every polygon, deleting the empty ones
    for ( i = 0; i < nPolyCount; i++ )
        mpImplPolyPolygon->mpPolyAry[i]->Clip( rRect );
    while ( nPolyCount )
    {
        if ( GetObject( nPolyCount-1 ).GetSize() <= 2 )
            Remove( nPolyCount-1 );
        nPolyCount--;
    }
}

Rectangle PolyPolygon::GetBoundRect() const
{
    long    nXMin=0, nXMax=0, nYMin=0, nYMax=0;
    bool    bFirst = true;
    sal_uInt16  nPolyCount = mpImplPolyPolygon->mnCount;

    for ( sal_uInt16 n = 0; n < nPolyCount; n++ )
    {
        const tools::Polygon*  pPoly = mpImplPolyPolygon->mpPolyAry[n];
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
        return Rectangle( nXMin, nYMin, nXMax, nYMax );
    else
        return Rectangle();
}

Polygon& PolyPolygon::operator[]( sal_uInt16 nPos )
{
    assert(nPos < Count() && "PolyPolygon::[](): nPos >= nSize");

    if ( mpImplPolyPolygon->mnRefCount > 1 )
    {
        mpImplPolyPolygon->mnRefCount--;
        mpImplPolyPolygon = new ImplPolyPolygon( *mpImplPolyPolygon );
    }

    return *(mpImplPolyPolygon->mpPolyAry[nPos]);
}

PolyPolygon& PolyPolygon::operator=( const tools::PolyPolygon& rPolyPoly )
{
    if (this == &rPolyPoly)
        return *this;

    DBG_ASSERT( rPolyPoly.mpImplPolyPolygon->mnRefCount < (SAL_MAX_UINT32-1), "PolyPolygon: RefCount overflow" );

    rPolyPoly.mpImplPolyPolygon->mnRefCount++;

    if ( mpImplPolyPolygon->mnRefCount > 1 )
        mpImplPolyPolygon->mnRefCount--;
    else
        delete mpImplPolyPolygon;

    mpImplPolyPolygon = rPolyPoly.mpImplPolyPolygon;
    return *this;
}

PolyPolygon& PolyPolygon::operator=( tools::PolyPolygon&& rPolyPoly )
{
    std::swap(mpImplPolyPolygon, rPolyPoly.mpImplPolyPolygon);
    return *this;
}

bool PolyPolygon::operator==( const tools::PolyPolygon& rPolyPoly ) const
{
    if ( rPolyPoly.mpImplPolyPolygon == mpImplPolyPolygon )
        return true;
    else
        return false;
}

SvStream& ReadPolyPolygon( SvStream& rIStream, tools::PolyPolygon& rPolyPoly )
{
    tools::Polygon* pPoly;
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
        if ( rPolyPoly.mpImplPolyPolygon->mnRefCount > 1 )
            rPolyPoly.mpImplPolyPolygon->mnRefCount--;
        else
            delete rPolyPoly.mpImplPolyPolygon;

        rPolyPoly.mpImplPolyPolygon = new ImplPolyPolygon( nPolyCount );

        for ( sal_uInt16 i = 0; i < nPolyCount; i++ )
        {
            pPoly = new tools::Polygon;
            ReadPolygon( rIStream, *pPoly );
            rPolyPoly.mpImplPolyPolygon->mpPolyAry[i] = pPoly;
        }
    }
    else
        rPolyPoly = tools::PolyPolygon();

    return rIStream;
}

SvStream& WritePolyPolygon( SvStream& rOStream, const tools::PolyPolygon& rPolyPoly )
{
    // Write number of polygons
    sal_uInt16 nPolyCount = rPolyPoly.mpImplPolyPolygon->mnCount;
    rOStream.WriteUInt16( nPolyCount );

    // output polygons
    for ( sal_uInt16 i = 0; i < nPolyCount; i++ )
        WritePolygon( rOStream, *(rPolyPoly.mpImplPolyPolygon->mpPolyAry[i]) );

    return rOStream;
}

void PolyPolygon::Read( SvStream& rIStream )
{
    VersionCompat aCompat( rIStream, StreamMode::READ );

    tools::Polygon* pPoly;
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
        if ( mpImplPolyPolygon->mnRefCount > 1 )
            mpImplPolyPolygon->mnRefCount--;
        else
            delete mpImplPolyPolygon;

        mpImplPolyPolygon = new ImplPolyPolygon( nPolyCount );

        for ( sal_uInt16 i = 0; i < nPolyCount; i++ )
        {
            pPoly = new tools::Polygon;
            pPoly->ImplRead( rIStream );
            mpImplPolyPolygon->mpPolyAry[i] = pPoly;
        }
    }
    else
        *this = tools::PolyPolygon();
}

void PolyPolygon::Write( SvStream& rOStream ) const
{
    VersionCompat aCompat( rOStream, StreamMode::WRITE, 1 );

    // Write number of polygons
    sal_uInt16 nPolyCount = mpImplPolyPolygon->mnCount;
    rOStream.WriteUInt16( nPolyCount );

    // Output polygons
    for ( sal_uInt16 i = 0; i < nPolyCount; i++ )
        mpImplPolyPolygon->mpPolyAry[i]->ImplWrite( rOStream );
}

// convert to basegfx::B2DPolyPolygon and return
basegfx::B2DPolyPolygon PolyPolygon::getB2DPolyPolygon() const
{
    basegfx::B2DPolyPolygon aRetval;

    for(sal_uInt16 a(0); a < mpImplPolyPolygon->mnCount; a++)
    {
        tools::Polygon* pCandidate = mpImplPolyPolygon->mpPolyAry[a];
        aRetval.append(pCandidate->getB2DPolygon());
    }

    return aRetval;
}

// constructor to convert from basegfx::B2DPolyPolygon
PolyPolygon::PolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    const sal_uInt16 nCount(sal_uInt16(rPolyPolygon.count()));
    DBG_ASSERT(sal_uInt32(nCount) == rPolyPolygon.count(),
        "PolyPolygon::PolyPolygon: Too many sub-polygons in given basegfx::B2DPolyPolygon (!)");

    if ( nCount )
    {
        mpImplPolyPolygon = new ImplPolyPolygon( nCount );

        for(sal_uInt16 a(0); a < nCount; a++)
        {
            basegfx::B2DPolygon aCandidate(rPolyPolygon.getB2DPolygon(sal_uInt32(a)));
            mpImplPolyPolygon->mpPolyAry[a] = new tools::Polygon( aCandidate );
        }
    }
    else
    {
        mpImplPolyPolygon = new ImplPolyPolygon( 16, 16 );
    }
}

} /* namespace tools */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
