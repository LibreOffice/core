/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <interpre.hxx>
#include <global.hxx>
#include <scmatrix.hxx>
#include <formula/token.hxx>

#include <cmath>
#include <vector>


using ::std::vector;
using namespace formula;

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
 * Forecasting of a linear change in data over time (y=a+b*x) with
 * superimposed absolute or relative seasonal deviations, using additive
 * respectively mulitplicative Holt-Winters method.
 *
 * Class is set up to be used with Calc's FORECAST.ETS functions and with
 * chart extrapolations (not yet implemented).
 *
 * Initialisation and forecasting calculations are based on
 * Engineering Statistics Handbook, 6.4.3.5 Triple Exponential Smoothing
 * see "http://www.itl.nist.gov/div898/handbook/pmc/section4/pmc435.htm"
 *
 * Prediction Interval calculations are based on
 * Yar & Chatfield, Prediction Intervals for the Holt-Winters forecasting
 * procedure, International Journal of Forecasting, 1990, Vol.6, pp127-137
 *
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
    bool bAdditive;                     // additive method or multiplicative method
    // accuracy indicators
    double fMAE;                        // mean absolute error
    double fMASE;                       // mean absolute scaled error
    double fMSE;                        // mean squared error
    double fRMSE;                       // root mean squared error
    double fSMAPE;                      // symmetric mean absolute error

    // constants used in determining best fit for alpha, beta, gamma
    const double cfMinABCResolution = 0.001;  // minimum change of alpha, beta, gamma
    const SCSIZE cnstp = 1;                   // number of steps to forecast ahead

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
    void calcAccuracyIndicators();
    bool GetForecast( double fTarget, double& rForecast );

public:
    ScETSForecastCalculation( SCSIZE nSize );
    bool PreprocessDataRange( ScMatrixRef rMatX, ScMatrixRef rMatY, int& rSamplesInPeriod,
                              bool bDataCompletion, int nAggregation, ScMatrixRef rTMat,
                              bool bIsAdditive, sal_uInt16& rPreprocError  );

    bool GetForecastRange( ScMatrixRef rTMat, ScMatrixRef rFcMat );
    bool GetStatisticValue( ScMatrixRef rTypeMat, ScMatrixRef rStatMat );
    bool GetPredictionIntervals( ScMatrixRef rTMat, ScMatrixRef rPIMat, double fPILevel );
};

ScETSForecastCalculation::ScETSForecastCalculation( SCSIZE nSize )  :
    sRange( 0 ),
    sSmoothed( 0 ),
    sPeriodicalIdx( 0 ),
    sForecast( 0 ),
    bInitialised( false )
{
    nCount = nSize;
    sRange.reserve( nCount );
}

