/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "interpre.hxx"
#include "global.hxx"
#include "scmatrix.hxx"

#include <vector>

// TODO: forecast loopt niet goed
// TODO: andere a, b, c bepaling? (geen 2, maar 8 deling en pas inzoomen als 'gebied' is begrenst)

using ::std::vector;

struct DataPoint
{
    double fX, fY;

    DataPoint() : fX( 0.0 ), fY( 0.0 ) {};
    DataPoint( double rX, double rY ) : fX( rX ), fY( rY ) {};
};

static bool lcl_SortByX( const DataPoint &lhs, const DataPoint &rhs ) { return lhs.fX < rhs.fX; }

/**
 * Triple Exponential Smoothing (Holt-Winters method)
 *
 * Class is set up to be used with Calc's FORECAST.ETS functions and with
 * chart extrapolations (not yet implemented).
 *
 * Calculations here are based on
 * Engineering Statistics Handbook, 6.4.3.5 Triple Exponential Smoothing>
 * see "http://www.itl.nist.gov/div898/handbook/pmc/section4/pmc435.htm"
 */
class ScETSForecastCalculation
{
    private:
    vector< DataPoint > sRange;         // data (X, Y)
    vector< double > sSmoothed;         // calculated smoothed data
    vector< double > sTrend;            // calculated trend factors
    vector< double > sPeriodicalIdx;    // calculated periodical indexes
    vector< DataPoint > sForecast;      // forecasted values (X, Y)
    SCSIZE nSamplesinPeriod;            // length of period
    double fStepSize;                   // increment of X in sRange and sForecast
    double fAlpha, fBeta, fGamma;       // constants to minimise the MSE in the ETS-equations
    double* pfI;                        // seasonal index
    SCSIZE nCount;                      // No of data points
    bool bInitialised;
    // accuracy indicators
    double fBias;                       // bias (mean error)
    double fMad;                        // mean absolute deviation
    double fMare;                       // mean absolute relative error
    double fMse;                        // mean square of the errors
    double fSae;                        // sum of absolute errors
    const double cfMinABCResolution = 0.001;  // minimum change in determining best fit for alpha, beta, gamma

    bool initData();
    bool prefillSmoothedData();
    bool prefillTrendData();
    bool prefillPeriodicalIdx();
    bool initCalc();
    void refill();
    void CalcAlphaBetaGamma();
    void CalcBetaGamma();
    void CalcGamma();
    void refillSmoothed( SCSIZE ni, SCSIZE nIdx );
    void refillTrend( SCSIZE ni );
    void refillPeriodicalIdx( SCSIZE ni, SCSIZE nIdx );
    void calcAccuracyIndicators( bool bShowOutput );

    public:
    ScETSForecastCalculation() : sRange( 0 ), sSmoothed( 0 ), sPeriodicalIdx( 0 ), sForecast( 0 ), bInitialised( false ) {};
    bool PreprocessDataRange( ScMatrixRef rMatX, ScMatrixRef rMatY, int& rSamplesInPeriod,
                              bool bDataCompletion, double fTarget, int& rPreprocError  );
    bool GetForecast( double fTarget, double& rForecast );

    // for charts and array output, not yet in use
    bool CalcForecastRange( double fTargetFrom, double fTargetTill );
};

