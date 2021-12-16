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

#include <iostream>
#include <cassert>
#include <algorithm>
#include <iterator>
#include <vector>
#include <utility>

#include <math.h>

#include <bezierclip.hxx>
#include <gauss.hxx>

// what to test
#define WITH_ASSERTIONS
//#define WITH_CONVEXHULL_TEST
//#define WITH_MULTISUBDIVIDE_TEST
//#define WITH_FATLINE_TEST
//#define WITH_CALCFOCUS_TEST
//#define WITH_SAFEPARAMBASE_TEST
//#define WITH_SAFEPARAMS_TEST
//#define WITH_SAFEPARAM_DETAILED_TEST
//#define WITH_SAFEFOCUSPARAM_CALCFOCUS
//#define WITH_SAFEFOCUSPARAM_TEST
//#define WITH_SAFEFOCUSPARAM_DETAILED_TEST
#define WITH_BEZIERCLIP_TEST

/* Implementation of the so-called 'Fat-Line Bezier Clipping Algorithm' by Sederberg et al.
 *
 * Actual reference is: T. W. Sederberg and T Nishita: Curve
 * intersection using Bezier clipping. In Computer Aided Design, 22
 * (9), 1990, pp. 538--549
 */

/* Misc helper
 * ===========
 */
int fallFac( int n, int k )
{
#ifdef WITH_ASSERTIONS
    assert(n>=k); // "For factorials, n must be greater or equal k"
    assert(n>=0); // "For factorials, n must be positive"
    assert(k>=0); // "For factorials, k must be positive"
#endif

    int res( 1 );

    while( k-- && n ) res *= n--;

    return res;
}

int fac( int n )
{
    return fallFac(n, n);
}

/* Bezier fat line clipping part
 * =============================
 */

void Impl_calcFatLine( FatLine& line, const Bezier& c )
{
    // Prepare normalized implicit line
    // ================================

    // calculate vector orthogonal to p1-p4:
    line.a = -(c.p0.y - c.p3.y);
    line.b = (c.p0.x - c.p3.x);

    // normalize
    const double len( sqrt( line.a*line.a + line.b*line.b ) );
    if( !tolZero(len) )
    {
        line.a /= len;
        line.b /= len;
    }

    line.c = -(line.a*c.p0.x + line.b*c.p0.y);

    // Determine bounding fat line from it
    // ===================================

    // calc control point distances
    const double dP2( calcLineDistance(line.a, line.b, line.c, c.p1.x, c.p1.y ) );
    const double dP3( calcLineDistance(line.a, line.b, line.c, c.p2.x, c.p2.y ) );

    // calc approximate bounding lines to curve (tight bounds are
    // possible here, but more expensive to calculate and thus not
    // worth the overhead)
    if( dP2 * dP3 > 0.0 )
    {
        line.dMin = 3.0/4.0 * std::min(0.0, std::min(dP2, dP3));
        line.dMax = 3.0/4.0 * std::max(0.0, std::max(dP2, dP3));
    }
    else
    {
        line.dMin = 4.0/9.0 * std::min(0.0, std::min(dP2, dP3));
        line.dMax = 4.0/9.0 * std::max(0.0, std::max(dP2, dP3));
    }
}

void Impl_calcBounds( Point2D&          leftTop,
                      Point2D&          rightBottom,
                      const Bezier&     c1          )
{
    leftTop.x = std::min( c1.p0.x, std::min( c1.p1.x, std::min( c1.p2.x, c1.p3.x ) ) );
    leftTop.y = std::min( c1.p0.y, std::min( c1.p1.y, std::min( c1.p2.y, c1.p3.y ) ) );
    rightBottom.x = std::max( c1.p0.x, std::max( c1.p1.x, std::max( c1.p2.x, c1.p3.x ) ) );
    rightBottom.y = std::max( c1.p0.y, std::max( c1.p1.y, std::max( c1.p2.y, c1.p3.y ) ) );
}

bool Impl_doBBoxIntersect( const Bezier& c1,
                           const Bezier& c2 )
{
    // calc rectangular boxes from c1 and c2
    Point2D lt1;
    Point2D rb1;
    Point2D lt2;
    Point2D rb2;

    Impl_calcBounds( lt1, rb1, c1 );
    Impl_calcBounds( lt2, rb2, c2 );

    if( std::min(rb1.x, rb2.x) < std::max(lt1.x, lt2.x) ||
        std::min(rb1.y, rb2.y) < std::max(lt1.y, lt2.y) )
    {
        return false;
    }
    else
    {
        return true;
    }
}

/* calculates two t's for the given bernstein control polygon: the first is
 * the intersection of the min value line with the convex hull from
 * the left, the second is the intersection of the max value line with
 * the convex hull from the right.
 */
bool Impl_calcSafeParams( double&           t1,
                          double&           t2,
                          const Polygon2D&  rPoly,
                          double            lowerYBound,
                          double            upperYBound )
{
    // need the convex hull of the control polygon, as this is
    // guaranteed to completely bound the curve
    Polygon2D convHull( convexHull(rPoly) );

    // init min and max buffers
    t1 = 0.0 ;
    double currLowerT( 1.0 );

    t2 = 1.0;
    double currHigherT( 0.0 );

    if( convHull.size() <= 1 )
        return false; // only one point? Then we're done with clipping

    /* now, clip against lower and higher bounds */
    Point2D p0;
    Point2D p1;

    bool bIntersection( false );

    for( Polygon2D::size_type i=0; i<convHull.size(); ++i )
    {
        // have to check against convHull.size() segments, as the
        // convex hull is, by definition, closed. Thus, for the
        // last point, we take the first point as partner.
        if( i+1 == convHull.size() )
        {
            // close the polygon
            p0 = convHull[i];
            p1 = convHull[0];
        }
        else
        {
            p0 = convHull[i];
            p1 = convHull[i+1];
        }

        // is the segment in question within or crossing the
        // horizontal band spanned by lowerYBound and upperYBound? If
        // not, we've got no intersection. If yes, we maybe don't have
        // an intersection, but we've got to update the permissible
        // range, nevertheless. This is because inside lying segments
        // leads to full range forbidden.
        if( (tolLessEqual(p0.y, upperYBound) || tolLessEqual(p1.y, upperYBound)) &&
            (tolGreaterEqual(p0.y, lowerYBound) || tolGreaterEqual(p1.y, lowerYBound)) )
        {
            // calc intersection of convex hull segment with
            // one of the horizontal bounds lines
            // to optimize a bit, r_x is calculated only in else case
            const double r_y( p1.y - p0.y );

            if( tolZero(r_y) )
            {
                // r_y is virtually zero, thus we've got a horizontal
                // line. Now check whether we maybe coincide with lower or
                // upper horizontal bound line.
                if( tolEqual(p0.y, lowerYBound) ||
                    tolEqual(p0.y, upperYBound) )
                {
                    // yes, simulate intersection then
                    currLowerT = std::min(currLowerT, std::min(p0.x, p1.x));
                    currHigherT = std::max(currHigherT, std::max(p0.x, p1.x));
                }
            }
            else
            {
                // check against lower and higher bounds
                // =====================================
                const double r_x( p1.x - p0.x );

                // calc intersection with horizontal dMin line
                const double currTLow( (lowerYBound - p0.y) * r_x / r_y + p0.x );

                // calc intersection with horizontal dMax line
                const double currTHigh( (upperYBound - p0.y) * r_x / r_y + p0.x );

                currLowerT = std::min(currLowerT, std::min(currTLow, currTHigh));
                currHigherT = std::max(currHigherT, std::max(currTLow, currTHigh));
            }

            // set flag that at least one segment is contained or
            // intersects given horizontal band.
            bIntersection = true;
        }
    }

#ifndef WITH_SAFEPARAMBASE_TEST
    // limit intersections found to permissible t parameter range
    t1 = std::max(0.0, currLowerT);
    t2 = std::min(1.0, currHigherT);
#endif

    return bIntersection;
}

/* calculates two t's for the given bernstein polynomial: the first is
 * the intersection of the min value line with the convex hull from
 * the left, the second is the intersection of the max value line with
 * the convex hull from the right.
 *
 * The polynomial coefficients c0 to c3 given to this method
 * must correspond to t values of 0, 1/3, 2/3 and 1, respectively.
 */
bool Impl_calcSafeParams_clip( double&          t1,
                               double&          t2,
                               const FatLine&   bounds,
                               double           c0,
                               double           c1,
                               double           c2,
                               double           c3 )
{
    /* first of all, determine convex hull of c0-c3 */
    Polygon2D poly(4);
    poly[0] = Point2D(0,        c0);
    poly[1] = Point2D(1.0/3.0,  c1);
    poly[2] = Point2D(2.0/3.0,  c2);
    poly[3] = Point2D(1,        c3);

#ifndef WITH_SAFEPARAM_DETAILED_TEST

    return Impl_calcSafeParams( t1, t2, poly, bounds.dMin, bounds.dMax );

#else
    bool bRet( Impl_calcSafeParams( t1, t2, poly, bounds.dMin, bounds.dMax ) );

    Polygon2D convHull( convexHull( poly ) );

    std::cout << "# convex hull testing" << std::endl
         << "plot [t=0:1] ";
    std::cout << " bez("
         << poly[0].x << ","
         << poly[1].x << ","
         << poly[2].x << ","
         << poly[3].x << ",t),bez("
         << poly[0].y << ","
         << poly[1].y << ","
         << poly[2].y << ","
         << poly[3].y << ",t), "
         << "t, " << bounds.dMin << ", "
         << "t, " << bounds.dMax << ", "
         << t1 << ", t, "
         << t2 << ", t, "
         << "'-' using ($1):($2) title \"control polygon\" with lp, "
         << "'-' using ($1):($2) title \"convex hull\" with lp" << std::endl;

    unsigned int k;
    for( k=0; k<poly.size(); ++k )
    {
        std::cout << poly[k].x << " " << poly[k].y << std::endl;
    }
    std::cout << poly[0].x << " " << poly[0].y << std::endl;
    std::cout << "e" << std::endl;

    for( k=0; k<convHull.size(); ++k )
    {
        std::cout << convHull[k].x << " " << convHull[k].y << std::endl;
    }
    std::cout << convHull[0].x << " " << convHull[0].y << std::endl;
    std::cout << "e" << std::endl;

    return bRet;
#endif
}

