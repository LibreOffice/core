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

#include <rtl/ustring.hxx>
#include <tools/toolsdllapi.h>
#include <tools/gen.hxx>
#include <tools/degree.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <o3tl/cow_wrapper.hxx>

#include <vector>

#define POLY_APPEND             (0xFFFF)
#define POLYPOLY_APPEND         (0xFFFF)

enum class PolyOptimizeFlags {
    NONE      = 0x0000,
    CLOSE     = 0x0001,
    NO_SAME   = 0x0002,
    EDGES     = 0x0004,
};
namespace o3tl
{
    template<> struct typed_flags<PolyOptimizeFlags> : is_typed_flags<PolyOptimizeFlags, 0x0007> {};
}

enum class PolyStyle
{
    Arc = 1,
    Pie = 2,
    Chord = 3
};

enum class PolyFlags : sal_uInt8
{
    Normal,   // start-/endpoint of a curve or a line
    Smooth,   // smooth transition between curves
    Control,  // control handles of a Bezier curve
    Symmetric // smooth and symmetrical transition between curves
};

class SvStream;
class ImplPolygon;
struct ImplPolyPolygon;

namespace basegfx
{
    class B2DPolygon;
    class B2DPolyPolygon;
}

namespace tools {

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC Polygon
{
public:
    typedef             o3tl::cow_wrapper<ImplPolygon> ImplType;
private:
    ImplType            mpImplPolygon;

public:
    SAL_DLLPRIVATE static void ImplReduceEdges( tools::Polygon& rPoly, const double& rArea, sal_uInt16 nPercent );
    SAL_DLLPRIVATE void ImplRead( SvStream& rIStream );
    SAL_DLLPRIVATE void ImplWrite( SvStream& rOStream ) const;

public:
                        Polygon();
                        explicit Polygon( sal_uInt16 nSize );
                        Polygon( sal_uInt16 nPoints, const Point* pPtAry,
                                 const PolyFlags* pFlagAry = nullptr );
                        explicit Polygon( const tools::Rectangle& rRect );
                        Polygon( const tools::Rectangle& rRect,
                                 sal_uInt32 nHorzRound, sal_uInt32 nVertRound );
                        Polygon( const Point& rCenter,
                                 tools::Long nRadX, tools::Long nRadY );
                        Polygon( const tools::Rectangle& rBound,
                                 const Point& rStart, const Point& rEnd,
                                 PolyStyle ePolyStyle = PolyStyle::Arc,
                                 const bool bClockWiseArcDirection = false);
                        Polygon( const Point& rBezPt1, const Point& rCtrlPt1,
                                 const Point& rBezPt2, const Point& rCtrlPt2,
                                 sal_uInt16 nPoints );

                        Polygon( const tools::Polygon& rPoly );
                        Polygon( tools::Polygon&& rPoly) noexcept;
                        ~Polygon();

    void                SetPoint( const Point& rPt, sal_uInt16 nPos );
    const Point&        GetPoint( sal_uInt16 nPos ) const;

    void                SetFlags( sal_uInt16 nPos, PolyFlags eFlags );
    PolyFlags           GetFlags( sal_uInt16 nPos ) const;
    bool                HasFlags() const;

    SAL_DLLPRIVATE bool IsRect() const;

    void                SetSize( sal_uInt16 nNewSize );
    sal_uInt16          GetSize() const;
    sal_uInt16          size() const { return GetSize(); } //for vector compatibility

    SAL_DLLPRIVATE void Clear();

    tools::Rectangle           GetBoundRect() const;
    bool                Contains( const Point& rPt ) const;
    double              CalcDistance( sal_uInt16 nPt1, sal_uInt16 nPt2 ) const;
    void                Clip( const tools::Rectangle& rRect );
    void                Optimize( PolyOptimizeFlags nOptimizeFlags );

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
    void                AdaptiveSubdivide( tools::Polygon& rResult, const double d = 1.0 ) const;
    static Polygon      SubdivideBezier( const Polygon& rPoly );

    void                Move( tools::Long nHorzMove, tools::Long nVertMove );
    void                Translate( const Point& rTrans );
    void                Scale( double fScaleX, double fScaleY );
    void                Rotate( const Point& rCenter, double fSin, double fCos );
    void                Rotate( const Point& rCenter, Degree10 nAngle10 );

    void                Insert( sal_uInt16 nPos, const Point& rPt );
    void                Insert( sal_uInt16 nPos, const tools::Polygon& rPoly );

    const Point&        operator[]( sal_uInt16 nPos ) const { return GetPoint( nPos ); }
    Point&              operator[]( sal_uInt16 nPos );

    tools::Polygon&     operator=( const tools::Polygon& rPoly );
    tools::Polygon&     operator=( tools::Polygon&& rPoly ) noexcept;
    bool                operator==( const tools::Polygon& rPoly ) const;
    bool                operator!=( const tools::Polygon& rPoly ) const
                            { return !(Polygon::operator==( rPoly )); }
    bool                IsEqual( const tools::Polygon& rPoly ) const;

    // streaming a Polygon does ignore PolyFlags, so use the Write Or Read
    // method to take care of PolyFlags
    TOOLS_DLLPUBLIC friend SvStream&    ReadPolygon( SvStream& rIStream, tools::Polygon& rPoly );
    TOOLS_DLLPUBLIC friend SvStream&    WritePolygon( SvStream& rOStream, const tools::Polygon& rPoly );

    void                Read( SvStream& rIStream );
    void                Write( SvStream& rOStream ) const;

    Point *             GetPointAry();
    const Point*        GetConstPointAry() const;
    const PolyFlags*    GetConstFlagAry() const;

    // convert to ::basegfx::B2DPolygon and return
    ::basegfx::B2DPolygon getB2DPolygon() const;

    // constructor to convert from ::basegfx::B2DPolygon
    // #i76339# made explicit
    explicit Polygon(const ::basegfx::B2DPolygon& rPolygon);
};


class SAL_WARN_UNUSED TOOLS_DLLPUBLIC PolyPolygon
{
private:
    o3tl::cow_wrapper<ImplPolyPolygon>  mpImplPolyPolygon;

