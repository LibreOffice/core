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
#ifndef INCLUDED_TOOLS_POLY_HXX
#define INCLUDED_TOOLS_POLY_HXX

#include <tools/toolsdllapi.h>
#include <tools/gen.hxx>
#include <tools/debug.hxx>

#include <vector>

#define POLY_APPEND             (0xFFFF)
#define POLYPOLY_APPEND         (0xFFFF)

#define POLY_OPTIMIZE_NONE      0x00000000UL
#define POLY_OPTIMIZE_OPEN      0x00000001UL
#define POLY_OPTIMIZE_CLOSE     0x00000002UL
#define POLY_OPTIMIZE_NO_SAME   0x00000004UL
#define POLY_OPTIMIZE_REDUCE    0x00000008UL
#define POLY_OPTIMIZE_EDGES     0x00000010UL

enum PolyStyle
{
    POLY_ARC = 1,
    POLY_PIE = 2,
    POLY_CHORD = 3
};

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

class SAL_WARN_UNUSED PolyOptimizeData
{
private:

    enum DataType   { DATA_NONE = 0, DATA_ABSOLUT = 1, DATA_PERCENT = 2 };
    DataType        eType;
    union           { sal_uIntPtr mnAbsolut; sal_uInt16 mnPercent; };

public:
                    PolyOptimizeData() : eType( DATA_NONE ) {}
                    PolyOptimizeData( sal_uIntPtr nAbsolut ) : eType( DATA_ABSOLUT ), mnAbsolut( nAbsolut ) {}
                    PolyOptimizeData( sal_uInt16 nPercent ) : eType( DATA_PERCENT ), mnPercent( nPercent ) {}

    sal_uIntPtr     GetAbsValue() const { (void) eType; DBG_ASSERT( eType == DATA_ABSOLUT, "Wrong data type" ); return mnAbsolut; }
    sal_uInt16      GetPercentValue() const { (void) eType; DBG_ASSERT( eType == DATA_PERCENT, "Wrong data type" ); return mnPercent; }
};

class SvStream;
class ImplPolygon;
class ImplPolyPolygon;
namespace tools { class PolyPolygon; }

namespace basegfx
{
    class B2DPolygon;
    class B2DPolyPolygon;
}

class TOOLS_DLLPUBLIC SAL_WARN_UNUSED Polygon
{
private:
    ImplPolygon*        mpImplPolygon;

    TOOLS_DLLPRIVATE inline void ImplMakeUnique();

public:
    static void         ImplReduceEdges( Polygon& rPoly, const double& rArea, sal_uInt16 nPercent );
    void                ImplRead( SvStream& rIStream );
    void                ImplWrite( SvStream& rOStream ) const;

public:
                        Polygon();
                        Polygon( sal_uInt16 nSize );
                        Polygon( sal_uInt16 nPoints, const Point* pPtAry,
                                 const sal_uInt8* pFlagAry = NULL );
                        Polygon( const Rectangle& rRect );
                        Polygon( const Rectangle& rRect,
                                 sal_uIntPtr nHorzRound, sal_uIntPtr nVertRound );
                        Polygon( const Point& rCenter,
                                 long nRadX, long nRadY,
                                 sal_uInt16 nPoints = 0 );
                        Polygon( const Rectangle& rBound,
                                 const Point& rStart, const Point& rEnd,
                                 PolyStyle ePolyStyle = POLY_ARC,
                                 bool bWholeCircle = false );
                        Polygon( const Point& rBezPt1, const Point& rCtrlPt1,
                                 const Point& rBezPt2, const Point& rCtrlPt2,
                                 sal_uInt16 nPoints = 0 );

                        Polygon( const Polygon& rPoly );
                        ~Polygon();

    void                SetPoint( const Point& rPt, sal_uInt16 nPos );
    const Point&        GetPoint( sal_uInt16 nPos ) const;

    void                SetFlags( sal_uInt16 nPos, PolyFlags eFlags );
    PolyFlags           GetFlags( sal_uInt16 nPos ) const;
    bool                HasFlags() const;

    bool                IsRect() const;

    void                SetSize( sal_uInt16 nNewSize );
    sal_uInt16          GetSize() const;

    void                Clear();

    Rectangle           GetBoundRect() const;
    double              GetSignedArea() const;
    bool                IsInside( const Point& rPt ) const;
    bool                IsRightOrientated() const;
    double              CalcDistance( sal_uInt16 nPt1, sal_uInt16 nPt2 );
    void                Clip( const Rectangle& rRect, bool bPolygon = true );
    void                Optimize( sal_uIntPtr nOptimizeFlags, const PolyOptimizeData* pData = NULL );

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
    static Polygon      SubdivideBezier( const Polygon& rPoly );

    void                Move( long nHorzMove, long nVertMove );
    void                Translate( const Point& rTrans );
    void                Scale( double fScaleX, double fScaleY );
    void                Rotate( const Point& rCenter, double fSin, double fCos );
    void                Rotate( const Point& rCenter, sal_uInt16 nAngle10 );

    void                Insert( sal_uInt16 nPos, const Point& rPt, PolyFlags eFlags = POLY_NORMAL );
    void                Insert( sal_uInt16 nPos, const Polygon& rPoly );