bool ScETSForecastCalculation::PreprocessDataRange( ScMatrixRef rMatX, ScMatrixRef rMatY, int& rSamplesInPeriod,
                                   bool bDataCompletion, double fTarget, int& rPreprocError  )
{
    sRange.resize( rMatX->GetElementCount() );
    nCount = sRange.size();
    //SAL_OFF_DEBUG( "tdf94635, sRange size = " << nCount );
    nSamplesinPeriod = rSamplesInPeriod;

    // MatX needs to be sorted
    for ( SCSIZE i = 0; i < nCount; i++ )
    {
        sRange[ i ].fX = rMatX->GetDouble( i );
        sRange[ i ].fY = rMatY->GetDouble( i );
/*
 * This constraint is an Excel constraint. For testing and for range output, it is useful
 * to have fTarget not less than the minimum X value
        if ( fTarget < sRange[ i ].fX )
        {
            // target cannot be less than end of time(X)-range
            //SAL_OFF_DEBUG( "tdf94635, target cannot be less than end of time(X)-range" );
            rPreprocError = errIllegalFPOperation; // NUM!
            return false;
        }
*/
    }
    sort( sRange.begin(), sRange.end(), lcl_SortByX );

// This constraint is an absolute constraint.
    if ( fTarget < sRange[ 0 ].fX )
    {
        // target cannot be less than start of time(X)-range
        //SAL_OFF_DEBUG( "tdf94635, target cannot be less than start of time(X)-range" );
        rPreprocError = errIllegalFPOperation; // NUM!
        return false;
    }


    // minimum step is _the_ (exact) stepsize
    fStepSize = sRange[ 1 ].fX - sRange[ 0 ].fX;
    for ( SCSIZE i = 1; i < nCount; i++ )
    {
        double fStep = sRange[ i ].fX - sRange[ i - 1 ].fX;
        if ( fStep < fStepSize )
            fStepSize = fStep;
    }
    //SAL_OFF_DEBUG( "fStepSize = " << fStepSize );
    // step must be constant (or gap multiple of step)
    bool bHasGap = false;
    for ( SCSIZE i = 1; i < nCount; i++ )
    {
        double fStep = sRange[ i ].fX - sRange[ i - 1 ].fX;
        if ( fStep != fStepSize )
        {
            if ( fmod( fStep, fStepSize ) != 0.0 )
            {
                // step not constant nor multiple of fStepSize in case of gaps
                //SAL_OFF_DEBUG( "tdf94635, step not constant nor multiple of minimum step" );
                rPreprocError = errNoValue;
                return false;
            }
            bHasGap = true;
        }
    }
    // fill gaps with values depending on bDataCompletion
    if ( bHasGap )
    {
        SCSIZE nMissingXCount = 0;
        double fOriginalCount = ( double ) nCount;
        for ( SCSIZE i = 1; i < nCount; i++ )
        {
            double fDist = sRange[ i ].fX - sRange[ i - 1 ].fX;
            if ( fDist > fStepSize )
            {
                // gap, fill mssing data points
                double fYGap = ( bDataCompletion ? ( sRange[ i ].fY + sRange[ i - 1 ].fY ) / 2.0 : 0.0 );
                for ( double fXGap = sRange[ i - 1].fX + fStepSize;  fXGap < sRange[ i ].fX; fXGap += fStepSize )
                {
                    sRange.insert( sRange.begin() + i, DataPoint( fXGap, fYGap ) );
                    i++;
                    nCount++;
                    nMissingXCount++;
                    if ( ( double ) nMissingXCount / fOriginalCount > 0.3 )
                    {
                        // maximum of 30% missing points exceeded
                        //SAL_OFF_DEBUG( "tdf94635, maximum of 30% missing values exceeded" );
                        rPreprocError = errNoValue;
                        return false;
                    }
                }
            }
        }
        //SAL_OFF_DEBUG( "tdf94635, " << nMissingXCount << " datapoints inserted (" <<
        //           100.0 * ( double ) nMissingXCount / fOriginalCount << " %), total datapoints " << nCount );
    }
    if ( !initData() )
        return false;

    return true;
}

bool ScETSForecastCalculation::initData()
{
    // give variuous vectors initial values
    sForecast.resize( nCount );
    sForecast[ 0 ] = sRange[ 0 ];

    if ( prefillTrendData() )
    {
        if ( prefillSmoothedData() )
        {
            if ( prefillPeriodicalIdx() )
                return true;
        }
    }

    return false;
}

bool ScETSForecastCalculation::prefillTrendData()
{
    // we need at least 2 periods in the data range
    if ( nCount < 2 * nSamplesinPeriod )
    {
        //SAL_OFF_DEBUG( "tdf94635, too few datapoints; we need at least 2 seasons." );
        return false;
    }

    double fSum = 0.0;
    for ( SCSIZE i = 0; i < nSamplesinPeriod; i++ )
        fSum += sRange[ i + nSamplesinPeriod ].fY - sRange[ i ].fY;
    double fTrend = fSum / ( double )( nSamplesinPeriod * nSamplesinPeriod );

    sTrend.resize( nCount );
    sTrend[ 0 ] = fTrend;
    //SAL_OFF_DEBUG( "tdf94635, initial sTrend  = " << fTrend );
    return true;
}

