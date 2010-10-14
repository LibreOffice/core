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

#ifndef BASEGFX_BEZIERCLIP_HXX
#define BASEGFX_BEZIERCLIP_HXX

#include <vector>

struct Point2D
{
    typedef double value_type;
    Point2D( double _x, double _y ) : x(_x), y(_y) {}
    Point2D() : x(), y() {}
    double x;
    double y;
};

struct Bezier
{
    Point2D p0;
    Point2D p1;
    Point2D p2;
    Point2D p3;

    Point2D&        operator[]( int i ) { return reinterpret_cast<Point2D*>(this)[i]; }
    const Point2D&  operator[]( int i ) const { return reinterpret_cast<const Point2D*>(this)[i]; }
};

struct FatLine
{
    // line L through p1 and p4 in normalized implicit form
    double a;
    double b;
    double c;

    // the upper and lower distance from this line
    double dMin;
    double dMax;
};

template <typename DataType> DataType calcLineDistance( const DataType& a,
                                                        const DataType& b,
                                                        const DataType& c,
                                                        const DataType& x,
                                                        const DataType& y )
{
    return a*x + b*y + c;
}

typedef ::std::vector< Point2D > Polygon2D;

/* little abs template */
template <typename NumType> NumType absval( NumType x )
{
    return x<0 ? -x : x;
}

Polygon2D convexHull( const Polygon2D& rPoly );

// TODO: find proper epsilon here (try ::std::numeric_limits<NumType>::epsilon()?)!
#define DBL_EPSILON 1.0e-100

/* little approximate comparions */
template <typename NumType> bool tolZero( NumType n ) { return fabs(n) < DBL_EPSILON; }
template <typename NumType> bool tolEqual( NumType n1, NumType n2 ) { return tolZero(n1-n2); }
template <typename NumType> bool tolLessEqual( NumType n1, NumType n2 ) { return tolEqual(n1,n2) || n1<n2; }
template <typename NumType> bool tolGreaterEqual( NumType n1, NumType n2 ) { return tolEqual(n1,n2) || n1>n2; }

#endif /* BASEGFX_BEZIERCLIP_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
