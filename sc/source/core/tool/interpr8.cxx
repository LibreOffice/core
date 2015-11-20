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
 * Forecasting of a linear change in data over time (y=a+b*x) with
 * superimposed relative seasonal deviations.
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
    void calcAccuracyIndicators( bool bShowOutput );

    public:
    ScETSForecastCalculation() : sRange( 0 ), sSmoothed( 0 ), sPeriodicalIdx( 0 ), sForecast( 0 ), bInitialised( false ) {};
    bool PreprocessDataRange( ScMatrixRef rMatX, ScMatrixRef rMatY, int& rSamplesInPeriod,
                              bool bDataCompletion, int nAggregation, double fTarget, int& rPreprocError  );
    bool GetForecast( double fTarget, double& rForecast );

    // for charts and array output, not yet in use
    bool CalcForecastRange( double fTargetFrom, double fTargetTill );
};

bool ScETSForecastCalculation::PreprocessDataRange( ScMatrixRef rMatX, ScMatrixRef rMatY, int& rSamplesInPeriod,
                                   bool bDataCompletion, int nAggregation, double fTarget, int& rPreprocError  )
{
    sRange.resize( rMatX->GetElementCount() );
    nCount = sRange.size();
    SAL_DEBUG( "PreprocessDataRange(), sRange size = " << nCount );
    nSamplesinPeriod = rSamplesInPeriod;

    // MatX needs to be sorted
    for ( SCSIZE i = 0; i < nCount; i++ )
    {
        sRange[ i ].fX = rMatX->GetDouble( i );
        sRange[ i ].fY = rMatY->GetDouble( i );
    }
    sort( sRange.begin(), sRange.end(), lcl_SortByX );

    if ( fTarget < sRange[ 0 ].fX )
    {
        // target cannot be less than start of time(X)-range
        //SAL_DEBUG( "PreprocessDataRange(), target cannot be less than start of time(X)-range" );
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
    SAL_DEBUG( "PreprocessDataRange(), fStepSize = " << fStepSize );
    // step must be constant (or gap multiple of step)
    bool bHasGap = false;
    bool bHasDoubles = false;
    for ( SCSIZE i = 1; i < nCount; i++ )
    {
        double fStep = sRange[ i ].fX - sRange[ i - 1 ].fX;
        if ( fStep != fStepSize )
        {
            if ( fmod( fStep, fStepSize ) != 0.0 )
            {
                // step not constant nor multiple of fStepSize in case of gaps
                //SAL_DEBUG( "PreprocessDataRange(), step not constant nor multiple of minimum step" );
                rPreprocError = errNoValue;
                return false;
            }
            bHasGap = true;
            if ( fStep == 0.0 )
                bHasDoubles = true;
        }
    }
    if ( bHasDoubles )
    {
// TODO: add code
        switch ( nAggregation )
        {
            case 1 : break;
            default : break;
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
                        //SAL_DEBUG( "PreprocessDataRange(), maximum of 30% missing values exceeded" );
                        rPreprocError = errNoValue;
                        return false;
                    }
                }
            }
        }
        //SAL_DEBUG( "PreprocessDataRange(), " << nMissingXCount << " datapoints inserted (" <<
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
    {
        SAL_DEBUG( "prefillTrendData(), " << nCount << " datapoints, but we need 2 seasons at least, which is  " << ( 2 * nSamplesinPeriod ) );
        return false;
    }

    double fSum = 0.0;
    for ( SCSIZE i = 0; i < nSamplesinPeriod; i++ )
        fSum += sRange[ i + nSamplesinPeriod ].fY - sRange[ i ].fY;
    double fTrend = fSum / ( double )( nSamplesinPeriod * nSamplesinPeriod );

    sTrend.resize( nCount );
    sTrend[ 0 ] = fTrend;
    SAL_DEBUG( "prefillTrendData(), initial sTrend  = " << fTrend );
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
            return false;
        }
    }

    sPeriodicalIdx.resize( nCount );
    double fI;
    for ( SCSIZE j = 0; j < nSamplesinPeriod; j++ )
    {
        fI = 0.0;
        for ( SCSIZE i = 0; i < nPeriods ; i++ )
            // (adjust average value for position within period)
            fI += ( sRange[ i * nSamplesinPeriod + j ].fY /
                    ( pfPeriodAverage[ i ] + ( ( double )j - 0.5 * ( nSamplesinPeriod - 1 ) ) * sTrend[ 0 ] ) );
        sPeriodicalIdx[ j ] = fI / nPeriods;
        SAL_DEBUG( "prefillPeriodicalIdx(), initial periodical index[" << j << "] = " << sPeriodicalIdx[ j ] );
    }
    return true;
}

