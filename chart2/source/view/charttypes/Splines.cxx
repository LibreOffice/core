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

#include "Splines.hxx"
#include <osl/diagnose.h>
#include <com/sun/star/drawing/Position3D.hpp>

#include <vector>
#include <algorithm>
#include <optional>
#include <cmath>
#include <limits>

namespace chart
{
using namespace ::com::sun::star;

namespace
{

typedef std::pair< double, double >   tPointType;
typedef std::vector< tPointType >     tPointVecType;
typedef tPointVecType::size_type        lcl_tSizeType;

class lcl_SplineCalculation
{
public:
    /** @descr creates an object that calculates cubic splines on construction

        @param rSortedPoints  the points for which splines shall be calculated, they need to be sorted in x values
        @param fY1FirstDerivation the resulting spline should have the first
               derivation equal to this value at the x-value of the first point
               of rSortedPoints.  If fY1FirstDerivation is set to infinity, a natural
               spline is calculated.
        @param fYnFirstDerivation the resulting spline should have the first
               derivation equal to this value at the x-value of the last point
               of rSortedPoints
     */
    lcl_SplineCalculation( tPointVecType && rSortedPoints,
                           double fY1FirstDerivation,
                           double fYnFirstDerivation );

    /** @descr creates an object that calculates cubic splines on construction
               for the special case of periodic cubic spline

        @param rSortedPoints  the points for which splines shall be calculated,
               they need to be sorted in x values. First and last y value must be equal
     */
    explicit lcl_SplineCalculation( tPointVecType && rSortedPoints);

    /** @descr this function corresponds to the function splint in [1].

        [1] Numerical Recipes in C, 2nd edition
            William H. Press, et al.,
            Section 3.3, page 116
    */
    double GetInterpolatedValue( double x );

private:
    /// a copy of the points given in the CTOR
    tPointVecType            m_aPoints;

    /// the result of the Calculate() method
    std::vector< double >         m_aSecDerivY;

    double m_fYp1;
    double m_fYpN;

    // these values are cached for performance reasons
    lcl_tSizeType m_nKLow;
    lcl_tSizeType m_nKHigh;
    double m_fLastInterpolatedValue;

    /** @descr this function corresponds to the function spline in [1].

        [1] Numerical Recipes in C, 2nd edition
            William H. Press, et al.,
            Section 3.3, page 115
    */
    void Calculate();

