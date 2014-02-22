/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <algorithm>
#include <iterator>
#include <vector>
#include <utility>

#include <math.h>

#include "bezierclip.hxx"
#include "gauss.hxx"




#define WITH_ASSERTIONS










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
    assert(n>=k); 
    assert(n>=0); 
    assert(k>=0); 
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
    
    

    
    line.a = -(c.p0.y - c.p3.y);
    line.b = (c.p0.x - c.p3.x);

    
    const double len( sqrt( line.a*line.a + line.b*line.b ) );
    if( !tolZero(len) )
    {
        line.a /= len;
        line.b /= len;
    }

    line.c = -(line.a*c.p0.x + line.b*c.p0.y);


    
    

    
    const double dP2( calcLineDistance(line.a, line.b, line.c, c.p1.x, c.p1.y ) );
    const double dP3( calcLineDistance(line.a, line.b, line.c, c.p2.x, c.p2.y ) );

    
    
    
    if( dP2 * dP3 > 0.0 )
    {
        line.dMin = 3.0/4.0 * ::std::min(0.0, ::std::min(dP2, dP3));
        line.dMax = 3.0/4.0 * ::std::max(0.0, ::std::max(dP2, dP3));
    }
    else
    {
        line.dMin = 4.0/9.0 * ::std::min(0.0, ::std::min(dP2, dP3));
        line.dMax = 4.0/9.0 * ::std::max(0.0, ::std::max(dP2, dP3));
    }
}

void Impl_calcBounds( Point2D&          leftTop,
                      Point2D&          rightBottom,
                      const Bezier&     c1          )
{
    leftTop.x = ::std::min( c1.p0.x, ::std::min( c1.p1.x, ::std::min( c1.p2.x, c1.p3.x ) ) );
    leftTop.y = ::std::min( c1.p0.y, ::std::min( c1.p1.y, ::std::min( c1.p2.y, c1.p3.y ) ) );
    rightBottom.x = ::std::max( c1.p0.x, ::std::max( c1.p1.x, ::std::max( c1.p2.x, c1.p3.x ) ) );
    rightBottom.y = ::std::max( c1.p0.y, ::std::max( c1.p1.y, ::std::max( c1.p2.y, c1.p3.y ) ) );
}

bool Impl_doBBoxIntersect( const Bezier& c1,
                           const Bezier& c2 )
{
    
    Point2D lt1;
    Point2D rb1;
    Point2D lt2;
    Point2D rb2;

    Impl_calcBounds( lt1, rb1, c1 );
    Impl_calcBounds( lt2, rb2, c2 );

    if( ::std::min(rb1.x, rb2.x) < ::std::max(lt1.x, lt2.x) ||
        ::std::min(rb1.y, rb2.y) < ::std::max(lt1.y, lt2.y) )
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
    
    
    Polygon2D convHull( convexHull(rPoly) );

    
    t1 = 0.0 ;
    double currLowerT( 1.0 );

    t2 = 1.0;
    double currHigherT( 0.0 );

    if( convHull.size() <= 1 )
        return false; 

    /* now, clip against lower and higher bounds */
    Point2D p0;
    Point2D p1;

    bool bIntersection( false );

    for( Polygon2D::size_type i=0; i<convHull.size(); ++i )
    {
        
        
        
        if( i+1 == convHull.size() )
        {
            
            p0 = convHull[i];
            p1 = convHull[0];
        }
        else
        {
            p0 = convHull[i];
            p1 = convHull[i+1];
        }

        
        
        
        
        
        
        if( (tolLessEqual(p0.y, upperYBound) || tolLessEqual(p1.y, upperYBound)) &&
            (tolGreaterEqual(p0.y, lowerYBound) || tolGreaterEqual(p1.y, lowerYBound)) )
        {
            
            
            
            const double r_y( p1.y - p0.y );

            if( tolZero(r_y) )
            {
                
                
                
                if( tolEqual(p0.y, lowerYBound) ||
                    tolEqual(p0.y, upperYBound) )
                {
                    
                    currLowerT = ::std::min(currLowerT, ::std::min(p0.x, p1.x));
                    currHigherT = ::std::max(currHigherT, ::std::max(p0.x, p1.x));
                }
            }
            else
            {
                
                
                const double r_x( p1.x - p0.x );

                
                const double currTLow( (lowerYBound - p0.y) * r_x / r_y + p0.x );

                
                const double currTHigh( (upperYBound - p0.y) * r_x / r_y + p0.x );

                currLowerT = ::std::min(currLowerT, ::std::min(currTLow, currTHigh));
                currHigherT = ::std::max(currHigherT, ::std::max(currTLow, currTHigh));
            }

            
            
            bIntersection = true;
        }
    }

#ifndef WITH_SAFEPARAMBASE_TEST
    
    t1 = ::std::max(0.0, currLowerT);
    t2 = ::std::min(1.0, currHigherT);
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

    cout << "# convex hull testing" << endl
         << "plot [t=0:1] ";
    cout << " bez("
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
         << "'-' using ($1):($2) title \"convex hull\" with lp" << endl;

    unsigned int k;
    for( k=0; k<poly.size(); ++k )
    {
        cout << poly[k].x << " " << poly[k].y << endl;
    }
    cout << poly[0].x << " " << poly[0].y << endl;
    cout << "e" << endl;

    for( k=0; k<convHull.size(); ++k )
    {
        cout << convHull[k].x << " " << convHull[k].y << endl;
    }
    cout << convHull[0].x << " " << convHull[0].y << endl;
    cout << "e" << endl;

    return bRet;
#endif
}



