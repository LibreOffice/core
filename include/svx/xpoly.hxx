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
#include <tools/degree.hxx>

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

class SVXCORE_DLLPUBLIC XPolygon final
{
    o3tl::cow_wrapper< ImpXPolygon > pImpXPolygon;

    // auxiliary functions for Bezier conversion
    SAL_DLLPRIVATE void    SubdivideBezier(sal_uInt16 nPos, bool bCalcFirst, double fT);
    SAL_DLLPRIVATE void    GenBezArc(const Point& rCenter, tools::Long nRx, tools::Long nRy,
                      tools::Long nXHdl, tools::Long nYHdl, Degree100 nStart, Degree100 nEnd,
                      sal_uInt16 nQuad, sal_uInt16 nFirst);
    SAL_DLLPRIVATE static bool CheckAngles(Degree100& nStart, Degree100 nEnd, Degree100& nA1, Degree100& nA2);

public:
    SAL_DLLPRIVATE XPolygon( sal_uInt16 nSize=16 );
    SAL_DLLPRIVATE XPolygon( const XPolygon& );
    SAL_DLLPRIVATE XPolygon( XPolygon&& );
    XPolygon( const tools::Polygon& rPoly );
    XPolygon( const tools::Rectangle& rRect, tools::Long nRx = 0, tools::Long nRy = 0 );
    XPolygon( const Point& rCenter, tools::Long nRx, tools::Long nRy,
              Degree100 nStartAngle = 0_deg100, Degree100 nEndAngle = 36000_deg100,
              bool bClose = true );

    ~XPolygon();

    sal_uInt16      GetSize() const;

    SAL_DLLPRIVATE void        SetPointCount( sal_uInt16 nPoints );
    SAL_DLLPRIVATE sal_uInt16      GetPointCount() const;

    void        Insert( sal_uInt16 nPos, const Point& rPt, PolyFlags eFlags );
    SAL_DLLPRIVATE void        Insert( sal_uInt16 nPos, const XPolygon& rXPoly );
    SAL_DLLPRIVATE void        Remove( sal_uInt16 nPos, sal_uInt16 nCount );
    SAL_DLLPRIVATE void        Move( tools::Long nHorzMove, tools::Long nVertMove );
    SAL_DLLPRIVATE tools::Rectangle   GetBoundRect() const;

    const Point&    operator[]( sal_uInt16 nPos ) const;
          Point&    operator[]( sal_uInt16 nPos );
    SAL_DLLPRIVATE XPolygon&       operator=( const XPolygon& );
    SAL_DLLPRIVATE XPolygon&       operator=( XPolygon&& );
    SAL_DLLPRIVATE bool            operator==( const XPolygon& rXPoly ) const;

    SAL_DLLPRIVATE PolyFlags  GetFlags( sal_uInt16 nPos ) const;
    SAL_DLLPRIVATE void        SetFlags( sal_uInt16 nPos, PolyFlags eFlags );
    SAL_DLLPRIVATE bool        IsControl(sal_uInt16 nPos) const;
    SAL_DLLPRIVATE bool        IsSmooth(sal_uInt16 nPos) const;

    // distance between two points
    SAL_DLLPRIVATE double  CalcDistance(sal_uInt16 nP1, sal_uInt16 nP2);

    // Bezier conversion
    SAL_DLLPRIVATE void CalcSmoothJoin(sal_uInt16 nCenter, sal_uInt16 nDrag, sal_uInt16 nPnt);
    SAL_DLLPRIVATE void CalcTangent(sal_uInt16 nCenter, sal_uInt16 nPrev, sal_uInt16 nNext);
    SAL_DLLPRIVATE void PointsToBezier(sal_uInt16 nFirst);

    // transformations
    void Scale(double fSx, double fSy);
    SAL_DLLPRIVATE void Distort(const tools::Rectangle& rRefRect, const XPolygon& rDistortedRect);

    // #116512# convert to basegfx::B2DPolygon and return
    basegfx::B2DPolygon getB2DPolygon() const;

    // #116512# constructor to convert from basegfx::B2DPolygon
    // #i76339# made explicit
    SAL_DLLPRIVATE explicit XPolygon(const basegfx::B2DPolygon& rPolygon);
};

// Class XPolyPolygon; like PolyPolygon, composed of XPolygons instead of Polygons

class ImpXPolyPolygon;

class XPolyPolygon final
{
    o3tl::cow_wrapper< ImpXPolyPolygon > pImpXPolyPolygon;

public:
                    XPolyPolygon();
                    XPolyPolygon( const XPolyPolygon& );
                    XPolyPolygon( XPolyPolygon&& );

                    ~XPolyPolygon();

    void            Insert( XPolygon&& rXPoly );
    void            Insert( const XPolyPolygon& rXPoly );
    void            Remove( sal_uInt16 nPos );
    const XPolygon& GetObject( sal_uInt16 nPos ) const;

    void            Clear();
    sal_uInt16          Count() const;

    tools::Rectangle       GetBoundRect() const;

    const XPolygon& operator[]( sal_uInt16 nPos ) const
                        { return GetObject( nPos ); }
    XPolygon&       operator[]( sal_uInt16 nPos );

    XPolyPolygon&   operator=( const XPolyPolygon& );
    XPolyPolygon&   operator=( XPolyPolygon&& );

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
