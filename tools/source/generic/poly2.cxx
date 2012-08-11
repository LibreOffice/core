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

#define POLY_CLIP_INT   0
#define POLY_CLIP_UNION 1
#define POLY_CLIP_DIFF  2
#define POLY_CLIP_XOR   3

#include <rtl/math.hxx>
#include <poly.h>
#include <tools/poly.hxx>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>

DBG_NAME( PolyPolygon )

ImplPolyPolygon::ImplPolyPolygon( sal_uInt16 nInitSize )
{
    mnRefCount  = 1;
    mnCount     = nInitSize;
    mnSize      = nInitSize;
    mnResize    = 16;
    mpPolyAry   = new SVPPOLYGON[ nInitSize ];
}

ImplPolyPolygon::ImplPolyPolygon( const ImplPolyPolygon& rImplPolyPoly )
{
    mnRefCount  = 1;
    mnCount     = rImplPolyPoly.mnCount;
    mnSize      = rImplPolyPoly.mnSize;
    mnResize    = rImplPolyPoly.mnResize;

    if ( rImplPolyPoly.mpPolyAry )
    {
        mpPolyAry = new SVPPOLYGON[mnSize];
        for ( sal_uInt16 i = 0; i < mnCount; i++ )
            mpPolyAry[i] = new Polygon( *rImplPolyPoly.mpPolyAry[i] );
    }
    else
        mpPolyAry = NULL;
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

PolyPolygon::PolyPolygon( sal_uInt16 nInitSize, sal_uInt16 nResize )
{
    DBG_CTOR( PolyPolygon, NULL );

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

PolyPolygon::PolyPolygon( const Polygon& rPoly )
{
    DBG_CTOR( PolyPolygon, NULL );

    if ( rPoly.GetSize() )
    {
        mpImplPolyPolygon = new ImplPolyPolygon( 1 );
        mpImplPolyPolygon->mpPolyAry[0] = new Polygon( rPoly );
    }
    else
        mpImplPolyPolygon = new ImplPolyPolygon( 16, 16 );
}

PolyPolygon::PolyPolygon( sal_uInt16 nPoly, const sal_uInt16* pPointCountAry,
                          const Point* pPtAry )
{
    DBG_CTOR( PolyPolygon, NULL );

    if ( nPoly > MAX_POLYGONS )
        nPoly = MAX_POLYGONS;

    mpImplPolyPolygon = new ImplPolyPolygon( nPoly );
    for ( sal_uInt16 i = 0; i < nPoly; i++ )
    {
        mpImplPolyPolygon->mpPolyAry[i] = new Polygon( *pPointCountAry, pPtAry );
        pPtAry += *pPointCountAry;
        pPointCountAry++;
    }
}

PolyPolygon::PolyPolygon( const PolyPolygon& rPolyPoly )
{
    DBG_CTOR( PolyPolygon, NULL );
    DBG_CHKOBJ( &rPolyPoly, PolyPolygon, NULL );
    DBG_ASSERT( rPolyPoly.mpImplPolyPolygon->mnRefCount < 0xFFFFFFFE, "PolyPolygon: RefCount overflow" );

    mpImplPolyPolygon = rPolyPoly.mpImplPolyPolygon;
    mpImplPolyPolygon->mnRefCount++;
}

PolyPolygon::~PolyPolygon()
{
    DBG_DTOR( PolyPolygon, NULL );

    if ( mpImplPolyPolygon->mnRefCount > 1 )
        mpImplPolyPolygon->mnRefCount--;
    else
        delete mpImplPolyPolygon;
}

void PolyPolygon::Insert( const Polygon& rPoly, sal_uInt16 nPos )
{
    DBG_CHKTHIS( PolyPolygon, NULL );

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
        mpImplPolyPolygon->mpPolyAry = new SVPPOLYGON[mpImplPolyPolygon->mnSize];
    else if ( mpImplPolyPolygon->mnCount == mpImplPolyPolygon->mnSize )
    {
        sal_uInt16      nOldSize = mpImplPolyPolygon->mnSize;
        sal_uInt16      nNewSize = nOldSize + mpImplPolyPolygon->mnResize;
        SVPPOLYGON* pNewAry;

        if ( nNewSize >= MAX_POLYGONS )
            nNewSize = MAX_POLYGONS;
        pNewAry = new SVPPOLYGON[nNewSize];
        memcpy( pNewAry, mpImplPolyPolygon->mpPolyAry, nPos*sizeof(SVPPOLYGON) );
        memcpy( pNewAry+nPos+1, mpImplPolyPolygon->mpPolyAry+nPos,
                (nOldSize-nPos)*sizeof(SVPPOLYGON) );
        delete[] mpImplPolyPolygon->mpPolyAry;
        mpImplPolyPolygon->mpPolyAry = pNewAry;
        mpImplPolyPolygon->mnSize = nNewSize;
    }
    else if ( nPos < mpImplPolyPolygon->mnCount )
    {
        memmove( mpImplPolyPolygon->mpPolyAry+nPos+1,
                 mpImplPolyPolygon->mpPolyAry+nPos,
                 (mpImplPolyPolygon->mnCount-nPos)*sizeof(SVPPOLYGON) );
    }

    mpImplPolyPolygon->mpPolyAry[nPos] = new Polygon( rPoly );
    mpImplPolyPolygon->mnCount++;
}

void PolyPolygon::Remove( sal_uInt16 nPos )
{
    DBG_CHKTHIS( PolyPolygon, NULL );
    DBG_ASSERT( nPos < Count(), "PolyPolygon::Remove(): nPos >= nSize" );

    if ( mpImplPolyPolygon->mnRefCount > 1 )
    {
        mpImplPolyPolygon->mnRefCount--;
        mpImplPolyPolygon = new ImplPolyPolygon( *mpImplPolyPolygon );
    }

    delete mpImplPolyPolygon->mpPolyAry[nPos];
    mpImplPolyPolygon->mnCount--;
    memmove( mpImplPolyPolygon->mpPolyAry+nPos,
             mpImplPolyPolygon->mpPolyAry+nPos+1,
             (mpImplPolyPolygon->mnCount-nPos)*sizeof(SVPPOLYGON) );
}

void PolyPolygon::Replace( const Polygon& rPoly, sal_uInt16 nPos )
{
    DBG_CHKTHIS( PolyPolygon, NULL );
    DBG_ASSERT( nPos < Count(), "PolyPolygon::Replace(): nPos >= nSize" );

    if ( mpImplPolyPolygon->mnRefCount > 1 )
    {
        mpImplPolyPolygon->mnRefCount--;
        mpImplPolyPolygon = new ImplPolyPolygon( *mpImplPolyPolygon );
    }

    delete mpImplPolyPolygon->mpPolyAry[nPos];
    mpImplPolyPolygon->mpPolyAry[nPos] = new Polygon( rPoly );
}

const Polygon& PolyPolygon::GetObject( sal_uInt16 nPos ) const
{
    DBG_CHKTHIS( PolyPolygon, NULL );
    DBG_ASSERT( nPos < Count(), "PolyPolygon::GetObject(): nPos >= nSize" );

    return *(mpImplPolyPolygon->mpPolyAry[nPos]);
}

sal_Bool PolyPolygon::IsRect() const
{
    sal_Bool bIsRect = sal_False;
    if ( Count() == 1 )
        bIsRect = mpImplPolyPolygon->mpPolyAry[ 0 ]->IsRect();
    return bIsRect;
}

void PolyPolygon::Clear()
{
    DBG_CHKTHIS( PolyPolygon, NULL );

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
            mpImplPolyPolygon->mpPolyAry = NULL;
            mpImplPolyPolygon->mnCount   = 0;
            mpImplPolyPolygon->mnSize    = mpImplPolyPolygon->mnResize;
        }
    }
}

