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

#include "Splines.hxx"
#include <rtl/math.hxx>

#include <vector>
#include <algorithm>
#include <functional>

#define MAX_BSPLINE_DEGREE 15

namespace chart
{
using namespace ::com::sun::star;

namespace
{

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

    /** @descr creates an object that calculates cublic splines on construction
               for the special case of periodic cubic spline

        @param rSortedPoints  the points for which splines shall be calculated,
               they need to be sorted in x values. First and last y value must be equal
     */
    lcl_SplineCalculation( const tPointVecType & rSortedPoints);

    /** @descr this function corresponds to the function splint in [1].

        [1] Numerical Recipies in C, 2nd edition
            William H. Press, et al.,
            Section 3.3, page 116
    */
    double GetInterpolatedValue( double x );

private:
    
    tPointVecType            m_aPoints;

    
    ::std::vector< double >         m_aSecDerivY;

    double m_fYp1;
    double m_fYpN;

    
    lcl_tSizeType m_nKLow;
    lcl_tSizeType m_nKHigh;
    double m_fLastInterpolatedValue;

    /** @descr this function corresponds to the function spline in [1].

        [1] Numerical Recipies in C, 2nd edition
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
            http:
            Section 5.3, page 129
    */
    void CalculatePeriodic();
};

lcl_SplineCalculation::lcl_SplineCalculation(
    const tPointVecType & rSortedPoints,
    double fY1FirstDerivation,
    double fYnFirstDerivation )
        : m_aPoints( rSortedPoints ),
          m_fYp1( fY1FirstDerivation ),
          m_fYpN( fYnFirstDerivation ),
          m_nKLow( 0 ),
          m_nKHigh( rSortedPoints.size() - 1 ),
          m_fLastInterpolatedValue(0.0)
{
    ::rtl::math::setInf( &m_fLastInterpolatedValue, false );
    Calculate();
}

lcl_SplineCalculation::lcl_SplineCalculation(
    const tPointVecType & rSortedPoints)
        : m_aPoints( rSortedPoints ),
          m_fYp1( 0.0 ),  /*dummy*/
          m_fYpN( 0.0 ),  /*dummy*/
          m_nKLow( 0 ),
          m_nKHigh( rSortedPoints.size() - 1 ),
          m_fLastInterpolatedValue(0.0)
{
    ::rtl::math::setInf( &m_fLastInterpolatedValue, false );
    CalculatePeriodic();
}

void lcl_SplineCalculation::Calculate()
{
    if( m_aPoints.size() <= 1 )
        return;

    
    const lcl_tSizeType n = m_aPoints.size() - 1;
    ::std::vector< double > u( n );
    m_aSecDerivY.resize( n + 1, 0.0 );

    if( ::rtl::math::isInf( m_fYp1 ) )
    {
        
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

    
    
    
    for( lcl_tSizeType k = n; k > 0; --k )
    {
        ( m_aSecDerivY[ k - 1 ] *= m_aSecDerivY[ k ] ) += u[ k - 1 ];
    }
}

void lcl_SplineCalculation::CalculatePeriodic()
{
    if( m_aPoints.size() <= 1 )
        return;

    
    const lcl_tSizeType n = m_aPoints.size() - 1;

    
    
    ::std::vector< double > u( n + 1, 0.0 );

    
    m_aSecDerivY.resize( n + 1, 0.0 );

    
    ::std::vector< double > Adiag( n + 1, 0.0 );

    
    ::std::vector< double > Aupper( n + 1, 0.0 );

    
    ::std::vector< double > Ddiag( n+1, 0.0 );

    
    ::std::vector< double > Rright( n-1, 0.0 );

    
    ::std::vector< double > Rupper( n, 0.0 );

    if (n<4)
    {
        if (n==3)
        {   
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
        
            double xDiff0 = m_aPoints[ 1 ].first - m_aPoints[ 0 ].first;
            double xDiff1 = m_aPoints[ 2 ].first - m_aPoints[ 1 ].first;
            double fHelp = 3.0 * (m_aPoints[ 0 ].second - m_aPoints[ 1 ].second) / (xDiff0*xDiff1);
            m_aSecDerivY[ 1 ] = fHelp ;
            m_aSecDerivY[ 2 ] = -fHelp ;
            m_aSecDerivY[ 0 ] = m_aSecDerivY[ 2 ] ;
        }
        else
        {
            
        }
    }
    else
    {
        double xDiff_i =1.0; 
        double xDiff_im1 =1.0;
        double yDiff_i = 1.0;
        double yDiff_im1 = 1.0;
        
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
        Ddiag[ n ] = Adiag[ n ] - fSum - Ddiag[ n-1 ] * Rupper[ n-1 ] * Rupper[ n-1 ]; 

        
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

        
        for ( lcl_tSizeType i=1; i<=n; ++i )
        {
            u[ i ] = u[i] / Ddiag[ i ];
        }

        
        m_aSecDerivY[ n ] = u[ n ];
        m_aSecDerivY[ n-1 ] = u[ n-1 ] - Rupper[ n-1 ] * m_aSecDerivY[ n ];
        for ( lcl_tSizeType i=n-2; i>=1; --i)
        {
            m_aSecDerivY[ i ] = u[ i ] - Rupper[ i ] * m_aSecDerivY[ i+1 ] - Rright[ i ] * m_aSecDerivY[ n ];
        }
        
        m_aSecDerivY[ 0 ] = m_aSecDerivY[ n ];
    }

    
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
        while( ( m_aPoints[ m_nKHigh ].first < x ) &&
               ( m_nKHigh <= n ) )
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




bool createParameterT(const tPointVecType aUniquePoints, double* t)
{   
    
    bool bIsSuccessful = true;
    const lcl_tSizeType n = aUniquePoints.size() - 1;
    t[0]=0.0;
    double dx = 0.0;
    double dy = 0.0;
    double fDiffMax = 1.0; 
    double fDenominator = 0.0; 
    for (lcl_tSizeType i=1; i<=n ; ++i)
    {   
        dx = aUniquePoints[i].first - aUniquePoints[i-1].first;
        dy = aUniquePoints[i].second - aUniquePoints[i-1].second;
        
        fDiffMax = (fabs(dx)>fabs(dy)) ? fabs(dx) : fabs(dy);
        if (fDiffMax == 0.0)
        {
            bIsSuccessful = false;
            break;
        }
        else
        {
            dx /= fDiffMax;
            dy /= fDiffMax;
            fDenominator += sqrt(sqrt(dx * dx + dy * dy)) * sqrt(fDiffMax);
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
                dx = aUniquePoints[i].first - aUniquePoints[i-1].first;
                dy = aUniquePoints[i].second - aUniquePoints[i-1].second;
                fDiffMax = (fabs(dx)>fabs(dy)) ? fabs(dx) : fabs(dy);
                
                dx /= fDiffMax;
                dy /= fDiffMax;
                fNumerator += sqrt(sqrt(dx * dx + dy * dy)) * sqrt(fDiffMax);
            }
            t[j] = fNumerator / fDenominator;

        }
        
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

void createKnotVector(const lcl_tSizeType n, const sal_uInt32 p, double* t, double* u)
{  
        for (lcl_tSizeType j = 0; j <= p; ++j)
        {
            u[j] = 0.0;
        }
        double fSum = 0.0;
        for (lcl_tSizeType j = 1; j <= n-p; ++j )
        {
            fSum = 0.0;
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
    
    double fRightFactor = 0.0;
    double fLeftFactor = 0.0;

    
    rowN[p] = 1.0; 

    
    for (sal_uInt32 s = 1; s <= p; ++s)
    {
        
        fRightFactor = ( u[i+1] - tk ) / ( u[i+1]- u[i-s+1] );
        
        rowN[p-s] = fRightFactor * rowN[p-s+1];

        
        for (sal_uInt32 j = s-1; j>=1 ; --j)
        {
            fLeftFactor = ( tk - u[i-j] ) / ( u[i-j+s] - u[i-j] ) ;
            fRightFactor = ( u[i-j+s+1] - tk ) / ( u[i-j+s+1] - u[i-j+1] );
            
            rowN[p-j] = fLeftFactor * rowN[p-j] + fRightFactor *  rowN[p-j+1];
        }

        
        fLeftFactor = ( tk - u[i] ) / ( u[i+s] - u[i] );
        
        rowN[p] = fLeftFactor * rowN[p];
    }
}

} 



void SplineCalculater::CalculateCubicSplines(
    const drawing::PolyPolygonShape3D& rInput
    , drawing::PolyPolygonShape3D& rResult
    , sal_uInt32 nGranularity )
{
    OSL_PRECOND( nGranularity > 0, "Granularity is invalid" );

    rResult.SequenceX.realloc(0);
    rResult.SequenceY.realloc(0);
    rResult.SequenceZ.realloc(0);

    sal_uInt32 nOuterCount = rInput.SequenceX.getLength();
    if( !nOuterCount )
        return;

    rResult.SequenceX.realloc(nOuterCount);
    rResult.SequenceY.realloc(nOuterCount);
    rResult.SequenceZ.realloc(nOuterCount);

    for( sal_uInt32 nOuter = 0; nOuter < nOuterCount; ++nOuter )
    {
        if( rInput.SequenceX[nOuter].getLength() <= 1 )
            continue; 

        sal_uInt32 nMaxIndexPoints = rInput.SequenceX[nOuter].getLength()-1; 
        const double* pOldX = rInput.SequenceX[nOuter].getConstArray();
        const double* pOldY = rInput.SequenceY[nOuter].getConstArray();
        const double* pOldZ = rInput.SequenceZ[nOuter].getConstArray();

        ::std::vector < double > aParameter(nMaxIndexPoints+1);
        aParameter[0]=0.0;
        for( sal_uInt32 nIndex=1; nIndex<=nMaxIndexPoints; nIndex++ )
        {
            aParameter[nIndex]=aParameter[nIndex-1]+1;
        }

        
        tPointVecType aInputX;
        aInputX.resize(nMaxIndexPoints+1);
        tPointVecType aInputY;
        aInputY.resize(nMaxIndexPoints+1);
        tPointVecType aInputZ;
        aInputZ.resize(nMaxIndexPoints+1);
        for (sal_uInt32 nN=0;nN<=nMaxIndexPoints; nN++ )
        {
          aInputX[ nN ].first=aParameter[nN];
          aInputX[ nN ].second=pOldX[ nN ];
          aInputY[ nN ].first=aParameter[nN];
          aInputY[ nN ].second=pOldY[ nN ];
          aInputZ[ nN ].first=aParameter[nN];
          aInputZ[ nN ].second=pOldZ[ nN ];
        }

        
        
        double fXDerivation;
        double fYDerivation;
        lcl_SplineCalculation* aSplineX;
        lcl_SplineCalculation* aSplineY;
        
        
        

        if( pOldX[ 0 ] == pOldX[nMaxIndexPoints] &&
            pOldY[ 0 ] == pOldY[nMaxIndexPoints] &&
            pOldZ[ 0 ] == pOldZ[nMaxIndexPoints] &&
            nMaxIndexPoints >=2 )
        {   
            aSplineX = new lcl_SplineCalculation( aInputX) ;
            aSplineY = new lcl_SplineCalculation( aInputY) ;
            
        }
        else 
        {
            double fInfty;
            ::rtl::math::setInf( &fInfty, false );
            fXDerivation = fInfty;
            fYDerivation = fInfty;
            aSplineX = new lcl_SplineCalculation( aInputX, fXDerivation, fXDerivation );
            aSplineY = new lcl_SplineCalculation( aInputY, fYDerivation, fYDerivation );
        }

        
        rResult.SequenceX[nOuter].realloc( nMaxIndexPoints*nGranularity + 1);
        rResult.SequenceY[nOuter].realloc( nMaxIndexPoints*nGranularity + 1);
        rResult.SequenceZ[nOuter].realloc( nMaxIndexPoints*nGranularity + 1);

        double* pNewX = rResult.SequenceX[nOuter].getArray();
        double* pNewY = rResult.SequenceY[nOuter].getArray();
        double* pNewZ = rResult.SequenceZ[nOuter].getArray();

        sal_uInt32 nNewPointIndex = 0; 
        
        double    fInc;   
        sal_uInt32 nj;     
        double    fParam; 

        for( sal_uInt32 ni = 0; ni < nMaxIndexPoints; ni++ )
        {
            
            pNewX[nNewPointIndex] = pOldX[ni];
            pNewY[nNewPointIndex] = pOldY[ni];
            pNewZ[nNewPointIndex] = pOldZ[ni];
            nNewPointIndex++;

            
            fInc = ( aParameter[ ni+1 ] - aParameter[ni] ) / static_cast< double >( nGranularity );
            for(nj = 1; nj < nGranularity; nj++)
            {
                fParam = aParameter[ni] + ( fInc * static_cast< double >( nj ) );

                pNewX[nNewPointIndex]=aSplineX->GetInterpolatedValue( fParam );
                pNewY[nNewPointIndex]=aSplineY->GetInterpolatedValue( fParam );
                
                pNewZ[nNewPointIndex] = pOldZ[ni];
                nNewPointIndex++;
            }
        }
        
        pNewX[nNewPointIndex] = pOldX[nMaxIndexPoints];
        pNewY[nNewPointIndex] = pOldY[nMaxIndexPoints];
        pNewZ[nNewPointIndex] = pOldZ[nMaxIndexPoints];
        delete aSplineX;
        delete aSplineY;
        
    }
}









void SplineCalculater::CalculateBSplines(
            const ::com::sun::star::drawing::PolyPolygonShape3D& rInput
            , ::com::sun::star::drawing::PolyPolygonShape3D& rResult
            , sal_uInt32 nResolution
            , sal_uInt32 nDegree )
{
    
    
    
    
    OSL_ASSERT( nResolution > 1 );
    OSL_ASSERT( nDegree >= 1 );

    
    sal_uInt32 p = std::min<sal_uInt32>(nDegree, MAX_BSPLINE_DEGREE);

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
            continue; 

        
        
        
        lcl_tSizeType nMaxIndexPoints = rInput.SequenceX[nOuter].getLength()-1; 
        const double* pOldX = rInput.SequenceX[nOuter].getConstArray();
        const double* pOldY = rInput.SequenceY[nOuter].getConstArray();
        const double* pOldZ = rInput.SequenceZ[nOuter].getConstArray();
        double fZCoordinate = pOldZ[0];
        tPointVecType aPointsIn;
        aPointsIn.resize(nMaxIndexPoints+1);
        for (lcl_tSizeType i = 0; i <= nMaxIndexPoints; ++i )
        {
          aPointsIn[ i ].first = pOldX[i];
          aPointsIn[ i ].second = pOldY[i];
        }
        aPointsIn.erase( ::std::unique( aPointsIn.begin(), aPointsIn.end()),
                     aPointsIn.end() );

        
        
        const lcl_tSizeType n = aPointsIn.size() - 1;
        if (n < 1 || p > n)
            continue; 
                      

        double* t = new double [n+1];
        if (!createParameterT(aPointsIn, t))
        {
            delete[] t;
            continue; 
        }

        lcl_tSizeType m = n + p + 1;
        double* u = new double [m+1];
        createKnotVector(n, p, t, u);

        
        
        
        
        
        double** aMatN = new double*[n+1];
        for (lcl_tSizeType row = 0; row <=n; ++row)
        {
            aMatN[row] = new double[p+1];
            for (sal_uInt32 col = 0; col <= p; ++col)
            aMatN[row][col] = 0.0;
        }
        lcl_tSizeType* aShift = new lcl_tSizeType[n+1];
        aMatN[0][0] = 1.0; 
        aShift[0] = 0;
        aMatN[n][0] = 1.0;
        aShift[n] = n;
        for (lcl_tSizeType k = 1; k<=n-1; ++k)
        { 
            

            
            
            lcl_tSizeType i = p;
            while (!(u[i] <= t[k] && t[k] < u[i+1]))
            {
                ++i;
            }

            
            aShift[k] = i - p;

            applyNtoParameterT(i, t[k], p, u, aMatN[k]);
        } 

        
        
        
        lcl_tSizeType r = 0; 
        lcl_tSizeType c = 0; 
        double fDivisor = 1.0; 
        double fEliminate = 1.0; 
        double fHelp;
        tPointType aHelp;
        lcl_tSizeType nHelp; 
        bool bIsSuccessful = true;
        for (c = 0 ; c <= n && bIsSuccessful; ++c)
        {
            
            r = c;
            while ( r < n && aMatN[r][c-aShift[r]] == 0 )
            {
                ++r;
            }
            if (aMatN[r][c-aShift[r]] == 0.0)
            {
                
                bIsSuccessful = false;
            }
            else
            {
                
                if (r != c)
                {
                    for ( sal_uInt32 i = 0; i <= p ; ++i)
                    {
                        fHelp = aMatN[r][i];
                        aMatN[r][i] = aMatN[c][i];
                        aMatN[c][i] = fHelp;
                    }
                    aHelp = aPointsIn[r];
                    aPointsIn[r] = aPointsIn[c];
                    aPointsIn[c] = aHelp;
                    nHelp = aShift[r];
                    aShift[r] = aShift[c];
                    aShift[c] = nHelp;
                }

                
                fDivisor = aMatN[c][c-aShift[c]]; 
                for (sal_uInt32 i = 0; i <= p; ++i)
                {
                    aMatN[c][i] /= fDivisor;
                }
                aPointsIn[c].first /= fDivisor;
                aPointsIn[c].second /= fDivisor;

                
                
                
                for ( r = c+1; r < n && aShift[r]<=c ; ++r)
                {
                    fEliminate = aMatN[r][0];
                    if (fEliminate != 0.0) 
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
        }
        if( bIsSuccessful)
        {
            
            for (lcl_tSizeType cc = n; cc >= 1; --cc )
            {
                
                
                
                
                r = cc - 1;
                while ( r !=0 && cc-r < p )
                {
                    fEliminate = aMatN[r][ cc - aShift[r] ];
                    if ( fEliminate != 0.0) 
                    {
                        
                        aMatN[r][cc - aShift[r]] = 0.0;
                        aPointsIn[r].first -= fEliminate * aPointsIn[cc].first;
                        aPointsIn[r].second -= fEliminate * aPointsIn[cc].second;
                    }
                    --r;
                }
            }
        }   
        if (bIsSuccessful)
        {
            
            
            lcl_tSizeType nNewSize = nResolution * n + 1;
            rResult.SequenceX[nOuter].realloc(nNewSize);
            rResult.SequenceY[nOuter].realloc(nNewSize);
            rResult.SequenceZ[nOuter].realloc(nNewSize);
            double* pNewX = rResult.SequenceX[nOuter].getArray();
            double* pNewY = rResult.SequenceY[nOuter].getArray();
            double* pNewZ = rResult.SequenceZ[nOuter].getArray();
            pNewX[0] = aPointsIn[0].first;
            pNewY[0] = aPointsIn[0].second;
            pNewZ[0] = fZCoordinate; 
            pNewX[nNewSize -1 ] = aPointsIn[n].first;
            pNewY[nNewSize -1 ] = aPointsIn[n].second;
            pNewZ[nNewSize -1 ] = fZCoordinate;
            double* aP = new double[m+1];
            lcl_tSizeType nLow = 0;
            for ( lcl_tSizeType nTIndex = 0; nTIndex <= n-1; ++nTIndex)
            {
                for (sal_uInt32 nResolutionStep = 1;
                     nResolutionStep <= nResolution && !( nTIndex == n-1 && nResolutionStep == nResolution);
                     ++nResolutionStep)
                {
                    lcl_tSizeType nNewIndex = nTIndex * nResolution + nResolutionStep;
                    double ux = t[nTIndex] + nResolutionStep * ( t[nTIndex+1] - t[nTIndex]) /nResolution;

                    
                    
                    while ( u[nLow] <= ux)
                    {
                        ++nLow;
                    }
                    --nLow;

                    
                    for (lcl_tSizeType i = nLow-p; i <= nLow; ++i)
                    {
                        aP[i] = aPointsIn[i].first;
                    }
                    for (sal_uInt32 lcl_Degree = 1; lcl_Degree <= p; ++lcl_Degree)
                    {
                        double fFactor = 0.0;
                        for (lcl_tSizeType i = nLow; i >= nLow + lcl_Degree - p; --i)
                        {
                            fFactor = ( ux - u[i] ) / ( u[i+p+1-lcl_Degree] - u[i]);
                            aP[i] = (1 - fFactor)* aP[i-1] + fFactor * aP[i];
                        }
                    }
                    pNewX[nNewIndex] = aP[nLow];

                    
                    for (lcl_tSizeType i = nLow - p; i <= nLow; ++i)
                    {
                        aP[i] = aPointsIn[i].second;
                    }
                    for (sal_uInt32 lcl_Degree = 1; lcl_Degree <= p; ++lcl_Degree)
                    {
                        double fFactor = 0.0;
                        for (lcl_tSizeType i = nLow; i >= nLow +lcl_Degree - p; --i)
                        {
                            fFactor = ( ux - u[i] ) / ( u[i+p+1-lcl_Degree] - u[i]);
                            aP[i] = (1 - fFactor)* aP[i-1] + fFactor * aP[i];
                        }
                    }
                    pNewY[nNewIndex] = aP[nLow];
                    pNewZ[nNewIndex] = fZCoordinate;
                }
            }
            delete[] aP;
        }
        delete[] aShift;
        for (lcl_tSizeType row = 0; row <=n; ++row)
        {
            delete[] aMatN[row];
        }
        delete[] aMatN;
        delete[] u;
        delete[] t;

    } 
}

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