void Impl_deCasteljauAt( Bezier&        part1,
                         Bezier&        part2,
                         const Bezier&  input,
                         double         t        )
{
    // deCasteljau bezier arc, scheme is:

    // First row is    C_0^n,C_1^n,...,C_n^n
    // Second row is         P_1^n,...,P_n^n
    // etc.
    // with P_k^r = (1 - x_s)P_{k-1}^{r-1} + x_s P_k{r-1}

    // this results in:

    // P1  P2  P3  P4
    // L1  P2  P3  R4
    //     L2  H   R3
    //         L3  R2
    //             L4/R1
    if( tolZero(t) )
    {
        // t is zero -> part2 is input curve, part1 is empty (input.p0, that is)
        part1.p0.x = part1.p1.x = part1.p2.x = part1.p3.x = input.p0.x;
        part1.p0.y = part1.p1.y = part1.p2.y = part1.p3.y = input.p0.y;
        part2 = input;
    }
    else if( tolEqual(t, 1.0) )
    {
        // t is one -> part1 is input curve, part2 is empty (input.p3, that is)
        part1 = input;
        part2.p0.x = part2.p1.x = part2.p2.x = part2.p3.x = input.p3.x;
        part2.p0.y = part2.p1.y = part2.p2.y = part2.p3.y = input.p3.y;
    }
    else
    {
        part1.p0.x = input.p0.x;                                    part1.p0.y = input.p0.y;
        part1.p1.x = (1.0 - t)*part1.p0.x + t*input.p1.x;           part1.p1.y = (1.0 - t)*part1.p0.y + t*input.p1.y;
        const double Hx ( (1.0 - t)*input.p1.x + t*input.p2.x ),    Hy ( (1.0 - t)*input.p1.y + t*input.p2.y );
        part1.p2.x = (1.0 - t)*part1.p1.x + t*Hx;                   part1.p2.y = (1.0 - t)*part1.p1.y + t*Hy;
        part2.p3.x = input.p3.x;                                    part2.p3.y = input.p3.y;
        part2.p2.x = (1.0 - t)*input.p2.x + t*input.p3.x;           part2.p2.y = (1.0 - t)*input.p2.y + t*input.p3.y;
        part2.p1.x = (1.0 - t)*Hx + t*part2.p2.x;                   part2.p1.y = (1.0 - t)*Hy + t*part2.p2.y;
        part2.p0.x = (1.0 - t)*part1.p2.x + t*part2.p1.x;           part2.p0.y = (1.0 - t)*part1.p2.y + t*part2.p1.y;
        part1.p3.x = part2.p0.x;                                    part1.p3.y = part2.p0.y;
    }
}

void printCurvesWithSafeRange( const Bezier& c1, const Bezier& c2, double t1_c1, double t2_c1,
                               const Bezier& c2_part, const FatLine& bounds_c2 )
{
    static int offset = 0;

    std::cout << "# safe param range testing" << std::endl
         << "plot [t=0.0:1.0] ";

    // clip safe ranges off c1
    Bezier c1_part1;
    Bezier c1_part2;
    Bezier c1_part3;

    // subdivide at t1_c1
    Impl_deCasteljauAt( c1_part1, c1_part2, c1, t1_c1 );
    // subdivide at t2_c1
    Impl_deCasteljauAt( c1_part1, c1_part3, c1_part2, t2_c1 );

    // output remaining segment (c1_part1)

    std::cout << "bez("
         << c1.p0.x+offset << ","
         << c1.p1.x+offset << ","
         << c1.p2.x+offset << ","
         << c1.p3.x+offset << ",t),bez("
         << c1.p0.y << ","
         << c1.p1.y << ","
         << c1.p2.y << ","
         << c1.p3.y << ",t), bez("
         << c2.p0.x+offset << ","
         << c2.p1.x+offset << ","
         << c2.p2.x+offset << ","
         << c2.p3.x+offset << ",t),bez("
         << c2.p0.y << ","
         << c2.p1.y << ","
         << c2.p2.y << ","
         << c2.p3.y << ",t), "
#if 1
         << "bez("
         << c1_part1.p0.x+offset << ","
         << c1_part1.p1.x+offset << ","
         << c1_part1.p2.x+offset << ","
         << c1_part1.p3.x+offset << ",t),bez("
         << c1_part1.p0.y << ","
         << c1_part1.p1.y << ","
         << c1_part1.p2.y << ","
         << c1_part1.p3.y << ",t), "
#endif
#if 1
         << "bez("
         << c2_part.p0.x+offset << ","
         << c2_part.p1.x+offset << ","
         << c2_part.p2.x+offset << ","
         << c2_part.p3.x+offset << ",t),bez("
         << c2_part.p0.y << ","
         << c2_part.p1.y << ","
         << c2_part.p2.y << ","
         << c2_part.p3.y << ",t), "
#endif
         << "linex("
         << bounds_c2.a << ","
         << bounds_c2.b << ","
         << bounds_c2.c << ",t)+" << offset << ", liney("
         << bounds_c2.a << ","
         << bounds_c2.b << ","
         << bounds_c2.c << ",t) title \"fat line (center)\", linex("
         << bounds_c2.a << ","
         << bounds_c2.b << ","
         << bounds_c2.c-bounds_c2.dMin << ",t)+" << offset << ", liney("
         << bounds_c2.a << ","
         << bounds_c2.b << ","
         << bounds_c2.c-bounds_c2.dMin << ",t) title \"fat line (min) \", linex("
         << bounds_c2.a << ","
         << bounds_c2.b << ","
         << bounds_c2.c-bounds_c2.dMax << ",t)+" << offset << ", liney("
         << bounds_c2.a << ","
         << bounds_c2.b << ","
         << bounds_c2.c-bounds_c2.dMax << ",t) title \"fat line (max) \"" << std::endl;

    offset += 1;
}

void printResultWithFinalCurves( const Bezier& c1, const Bezier& c1_part,
                                 const Bezier& c2, const Bezier& c2_part,
                                 double t1_c1, double t2_c1 )
{
    static int offset = 0;

    std::cout << "# final result" << std::endl
         << "plot [t=0.0:1.0] ";

    std::cout << "bez("
         << c1.p0.x+offset << ","
         << c1.p1.x+offset << ","
         << c1.p2.x+offset << ","
         << c1.p3.x+offset << ",t),bez("
         << c1.p0.y << ","
         << c1.p1.y << ","
         << c1.p2.y << ","
         << c1.p3.y << ",t), bez("
         << c1_part.p0.x+offset << ","
         << c1_part.p1.x+offset << ","
         << c1_part.p2.x+offset << ","
         << c1_part.p3.x+offset << ",t),bez("
         << c1_part.p0.y << ","
         << c1_part.p1.y << ","
         << c1_part.p2.y << ","
         << c1_part.p3.y << ",t), "
         << " pointmarkx(bez("
         << c1.p0.x+offset << ","
         << c1.p1.x+offset << ","
         << c1.p2.x+offset << ","
         << c1.p3.x+offset << ","
         << t1_c1 << "),t), "
         << " pointmarky(bez("
         << c1.p0.y << ","
         << c1.p1.y << ","
         << c1.p2.y << ","
         << c1.p3.y << ","
         << t1_c1 << "),t), "
         << " pointmarkx(bez("
         << c1.p0.x+offset << ","
         << c1.p1.x+offset << ","
         << c1.p2.x+offset << ","
         << c1.p3.x+offset << ","
         << t2_c1 << "),t), "
         << " pointmarky(bez("
         << c1.p0.y << ","
         << c1.p1.y << ","
         << c1.p2.y << ","
         << c1.p3.y << ","
         << t2_c1 << "),t), "

         << "bez("
         << c2.p0.x+offset << ","
         << c2.p1.x+offset << ","
         << c2.p2.x+offset << ","
         << c2.p3.x+offset << ",t),bez("
         << c2.p0.y << ","
         << c2.p1.y << ","
         << c2.p2.y << ","
         << c2.p3.y << ",t), "
         << "bez("
         << c2_part.p0.x+offset << ","
         << c2_part.p1.x+offset << ","
         << c2_part.p2.x+offset << ","
         << c2_part.p3.x+offset << ",t),bez("
         << c2_part.p0.y << ","
         << c2_part.p1.y << ","
         << c2_part.p2.y << ","
         << c2_part.p3.y << ",t)" << std::endl;

    offset += 1;
}

/** determine parameter ranges [0,t1) and (t2,1] on c1, where c1 is guaranteed to lie outside c2.
      Returns false, if the two curves don't even intersect.

    @param t1
    Range [0,t1) on c1 is guaranteed to lie outside c2

    @param t2
    Range (t2,1] on c1 is guaranteed to lie outside c2

    @param c1_orig
    Original curve c1

    @param c1_part
    Subdivided current part of c1

    @param c2_orig
    Original curve c2

    @param c2_part
    Subdivided current part of c2
 */
bool Impl_calcClipRange( double&        t1,
                         double&        t2,
                         const Bezier&  c1_orig,
                         const Bezier&  c1_part,
                         const Bezier&  c2_orig,
                         const Bezier&  c2_part )
{
    // TODO: Maybe also check fat line orthogonal to P0P3, having P0
    //       and P3 as the extremal points

    if( Impl_doBBoxIntersect(c1_part, c2_part) )
    {
        // Calculate fat lines around c1
        FatLine bounds_c2;

        // must use the subdivided version of c2, since the fat line
        // algorithm works implicitly with the convex hull bounding
        // box.
        Impl_calcFatLine(bounds_c2, c2_part);

        // determine clip positions on c2. Can use original c1 (which
        // is necessary anyway, to get the t's on the original curve),
        // since the distance calculations work directly in the
        // Bernstein polynomial parameter domain.
        if( Impl_calcSafeParams_clip( t1, t2, bounds_c2,
                                      calcLineDistance( bounds_c2.a,
                                                        bounds_c2.b,
                                                        bounds_c2.c,
                                                        c1_orig.p0.x,
                                                        c1_orig.p0.y    ),
                                      calcLineDistance( bounds_c2.a,
                                                        bounds_c2.b,
                                                        bounds_c2.c,
                                                        c1_orig.p1.x,
                                                        c1_orig.p1.y    ),
                                      calcLineDistance( bounds_c2.a,
                                                        bounds_c2.b,
                                                        bounds_c2.c,
                                                        c1_orig.p2.x,
                                                        c1_orig.p2.y    ),
                                      calcLineDistance( bounds_c2.a,
                                                        bounds_c2.b,
                                                        bounds_c2.c,
                                                        c1_orig.p3.x,
                                                        c1_orig.p3.y    ) ) )
        {
            //printCurvesWithSafeRange(c1_orig, c2_orig, t1, t2, c2_part, bounds_c2);

            // they do intersect
            return true;
        }
    }

    // they don't intersect: nothing to do
    return false;
}

