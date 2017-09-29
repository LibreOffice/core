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
#ifndef INCLUDED_SVX_XPOLY_HXX
#define INCLUDED_SVX_XPOLY_HXX

#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <svx/svxdllapi.h>
#include <o3tl/cow_wrapper.hxx>
#include <tools/poly.hxx>

class Point;
namespace tools { class Rectangle; }
class SvStream;
namespace tools {
    class Polygon;
    class PolyPolygon;
}
class OutputDevice;

#define XPOLYPOLY_APPEND     0xFFFF
#define XPOLY_APPEND         0xFFFF


// Class XPolygon has a point-array and a flag-array, which contains information about a particular point

class ImpXPolygon;

class SVX_DLLPUBLIC XPolygon
{
protected:
    o3tl::cow_wrapper< ImpXPolygon > pImpXPolygon;

    // auxiliary functions for Bezier conversion
    void    SubdivideBezier(sal_uInt16 nPos, bool bCalcFirst, double fT);
    void    GenBezArc(const Point& rCenter, long nRx, long nRy,
                      long nXHdl, long nYHdl, sal_uInt16 nStart, sal_uInt16 nEnd,
                      sal_uInt16 nQuad, sal_uInt16 nFirst);
    static bool CheckAngles(sal_uInt16& nStart, sal_uInt16 nEnd, sal_uInt16& nA1, sal_uInt16& nA2);

public:
    XPolygon( sal_uInt16 nSize=16 );
    XPolygon( const XPolygon& rXPoly );
    XPolygon( XPolygon&& rXPoly );
    XPolygon( const tools::Polygon& rPoly );
    XPolygon( const tools::Rectangle& rRect, long nRx = 0, long nRy = 0 );
    XPolygon( const Point& rCenter, long nRx, long nRy,
              sal_uInt16 nStartAngle = 0, sal_uInt16 nEndAngle = 3600,
              bool bClose = true );

    ~XPolygon();

    sal_uInt16      GetSize() const;

    void        SetPointCount( sal_uInt16 nPoints );
    sal_uInt16      GetPointCount() const;

    void        Insert( sal_uInt16 nPos, const Point& rPt, PolyFlags eFlags );
    void        Insert( sal_uInt16 nPos, const XPolygon& rXPoly );
    void        Remove( sal_uInt16 nPos, sal_uInt16 nCount );
    void        Move( long nHorzMove, long nVertMove );
    tools::Rectangle   GetBoundRect() const;

    const Point&    operator[]( sal_uInt16 nPos ) const;
          Point&    operator[]( sal_uInt16 nPos );
    XPolygon&       operator=( const XPolygon& rXPoly );
    XPolygon&       operator=( XPolygon&& rXPoly );
    bool            operator==( const XPolygon& rXPoly ) const;

    PolyFlags  GetFlags( sal_uInt16 nPos ) const;
    void        SetFlags( sal_uInt16 nPos, PolyFlags eFlags );
    bool        IsControl(sal_uInt16 nPos) const;
    bool        IsSmooth(sal_uInt16 nPos) const;

    // distance between two points
    double  CalcDistance(sal_uInt16 nP1, sal_uInt16 nP2);

    // Bezier conversion
    void CalcSmoothJoin(sal_uInt16 nCenter, sal_uInt16 nDrag, sal_uInt16 nPnt);
    void CalcTangent(sal_uInt16 nCenter, sal_uInt16 nPrev, sal_uInt16 nNext);
    void PointsToBezier(sal_uInt16 nFirst);

    // transformations
    void Scale(double fSx, double fSy);
    void Distort(const tools::Rectangle& rRefRect, const XPolygon& rDistortedRect);

    // #116512# convert to basegfx::B2DPolygon and return
    basegfx::B2DPolygon getB2DPolygon() const;

    // #116512# constructor to convert from basegfx::B2DPolygon
     // #i76339# made explicit
     explicit XPolygon(const basegfx::B2DPolygon& rPolygon);
};

// Class XPolyPolygon; like PolyPolygon, composed of XPolygons instead of Polygons

class ImpXPolyPolygon;

class SVX_DLLPUBLIC XPolyPolygon
{
protected:
    o3tl::cow_wrapper< ImpXPolyPolygon > pImpXPolyPolygon;

public:
                    XPolyPolygon();
                    XPolyPolygon( const XPolyPolygon& rXPolyPoly );
                    XPolyPolygon( XPolyPolygon&& rXPolyPoly );

                    ~XPolyPolygon();

    void            Insert( const XPolygon& rXPoly );
    void            Insert( const XPolyPolygon& rXPoly );
    void            Remove( sal_uInt16 nPos );
    const XPolygon& GetObject( sal_uInt16 nPos ) const;

    void            Clear();
    sal_uInt16          Count() const;

    tools::Rectangle       GetBoundRect() const;

    const XPolygon& operator[]( sal_uInt16 nPos ) const
                        { return GetObject( nPos ); }
    XPolygon&       operator[]( sal_uInt16 nPos );

    XPolyPolygon&   operator=( const XPolyPolygon& rXPolyPoly );
    XPolyPolygon&   operator=( XPolyPolygon&& rXPolyPoly );

    // transformations
    void Distort(const tools::Rectangle& rRefRect, const XPolygon& rDistortedRect);

    // #116512# convert to basegfx::B2DPolyPolygon and return
    basegfx::B2DPolyPolygon getB2DPolyPolygon() const;

    // #116512# constructor to convert from basegfx::B2DPolyPolygon
     // #i76339# made explicit
     explicit XPolyPolygon(const basegfx::B2DPolyPolygon& rPolyPolygon);
};

#endif // INCLUDED_SVX_XPOLY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
