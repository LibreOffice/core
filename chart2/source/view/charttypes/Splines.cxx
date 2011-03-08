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


// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"

#include "Splines.hxx"
#include <rtl/math.hxx>

#include <vector>
#include <algorithm>
#include <functional>

// header for DBG_ASSERT
#include <tools/debug.hxx>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;

namespace
{

template< typename T >
struct lcl_EqualsFirstDoubleOfPair : ::std::binary_function< ::std::pair< double, T >, ::std::pair< double, T >, bool >
{
    inline bool operator() ( const ::std::pair< double, T > & rOne, const ::std::pair< double, T > & rOther )
    {
        return ( ::rtl::math::approxEqual( rOne.first, rOther.first ) );
    }
};

//-----------------------------------------------------------------------------

typedef ::std::pair< double, double >   tPointType;
typedef ::std::vector< tPointType >     tPointVecType;
typedef tPointVecType::size_type        lcl_tSizeType;

class lcl_SplineCalculation
{
public:
    /** @descr creates an object that calculates cublic splines on construction

        @param rSortedPoints  the points for which splines shall be calculated, they need to be sorted in x values
        @param fY1FirstDerivation the resulting spline should have the first
               derivation equal to this value at the x-value of the first point
               of rSortedPoints.  If fY1FirstDerivation is set to infinity, a natural
               spline is calculated.
        @param fYnFirstDerivation the resulting spline should have the first
               derivation equal to this value at the x-value of the last point
               of rSortedPoints
     */
    lcl_SplineCalculation( const tPointVecType & rSortedPoints,
                           double fY1FirstDerivation,
                           double fYnFirstDerivation );

    /** @descr this function corresponds to the function splint in [1].

        [1] Numerical Recipies in C, 2nd edition
            William H. Press, et al.,
            Section 3.3, page 116
    */
    double GetInterpolatedValue( double x );

private:
    /// a copy of the points given in the CTOR
    tPointVecType            m_aPoints;

    /// the result of the Calculate() method
    ::std::vector< double >         m_aSecDerivY;

    double m_fYp1;
    double m_fYpN;

    // these values are cached for performance reasons
    tPointVecType::size_type m_nKLow;
    tPointVecType::size_type m_nKHigh;
    double m_fLastInterpolatedValue;