/* Tangent intersection part
 * =========================
 */

void Impl_calcFocus( Bezier& res, const Bezier& c )
{
    // arbitrary small value, for now
    // TODO: find meaningful value
    const double minPivotValue( 1.0e-20 );

    Point2D::value_type fMatrix[6];
    Point2D::value_type fRes[2];

    // calc new curve from hodograph, c and linear blend

    // Coefficients for derivative of c are (C_i=n(C_{i+1} - C_i)):

    // 3(P1 - P0), 3(P2 - P1), 3(P3 - P2) (bezier curve of degree 2)

    // The hodograph is then (bezier curve of 2nd degree is P0(1-t)^2 + 2P1(1-t)t + P2t^2):

    // 3(P1 - P0)(1-t)^2 + 6(P2 - P1)(1-t)t + 3(P3 - P2)t^2

    // rotate by 90 degrees: x=-y, y=x and you get the normal vector function N(t):

    // x(t) = -(3(P1.y - P0.y)(1-t)^2 + 6(P2.y - P1.y)(1-t)t + 3(P3.y - P2.y)t^2)
    // y(t) =   3(P1.x - P0.x)(1-t)^2 + 6(P2.x - P1.x)(1-t)t + 3(P3.x - P2.x)t^2

    // Now, the focus curve is defined to be F(t)=P(t) + c(t)N(t),
    // where P(t) is the original curve, and c(t)=c0(1-t) + c1 t

    // This results in the following expression for F(t):

    // x(t) =  P0.x (1-t)^3 + 3 P1.x (1-t)^2t + 3 P2.x (1.t)t^2 + P3.x t^3 -
    //          (c0(1-t) + c1 t)(3(P1.y - P0.y)(1-t)^2 + 6(P2.y - P1.y)(1-t)t + 3(P3.y - P2.y)t^2)

    // y(t) =  P0.y (1-t)^3 + 3 P1.y (1-t)^2t + 3 P2.y (1.t)t^2 + P3.y t^3 +
    //          (c0(1-t) + c1 t)(3(P1.x - P0.x)(1-t)^2 + 6(P2.x - P1.x)(1-t)t + 3(P3.x - P2.x)t^2)

    // As a heuristic, we set F(0)=F(1) (thus, the curve is closed and _tends_ to be small):

    // For F(0), the following results:

    // x(0) = P0.x - c0 3(P1.y - P0.y)
    // y(0) = P0.y + c0 3(P1.x - P0.x)

    // For F(1), the following results:

    // x(1) = P3.x - c1 3(P3.y - P2.y)
    // y(1) = P3.y + c1 3(P3.x - P2.x)

    // Reorder, collect and substitute into F(0)=F(1):

    // P0.x - c0 3(P1.y - P0.y) = P3.x - c1 3(P3.y - P2.y)
    // P0.y + c0 3(P1.x - P0.x) = P3.y + c1 3(P3.x - P2.x)

    // which yields

    // (P0.y - P1.y)c0 + (P3.y - P2.y)c1 = (P3.x - P0.x)/3
    // (P1.x - P0.x)c0 + (P2.x - P3.x)c1 = (P3.y - P0.y)/3

    // so, this is what we calculate here (determine c0 and c1):
    fMatrix[0] = c.p1.x - c.p0.x;
    fMatrix[1] = c.p2.x - c.p3.x;
    fMatrix[2] = (c.p3.y - c.p0.y)/3.0;
    fMatrix[3] = c.p0.y - c.p1.y;
    fMatrix[4] = c.p3.y - c.p2.y;
    fMatrix[5] = (c.p3.x - c.p0.x)/3.0;

    // TODO: determine meaningful value for
    if( !solve(fMatrix, 2, 3, fRes, minPivotValue) )
    {
        // TODO: generate meaningful values here
        // singular or nearly singular system -- use arbitrary
        // values for res
        fRes[0] = 0.0;
        fRes[1] = 1.0;

        std::cerr << "Matrix singular!" << std::endl;
    }

    // now, the reordered and per-coefficient collected focus curve is
    // the following third degree bezier curve F(t):

    // x(t) =  P0.x (1-t)^3 + 3 P1.x (1-t)^2t + 3 P2.x (1.t)t^2 + P3.x t^3 -
    //          (c0(1-t) + c1 t)(3(P1.y - P0.y)(1-t)^2 + 6(P2.y - P1.y)(1-t)t + 3(P3.y - P2.y)t^2)
    //      =  P0.x (1-t)^3 + 3 P1.x (1-t)^2t + 3 P2.x (1.t)t^2 + P3.x t^3 -
    //         (3c0P1.y(1-t)^3 - 3c0P0.y(1-t)^3 + 6c0P2.y(1-t)^2t - 6c0P1.y(1-t)^2t +
    //          3c0P3.y(1-t)t^2 - 3c0P2.y(1-t)t^2 +
    //          3c1P1.y(1-t)^2t - 3c1P0.y(1-t)^2t + 6c1P2.y(1-t)t^2 - 6c1P1.y(1-t)t^2 +
    //          3c1P3.yt^3 - 3c1P2.yt^3)
    //      =  (P0.x - 3 c0 P1.y + 3 c0 P0.y)(1-t)^3 +
    //         3(P1.x - c1 P1.y + c1 P0.y - 2 c0 P2.y + 2 c0 P1.y)(1-t)^2t +
    //         3(P2.x - 2 c1 P2.y + 2 c1 P1.y - c0 P3.y + c0 P2.y)(1-t)t^2 +
    //         (P3.x - 3 c1 P3.y + 3 c1 P2.y)t^3
    //      =  (P0.x - 3 c0(P1.y - P0.y))(1-t)^3 +
    //         3(P1.x - c1(P1.y - P0.y) - 2c0(P2.y - P1.y))(1-t)^2t +
    //         3(P2.x - 2 c1(P2.y - P1.y) - c0(P3.y - P2.y))(1-t)t^2 +
    //         (P3.x - 3 c1(P3.y - P2.y))t^3

    // y(t) =  P0.y (1-t)^3 + 3 P1.y (1-t)^2t + 3 P2.y (1-t)t^2 + P3.y t^3 +
    //          (c0(1-t) + c1 t)(3(P1.x - P0.x)(1-t)^2 + 6(P2.x - P1.x)(1-t)t + 3(P3.x - P2.x)t^2)
    //      =  P0.y (1-t)^3 + 3 P1.y (1-t)^2t + 3 P2.y (1-t)t^2 + P3.y t^3 +
    //         3c0(P1.x - P0.x)(1-t)^3 + 6c0(P2.x - P1.x)(1-t)^2t + 3c0(P3.x - P2.x)(1-t)t^2 +
    //         3c1(P1.x - P0.x)(1-t)^2t + 6c1(P2.x - P1.x)(1-t)t^2 + 3c1(P3.x - P2.x)t^3
    //      =  (P0.y + 3 c0 (P1.x - P0.x))(1-t)^3 +
    //         3(P1.y + 2 c0 (P2.x - P1.x) + c1 (P1.x - P0.x))(1-t)^2t +
    //         3(P2.y + c0 (P3.x - P2.x) + 2 c1 (P2.x - P1.x))(1-t)t^2 +
    //         (P3.y + 3 c1 (P3.x - P2.x))t^3

    // Therefore, the coefficients F0 to F3 of the focus curve are:

    // F0.x = (P0.x - 3 c0(P1.y - P0.y))                    F0.y = (P0.y + 3 c0 (P1.x - P0.x))
    // F1.x = (P1.x - c1(P1.y - P0.y) - 2c0(P2.y - P1.y))   F1.y = (P1.y + 2 c0 (P2.x - P1.x) + c1 (P1.x - P0.x))
    // F2.x = (P2.x - 2 c1(P2.y - P1.y) - c0(P3.y - P2.y))  F2.y = (P2.y + c0 (P3.x - P2.x) + 2 c1 (P2.x - P1.x))
    // F3.x = (P3.x - 3 c1(P3.y - P2.y))                    F3.y = (P3.y + 3 c1 (P3.x - P2.x))

    res.p0.x = c.p0.x - 3*fRes[0]*(c.p1.y - c.p0.y);
    res.p1.x = c.p1.x - fRes[1]*(c.p1.y - c.p0.y) - 2*fRes[0]*(c.p2.y - c.p1.y);
    res.p2.x = c.p2.x - 2*fRes[1]*(c.p2.y - c.p1.y) - fRes[0]*(c.p3.y - c.p2.y);
    res.p3.x = c.p3.x - 3*fRes[1]*(c.p3.y - c.p2.y);

    res.p0.y = c.p0.y + 3*fRes[0]*(c.p1.x - c.p0.x);
    res.p1.y = c.p1.y + 2*fRes[0]*(c.p2.x - c.p1.x) + fRes[1]*(c.p1.x - c.p0.x);
    res.p2.y = c.p2.y + fRes[0]*(c.p3.x - c.p2.x) + 2*fRes[1]*(c.p2.x - c.p1.x);
    res.p3.y = c.p3.y + 3*fRes[1]*(c.p3.x - c.p2.x);
}