bool ScETSForecastCalculation::PreprocessDataRange( ScMatrixRef rMatX, ScMatrixRef rMatY, int& rSamplesInPeriod,
                                                    bool bDataCompletion, int nAggregation, ScMatrixRef rTMat,
                                                    bool bIsAdditive, sal_uInt16& rPreprocError  )
{
    bAdditive = bIsAdditive;

    // sRange needs to be sorted by fX
    for ( SCSIZE i = 0; i < nCount; i++ )
        sRange.push_back( DataPoint( rMatX->GetDouble( i ), rMatY->GetDouble( i ) ) );
    sort( sRange.begin(), sRange.end(), lcl_SortByX );

    if ( rTMat )
    {
        for ( SCSIZE i = 0; i < rTMat->GetElementCount(); i++ )
        {
            if ( rTMat->GetDouble( 0 ) < sRange[ 0 ].fX )
            {
                // target cannot be less than start of time(X)-range
                rPreprocError = errIllegalFPOperation; // #NUM!
                return false;
            }
        }
    }

    nSamplesinPeriod = rSamplesInPeriod;

    // minimum step is _the_ (exact) stepsize
    fStepSize = sRange[ 1 ].fX - sRange[ 0 ].fX;
    for ( SCSIZE i = 1; i < nCount; i++ )
    {
        double fStep = sRange[ i ].fX - sRange[ i - 1 ].fX;
        if ( fStep == 0.0 )
        {
            if ( nAggregation == 0 )
            {
                // call by ScForecast_Ets_SE(), no handling of doubles
                rPreprocError = errNoValue; // #VALUE!
                return false;
            }
            double fTmp = sRange[ i - 1 ].fY;
            SCSIZE nCounter = 1;
            switch ( nAggregation )
            {
                case 1 : // AVERAGE (default)
                case 7 : // SUM
                         while ( sRange[ i ].fX == sRange[ i - 1 ].fX )
                         {
                             fTmp += sRange[ i ].fY;
                             nCounter++;
                             sRange.erase( sRange.begin() + i );  //not efficient, but neccessary as sRange is ordered
                             --nCount;
                         }
                         sRange[ i - 1 ].fY = ( nAggregation == 1 ? fTmp / nCounter : fTmp );
                         break;

                case 2 : // COUNT
                case 3 : // COUNTA (no difference with COUNT as there are no non-numeric Y-values)
                         while ( sRange[ i ].fX == sRange[ i - 1 ].fX )
                         {
                             nCounter++;
                             sRange.erase( sRange.begin() + i );  //not efficient, but neccessary as sRange is ordered
                             --nCount;
                         }
                         sRange[ i - 1 ].fY = nCounter;
                         break;

                case 4 : // MAX
                         while ( sRange[ i ].fX == sRange[ i - 1 ].fX )
                         {
                             if ( sRange[ i ].fY > fTmp )
                                 fTmp = sRange[ i ].fY;
                             sRange.erase( sRange.begin() + i );  //not efficient, but neccessary as sRange is ordered
                             --nCount;
                         }
                         sRange[ i - 1 ].fY = fTmp;
                         break;

                case 5 : // MEDIAN
                         {
                             vector< double > sTmp;
                             sTmp.push_back( sRange[ i - 1 ].fY );
                             while ( sRange[ i ].fX == sRange[ i - 1 ].fX )
                             {
                                 sTmp.push_back( sRange[ i ].fY );
                                 nCounter++;
                                 sRange.erase( sRange.begin() + i );  //not efficient, but neccessary as sRange is ordered
                                 --nCount;
                             }
                             sort( sTmp.begin(), sTmp.end() );

                             if ( nCounter % 2 )
                                 sRange[ i - 1 ].fY = sTmp[ nCounter / 2 ];
                             else
                                 sRange[ i - 1 ].fY = ( sTmp[ nCounter / 2 ] + sTmp[ nCounter / 2 - 1 ] ) / 2.0;
                         }
                         break;

                case 6 : // MIN
                         while ( sRange[ i ].fX == sRange[ i - 1 ].fX )
                         {
                             if ( sRange[ i ].fY < fTmp )
                                 fTmp = sRange[ i ].fY;
                             sRange.erase( sRange.begin() + i );  //not efficient, but neccessary as sRange is ordered
                             --nCount;
                         }
                         sRange[ i - 1 ].fY = fTmp;
                         break;
            }
            fStep = ( i < nCount ? sRange[ i ].fX - sRange[ i - 1 ].fX : fStepSize );
        }
        if ( fStep < fStepSize )
            fStepSize = fStep;
    }

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
                        rPreprocError = errNoValue;
                        return false;
                    }
                }
            }
        }
    }
    if ( !initData() )
        return false;

    return true;
}

bool ScETSForecastCalculation::initData()
{
    // give variuous vectors size and initial value
    sSmoothed.resize( nCount );
    sTrend.resize( nCount );
    sPeriodicalIdx.resize( nCount );
    sForecast.resize( nCount );
    sForecast[ 0 ] = sRange[ 0 ];

    if ( prefillTrendData() )
    {
        if ( prefillPeriodicalIdx() )
        {
            if ( prefillSmoothedData() )
                return true;
        }
    }

    return false;
}