void Impl_deCasteljauAt( Bezier&        part1,
                         Bezier&        part2,
                         const Bezier&  input,
                         double         t        )
{
    
    //
    
    
    
    
    //
    
    //
    
    
    
    
    
    if( tolZero(t) )
    {
        
        part1.p0.x = part1.p1.x = part1.p2.x = part1.p3.x = input.p0.x;
        part1.p0.y = part1.p1.y = part1.p2.y = part1.p3.y = input.p0.y;
        part2 = input;
    }
    else if( tolEqual(t, 1.0) )
    {
        
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

    cout << "# safe param range testing" << endl
         << "plot [t=0.0:1.0] ";

    
    Bezier c1_part1;
    Bezier c1_part2;
    Bezier c1_part3;

    
    Impl_deCasteljauAt( c1_part1, c1_part2, c1, t1_c1 );
    
    Impl_deCasteljauAt( c1_part1, c1_part3, c1_part2, t2_c1 );

    

    cout << "bez("
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
         << bounds_c2.c-bounds_c2.dMax << ",t) title \"fat line (max) \"" << endl;

    offset += 1;
}



void printResultWithFinalCurves( const Bezier& c1, const Bezier& c1_part,
                                 const Bezier& c2, const Bezier& c2_part,
                                 double t1_c1, double t2_c1 )
{
    static int offset = 0;

    cout << "# final result" << endl
         << "plot [t=0.0:1.0] ";

    cout << "bez("
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
         << c2_part.p3.y << ",t)" << endl;

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
    
    

    if( Impl_doBBoxIntersect(c1_part, c2_part) )
    {
        
        FatLine bounds_c2;

        
        
        
        Impl_calcFatLine(bounds_c2, c2_part);

        
        
        
        
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
            

            
            return true;
        }
    }

    
    return false;
}



/* Tangent intersection part
 * =========================
 */



