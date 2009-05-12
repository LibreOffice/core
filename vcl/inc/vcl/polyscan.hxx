/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: polyscan.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_POLYSCAN_HXX
#define _SV_POLYSCAN_HXX

#include <tools/gen.hxx>

// -----------------
// - ScanlinePoint -
// -----------------

struct ScanlinePoint
{
    long            mnX;
    ScanlinePoint*  mpNext;

                    ScanlinePoint() : mnX( 0L ), mpNext( NULL ) {};
                    ScanlinePoint( long nX, ScanlinePoint* pNext ) : mnX( nX ), mpNext( pNext ) {};
                    ~ScanlinePoint() {};

    void            Set( long nX, ScanlinePoint* pNext ) { mnX = nX, mpNext = pNext; }
};

// -------------------
// - PolyScanSegment -
// -------------------

struct PolyScanSegment
{
    long            mnStart;
    long            mnEnd;

                    PolyScanSegment() : mnStart( 0L ), mnEnd( 0L ) {};
                    PolyScanSegment( long nStart, long nEnd ) : mnStart( nStart ), mnEnd( nEnd ) {};
                    ~PolyScanSegment() {};
};

// ----------------
// - PolyScanline -
// ----------------

struct  ScanlinePoint;
class   Polygon;
class   PolyPolygon;

class PolyScanline
{
private:

    ScanlinePoint*          mpFirst;
    ScanlinePoint*          mpLast;
    ScanlinePoint*          mpAct;
    long                    mnLeft;
    long                    mnRight;

    void                    ImplDelete();

public:

                            PolyScanline();
                            ~PolyScanline();

    void                    Insert( long nX );
    void                    Set( long nStart, long nEnd );
    void                    Set( const PolyScanSegment& rSegment ) { Set( rSegment.mnStart, rSegment.mnEnd ); }

    inline BOOL             GetFirstX( long& rX );
    inline BOOL             GetNextX( long& rX );

    BOOL                    GetFirstSegment( PolyScanSegment& rSegment );
    BOOL                    GetNextSegment( PolyScanSegment& rSegment );
};

// ------------------------------------------------------------------------

inline BOOL PolyScanline::GetFirstX( long& rX )
{
    mpAct = mpFirst;
    return( mpAct ? ( rX = mpAct->mnX, mpAct = mpAct->mpNext, TRUE ) : FALSE );
}

// ------------------------------------------------------------------------

inline BOOL PolyScanline::GetNextX( long& rX )
{
    return( mpAct ? ( rX = mpAct->mnX, mpAct = mpAct->mpNext, TRUE ) : FALSE );
}

// ---------------
// - PolyScanner -
// ---------------

class PolyScanner
{
private:

    PolyScanline*   mpArray;
    long            mnLeft;
    long            mnTop;
    long            mnRight;
    long            mnBottom;

                    PolyScanner() {};

protected:

    void            InsertLine( const Point& rStart, const Point& rEnd );

public:

                    PolyScanner( const Rectangle& rRect );
                    PolyScanner( const Polygon& rPoly );
                    PolyScanner( const PolyPolygon& rPolyPoly );
                    ~PolyScanner();

    long            Left() const { return mnLeft; }
    long            Top() const { return mnTop; }
    long            Right() const { return mnRight; }
    long            Bottom() const { return mnBottom; }

    long            Width() const { return( mnRight - mnLeft + 1L ); }
    long            Height() const { return( mnBottom - mnTop + 1L ); }

    Rectangle       GetBoundRect() const { return Rectangle( mnLeft, mnTop, mnRight, mnBottom ); }

    ULONG           Count() const { return Height(); }
    PolyScanline*   operator[]( ULONG nPos ) const;
};

#endif // _SV_POLYSCAN_HXX
