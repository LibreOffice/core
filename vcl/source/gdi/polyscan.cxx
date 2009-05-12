/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: polyscan.cxx,v $
 * $Revision: 1.6 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <string.h>
#include <tools/new.hxx>
#include <vcl/salbtype.hxx>
#include "polyscan.hxx"
#include <tools/poly.hxx>

// ----------------
// - PolyScanline -
// ----------------

PolyScanline::PolyScanline() :
        mpFirst ( NULL ),
        mpLast  ( NULL ),
        mpAct   ( NULL ),
        mnLeft  ( 0L ),
        mnRight ( 0L )
{
}

// ------------------------------------------------------------------------

PolyScanline::~PolyScanline()
{
    ImplDelete();
}

// ------------------------------------------------------------------------

void PolyScanline::ImplDelete()
{
    ScanlinePoint* pAct = mpFirst;

    while( pAct )
    {
        ScanlinePoint* pNext = pAct->mpNext;
        delete pAct;
        pAct = pNext;
    }

    mnLeft = mnRight = 0L;
    mpFirst = mpAct = mpLast = NULL;
}

// ------------------------------------------------------------------------

void PolyScanline::Insert( long nX )
{
    // first point to insert?
    if( !mpFirst )
        mpLast = mpFirst = new ScanlinePoint( mnLeft = mnRight = nX, NULL );
    else
    {
        // insert at the beginning of the scanline
        if( nX <= mpFirst->mnX )
            mpFirst = new ScanlinePoint( mnLeft = nX, mpFirst );
        else if( nX >= mnRight )
            mpLast = mpLast->mpNext = new ScanlinePoint( mnRight = nX, NULL );
        else
        {
            ScanlinePoint* pLast = mpFirst;
            ScanlinePoint* pAct = mpFirst->mpNext;

            while( pAct )
            {
                // insert in the midlle of the scanline?
                if( nX <= pAct->mnX )
                {
                    pLast->mpNext = new ScanlinePoint( nX, pAct );
                    break;
                }

                pLast = pAct;
                pAct = pAct->mpNext;
            }
        }
    }
}

// ------------------------------------------------------------------------

void PolyScanline::Set( long nStart, long nEnd )
{
    if( mpFirst )
        ImplDelete();

    if( nStart <= nEnd )
        mpFirst = new ScanlinePoint( mnLeft = nStart, mpLast = new ScanlinePoint( mnRight = nEnd, NULL ) );
    else
        mpFirst = new ScanlinePoint( mnLeft = nEnd, mpLast = new ScanlinePoint( mnRight = nStart, NULL ) );
}

// ------------------------------------------------------------------------

BOOL PolyScanline::GetFirstSegment( PolyScanSegment& rSegment )
{
    BOOL bRet = GetFirstX( rSegment.mnStart );

    if( bRet && !GetNextX( rSegment.mnEnd ) )
        rSegment.mnEnd = rSegment.mnStart;

    return bRet;
}

// ------------------------------------------------------------------------

BOOL PolyScanline::GetNextSegment( PolyScanSegment& rSegment )
{
    BOOL bRet = GetNextX( rSegment.mnStart );

    if( bRet && !GetNextX( rSegment.mnEnd ) )
        rSegment.mnEnd = rSegment.mnStart;

    return bRet;
}

// ---------------
// - PolyScanner -
// ---------------

PolyScanner::PolyScanner( const Rectangle& rRect )
{
    if( !rRect.IsEmpty() )
    {
        Rectangle   aRect( rRect );
        ULONG       nHeight;

        aRect.Justify();
        mnLeft = aRect.Left();
        mnTop = aRect.Top();
        mnRight = aRect.Right();
        mnBottom = aRect.Bottom();
        mpArray = new PolyScanline[ nHeight = Height() ];

        for( ULONG i = 0UL; i < nHeight; i++ )
            mpArray[ i ].Set( mnLeft, mnRight );
    }
    else
    {
        mnLeft = mnTop = mnRight = mnBottom = 0L;
        mpArray = NULL;
    }
}

// ------------------------------------------------------------------------