bool ScETSForecastCalculation::prefillTrendData()
{
    // we need at least 2 periods in the data range
    if ( nCount < 2 * nSamplesinPeriod )
        return false;

    double fSum = 0.0;
    for ( SCSIZE i = 0; i < nSamplesinPeriod; i++ )
        fSum += sRange[ i + nSamplesinPeriod ].fY - sRange[ i ].fY;
    double fTrend = fSum / ( double )( nSamplesinPeriod * nSamplesinPeriod );

    sTrend[ 0 ] = fTrend;
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
            SAL_WARN( "sc.core", "prefillPeriodicalIdx(), average of 0 will cause divide by zero error, quitting calculation" );
// TODO : set global error to DIV0 (how?)
            return false;
        }
    }

    double fI;
    for ( SCSIZE j = 0; j < nSamplesinPeriod; j++ )
    {
        fI = 0.0;
        for ( SCSIZE i = 0; i < nPeriods ; i++ )
            // (adjust average value for position within period)
            if ( bAdditive )
                fI += ( sRange[ i * nSamplesinPeriod + j ].fY -
                      ( pfPeriodAverage[ i ] + ( ( double )j - 0.5 * ( nSamplesinPeriod - 1 ) ) * sTrend[ 0 ] ) );
            else
                fI += ( sRange[ i * nSamplesinPeriod + j ].fY /
                      ( pfPeriodAverage[ i ] + ( ( double )j - 0.5 * ( nSamplesinPeriod - 1 ) ) * sTrend[ 0 ] ) );
        sPeriodicalIdx[ j ] = fI / nPeriods;
    }
    return true;
}

bool ScETSForecastCalculation::prefillSmoothedData()
{
    sSmoothed[ 0 ] = sRange[ 0 ].fY / sPeriodicalIdx[ 0 ];
    return true;
}

bool ScETSForecastCalculation::initCalc()
{
    if ( !bInitialised )
    {
        CalcAlphaBetaGamma();

        bInitialised = true;
        calcAccuracyIndicators();
    }
    return true;
}

void ScETSForecastCalculation::calcAccuracyIndicators()
{
    double fSumAbsErr     = 0.0;
    double fSumDivisor    = 0.0;
    double fSumErrSq      = 0.0;
    double fSumAbsPercErr = 0.0;

    for ( SCSIZE i = cnstp; i < nCount; i++ )
    {
        double fError = sForecast[ i ].fY - sRange[ i ].fY;
        fSumAbsErr    += fabs( fError );
        fSumErrSq     += fError * fError;
        fSumAbsPercErr += fabs( fError ) / ( fabs( sForecast[ i ].fY ) + fabs( sRange[ i ].fY ) );
    }

    for ( SCSIZE i = cnstp + 1; i < nCount; i++ )
        fSumDivisor += fabs( sRange[ i ].fY - sRange[ i - 1 ].fY );

    int nCalcCount = nCount - cnstp;
    fMAE   = fSumAbsErr / nCalcCount;
    fMASE  = fSumAbsErr / ( nCalcCount * fSumDivisor / ( nCalcCount - 1 ) );
    fMSE   = fSumErrSq / nCalcCount;
    fRMSE  = sqrt( fMSE );
    // SMAPE calculation may be different in Excel (i.e. result is half of our calculation)
    // in that case, remove '* 2.0' from code; both methods are being used
    fSMAPE = fSumAbsPercErr * 2.0 / nCalcCount;
}