    /** @descr this function corresponds to the algorithm 4.76 in [2] and
        theorem 5.3.7 in [3]

        [2] Engeln-Müllges, Gisela: Numerik-Algorithmen: Verfahren, Beispiele, Anwendungen
            Springer, Berlin; Auflage: 9., überarb. und erw. A. (8. Dezember 2004)
            Section 4.10.2, page 175

        [3] Hanrath, Wilhelm: Mathematik III / Numerik, Vorlesungsskript zur
            Veranstaltung im WS 2007/2008
            Fachhochschule Aachen, 2009-09-19
            Numerik_01.pdf, downloaded 2011-04-19 via
            http://www.fh-aachen.de/index.php?id=11424&no_cache=1&file=5016&uid=44191
            Section 5.3, page 129
    */
    void CalculatePeriodic();
};

lcl_SplineCalculation::lcl_SplineCalculation(
    tPointVecType && rSortedPoints,
    double fY1FirstDerivation,
    double fYnFirstDerivation )
        : m_aPoints( std::move(rSortedPoints) ),
          m_fYp1( fY1FirstDerivation ),
          m_fYpN( fYnFirstDerivation ),
          m_nKLow( 0 ),
          m_nKHigh( m_aPoints.size() - 1 ),
          m_fLastInterpolatedValue(std::numeric_limits<double>::infinity())
{
    Calculate();
}

lcl_SplineCalculation::lcl_SplineCalculation(
    tPointVecType && rSortedPoints)
        : m_aPoints( std::move(rSortedPoints) ),
          m_fYp1( 0.0 ),  /*dummy*/
          m_fYpN( 0.0 ),  /*dummy*/
          m_nKLow( 0 ),
          m_nKHigh( m_aPoints.size() - 1 ),
          m_fLastInterpolatedValue(std::numeric_limits<double>::infinity())
{
    CalculatePeriodic();
}

void lcl_SplineCalculation::Calculate()
{
    if( m_aPoints.size() <= 1 )
        return;

    // n is the last valid index to m_aPoints
    const lcl_tSizeType n = m_aPoints.size() - 1;
    std::vector< double > u( n );
    m_aSecDerivY.resize( n + 1, 0.0 );

    if( std::isinf( m_fYp1 ) )
    {
        // natural spline
        m_aSecDerivY[ 0 ] = 0.0;
        u[ 0 ] = 0.0;
    }
    else
    {
        m_aSecDerivY[ 0 ] = -0.5;
        double xDiff = m_aPoints[ 1 ].first - m_aPoints[ 0 ].first;
        u[ 0 ] = ( 3.0 / xDiff ) *
            ((( m_aPoints[ 1 ].second - m_aPoints[ 0 ].second ) / xDiff ) - m_fYp1 );
    }

    for( lcl_tSizeType i = 1; i < n; ++i )
    {
        tPointType
            p_i = m_aPoints[ i ],
            p_im1 = m_aPoints[ i - 1 ],
            p_ip1 = m_aPoints[ i + 1 ];

        double sig = ( p_i.first - p_im1.first ) /
            ( p_ip1.first - p_im1.first );
        double p = sig * m_aSecDerivY[ i - 1 ] + 2.0;

        m_aSecDerivY[ i ] = ( sig - 1.0 ) / p;
        u[ i ] =
            ( ( p_ip1.second - p_i.second ) /
              ( p_ip1.first - p_i.first ) ) -
            ( ( p_i.second - p_im1.second ) /
              ( p_i.first - p_im1.first ) );
        u[ i ] =
            ( 6.0 * u[ i ] / ( p_ip1.first - p_im1.first )
              - sig * u[ i - 1 ] ) / p;
    }

    // initialize to values for natural splines (used for m_fYpN equal to
    // infinity)
    double qn = 0.0;
    double un = 0.0;

    if( ! std::isinf( m_fYpN ) )
    {
        qn = 0.5;
        double xDiff = m_aPoints[ n ].first - m_aPoints[ n - 1 ].first;
        un = ( 3.0 / xDiff ) *
            ( m_fYpN - ( m_aPoints[ n ].second - m_aPoints[ n - 1 ].second ) / xDiff );
    }

    m_aSecDerivY[ n ] = ( un - qn * u[ n - 1 ] ) / ( qn * m_aSecDerivY[ n - 1 ] + 1.0 );

    // note: the algorithm in [1] iterates from n-1 to 0, but as size_type
    // may be (usually is) an unsigned type, we can not write k >= 0, as this
    // is always true.
    for( lcl_tSizeType k = n; k > 0; --k )
    {
        m_aSecDerivY[ k - 1 ] = (m_aSecDerivY[ k - 1 ] * m_aSecDerivY[ k ] ) + u[ k - 1 ];
    }
}

void lcl_SplineCalculation::CalculatePeriodic()
{
    if( m_aPoints.size() <= 1 )
        return;

    // n is the last valid index to m_aPoints
    const lcl_tSizeType n = m_aPoints.size() - 1;

    // u is used for vector f in A*c=f in [3], vector a in  Ax=a in [2],
    // vector z in Rtranspose z = a and Dr=z in [2]
    std::vector< double > u( n + 1, 0.0 );

    // used for vector c in A*c=f and vector x in Ax=a in [2]
    m_aSecDerivY.resize( n + 1, 0.0 );

    // diagonal of matrix A, used index 1 to n
    std::vector< double > Adiag( n + 1, 0.0 );

    // secondary diagonal of matrix A with index 1 to n-1 and upper right element in A[n]
    std::vector< double > Aupper( n + 1, 0.0 );

    // diagonal of matrix D in A=(R transpose)*D*R in [2], used index 1 to n
    std::vector< double > Ddiag( n+1, 0.0 );

    // right column of matrix R, used index 1 to n-2
    std::vector< double > Rright( n-1, 0.0 );

    // secondary diagonal of matrix R, used index 1 to n-1
    std::vector< double > Rupper( n, 0.0 );

    if (n<4)
    {
        if (n==3)
        {   // special handling of three polynomials, that are four points
            double xDiff0 = m_aPoints[ 1 ].first - m_aPoints[ 0 ].first ;
            double xDiff1 = m_aPoints[ 2 ].first - m_aPoints[ 1 ].first ;
            double xDiff2 = m_aPoints[ 3 ].first - m_aPoints[ 2 ].first ;
            double xDiff2p1 = xDiff2 + xDiff1;
            double xDiff0p2 = xDiff0 + xDiff2;
            double xDiff1p0 = xDiff1 + xDiff0;
            double fFactor = 1.5 / (xDiff0*xDiff1 + xDiff1*xDiff2 + xDiff2*xDiff0);
            double yDiff0 = (m_aPoints[ 1 ].second - m_aPoints[ 0 ].second) / xDiff0;
            double yDiff1 = (m_aPoints[ 2 ].second - m_aPoints[ 1 ].second) / xDiff1;
            double yDiff2 = (m_aPoints[ 0 ].second - m_aPoints[ 2 ].second) / xDiff2;
            m_aSecDerivY[ 1 ] = fFactor * (yDiff1*xDiff2p1 - yDiff0*xDiff0p2);
            m_aSecDerivY[ 2 ] = fFactor * (yDiff2*xDiff0p2 - yDiff1*xDiff1p0);
            m_aSecDerivY[ 3 ] = fFactor * (yDiff0*xDiff1p0 - yDiff2*xDiff2p1);
            m_aSecDerivY[ 0 ] = m_aSecDerivY[ 3 ];
        }
        else if (n==2)
        {
        // special handling of two polynomials, that are three points
            double xDiff0 = m_aPoints[ 1 ].first - m_aPoints[ 0 ].first;
            double xDiff1 = m_aPoints[ 2 ].first - m_aPoints[ 1 ].first;
            double fHelp = 3.0 * (m_aPoints[ 0 ].second - m_aPoints[ 1 ].second) / (xDiff0*xDiff1);
            m_aSecDerivY[ 1 ] = fHelp ;
            m_aSecDerivY[ 2 ] = -fHelp ;
            m_aSecDerivY[ 0 ] = m_aSecDerivY[ 2 ] ;
        }
        else
        {
            // should be handled with natural spline, periodic not possible.
        }
    }
    else
    {
        double xDiff_i =1.0; // values are dummy;
        double xDiff_im1 =1.0;
        double yDiff_i = 1.0;
        double yDiff_im1 = 1.0;
        // fill matrix A and fill right side vector u
        for( lcl_tSizeType i=1; i<n; ++i )
        {
            xDiff_im1 = m_aPoints[ i ].first - m_aPoints[ i-1 ].first;
            xDiff_i = m_aPoints[ i+1 ].first - m_aPoints[ i ].first;
            yDiff_im1 = (m_aPoints[ i ].second - m_aPoints[ i-1 ].second) / xDiff_im1;
            yDiff_i = (m_aPoints[ i+1 ].second - m_aPoints[ i ].second) / xDiff_i;
            Adiag[ i ] = 2 * (xDiff_im1 + xDiff_i);
            Aupper[ i ] = xDiff_i;
            u [ i ] = 3 * (yDiff_i - yDiff_im1);
        }
        xDiff_im1 = m_aPoints[ n ].first - m_aPoints[ n-1 ].first;
        xDiff_i = m_aPoints[ 1 ].first - m_aPoints[ 0 ].first;
        yDiff_im1 = (m_aPoints[ n ].second - m_aPoints[ n-1 ].second) / xDiff_im1;
        yDiff_i = (m_aPoints[ 1 ].second - m_aPoints[ 0 ].second) / xDiff_i;
        Adiag[ n ] = 2 * (xDiff_im1 + xDiff_i);
        Aupper[ n ] = xDiff_i;
        u [ n ] = 3 * (yDiff_i - yDiff_im1);

        // decomposite A=(R transpose)*D*R
        Ddiag[1] = Adiag[1];
        Rupper[1] = Aupper[1] / Ddiag[1];
        Rright[1] = Aupper[n] / Ddiag[1];
        for( lcl_tSizeType i=2; i<=n-2; ++i )
        {
            Ddiag[i] = Adiag[i] - Aupper[ i-1 ] * Rupper[ i-1 ];
            Rupper[ i ] = Aupper[ i ] / Ddiag[ i ];
            Rright[ i ] = - Rright[ i-1 ] * Aupper[ i-1 ] / Ddiag[ i ];
        }
        Ddiag[ n-1 ] = Adiag[ n-1 ] - Aupper[ n-2 ] * Rupper[ n-2 ];
        Rupper[ n-1 ] = ( Aupper[ n-1 ] - Aupper[ n-2 ] * Rright[ n-2] ) / Ddiag[ n-1 ];
        double fSum = 0.0;
        for ( lcl_tSizeType i=1; i<=n-2; ++i )
        {
            fSum += Ddiag[ i ] * Rright[ i ] * Rright[ i ];
        }
        Ddiag[ n ] = Adiag[ n ] - fSum - Ddiag[ n-1 ] * Rupper[ n-1 ] * Rupper[ n-1 ]; // bug in [2]!

        // solve forward (R transpose)*z=u, overwrite u with z
        for ( lcl_tSizeType i=2; i<=n-1; ++i )
        {
            u[ i ] -= u[ i-1 ]* Rupper[ i-1 ];
        }
        fSum = 0.0;
        for ( lcl_tSizeType i=1; i<=n-2; ++i )
        {
            fSum += Rright[ i ] * u[ i ];
        }
        u[ n ] = u[ n ] - fSum - Rupper[ n - 1] * u[ n-1 ];

        // solve forward D*r=z, z is in u, overwrite u with r
        for ( lcl_tSizeType i=1; i<=n; ++i )
        {
            u[ i ] = u[i] / Ddiag[ i ];
        }

        // solve backward R*x= r, r is in u
        m_aSecDerivY[ n ] = u[ n ];
        m_aSecDerivY[ n-1 ] = u[ n-1 ] - Rupper[ n-1 ] * m_aSecDerivY[ n ];
        for ( lcl_tSizeType i=n-2; i>=1; --i)
        {
            m_aSecDerivY[ i ] = u[ i ] - Rupper[ i ] * m_aSecDerivY[ i+1 ] - Rright[ i ] * m_aSecDerivY[ n ];
        }
        // periodic
        m_aSecDerivY[ 0 ] = m_aSecDerivY[ n ];
    }

    // adapt m_aSecDerivY for usage in GetInterpolatedValue()
    for( lcl_tSizeType i = 0; i <= n ; ++i )
    {
        m_aSecDerivY[ i ] *= 2.0;
    }

}

double lcl_SplineCalculation::GetInterpolatedValue( double x )
{
    OSL_PRECOND( ( m_aPoints[ 0 ].first <= x ) &&
                ( x <= m_aPoints[ m_aPoints.size() - 1 ].first ),
                "Trying to extrapolate" );

    const lcl_tSizeType n = m_aPoints.size() - 1;
    if( x < m_fLastInterpolatedValue )
    {
        m_nKLow = 0;
        m_nKHigh = n;

        // calculate m_nKLow and m_nKHigh
        // first initialization is done in CTOR
        while( m_nKHigh - m_nKLow > 1 )
        {
            lcl_tSizeType k = ( m_nKHigh + m_nKLow ) / 2;
            if( m_aPoints[ k ].first > x )
                m_nKHigh = k;
            else
                m_nKLow = k;
        }
    }
    else
    {
        while( ( m_nKHigh <= n ) &&
               ( m_aPoints[ m_nKHigh ].first < x ) )
        {
            ++m_nKHigh;
            ++m_nKLow;
        }
        OSL_ENSURE( m_nKHigh <= n, "Out of Bounds" );
    }
    m_fLastInterpolatedValue = x;

    double h = m_aPoints[ m_nKHigh ].first - m_aPoints[ m_nKLow ].first;
    OSL_ENSURE( h != 0, "Bad input to GetInterpolatedValue()" );

    double a = ( m_aPoints[ m_nKHigh ].first - x ) / h;
    double b = ( x - m_aPoints[ m_nKLow ].first  ) / h;

    return ( a * m_aPoints[ m_nKLow ].second +
             b * m_aPoints[ m_nKHigh ].second +
             (( a*a*a - a ) * m_aSecDerivY[ m_nKLow ] +
              ( b*b*b - b ) * m_aSecDerivY[ m_nKHigh ] ) *
             ( h*h ) / 6.0 );
}

// helper methods for B-spline

// Create parameter t_0 to t_n using the centripetal method with a power of 0.5
bool createParameterT(const tPointVecType& rUniquePoints, double* t)
{   // precondition: no adjacent identical points
    // postcondition: 0 = t_0 < t_1 < ... < t_n = 1
    bool bIsSuccessful = true;
    const lcl_tSizeType n = rUniquePoints.size() - 1;
    t[0]=0.0;
    double fDenominator = 0.0; // initialized for summing up
    for (lcl_tSizeType i=1; i<=n ; ++i)
    {   // 4th root(dx^2+dy^2)
        double dx = rUniquePoints[i].first - rUniquePoints[i-1].first;
        double dy = rUniquePoints[i].second - rUniquePoints[i-1].second;
        if (dx == 0 && dy == 0)
        {
            bIsSuccessful = false;
            break;
        }
        else
        {
            fDenominator += sqrt(std::hypot(dx, dy));
        }
    }
    if (fDenominator == 0.0)
    {
        bIsSuccessful = false;
    }
    if (bIsSuccessful)
    {
        for (lcl_tSizeType j=1; j<=n ; ++j)
        {
            double fNumerator = 0.0;
            for (lcl_tSizeType i=1; i<=j ; ++i)
            {
                double dx = rUniquePoints[i].first - rUniquePoints[i-1].first;
                double dy = rUniquePoints[i].second - rUniquePoints[i-1].second;
                fNumerator += sqrt(std::hypot(dx, dy));
            }
            t[j] = fNumerator / fDenominator;

        }
        // postcondition check
        t[n] = 1.0;
        double fPrevious = 0.0;
        for (lcl_tSizeType i=1; i <= n && bIsSuccessful ; ++i)
        {
            if (fPrevious >= t[i])
            {
                bIsSuccessful = false;
            }
            else
            {
                fPrevious = t[i];
            }
        }
    }
    return bIsSuccessful;
}

void createKnotVector(const lcl_tSizeType n, const sal_uInt32 p, const double* t, double* u)
{  // precondition: 0 = t_0 < t_1 < ... < t_n = 1
        for (lcl_tSizeType j = 0; j <= p; ++j)
        {
            u[j] = 0.0;
        }
        for (lcl_tSizeType j = 1; j <= n-p; ++j )
        {
            double fSum = 0.0;
            for (lcl_tSizeType i = j; i <= j+p-1; ++i)
            {
                fSum += t[i];
            }
            assert(p != 0);
            u[j+p] = fSum / p ;
        }
        for (lcl_tSizeType j = n+1; j <= n+1+p; ++j)
        {
            u[j] = 1.0;
        }
}

void applyNtoParameterT(const lcl_tSizeType i,const double tk,const sal_uInt32 p,const double* u, double* rowN)
{
    // get N_p(t_k) recursively, only N_(i-p) till N_(i) are relevant, all other N_# are zero

    // initialize with indicator function degree 0
    rowN[p] = 1.0; // all others are zero

    // calculate up to degree p
    for (sal_uInt32 s = 1; s <= p; ++s)
    {
        // first element
        double fLeftFactor = 0.0;
        double fRightFactor = ( u[i+1] - tk ) / ( u[i+1]- u[i-s+1] );
        // i-s "true index" - (i-p)"shift" = p-s
        rowN[p-s] = fRightFactor * rowN[p-s+1];

        // middle elements
        for (sal_uInt32 j = s-1; j>=1 ; --j)
        {
            fLeftFactor = ( tk - u[i-j] ) / ( u[i-j+s] - u[i-j] ) ;
            fRightFactor = ( u[i-j+s+1] - tk ) / ( u[i-j+s+1] - u[i-j+1] );
            // i-j "true index" - (i-p)"shift" = p-j
            rowN[p-j] = fLeftFactor * rowN[p-j] + fRightFactor *  rowN[p-j+1];
        }

        // last element
        fLeftFactor = ( tk - u[i] ) / ( u[i+s] - u[i] );
        // i "true index" - (i-p)"shift" = p
        rowN[p] = fLeftFactor * rowN[p];
    }
}

} //  anonymous namespace

// Calculates uniform parametric splines with subinterval length 1,
// according ODF1.2 part 1, chapter 'chart interpolation'.
void SplineCalculater::CalculateCubicSplines(
    const std::vector<std::vector<css::drawing::Position3D>>& rInput
    , std::vector<std::vector<css::drawing::Position3D>>& rResult
    , sal_uInt32 nGranularity )
{
    OSL_PRECOND( nGranularity > 0, "Granularity is invalid" );

    sal_uInt32 nOuterCount = rInput.size();

    rResult.resize(nOuterCount);

    auto pSequence = rResult.data();

    if( !nOuterCount )
        return;

    for( sal_uInt32 nOuter = 0; nOuter < nOuterCount; ++nOuter )
    {
        if( rInput[nOuter].size() <= 1 )
            continue; //we need at least two points

        sal_uInt32 nMaxIndexPoints = rInput[nOuter].size()-1; // is >=1
        const css::drawing::Position3D* pOld = rInput[nOuter].data();

        std::vector < double > aParameter(nMaxIndexPoints+1);
        aParameter[0]=0.0;
        for( sal_uInt32 nIndex=1; nIndex<=nMaxIndexPoints; nIndex++ )
        {
            aParameter[nIndex]=aParameter[nIndex-1]+1;
        }

        // Split the calculation to X, Y and Z coordinate
        tPointVecType aInputX;
        aInputX.resize(nMaxIndexPoints+1);
        tPointVecType aInputY;
        aInputY.resize(nMaxIndexPoints+1);
        tPointVecType aInputZ;
        aInputZ.resize(nMaxIndexPoints+1);
        for (sal_uInt32 nN=0;nN<=nMaxIndexPoints; nN++ )
        {
          aInputX[ nN ].first=aParameter[nN];
          aInputX[ nN ].second=pOld[ nN ].PositionX;
          aInputY[ nN ].first=aParameter[nN];
          aInputY[ nN ].second=pOld[ nN ].PositionY;
          aInputZ[ nN ].first=aParameter[nN];
          aInputZ[ nN ].second=pOld[ nN ].PositionZ;
        }

        // generate a spline for each coordinate. It holds the complete
        // information to calculate each point of the curve
        std::optional<lcl_SplineCalculation> aSplineX;
        std::optional<lcl_SplineCalculation> aSplineY;
        // lcl_SplineCalculation* aSplineZ; the z-coordinates of all points in
        // a data series are equal. No spline calculation needed, but copy
        // coordinate to output

        if( pOld[ 0 ].PositionX == pOld[nMaxIndexPoints].PositionX &&
            pOld[ 0 ].PositionY == pOld[nMaxIndexPoints].PositionY &&
            pOld[ 0 ].PositionZ == pOld[nMaxIndexPoints].PositionZ &&
            nMaxIndexPoints >=2 )
        {   // periodic spline
            aSplineX.emplace(std::move(aInputX));
            aSplineY.emplace(std::move(aInputY));
        }
        else // generate the kind "natural spline"
        {
            double fXDerivation = std::numeric_limits<double>::infinity();
            double fYDerivation = std::numeric_limits<double>::infinity();
            aSplineX.emplace(std::move(aInputX), fXDerivation, fXDerivation);
            aSplineY.emplace(std::move(aInputY), fYDerivation, fYDerivation);
        }

        // fill result polygon with calculated values
        pSequence[nOuter].resize( nMaxIndexPoints*nGranularity + 1);

        css::drawing::Position3D* pNew = pSequence[nOuter].data();

        sal_uInt32 nNewPointIndex = 0; // Index in result points

        for( sal_uInt32 ni = 0; ni < nMaxIndexPoints; ni++ )
        {
            // given point is surely a curve point
            pNew[nNewPointIndex].PositionX = pOld[ni].PositionX;
            pNew[nNewPointIndex].PositionY = pOld[ni].PositionY;
            pNew[nNewPointIndex].PositionZ = pOld[ni].PositionZ;
            nNewPointIndex++;

            // calculate intermediate points
            double fInc = ( aParameter[ ni+1 ] - aParameter[ni] ) / static_cast< double >( nGranularity );
            for(sal_uInt32 nj = 1; nj < nGranularity; nj++)
            {
                double fParam = aParameter[ni] + ( fInc * static_cast< double >( nj ) );

                pNew[nNewPointIndex].PositionX = aSplineX->GetInterpolatedValue( fParam );
                pNew[nNewPointIndex].PositionY = aSplineY->GetInterpolatedValue( fParam );
                // pNewZ[nNewPointIndex]=aSplineZ->GetInterpolatedValue( fParam );
                pNew[nNewPointIndex].PositionZ = pOld[ni].PositionZ;
                nNewPointIndex++;
            }
        }
        // add last point
        pNew[nNewPointIndex] = pOld[nMaxIndexPoints];
    }
}

void SplineCalculater::CalculateBSplines(
            const std::vector<std::vector<css::drawing::Position3D>>& rInput
            , std::vector<std::vector<css::drawing::Position3D>>& rResult
            , sal_uInt32 nResolution
            , sal_uInt32 nDegree )
{
    // nResolution is ODF1.2 file format attribute chart:spline-resolution and
    // ODF1.2 spec variable k. Causion, k is used as index in the spec in addition.
    // nDegree is ODF1.2 file format attribute chart:spline-order and
    // ODF1.2 spec variable p
    OSL_ASSERT( nResolution > 1 );
    OSL_ASSERT( nDegree >= 1 );

    // limit the b-spline degree at 15 to prevent insanely large sets of points
    sal_uInt32 p = std::min<sal_uInt32>(nDegree, 15);

    sal_Int32 nOuterCount = rInput.size();

    rResult.resize(nOuterCount);

    auto pSequence = rResult.data();

    if( !nOuterCount )
        return; // no input

    for( sal_Int32 nOuter = 0; nOuter < nOuterCount; ++nOuter )
    {
        if( rInput[nOuter].size() <= 1 )
            continue; // need at least 2 points, next piece of the series

        // Copy input to vector of points and remove adjacent double points. The
        // Z-coordinate is equal for all points in a series and holds the depth
        // in 3D mode, simple copying is enough.
        lcl_tSizeType nMaxIndexPoints = rInput[nOuter].size()-1; // is >=1
        const css::drawing::Position3D* pOld = rInput[nOuter].data();
        double fZCoordinate = pOld[0].PositionZ;
        tPointVecType aPointsIn;
        aPointsIn.resize(nMaxIndexPoints+1);
        for (lcl_tSizeType i = 0; i <= nMaxIndexPoints; ++i )
        {
          aPointsIn[ i ].first = pOld[i].PositionX;
          aPointsIn[ i ].second = pOld[i].PositionY;
        }
        aPointsIn.erase( std::unique( aPointsIn.begin(), aPointsIn.end()),
                     aPointsIn.end() );

        // n is the last valid index to the reduced aPointsIn
        // There are n+1 valid data points.
        const lcl_tSizeType n = aPointsIn.size() - 1;
        if (n < 1 || p > n)
            continue; // need at least 2 points, degree p needs at least n+1 points
                      // next piece of series

        std::vector<double> t(n + 1);
        if (!createParameterT(aPointsIn, t.data()))
        {
            continue; // next piece of series
        }

        lcl_tSizeType m = n + p + 1;
        std::vector<double> u(m + 1);
        createKnotVector(n, p, t.data(), u.data());

        // The matrix N contains the B-spline basis functions applied to parameters.
        // In each row only p+1 adjacent elements are non-zero. The starting
        // column in a higher row is equal or greater than in the lower row.
        // To store this matrix the non-zero elements are shifted to column 0
        // and the amount of shifting is remembered in an array.
        std::vector<std::vector<double>> aMatN(n + 1, std::vector<double>(p + 1));
        std::vector<lcl_tSizeType> aShift(n + 1);
        aMatN[0][0] = 1.0; //all others are zero
        aShift[0] = 0;
        aMatN[n][0] = 1.0;
        aShift[n] = n;
        for (lcl_tSizeType k = 1; k<=n-1; ++k)
        { // all basis functions are applied to t_k,
            // results are elements in row k in matrix N

            // find the one interval with u_i <= t_k < u_(i+1)
            // remember u_0 = ... = u_p = 0.0 and u_(m-p) = ... u_m = 1.0 and 0<t_k<1
            lcl_tSizeType i = p;
            while (u[i] > t[k] || t[k] >= u[i+1])
            {
                ++i;
            }

            // index in reduced matrix aMatN = (index in full matrix N) - (i-p)
            aShift[k] = i - p;

            applyNtoParameterT(i, t[k], p, u.data(), aMatN[k].data());
        } // next row k

        // Get matrix C of control points from the matrix equation aMatN * C = aPointsIn
        // aPointsIn is overwritten with C.
        // Gaussian elimination is possible without pivoting, see reference
        lcl_tSizeType r = 0; // true row index
        lcl_tSizeType c = 0; // true column index
        double fDivisor = 1.0; // used for diagonal element
        double fEliminate = 1.0; // used for the element, that will become zero
        bool bIsSuccessful = true;
        for (c = 0 ; c <= n && bIsSuccessful; ++c)
        {
            // search for first non-zero downwards
            r = c;
            while ( r < n && aMatN[r][c-aShift[r]] == 0 )
            {
                ++r;
            }
            if (aMatN[r][c-aShift[r]] == 0.0)
            {
                // Matrix N is singular, although this is mathematically impossible
                bIsSuccessful = false;
            }
            else
            {
                // exchange total row r with total row c if necessary
                if (r != c)
                {
                    std::swap( aMatN[r], aMatN[c] );
                    std::swap( aPointsIn[r], aPointsIn[c] );
                    std::swap( aShift[r], aShift[c] );
                }

                // divide row c, so that element(c,c) becomes 1
                fDivisor = aMatN[c][c-aShift[c]]; // not zero, see above
                for (sal_uInt32 i = 0; i <= p; ++i)
                {
                    aMatN[c][i] /= fDivisor;
                }
                aPointsIn[c].first /= fDivisor;
                aPointsIn[c].second /= fDivisor;

                // eliminate forward, examine row c+1 to n-1 (worst case)
                // stop if first non-zero element in row has an higher column as c
                // look at nShift for that, elements in nShift are equal or increasing
                for ( r = c+1; r < n && aShift[r]<=c ; ++r)
                {
                    fEliminate = aMatN[r][0];
                    if (fEliminate != 0.0) // else accidentally zero, nothing to do
                    {
                        for (sal_uInt32 i = 1; i <= p; ++i)
                        {
                            aMatN[r][i-1] = aMatN[r][i] - fEliminate * aMatN[c][i];
                        }
                        aMatN[r][p]=0;
                        aPointsIn[r].first -= fEliminate * aPointsIn[c].first;
                        aPointsIn[r].second -= fEliminate * aPointsIn[c].second;
                        ++aShift[r];
                    }
                }
            }
        }// upper triangle form is reached
        if( bIsSuccessful)
        {
            // eliminate backwards, begin with last column
            for (lcl_tSizeType cc = n; cc >= 1; --cc )
            {
                // In row cc the diagonal element(cc,cc) == 1 and all elements left from
                // diagonal are zero and do not influence other rows.
                // Full matrix N has semibandwidth < p, therefore element(r,c) is
                // zero, if abs(r-cc)>=p.  abs(r-cc)=cc-r, because r<cc.
                r = cc - 1;
                while ( r !=0 && cc-r < p )
                {
                    fEliminate = aMatN[r][ cc - aShift[r] ];
                    if ( fEliminate != 0.0) // else element is accidentally zero, no action needed
                    {
                        // row r -= fEliminate * row cc only relevant for right side
                        aMatN[r][cc - aShift[r]] = 0.0;
                        aPointsIn[r].first -= fEliminate * aPointsIn[cc].first;
                        aPointsIn[r].second -= fEliminate * aPointsIn[cc].second;
                    }
                    --r;
                }
            }
            // aPointsIn contains the control points now.

            // calculate the intermediate points according given resolution
            // using deBoor-Cox algorithm
            lcl_tSizeType nNewSize = nResolution * n + 1;
            pSequence[nOuter].resize(nNewSize);
            css::drawing::Position3D* pNew = pSequence[nOuter].data();
            pNew[0].PositionX = aPointsIn[0].first;
            pNew[0].PositionY = aPointsIn[0].second;
            pNew[0].PositionZ = fZCoordinate; // Precondition: z-coordinates of all points of a series are equal
            pNew[nNewSize -1 ].PositionX = aPointsIn[n].first;
            pNew[nNewSize -1 ].PositionY = aPointsIn[n].second;
            pNew[nNewSize -1 ].PositionZ = fZCoordinate;
            std::vector<double> aP(m + 1);
            lcl_tSizeType nLow = 0;
            for ( lcl_tSizeType nTIndex = 0; nTIndex <= n-1; ++nTIndex)
            {
                for (sal_uInt32 nResolutionStep = 1;
                     nResolutionStep <= nResolution && ( nTIndex != n-1 || nResolutionStep != nResolution);
                     ++nResolutionStep)
                {
                    lcl_tSizeType nNewIndex = nTIndex * nResolution + nResolutionStep;
                    double ux = t[nTIndex] + nResolutionStep * ( t[nTIndex+1] - t[nTIndex]) /nResolution;

                    // get index nLow, so that u[nLow]<= ux < u[nLow +1]
                    // continue from previous nLow
                    while ( u[nLow] <= ux)
                    {
                        ++nLow;
                    }
                    --nLow;

                    // x-coordinate
                    for (lcl_tSizeType i = nLow-p; i <= nLow; ++i)
                    {
                        aP[i] = aPointsIn[i].first;
                    }
                    for (sal_uInt32 lcl_Degree = 1; lcl_Degree <= p; ++lcl_Degree)
                    {
                        for (lcl_tSizeType i = nLow; i >= nLow + lcl_Degree - p; --i)
                        {
                            double fFactor = ( ux - u[i] ) / ( u[i+p+1-lcl_Degree] - u[i]);
                            aP[i] = (1 - fFactor)* aP[i-1] + fFactor * aP[i];
                        }
                    }
                    pNew[nNewIndex].PositionX = aP[nLow];

                    // y-coordinate
                    for (lcl_tSizeType i = nLow - p; i <= nLow; ++i)
                    {
                        aP[i] = aPointsIn[i].second;
                    }
                    for (sal_uInt32 lcl_Degree = 1; lcl_Degree <= p; ++lcl_Degree)
                    {
                        for (lcl_tSizeType i = nLow; i >= nLow +lcl_Degree - p; --i)
                        {
                            double fFactor = ( ux - u[i] ) / ( u[i+p+1-lcl_Degree] - u[i]);
                            aP[i] = (1 - fFactor)* aP[i-1] + fFactor * aP[i];
                        }
                    }
                    pNew[nNewIndex].PositionY = aP[nLow];
                    pNew[nNewIndex].PositionZ = fZCoordinate;
                }
            }
        }
    } // next piece of the series
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