bool Impl_calcSafeParams_focus( double&         t1,
                                double&         t2,
                                const Bezier&   curve,
                                const Bezier&   focus )
{
    // now, we want to determine which normals of the original curve
    // P(t) intersect with the focus curve F(t). The condition for
    // this statement is P'(t)(P(t) - F) = 0, i.e. hodograph P'(t) and
    // line through P(t) and F are perpendicular.
    // If you expand this equation, you end up with something like

    // (\sum_{i=0}^n (P_i - F)B_i^n(t))^T (\sum_{j=0}^{n-1} n(P_{j+1} - P_j)B_j^{n-1}(t))

    // Multiplying that out (as the scalar product is linear, we can
    // extract some terms) yields:

    // (P_i - F)^T n(P_{j+1} - P_j) B_i^n(t)B_j^{n-1}(t) + ...

    // If we combine the B_i^n(t)B_j^{n-1}(t) product, we arrive at a
    // Bernstein polynomial of degree 2n-1, as

    // \binom{n}{i}(1-t)^{n-i}t^i) \binom{n-1}{j}(1-t)^{n-1-j}t^j) =
    // \binom{n}{i}\binom{n-1}{j}(1-t)^{2n-1-i-j}t^{i+j}

    // Thus, with the defining equation for a 2n-1 degree Bernstein
    // polynomial

    // \sum_{i=0}^{2n-1} d_i B_i^{2n-1}(t)

    // the d_i are calculated as follows:

    // d_i = \sum_{j+k=i, j\in\{0,...,n\}, k\in\{0,...,n-1\}} \frac{\binom{n}{j}\binom{n-1}{k}}{\binom{2n-1}{i}} n (P_{k+1} - P_k)^T(P_j - F)

    // Okay, but F is now not a single point, but itself a curve
    // F(u). Thus, for every value of u, we get a different 2n-1
    // bezier curve from the above equation. Therefore, we have a
    // tensor product bezier patch, with the following defining
    // equation:

    // d(t,u) = \sum_{i=0}^{2n-1} \sum_{j=0}^m B_i^{2n-1}(t) B_j^{m}(u) d_{ij}, where
    // d_{ij} = \sum_{k+l=i, l\in\{0,...,n\}, k\in\{0,...,n-1\}} \frac{\binom{n}{l}\binom{n-1}{k}}{\binom{2n-1}{i}} n (P_{k+1} - P_k)^T(P_l - F_j)

    // as above, only that now F is one of the focus' control points.

    // Note the difference in the binomial coefficients to the
    // reference paper, these formulas most probably contained a typo.

    // To determine, where D(t,u) is _not_ zero (these are the parts
    // of the curve that don't share normals with the focus and can
    // thus be safely clipped away), we project D(u,t) onto the
    // (d(t,u), t) plane, determine the convex hull there and proceed
    // as for the curve intersection part (projection is orthogonal to
    // u axis, thus simply throw away u coordinate).

    // \fallfac are so-called falling factorials (see Concrete
    // Mathematics, p. 47 for a definition).

    // now, for tensor product bezier curves, the convex hull property
    // holds, too. Thus, we simply project the control points (t_{ij},
    // u_{ij}, d_{ij}) onto the (t,d) plane and calculate the
    // intersections of the convex hull with the t axis, as for the
    // bezier clipping case.

    // calc polygon of control points (t_{ij}, d_{ij}):

    const int n( 3 ); // cubic bezier curves, as a matter of fact
    const int i_card( 2*n );
    const int j_card( n + 1 );
    const int k_max( n-1 );
    Polygon2D controlPolygon( i_card*j_card ); // vector of (t_{ij}, d_{ij}) in row-major order

    int i, j, k, l; // variable notation from formulas above and Sederberg article
    Point2D::value_type d;
    for( i=0; i<i_card; ++i )
    {
        for( j=0; j<j_card; ++j )
        {
            // calc single d_{ij} sum:
            for( d=0.0, k=std::max(0,i-n); k<=k_max && k<=i; ++k )
            {
                l = i - k; // invariant: k + l = i
                assert(k>=0 && k<=n-1); // k \in {0,...,n-1}
                assert(l>=0 && l<=n);   // l \in {0,...,n}

                // TODO: find, document and assert proper limits for n and int's max_val.
                // This becomes important should anybody wants to use
                // this code for higher-than-cubic beziers
                d += static_cast<double>(fallFac(n,l)*fallFac(n-1,k)*fac(i)) /
                    static_cast<double>(fac(l)*fac(k) * fallFac(2*n-1,i)) * n *
                    ( (curve[k+1].x - curve[k].x)*(curve[l].x - focus[j].x) +   // dot product here
                      (curve[k+1].y - curve[k].y)*(curve[l].y - focus[j].y) );
            }

            // Note that the t_{ij} values are evenly spaced on the
            // [0,1] interval, thus t_{ij}=i/(2n-1)
            controlPolygon[ i*j_card + j ] = Point2D( i/(2.0*n-1.0), d );
        }
    }

#ifndef WITH_SAFEFOCUSPARAM_DETAILED_TEST

    // calc safe parameter range, to determine [0,t1] and [t2,1] where
    // no zero crossing is guaranteed.
    return Impl_calcSafeParams( t1, t2, controlPolygon, 0.0, 0.0 );

#else
    bool bRet( Impl_calcSafeParams( t1, t2, controlPolygon, 0.0, 0.0 ) );

    Polygon2D convHull( convexHull( controlPolygon ) );

    std::cout << "# convex hull testing (focus)" << std::endl
         << "plot [t=0:1] ";
    std::cout << "'-' using ($1):($2) title \"control polygon\" with lp, "
         << "'-' using ($1):($2) title \"convex hull\" with lp" << std::endl;

    unsigned int count;
    for( count=0; count<controlPolygon.size(); ++count )
    {
        std::cout << controlPolygon[count].x << " " << controlPolygon[count].y << std::endl;
    }
    std::cout << controlPolygon[0].x << " " << controlPolygon[0].y << std::endl;
    std::cout << "e" << std::endl;

    for( count=0; count<convHull.size(); ++count )
    {
        std::cout << convHull[count].x << " " << convHull[count].y << std::endl;
    }
    std::cout << convHull[0].x << " " << convHull[0].y << std::endl;
    std::cout << "e" << std::endl;

    return bRet;
#endif
}

/** Calc all values t_i on c1, for which safeRanges functor does not
    give a safe range on c1 and c2.

    This method is the workhorse of the bezier clipping. Because c1
    and c2 must be alternatingly tested against each other (first
    determine safe parameter interval on c1 with regard to c2, then
    the other way around), we call this method recursively with c1 and
    c2 swapped.

    @param result
    Output iterator where the final t values are added to. If curves
    don't intersect, nothing is added.

    @param delta
    Maximal allowed distance to true critical point (measured in the
    original curve's coordinate system)

    @param safeRangeFunctor
    Functor object, that must provide the following operator():
    bool safeRangeFunctor( double& t1,
                           double& t2,
                           const Bezier& c1_orig,
                           const Bezier& c1_part,
                           const Bezier& c2_orig,
                           const Bezier& c2_part );
    This functor must calculate the safe ranges [0,t1] and [t2,1] on
    c1_orig, where c1_orig is 'safe' from c2_part. If the whole
    c1_orig is safe, false must be returned, true otherwise.
 */