void ScETSForecastCalculation::CalcAlphaBetaGamma()
{
    double f0, f1, f2;
    double fE0, fE2;

    f0 = 0.0;
    fAlpha = f0;
    CalcBetaGamma();
    refill();
    fE0 = fMSE;

    f2 = 1.0;
    fAlpha = f2;
    CalcBetaGamma();
    refill();
    fE2 = fMSE;

    f1 = 0.5;
    fAlpha = f1;
    CalcBetaGamma();
    refill();

    if ( fE0 == fMSE and fMSE == fE2 )
    {
        fAlpha = 0;
        CalcBetaGamma();
        refill();
        return;
    }
    while ( ( f2 - f1 ) > cfMinABCResolution )
    {
        if ( fE2 > fE0 )
        {
            f2 = f1;
            fE2 = fMSE;
            f1 = ( f0 + f1 ) / 2;
        }
        else
        {
            f0 = f1;
            fE0 = fMSE;
            f1 = ( f1 + f2 ) / 2;
        }
        fAlpha = f1;
        CalcBetaGamma();
        refill();
    }
    if ( fE2 > fE0 )
    {
        if ( fE0 < fMSE )
        {
            fAlpha = f0;
            CalcBetaGamma();
            refill();
        }
    }
    else
    {
        if ( fE2 < fMSE )
        {
            fAlpha = f2;
            CalcBetaGamma();
            refill();
        }
    }
    calcAccuracyIndicators();

    return;
}

void ScETSForecastCalculation::CalcBetaGamma()
{
    double f0, f1, f2;
    double fE0, fE2;

    f0 = 0.0;
    fBeta = f0;
    CalcGamma();
    refill();
    fE0 = fMSE;

    f2 = 1.0;
    fBeta = f2;
    CalcGamma();
    refill();
    fE2 = fMSE;

    f1 = 0.5;
    fBeta = f1;
    CalcGamma();
    refill();

    if ( fE0 == fMSE and fMSE == fE2 )
    {
        fBeta = 0;
        CalcGamma();
        refill();
        return;
    }
    while ( ( f2 - f1 ) > cfMinABCResolution )
    {
        if ( fE2 > fE0 )
        {
            f2 = f1;
            fE2 = fMSE;
            f1 = ( f0 + f1 ) / 2;
        }
        else
        {
            f0 = f1;
            fE0 = fMSE;
            f1 = ( f1 + f2 ) / 2;
        }
        fBeta = f1;
        CalcGamma();
        refill();
    }
    if ( fE2 > fE0 )
    {
        if ( fE0 < fMSE )
        {
            fBeta = f0;
            CalcGamma();
            refill();
        }
    }
    else
    {
        if ( fE2 < fMSE )
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
    fE0 = fMSE;

    f2 = 1.0;
    fGamma = f2;
    refill();
    fE2 = fMSE;

    f1 = 0.5;
    fGamma = f1;
    refill();

    if ( fE0 == fMSE and fMSE == fE2 )
    {
        fGamma = 0;
        refill();
        return;
    }
    while ( ( f2 - f1 ) > cfMinABCResolution )
    {
        if ( fE2 > fE0 )
        {
            f2 = f1;
            fE2 = fMSE;
            f1 = ( f0 + f1 ) / 2;
        }
        else
        {
            f0 = f1;
            fE0 = fMSE;
            f1 = ( f1 + f2 ) / 2;
        }
        fGamma = f1;
        refill();
    }
    if ( fE2 > fE0 )
    {
        if ( fE0 < fMSE )
        {
            fGamma = f0;
            refill();
        }
    }
    else
    {
        if ( fE2 < fMSE )
        {
            fGamma = f2;
            refill();
        }
    }
}

void ScETSForecastCalculation::refill()
{
    // refill sSmoothed, sTrend, sPeriodicalidx and sForecast with values
    // using the calculated fAlpha, fBeta, fGamma
    // forecast cnstp steps ahead

    for ( SCSIZE i = 1; i < nCount; i++ )
    {
        SCSIZE nIdx;
        if ( bAdditive )
            nIdx = ( i > nSamplesinPeriod ? i - nSamplesinPeriod : i );
        else
            nIdx = ( i >= nSamplesinPeriod ? i - nSamplesinPeriod : i );
        refillSmoothed( i, nIdx );
        refillTrend( i );
        refillPeriodicalIdx( i, nIdx );

        sForecast[ i ].fX = sRange[ i ].fX;
        if ( i >= cnstp )
        {
            double fVal;
            if ( bAdditive )
                fVal = sSmoothed[ i - 1 ] + ( double ) cnstp * sTrend[ i - 1 ] + sPeriodicalIdx[ nIdx ];
            else
                fVal = ( sSmoothed[ i - 1 ] + ( double ) cnstp * sTrend[ i - 1 ] ) * sPeriodicalIdx[ nIdx ];
            sForecast[ i ].fY = fVal;
        }
        else
            sForecast[ i ].fY = sSmoothed[ i ];

    }
    calcAccuracyIndicators();
}

void ScETSForecastCalculation::refillSmoothed( SCSIZE ni, SCSIZE nIdx )
{
    double fS;
    if ( bAdditive )
        fS = fAlpha * ( sRange[ ni ].fY - sPeriodicalIdx[ nIdx ] ) +
             ( 1 - fAlpha ) * ( sSmoothed[ ni - 1 ] + sTrend[ ni - 1 ] );
    else
        fS = fAlpha * ( sRange[ ni ].fY / sPeriodicalIdx[ nIdx ] ) +
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
    double fI;
    if ( bAdditive )
        fI = fBeta * ( sRange[ ni ].fY - sSmoothed[ ni ] ) +
             ( 1 - fBeta ) * sPeriodicalIdx[ nIdx ];
    else
        fI = fBeta * ( sRange[ ni ].fY / sSmoothed[ ni ] ) +
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
        if ( bAdditive )
            rForecast = sSmoothed[ nCount - 1 ] + n * sTrend[ nCount - 1 ] +
                        sPeriodicalIdx[ nCount - 1 - nSamplesinPeriod + ( n % nSamplesinPeriod ) ];
        else
            rForecast = ( sSmoothed[ nCount - 1 ] + n * sTrend[ nCount - 1 ] ) *
                        sPeriodicalIdx[ nCount - 1 - nSamplesinPeriod + ( n % nSamplesinPeriod ) ];

        if ( fInterpolate >= cfMinABCResolution )
        {
            double fInterpolateFactor = fInterpolate / fStepSize;
            double fFc_1;
            if ( bAdditive )
                fFc_1 = sSmoothed[ nCount - 1 ] + ( n + 1 ) * sTrend[ nCount - 1 ] +
                        sPeriodicalIdx[ nCount - 1 - nSamplesinPeriod + ( ( n + 1 ) % nSamplesinPeriod ) ];
            else
                fFc_1 = ( sSmoothed[ nCount - 1 ] + ( n + 1 ) * sTrend[ nCount - 1 ] ) *
                        sPeriodicalIdx[ nCount - 1 - nSamplesinPeriod + ( ( n + 1 ) % nSamplesinPeriod ) ];
            rForecast = fInterpolateFactor * fFc_1 + ( 1.0 - fInterpolateFactor ) * rForecast;
        }
    }
    return true;
}