void Impl_calcFocus( Bezier& res, const Bezier& c )
{
    
    
    const double minPivotValue( 1.0e-20 );

    Point2D::value_type fMatrix[6];
    Point2D::value_type fRes[2];

    

    
    //
    
    //
    
    //
    
    //
    
    //
    
    
    //
    
    
    //
    
    //
    
    
    //
    
    
    //
    
    //
    
    //
    
    
    //
    
    //
    
    
    //
    
    //
    
    
    //
    
    //
    
    
    //

    
    fMatrix[0] = c.p1.x - c.p0.x;
    fMatrix[1] = c.p2.x - c.p3.x;
    fMatrix[2] = (c.p3.y - c.p0.y)/3.0;
    fMatrix[3] = c.p0.y - c.p1.y;
    fMatrix[4] = c.p3.y - c.p2.y;
    fMatrix[5] = (c.p3.x - c.p0.x)/3.0;

    
    if( !solve(fMatrix, 2, 3, fRes, minPivotValue) )
    {
        
        
        
        fRes[0] = 0.0;
        fRes[1] = 1.0;

        cerr << "Matrix singular!" << endl;
    }

    
    
    //
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    //
    
    
    
    
    
    
    
    
    
    //
    
    //
    
    
    
    
    //
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
    
    
    
    
    
    //
    
    //
    
    
    //
    
    //
    
    
    //
    
    
    //
    
    
    //
    
    //
    
    //
    
    //
    //
    
    
    
    
    
    //
    
    
    //
    
    //
    
    
    //
    
    
    
    
    
    
    //
    
    
    //

    
    
    
    
    

    //
    
    //
    const int n( 3 ); 
    const int i_card( 2*n );
    const int j_card( n + 1 );
    const int k_max( n-1 );
    Polygon2D controlPolygon( i_card*j_card ); 

    int i, j, k, l; 
    Point2D::value_type d;
    for( i=0; i<i_card; ++i )
    {
        for( j=0; j<j_card; ++j )
        {
            
            for( d=0.0, k=::std::max(0,i-n); k<=k_max && k<=i; ++k )
            {
                l = i - k; 
                assert(k>=0 && k<=n-1); 
                assert(l>=0 && l<=n);   

                
                
                
                d += static_cast<double>(fallFac(n,l)*fallFac(n-1,k)*fac(i)) /
                    static_cast<double>(fac(l)*fac(k) * fallFac(2*n-1,i)) * n *
                    ( (curve[k+1].x - curve[k].x)*(curve[l].x - focus[j].x) +   
                      (curve[k+1].y - curve[k].y)*(curve[l].y - focus[j].y) );
            }

            
            
            controlPolygon[ i*j_card + j ] = Point2D( i/(2.0*n-1.0), d );
        }
    }

#ifndef WITH_SAFEFOCUSPARAM_DETAILED_TEST

    
    
    return Impl_calcSafeParams( t1, t2, controlPolygon, 0.0, 0.0 );

#else
    bool bRet( Impl_calcSafeParams( t1, t2, controlPolygon, 0.0, 0.0 ) );

    Polygon2D convHull( convexHull( controlPolygon ) );

    cout << "# convex hull testing (focus)" << endl
         << "plot [t=0:1] ";
    cout << "'-' using ($1):($2) title \"control polygon\" with lp, "
         << "'-' using ($1):($2) title \"convex hull\" with lp" << endl;

    unsigned int count;
    for( count=0; count<controlPolygon.size(); ++count )
    {
        cout << controlPolygon[count].x << " " << controlPolygon[count].y << endl;
    }
    cout << controlPolygon[0].x << " " << controlPolygon[0].y << endl;
    cout << "e" << endl;

    for( count=0; count<convHull.size(); ++count )
    {
        cout << convHull[count].x << " " << convHull[count].y << endl;
    }
    cout << convHull[0].x << " " << convHull[0].y << endl;
    cout << "e" << endl;

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
template <class Functor> void Impl_applySafeRanges_rec( ::std::back_insert_iterator< ::std::vector< ::std::pair<double, double> > >&    result,
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
    
    

    
    

    
    
    
    
    
    
    

    for( int i=0; i<recursionLevel; ++i ) cerr << " ";
    if( recursionLevel % 2 )
    {
        cerr << "level: " << recursionLevel
             << " t: "
             << last_t1_c2 + (last_t2_c2 - last_t1_c2)/2.0
             << ", c1: " << last_t1_c2 << " " << last_t2_c2
             << ", c2: " << last_t1_c1 << " " << last_t2_c1
             << endl;
    }
    else
    {
        cerr << "level: " << recursionLevel
             << " t: "
             << last_t1_c1 + (last_t2_c1 - last_t1_c1)/2.0
             << ", c1: " << last_t1_c1 << " " << last_t2_c1
             << ", c2: " << last_t1_c2 << " " << last_t2_c2
             << endl;
    }

    
    

    double t1_c1, t2_c1;

    
    
    
    
    

    
    if( safeRangeFunctor( t1_c1, t2_c1, c1_orig, c1_part, c2_orig, c2_part ) )
    {
        
        
        
        
        
        if( tolLessEqual(t1_c1, last_t2_c1) && tolGreaterEqual(t2_c1, last_t1_c1) )
        {
            
            
            
            
            
            
            
            t1_c1 = ::std::max(t1_c1, last_t1_c1);
            t2_c1 = ::std::min(t2_c1, last_t2_c1);

            
            

            
            

#if 1
            if( fabs(last_t2_c1 - last_t1_c1) < 0.0001 &&
                fabs(last_t2_c2 - last_t1_c2) < 0.0001  )
#else
            if( fabs(last_t2_c1 - last_t1_c1) < 0.01 &&
                fabs(last_t2_c2 - last_t1_c2) < 0.01    )
#endif
            {
                
                if( recursionLevel % 2 )
                {
                    
                    *result++ = ::std::make_pair( last_t1_c2 + (last_t2_c2 - last_t1_c2)/2.0,
                                                  last_t1_c1 + (last_t2_c1 - last_t1_c1)/2.0 );
                }
                else
                {
                    *result++ = ::std::make_pair( last_t1_c1 + (last_t2_c1 - last_t1_c1)/2.0,
                                                  last_t1_c2 + (last_t2_c2 - last_t1_c2)/2.0 );
                }

#if 0
                
                printResultWithFinalCurves( c1_orig, c1_part, c2_orig, c2_part, t1_c1, t2_c1 );
#else
                
                Bezier focus;
                Impl_calcFocus(focus, c2_part); 

                safeRangeFunctor( t1_c1, t2_c1, c1_orig, c1_part, c2_orig, c2_part );

                
                printResultWithFinalCurves( c1_orig, c1_part, c2_orig, focus, last_t1_c1, last_t2_c1 );
#endif
            }
            else
            {
                
                
                

                if( false )
                {
                    
                    

                    Bezier part1;
                    Bezier part2;

                    double intervalMiddle;

                    if( last_t2_c1 - last_t1_c1 > last_t2_c2 - last_t1_c2 )
                    {
                        
                        

                        intervalMiddle = last_t1_c1 + (last_t2_c1 - last_t1_c1)/2.0;

                        
                        
                        
                        
                        Impl_deCasteljauAt( part1, part2, c1_part, 0.5 );

                        
                        Impl_applySafeRanges_rec( result, delta, safeRangeFunctor, recursionLevel+1,
                                                  c2_orig, c2_part, last_t1_c2, last_t2_c2,
                                                  c1_orig, part1, last_t1_c1, intervalMiddle );

                        Impl_applySafeRanges_rec( result, delta, safeRangeFunctor, recursionLevel+1,
                                                  c2_orig, c2_part, last_t1_c2, last_t2_c2,
                                                  c1_orig, part2, intervalMiddle, last_t2_c1 );
                    }
                    else
                    {
                        
                        

                        intervalMiddle = last_t1_c2 + (last_t2_c2 - last_t1_c2)/2.0;

                        
                        
                        
                        
                        Impl_deCasteljauAt( part1, part2, c2_part, 0.5 );

                        
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
                    
                    

                    
                    Bezier c1_part1;
                    Bezier c1_part2;
                    Bezier c1_part3;

                    
                    Impl_deCasteljauAt( c1_part1, c1_part2, c1_orig, t1_c1 );

                    
                    
                    
                    
                    
                    
                    
                    Impl_deCasteljauAt( c1_part1, c1_part3, c1_part2, (t2_c1-t1_c1)/(1.0-t1_c1) );

                    
                    
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
        
        Bezier focus;
        Impl_calcFocus(focus, c2_part); 
                                        
                                        

        
        bool bRet( Impl_calcSafeParams_focus( t1_c1, t2_c1,
                                              c1_orig, 
                                              focus ) );

        cerr << "range: " << t2_c1 - t1_c1 << ", ret: " << bRet << endl;

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
void clipBezier( ::std::back_insert_iterator< ::std::vector< ::std::pair<double, double> > >&   result,
                 double                                                                         delta,
                 const Bezier&                                                                  c1,
                 const Bezier&                                                                  c2        )
{
#if 0
    
    
    
    
    

    
    
    

    ::std::vector< ::std::pair<double,double> > results;
    ::std::back_insert_iterator< ::std::vector< ::std::pair<double, double> > > ii(results);

    Impl_calcCollinearNormals( ii, delta, 0, c1, c1, 0.0, 1.0, c2, c2, 0.0, 1.0 );

    
    
    
    
    
    
    

    
    ::std::vector< Bezier > c1_segments( results.size()+1 );
    ::std::vector< Bezier > c2_segments( results.size()+1 );
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
    
#endif
    
}

int main(int argc, const char *argv[])
{
    double curr_Offset( 0 );
    unsigned int i,j,k;

    Bezier someCurves[] =
        {












































            {Point2D(2.505,0.0),Point2D(2.505+4.915,4.300),Point2D(2.505+3.213,10.019),Point2D(2.505-2.505,10.255)},
            {Point2D(5.33+9.311,0.0),Point2D(5.33+9.311-13.279,4.205),Point2D(5.33+9.311-10.681,9.119),Point2D(5.33+9.311-2.603,10.254)}














        };

    
    cout << "#!/usr/bin/gnuplot -persist" << endl
         << "#" << endl
         << "# automatically generated by bezierclip, don't change!" << endl
         << "#" << endl
         << "set parametric" << endl
         << "bez(p,q,r,s,t) = p*(1-t)**3+q*3*(1-t)**2*t+r*3*(1-t)*t**2+s*t**3" << endl
         << "bezd(p,q,r,s,t) = 3*(q-p)*(1-t)**2+6*(r-q)*(1-t)*t+3*(s-r)*t**2" << endl
         << "pointmarkx(c,t) = c-0.03*t" << endl
         << "pointmarky(c,t) = c+0.03*t" << endl
         << "linex(a,b,c,t) = a*-c + t*-b" << endl
         << "liney(a,b,c,t) = b*-c + t*a" << endl << endl
         << "# end of setup" << endl << endl;

#ifdef WITH_CONVEXHULL_TEST
    
    const double convHull_xOffset( curr_Offset );
    curr_Offset += 20;
    cout << "# convex hull testing" << endl
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

        cout << " bez("
             << aTestPoly[0].x << ","
             << aTestPoly[1].x << ","
             << aTestPoly[2].x << ","
             << aTestPoly[3].x << ",t),bez("
             << aTestPoly[0].y << ","
             << aTestPoly[1].y << ","
             << aTestPoly[2].y << ","
             << aTestPoly[3].y << ",t), '-' using ($1):($2) title \"convex hull " << i << "\" with lp";

        if( i+1<sizeof(someCurves)/sizeof(Bezier) )
            cout << ",\\" << endl;
        else
            cout << endl;
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
            cout << convHull[k].x << " " << convHull[k].y << endl;
        }
        cout << convHull[0].x << " " << convHull[0].y << endl;
        cout << "e" << endl;
    }
#endif

#ifdef WITH_MULTISUBDIVIDE_TEST
    
    const double multiSubdivide_xOffset( curr_Offset );
    curr_Offset += 20;
    cout << "# multi subdivide testing" << endl
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

        
        Impl_deCasteljauAt( c1_part1, c1_part2, c, t1 );

        
        
        
        
        
        
        
        Impl_deCasteljauAt( c1_part1, c1_part3, c1_part2, (t2-t1)/(1.0-t1) );

        
        Impl_deCasteljauAt( c1_part3, c1_part2, c, t2 );

        cout << " bez("
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
            cout << ",\\" << endl;
        else
            cout << endl;
    }
#endif

#ifdef WITH_FATLINE_TEST
    
    const double fatLine_xOffset( curr_Offset );
    curr_Offset += 20;
    cout << "# fat line testing" << endl
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

        cout << " bez("
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
            cout << ",\\" << endl;
        else
            cout << endl;
    }
#endif

#ifdef WITH_CALCFOCUS_TEST
    
    const double focus_xOffset( curr_Offset );
    curr_Offset += 20;
    cout << "# focus line testing" << endl
         << "plot [t=0:1] ";
    for( i=0; i<sizeof(someCurves)/sizeof(Bezier); ++i )
    {
        Bezier c( someCurves[i] );

        c.p0.x += focus_xOffset;
        c.p1.x += focus_xOffset;
        c.p2.x += focus_xOffset;
        c.p3.x += focus_xOffset;

        
        Bezier focus;
        Impl_calcFocus(focus, c);

        cout << " bez("
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
            cout << ",\\" << endl;
        else
            cout << endl;
    }
#endif

#ifdef WITH_SAFEPARAMBASE_TEST
    
    double safeParamsBase_xOffset( curr_Offset );
    cout << "# safe param base method testing" << endl
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

        cout << " bez("
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
            cout << t1+safeParamsBase_xOffset << ", t, "
                 << t2+safeParamsBase_xOffset << ", t, ";
        }
        cout << "'-' using ($1):($2) title \"control polygon\" with lp, "
             << "'-' using ($1):($2) title \"convex hull\" with lp";

        if( i+1<sizeof(someCurves)/sizeof(Bezier) )
            cout << ",\\" << endl;
        else
            cout << endl;

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
            cout << poly[k].x << " " << poly[k].y << endl;
        }
        cout << poly[0].x << " " << poly[0].y << endl;
        cout << "e" << endl;

        for( k=0; k<convHull.size(); ++k )
        {
            cout << convHull[k].x << " " << convHull[k].y << endl;
        }
        cout << convHull[0].x << " " << convHull[0].y << endl;
        cout << "e" << endl;

        safeParamsBase_xOffset += 2;
    }
    curr_Offset += 20;
#endif

#ifdef WITH_SAFEPARAMS_TEST
    
    const double safeParams_xOffset( curr_Offset );
    curr_Offset += 20;
    cout << "# safe param range testing" << endl
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
                
                Bezier c1_part1;
                Bezier c1_part2;
                Bezier c1_part3;

                
                Impl_deCasteljauAt( c1_part1, c1_part2, c1, t1 );
                
                Impl_deCasteljauAt( c1_part1, c1_part3, c1_part2, (t2-t1)/(1.0-t1) );

                

                cout << " bez("
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
                    cout << ",\\" << endl;
                else
                    cout << endl;
            }
        }
    }
