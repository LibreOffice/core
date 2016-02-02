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

#ifndef INCLUDED_BASEGFX_SOURCE_WORKBENCH_BEZIERCLIP_HXX
#define INCLUDED_BASEGFX_SOURCE_WORKBENCH_BEZIERCLIP_HXX

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

/* little approximate comparisons */
template <typename NumType> bool tolZero( NumType n ) { return fabs(n) < DBL_EPSILON; }
template <typename NumType> bool tolEqual( NumType n1, NumType n2 ) { return tolZero(n1-n2); }
template <typename NumType> bool tolLessEqual( NumType n1, NumType n2 ) { return tolEqual(n1,n2) || n1<n2; }
template <typename NumType> bool tolGreaterEqual( NumType n1, NumType n2 ) { return tolEqual(n1,n2) || n1>n2; }

#endif // INCLUDED_BASEGFX_SOURCE_WORKBENCH_BEZIERCLIP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