    enum class PolyClipOp {
        INTERSECT,
        UNION
    };
    TOOLS_DLLPRIVATE void  ImplDoOperation( const tools::PolyPolygon& rPolyPoly, tools::PolyPolygon& rResult, PolyClipOp nOperation ) const;

public:
                        explicit PolyPolygon( sal_uInt16 nInitSize = 16 );
                        explicit PolyPolygon( const tools::Polygon& rPoly );
                        explicit PolyPolygon( const tools::Rectangle& );
                        PolyPolygon( const tools::PolyPolygon& rPolyPoly );
                        PolyPolygon( tools::PolyPolygon&& rPolyPoly ) noexcept;
                        ~PolyPolygon();

    void                Insert( const tools::Polygon& rPoly, sal_uInt16 nPos = POLYPOLY_APPEND );
    void                Remove( sal_uInt16 nPos );
    void                Replace( const Polygon& rPoly, sal_uInt16 nPos );
    const tools::Polygon& GetObject( sal_uInt16 nPos ) const;

    bool                IsRect() const;

    void                Clear();

    sal_uInt16          Count() const;
    tools::Rectangle           GetBoundRect() const;
    void                Clip( const tools::Rectangle& rRect );
    void                Optimize( PolyOptimizeFlags nOptimizeFlags );

    /** Adaptive subdivision of polygons with curves

        This method adaptively subdivides bezier arcs within the
        polygon to straight line segments and returns the resulting
        polygon.

        @param rResult
        The resulting subdivided polygon

        If the polygon is in device coordinates, then the difference between the subdivided and
        the original polygon is guaranteed to be smaller than one
        pixel.
     */
    void                AdaptiveSubdivide( tools::PolyPolygon& rResult ) const;
    static tools::PolyPolygon  SubdivideBezier( const tools::PolyPolygon& rPolyPoly );

    void                GetIntersection( const tools::PolyPolygon& rPolyPoly, tools::PolyPolygon& rResult ) const;
    void                GetUnion( const tools::PolyPolygon& rPolyPoly, tools::PolyPolygon& rResult ) const;

    void                Move( tools::Long nHorzMove, tools::Long nVertMove );
    void                Translate( const Point& rTrans );
    void                Scale( double fScaleX, double fScaleY );
    void                Rotate( const Point& rCenter, double fSin, double fCos );
    void                Rotate( const Point& rCenter, Degree10 nAngle10 );

    const tools::Polygon& operator[]( sal_uInt16 nPos ) const { return GetObject( nPos ); }
    tools::Polygon& operator[]( sal_uInt16 nPos );

    tools::PolyPolygon& operator=( const tools::PolyPolygon& rPolyPoly );
    tools::PolyPolygon& operator=( tools::PolyPolygon&& rPolyPoly ) noexcept;
    bool                operator==( const tools::PolyPolygon& rPolyPoly ) const;
    bool                operator!=( const tools::PolyPolygon& rPolyPoly ) const
                            { return !(PolyPolygon::operator==( rPolyPoly )); }

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

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const Polygon& poly )
{
    stream << "<" << poly.GetSize() << ":";
    for (sal_uInt16 i = 0; i < poly.GetSize(); i++)
    {
        if (i > 0)
            stream << "--";
        stream << poly.GetPoint(i);

        OUString aFlag;
        if (poly.GetFlags(i) == PolyFlags::Normal)
            aFlag = "Normal";
        else if (poly.GetFlags(i) == PolyFlags::Smooth)
            aFlag = "Smooth";
        else if (poly.GetFlags(i) == PolyFlags::Control)
            aFlag = "Control";
        else if (poly.GetFlags(i) == PolyFlags::Symmetric)
            aFlag = "Symmetric";

        stream << ";f=" << aFlag;
    }
    stream << ">";
    return stream;
}

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const PolyPolygon& poly )
{
    stream << "[" << poly.Count() << ":";
    for (sal_uInt16 i = 0; i < poly.Count(); i++)
    {
        if (i > 0)
            stream << ",";
        stream << poly.GetObject(i);
    }
    stream << "]";
    return stream;
}

} /* namespace tools */

typedef std::vector< tools::PolyPolygon > PolyPolyVector;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