template <class Functor> void Impl_applySafeRanges_rec( std::back_insert_iterator< std::vector< std::pair<double, double> > >&    result,
                                                        double                                                                          delta,
                                                        const Functor&                                                                  safeRangeFunctor,
                                                        int                                                                             recursionLevel,
                                                        const Bezier&                                                                   c1_orig,
                                                        const Bezier&                                                                   c1_part,
                                                        double                                                                          last_t1_c1,
                                                        double                                                                          last_t2_c1,
                                                        const Bezier&                                                                   c2_orig,
                                                        const Bezier&                                                                   c2_part,
                                                        double                                                                          last_t1_c2,
                                                        double                                                                          last_t2_c2  )
{
    // check end condition
    // ===================

    // TODO: tidy up recursion handling. maybe put everything in a
    // struct and swap that here at method entry

    // TODO: Implement limit on recursion depth. Should that limit be
    // reached, chances are that we're on a higher-order tangency. For
    // this case, AW proposed to take the middle of the current
    // interval, and to correct both curve's tangents at that new
    // endpoint to be equal. That virtually generates a first-order
    // tangency, and justifies to return a single intersection
    // point. Otherwise, inside/outside test might fail here.

    for( int i=0; i<recursionLevel; ++i ) std::cerr << " ";
    if( recursionLevel % 2 )
    {
        std::cerr << std::endl << "level: " << recursionLevel
             << " t: "
             << last_t1_c2 + (last_t2_c2 - last_t1_c2)/2.0
             << ", c1: " << last_t1_c2 << " " << last_t2_c2
             << ", c2: " << last_t1_c1 << " " << last_t2_c1
             << std::endl;
    }
    else
    {
        std::cerr << std::endl << "level: " << recursionLevel
             << " t: "
             << last_t1_c1 + (last_t2_c1 - last_t1_c1)/2.0
             << ", c1: " << last_t1_c1 << " " << last_t2_c1
             << ", c2: " << last_t1_c2 << " " << last_t2_c2
             << std::endl;
    }

    // refine solution
    // ===============

    double t1_c1, t2_c1;

    // Note: we first perform the clipping and only test for precision
    // sufficiency afterwards, since we want to exploit the fact that
    // Impl_calcClipRange returns false if the curves don't
    // intersect. We would have to check that separately for the end
    // condition, otherwise.

    // determine safe range on c1_orig
    if( safeRangeFunctor( t1_c1, t2_c1, c1_orig, c1_part, c2_orig, c2_part ) )
    {
        // now, t1 and t2 are calculated on the original curve
        // (but against a fat line calculated from the subdivided
        // c2, namely c2_part). If the [t1,t2] range is outside
        // our current [last_t1,last_t2] range, we're done in this
        // branch - the curves no longer intersect.
        if( tolLessEqual(t1_c1, last_t2_c1) && tolGreaterEqual(t2_c1, last_t1_c1) )
        {
            // As noted above, t1 and t2 are calculated on the
            // original curve, but against a fat line
            // calculated from the subdivided c2, namely
            // c2_part. Our domain to work on is
            // [last_t1,last_t2], on the other hand, so values
            // of [t1,t2] outside that range are irrelevant
            // here. Clip range appropriately.
            t1_c1 = std::max(t1_c1, last_t1_c1);
            t2_c1 = std::min(t2_c1, last_t2_c1);

            // TODO: respect delta
            // for now, end condition is just a fixed threshold on the t's

            // check end condition
            // ===================

#if 1
            if( fabs(last_t2_c1 - last_t1_c1) < 0.0001 &&
                fabs(last_t2_c2 - last_t1_c2) < 0.0001  )
#else
            if( fabs(last_t2_c1 - last_t1_c1) < 0.01 &&
                fabs(last_t2_c2 - last_t1_c2) < 0.01    )
#endif
            {
                // done. Add to result
                if( recursionLevel % 2 )
                {
                    // uneven level: have to swap the t's, since curves are swapped, too
                    *result++ = std::make_pair( last_t1_c2 + (last_t2_c2 - last_t1_c2)/2.0,
                                                  last_t1_c1 + (last_t2_c1 - last_t1_c1)/2.0 );
                }
                else
                {
                    *result++ = std::make_pair( last_t1_c1 + (last_t2_c1 - last_t1_c1)/2.0,
                                                  last_t1_c2 + (last_t2_c2 - last_t1_c2)/2.0 );
                }

#if 0
                //printResultWithFinalCurves( c1_orig, c1_part, c2_orig, c2_part, last_t1_c1, last_t2_c1 );
                printResultWithFinalCurves( c1_orig, c1_part, c2_orig, c2_part, t1_c1, t2_c1 );
#else
                // calc focus curve of c2
                Bezier focus;
                Impl_calcFocus(focus, c2_part); // need to use subdivided c2

                safeRangeFunctor( t1_c1, t2_c1, c1_orig, c1_part, c2_orig, c2_part );

                //printResultWithFinalCurves( c1_orig, c1_part, c2_orig, focus, t1_c1, t2_c1 );
                printResultWithFinalCurves( c1_orig, c1_part, c2_orig, focus, last_t1_c1, last_t2_c1 );
#endif
            }
            else
            {
                // heuristic: if parameter range is not reduced by at least
                // 20%, subdivide longest curve, and clip shortest against
                // both parts of longest
//                if( (last_t2_c1 - last_t1_c1 - t2_c1 + t1_c1) / (last_t2_c1 - last_t1_c1) < 0.2 )
                if( false )
                {
                    // subdivide and descend
                    // =====================

                    Bezier part1;
                    Bezier part2;

                    double intervalMiddle;

                    if( last_t2_c1 - last_t1_c1 > last_t2_c2 - last_t1_c2 )
                    {
                        // subdivide c1
                        // ============

                        intervalMiddle = last_t1_c1 + (last_t2_c1 - last_t1_c1)/2.0;

                        // subdivide at the middle of the interval (as
                        // we're not subdividing on the original
                        // curve, this simply amounts to subdivision
                        // at 0.5)
                        Impl_deCasteljauAt( part1, part2, c1_part, 0.5 );

                        // and descend recursively with swapped curves
                        Impl_applySafeRanges_rec( result, delta, safeRangeFunctor, recursionLevel+1,
                                                  c2_orig, c2_part, last_t1_c2, last_t2_c2,
                                                  c1_orig, part1, last_t1_c1, intervalMiddle );

                        Impl_applySafeRanges_rec( result, delta, safeRangeFunctor, recursionLevel+1,
                                                  c2_orig, c2_part, last_t1_c2, last_t2_c2,
                                                  c1_orig, part2, intervalMiddle, last_t2_c1 );
                    }
                    else
                    {
                        // subdivide c2
                        // ============

                        intervalMiddle = last_t1_c2 + (last_t2_c2 - last_t1_c2)/2.0;

                        // subdivide at the middle of the interval (as
                        // we're not subdividing on the original
                        // curve, this simply amounts to subdivision
                        // at 0.5)
                        Impl_deCasteljauAt( part1, part2, c2_part, 0.5 );

                        // and descend recursively with swapped curves
                        Impl_applySafeRanges_rec( result, delta, safeRangeFunctor, recursionLevel+1,
                                                  c2_orig, part1, last_t1_c2, intervalMiddle,
                                                  c1_orig, c1_part, last_t1_c1, last_t2_c1 );

                        Impl_applySafeRanges_rec( result, delta, safeRangeFunctor, recursionLevel+1,
                                                  c2_orig, part2, intervalMiddle, last_t2_c2,
                                                  c1_orig, c1_part, last_t1_c1, last_t2_c1 );
                    }
                }
                else
                {
                    // apply calculated clip
                    // =====================

                    // clip safe ranges off c1_orig
                    Bezier c1_part1;
                    Bezier c1_part2;
                    Bezier c1_part3;

                    // subdivide at t1_c1
                    Impl_deCasteljauAt( c1_part1, c1_part2, c1_orig, t1_c1 );

                    // subdivide at t2_c1. As we're working on
                    // c1_part2 now, we have to adapt t2_c1 since
                    // we're no longer in the original parameter
                    // interval. This is based on the following
                    // assumption: t2_new = (t2-t1)/(1-t1), which
                    // relates the t2 value into the new parameter
                    // range [0,1] of c1_part2.
                    Impl_deCasteljauAt( c1_part1, c1_part3, c1_part2, (t2_c1-t1_c1)/(1.0-t1_c1) );

                    // descend with swapped curves and c1_part1 as the
                    // remaining (middle) segment
                    Impl_applySafeRanges_rec( result, delta, safeRangeFunctor, recursionLevel+1,
                                              c2_orig, c2_part, last_t1_c2, last_t2_c2,
                                              c1_orig, c1_part1, t1_c1, t2_c1 );
                }
            }
        }
    }
}

struct ClipBezierFunctor
{
    bool operator()( double& t1_c1,
                     double& t2_c1,
                     const Bezier& c1_orig,
                     const Bezier& c1_part,
                     const Bezier& c2_orig,
                     const Bezier& c2_part ) const
    {
        return Impl_calcClipRange( t1_c1, t2_c1, c1_orig, c1_part, c2_orig, c2_part );
    }
};

struct BezierTangencyFunctor
{
    bool operator()( double& t1_c1,
                     double& t2_c1,
                     const Bezier& c1_orig,
                     const Bezier& c1_part,
                     const Bezier& c2_orig,
                     const Bezier& c2_part ) const
    {
        // calc focus curve of c2
        Bezier focus;
        Impl_calcFocus(focus, c2_part); // need to use subdivided c2
                                        // here, as the whole curve is
                                        // used for focus calculation

        // determine safe range on c1_orig
        bool bRet( Impl_calcSafeParams_focus( t1_c1, t2_c1,
                                              c1_orig, // use orig curve here, need t's on original curve
                                              focus ) );

        std::cerr << "range: " << t2_c1 - t1_c1 << ", ret: " << bRet << std::endl;

        return bRet;
    }
};

/** Perform a bezier clip (curve against curve)

    @param result
    Output iterator where the final t values are added to. This
    iterator will remain empty, if there are no intersections.

    @param delta
    Maximal allowed distance to true intersection (measured in the
    original curve's coordinate system)
 */
void clipBezier( std::back_insert_iterator< std::vector< std::pair<double, double> > >&   result,
                 double                                                                         delta,
                 const Bezier&                                                                  c1,
                 const Bezier&                                                                  c2        )
{
#if 0
    // first of all, determine list of collinear normals. Collinear
    // normals typically separate two intersections, thus, subdivide
    // at all collinear normal's t values beforehand. This will cater
    // for tangent intersections, where two or more intersections are
    // infinitesimally close together.

    // TODO: evaluate effects of higher-than-second-order
    // tangencies. Sederberg et al. state that collinear normal
    // algorithm then degrades quickly.

    std::vector< std::pair<double,double> > results;
    std::back_insert_iterator< std::vector< std::pair<double, double> > > ii(results);

    Impl_calcCollinearNormals( ii, delta, 0, c1, c1, 0.0, 1.0, c2, c2, 0.0, 1.0 );

    // As Sederberg's collinear normal theorem is only sufficient, not
    // necessary for two intersections left and right, we've to test
    // all segments generated by the collinear normal algorithm
    // against each other. In other words, if the two curves are both
    // divided in a left and a right part, the collinear normal
    // theorem does _not_ state that the left part of curve 1 does not
    // e.g. intersect with the right part of curve 2.

    // divide c1 and c2 at collinear normal intersection points
    std::vector< Bezier > c1_segments( results.size()+1 );
    std::vector< Bezier > c2_segments( results.size()+1 );
    Bezier c1_remainder( c1 );
    Bezier c2_remainder( c2 );
    unsigned int i;
    for( i=0; i<results.size(); ++i )
    {
        Bezier c1_part2;
        Impl_deCasteljauAt( c1_segments[i], c1_part2, c1_remainder, results[i].first );
        c1_remainder = c1_part2;

        Bezier c2_part2;
        Impl_deCasteljauAt( c2_segments[i], c2_part2, c2_remainder, results[i].second );
        c2_remainder = c2_part2;
    }
    c1_segments[i] = c1_remainder;
    c2_segments[i] = c2_remainder;

    // now, c1/c2_segments contain all segments, then
    // clip every resulting segment against every other
    unsigned int c1_curr, c2_curr;
    for( c1_curr=0; c1_curr<c1_segments.size(); ++c1_curr )
    {
        for( c2_curr=0; c2_curr<c2_segments.size(); ++c2_curr )
        {
            if( c1_curr != c2_curr )
            {
                Impl_clipBezier_rec(result, delta, 0,
                                    c1_segments[c1_curr], c1_segments[c1_curr],
                                    0.0, 1.0,
                                    c2_segments[c2_curr], c2_segments[c2_curr],
                                    0.0, 1.0);
            }
        }
    }
#else
    Impl_applySafeRanges_rec( result, delta, BezierTangencyFunctor(), 0, c1, c1, 0.0, 1.0, c2, c2, 0.0, 1.0 );
    //Impl_applySafeRanges_rec( result, delta, ClipBezierFunctor(), 0, c1, c1, 0.0, 1.0, c2, c2, 0.0, 1.0 );
#endif
    // that's it, boys'n'girls!
}