    /** @descr this function corresponds to the function spline in [1].

        [1] Numerical Recipies in C, 2nd edition
            William H. Press, et al.,
            Section 3.3, page 115
    */
    void Calculate();
};

//-----------------------------------------------------------------------------

lcl_SplineCalculation::lcl_SplineCalculation(
    const tPointVecType & rSortedPoints,
    double fY1FirstDerivation,
    double fYnFirstDerivation )
        : m_aPoints( rSortedPoints ),
          m_fYp1( fY1FirstDerivation ),
          m_fYpN( fYnFirstDerivation ),
          m_nKLow( 0 ),
          m_nKHigh( rSortedPoints.size() - 1 )
{
    ::rtl::math::setInf( &m_fLastInterpolatedValue, sal_False );

    // remove points that have equal x-values
    m_aPoints.erase( ::std::unique( m_aPoints.begin(), m_aPoints.end(),
                             lcl_EqualsFirstDoubleOfPair< double >() ),
                     m_aPoints.end() );
    Calculate();
}

void lcl_SplineCalculation::Calculate()
{
    if( m_aPoints.size() <= 1 )
        return;

    // n is the last valid index to m_aPoints
    const tPointVecType::size_type n = m_aPoints.size() - 1;
    ::std::vector< double > u( n );
    m_aSecDerivY.resize( n + 1, 0.0 );

    if( ::rtl::math::isInf( m_fYp1 ) )
    {
        // natural spline
        m_aSecDerivY[ 0 ] = 0.0;
        u[ 0 ] = 0.0;
    }
    else
    {
        m_aSecDerivY[ 0 ] = -0.5;
        double xDiff = ( m_aPoints[ 1 ].first - m_aPoints[ 0 ].first );
        u[ 0 ] = ( 3.0 / xDiff ) *
            ((( m_aPoints[ 1 ].second - m_aPoints[ 0 ].second ) / xDiff ) - m_fYp1 );
    }

    for( tPointVecType::size_type i = 1; i < n; ++i )
    {
        ::std::pair< double, double >
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

    if( ! ::rtl::math::isInf( m_fYpN ) )
    {
        qn = 0.5;
        double xDiff = ( m_aPoints[ n ].first - m_aPoints[ n - 1 ].first );
        un = ( 3.0 / xDiff ) *
            ( m_fYpN - ( m_aPoints[ n ].second - m_aPoints[ n - 1 ].second ) / xDiff );
    }

    m_aSecDerivY[ n ] = ( un - qn * u[ n - 1 ] ) * ( qn * m_aSecDerivY[ n - 1 ] + 1.0 );

    // note: the algorithm in [1] iterates from n-1 to 0, but as size_type
    // may be (usuall is) an unsigned type, we can not write k >= 0, as this
    // is always true.
    for( tPointVecType::size_type k = n; k > 0; --k )
    {
        ( m_aSecDerivY[ k - 1 ] *= m_aSecDerivY[ k ] ) += u[ k - 1 ];
    }
}

double lcl_SplineCalculation::GetInterpolatedValue( double x )
{
    DBG_ASSERT( ( m_aPoints[ 0 ].first <= x ) &&
                ( x <= m_aPoints[ m_aPoints.size() - 1 ].first ),
                "Trying to extrapolate" );

    const tPointVecType::size_type n = m_aPoints.size() - 1;
    if( x < m_fLastInterpolatedValue )
    {
        m_nKLow = 0;
        m_nKHigh = n;

        // calculate m_nKLow and m_nKHigh
        // first initialization is done in CTOR
        while( m_nKHigh - m_nKLow > 1 )
        {
            tPointVecType::size_type k = ( m_nKHigh + m_nKLow ) / 2;
            if( m_aPoints[ k ].first > x )
                m_nKHigh = k;
            else
                m_nKLow = k;
        }
    }
    else
    {
        while( ( m_aPoints[ m_nKHigh ].first < x ) &&
               ( m_nKHigh <= n ) )
        {
            ++m_nKHigh;
            ++m_nKLow;
        }
        DBG_ASSERT( m_nKHigh <= n, "Out of Bounds" );
    }
    m_fLastInterpolatedValue = x;

    double h = m_aPoints[ m_nKHigh ].first - m_aPoints[ m_nKLow ].first;
    DBG_ASSERT( h != 0, "Bad input to GetInterpolatedValue()" );

    double a = ( m_aPoints[ m_nKHigh ].first - x ) / h;
    double b = ( x - m_aPoints[ m_nKLow ].first  ) / h;

    return ( a * m_aPoints[ m_nKLow ].second +
             b * m_aPoints[ m_nKHigh ].second +
             (( a*a*a - a ) * m_aSecDerivY[ m_nKLow ] +
              ( b*b*b - b ) * m_aSecDerivY[ m_nKHigh ] ) *
             ( h*h ) / 6.0 );
}

//-----------------------------------------------------------------------------

//create knot vector for B-spline
double* createTVector( sal_Int32 n, sal_Int32 k )
{
    double* t = new double [n + k + 1];
    for (sal_Int32 i=0; i<=n+k; i++ )
    {
        if(i < k)
            t[i] = 0;
        else if(i <= n)
            t[i] = i-k+1;
        else
            t[i] = n-k+2;
    }
    return t;
}

//calculate left knot vector
double TLeft (double x, sal_Int32 i, sal_Int32 k, const double *t )
{
    double deltaT = t[i + k - 1] - t[i];
    return (deltaT == 0.0)
               ? 0.0
               : (x - t[i]) / deltaT;
}

//calculate right knot vector
double TRight(double x, sal_Int32 i, sal_Int32 k, const double *t )
{
    double deltaT = t[i + k] - t[i + 1];
    return (deltaT == 0.0)
               ? 0.0
               : (t[i + k] - x) / deltaT;
}

//calculate weight vector
void BVector(double x, sal_Int32 n, sal_Int32 k, double *b, const double *t)
{
    sal_Int32 i = 0;
    for( i=0; i<=n+k; i++ )
        b[i]=0;

    sal_Int32 i0 = (sal_Int32)floor(x) + k - 1;
    b [i0] = 1;

    for( sal_Int32 j=2; j<=k; j++ )
        for( i=0; i<=i0; i++ )
            b[i] = TLeft(x, i, j, t) * b[i] + TRight(x, i, j, t) * b [i + 1];
}

} //  anonymous namespace

//-----------------------------------------------------------------------------

void SplineCalculater::CalculateCubicSplines(
    const drawing::PolyPolygonShape3D& rInput
    , drawing::PolyPolygonShape3D& rResult
    , sal_Int32 nGranularity )
{
    DBG_ASSERT( nGranularity > 0, "Granularity is invalid" );

    rResult.SequenceX.realloc(0);
    rResult.SequenceY.realloc(0);
    rResult.SequenceZ.realloc(0);

    sal_Int32 nOuterCount = rInput.SequenceX.getLength();
    if( !nOuterCount )
        return;

    rResult.SequenceX.realloc(nOuterCount);
    rResult.SequenceY.realloc(nOuterCount);
    rResult.SequenceZ.realloc(nOuterCount);

    for( sal_Int32 nOuter = 0; nOuter < nOuterCount; ++nOuter )
    {
        if( rInput.SequenceX[nOuter].getLength() <= 1 )
            continue; //we need at least two points

        sal_Int32 nMaxIndexPoints = rInput.SequenceX[nOuter].getLength()-1; // is >=1
        const double* pOldX = rInput.SequenceX[nOuter].getConstArray();
        const double* pOldY = rInput.SequenceY[nOuter].getConstArray();
        const double* pOldZ = rInput.SequenceZ[nOuter].getConstArray();

        // #i13699# The curve gets a parameter and then for each coordinate a
        // separate spline will be calculated using the parameter as first argument
        // and the point coordinate as second argument. Therefore the points need
        // not to be sorted in its x-coordinates. The parameter is sorted by
        // construction.

        ::std::vector < double > aParameter(nMaxIndexPoints+1);
        aParameter[0]=0.0;
        for( sal_Int32 nIndex=1; nIndex<=nMaxIndexPoints; nIndex++ )
        {
            // The euclidian distance leads to curve loops for functions having single extreme points
            // use increment of 1 instead
            aParameter[nIndex]=aParameter[nIndex-1]+1;
        }
        // Split the calculation to X, Y and Z coordinate
        tPointVecType aInputX;
        aInputX.resize(nMaxIndexPoints+1);
        tPointVecType aInputY;
        aInputY.resize(nMaxIndexPoints+1);
        tPointVecType aInputZ;
        aInputZ.resize(nMaxIndexPoints+1);
        for (sal_Int32 nN=0;nN<=nMaxIndexPoints; nN++ )
        {
          aInputX[ nN ].first=aParameter[nN];
          aInputX[ nN ].second=pOldX[ nN ];
          aInputY[ nN ].first=aParameter[nN];
          aInputY[ nN ].second=pOldY[ nN ];
          aInputZ[ nN ].first=aParameter[nN];
          aInputZ[ nN ].second=pOldZ[ nN ];
        }

        // generate a spline for each coordinate. It holds the complete
        // information to calculate each point of the curve
        double fXDerivation;
        double fYDerivation;
        double fZDerivation;
        if( pOldX[ 0 ] == pOldX[nMaxIndexPoints] &&
            pOldY[ 0 ] == pOldY[nMaxIndexPoints] &&
            pOldZ[ 0 ] == pOldZ[nMaxIndexPoints] )
        {
            // #i101050# avoid a corner in closed lines, which are smoothed by spline
            // This derivation are special for parameter of kind 0,1,2,3... If you
            // change generating parameters (see above), then adapt derivations too.)
            fXDerivation = 0.5 * (pOldX[1]-pOldX[nMaxIndexPoints-1]);
            fYDerivation = 0.5 * (pOldY[1]-pOldY[nMaxIndexPoints-1]);
            fZDerivation = 0.5 * (pOldZ[1]-pOldZ[nMaxIndexPoints-1]);
        }
        else // generate the kind "natural spline"
        {
            double fInfty;
            ::rtl::math::setInf( &fInfty, sal_False );
            fXDerivation = fInfty;
            fYDerivation = fInfty;
            fZDerivation = fInfty;
        }
        lcl_SplineCalculation aSplineX( aInputX, fXDerivation, fXDerivation );
        lcl_SplineCalculation aSplineY( aInputY, fYDerivation, fYDerivation );
        lcl_SplineCalculation aSplineZ( aInputZ, fZDerivation, fZDerivation );

        // fill result polygon with calculated values
        rResult.SequenceX[nOuter].realloc( nMaxIndexPoints*nGranularity + 1);
        rResult.SequenceY[nOuter].realloc( nMaxIndexPoints*nGranularity + 1);
        rResult.SequenceZ[nOuter].realloc( nMaxIndexPoints*nGranularity + 1);

        double* pNewX = rResult.SequenceX[nOuter].getArray();
        double* pNewY = rResult.SequenceY[nOuter].getArray();
        double* pNewZ = rResult.SequenceZ[nOuter].getArray();

        sal_Int32 nNewPointIndex = 0; // Index in result points
        // needed for inner loop
        double    fInc;   // step for intermediate points
        sal_Int32 nj;     // for loop
        double    fParam; // a intermediate parameter value

        for( sal_Int32 ni = 0; ni < nMaxIndexPoints; ni++ )
        {
            // given point is surely a curve point
            pNewX[nNewPointIndex] = pOldX[ni];
            pNewY[nNewPointIndex] = pOldY[ni];
            pNewZ[nNewPointIndex] = pOldZ[ni];
            nNewPointIndex++;

            // calculate intermediate points
            fInc = ( aParameter[ ni+1 ] - aParameter[ni] ) / static_cast< double >( nGranularity );
            for(nj = 1; nj < nGranularity; nj++)
            {
                fParam = aParameter[ni] + ( fInc * static_cast< double >( nj ) );

                pNewX[nNewPointIndex]=aSplineX.GetInterpolatedValue( fParam );
                pNewY[nNewPointIndex]=aSplineY.GetInterpolatedValue( fParam );
                pNewZ[nNewPointIndex]=aSplineZ.GetInterpolatedValue( fParam );
                nNewPointIndex++;
            }
        }
        // add last point
        pNewX[nNewPointIndex] = pOldX[nMaxIndexPoints];
        pNewY[nNewPointIndex] = pOldY[nMaxIndexPoints];
        pNewZ[nNewPointIndex] = pOldZ[nMaxIndexPoints];
    }
}

void SplineCalculater::CalculateBSplines(
            const ::com::sun::star::drawing::PolyPolygonShape3D& rInput
            , ::com::sun::star::drawing::PolyPolygonShape3D& rResult
            , sal_Int32 nGranularity
            , sal_Int32 nDegree )
{
    // #issue 72216#
    // k is the order of the BSpline, nDegree is the degree of its polynoms
    sal_Int32 k = nDegree + 1;

    rResult.SequenceX.realloc(0);
    rResult.SequenceY.realloc(0);
    rResult.SequenceZ.realloc(0);

    sal_Int32 nOuterCount = rInput.SequenceX.getLength();
    if( !nOuterCount )
        return; // no input

    rResult.SequenceX.realloc(nOuterCount);
    rResult.SequenceY.realloc(nOuterCount);
    rResult.SequenceZ.realloc(nOuterCount);

    for( sal_Int32 nOuter = 0; nOuter < nOuterCount; ++nOuter )
    {
        if( rInput.SequenceX[nOuter].getLength() <= 1 )
            continue; // need at least 2 control points

        sal_Int32 n = rInput.SequenceX[nOuter].getLength()-1; // maximum index of control points

        double fCurveparam =0.0; // parameter for the curve
        // 0<= fCurveparam < fMaxCurveparam
        double fMaxCurveparam = 2.0+ n - k;
        if (fMaxCurveparam <= 0.0)
            return; // not enough control points for desired spline order

        if (nGranularity < 1)
            return; //need at least 1 line for each part beween the control points

        const double* pOldX = rInput.SequenceX[nOuter].getConstArray();
        const double* pOldY = rInput.SequenceY[nOuter].getConstArray();
        const double* pOldZ = rInput.SequenceZ[nOuter].getConstArray();

        // keep this amount of steps to go well with old version
        sal_Int32 nNewSectorCount = nGranularity * n;
        double fCurveStep = fMaxCurveparam/static_cast< double >(nNewSectorCount);

        double *b       = new double [n + k + 1]; // values of blending functions

        const double* t = createTVector(n, k); // knot vector

        rResult.SequenceX[nOuter].realloc(nNewSectorCount+1);
        rResult.SequenceY[nOuter].realloc(nNewSectorCount+1);
        rResult.SequenceZ[nOuter].realloc(nNewSectorCount+1);
        double* pNewX = rResult.SequenceX[nOuter].getArray();
        double* pNewY = rResult.SequenceY[nOuter].getArray();
        double* pNewZ = rResult.SequenceZ[nOuter].getArray();

        // variables needed inside loop, when calculating one point of output
        sal_Int32 nPointIndex =0; //index of given contol points
        double fX=0.0;
        double fY=0.0;
        double fZ=0.0; //coordinates of a new BSpline point

        for(sal_Int32 nNewSector=0; nNewSector<nNewSectorCount; nNewSector++)
        { // in first looping fCurveparam has value 0.0

            // Calculate the values of the blending functions for actual curve parameter
            BVector(fCurveparam, n, k, b, t);

            // output point(fCurveparam) = sum over {input point * value of blending function}
            fX = 0.0;
            fY = 0.0;
            fZ = 0.0;
            for (nPointIndex=0;nPointIndex<=n;nPointIndex++)
            {
                fX +=pOldX[nPointIndex]*b[nPointIndex];
                fY +=pOldY[nPointIndex]*b[nPointIndex];
                fZ +=pOldZ[nPointIndex]*b[nPointIndex];
            }
            pNewX[nNewSector] = fX;
            pNewY[nNewSector] = fY;
            pNewZ[nNewSector] = fZ;

            fCurveparam += fCurveStep; //for next looping
        }
        // add last control point to BSpline curve
        pNewX[nNewSectorCount] = pOldX[n];
        pNewY[nNewSectorCount] = pOldY[n];
        pNewZ[nNewSectorCount] = pOldZ[n];

        delete[] t;
        delete[] b;
    }
}

//.............................................................................
} //namespace chart
//.............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