bool ScETSForecastCalculation::prefillSmoothedData()
{
    if ( nCount < 2 * nSamplesinPeriod )
    {
        //SAL_OFF_DEBUG( "tdf94635, " << nCount << " datapoints, but we need 2 seasons at least, which is  " << ( 2 * nSamplesinPeriod ) );
        return false;
    }
    double fPeriod2Average = 0.0;

    for ( SCSIZE i = 0; i < nSamplesinPeriod; i ++ )
        fPeriod2Average += sRange[ i ].fY;
    fPeriod2Average /= ( double )nSamplesinPeriod;
    double fOffset = fPeriod2Average - ( sTrend[ 0 ] * 2.0 / ( double )nSamplesinPeriod ) ;

    sSmoothed.resize( nCount );
    sSmoothed[ 0 ] = fOffset;
    //SAL_OFF_DEBUG( "tdf95635, initial sSmoothed[ 0 ] = " << sSmoothed[ 0 ] );
    return true;
}

bool ScETSForecastCalculation::prefillPeriodicalIdx()
{
    // use as many complete periods as available
    SCSIZE nPeriods = nCount / nSamplesinPeriod;
    double* pfPeriodAverage = new double[ nPeriods ];
    for ( SCSIZE i = 0; i < nPeriods ; i++ )
    {
        pfPeriodAverage[ i ] = 0.0;
        for ( SCSIZE j = 0; j < nSamplesinPeriod; j++ )
            pfPeriodAverage[ i ] += sRange[ i * nSamplesinPeriod + j ].fY;
        pfPeriodAverage[ i ] /= ( double ) nSamplesinPeriod;
        if ( pfPeriodAverage[ i ] == 0.0 )
        {
            SAL_WARN( "sc.core", "tdf94635, average of 0 will cause divide by zero error, quitting calculation" );
            return false;
        }
    }

    sPeriodicalIdx.resize( nCount );
    double fI;
    for ( SCSIZE j = 0; j < nSamplesinPeriod; j++ )
    {
        fI = 0.0;
        for ( SCSIZE i = 0; i < nPeriods ; i++ )
            fI += ( sRange[ i * nSamplesinPeriod + j ].fY / pfPeriodAverage[ i ] );
        sPeriodicalIdx[ j ] = fI / nPeriods;
        //SAL_OFF_DEBUG( "tdf94635, initial periodical index[" << j << "] = " << sPeriodicalIdx[ j ] );
    }
    return true;
}

bool ScETSForecastCalculation::initCalc()
{
    if ( !bInitialised )
    {
        CalcAlphaBetaGamma();

        bInitialised = true;
        //for ( SCSIZE i = 1; i < nCount; i++ )
        //{
            //SAL_OFF_DEBUG( "tdf94635, x = " << sForecast[ i ].fX << ", smoothed= " << sSmoothed[ i ] <<
            //           ", trend= " << sTrend[ i ] << ", per.idx= " << sPeriodicalIdx[ i ] <<
            //           ", forecast= " << sForecast[ i].fY );
        //}
    }
    calcAccuracyIndicators( true );

    return true;
}

void ScETSForecastCalculation::refill()
{
    // refill sSmoothed, sTrend, sPeriodicalidx and sForecast with values
    // using the calculated fAlpha, fBeta, fGamma
    for ( SCSIZE i = 1; i < nCount; i++ )
    {
        SCSIZE nIdx = i - ( i >= nSamplesinPeriod ? nSamplesinPeriod : 0 );
        refillSmoothed( i, nIdx );
        refillTrend( i );
        refillPeriodicalIdx( i, nIdx );

        double fVal = ( sSmoothed[ i ] + sTrend[ i ] ) * sPeriodicalIdx[ nIdx ];
        sForecast[ i ].fX = sRange[ 0 ].fX * i * fStepSize;
        sForecast[ i ].fY = fVal;
    }
    calcAccuracyIndicators( false );
}