bool ScETSForecastCalculation::GetForecastRange( ScMatrixRef rTMat, ScMatrixRef rFcMat )
{
    SCSIZE nC, nR;
    rTMat->GetDimensions( nC, nR );

    for ( SCSIZE i = 0; i < nR; i++ )
    {
        for ( SCSIZE j = 0; j < nC; j++ )
        {
            double fTarget = rTMat->GetDouble( j, i );
            double fForecast;
            if ( GetForecast( fTarget, fForecast ) )
                rFcMat->PutDouble( fForecast, j, i );
            else
                return false;
        }
    }
    return true;
}

bool ScETSForecastCalculation::GetStatisticValue( ScMatrixRef rTypeMat, ScMatrixRef rStatMat )
{
    if ( !initCalc() )
        return false;

    SCSIZE nC, nR;
    rTypeMat->GetDimensions( nC, nR );
    for ( SCSIZE i = 0; i < nR; i++ )
    {
        for ( SCSIZE j = 0; j < nC; j++ )
        {
            switch ( static_cast< int >( rTypeMat->GetDouble( j, i ) ) )
            {
                case 1 : // alpha value
                    rStatMat->PutDouble( fAlpha, j, i );
                    break;
                case 2 : // beta value
                    rStatMat->PutDouble( fBeta, j, i );
                    break;
                case 3 : // gamma value
                    rStatMat->PutDouble( fGamma, j, i );
                    break;
                case 4 : // MASE value, Mean Absolute Scaled Error
                    rStatMat->PutDouble( fMASE, j, i );
                    break;
                case 5 : // SMAPE value, Symmetric Mean Absolute Percentage Error
                    rStatMat->PutDouble( fSMAPE, j, i );
                    break;
                case 6 : // MAE value, Mean Absolute Error
                    rStatMat->PutDouble( fMAE, j, i );
                    break;
                case 7 : // RMSE value, Root Mean Squared Error
                    rStatMat->PutDouble( fRMSE, j, i );
                    break;
                case 8 : // step size
                    rStatMat->PutDouble( fStepSize, j, i );
                    break;
            }
        }
    }
    return true;
}

