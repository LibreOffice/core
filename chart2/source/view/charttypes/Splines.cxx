#include "Splines.hxx"

#ifndef INCLUDED_RTL_MATH_HXX
#include <rtl/math.hxx>
#endif

#include <vector>
#include <algorithm>
#include <functional>

// header for DBG_ASSERT
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;

namespace
{

template< typename T, typename U >
    struct lcl_LessFirstOfPair : ::std::binary_function< ::std::pair< T, U >, ::std::pair< T, U >, bool >
{
    inline bool operator() ( const ::std::pair< T, U > & rOne, const ::std::pair< T, U > & rOther )
    {
        return ( rOne.first < rOther.first );
    }
};

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

        @param rPoints  the points for which splines shall be calculated
        @param fY1FirstDerivation the resulting spline should have the first
               derivation equal to this value at the x-value of the first point
               of rPoints.  If fY1FirstDerivation is set to infinity, a natural
               spline is calculated.
        @param fYnFirstDerivation the resulting spline should have the first
               derivation equal to this value at the x-value of the last point
               of rPoints
     */
    lcl_SplineCalculation( const tPointVecType & rPoints,
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
    const tPointVecType & rPoints,
    double fY1FirstDerivation,
    double fYnFirstDerivation )
        : m_aPoints( rPoints ),
          m_fYp1( fY1FirstDerivation ),
          m_fYpN( fYnFirstDerivation ),
          m_nKLow( 0 ),
          m_nKHigh( rPoints.size() - 1 )
{
    ::rtl::math::setInf( &m_fLastInterpolatedValue, sal_False );

    ::std::sort( m_aPoints.begin(), m_aPoints.end(),
          lcl_LessFirstOfPair< double, double >() );

    // #108301# remove points that have equal x-values
    m_aPoints.erase( ::std::unique( m_aPoints.begin(), m_aPoints.end(),
                             lcl_EqualsFirstDoubleOfPair< double >() ),
                     m_aPoints.end() );

    Calculate();
}

void lcl_SplineCalculation::Calculate()
{
    // n is the last valid index to m_aPoints
    const tPointVecType::size_type n = m_aPoints.size() - 1;
    if( n < 1 )
        return;

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
    DBG_ASSERT( m_aPoints.size() < 1 ||
                ( ( m_aPoints[ 0 ].first <= x ) &&
                  ( x <= m_aPoints[ m_aPoints.size() - 1 ].first ) ),
                "Trying to extrapolate" );

    const tPointVecType::size_type n = m_aPoints.size() - 1;
    if( n < 1 )
    {
        double fNan;
        ::rtl::math::setNan( & fNan );
        return fNan;
    }

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

// this is the maximum number of points that will be inserted into an XPolygon
//const lcl_tSizeType nPolygonSizeThreshold = 0xff00;

tPointVecType makeVector( const drawing::PolyPolygonShape3D& rPoly, sal_Int32 nPolyIndex=0 )
{
    //creates a vector from only one poly within the PolyPolygon
    //the third dimension is ignored (3D->2D)

    tPointVecType aRet;
    if(nPolyIndex>=0&&nPolyIndex<rPoly.SequenceX.getLength())
    {
        sal_Int32 nPointCount = rPoly.SequenceX[nPolyIndex].getLength();
        if(nPointCount)
        {
            const double* pXSequence = rPoly.SequenceX[nPolyIndex].getConstArray();
            const double* pYSequence = rPoly.SequenceY[nPolyIndex].getConstArray();
            aRet.resize(nPointCount);
            for(sal_Int32 nN=0;nN<nPointCount;nN++)
            {
                aRet[ nN ].first  = pXSequence[nN];
                aRet[ nN ].second = pYSequence[nN];
            }
        }
    }
    return aRet;
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
    for( sal_Int32 i=0; i<=n+k; i++ )
        b[i]=0;

    sal_Int32 i0 = (sal_Int32)floor(x) + k - 1;
    b [i0] = 1;

    for( sal_Int32 j=2; j<=k; j++ )
        for( sal_Int32 i=0; i<=i0; i++ )
            b[i] = TLeft(x, i, j, t) * b[i] + TRight(x, i, j, t) * b [i + 1];
}

//calculate single point
void BSPoint(sal_Int32 n, double& rY1, double& rY2
             , const double* pKnownPointsY, double *b)
{
    for (sal_Int32 i = 0; i <= n; i ++)
    {
        rY1 = rY1 + b[i] * pKnownPointsY[i];
        rY2 = rY2 + b[n-i] * pKnownPointsY[i];
    }
}

} //  anonymous namespace

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
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

    if( !rInput.SequenceX.getLength() )
        return;
    if(!rInput.SequenceX[0].getLength())
        return;

