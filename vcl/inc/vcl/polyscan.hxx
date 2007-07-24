/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: polyscan.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 10:01:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_POLYSCAN_HXX
#define _SV_POLYSCAN_HXX

#ifndef _GEN_HXX
#include <tools/gen.hxx>
#endif

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