void ScETSForecastCalculation::CalcAlphaBetaGamma()
{
    double f0, f1, f2;
    double fE0, fE2;

    f0 = 0.0;
    fAlpha = f0;
    CalcBetaGamma();
    refill();
    fE0 = fMse;

    f2 = 1.0;
    fAlpha = f2;
    CalcBetaGamma();
    refill();
    fE2 = fMse;

    f1 = 0.5;
    fAlpha = f1;
    CalcBetaGamma();
    refill();

    if ( fE0 == fMse and fMse == fE2 )
    {
        fAlpha = 0;
        CalcBetaGamma();
        refill();
        //SAL_OFF_DEBUG( "tdf94635, calc alpha, E0=" << fE0 << ", E1=" << fMse << ", E2=" << fE2 << ", fAlpha=0" );
        return;
    }
    while ( ( f2 - f1 ) > cfMinABCResolution )
    {
        if ( fE2 > fE0 )
        {
            f2 = f1;
            fE2 = fMse;
            f1 = ( f0 + f1 ) / 2;
        }
        else
        {
            f0 = f1;
            fE0 = fMse;
            f1 = ( f1 + f2 ) / 2;
        }
        fAlpha = f1;
        CalcBetaGamma();
        refill();
        //SAL_OFF_DEBUG( "tdf94635, calc alpha, alpha=" << fAlpha << ", beta=" << fBeta << ", E0=" << fE0 << ", E1=" << fMse << ", E2=" << fE2 );
    }
    if ( fE2 > fE0 )
    {
        if ( fE0 < fMse )
        {
            fAlpha = f0;
            CalcBetaGamma();
            refill();
        }
    }
    else
    {
        if ( fE2 < fMse )
        {
            fAlpha = f2;
            CalcBetaGamma();
            refill();
        }
    }
    calcAccuracyIndicators( true );

    return;
}

void ScETSForecastCalculation::CalcBetaGamma()
{
    double f0, f1, f2;
    double fE0, fE2;
//    SCSIZE i = 0;

    f0 = 0.0;
    fBeta = f0;
    CalcGamma();
    refill();
    fE0 = fMse;

    f2 = 1.0;
    fBeta = f2;
    CalcGamma();
    refill();
    fE2 = fMse;

    f1 = 0.5;
    fBeta = f1;
    CalcGamma();
    refill();

    if ( fE0 == fMse and fMse == fE2 )
    {
        fBeta = 0;
        CalcGamma();
        refill();
        //SAL_OFF_DEBUG( "tdf94635, calc beta, E0=" << fE0 << ", E1=" << fMse << ", E2=" << fE2 << ", fBeta=0" );
        return;
    }
    //SAL_OFF_DEBUG( "tdf94635, calc beta, i=" << i << ", E0=" << fE0 << ", E1=" << fMse << ", E2=" << fE2 );
    while ( ( f2 - f1 ) > cfMinABCResolution )
    {
        if ( fE2 > fE0 )
        {
            f2 = f1;
            fE2 = fMse;
            f1 = ( f0 + f1 ) / 2;
        }
        else
        {
            f0 = f1;
            fE0 = fMse;
            f1 = ( f1 + f2 ) / 2;
        }
        fBeta = f1;
        CalcGamma();
        refill();
        //SAL_OFF_DEBUG( "tdf94635, calc beta, alpha=" << fAlpha << ", beta=" << fBeta << ", E0=" << fE0 << ", E1=" << fMse << ", E2=" << fE2 );
    }
    if ( fE2 > fE0 )
    {
        if ( fE0 < fMse )
        {
            fBeta = f0;
            CalcGamma();
            refill();
        }
    }
    else
    {
        if ( fE2 < fMse )
        {
            fBeta = f2;
            CalcGamma();
            refill();
        }
    }
}