    // calculate second derivates
    double fInfty;
    ::rtl::math::setInf( &fInfty, sal_False );
    lcl_SplineCalculation aSpline( makeVector(rInput), fInfty, fInfty );

    // fill result polygon with calculated values
    lcl_tSizeType nOldPointCount = rInput.SequenceX[0].getLength();

    rResult.SequenceX.realloc(1);
    rResult.SequenceY.realloc(1);
    rResult.SequenceZ.realloc(1);
    rResult.SequenceX[0].realloc( (nOldPointCount-1)*nGranularity + 1);
    rResult.SequenceY[0].realloc( (nOldPointCount-1)*nGranularity + 1);
    rResult.SequenceZ[0].realloc( (nOldPointCount-1)*nGranularity + 1);

    double* pNewX = rResult.SequenceX[0].getArray();
    double* pNewY = rResult.SequenceY[0].getArray();
    double* pNewZ = rResult.SequenceZ[0].getArray();

    const double* pOldX = rInput.SequenceX[0].getConstArray();
    const double* pOldY = rInput.SequenceY[0].getConstArray();
    const double* pOldZ = rInput.SequenceZ[0].getConstArray();

    //we know that their is at least one point here
    pNewX[0]=pOldX[0];
    pNewY[0]=pOldY[0];
    pNewZ[0]=pOldZ[0];

    // calculate all additional points on spline curve
    sal_Int32 nNewPointIndex = 1;
    for( lcl_tSizeType i = 1; i < nOldPointCount; ++i )
    {
        double fBaseX = pOldX[ i - 1 ];
        double fInc = ( pOldX[ i ] - fBaseX ) /
            static_cast< double >( nGranularity );

        for( sal_Int32 j = 1; j <= nGranularity; ++j, nNewPointIndex++ )
        {
            double x = fBaseX + ( fInc * static_cast< double >( j ) );

            pNewX[nNewPointIndex]=x;
            pNewY[nNewPointIndex]=aSpline.GetInterpolatedValue( x );
            pNewZ[nNewPointIndex]=pOldZ[i];
        }
    }
}

void SplineCalculater::CalculateBSplines(
            const ::com::sun::star::drawing::PolyPolygonShape3D& rInput
            , ::com::sun::star::drawing::PolyPolygonShape3D& rResult
            , sal_Int32 nGranularity
            , sal_Int32 nDegree )
{
    DBG_ASSERT( nGranularity > 0, "Granularity is invalid" );
    rResult.SequenceX.realloc(0);
    rResult.SequenceY.realloc(0);
    rResult.SequenceZ.realloc(0);

    if( !rInput.SequenceX.getLength() )
        return;
    if(!rInput.SequenceX[0].getLength())
        return;

    const double* pOldX = rInput.SequenceX[0].getConstArray();
    const double* pOldY = rInput.SequenceY[0].getConstArray();

    sal_Int32 n = rInput.SequenceX[0].getLength()-1;//maximim index of control points
    sal_Int32 nNewSectorCount = nGranularity * n;

    double *b       = new double [n + nDegree + 1];
    double xStep    = ((double) n - (double)nDegree + 2.0) / (double) nNewSectorCount;
    double dStep    = ( pOldX[n] - pOldX[0] ) / (double) nNewSectorCount;
    double dXUp     = pOldX[0];
    double dXDown   = pOldX[n];
    double x        = 0.0;

    const double* t = createTVector(n, nDegree);

    sal_Int32 nHalf  = nNewSectorCount / 2 + 1;
    rResult.SequenceX.realloc(1);
    rResult.SequenceY.realloc(1);
    rResult.SequenceZ.realloc(1);
    rResult.SequenceX[0].realloc(nNewSectorCount+1);
    rResult.SequenceY[0].realloc(nNewSectorCount+1);
    rResult.SequenceZ[0].realloc(nNewSectorCount+1);
    double* pNewX = rResult.SequenceX[0].getArray();
    double* pNewY = rResult.SequenceY[0].getArray();

    for(sal_Int32 j=0; j<=nHalf; j++ )
    {
        double fY1=0.0;
        double fY2=0.0;

        BVector(x, n, nDegree, b, t);
        BSPoint(n, fY1, fY2, pOldY, b);

        pNewX[j]                   = floor(dXUp);//(sal_Int32)(floor(dXUp)+0.5);
        pNewY[j]                   = fY1;
        pNewX[nNewSectorCount - j] = floor(dXDown);//(sal_Int32)(floor(dXDown)+0.5);
        pNewY[nNewSectorCount - j] = fY2;

        x      += xStep;
        dXUp   += dStep;
        dXDown -= dStep;
    }

    delete[] t;
    delete[] b;
}

//.............................................................................
} //namespace chart
//.............................................................................