bool ScETSForecastCalculation::prefillSmoothedData()
{
    sSmoothed.resize( nCount );
    sSmoothed[ 0 ] = sRange[ 0 ].fY / sPeriodicalIdx[ 0 ];
    SAL_DEBUG( "prefillSmoothedData(), initial sSmoothed[ 0 ] = " << sSmoothed[ 0 ] );
    return true;
}

bool ScETSForecastCalculation::initCalc()
{
    if ( !bInitialised )
    {
        CalcAlphaBetaGamma();

        bInitialised = true;
        for ( SCSIZE i = 0; i < nCount; i++ )
        {
            SAL_DEBUG( "initCalc(), x = " << sForecast[ i ].fX << ", smoothed= " << sSmoothed[ i ] <<
                       ", trend= " << sTrend[ i ] << ", per.idx= " << sPeriodicalIdx[ i ] <<
                       ", forecast= " << sForecast[ i].fY );
        }
    }
    calcAccuracyIndicators( true );

    return true;
}

void ScETSForecastCalculation::calcAccuracyIndicators( bool bShowOutput )
{
    double fSumErr       = 0.0;
    double fSumAbsErr    = 0.0;
    double fSumAbsRelErr = 0.0;
    double fSumErrSq     = 0.0;

    for ( SCSIZE i = cnstp; i < nCount; i++ )
    {
        double fError = sForecast[ i ].fY - sRange[ i ].fY;
        fSumErr       += fError;
        fSumAbsErr    += fabs( fError );
        fSumAbsRelErr += fabs( fError / sRange[ i ].fY );
        fSumErrSq     += fError * fError;
    }

    fBias = fSumErr / ( nCount - cnstp );
    fMad  = fSumAbsErr / ( nCount - cnstp );
    fMare = fSumAbsRelErr / ( nCount - cnstp );
    fMse  = fSumErrSq / ( nCount - cnstp );
    fSae  = fSumAbsErr;

    if ( bShowOutput  )
    {
        SAL_DEBUG( "calcAccuracyIndicators(), fAlpha = " << fAlpha << ", fBeta = " << fBeta << ", fGamma = " << fGamma );
        SAL_DEBUG( "calcAccuracyIndicators(), fBias= " << fBias << ", fMad= " << fMad << ", fMare= " << fMare << ", fMse= " << fMse << ", fSae= " << fSae );
    }
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
        fBeta = f1;
        CalcGamma();
        refill();
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

void ScETSForecastCalculation::refill()
{
    // refill sSmoothed, sTrend, sPeriodicalidx and sForecast with values
    // using the calculated fAlpha, fBeta, fGamma
    // forecast cnstp steps ahead

    for ( SCSIZE i = 1; i < nCount; i++ )
    {
        SCSIZE nIdx = i - ( i >= nSamplesinPeriod ? nSamplesinPeriod : 0 );
        refillSmoothed( i, nIdx );
        refillTrend( i );
        refillPeriodicalIdx( i, nIdx );

        sForecast[ i ].fX = sRange[ i ].fX;
        if ( i >= cnstp )
        {
            double fVal = ( sSmoothed[ i - 1 ] + ( double ) cnstp * sTrend[ i - 1 ] ) * sPeriodicalIdx[ nIdx ];
            sForecast[ i ].fY = fVal;
        }
        else
            sForecast[ i ].fY = sSmoothed[ i ];

    }
    calcAccuracyIndicators( false );
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
            sForecast.push_back( DataPoint( fX, fY ) );
        }
    }

    return true;
}


void ScInterpreter::ScForecast_Ets()
{
    sal_uInt8 nParamCount = GetByte();
    if ( !MustHaveParamCount( nParamCount, 3, 6 ) )
        return;

    int nAggregation;
    if ( nParamCount == 6 )
    {
        if ( IsMissing() )
        {
            nAggregation = 1;
            Pop();
        }
        else
            nAggregation = ( int ) GetDouble();
    }
    else
        nAggregation = 1;

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
                PushIllegalParameter();
                return;
            }
        }
    }
    else
        bDataCompletion = true;
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

    // required arguments
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
    double fTarget = GetDouble();

    if ( nSamplesinPeriod == 0 )
    {
        // linear forecast
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
            PushError( errIllegalFPOperation ); // NUM!
            return;
        }
    }

    ScETSForecastCalculation sETSCalc;
    int nErrorValue;
    if ( !sETSCalc.PreprocessDataRange( pMatX, pMatY, nSamplesinPeriod, bDataCompletion, nAggregation, fTarget, nErrorValue ) )
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