#endif

#ifdef WITH_SAFEPARAM_DETAILED_TEST
    
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

        
        Impl_calcClipRange(t1, t2, c1, c1, c2, c2);
    }
#endif

#ifdef WITH_SAFEFOCUSPARAM_TEST
    
    const double safeParamsFocus_xOffset( curr_Offset );
    curr_Offset += 20;
    cout << "# safe param range from focus testing" << endl
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
                
                Bezier c1_part1;
                Bezier c1_part2;
                Bezier c1_part3;

                
                Impl_deCasteljauAt( c1_part1, c1_part2, c2, 0.30204 );

                
                
                
                
                
                
                
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
            
            bool bRet( Impl_calcSafeParams_focus( t1, t2,
                                                  c1, focus ) );

            cerr << "t1: " << t1 << ", t2: " << t2 << endl;

            
            Bezier c1_part1;
            Bezier c1_part2;
            Bezier c1_part3;

            
            Impl_deCasteljauAt( c1_part1, c1_part2, c1, t1 );
            
            Impl_deCasteljauAt( c1_part1, c1_part3, c1_part2, (t2-t1)/(1.0-t1) );

            

            cout << " bez("
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
                cout << ", bez("
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
                cout << ",\\" << endl;
            else
                cout << endl;
        }
    }
#endif

#ifdef WITH_SAFEFOCUSPARAM_DETAILED_TEST
    
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

        
        Impl_calcSafeParams_focus( t1, t2,
                                   c1, focus );
    }
#endif

#ifdef WITH_BEZIERCLIP_TEST
    ::std::vector< ::std::pair<double, double> >                                result;
    ::std::back_insert_iterator< ::std::vector< ::std::pair<double, double> > > ii(result);

    
    const double bezierClip_xOffset( curr_Offset );
    curr_Offset += 20;
    cout << endl << endl << "# bezier clip testing" << endl
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

            cout << " bez("
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
                cout << ",\\" << endl;
            else
                cout << endl;
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
                cout << result[k].first << endl;
            }
            cout << "e" << endl;

            for( k=0; k<result.size(); ++k )
            {
                cout << result[k].second << endl;
            }
            cout << "e" << endl;
        }
    }
#endif

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