bool ScETSForecastCalculation::GetPredictionIntervals( ScMatrixRef rTMat, ScMatrixRef rPIMat, double fPILevel )
{
    if ( !initCalc() )
        return false;

    // fill array with forecasts
    SCSIZE nC, nR;
    rTMat->GetDimensions( nC, nR );
    double fMaxTarget = rTMat->GetDouble( 0, 0 );
    for ( SCSIZE i = 0; i < nR; i++ )
    {
        for ( SCSIZE j = 0; j < nC; j++ )
        {
            if ( fMaxTarget < rTMat->GetDouble( j, i ) )
                fMaxTarget = rTMat->GetDouble( j, i );
            if ( fPILevel < 1.0 )          // DUMMY INSTRUCTION!!!
                rPIMat->PutDouble( fMaxTarget, j, i );  // DUMMY INSTRUCTION!!!
        }
    }
/*
    vector< DataPoint > sFc;
    SCSIZE nFcCount = ( fMaxTarget - sRange[ nCount - 1 ].fX ) / fStepSize;
    if ( fmod( fMaxTarget - sRange[ nCount - 1 ].fX, fStepSize ) != 0.0 )
        nFcCount++;
    sFc.reserve( nFcCount );

    for ( SCSIZE i = 0; i < nFcCount; i++ )
    {
        double fTarget = sRange[ nCount - 1 ].fX + i * fStepSize;
        double fForecast;
        if ( GetForecast( fTarget, fForecast ) )
            sFc.push_back( DataPoint( fTarget, fForecast ) );
        else
            return false;
    }
*/

// TODO : add code here
    if ( bAdditive )
    {
        return true;
    }
    else
    {
        return false;
    }
}



