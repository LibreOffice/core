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

#include <algorithm>
#include <vector>

#include "bezierclip.hxx"

/* Implements the theta function from Sedgewick: Algorithms in XXX, chapter 24 */
template <class PointType> double theta( const PointType& p1, const PointType& p2 )
{
    typename PointType::value_type dx, dy, ax, ay;
    double t;

    dx = p2.x - p1.x; ax = absval( dx );
    dy = p2.y - p1.y; ay = absval( dy );
    t = (ax+ay == 0) ? 0 : (double) dy/(ax+ay);
    if( dx < 0 )
        t = 2-t;
    else if( dy < 0 )
        t = 4+t;

    return t*90.0;
}

/* Model of LessThanComparable for theta sort.
 * Uses the theta function from Sedgewick: Algorithms in XXX, chapter 24
 */
template <class PointType> class ThetaCompare
{
public:
    explicit ThetaCompare( const PointType& rRefPoint ) : maRefPoint( rRefPoint ) {}

    bool operator() ( const PointType& p1, const PointType& p2 )
    {
        // return true, if p1 precedes p2 in the angle relative to maRefPoint
        return theta(maRefPoint, p1) < theta(maRefPoint, p2);
    }

    double operator() ( const PointType& p ) const
    {
        return theta(maRefPoint, p);
    }

private:
    PointType maRefPoint;
};

/* Implementation of the predicate 'counter-clockwise' for three points, from Sedgewick: Algorithms in XXX, chapter 24 */
template <class PointType, class CmpFunctor> typename PointType::value_type ccw( const PointType& p0, const PointType& p1, const PointType& p2, CmpFunctor& thetaCmp )
{
    typename PointType::value_type dx1, dx2, dy1, dy2;
    typename PointType::value_type theta0( thetaCmp(p0) );
    typename PointType::value_type theta1( thetaCmp(p1) );
    typename PointType::value_type theta2( thetaCmp(p2) );

#if 0
    if( theta0 == theta1 ||
        theta0 == theta2 ||
        theta1 == theta2 )
    {
        // cannot reliably compare, as at least two points are
        // theta-equal. See bug description below
        return 0;
    }
#endif

    dx1 = p1.x - p0.x; dy1 = p1.y - p0.y;
    dx2 = p2.x - p0.x; dy2 = p2.y - p0.y;

    if( dx1*dy2 > dy1*dx2 )
        return +1;

    if( dx1*dy2 < dy1*dx2 )
        return -1;

    if( (dx1*dx2 < 0) || (dy1*dy2 < 0) )
        return -1;

    if( (dx1*dx1 + dy1*dy1) < (dx2*dx2 + dy2*dy2) )
        return +1;

    return 0;
}

/*
 Bug
 ===

 Sometimes, the resulting polygon is not the convex hull (see below
 for an edge configuration to reproduce that problem)

 Problem analysis:
 =================

 The root cause of this bug is the fact that the second part of
 the algorithm (the 'wrapping' of the point set) relies on the
 previous theta sorting. Namely, it is required that the
 generated point ordering, when interpreted as a polygon, is not
 self-intersecting. This property, although, cannot be
 guaranteed due to limited floating point accuracy. For example,
 for two points very close together, and at the same time very
 far away from the theta reference point p1, can appear on the
 same theta value (because floating point accuracy is limited),
 although on different rays to p1 when inspected locally.

 Example:

                    /
             P3    /
               |\ /
               | /
               |/ \
             P2    \
                    \
      ...____________\
     P1

 Here, P2 and P3 are theta-equal relative to P1, but the local
 ccw measure always says 'left turn'. Thus, the convex hull is
 wrong at this place.

 Solution:
 =========

 If two points are theta-equal and checked via ccw, ccw must
 also classify them as 'equal'. Thus, the second stage of the
 convex hull algorithm sorts the first one out, effectively
 reducing a cluster of theta-equal points to only one. This
 single point can then be treated correctly.
*/

/* Implementation of Graham's convex hull algorithm, see Sedgewick: Algorithms in XXX, chapter 25 */
Polygon2D convexHull( const Polygon2D& rPoly )
{
    const Polygon2D::size_type N( rPoly.size() );
    Polygon2D result( N + 1 );
    std::copy(rPoly.begin(), rPoly.end(), result.begin()+1 );
    Polygon2D::size_type min, i;

    // determine safe point on hull (smallest y value)
    for( min=1, i=2; i<=N; ++i )
    {
        if( result[i].y < result[min].y )
            min = i;
    }

    // determine safe point on hull (largest x value)
    for( i=1; i<=N; ++i )
    {
        if( result[i].y == result[min].y &&
            result[i].x > result[min].x )
        {
            min = i;
        }
    }

    // TODO: add inner elimination optimization from Sedgewick: Algorithms in XXX, chapter 25
    // TODO: use radix sort instead of std::sort(), calc theta only once and store

    // setup first point and sort
    std::swap( result[1], result[min] );
    ThetaCompare<Point2D> cmpFunc(result[1]);
    std::sort( result.begin()+1, result.end(), cmpFunc );

    // setup sentinel
    result[0] = result[N];

    // generate convex hull
    Polygon2D::size_type M;
    for( M=3, i=4; i<=N; ++i )
    {
        while( ccw(result[M], result[M-1], result[i], cmpFunc) >= 0 )
            --M;

        ++M;
        std::swap( result[M], result[i] );
    }

    // copy range [1,M] to output
    return Polygon2D( result.begin()+1, result.begin()+M+1 );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
