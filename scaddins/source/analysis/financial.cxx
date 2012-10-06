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



double SAL_CALL AnalysisAddIn::getAmordegrc( constREFXPS& xOpt,
    double fCost, sal_Int32 nDate, sal_Int32 nFirstPer, double fRestVal,
    double fPer, double fRate, const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( nDate > nFirstPer || fRate <= 0.0 || fRestVal > fCost )
        THROW_IAE;

    double fRet = GetAmordegrc( GetNullDate( xOpt ), fCost, nDate, nFirstPer, fRestVal, fPer, fRate, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getAmorlinc( constREFXPS& xOpt,
    double fCost, sal_Int32 nDate, sal_Int32 nFirstPer, double fRestVal,
    double fPer, double fRate, const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( nDate > nFirstPer || fRate <= 0.0 || fRestVal > fCost )
        THROW_IAE;

    double fRet = GetAmorlinc( GetNullDate( xOpt ), fCost, nDate, nFirstPer, fRestVal, fPer, fRate, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getAccrint( constREFXPS& xOpt,
    sal_Int32 nIssue, sal_Int32 /*nFirstInter*/, sal_Int32 nSettle, double fRate,
    const ANY &rVal, sal_Int32 nFreq, const ANY& rOB ) THROWDEF_RTE_IAE
{
    double      fVal = aAnyConv.getDouble( xOpt, rVal, 1000.0 );

    if( fRate <= 0.0 || fVal <= 0.0 || CHK_Freq || nIssue >= nSettle )
        THROW_IAE;

    double fRet = fVal * fRate * GetYearDiff( GetNullDate( xOpt ), nIssue, nSettle, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getAccrintm( constREFXPS& xOpt,
    sal_Int32 nIssue, sal_Int32 nSettle, double fRate, const ANY& rVal, const ANY& rOB ) THROWDEF_RTE_IAE
{
    double      fVal = aAnyConv.getDouble( xOpt, rVal, 1000.0 );

    if( fRate <= 0.0 || fVal <= 0.0 || nIssue >= nSettle )
        THROW_IAE;

    double fRet = fVal * fRate * GetYearDiff( GetNullDate( xOpt ), nIssue, nSettle, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getReceived( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fInvest, double fDisc, const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( fInvest <= 0.0 || fDisc <= 0.0 )
        THROW_IAE;

    double fRet = fInvest / ( 1.0 - ( fDisc * GetYearDiff( GetNullDate( xOpt ), nSettle, nMat, getDateMode( xOpt, rOB ) ) ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getDisc( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fPrice, double fRedemp, const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( fPrice <= 0.0 || fRedemp <= 0.0 || nSettle >= nMat )
        THROW_IAE;
    double fRet = ( 1.0 - fPrice / fRedemp ) / GetYearFrac( xOpt, nSettle, nMat, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getDuration( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fCoup, double fYield, sal_Int32 nFreq, const ANY& rOB )
    THROWDEF_RTE_IAE
{
    if( fCoup < 0.0 || fYield < 0.0 || CHK_Freq || nSettle >= nMat )
        THROW_IAE;

    double fRet = GetDuration( GetNullDate( xOpt ),  nSettle, nMat, fCoup, fYield, nFreq, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getEffect( double fNominal, sal_Int32 nPeriods ) THROWDEF_RTE_IAE
{
    if( nPeriods < 1 || fNominal <= 0.0 )
        THROW_IAE;

    double  fPeriods = nPeriods;

    double fRet = pow( 1.0 + fNominal / fPeriods, fPeriods ) - 1.0;
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getCumprinc( double fRate, sal_Int32 nNumPeriods, double fVal,
    sal_Int32 nStartPer, sal_Int32 nEndPer, sal_Int32 nPayType ) THROWDEF_RTE_IAE
{
    double fRmz, fKapZ;

    if( nStartPer < 1 || nEndPer < nStartPer || fRate <= 0.0 || nEndPer > nNumPeriods  || nNumPeriods <= 0 ||
        fVal <= 0.0 || ( nPayType != 0 && nPayType != 1 ) )
        THROW_IAE;

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
    sal_Int32 nStartPer, sal_Int32 nEndPer, sal_Int32 nPayType ) THROWDEF_RTE_IAE
{
    double fRmz, fZinsZ;

    if( nStartPer < 1 || nEndPer < nStartPer || fRate <= 0.0 || nEndPer > nNumPeriods  || nNumPeriods <= 0 ||
        fVal <= 0.0 || ( nPayType != 0 && nPayType != 1 ) )
        THROW_IAE;

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


double SAL_CALL AnalysisAddIn::getPrice( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fRate, double fYield, double fRedemp, sal_Int32 nFreq,
    const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( fYield < 0.0 || fRate < 0.0 || fRedemp <= 0.0 || CHK_Freq || nSettle >= nMat )
        THROW_IAE;

    double fRet = getPrice_( GetNullDate( xOpt ), nSettle, nMat, fRate, fYield, fRedemp, nFreq, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getPricedisc( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fDisc, double fRedemp, const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( fDisc <= 0.0 || fRedemp <= 0.0 || nSettle >= nMat )
        THROW_IAE;

    double fRet = fRedemp * ( 1.0 - fDisc * GetYearDiff( GetNullDate( xOpt ), nSettle, nMat, getDateMode( xOpt, rOB ) ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getPricemat( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, double fRate, double fYield, const ANY& rOB )
    THROWDEF_RTE_IAE
{
    if( fRate < 0.0 || fYield < 0.0 || nSettle >= nMat )
        THROW_IAE;

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


double SAL_CALL AnalysisAddIn::getMduration( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fCoup, double fYield, sal_Int32 nFreq, const ANY& rOB )
    THROWDEF_RTE_IAE
{
    if( fCoup < 0.0 || fYield < 0.0 || CHK_Freq )
        THROW_IAE;

    double      fRet = GetDuration( GetNullDate( xOpt ),  nSettle, nMat, fCoup, fYield, nFreq, getDateMode( xOpt, rOB ) );
    fRet /= 1.0 + ( fYield / double( nFreq ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getNominal( double fRate, sal_Int32 nPeriods ) THROWDEF_RTE_IAE
{
    if( fRate <= 0.0 || nPeriods < 0 )
        THROW_IAE;

    double  fPeriods = nPeriods;
    double fRet = ( pow( fRate + 1.0, 1.0 / fPeriods ) - 1.0 ) * fPeriods;
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getDollarfr( double fDollarDec, sal_Int32 nFrac ) THROWDEF_RTE_IAE
{
    if( nFrac <= 0 )
        THROW_IAE;

    double  fInt;
    double  fFrac = nFrac;

    double  fRet = modf( fDollarDec, &fInt );

    fRet *= fFrac;

    fRet *= pow( 10.0, -ceil( log10( fFrac ) ) );

    fRet += fInt;

    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getDollarde( double fDollarFrac, sal_Int32 nFrac ) THROWDEF_RTE_IAE
{
    if( nFrac <= 0 )
        THROW_IAE;

    double  fInt;
    double  fFrac = nFrac;

    double  fRet = modf( fDollarFrac, &fInt );

    fRet /= fFrac;

    fRet *= pow( 10.0, ceil( log10( fFrac ) ) );

    fRet += fInt;

    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getYield( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fCoup, double fPrice, double fRedemp, sal_Int32 nFreq, const ANY& rOB )
    THROWDEF_RTE_IAE
{
    if( fCoup < 0.0 || fPrice <= 0.0 || fRedemp <= 0.0 || CHK_Freq || nSettle >= nMat )
        THROW_IAE;

    double fRet = getYield_( GetNullDate( xOpt ), nSettle, nMat, fCoup, fPrice, fRedemp, nFreq, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getYielddisc( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fPrice, double fRedemp, const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( fPrice <= 0.0 || fRedemp <= 0.0 || nSettle >= nMat )
        THROW_IAE;

    sal_Int32   nNullDate = GetNullDate( xOpt );

    double fRet = ( fRedemp / fPrice ) - 1.0;
    fRet /= GetYearFrac( nNullDate, nSettle, nMat, getDateMode( xOpt, rOB ) );

    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getYieldmat( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, double fRate, double fPrice, const ANY& rOB )
    THROWDEF_RTE_IAE
{
    if( fPrice <= 0.0 || fRate <= 0.0 || nSettle >= nMat )
        THROW_IAE;

    double fRet = GetYieldmat( GetNullDate( xOpt ),  nSettle, nMat, nIssue, fRate, fPrice, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getTbilleq( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fDisc ) THROWDEF_RTE_IAE
{
    nMat++;

    sal_Int32   nDiff = GetDiffDate360( xOpt, nSettle, nMat, sal_True );

    if( fDisc <= 0.0 || nSettle >= nMat || nDiff > 360 )
        THROW_IAE;

    double fRet = ( 365 * fDisc ) / ( 360 - ( fDisc * double( nDiff ) ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getTbillprice( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fDisc ) THROWDEF_RTE_IAE
{
    if( fDisc <= 0.0 || nSettle > nMat )
        THROW_IAE;

    nMat++;

    double  fFraction = GetYearFrac( xOpt, nSettle, nMat, 0 );  // method: USA 30/360

    double  fDummy;
    if( modf( fFraction, &fDummy ) == 0.0 )
        THROW_IAE;

    double fRet = 100.0 * ( 1.0 - fDisc * fFraction );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getTbillyield( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fPrice )
    THROWDEF_RTE_IAE
{
    sal_Int32   nDiff = GetDiffDate360( xOpt, nSettle, nMat, sal_True );
    nDiff++;

    if( fPrice <= 0.0 || nSettle >= nMat || nDiff > 360 )
        THROW_IAE;

    double      fRet = 100.0;
    fRet /= fPrice;
    fRet--;
    fRet /= double( nDiff );
    fRet *= 360.0;

    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getOddfprice( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, sal_Int32 nFirstCoup,
    double fRate, double fYield, double fRedemp, sal_Int32 nFreq, const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( fRate < 0.0 || fYield < 0.0 || CHK_Freq || nMat <= nFirstCoup || nFirstCoup <= nSettle || nSettle <= nIssue )
        THROW_IAE;

    double fRet = GetOddfprice( GetNullDate( xOpt ), nSettle, nMat, nIssue, nFirstCoup, fRate, fYield, fRedemp, nFreq, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getOddfyield( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, sal_Int32 nFirstCoup,
    double fRate, double fPrice, double fRedemp, sal_Int32 nFreq, const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( fRate < 0.0 || fPrice <= 0.0 || CHK_Freq || nMat <= nFirstCoup || nFirstCoup <= nSettle || nSettle <= nIssue )
        THROW_IAE;

    double fRet = GetOddfyield( GetNullDate( xOpt ), nSettle, nMat, nIssue, nFirstCoup, fRate, fPrice, fRedemp, nFreq,
                        getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getOddlprice( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nLastInterest,
    double fRate, double fYield, double fRedemp, sal_Int32 nFreq, const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( fRate < 0.0 || fYield < 0.0 || CHK_Freq || nMat <= nSettle || nSettle <= nLastInterest )
        THROW_IAE;

    double fRet = GetOddlprice( GetNullDate( xOpt ), nSettle, nMat, nLastInterest, fRate, fYield, fRedemp, nFreq,
                        getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getOddlyield( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nLastInterest,
    double fRate, double fPrice, double fRedemp, sal_Int32 nFreq, const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( fRate < 0.0 || fPrice <= 0.0 || CHK_Freq || nMat <= nSettle || nSettle <= nLastInterest )
        THROW_IAE;

    double fRet = GetOddlyield( GetNullDate( xOpt ), nSettle, nMat, nLastInterest, fRate, fPrice, fRedemp, nFreq,
                        getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


// ============================================================================
// XIRR helper functions

#define V_(i) (*rValues.Get(i))
#define D_(i) (*rDates.Get(i))

/** Calculates the resulting amount for the passed interest rate and the given XIRR parameters. */
double lcl_sca_XirrResult( const ScaDoubleList& rValues, const ScaDoubleList& rDates, double fRate )
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
double lcl_sca_XirrResult_Deriv1( const ScaDoubleList& rValues, const ScaDoubleList& rDates, double fRate )
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


// ----------------------------------------------------------------------------
// XIRR calculation

double SAL_CALL AnalysisAddIn::getXirr(
    constREFXPS& xOpt, const SEQSEQ( double )& rValues, const SEQSEQ( sal_Int32 )& rDates, const ANY& rGuessRate ) THROWDEF_RTE_IAE
{
    ScaDoubleList aValues, aDates;
    aValues.Append( rValues );
    aDates.Append( rDates );

    if( (aValues.Count() < 2) || (aValues.Count() != aDates.Count()) )
        THROW_IAE;

    // result interest rate, initialized with passed guessed rate, or 10%
    double fResultRate = aAnyConv.getDouble( xOpt, rGuessRate, 0.1 );
    if( fResultRate <= -1 )
        THROW_IAE;

    // maximum epsilon for end of iteration
    static const double fMaxEps = 1e-10;
    // maximum number of iterations
    static const sal_Int32 nMaxIter = 50;

    // Newton's method - try to find a fResultRate, so that lcl_sca_XirrResult() returns 0.
    double fNewRate, fRateEps, fResultValue;
    sal_Int32 nIter = 0;
    bool bContLoop;
    do
    {
        fResultValue = lcl_sca_XirrResult( aValues, aDates, fResultRate );
        fNewRate = fResultRate - fResultValue / lcl_sca_XirrResult_Deriv1( aValues, aDates, fResultRate );
        fRateEps = fabs( fNewRate - fResultRate );
        fResultRate = fNewRate;
        bContLoop = (fRateEps > fMaxEps) && (fabs( fResultValue ) > fMaxEps);
    }
    while( bContLoop && (++nIter < nMaxIter) );

    if( bContLoop )
        THROW_IAE;
    RETURN_FINITE( fResultRate );
}


// ============================================================================

double SAL_CALL AnalysisAddIn::getXnpv(
    double fRate, const SEQSEQ( double )& rValues, const SEQSEQ( sal_Int32 )& rDates ) THROWDEF_RTE_IAE
{
    ScaDoubleList aValList;
    ScaDoubleList aDateList;

    aValList.Append( rValues );
    aDateList.Append( rDates );

    sal_Int32           nNum = aValList.Count();

    if( nNum != sal_Int32( aDateList.Count() ) || nNum < 2 )
        THROW_IAE;

    double              fRet = 0.0;
    double              fNull = *aDateList.Get( 0 );
    fRate++;

    for( sal_Int32 i = 0 ; i < nNum ; i++ )
        fRet += *aValList.Get( i ) / ( pow( fRate, ( *aDateList.Get( i ) - fNull ) / 365.0 ) );

    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getIntrate( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fInvest, double fRedemp, const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( fInvest <= 0.0 || fRedemp <= 0.0 || nSettle >= nMat )
        THROW_IAE;

    double fRet = ( ( fRedemp / fInvest ) - 1.0 ) / GetYearDiff( GetNullDate( xOpt ), nSettle, nMat, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getCoupncd( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const ANY& rOB ) THROWDEF_RTE_IAE
{
    double fRet = GetCoupncd( GetNullDate( xOpt ), nSettle, nMat, nFreq, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getCoupdays( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const ANY& rOB ) THROWDEF_RTE_IAE
{
    double fRet = GetCoupdays( GetNullDate( xOpt ), nSettle, nMat, nFreq, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getCoupdaysnc( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const ANY& rOB ) THROWDEF_RTE_IAE
{
    double fRet = GetCoupdaysnc( GetNullDate( xOpt ), nSettle, nMat, nFreq, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getCoupdaybs( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const ANY& rOB ) THROWDEF_RTE_IAE
{
    double fRet = GetCoupdaybs( GetNullDate( xOpt ), nSettle, nMat, nFreq, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getCouppcd( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const ANY& rOB ) THROWDEF_RTE_IAE
{
    double fRet = GetCouppcd( GetNullDate( xOpt ), nSettle, nMat, nFreq, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getCoupnum( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const ANY& rOB ) THROWDEF_RTE_IAE
{
    double fRet = GetCoupnum( GetNullDate( xOpt ), nSettle, nMat, nFreq, getDateMode( xOpt, rOB ) );
    RETURN_FINITE( fRet );
}


double SAL_CALL AnalysisAddIn::getFvschedule( double fPrinc, const SEQSEQ( double )& rSchedule ) THROWDEF_RTE_IAE
{
    ScaDoubleList aSchedList;

    aSchedList.Append( rSchedule );

    for( const double* p = aSchedList.First() ; p ; p = aSchedList.Next() )
        fPrinc *= 1.0 + *p;

    RETURN_FINITE( fPrinc );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