PolyScanner::PolyScanner( const Polygon& rPoly )
{
    const long nCount = rPoly.GetSize();

    if( nCount )
    {
        long    nLast = nCount - 1;
        Point   aFirst( rPoly[ 0 ] );
        Point   aLast( rPoly[ (USHORT) nLast ] );

        while( nLast && ( aLast == aFirst ) )
            aLast = rPoly[ (USHORT) --nLast ];

        if( !nLast )
        {
            aLast = rPoly[ 0 ];
            mnLeft = mnRight = aLast.X();
            mnTop = mnBottom = aLast.Y();
            mpArray = new PolyScanline[ 1UL ];
            mpArray[ 0 ].Set( mnLeft, mnRight );
        }
        else
        {
            const Rectangle aRect( rPoly.GetBoundRect() );
            ULONG           nHeight;

            mnLeft = aRect.Left();
            mnTop = aRect.Top();
            mnRight = aRect.Right();
            mnBottom = aRect.Bottom();
            aLast = aFirst;
            mpArray = new PolyScanline[ nHeight = Height() ];

            for( long i = 1L; i <= nLast; i++ )
            {
                const Point& rPt = rPoly[ (USHORT) i ];

                if( rPt != aLast )
                {
                    InsertLine( aLast, rPt );
                    aLast = rPt;
                }
            }

            InsertLine( aLast, aFirst );
        }
    }
    else
        mpArray = NULL;
}

// ------------------------------------------------------------------------

PolyScanner::PolyScanner( const PolyPolygon& rPolyPoly )
{
    mpArray = NULL;
}

// ------------------------------------------------------------------------

PolyScanner::~PolyScanner()
{
    delete[] mpArray;
}

// ------------------------------------------------------------------------

PolyScanline* PolyScanner::operator[]( ULONG nPos ) const
{
    DBG_ASSERT( nPos < Count(), "nPos out of range!" );
    return( mpArray ? ( mpArray + nPos ) : NULL );
}

// ------------------------------------------------------------------------

void PolyScanner::InsertLine( const Point& rStart, const Point& rEnd )
{
    long nX, nY;

    if( rStart.Y() == rEnd.Y() )
        mpArray[ rStart.Y() - mnTop ].Insert( rStart.X() );
    else if( rStart.X() == rEnd.X() )
    {
        // vertical line
        const long nEndY = rEnd.Y();

        nX = rStart.X();
        nY = rStart.Y();

        if( nEndY > nY )
            while( nY < nEndY )
                mpArray[ nY++ - mnTop ].Insert( nX );
        else
            while( nY > nEndY )
                mpArray[ nY-- - mnTop ].Insert( nX );
    }
    else
    {
        const long  nDX = labs( rEnd.X() - rStart.X() );
        const long  nDY = labs( rEnd.Y() - rStart.Y() );
        const long  nStartX = rStart.X();
        const long  nStartY = rStart.Y();
        const long  nEndX = rEnd.X();
        const long  nEndY = rEnd.Y();
        const long  nXInc = ( nStartX < nEndX ) ? 1L : -1L;
        const long  nYInc = ( nStartY < nEndY ) ? 1L : -1L;
        long        nLastX = nStartX;
        long        nLastY = nStartY;
        BOOL        bLast = FALSE;

        mpArray[ nStartY - mnTop ].Insert( nStartX );

        if( nDX >= nDY )
        {
            const long  nDYX = ( nDY - nDX ) << 1;
            const long  nDY2 = nDY << 1;
            long        nD = nDY2 - nDX;

            for( nX = nStartX, nY = nLastY = nStartY; nX != nEndX; )
            {
                if( nY != nLastY )
                {
                    if( bLast )
                        mpArray[ nLastY - mnTop ].Insert( nLastX );

                    mpArray[ nY - mnTop ].Insert( nX );
                    bLast = FALSE;
                }
                else
                    bLast = TRUE;

                nLastX = nX;
                nLastY = nY;

                if( nD < 0L )
                    nD += nDY2;
                else
                {
                    nD += nDYX;
                    nY += nYInc;
                }

                nX += nXInc;
            }
        }
        else
        {
            const long  nDYX = ( nDX - nDY ) << 1;
            const long  nDY2 = nDX << 1;
            long        nD = nDY2 - nDY;

            for( nX = nStartX, nY = nStartY; nY != nEndY; )
            {
                if( nY != nLastY )
                {
                    if( bLast )
                        mpArray[ nLastY - mnTop ].Insert( nLastX );

                    mpArray[ nY - mnTop ].Insert( nX );
                    bLast = FALSE;
                }
                else
                    bLast = TRUE;

                nLastX = nX;
                nLastY = nY;

                if( nD < 0L )
                    nD += nDY2;
                else
                {
                    nD += nDYX;
                    nX += nXInc;
                }

                nY += nYInc;
            }
        }

        if( bLast )
            mpArray[ nLastY - mnTop ].Insert( nLastX );
    }
}