    const Point&        operator[]( sal_uInt16 nPos ) const { return GetPoint( nPos ); }
    Point&              operator[]( sal_uInt16 nPos );

    Polygon&            operator=( const Polygon& rPoly );
    bool                operator==( const Polygon& rPoly ) const;
    bool                operator!=( const Polygon& rPoly ) const
                            { return !(Polygon::operator==( rPoly )); }
    bool                IsEqual( const Polygon& rPoly ) const;

    // streaming a Polygon does ignore PolyFlags, so use the Write Or Read
    // method to take care of PolyFlags
    TOOLS_DLLPUBLIC friend SvStream&    ReadPolygon( SvStream& rIStream, Polygon& rPoly );
    TOOLS_DLLPUBLIC friend SvStream&    WritePolygon( SvStream& rOStream, const Polygon& rPoly );

    void                Read( SvStream& rIStream );
    void                Write( SvStream& rOStream ) const;

    const Point*        GetConstPointAry() const;
    const sal_uInt8*    GetConstFlagAry() const;

    // convert to ::basegfx::B2DPolygon and return
    ::basegfx::B2DPolygon getB2DPolygon() const;

    // constructor to convert from ::basegfx::B2DPolygon
    // #i76339# made explicit
    explicit Polygon(const ::basegfx::B2DPolygon& rPolygon);
};

namespace tools {

class TOOLS_DLLPUBLIC SAL_WARN_UNUSED PolyPolygon
{
private:
    ImplPolyPolygon*    mpImplPolyPolygon;

    TOOLS_DLLPRIVATE void  ImplDoOperation( const tools::PolyPolygon& rPolyPoly, tools::PolyPolygon& rResult, sal_uIntPtr nOperation ) const;
    TOOLS_DLLPRIVATE void *ImplCreateArtVpath() const;
    TOOLS_DLLPRIVATE void  ImplSetFromArtVpath( void *pVpath );

public:
                        PolyPolygon( sal_uInt16 nInitSize = 16, sal_uInt16 nResize = 16 );
                        PolyPolygon( const Polygon& rPoly );
                        PolyPolygon( const tools::PolyPolygon& rPolyPoly );
                        ~PolyPolygon();

    void                Insert( const Polygon& rPoly, sal_uInt16 nPos = POLYPOLY_APPEND );
    void                Remove( sal_uInt16 nPos );
    void                Replace( const Polygon& rPoly, sal_uInt16 nPos );
    const Polygon&      GetObject( sal_uInt16 nPos ) const;

    bool                IsRect() const;

    void                Clear();

    sal_uInt16          Count() const;
    Rectangle           GetBoundRect() const;
    void                Clip( const Rectangle& rRect );
    void                Optimize( sal_uIntPtr nOptimizeFlags, const PolyOptimizeData* pData = NULL );

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
    void                AdaptiveSubdivide( tools::PolyPolygon& rResult, const double d = 1.0 ) const;
    static tools::PolyPolygon  SubdivideBezier( const tools::PolyPolygon& rPolyPoly );

    void                GetIntersection( const tools::PolyPolygon& rPolyPoly, tools::PolyPolygon& rResult ) const;
    void                GetUnion( const tools::PolyPolygon& rPolyPoly, tools::PolyPolygon& rResult ) const;

    void                Move( long nHorzMove, long nVertMove );
    void                Translate( const Point& rTrans );
    void                Scale( double fScaleX, double fScaleY );
    void                Rotate( const Point& rCenter, double fSin, double fCos );
    void                Rotate( const Point& rCenter, sal_uInt16 nAngle10 );

    const Polygon&      operator[]( sal_uInt16 nPos ) const { return GetObject( nPos ); }
    Polygon&            operator[]( sal_uInt16 nPos );

    tools::PolyPolygon&        operator=( const tools::PolyPolygon& rPolyPoly );
    bool                operator==( const tools::PolyPolygon& rPolyPoly ) const;
    bool                operator!=( const tools::PolyPolygon& rPolyPoly ) const
                            { return !(PolyPolygon::operator==( rPolyPoly )); }

    bool                IsEqual( const tools::PolyPolygon& rPolyPoly ) const;

    TOOLS_DLLPUBLIC friend SvStream&    ReadPolyPolygon( SvStream& rIStream, tools::PolyPolygon& rPolyPoly );
    TOOLS_DLLPUBLIC friend SvStream&    WritePolyPolygon( SvStream& rOStream, const tools::PolyPolygon& rPolyPoly );

    void                Read( SvStream& rIStream );
    void                Write( SvStream& rOStream ) const;

    // convert to ::basegfx::B2DPolyPolygon and return
    ::basegfx::B2DPolyPolygon getB2DPolyPolygon() const;

    // constructor to convert from ::basegfx::B2DPolyPolygon
     // #i76339# made explicit
     explicit PolyPolygon(const ::basegfx::B2DPolyPolygon& rPolyPolygon);
};

} /* namespace tools */

typedef std::vector< tools::PolyPolygon > PolyPolyVector;


template<typename charT, typename traits>
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const tools::PolyPolygon& rPolyPoly)
{
    if (!rPolyPoly.Count())
        stream << "EMPTY";
    for (sal_uInt16 i = 0; i < rPolyPoly.Count(); ++i)
        stream << "[" << i << "] " << rPolyPoly.GetObject(i);
    return stream;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