void ScETSForecastCalculation::CalcGamma()
{
    double f0, f1, f2;
    double fE0, fE2;

    f0 = 0.0;
    fGamma = f0;
    refill();
    fE0 = fMse;

    f2 = 1.0;
    fGamma = f2;
    refill();
    fE2 = fMse;

    f1 = 0.5;
    fGamma = f1;
    refill();

    if ( fE0 == fMse and fMse == fE2 )
    {
        fGamma = 0;
        refill();
        //SAL_OFF_DEBUG( "tdf94635, calc gamma, E0=" << fE0 << ", E1=" << fMse << ", E2=" << fE2 << ", fGamma=0" );
        return;
    }
    while ( ( f2 - f1 ) > cfMinABCResolution )
    {
        if ( fE2 > fE0 )
        {
            f2 = f1;
            fE2 = fMse;
            f1 = ( f0 + f1 ) / 2;
        }
        else
        {
            f0 = f1;
            fE0 = fMse;
            f1 = ( f1 + f2 ) / 2;
        }
        fGamma = f1;
        refill();
        //SAL_OFF_DEBUG( "tdf94635, calc gamma, alpha=" << fAlpha << ", beta=" << fBeta << ", gamma=" << fGamma << ", E0=" << fE0 << ", E1=" << fMse << ", E2=" << fE2 );
    }
    if ( fE2 > fE0 )
    {
        if ( fE0 < fMse )
        {
            fGamma = f0;
            refill();
        }
    }
    else
    {
        if ( fE2 < fMse )
        {
            fGamma = f2;
            refill();
        }
    }
}

void ScETSForecastCalculation::refillSmoothed( SCSIZE ni, SCSIZE nIdx )
{
    double fS = fAlpha * ( sRange[ ni ].fY / sPeriodicalIdx[ nIdx ] ) +
                ( 1 - fAlpha ) * ( sSmoothed[ ni - 1 ] + sTrend[ ni - 1 ] );
    sSmoothed[ ni ] =  fS;
}

void ScETSForecastCalculation::refillTrend( SCSIZE ni )
{
    double fT = fGamma * ( sSmoothed[ ni ] - sSmoothed[ ni - 1 ] ) +
                ( 1 - fGamma ) * sTrend[ ni - 1 ];
    sTrend[ ni ] = fT;
}

void ScETSForecastCalculation::refillPeriodicalIdx( SCSIZE ni, SCSIZE nIdx )
{
    double fI = fBeta * ( sRange[ ni ].fY / sSmoothed[ ni ] ) +
                ( 1 - fBeta ) * sPeriodicalIdx[ nIdx ];
    sPeriodicalIdx[ ni ] = fI;
}

bool ScETSForecastCalculation::GetForecast( double fTarget, double& rForecast )
{
    if ( !initCalc() )
        return false;

    if ( fTarget <= sRange[ nCount - 1 ].fX )
    {
        SCSIZE n = ( fTarget - sRange[ 0 ].fX ) / fStepSize;
        double fInterpolate = fmod( fTarget - sRange[ 0 ].fX, fStepSize );
        //SAL_OFF_DEBUG( "tdf94635, Forecast n = " << n << ", interpolate= " << fInterpolate );
        rForecast = sForecast[ n ].fY;

        if ( fInterpolate >= cfMinABCResolution )
        {
            double fInterpolateFactor = fInterpolate / fStepSize;
            double fFc_1 = sForecast[ n + 1 ].fY;
            rForecast = fInterpolateFactor * fFc_1 + ( 1.0 - fInterpolateFactor ) * rForecast;
        }
    }
    else
    {
        SCSIZE n = ( fTarget - sRange[ nCount - 1 ].fX ) / fStepSize;
        double fInterpolate = fmod( fTarget - sRange[ nCount - 1 ].fX, fStepSize );
        //SAL_OFF_DEBUG( "tdf94635, Forecast n = " << n << ", interpolate= " << fInterpolate );
        rForecast = ( sSmoothed[ nCount - 1 ] + n * sTrend[ nCount - 1 ] ) *
                    sPeriodicalIdx[ nCount - 1 - nSamplesinPeriod + ( n % nSamplesinPeriod ) ];

        if ( fInterpolate >= cfMinABCResolution )
        {
            double fInterpolateFactor = fInterpolate / fStepSize;
            double fFc_1 = ( sSmoothed[ nCount - 1 ] + n * sTrend[ nCount - 1 ] ) *
                           sPeriodicalIdx[ nCount - 1 - nSamplesinPeriod + ( n % nSamplesinPeriod ) ];
            rForecast = fInterpolateFactor * fFc_1 + ( 1.0 - fInterpolateFactor ) * rForecast;
        }
    }
    //SAL_OFF_DEBUG( "tdf94635, forecast, x = " << fTarget << ", y = " << rForecast );

    return true;
}

