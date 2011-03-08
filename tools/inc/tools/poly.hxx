/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _TL_POLY_HXX
#define _TL_POLY_HXX

#include "tools/toolsdllapi.h"
#include <tools/gen.hxx>
#include <tools/debug.hxx>

#include <vector>

// -----------
// - Defines -
// -----------

#define POLY_APPEND             (0xFFFF)
#define POLYPOLY_APPEND         (0xFFFF)

// ------------------------------------------------------------------------

#define POLY_OPTIMIZE_NONE      0x00000000UL
#define POLY_OPTIMIZE_OPEN      0x00000001UL
#define POLY_OPTIMIZE_CLOSE     0x00000002UL
#define POLY_OPTIMIZE_NO_SAME   0x00000004UL
#define POLY_OPTIMIZE_REDUCE    0x00000008UL
#define POLY_OPTIMIZE_EDGES     0x00000010UL

// -------------
// - PolyStyle -
// -------------

enum PolyStyle
{
    POLY_ARC = 1,
    POLY_PIE = 2,
    POLY_CHORD = 3
};

// -------------
// - PolyFlags -
// -------------

#ifndef ENUM_POLYFLAGS_DECLARED
#define ENUM_POLYFLAGS_DECLARED
enum PolyFlags
{
    POLY_NORMAL,
    POLY_SMOOTH,
    POLY_CONTROL,
    POLY_SYMMTR
};
#endif

// ----------------
// - PolyOptimize -
// ----------------

class PolyOptimizeData
{
private:

    enum DataType   { DATA_NONE = 0, DATA_ABSOLUT = 1, DATA_PERCENT = 2 };
    DataType        eType;
    union           { ULONG mnAbsolut; USHORT mnPercent; };

public:

                    PolyOptimizeData() : eType( DATA_NONE ) {}
                    PolyOptimizeData( ULONG nAbsolut ) : eType( DATA_ABSOLUT ), mnAbsolut( nAbsolut ) {}
                    PolyOptimizeData( USHORT nPercent ) : eType( DATA_PERCENT ), mnPercent( nPercent ) {}

    ULONG           GetAbsValue() const { DBG_ASSERT( eType == DATA_ABSOLUT, "Wrong data type" ); return mnAbsolut; }
    USHORT          GetPercentValue() const { DBG_ASSERT( eType == DATA_PERCENT, "Wrong data type" ); return mnPercent; }
};

// -----------
// - Polygon -
// -----------

class SvStream;
class ImplPolygon;
class ImplPolyPolygon;
class PolyPolygon;

namespace basegfx
{
    class B2DPolygon;
    class B2DPolyPolygon;
} // end of namespace basegfx

class TOOLS_DLLPUBLIC Polygon
{
private:

    ImplPolygon*        mpImplPolygon;

    TOOLS_DLLPRIVATE inline void ImplMakeUnique();

public:

    Point*              ImplGetPointAry();
    BYTE*               ImplGetFlagAry();

    static void         ImplReduceEdges( Polygon& rPoly, const double& rArea, USHORT nPercent );
    void                ImplRead( SvStream& rIStream );
    void                ImplWrite( SvStream& rOStream ) const;

public:
                        Polygon();
                        Polygon( USHORT nSize );
                        Polygon( USHORT nPoints, const Point* pPtAry,
                                 const BYTE* pFlagAry = NULL );
                        Polygon( const Rectangle& rRect );
                        Polygon( const Rectangle& rRect,
                                 ULONG nHorzRound, ULONG nVertRound );
                        Polygon( const Point& rCenter,
                                 long nRadX, long nRadY,
                                 USHORT nPoints = 0 );
                        Polygon( const Rectangle& rBound,
                                 const Point& rStart, const Point& rEnd,
                                 PolyStyle ePolyStyle = POLY_ARC );
                        Polygon( const Point& rBezPt1, const Point& rCtrlPt1,
                                 const Point& rBezPt2, const Point& rCtrlPt2,
                                 USHORT nPoints = 0 );

                        Polygon( const Polygon& rPoly );
                        ~Polygon();

    void                SetPoint( const Point& rPt, USHORT nPos );
    const Point&        GetPoint( USHORT nPos ) const;