void PolyPolygon::Optimize( sal_uIntPtr nOptimizeFlags, const PolyOptimizeData* pData )
{
    DBG_CHKTHIS( PolyPolygon, NULL );

    if( nOptimizeFlags )
    {
        double      fArea;
        const sal_Bool  bEdges = ( nOptimizeFlags & POLY_OPTIMIZE_EDGES ) == POLY_OPTIMIZE_EDGES;
        sal_uInt16      nPercent = 0;

        if( bEdges )
        {
            const Rectangle aBound( GetBoundRect() );

            fArea = ( aBound.GetWidth() + aBound.GetHeight() ) * 0.5;
            nPercent = pData ? pData->GetPercentValue() : 50;
            nOptimizeFlags &= ~POLY_OPTIMIZE_EDGES;
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
                mpImplPolyPolygon->mpPolyAry[ i ]->Optimize( POLY_OPTIMIZE_NO_SAME );
                Polygon::ImplReduceEdges( *( mpImplPolyPolygon->mpPolyAry[ i ] ), fArea, nPercent );
            }

            if( nOptimizeFlags )
                mpImplPolyPolygon->mpPolyAry[ i ]->Optimize( nOptimizeFlags, pData );
        }
    }
}

void PolyPolygon::AdaptiveSubdivide( PolyPolygon& rResult, const double d ) const
{
    DBG_CHKTHIS( PolyPolygon, NULL );

    rResult.Clear();

    Polygon aPolygon;

    for( sal_uInt16 i = 0; i < mpImplPolyPolygon->mnCount; i++ )
    {
        mpImplPolyPolygon->mpPolyAry[ i ]->AdaptiveSubdivide( aPolygon, d );
        rResult.Insert( aPolygon );
    }
}