bool ScETSForecastCalculation::CalcForecastRange( double fTargetFrom, double fTargetTill )
{
    if ( !initCalc() )
        return false;

    if ( fTargetTill > sRange[ nCount - 1 ].fX )
    {
        SCSIZE nStart = ( fTargetFrom - sRange[ nCount - 1 ].fX ) / fStepSize;
        SCSIZE nSteps = ( fTargetTill - fTargetFrom ) / fStepSize;
        if ( fmod( ( fTargetTill - sRange[ nCount - 1 ].fX ), fStepSize ) >= cfMinABCResolution )
            nSteps++;

        for ( SCSIZE i = nStart; i <= nSteps; i++ )
        {
            double fY = ( sSmoothed[ nCount - 1 ] + i * sTrend[ nCount - 1 ] ) *
                        sPeriodicalIdx[ nCount - 1 - nSamplesinPeriod + ( i % nSamplesinPeriod ) ];
            double fX = sForecast[ nCount - 1 ].fX + i * fStepSize;
            //SAL_OFF_DEBUG( "tdf94635, calcforecastrange, x= " << fX << ", y= " << fY );
            sForecast.push_back( DataPoint( fX, fY ) );
        }
    }

    return true;
}

void ScETSForecastCalculation::calcAccuracyIndicators( bool /*bShowOutput*/ )
{
    double fSumErr       = 0.0;
    double fSumAbsErr    = 0.0;
    double fSumAbsRelErr = 0.0;
    double fSumErrSq     = 0.0;

    for ( SCSIZE i = 1; i < nCount; i++ )
    {
        double fError = sForecast[ i ].fY - sRange[ i ].fY;
        fSumErr       += fError;
        fSumAbsErr    += fabs( fError );
        fSumAbsRelErr += fabs( fError / sRange[ i ].fY );
        fSumErrSq     += fError * fError;
    }

    fBias = fSumErr / ( nCount - 1 );
    fMad  = fSumAbsErr / ( nCount - 1 );
    fMare = fSumAbsRelErr / ( nCount - 1 );
    fMse  = fSumErrSq / ( nCount - 1 );
    fSae  = fSumAbsErr;

    //if ( bShowOutput  )
    //{
        //SAL_OFF_DEBUG( "tdf94635, fAlpha = " << fAlpha << ", fBeta = " << fBeta << ", fGamma = " << fGamma );
        //SAL_OFF_DEBUG( "tdf94635, fBias= " << fBias << ", fMad= " << fMad << ", fMare= " << fMare << ", fMse= " << fMse << ", fSae= " << fSae );
    //}
}


