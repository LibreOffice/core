/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

    inline sal_Bool             GetFirstX( long& rX );
    inline sal_Bool             GetNextX( long& rX );

    sal_Bool                    GetFirstSegment( PolyScanSegment& rSegment );
    sal_Bool                    GetNextSegment( PolyScanSegment& rSegment );
};

// ------------------------------------------------------------------------

inline sal_Bool PolyScanline::GetFirstX( long& rX )
{
    mpAct = mpFirst;
    return( mpAct ? ( rX = mpAct->mnX, mpAct = mpAct->mpNext, sal_True ) : sal_False );
}

// ------------------------------------------------------------------------

inline sal_Bool PolyScanline::GetNextX( long& rX )
{
    return( mpAct ? ( rX = mpAct->mnX, mpAct = mpAct->mpNext, sal_True ) : sal_False );
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

    sal_uLong           Count() const { return Height(); }
    PolyScanline*   operator[]( sal_uLong nPos ) const;
};

#endif // _SV_POLYSCAN_HXX