    void                SetFlags( USHORT nPos, PolyFlags eFlags );
    PolyFlags           GetFlags( USHORT nPos ) const;
    sal_Bool            HasFlags() const;

    BOOL                IsControl( USHORT nPos ) const;
    BOOL                IsSmooth( USHORT nPos ) const;
    BOOL                IsRect() const;

    void                SetSize( USHORT nNewSize );
    USHORT              GetSize() const;

    void                Clear();

    Rectangle           GetBoundRect() const;
    double              GetArea() const;
    double              GetSignedArea() const;
    BOOL                IsInside( const Point& rPt ) const;
    BOOL                IsRightOrientated() const;
    double              CalcDistance( USHORT nPt1, USHORT nPt2 );
    void                Clip( const Rectangle& rRect, BOOL bPolygon = TRUE );
    void                Optimize( ULONG nOptimizeFlags, const PolyOptimizeData* pData = NULL );

    /** Adaptive subdivision of polygons with curves

        This method adaptively subdivides bezier arcs within the
        polygon to straight line segments and returns the resulting
        polygon.

        @param rResult
        The resulting subdivided polygon

        @param d
        This parameter controls the amount of subdivision. The
        original curve is guaranteed to not differ by more than this
        amount per bezier segment from the subdivided
        lines. Concretely, if the polygon is in device coordinates and
        d equals 1.0, then the difference between the subdivided and
        the original polygon is guaranteed to be smaller than one
        pixel.
     */
    void                AdaptiveSubdivide( Polygon& rResult, const double d = 1.0 ) const;

    void                GetIntersection( const PolyPolygon& rPolyPoly, PolyPolygon& rResult ) const;
    void                GetUnion( const PolyPolygon& rPolyPoly, PolyPolygon& rResult ) const;
    void                GetDifference( const PolyPolygon& rPolyPoly, PolyPolygon& rResult ) const;
    void                GetXOR( const PolyPolygon& rPolyPoly, PolyPolygon& rResult ) const;

    void                Move( long nHorzMove, long nVertMove );
    void                Translate( const Point& rTrans );
    void                Scale( double fScaleX, double fScaleY );
    void                Rotate( const Point& rCenter, double fSin, double fCos );
    void                Rotate( const Point& rCenter, USHORT nAngle10 );
    void                SlantX( long nYRef, double fSin, double fCos );
    void                SlantY( long nXRef, double fSin, double fCos );
    void                Distort( const Rectangle& rRefRect, const Polygon& rDistortedRect );

    void                Insert( USHORT nPos, const Point& rPt, PolyFlags eFlags = POLY_NORMAL );
    void                Insert( USHORT nPos, const Polygon& rPoly );
    void                Remove( USHORT nPos, USHORT nCount );

    const Point&        operator[]( USHORT nPos ) const { return GetPoint( nPos ); }
    Point&              operator[]( USHORT nPos );

    Polygon&            operator=( const Polygon& rPoly );
    BOOL                operator==( const Polygon& rPoly ) const;
    BOOL                operator!=( const Polygon& rPoly ) const
                            { return !(Polygon::operator==( rPoly )); }
    sal_Bool            IsEqual( const Polygon& rPoly ) const;

    // streaming a Polygon does ignore PolyFlags, so use the Write Or Read
    // method to take care of PolyFlags
    TOOLS_DLLPUBLIC friend SvStream&    operator>>( SvStream& rIStream, Polygon& rPoly );
    TOOLS_DLLPUBLIC friend SvStream&    operator<<( SvStream& rOStream, const Polygon& rPoly );

    void                Read( SvStream& rIStream );
    void                Write( SvStream& rOStream ) const;

    const Point*        GetConstPointAry() const;
    const BYTE*         GetConstFlagAry() const;

    // convert to ::basegfx::B2DPolygon and return
    ::basegfx::B2DPolygon getB2DPolygon() const;

    // constructor to convert from ::basegfx::B2DPolygon
    // #i76339# made explicit
    explicit Polygon(const ::basegfx::B2DPolygon& rPolygon);
};

// ---------------
// - PolyPolygon -
// ---------------

class TOOLS_DLLPUBLIC PolyPolygon
{
private:

    ImplPolyPolygon*    mpImplPolyPolygon;

