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

#include "analysis.hxx"
#include "analysishelper.hxx"
#include <rtl/math.hxx>

using namespace sca::analysis;

double SAL_CALL AnalysisAddIn::getAmordegrc( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    double fCost, sal_Int32 nDate, sal_Int32 nFirstPer, double fRestVal,
    double fPer, double fRate, const css::uno::Any& rOB )
{
    if( nDate > nFirstPer || fRate <= 0.0 || fRestVal > fCost ||
        fCost <= 0.0 || fRestVal < 0 || fPer < 0 )
        throw css::lang::IllegalArgumentException();

    double fRet = GetAmordegrc( GetNullDate( xOpt ), fCost, nDate, nFirstPer, fRestVal, fPer, fRate, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getAmorlinc( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    double fCost, sal_Int32 nDate, sal_Int32 nFirstPer, double fRestVal,
    double fPer, double fRate, const css::uno::Any& rOB )
{
    if ( nDate > nFirstPer || fRate <= 0.0 || fRestVal > fCost ||
         fCost <= 0.0 || fRestVal < 0 || fPer < 0 )
        throw css::lang::IllegalArgumentException();

    double fRet = GetAmorlinc( GetNullDate( xOpt ), fCost, nDate, nFirstPer, fRestVal, fPer, fRate, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getAccrint( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nIssue, sal_Int32 /*nFirstInter*/, sal_Int32 nSettle, double fRate,
    const css::uno::Any &rVal, sal_Int32 nFreq, const css::uno::Any& rOB )
{
    double      fVal = aAnyConv.getDouble( xOpt, rVal, 1000.0 );

    if( fRate <= 0.0 || fVal <= 0.0 || CHK_Freq || nIssue >= nSettle )
        throw css::lang::IllegalArgumentException();

    double fRet = fVal * fRate * GetYearDiff( GetNullDate( xOpt ), nIssue, nSettle, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getAccrintm( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nIssue, sal_Int32 nSettle, double fRate, const css::uno::Any& rVal, const css::uno::Any& rOB )
{
    double      fVal = aAnyConv.getDouble( xOpt, rVal, 1000.0 );

    if( fRate <= 0.0 || fVal <= 0.0 || nIssue >= nSettle )
        throw css::lang::IllegalArgumentException();

    double fRet = fVal * fRate * GetYearDiff( GetNullDate( xOpt ), nIssue, nSettle, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getReceived( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fInvest, double fDisc, const css::uno::Any& rOB )
{
    if( fInvest <= 0.0 || fDisc <= 0.0 || nSettle >= nMat )
        throw css::lang::IllegalArgumentException();

    double fRet = fInvest / ( 1.0 - ( fDisc * GetYearDiff( GetNullDate( xOpt ), nSettle, nMat, getDateMode( xOpt, rOB ) ) ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getDisc( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fPrice, double fRedemp, const css::uno::Any& rOB )
{
    if( fPrice <= 0.0 || fRedemp <= 0.0 || nSettle >= nMat )
        throw css::lang::IllegalArgumentException();
    double fRet = ( 1.0 - fPrice / fRedemp ) / GetYearFrac( xOpt, nSettle, nMat, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getDuration( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fCoup, double fYield, sal_Int32 nFreq, const css::uno::Any& rOB )
{
    if( fCoup < 0.0 || fYield < 0.0 || CHK_Freq || nSettle >= nMat )
        throw css::lang::IllegalArgumentException();

    double fRet = GetDuration( GetNullDate( xOpt ),  nSettle, nMat, fCoup, fYield, nFreq, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getEffect( double fNominal, sal_Int32 nPeriods )
{
    if( nPeriods < 1 || fNominal <= 0.0 )
        throw css::lang::IllegalArgumentException();

    double  fPeriods = nPeriods;

    double fRet = pow( 1.0 + fNominal / fPeriods, fPeriods ) - 1.0;
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getCumprinc( double fRate, sal_Int32 nNumPeriods, double fVal,
    sal_Int32 nStartPer, sal_Int32 nEndPer, sal_Int32 nPayType )
{
    double fRmz, fKapZ;

    if( nStartPer < 1 || nEndPer < nStartPer || fRate <= 0.0 || nEndPer > nNumPeriods  || nNumPeriods <= 0 ||
        fVal <= 0.0 || ( nPayType != 0 && nPayType != 1 ) )
        throw css::lang::IllegalArgumentException();

    fRmz = GetRmz( fRate, nNumPeriods, fVal, 0.0, nPayType );

    fKapZ = 0.0;

    sal_uInt32  nStart = sal_uInt32( nStartPer );
    sal_uInt32  nEnd = sal_uInt32( nEndPer );

    if( nStart == 1 )
    {
        if( nPayType <= 0 )
            fKapZ = fRmz + fVal * fRate;
        else
            fKapZ = fRmz;

        nStart++;
    }

    for( sal_uInt32 i = nStart ; i <= nEnd ; i++ )
    {
        if( nPayType > 0 )
            fKapZ += fRmz - ( GetZw( fRate, double( i - 2 ), fRmz, fVal, 1 ) - fRmz ) * fRate;
        else
            fKapZ += fRmz - GetZw( fRate, double( i - 1 ), fRmz, fVal, 0 ) * fRate;
    }

    RETURN_FINITE( fKapZ );
}


double SAL_CALL AnalysisAddIn::getCumipmt( double fRate, sal_Int32 nNumPeriods, double fVal,
    sal_Int32 nStartPer, sal_Int32 nEndPer, sal_Int32 nPayType )
{
    double fRmz, fZinsZ;

    if( nStartPer < 1 || nEndPer < nStartPer || fRate <= 0.0 || nEndPer > nNumPeriods  || nNumPeriods <= 0 ||
        fVal <= 0.0 || ( nPayType != 0 && nPayType != 1 ) )
        throw css::lang::IllegalArgumentException();

    fRmz = GetRmz( fRate, nNumPeriods, fVal, 0.0, nPayType );

    fZinsZ = 0.0;

    sal_uInt32  nStart = sal_uInt32( nStartPer );
    sal_uInt32  nEnd = sal_uInt32( nEndPer );

    if( nStart == 1 )
    {
        if( nPayType <= 0 )
            fZinsZ = -fVal;

        nStart++;
    }

    for( sal_uInt32 i = nStart ; i <= nEnd ; i++ )
    {
        if( nPayType > 0 )
            fZinsZ += GetZw( fRate, double( i - 2 ), fRmz, fVal, 1 ) - fRmz;
        else
            fZinsZ += GetZw( fRate, double( i - 1 ), fRmz, fVal, 0 );
    }

    fZinsZ *= fRate;

    RETURN_FINITE( fZinsZ );
}


double SAL_CALL AnalysisAddIn::getPrice( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fRate, double fYield, double fRedemp, sal_Int32 nFreq,
    const css::uno::Any& rOB )
{
    if( fYield < 0.0 || fRate < 0.0 || fRedemp <= 0.0 || CHK_Freq || nSettle >= nMat )
        throw css::lang::IllegalArgumentException();

    double fRet = getPrice_( GetNullDate( xOpt ), nSettle, nMat, fRate, fYield, fRedemp, nFreq, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getPricedisc( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fDisc, double fRedemp, const css::uno::Any& rOB )
{
    if( fDisc <= 0.0 || fRedemp <= 0.0 || nSettle >= nMat )
        throw css::lang::IllegalArgumentException();

    double fRet = fRedemp * ( 1.0 - fDisc * GetYearDiff( GetNullDate( xOpt ), nSettle, nMat, getDateMode( xOpt, rOB ) ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getPricemat( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, double fRate, double fYield, const css::uno::Any& rOB )
{
    if( fRate < 0.0 || fYield < 0.0 || nSettle >= nMat )
        throw css::lang::IllegalArgumentException();

    sal_Int32   nNullDate = GetNullDate( xOpt );
    sal_Int32   nBase = getDateMode( xOpt, rOB );

    double      fIssMat = GetYearFrac( nNullDate, nIssue, nMat, nBase );
    double      fIssSet = GetYearFrac( nNullDate, nIssue, nSettle, nBase );
    double      fSetMat = GetYearFrac( nNullDate, nSettle, nMat, nBase );

    double      fRet = 1.0 + fIssMat * fRate;
    fRet /= 1.0 + fSetMat * fYield;
    fRet -= fIssSet * fRate;
    fRet *= 100.0;

    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getMduration( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fCoup, double fYield, sal_Int32 nFreq, const css::uno::Any& rOB )
{
    if( fCoup < 0.0 || fYield < 0.0 || CHK_Freq )
        throw css::lang::IllegalArgumentException();

    double      fRet = GetDuration( GetNullDate( xOpt ),  nSettle, nMat, fCoup, fYield, nFreq, getDateMode( xOpt, rOB ) );
    fRet /= 1.0 + ( fYield / double( nFreq ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getNominal( double fRate, sal_Int32 nPeriods )
{
    if( fRate <= 0.0 || nPeriods < 0 )
        throw css::lang::IllegalArgumentException();

    double  fPeriods = nPeriods;
    double fRet = ( pow( fRate + 1.0, 1.0 / fPeriods ) - 1.0 ) * fPeriods;
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getDollarfr( double fDollarDec, sal_Int32 nFrac )
{
    if( nFrac <= 0 )
        throw css::lang::IllegalArgumentException();

    double  fInt;
    double  fFrac = nFrac;

    double  fRet = modf( fDollarDec, &fInt );

    fRet *= fFrac;

    fRet *= pow( 10.0, -ceil( log10( fFrac ) ) );

    fRet += fInt;

    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getDollarde( double fDollarFrac, sal_Int32 nFrac )
{
    if( nFrac <= 0 )
        throw css::lang::IllegalArgumentException();

    double  fInt;
    double  fFrac = nFrac;

    double  fRet = modf( fDollarFrac, &fInt );

    fRet /= fFrac;

    fRet *= pow( 10.0, ceil( log10( fFrac ) ) );

    fRet += fInt;

    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getYield( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fCoup, double fPrice, double fRedemp, sal_Int32 nFreq, const css::uno::Any& rOB )
{
    if( fCoup < 0.0 || fPrice <= 0.0 || fRedemp <= 0.0 || CHK_Freq || nSettle >= nMat )
        throw css::lang::IllegalArgumentException();

    double fRet = getYield_( GetNullDate( xOpt ), nSettle, nMat, fCoup, fPrice, fRedemp, nFreq, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getYielddisc( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fPrice, double fRedemp, const css::uno::Any& rOB )
{
    if( fPrice <= 0.0 || fRedemp <= 0.0 || nSettle >= nMat )
        throw css::lang::IllegalArgumentException();

    sal_Int32   nNullDate = GetNullDate( xOpt );

    double fRet = ( fRedemp / fPrice ) - 1.0;
    fRet /= GetYearFrac( nNullDate, nSettle, nMat, getDateMode( xOpt, rOB ) );

    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getYieldmat( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, double fRate, double fPrice, const css::uno::Any& rOB )
{
    if( fPrice <= 0.0 || fRate < 0.0 || nSettle >= nMat || nSettle < nIssue)
        throw css::lang::IllegalArgumentException();

    double fRet = GetYieldmat( GetNullDate( xOpt ),  nSettle, nMat, nIssue, fRate, fPrice, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getTbilleq( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fDisc )
{
    nMat++;

    sal_Int32   nDiff = GetDiffDate360( xOpt, nSettle, nMat, true );

    if( fDisc <= 0.0 || nSettle >= nMat || nDiff > 360 )
        throw css::lang::IllegalArgumentException();

    double fRet = ( 365 * fDisc ) / ( 360 - ( fDisc * double( nDiff ) ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getTbillprice( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fDisc )
{
    if( fDisc <= 0.0 || nSettle > nMat )
        throw css::lang::IllegalArgumentException();

    nMat++;

    double  fFraction = GetYearFrac( xOpt, nSettle, nMat, 0 );  // method: USA 30/360

    double  fDummy;
    if( modf( fFraction, &fDummy ) == 0.0 )
        throw css::lang::IllegalArgumentException();

    double fRet = 100.0 * ( 1.0 - fDisc * fFraction );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getTbillyield( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fPrice )
{
    sal_Int32   nDiff = GetDiffDate360( xOpt, nSettle, nMat, true );
    nDiff++;

    if( fPrice <= 0.0 || nSettle >= nMat || nDiff > 360 )
        throw css::lang::IllegalArgumentException();

    double      fRet = 100.0;
    fRet /= fPrice;
    fRet--;
    fRet /= double( nDiff );
    fRet *= 360.0;

    RETURN_FINITE( fRet );
}

// Encapsulation violation: We *know* that GetOddfprice() always
// throws.

SAL_WNOUNREACHABLE_CODE_PUSH

double SAL_CALL AnalysisAddIn::getOddfprice( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, sal_Int32 nFirstCoup,
    double fRate, double fYield, double fRedemp, sal_Int32 nFreq, const css::uno::Any& rOB )
{
    if( fRate < 0.0 || fYield < 0.0 || CHK_Freq || nMat <= nFirstCoup || nFirstCoup <= nSettle || nSettle <= nIssue )
        throw css::lang::IllegalArgumentException();

    double fRet = GetOddfprice( GetNullDate( xOpt ), nSettle, nMat, nIssue, nFirstCoup, fRate, fYield, fRedemp, nFreq, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}

SAL_WNOUNREACHABLE_CODE_POP

// Encapsulation violation: We *know* that Getoddfyield() always
// throws.

SAL_WNOUNREACHABLE_CODE_PUSH

double SAL_CALL AnalysisAddIn::getOddfyield( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, sal_Int32 nFirstCoup,
    double fRate, double fPrice, double fRedemp, sal_Int32 nFreq, const css::uno::Any& rOB )
{
    if( fRate < 0.0 || fPrice <= 0.0 || CHK_Freq || nMat <= nFirstCoup || nFirstCoup <= nSettle || nSettle <= nIssue )
        throw css::lang::IllegalArgumentException();

    double fRet = GetOddfyield( GetNullDate( xOpt ), nSettle, nMat, nIssue, nFirstCoup, fRate, fPrice, fRedemp, nFreq,
                        getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}

SAL_WNOUNREACHABLE_CODE_POP

double SAL_CALL AnalysisAddIn::getOddlprice( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nLastInterest,
    double fRate, double fYield, double fRedemp, sal_Int32 nFreq, const css::uno::Any& rOB )
{
    if( fRate <= 0.0 || fYield < 0.0 || fRedemp <= 0.0 || CHK_Freq || nMat <= nSettle || nSettle <= nLastInterest )
        throw css::lang::IllegalArgumentException();

    double fRet = GetOddlprice( GetNullDate( xOpt ), nSettle, nMat, nLastInterest, fRate, fYield, fRedemp, nFreq,
                        getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getOddlyield( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nLastInterest,
    double fRate, double fPrice, double fRedemp, sal_Int32 nFreq, const css::uno::Any& rOB )
{
    if( fRate <= 0.0 || fPrice <= 0.0 || fRedemp <= 0.0 || CHK_Freq || nMat <= nSettle || nSettle <= nLastInterest )
        throw css::lang::IllegalArgumentException();

    double fRet = GetOddlyield( GetNullDate( xOpt ), nSettle, nMat, nLastInterest, fRate, fPrice, fRedemp, nFreq,
                        getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}

// XIRR helper functions

#define V_(i) (rValues.Get(i))
#define D_(i) (rDates.Get(i))

/** Calculates the resulting amount for the passed interest rate and the given XIRR parameters. */
static double lcl_sca_XirrResult( const ScaDoubleList& rValues, const ScaDoubleList& rDates, double fRate )
{
    /*  V_0 ... V_n = input values.
        D_0 ... D_n = input dates.
        R           = input interest rate.

        r   := R+1
        E_i := (D_i-D_0) / 365

                    n    V_i                n    V_i
        f(R)  =  SUM   -------  =  V_0 + SUM   ------- .
                   i=0  r^E_i              i=1  r^E_i
    */
    double D_0 = D_(0);
    double r = fRate + 1.0;
    double fResult = V_(0);
    for( sal_uInt32 i = 1, nCount = rValues.Count(); i < nCount; ++i )
        fResult += V_(i) / pow( r, (D_(i) - D_0) / 365.0 );
    return fResult;
}

/** Calculates the first derivation of lcl_sca_XirrResult(). */
static double lcl_sca_XirrResult_Deriv1( const ScaDoubleList& rValues, const ScaDoubleList& rDates, double fRate )
{
    /*  V_0 ... V_n = input values.
        D_0 ... D_n = input dates.
        R           = input interest rate.

        r   := R+1
        E_i := (D_i-D_0) / 365

                             n    V_i
        f'(R)  =  [ V_0 + SUM   ------- ]'
                            i=1  r^E_i

                         n           V_i                 n    E_i V_i
               =  0 + SUM   -E_i ----------- r'  =  - SUM   ----------- .
                        i=1       r^(E_i+1)             i=1  r^(E_i+1)
    */
    double D_0 = D_(0);
    double r = fRate + 1.0;
    double fResult = 0.0;
    for( sal_uInt32 i = 1, nCount = rValues.Count(); i < nCount; ++i )
    {
        double E_i = (D_(i) - D_0) / 365.0;
        fResult -= E_i * V_(i) / pow( r, E_i + 1.0 );
    }
    return fResult;
}

#undef V_
#undef D_

// XIRR calculation

double SAL_CALL AnalysisAddIn::getXirr(
    const css::uno::Reference< css::beans::XPropertySet >& xOpt, const css::uno::Sequence< css::uno::Sequence< double > >& rValues, const css::uno::Sequence< css::uno::Sequence< sal_Int32 > >& rDates, const css::uno::Any& rGuessRate )
{
    ScaDoubleList aValues, aDates;
    aValues.Append( rValues );
    aDates.Append( rDates );

    if( (aValues.Count() < 2) || (aValues.Count() != aDates.Count()) )
        throw css::lang::IllegalArgumentException();

    // result interest rate, initialized with passed guessed rate, or 10%
    double fResultRate = aAnyConv.getDouble( xOpt, rGuessRate, 0.1 );
    if( fResultRate <= -1 )
        throw css::lang::IllegalArgumentException();

    // maximum epsilon for end of iteration
    static const double fMaxEps = 1e-10;
    // maximum number of iterations
    static const sal_Int32 nMaxIter = 50;

    // Newton's method - try to find a fResultRate, so that lcl_sca_XirrResult() returns 0.
    sal_Int32 nIter = 0;
    double fResultValue;
    sal_Int32 nIterScan = 0;
    bool bContLoop = false;
    bool bResultRateScanEnd = false;

    // First the inner while-loop will be executed using the default Value fResultRate
    // or the user guessed fResultRate if those do not deliver a solution for the
    // Newton's method then the range from -0.99 to +0.99 will be scanned with a
    // step size of 0.01 to find fResultRate's value which can deliver a solution
    do
    {
        if (nIterScan >=1)
            fResultRate = -0.99 + (nIterScan -1)* 0.01;
        do
        {
            fResultValue = lcl_sca_XirrResult( aValues, aDates, fResultRate );
            double fNewRate = fResultRate - fResultValue / lcl_sca_XirrResult_Deriv1( aValues, aDates, fResultRate );
            double fRateEps = fabs( fNewRate - fResultRate );
            fResultRate = fNewRate;
            bContLoop = (fRateEps > fMaxEps) && (fabs( fResultValue ) > fMaxEps);
        }
        while( bContLoop && (++nIter < nMaxIter) );
        nIter = 0;
        if (  ::rtl::math::isNan(fResultRate)  || ::rtl::math::isInf(fResultRate)
            ||::rtl::math::isNan(fResultValue) || ::rtl::math::isInf(fResultValue))
            bContLoop = true;

        ++nIterScan;
        bResultRateScanEnd = (nIterScan >= 200);
    }
    while(bContLoop && !bResultRateScanEnd);

    if( bContLoop )
        throw css::lang::IllegalArgumentException();
    RETURN_FINITE( fResultRate );
}


double SAL_CALL AnalysisAddIn::getXnpv(
    double fRate, const css::uno::Sequence< css::uno::Sequence< double > >& rValues, const css::uno::Sequence< css::uno::Sequence< sal_Int32 > >& rDates )
{
    ScaDoubleList aValList;
    ScaDoubleList aDateList;

    aValList.Append( rValues );
    aDateList.Append( rDates );

    sal_Int32           nNum = aValList.Count();

    if( nNum != sal_Int32( aDateList.Count() ) || nNum < 2 )
        throw css::lang::IllegalArgumentException();

    double              fRet = 0.0;
    double              fNull = aDateList.Get( 0 );
    fRate++;

    for( sal_Int32 i = 0 ; i < nNum ; i++ )
        fRet += aValList.Get( i ) / ( pow( fRate, ( aDateList.Get( i ) - fNull ) / 365.0 ) );

    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getIntrate( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fInvest, double fRedemp, const css::uno::Any& rOB )
{
    if( fInvest <= 0.0 || fRedemp <= 0.0 || nSettle >= nMat )
        throw css::lang::IllegalArgumentException();

    double fRet = ( ( fRedemp / fInvest ) - 1.0 ) / GetYearDiff( GetNullDate( xOpt ), nSettle, nMat, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getCoupncd( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const css::uno::Any& rOB )
{
    double fRet = GetCoupncd( GetNullDate( xOpt ), nSettle, nMat, nFreq, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getCoupdays( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const css::uno::Any& rOB )
{
    double fRet = GetCoupdays( GetNullDate( xOpt ), nSettle, nMat, nFreq, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getCoupdaysnc( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const css::uno::Any& rOB )
{
    double fRet = GetCoupdaysnc( GetNullDate( xOpt ), nSettle, nMat, nFreq, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getCoupdaybs( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const css::uno::Any& rOB )
{
    double fRet = GetCoupdaybs( GetNullDate( xOpt ), nSettle, nMat, nFreq, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getCouppcd( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const css::uno::Any& rOB )
{
    double fRet = GetCouppcd( GetNullDate( xOpt ), nSettle, nMat, nFreq, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getCoupnum( const css::uno::Reference< css::beans::XPropertySet >& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const css::uno::Any& rOB )
{
    double fRet = GetCoupnum( GetNullDate( xOpt ), nSettle, nMat, nFreq, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getFvschedule( double fPrinc, const css::uno::Sequence< css::uno::Sequence< double > >& rSchedule )
{
    ScaDoubleList aSchedList;

    aSchedList.Append( rSchedule );

    for( sal_uInt32 i = 0; i < aSchedList.Count(); ++i )
        fPrinc *= 1.0 + aSchedList.Get(i);

    RETURN_FINITE( fPrinc );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