void PolyPolygon::GetIntersection( const PolyPolygon& rPolyPoly, PolyPolygon& rResult ) const
{
    ImplDoOperation( rPolyPoly, rResult, POLY_CLIP_INT );
}

void PolyPolygon::GetUnion( const PolyPolygon& rPolyPoly, PolyPolygon& rResult ) const
{
    ImplDoOperation( rPolyPoly, rResult, POLY_CLIP_UNION );
}

void PolyPolygon::GetDifference( const PolyPolygon& rPolyPoly, PolyPolygon& rResult ) const
{
    ImplDoOperation( rPolyPoly, rResult, POLY_CLIP_DIFF );
}

void PolyPolygon::GetXOR( const PolyPolygon& rPolyPoly, PolyPolygon& rResult ) const
{
    ImplDoOperation( rPolyPoly, rResult, POLY_CLIP_XOR );
}

void PolyPolygon::ImplDoOperation( const PolyPolygon& rPolyPoly, PolyPolygon& rResult, sal_uIntPtr nOperation ) const
{
    // Convert to B2DPolyPolygon, temporarily. It might be
    // advantageous in the future, to have a PolyPolygon adaptor that
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

        case POLY_CLIP_UNION:
        {
            // merge A and B (OR)
            aMergePolyPolygonA = basegfx::tools::solvePolygonOperationOr(aMergePolyPolygonA, aMergePolyPolygonB);
            break;
        }

        case POLY_CLIP_DIFF:
        {
            // substract B from A (DIFF)
            aMergePolyPolygonA = basegfx::tools::solvePolygonOperationDiff(aMergePolyPolygonA, aMergePolyPolygonB);
            break;
        }

        case POLY_CLIP_XOR:
        {
            // compute XOR between poly A and B
            aMergePolyPolygonA = basegfx::tools::solvePolygonOperationXor(aMergePolyPolygonA, aMergePolyPolygonB);
            break;
        }

        default:
        case POLY_CLIP_INT:
        {
            // cut poly 1 against polys 2..n (AND)
            aMergePolyPolygonA = basegfx::tools::solvePolygonOperationAnd(aMergePolyPolygonA, aMergePolyPolygonB);
            break;
        }
    }

    rResult = PolyPolygon( aMergePolyPolygonA );
}

sal_uInt16 PolyPolygon::Count() const
{
    DBG_CHKTHIS( PolyPolygon, NULL );
    return mpImplPolyPolygon->mnCount;
}