int main(int argc, const char *argv[])
{
    double curr_Offset( 0 );
    unsigned int i,j,k;

    Bezier someCurves[] =
        {
//            {Point2D(0.0,0.0),Point2D(0.0,1.0),Point2D(1.0,1.0),Point2D(1.0,0.0)},
//            {Point2D(0.0,0.0),Point2D(0.0,1.0),Point2D(1.0,1.0),Point2D(1.0,0.5)},
//            {Point2D(1.0,0.0),Point2D(0.0,0.0),Point2D(0.0,1.0),Point2D(1.0,1.0)}
//            {Point2D(0.25+1,0.5),Point2D(0.25+1,0.708333),Point2D(0.423611+1,0.916667),Point2D(0.770833+1,0.980324)},
//            {Point2D(0.0+1,0.0),Point2D(0.0+1,1.0),Point2D(1.0+1,1.0),Point2D(1.0+1,0.5)}

// tangency1
//            {Point2D(0.627124+1,0.828427),Point2D(0.763048+1,0.828507),Point2D(0.885547+1,0.77312),Point2D(0.950692+1,0.67325)},
//            {Point2D(0.0,1.0),Point2D(0.1,1.0),Point2D(0.4,1.0),Point2D(0.5,1.0)}

//            {Point2D(0.0,0.0),Point2D(0.0,1.0),Point2D(1.0,1.0),Point2D(1.0,0.5)},
//            {Point2D(0.60114,0.933091),Point2D(0.69461,0.969419),Point2D(0.80676,0.992976),Point2D(0.93756,0.998663)}
//            {Point2D(1.0,0.0),Point2D(0.0,0.0),Point2D(0.0,1.0),Point2D(1.0,1.0)},
//            {Point2D(0.62712,0.828427),Point2D(0.76305,0.828507),Point2D(0.88555,0.77312),Point2D(0.95069,0.67325)}

// clipping1
//            {Point2D(0.0,0.0),Point2D(0.0,3.5),Point2D(1.0,-2.5),Point2D(1.0,1.0)},
//            {Point2D(0.0,1.0),Point2D(3.5,1.0),Point2D(-2.5,0.0),Point2D(1.0,0.0)}

// tangency2
//            {Point2D(0.0,1.0),Point2D(3.5,1.0),Point2D(-2.5,0.0),Point2D(1.0,0.0)},
//            {Point2D(15.3621,0.00986464),Point2D(15.3683,0.0109389),Point2D(15.3682,0.0109315),Point2D(15.3621,0.00986464)}

// tangency3
//            {Point2D(1.0,0.0),Point2D(0.0,0.0),Point2D(0.0,1.0),Point2D(1.0,1.0)},
//            {Point2D(-0.5,0.0),Point2D(0.5,0.0),Point2D(0.5,1.0),Point2D(-0.5,1.0)}

// tangency4
//            {Point2D(-0.5,0.0),Point2D(0.5,0.0),Point2D(0.5,1.0),Point2D(-0.5,1.0)},
//            {Point2D(0.26,0.4),Point2D(0.25,0.5),Point2D(0.25,0.5),Point2D(0.26,0.6)}

// tangency5
//            {Point2D(0.0,0.0),Point2D(0.0,3.5),Point2D(1.0,-2.5),Point2D(1.0,1.0)},
//            {Point2D(15.3621,0.00986464),Point2D(15.3683,0.0109389),Point2D(15.3682,0.0109315),Point2D(15.3621,0.00986464)}

// tangency6
//            {Point2D(0.0,0.0),Point2D(0.0,3.5),Point2D(1.0,-2.5),Point2D(1.0,1.0)},
//            {Point2D(15.3621,10.00986464),Point2D(15.3683,10.0109389),Point2D(15.3682,10.0109315),Point2D(15.3621,10.00986464)}

// tangency7
//            {Point2D(2.505,0.0),Point2D(2.505+4.915,4.300),Point2D(2.505+3.213,10.019),Point2D(2.505-2.505,10.255)},
//            {Point2D(15.3621,10.00986464),Point2D(15.3683,10.0109389),Point2D(15.3682,10.0109315),Point2D(15.3621,10.00986464)}

// tangency Sederberg example
            {Point2D(2.505,0.0),Point2D(2.505+4.915,4.300),Point2D(2.505+3.213,10.019),Point2D(2.505-2.505,10.255)},
            {Point2D(5.33+9.311,0.0),Point2D(5.33+9.311-13.279,4.205),Point2D(5.33+9.311-10.681,9.119),Point2D(5.33+9.311-2.603,10.254)}

// clipping2
//            {Point2D(-0.5,0.0),Point2D(0.5,0.0),Point2D(0.5,1.0),Point2D(-0.5,1.0)},
//            {Point2D(0.2575,0.4),Point2D(0.2475,0.5),Point2D(0.2475,0.5),Point2D(0.2575,0.6)}

//            {Point2D(0.0,0.1),Point2D(0.2,3.5),Point2D(1.0,-2.5),Point2D(1.1,1.2)},
//            {Point2D(0.0,1.0),Point2D(3.5,0.9),Point2D(-2.5,0.1),Point2D(1.1,0.2)}
//            {Point2D(0.0,0.1),Point2D(0.2,3.0),Point2D(1.0,-2.0),Point2D(1.1,1.2)},
//            {Point2D(0.627124+1,0.828427),Point2D(0.763048+1,0.828507),Point2D(0.885547+1,0.77312),Point2D(0.950692+1,0.67325)}
//            {Point2D(0.0,1.0),Point2D(3.0,0.9),Point2D(-2.0,0.1),Point2D(1.1,0.2)}
//            {Point2D(0.0,4.0),Point2D(0.1,5.0),Point2D(0.9,5.0),Point2D(1.0,4.0)},
//            {Point2D(0.0,0.0),Point2D(0.1,0.5),Point2D(0.9,0.5),Point2D(1.0,0.0)},
//            {Point2D(0.0,0.1),Point2D(0.1,1.5),Point2D(0.9,1.5),Point2D(1.0,0.1)},
//            {Point2D(0.0,-4.0),Point2D(0.1,-5.0),Point2D(0.9,-5.0),Point2D(1.0,-4.0)}
        };

    // output gnuplot setup
    std::cout << "#!/usr/bin/gnuplot -persist" << std::endl
         << "#" << std::endl
         << "# automatically generated by bezierclip, don't change!" << std::endl
         << "#" << std::endl
         << "set parametric" << std::endl
         << "bez(p,q,r,s,t) = p*(1-t)**3+q*3*(1-t)**2*t+r*3*(1-t)*t**2+s*t**3" << std::endl
         << "bezd(p,q,r,s,t) = 3*(q-p)*(1-t)**2+6*(r-q)*(1-t)*t+3*(s-r)*t**2" << std::endl
         << "pointmarkx(c,t) = c-0.03*t" << std::endl
         << "pointmarky(c,t) = c+0.03*t" << std::endl
         << "linex(a,b,c,t) = a*-c + t*-b" << std::endl
         << "liney(a,b,c,t) = b*-c + t*a" << std::endl << std::endl
         << "# end of setup" << std::endl << std::endl;

#ifdef WITH_CONVEXHULL_TEST
    // test convex hull algorithm
    const double convHull_xOffset( curr_Offset );
    curr_Offset += 20;
    std::cout << "# convex hull testing" << std::endl
         << "plot [t=0:1] ";
    for( i=0; i<sizeof(someCurves)/sizeof(Bezier); ++i )
    {
        Polygon2D aTestPoly(4);
        aTestPoly[0] = someCurves[i].p0;
        aTestPoly[1] = someCurves[i].p1;
        aTestPoly[2] = someCurves[i].p2;
        aTestPoly[3] = someCurves[i].p3;

        aTestPoly[0].x += convHull_xOffset;
        aTestPoly[1].x += convHull_xOffset;
        aTestPoly[2].x += convHull_xOffset;
        aTestPoly[3].x += convHull_xOffset;

        std::cout << " bez("
             << aTestPoly[0].x << ","
             << aTestPoly[1].x << ","
             << aTestPoly[2].x << ","
             << aTestPoly[3].x << ",t),bez("
             << aTestPoly[0].y << ","
             << aTestPoly[1].y << ","
             << aTestPoly[2].y << ","
             << aTestPoly[3].y << ",t), '-' using ($1):($2) title \"convex hull " << i << "\" with lp";

        if( i+1<sizeof(someCurves)/sizeof(Bezier) )
            std::cout << ",\\" << std::endl;
        else
            std::cout << std::endl;
    }
    for( i=0; i<sizeof(someCurves)/sizeof(Bezier); ++i )
    {
        Polygon2D aTestPoly(4);
        aTestPoly[0] = someCurves[i].p0;
        aTestPoly[1] = someCurves[i].p1;
        aTestPoly[2] = someCurves[i].p2;
        aTestPoly[3] = someCurves[i].p3;

        aTestPoly[0].x += convHull_xOffset;
        aTestPoly[1].x += convHull_xOffset;
        aTestPoly[2].x += convHull_xOffset;
        aTestPoly[3].x += convHull_xOffset;

        Polygon2D convHull( convexHull(aTestPoly) );

        for( k=0; k<convHull.size(); ++k )
        {
            std::cout << convHull[k].x << " " << convHull[k].y << std::endl;
        }
        std::cout << convHull[0].x << " " << convHull[0].y << std::endl;
        std::cout << "e" << std::endl;
    }
#endif

#ifdef WITH_MULTISUBDIVIDE_TEST
    // test convex hull algorithm
    const double multiSubdivide_xOffset( curr_Offset );
    curr_Offset += 20;
    std::cout << "# multi subdivide testing" << std::endl
         << "plot [t=0:1] ";
    for( i=0; i<sizeof(someCurves)/sizeof(Bezier); ++i )
    {
        Bezier c( someCurves[i] );
        Bezier c1_part1;
        Bezier c1_part2;
        Bezier c1_part3;

        c.p0.x += multiSubdivide_xOffset;
        c.p1.x += multiSubdivide_xOffset;
        c.p2.x += multiSubdivide_xOffset;
        c.p3.x += multiSubdivide_xOffset;

        const double t1( 0.1+i/(3.0*sizeof(someCurves)/sizeof(Bezier)) );
        const double t2( 0.9-i/(3.0*sizeof(someCurves)/sizeof(Bezier)) );

        // subdivide at t1
        Impl_deCasteljauAt( c1_part1, c1_part2, c, t1 );

        // subdivide at t2_c1. As we're working on
        // c1_part2 now, we have to adapt t2_c1 since
        // we're no longer in the original parameter
        // interval. This is based on the following
        // assumption: t2_new = (t2-t1)/(1-t1), which
        // relates the t2 value into the new parameter
        // range [0,1] of c1_part2.
        Impl_deCasteljauAt( c1_part1, c1_part3, c1_part2, (t2-t1)/(1.0-t1) );

        // subdivide at t2
        Impl_deCasteljauAt( c1_part3, c1_part2, c, t2 );

        std::cout << " bez("
             << c1_part1.p0.x << ","
             << c1_part1.p1.x << ","
             << c1_part1.p2.x << ","
             << c1_part1.p3.x << ",t), bez("
             << c1_part1.p0.y+0.01 << ","
             << c1_part1.p1.y+0.01 << ","
             << c1_part1.p2.y+0.01 << ","
             << c1_part1.p3.y+0.01 << ",t) title \"middle " << i << "\", "
             << " bez("
             << c1_part2.p0.x << ","
             << c1_part2.p1.x << ","
             << c1_part2.p2.x << ","
             << c1_part2.p3.x << ",t), bez("
             << c1_part2.p0.y << ","
             << c1_part2.p1.y << ","
             << c1_part2.p2.y << ","
             << c1_part2.p3.y << ",t) title \"right " << i << "\", "
             << " bez("
             << c1_part3.p0.x << ","
             << c1_part3.p1.x << ","
             << c1_part3.p2.x << ","
             << c1_part3.p3.x << ",t), bez("
             << c1_part3.p0.y << ","
             << c1_part3.p1.y << ","
             << c1_part3.p2.y << ","
             << c1_part3.p3.y << ",t) title \"left " << i << "\"";

        if( i+1<sizeof(someCurves)/sizeof(Bezier) )
            std::cout << ",\\" << std::endl;
        else
            std::cout << std::endl;
    }
#endif

#ifdef WITH_FATLINE_TEST
    // test fatline algorithm
    const double fatLine_xOffset( curr_Offset );
    curr_Offset += 20;
    std::cout << "# fat line testing" << std::endl
         << "plot [t=0:1] ";
    for( i=0; i<sizeof(someCurves)/sizeof(Bezier); ++i )
    {
        Bezier c( someCurves[i] );

        c.p0.x += fatLine_xOffset;
        c.p1.x += fatLine_xOffset;
        c.p2.x += fatLine_xOffset;
        c.p3.x += fatLine_xOffset;

        FatLine line;

        Impl_calcFatLine(line, c);

        std::cout << " bez("
             << c.p0.x << ","
             << c.p1.x << ","
             << c.p2.x << ","
             << c.p3.x << ",t), bez("
             << c.p0.y << ","
             << c.p1.y << ","
             << c.p2.y << ","
             << c.p3.y << ",t) title \"bezier " << i << "\", linex("
             << line.a << ","
             << line.b << ","
             << line.c << ",t), liney("
             << line.a << ","
             << line.b << ","
             << line.c << ",t) title \"fat line (center) on " << i << "\", linex("
             << line.a << ","
             << line.b << ","
             << line.c-line.dMin << ",t), liney("
             << line.a << ","
             << line.b << ","
             << line.c-line.dMin << ",t) title \"fat line (min) on " << i << "\", linex("
             << line.a << ","
             << line.b << ","
             << line.c-line.dMax << ",t), liney("
             << line.a << ","
             << line.b << ","
             << line.c-line.dMax << ",t) title \"fat line (max) on " << i << "\"";

        if( i+1<sizeof(someCurves)/sizeof(Bezier) )
            std::cout << ",\\" << std::endl;
        else
            std::cout << std::endl;
    }
#endif

#ifdef WITH_CALCFOCUS_TEST
    // test focus curve algorithm
    const double focus_xOffset( curr_Offset );
    curr_Offset += 20;
    std::cout << "# focus line testing" << std::endl
         << "plot [t=0:1] ";
    for( i=0; i<sizeof(someCurves)/sizeof(Bezier); ++i )
    {
        Bezier c( someCurves[i] );

        c.p0.x += focus_xOffset;
        c.p1.x += focus_xOffset;
        c.p2.x += focus_xOffset;
        c.p3.x += focus_xOffset;

        // calc focus curve
        Bezier focus;
        Impl_calcFocus(focus, c);

        std::cout << " bez("
             << c.p0.x << ","
             << c.p1.x << ","
             << c.p2.x << ","
             << c.p3.x << ",t), bez("
             << c.p0.y << ","
             << c.p1.y << ","
             << c.p2.y << ","
             << c.p3.y << ",t) title \"bezier " << i << "\", bez("
             << focus.p0.x << ","
             << focus.p1.x << ","
             << focus.p2.x << ","
             << focus.p3.x << ",t), bez("
             << focus.p0.y << ","
             << focus.p1.y << ","
             << focus.p2.y << ","
             << focus.p3.y << ",t) title \"focus " << i << "\"";

        if( i+1<sizeof(someCurves)/sizeof(Bezier) )
            std::cout << ",\\" << std::endl;
        else
            std::cout << std::endl;
    }
#endif

#ifdef WITH_SAFEPARAMBASE_TEST
    // test safe params base method
    double safeParamsBase_xOffset( curr_Offset );
    std::cout << "# safe param base method testing" << std::endl
         << "plot [t=0:1] ";
    for( i=0; i<sizeof(someCurves)/sizeof(Bezier); ++i )
    {
        Bezier c( someCurves[i] );

        c.p0.x += safeParamsBase_xOffset;
        c.p1.x += safeParamsBase_xOffset;
        c.p2.x += safeParamsBase_xOffset;
        c.p3.x += safeParamsBase_xOffset;

        Polygon2D poly(4);
        poly[0] = c.p0;
        poly[1] = c.p1;
        poly[2] = c.p2;
        poly[3] = c.p3;

        double t1, t2;

        bool bRet( Impl_calcSafeParams( t1, t2, poly, 0, 1 ) );

        Polygon2D convHull( convexHull( poly ) );

        std::cout << " bez("
             << poly[0].x << ","
             << poly[1].x << ","
             << poly[2].x << ","
             << poly[3].x << ",t),bez("
             << poly[0].y << ","
             << poly[1].y << ","
             << poly[2].y << ","
             << poly[3].y << ",t), "
             << "t+" << safeParamsBase_xOffset << ", 0, "
             << "t+" << safeParamsBase_xOffset << ", 1, ";
        if( bRet )
        {
            std::cout << t1+safeParamsBase_xOffset << ", t, "
                 << t2+safeParamsBase_xOffset << ", t, ";
        }
        std::cout << "'-' using ($1):($2) title \"control polygon\" with lp, "
             << "'-' using ($1):($2) title \"convex hull\" with lp";

        if( i+1<sizeof(someCurves)/sizeof(Bezier) )
            std::cout << ",\\" << std::endl;
        else
            std::cout << std::endl;

        safeParamsBase_xOffset += 2;
    }

    safeParamsBase_xOffset = curr_Offset;
    for( i=0; i<sizeof(someCurves)/sizeof(Bezier); ++i )
    {
        Bezier c( someCurves[i] );

        c.p0.x += safeParamsBase_xOffset;
        c.p1.x += safeParamsBase_xOffset;
        c.p2.x += safeParamsBase_xOffset;
        c.p3.x += safeParamsBase_xOffset;

        Polygon2D poly(4);
        poly[0] = c.p0;
        poly[1] = c.p1;
        poly[2] = c.p2;
        poly[3] = c.p3;

        double t1, t2;

        Impl_calcSafeParams( t1, t2, poly, 0, 1 );

        Polygon2D convHull( convexHull( poly ) );

        unsigned int k;
        for( k=0; k<poly.size(); ++k )
        {
            std::cout << poly[k].x << " " << poly[k].y << std::endl;
        }
        std::cout << poly[0].x << " " << poly[0].y << std::endl;
        std::cout << "e" << std::endl;

        for( k=0; k<convHull.size(); ++k )
        {
            std::cout << convHull[k].x << " " << convHull[k].y << std::endl;
        }
        std::cout << convHull[0].x << " " << convHull[0].y << std::endl;
        std::cout << "e" << std::endl;

        safeParamsBase_xOffset += 2;
    }
    curr_Offset += 20;
#endif

#ifdef WITH_SAFEPARAMS_TEST
    // test safe parameter range algorithm
    const double safeParams_xOffset( curr_Offset );
    curr_Offset += 20;
    std::cout << "# safe param range testing" << std::endl
         << "plot [t=0.0:1.0] ";
    for( i=0; i<sizeof(someCurves)/sizeof(Bezier); ++i )
    {
        for( j=i+1; j<sizeof(someCurves)/sizeof(Bezier); ++j )
        {
            Bezier c1( someCurves[i] );
            Bezier c2( someCurves[j] );

            c1.p0.x += safeParams_xOffset;
            c1.p1.x += safeParams_xOffset;
            c1.p2.x += safeParams_xOffset;
            c1.p3.x += safeParams_xOffset;
            c2.p0.x += safeParams_xOffset;
            c2.p1.x += safeParams_xOffset;
            c2.p2.x += safeParams_xOffset;
            c2.p3.x += safeParams_xOffset;

            double t1, t2;

            if( Impl_calcClipRange(t1, t2, c1, c1, c2, c2) )
            {
                // clip safe ranges off c1
                Bezier c1_part1;
                Bezier c1_part2;
                Bezier c1_part3;

                // subdivide at t1_c1
                Impl_deCasteljauAt( c1_part1, c1_part2, c1, t1 );
                // subdivide at t2_c1
                Impl_deCasteljauAt( c1_part1, c1_part3, c1_part2, (t2-t1)/(1.0-t1) );

                // output remaining segment (c1_part1)

                std::cout << " bez("
                     << c1.p0.x << ","
                     << c1.p1.x << ","
                     << c1.p2.x << ","
                     << c1.p3.x << ",t),bez("
                     << c1.p0.y << ","
                     << c1.p1.y << ","
                     << c1.p2.y << ","
                     << c1.p3.y << ",t), bez("
                     << c2.p0.x << ","
                     << c2.p1.x << ","
                     << c2.p2.x << ","
                     << c2.p3.x << ",t),bez("
                     << c2.p0.y << ","
                     << c2.p1.y << ","
                     << c2.p2.y << ","
                     << c2.p3.y << ",t), bez("
                     << c1_part1.p0.x << ","
                     << c1_part1.p1.x << ","
                     << c1_part1.p2.x << ","
                     << c1_part1.p3.x << ",t),bez("
                     << c1_part1.p0.y << ","
                     << c1_part1.p1.y << ","
                     << c1_part1.p2.y << ","
                     << c1_part1.p3.y << ",t)";

                if( i+2<sizeof(someCurves)/sizeof(Bezier) )
                    std::cout << ",\\" << std::endl;
                else
                    std::cout << std::endl;
            }
        }
    }
#endif

#ifdef WITH_SAFEPARAM_DETAILED_TEST
    // test safe parameter range algorithm
    const double safeParams2_xOffset( curr_Offset );
    curr_Offset += 20;
    if( sizeof(someCurves)/sizeof(Bezier) > 1 )
    {
        Bezier c1( someCurves[0] );
        Bezier c2( someCurves[1] );

        c1.p0.x += safeParams2_xOffset;
        c1.p1.x += safeParams2_xOffset;
        c1.p2.x += safeParams2_xOffset;
        c1.p3.x += safeParams2_xOffset;
        c2.p0.x += safeParams2_xOffset;
        c2.p1.x += safeParams2_xOffset;
        c2.p2.x += safeParams2_xOffset;
        c2.p3.x += safeParams2_xOffset;

        double t1, t2;

        // output happens here
        Impl_calcClipRange(t1, t2, c1, c1, c2, c2);
    }
#endif

#ifdef WITH_SAFEFOCUSPARAM_TEST
    // test safe parameter range from focus algorithm
    const double safeParamsFocus_xOffset( curr_Offset );
    curr_Offset += 20;
    std::cout << "# safe param range from focus testing" << std::endl
         << "plot [t=0.0:1.0] ";
    for( i=0; i<sizeof(someCurves)/sizeof(Bezier); ++i )
    {
        for( j=i+1; j<sizeof(someCurves)/sizeof(Bezier); ++j )
        {
            Bezier c1( someCurves[i] );
            Bezier c2( someCurves[j] );

            c1.p0.x += safeParamsFocus_xOffset;
            c1.p1.x += safeParamsFocus_xOffset;
            c1.p2.x += safeParamsFocus_xOffset;
            c1.p3.x += safeParamsFocus_xOffset;
            c2.p0.x += safeParamsFocus_xOffset;
            c2.p1.x += safeParamsFocus_xOffset;
            c2.p2.x += safeParamsFocus_xOffset;
            c2.p3.x += safeParamsFocus_xOffset;

            double t1, t2;

            Bezier focus;
#ifdef WITH_SAFEFOCUSPARAM_CALCFOCUS
#if 0
            {
                // clip safe ranges off c1_orig
                Bezier c1_part1;
                Bezier c1_part2;
                Bezier c1_part3;

                // subdivide at t1_c1
                Impl_deCasteljauAt( c1_part1, c1_part2, c2, 0.30204 );

                // subdivide at t2_c1. As we're working on
                // c1_part2 now, we have to adapt t2_c1 since
                // we're no longer in the original parameter
                // interval. This is based on the following
                // assumption: t2_new = (t2-t1)/(1-t1), which
                // relates the t2 value into the new parameter
                // range [0,1] of c1_part2.
                Impl_deCasteljauAt( c1_part1, c1_part3, c1_part2, (0.57151-0.30204)/(1.0-0.30204) );

                c2 = c1_part1;
                Impl_calcFocus( focus, c2 );
            }
#else
            Impl_calcFocus( focus, c2 );
#endif
#else
            focus = c2;
#endif
            // determine safe range on c1
            bool bRet( Impl_calcSafeParams_focus( t1, t2,
                                                  c1, focus ) );

            std::cerr << "t1: " << t1 << ", t2: " << t2 << std::endl;

            // clip safe ranges off c1
            Bezier c1_part1;
            Bezier c1_part2;
            Bezier c1_part3;

            // subdivide at t1_c1
            Impl_deCasteljauAt( c1_part1, c1_part2, c1, t1 );
            // subdivide at t2_c1
            Impl_deCasteljauAt( c1_part1, c1_part3, c1_part2, (t2-t1)/(1.0-t1) );

            // output remaining segment (c1_part1)

            std::cout << " bez("
                 << c1.p0.x << ","
                 << c1.p1.x << ","
                 << c1.p2.x << ","
                 << c1.p3.x << ",t),bez("
                 << c1.p0.y << ","
                 << c1.p1.y << ","
                 << c1.p2.y << ","
                 << c1.p3.y << ",t) title \"c1\", "
#ifdef WITH_SAFEFOCUSPARAM_CALCFOCUS
                 << "bez("
                 << c2.p0.x << ","
                 << c2.p1.x << ","
                 << c2.p2.x << ","
                 << c2.p3.x << ",t),bez("
                 << c2.p0.y << ","
                 << c2.p1.y << ","
                 << c2.p2.y << ","
                 << c2.p3.y << ",t) title \"c2\", "
                 << "bez("
                 << focus.p0.x << ","
                 << focus.p1.x << ","
                 << focus.p2.x << ","
                 << focus.p3.x << ",t),bez("
                 << focus.p0.y << ","
                 << focus.p1.y << ","
                 << focus.p2.y << ","
                 << focus.p3.y << ",t) title \"focus\"";
#else
                 << "bez("
                 << c2.p0.x << ","
                 << c2.p1.x << ","
                 << c2.p2.x << ","
                 << c2.p3.x << ",t),bez("
                 << c2.p0.y << ","
                 << c2.p1.y << ","
                 << c2.p2.y << ","
                 << c2.p3.y << ",t) title \"focus\"";
#endif
            if( bRet )
            {
                std::cout << ", bez("
                     << c1_part1.p0.x << ","
                     << c1_part1.p1.x << ","
                     << c1_part1.p2.x << ","
                     << c1_part1.p3.x << ",t),bez("
                     << c1_part1.p0.y+0.01 << ","
                     << c1_part1.p1.y+0.01 << ","
                     << c1_part1.p2.y+0.01 << ","
                     << c1_part1.p3.y+0.01 << ",t) title \"part\"";
            }

            if( i+2<sizeof(someCurves)/sizeof(Bezier) )
                std::cout << ",\\" << std::endl;
            else
                std::cout << std::endl;
        }
    }
#endif

#ifdef WITH_SAFEFOCUSPARAM_DETAILED_TEST
    // test safe parameter range algorithm
    const double safeParams3_xOffset( curr_Offset );
    curr_Offset += 20;
    if( sizeof(someCurves)/sizeof(Bezier) > 1 )
    {
        Bezier c1( someCurves[0] );
        Bezier c2( someCurves[1] );

        c1.p0.x += safeParams3_xOffset;
        c1.p1.x += safeParams3_xOffset;
        c1.p2.x += safeParams3_xOffset;
        c1.p3.x += safeParams3_xOffset;
        c2.p0.x += safeParams3_xOffset;
        c2.p1.x += safeParams3_xOffset;
        c2.p2.x += safeParams3_xOffset;
        c2.p3.x += safeParams3_xOffset;

        double t1, t2;

        Bezier focus;
#ifdef WITH_SAFEFOCUSPARAM_CALCFOCUS
        Impl_calcFocus( focus, c2 );
#else
        focus = c2;
#endif

        // determine safe range on c1, output happens here
        Impl_calcSafeParams_focus( t1, t2,
                                   c1, focus );
    }
#endif

#ifdef WITH_BEZIERCLIP_TEST
    std::vector< std::pair<double, double> >                                result;
    std::back_insert_iterator< std::vector< std::pair<double, double> > > ii(result);

    // test full bezier clipping
    const double bezierClip_xOffset( curr_Offset );
    std::cout << std::endl << std::endl << "# bezier clip testing" << std::endl
         << "plot [t=0:1] ";
    for( i=0; i<sizeof(someCurves)/sizeof(Bezier); ++i )
    {
        for( j=i+1; j<sizeof(someCurves)/sizeof(Bezier); ++j )
        {
            Bezier c1( someCurves[i] );
            Bezier c2( someCurves[j] );

            c1.p0.x += bezierClip_xOffset;
            c1.p1.x += bezierClip_xOffset;
            c1.p2.x += bezierClip_xOffset;
            c1.p3.x += bezierClip_xOffset;
            c2.p0.x += bezierClip_xOffset;
            c2.p1.x += bezierClip_xOffset;
            c2.p2.x += bezierClip_xOffset;
            c2.p3.x += bezierClip_xOffset;

            std::cout << " bez("
                 << c1.p0.x << ","
                 << c1.p1.x << ","
                 << c1.p2.x << ","
                 << c1.p3.x << ",t),bez("
                 << c1.p0.y << ","
                 << c1.p1.y << ","
                 << c1.p2.y << ","
                 << c1.p3.y << ",t), bez("
                 << c2.p0.x << ","
                 << c2.p1.x << ","
                 << c2.p2.x << ","
                 << c2.p3.x << ",t),bez("
                 << c2.p0.y << ","
                 << c2.p1.y << ","
                 << c2.p2.y << ","
                 << c2.p3.y << ",t), '-' using (bez("
                 << c1.p0.x << ","
                 << c1.p1.x << ","
                 << c1.p2.x << ","
                 << c1.p3.x
                 << ",$1)):(bez("
                 << c1.p0.y << ","
                 << c1.p1.y << ","
                 << c1.p2.y << ","
                 << c1.p3.y << ",$1)) title \"bezier " << i << " clipped against " << j << " (t on " << i << ")\", "
                 << " '-' using (bez("
                 << c2.p0.x << ","
                 << c2.p1.x << ","
                 << c2.p2.x << ","
                 << c2.p3.x
                 << ",$1)):(bez("
                 << c2.p0.y << ","
                 << c2.p1.y << ","
                 << c2.p2.y << ","
                 << c2.p3.y << ",$1)) title \"bezier " << i << " clipped against " << j << " (t on " << j << ")\"";

            if( i+2<sizeof(someCurves)/sizeof(Bezier) )
                std::cout << ",\\" << std::endl;
            else
                std::cout << std::endl;
        }
    }
    for( i=0; i<sizeof(someCurves)/sizeof(Bezier); ++i )
    {
        for( j=i+1; j<sizeof(someCurves)/sizeof(Bezier); ++j )
        {
            result.clear();
            Bezier c1( someCurves[i] );
            Bezier c2( someCurves[j] );

            c1.p0.x += bezierClip_xOffset;
            c1.p1.x += bezierClip_xOffset;
            c1.p2.x += bezierClip_xOffset;
            c1.p3.x += bezierClip_xOffset;
            c2.p0.x += bezierClip_xOffset;
            c2.p1.x += bezierClip_xOffset;
            c2.p2.x += bezierClip_xOffset;
            c2.p3.x += bezierClip_xOffset;

            clipBezier( ii, 0.00001, c1, c2 );

            for( k=0; k<result.size(); ++k )
            {
                std::cout << result[k].first << std::endl;
            }
            std::cout << "e" << std::endl;

            for( k=0; k<result.size(); ++k )
            {
                std::cout << result[k].second << std::endl;
            }
            std::cout << "e" << std::endl;
        }
    }
#endif

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