void ScInterpreter::ScForecast_Ets()
{
    sal_uInt8 nParamCount = GetByte();
    //SAL_OFF_DEBUG( "tdf94635, nParamCount=" << (int) nParamCount );
    if ( !MustHaveParamCount( nParamCount, 3, 6 ) )
        return;

    int nAggregation;
    if ( nParamCount == 6 )
    {
        if ( IsMissing() )
        {
            nAggregation = 0;
            Pop();
        }
        else
            nAggregation = ( int ) GetDouble();
    }
    else
        nAggregation = 0;
    //SAL_OFF_DEBUG( "tdf94635, nAggregation=" << nAggregation );
    // tests with Excel2016 showed that aggregation value 0 is not allowed, although i
    // it is mentioned by MS as default value (being AVERAGE) in their help text
    // TODO: implement change properly in code
    if ( nAggregation == 0 )
        nAggregation = 1; // 'proper' default value

    bool bDataCompletion;
    if ( nParamCount >= 5 )
    {
        if ( IsMissing() )
        {
            bDataCompletion = true;
            Pop();
        }
        else
        {
            int nTemp = ( int ) GetDouble();
            if ( nTemp == 0 || nTemp == 1 )
                bDataCompletion = nTemp;
            else
            {
                // invalid value for argument DataCompletion
                //SAL_OFF_DEBUG( "tdf94635, invalid value for bDataCompletion :" << nTemp );
                PushIllegalParameter();
                return;
            }
        }
    }
    else
        bDataCompletion = true;
    //SAL_OFF_DEBUG( "tdf94635, bDataCompletion=" << bDataCompletion );
    int nSamplesinPeriod;
    if ( nParamCount >= 4 )
    {
        if ( IsMissing() )
        {
            nSamplesinPeriod = 1;
            Pop();
        }
        else
            nSamplesinPeriod = ( int ) GetDouble();
    }
    else
        nSamplesinPeriod = 1;
    //SAL_OFF_DEBUG( "tdf94635, nSamplesinPeriod=" << nSamplesinPeriod );

    // required arguments
    ScMatrixRef pMatX = GetMatrix();
    ScMatrixRef pMatY = GetMatrix();
    if ( !pMatX || !pMatY )
    {
        //if ( !pMatX )
            //SAL_OFF_DEBUG( "tdf94635, MatX bestaat niet" );
        //if ( !pMatY )
            //SAL_OFF_DEBUG( "tdf94635, MatY bestaat niet" );
        PushIllegalParameter();
        return;
    }
    SCSIZE nCX, nCY;
    SCSIZE nRX, nRY;
    pMatX->GetDimensions( nCX, nRX );
    pMatY->GetDimensions( nCY, nRY );
    if ( nRX != nRY || nCX != nCY ||
         !pMatX->IsNumeric() || !pMatY->IsNumeric() )
    {
        //SAL_OFF_DEBUG( "tdf94635, Matrix X en Y hebben verschillende grootte of X of Y is niet numeriek" );
        PushIllegalArgument();
        return;
    }
    double fTarget = GetDouble();
    //SAL_OFF_DEBUG( "tdf94635, fTarget=" << fTarget );

    if ( nSamplesinPeriod == 0 )
    {
        // linear forecast
        //SAL_OFF_DEBUG ( "tdf94635, linear forecast" );
        double fForecast = LinearForecast( pMatX, pMatY, fTarget );
        if ( !nGlobalError )
            PushDouble( fForecast );
        return;
    }
    else
    {
        if ( nSamplesinPeriod < 0 || nSamplesinPeriod > 8760 )
        {
            // SamplesInPeriod outside allowed range (8760 is number of hours in year)
            //SAL_OFF_DEBUG( "tdf94635, SamplesInPeriod < 0 or > 8760 (hours in year)" );
            PushError( errIllegalFPOperation ); // NUM!
            return;
        }
    }

    ScETSForecastCalculation sETSCalc;
    int nErrorValue;
    if ( !sETSCalc.PreprocessDataRange( pMatX, pMatY, nSamplesinPeriod, bDataCompletion, fTarget, nErrorValue ) )
    {
        PushError( nErrorValue );
        return;
    }

    double fForecast;
    if ( sETSCalc.GetForecast( fTarget, fForecast ) )
        PushDouble( fForecast ) ;
    else
        PushError( 521 ); // function not yet available TODO: provide correct error code

    return;
}

void ScInterpreter::ScForecast_Ets_CI()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 7 ) )
        return;

    // fake error as function is still empty
    PushNA();
}

void ScInterpreter::ScForecast_Ets_SE()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 5 ) )
        return;

    // fake error as function is still empty
    PushNA();
}

void ScInterpreter::ScForecast_Ets_ST()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 4, 7 ) )
        return;

    // fake error as function is still empty
    PushNA();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