    TOOLS_DLLPRIVATE void  ImplDoOperation( const PolyPolygon& rPolyPoly, PolyPolygon& rResult, ULONG nOperation ) const;
    TOOLS_DLLPRIVATE void *ImplCreateArtVpath() const;
    TOOLS_DLLPRIVATE void  ImplSetFromArtVpath( void *pVpath );

public:

                        PolyPolygon( USHORT nInitSize = 16, USHORT nResize = 16 );
                        PolyPolygon( const Polygon& rPoly );
                        PolyPolygon( USHORT nPoly, const USHORT* pPointCountAry,
                                     const Point* pPtAry );
                        PolyPolygon( const PolyPolygon& rPolyPoly );
                        ~PolyPolygon();

    void                Insert( const Polygon& rPoly, USHORT nPos = POLYPOLY_APPEND );
    void                Remove( USHORT nPos );
    void                Replace( const Polygon& rPoly, USHORT nPos );
    const Polygon&      GetObject( USHORT nPos ) const;

    BOOL                IsRect() const;

    void                Clear();

    USHORT              Count() const;
    Rectangle           GetBoundRect() const;
    void                Clip( const Rectangle& rRect );
    void                Optimize( ULONG nOptimizeFlags, const PolyOptimizeData* pData = NULL );

    /** Adaptive subdivision of polygons with curves

        This method adaptively subdivides bezier arcs within the
        polygon to straight line segments and returns the resulting
        polygon.

        @param rResult
        The resulting subdivided polygon

        @param d
        This parameter controls the amount of subdivision. The
        original curve is guaranteed to not differ by more than this
        amount per bezier segment from the subdivided
        lines. Concretely, if the polygon is in device coordinates and
        d equals 1.0, then the difference between the subdivided and
        the original polygon is guaranteed to be smaller than one
        pixel.
     */
    void                AdaptiveSubdivide( PolyPolygon& rResult, const double d = 1.0 ) const;

    void                GetIntersection( const PolyPolygon& rPolyPoly, PolyPolygon& rResult ) const;
    void                GetUnion( const PolyPolygon& rPolyPoly, PolyPolygon& rResult ) const;
    void                GetDifference( const PolyPolygon& rPolyPoly, PolyPolygon& rResult ) const;
    void                GetXOR( const PolyPolygon& rPolyPoly, PolyPolygon& rResult ) const;

    void                Move( long nHorzMove, long nVertMove );
    void                Translate( const Point& rTrans );
    void                Scale( double fScaleX, double fScaleY );
    void                Rotate( const Point& rCenter, double fSin, double fCos );
    void                Rotate( const Point& rCenter, USHORT nAngle10 );
    void                SlantX( long nYRef, double fSin, double fCos );
    void                SlantY( long nXRef, double fSin, double fCos );
    void                Distort( const Rectangle& rRefRect, const Polygon& rDistortedRect );

    const Polygon&      operator[]( USHORT nPos ) const { return GetObject( nPos ); }
    Polygon&            operator[]( USHORT nPos );

    PolyPolygon&        operator=( const PolyPolygon& rPolyPoly );
    BOOL                operator==( const PolyPolygon& rPolyPoly ) const;
    BOOL                operator!=( const PolyPolygon& rPolyPoly ) const
                            { return !(PolyPolygon::operator==( rPolyPoly )); }

    sal_Bool            IsEqual( const PolyPolygon& rPolyPoly ) const;

    TOOLS_DLLPUBLIC friend SvStream&    operator>>( SvStream& rIStream, PolyPolygon& rPolyPoly );
    TOOLS_DLLPUBLIC friend SvStream&    operator<<( SvStream& rOStream, const PolyPolygon& rPolyPoly );

    void                Read( SvStream& rIStream );
    void                Write( SvStream& rOStream ) const;

    // convert to ::basegfx::B2DPolyPolygon and return
    ::basegfx::B2DPolyPolygon getB2DPolyPolygon() const;

    // constructor to convert from ::basegfx::B2DPolyPolygon
     // #i76339# made explicit
     explicit PolyPolygon(const ::basegfx::B2DPolyPolygon& rPolyPolygon);
};

typedef std::vector< PolyPolygon > PolyPolyVector;

#endif  // _SV_POLY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