void ScInterpreter::ScForecast_Ets( ScETSType eETSType )
{
    sal_uInt8 nParamCount = GetByte();
    switch ( eETSType )
    {
        case etsAdd :
        case etsMult :
        case etsStatAdd :
        case etsStatMult :
            if ( !MustHaveParamCount( nParamCount, 3, 6 ) )
                return;
            break;
        case etsPIAdd :
        case etsPIMult :
            if ( !MustHaveParamCount( nParamCount, 3, 7 ) )
            {
                return;
            }
            break;
        case etsSeason :
            if ( !MustHaveParamCount( nParamCount, 3, 5 ) )
                return;
            break;
    }

    int nAggregation;
    if ( eETSType != etsSeason )
    {
        if ( nParamCount >= 6 )
            nAggregation = static_cast< int >( GetDoubleWithDefault( 1.0 ) );
        else
            nAggregation = 1;
        if ( nAggregation < 1 || nAggregation > 6 )
        {
            PushIllegalParameter();
            return;
        }
    }
    else
        nAggregation = 0;

    bool bDataCompletion;
    if ( nParamCount >= 5 )
    {
        int nTemp = static_cast< int >( GetDoubleWithDefault( 1.0 ) );
        if ( nTemp == 0 || nTemp == 1 )
            bDataCompletion = nTemp;
        else
        {
            PushIllegalParameter();
            return;
        }
    }
    else
        bDataCompletion = true;
    int nSamplesinPeriod;
    if ( nParamCount >= 4 )
        nSamplesinPeriod = static_cast< int >( GetDoubleWithDefault( 1.0 ) );
    else
        nSamplesinPeriod = 1;

    // required arguments
    double fPILevel;
    if ( eETSType == etsPIAdd || eETSType == etsPIMult )
    {
        fPILevel = GetDouble();
        if ( fPILevel < 0 || fPILevel > 1 )
        {
            PushIllegalParameter();
            return;
        }
    }

    ScMatrixRef pTypeMat;
    if ( eETSType == etsStatAdd || eETSType == etsStatMult )
    {
        pTypeMat = GetMatrix();
        SCSIZE nC, nR;
        pTypeMat->GetDimensions( nC, nR );
        for ( SCSIZE i = 0; i < nR; i++ )
        {
            for ( SCSIZE j = 0; j < nC; j++ )
            {
                if ( static_cast< int >( pTypeMat->GetDouble( j, i ) ) < 1 ||
                     static_cast< int >( pTypeMat->GetDouble( j, i ) ) > 8 )
                {
                    PushIllegalParameter();
                    return;
                }
            }
        }
    }

    ScMatrixRef pMatX = GetMatrix();
    ScMatrixRef pMatY = GetMatrix();
    if ( !pMatX || !pMatY )
    {
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
        PushIllegalArgument();
        return;
    }

    ScMatrixRef pTMat;
    if ( eETSType != etsStatAdd && eETSType != etsStatMult )
    {
        pTMat = GetMatrix();
        if ( !pTMat )
        {
            PushIllegalArgument();
            return;
        }
    }

    if ( nSamplesinPeriod == 0 && eETSType != etsStatAdd && eETSType != etsStatMult )
    {
        // linear forecast
        double fForecast = LinearForecast( pMatX, pMatY, pTMat->GetDouble( 0 ) );
        PushDouble( fForecast );
        return;
    }
    else
    {
        if ( nSamplesinPeriod < 0 || nSamplesinPeriod > 8760 )
        {
            PushError( errIllegalFPOperation ); // NUM!
            return;
        }
    }

    ScETSForecastCalculation sETSCalc( pMatX->GetElementCount() );
    sal_uInt16 nErrorValue;
    bool bIsAdditive = ( eETSType == etsAdd || eETSType == etsSeason ||
                         eETSType == etsPIAdd || eETSType == etsStatAdd );
    if ( !sETSCalc.PreprocessDataRange( pMatX, pMatY, nSamplesinPeriod, bDataCompletion,
                                        nAggregation,
                                        ( eETSType != etsStatAdd && eETSType != etsStatMult ? pTMat : 0 ),
                                        bIsAdditive, nErrorValue ) )
    {
        PushError( nErrorValue );
        return;
    }

    switch ( eETSType )
    {
        case etsAdd    :
        case etsMult   :
        case etsSeason :
            {
                ScMatrixRef pFcMat;
                SCSIZE nC, nR;
                pTMat->GetDimensions( nC, nR );
                pFcMat = GetNewMat( nC, nR );
                if ( sETSCalc.GetForecastRange( pTMat, pFcMat ) )
                    PushMatrix( pFcMat );
                else
                    PushNA();
            }
            break;
        case etsPIAdd :
        case etsPIMult :
            {
                ScMatrixRef pPIMat;
                SCSIZE nC, nR;
                pTMat->GetDimensions( nC, nR );
                pPIMat = GetNewMat( nC, nR );
                if ( sETSCalc.GetPredictionIntervals( pTMat, pPIMat, fPILevel ) )
                    PushMatrix( pPIMat );
                else
                    PushNA();
            }
            break;
            break;
        case etsStatAdd  :
        case etsStatMult :
            {
                ScMatrixRef pStatMat;
                SCSIZE nC, nR;
                pTypeMat->GetDimensions( nC, nR );
                pStatMat = GetNewMat( nC, nR );
                if ( sETSCalc.GetStatisticValue( pTypeMat, pStatMat ) )
                    PushMatrix( pStatMat );
                else
                    PushNoValue();
            }
            break;
    }

    return;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