void PolyPolygon::Move( long nHorzMove, long nVertMove )
{
    DBG_CHKTHIS( PolyPolygon, NULL );

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
    DBG_CHKTHIS( PolyPolygon, NULL );

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
    DBG_CHKTHIS( PolyPolygon, NULL );

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
    DBG_CHKTHIS( PolyPolygon, NULL );
    nAngle10 %= 3600;

    if( nAngle10 )
    {
        const double fAngle = F_PI1800 * nAngle10;
        Rotate( rCenter, sin( fAngle ), cos( fAngle ) );
    }
}

void PolyPolygon::Rotate( const Point& rCenter, double fSin, double fCos )
{
    DBG_CHKTHIS( PolyPolygon, NULL );

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
    DBG_CHKTHIS( PolyPolygon, NULL );

    long    nXMin=0, nXMax=0, nYMin=0, nYMax=0;
    sal_Bool    bFirst = sal_True;
    sal_uInt16  nPolyCount = mpImplPolyPolygon->mnCount;

    for ( sal_uInt16 n = 0; n < nPolyCount; n++ )
    {
        const Polygon*  pPoly = mpImplPolyPolygon->mpPolyAry[n];
        const Point*    pAry = pPoly->GetConstPointAry();
        sal_uInt16          nPointCount = pPoly->GetSize();

        for ( sal_uInt16 i = 0; i < nPointCount; i++ )
        {
            const Point* pPt = &pAry[ i ];

            if ( bFirst )
            {
                nXMin = nXMax = pPt->X();
                nYMin = nYMax = pPt->Y();
                bFirst = sal_False;
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
    DBG_CHKTHIS( PolyPolygon, NULL );
    DBG_ASSERT( nPos < Count(), "PolyPolygon::[](): nPos >= nSize" );

    if ( mpImplPolyPolygon->mnRefCount > 1 )
    {
        mpImplPolyPolygon->mnRefCount--;
        mpImplPolyPolygon = new ImplPolyPolygon( *mpImplPolyPolygon );
    }

    return *(mpImplPolyPolygon->mpPolyAry[nPos]);
}

PolyPolygon& PolyPolygon::operator=( const PolyPolygon& rPolyPoly )
{
    DBG_CHKTHIS( PolyPolygon, NULL );
    DBG_CHKOBJ( &rPolyPoly, PolyPolygon, NULL );
    DBG_ASSERT( rPolyPoly.mpImplPolyPolygon->mnRefCount < 0xFFFFFFFE, "PolyPolygon: RefCount overflow" );

    rPolyPoly.mpImplPolyPolygon->mnRefCount++;

    if ( mpImplPolyPolygon->mnRefCount > 1 )
        mpImplPolyPolygon->mnRefCount--;
    else
        delete mpImplPolyPolygon;

    mpImplPolyPolygon = rPolyPoly.mpImplPolyPolygon;
    return *this;
}

sal_Bool PolyPolygon::operator==( const PolyPolygon& rPolyPoly ) const
{
    DBG_CHKTHIS( PolyPolygon, NULL );
    DBG_CHKOBJ( &rPolyPoly, PolyPolygon, NULL );

    if ( rPolyPoly.mpImplPolyPolygon == mpImplPolyPolygon )
        return sal_True;
    else
        return sal_False;
}

sal_Bool PolyPolygon::IsEqual( const PolyPolygon& rPolyPoly ) const
{
    sal_Bool bIsEqual = sal_True;
    if ( Count() != rPolyPoly.Count() )
        bIsEqual = sal_False;
    else
    {
        sal_uInt16 i;
        for ( i = 0; i < Count(); i++ )
        {
            if (!GetObject( i ).IsEqual( rPolyPoly.GetObject( i ) ) )
            {
                bIsEqual = sal_False;
                break;
            }
        }
    }
    return bIsEqual;
}

SvStream& operator>>( SvStream& rIStream, PolyPolygon& rPolyPoly )
{
    DBG_CHKOBJ( &rPolyPoly, PolyPolygon, NULL );
    DBG_ASSERTWARNING( rIStream.GetVersion(), "PolyPolygon::>> - Solar-Version not set on rIStream" );

    Polygon* pPoly;
    sal_uInt16   nPolyCount;

    // read number of polygons
    rIStream >> nPolyCount;

    if( nPolyCount )
    {
        if ( rPolyPoly.mpImplPolyPolygon->mnRefCount > 1 )
            rPolyPoly.mpImplPolyPolygon->mnRefCount--;
        else
            delete rPolyPoly.mpImplPolyPolygon;

        rPolyPoly.mpImplPolyPolygon = new ImplPolyPolygon( nPolyCount );

        for ( sal_uInt16 i = 0; i < nPolyCount; i++ )
        {
            pPoly = new Polygon;
            rIStream >> *pPoly;
            rPolyPoly.mpImplPolyPolygon->mpPolyAry[i] = pPoly;
        }
    }
    else
        rPolyPoly = PolyPolygon();

    return rIStream;
}

SvStream& operator<<( SvStream& rOStream, const PolyPolygon& rPolyPoly )
{
    DBG_CHKOBJ( &rPolyPoly, PolyPolygon, NULL );
    DBG_ASSERTWARNING( rOStream.GetVersion(), "PolyPolygon::<< - Solar-Version not set on rOStream" );

    // Write number of polygons
    sal_uInt16 nPolyCount = rPolyPoly.mpImplPolyPolygon->mnCount;
    rOStream << nPolyCount;

    // output polygons
    for ( sal_uInt16 i = 0; i < nPolyCount; i++ )
        rOStream << *(rPolyPoly.mpImplPolyPolygon->mpPolyAry[i]);

    return rOStream;
}

void PolyPolygon::Read( SvStream& rIStream )
{
    VersionCompat aCompat( rIStream, STREAM_READ );

    DBG_CHKTHIS( PolyPolygon, NULL );
    DBG_ASSERTWARNING( rIStream.GetVersion(), "PolyPolygon::>> - Solar-Version not set on rIStream" );

    Polygon* pPoly;
    sal_uInt16   nPolyCount;

    // Read number of polygons
    rIStream >> nPolyCount;

    if( nPolyCount )
    {
        if ( mpImplPolyPolygon->mnRefCount > 1 )
            mpImplPolyPolygon->mnRefCount--;
        else
            delete mpImplPolyPolygon;

        mpImplPolyPolygon = new ImplPolyPolygon( nPolyCount );

        for ( sal_uInt16 i = 0; i < nPolyCount; i++ )
        {
            pPoly = new Polygon;
            pPoly->ImplRead( rIStream );
            mpImplPolyPolygon->mpPolyAry[i] = pPoly;
        }
    }
    else
        *this = PolyPolygon();
}

void PolyPolygon::Write( SvStream& rOStream ) const
{
    VersionCompat aCompat( rOStream, STREAM_WRITE, 1 );

    DBG_CHKTHIS( PolyPolygon, NULL );
    DBG_ASSERTWARNING( rOStream.GetVersion(), "PolyPolygon::<< - Solar-Version not set on rOStream" );

    // Write number of polygons
    sal_uInt16 nPolyCount = mpImplPolyPolygon->mnCount;
    rOStream << nPolyCount;

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
        Polygon* pCandidate = mpImplPolyPolygon->mpPolyAry[a];
        aRetval.append(pCandidate->getB2DPolygon());
    }

    return aRetval;
}

// constructor to convert from basegfx::B2DPolyPolygon
PolyPolygon::PolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon)
{
    DBG_CTOR( PolyPolygon, NULL );
    const sal_uInt16 nCount(sal_uInt16(rPolyPolygon.count()));
    DBG_ASSERT(sal_uInt32(nCount) == rPolyPolygon.count(),
        "PolyPolygon::PolyPolygon: Too many sub-polygons in given basegfx::B2DPolyPolygon (!)");

    if ( nCount )
    {
        mpImplPolyPolygon = new ImplPolyPolygon( nCount );

        for(sal_uInt16 a(0); a < nCount; a++)
        {
            basegfx::B2DPolygon aCandidate(rPolyPolygon.getB2DPolygon(sal_uInt32(a)));
            mpImplPolyPolygon->mpPolyAry[a] = new Polygon( aCandidate );
        }
    }
    else
    {
        mpImplPolyPolygon = new ImplPolyPolygon( 16, 16 );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
