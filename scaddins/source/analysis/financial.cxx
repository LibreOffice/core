/*************************************************************************
 *
 *  $RCSfile: financial.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: dr $ $Date: 2001-08-17 10:00:19 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "analysis.hxx"
#include "analysishelper.hxx"




double SAL_CALL AnalysisAddIn::getAmordegrc( constREFXPS& xOpt,
    double fCost, sal_Int32 nDate, sal_Int32 nFirstPer, double fRestVal,
    double fPer, double fRate, const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( nDate > nFirstPer || fRate <= 0.0 || fRestVal > fCost )
        THROW_IAE;

    return GetAmordegrc( GetNullDate( xOpt ), fCost, nDate, nFirstPer, fRestVal, fPer, fRate, GetOptBase( rOB ) );
}


double SAL_CALL AnalysisAddIn::getAmorlinc( constREFXPS& xOpt,
    double fCost, sal_Int32 nDate, sal_Int32 nFirstPer, double fRestVal,
    double fPer, double fRate, const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( nDate > nFirstPer || fRate <= 0.0 || fRestVal > fCost )
        THROW_IAE;

    return GetAmorlinc( GetNullDate( xOpt ), fCost, nDate, nFirstPer, fRestVal, fPer, fRate, GetOptBase( rOB ) );
}


double SAL_CALL AnalysisAddIn::getAccrint( constREFXPS& xOpt,
    sal_Int32 nIssue, sal_Int32 nFirstInter, sal_Int32 nSettle, double fRate, double fVal, sal_Int32 nFreq,
    const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( fRate <= 0.0 || fVal <= 0.0 || CHK_Freq || nIssue >= nSettle )
        THROW_IAE;

    return fVal * fRate * GetYearDiff( GetNullDate( xOpt ), nIssue, nSettle, GetOptBase( rOB ) );
}


double SAL_CALL AnalysisAddIn::getAccrintm( constREFXPS& xOpt,
    sal_Int32 nIssue, sal_Int32 nSettle, double fRate, const ANY& rVal, const ANY& rOB ) THROWDEF_RTE_IAE
{
    double      fVal = GetOpt( rVal, 1000.0 );

    if( fRate <= 0.0 || fVal <= 0.0 || nIssue >= nSettle )
        THROW_IAE;

    return fVal * fRate * GetYearDiff( GetNullDate( xOpt ), nIssue, nSettle, GetOptBase( rOB ) );
}


double SAL_CALL AnalysisAddIn::getReceived( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fInvest, double fDisc, const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( fInvest <= 0.0 || fDisc <= 0.0 )
        THROW_IAE;

    return fInvest / ( 1.0 - ( fDisc * GetYearDiff( GetNullDate( xOpt ), nSettle, nMat, GetOptBase( rOB ) ) ) );
}


double SAL_CALL AnalysisAddIn::getDisc( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fPrice, double fRedemp, const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( fPrice <= 0.0 || fRedemp <= 0.0 || nSettle >= nMat )
        THROW_IAE;
    return ( 1.0 - fPrice / fRedemp ) / GetYearFrac( xOpt, nSettle, nMat, GetOptBase( rOB ) );
}


double SAL_CALL AnalysisAddIn::getDuration( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fCoup, double fYield, sal_Int32 nFreq, const ANY& rOB )
    THROWDEF_RTE_IAE
{
    if( fCoup < 0.0 || fYield < 0.0 || CHK_Freq || nSettle >= nMat )
        THROW_IAE;

    return GetDuration( GetNullDate( xOpt ),  nSettle, nMat, fCoup, fYield, nFreq, GetOptBase( rOB ) );
}


double SAL_CALL AnalysisAddIn::getEffect( double fNominal, sal_Int32 nPeriods ) THROWDEF_RTE_IAE
{
    if( nPeriods < 1 || fNominal <= 0.0 )
        THROW_IAE;

    double  fPeriods = nPeriods;

    return pow( 1.0 + fNominal / fPeriods, fPeriods ) - 1.0;
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

    sal_uInt32  nAnfang = sal_uInt32( nStartPer );
    sal_uInt32  nEnde = sal_uInt32( nEndPer );

    if( nAnfang == 1 )
    {
        if( nPayType <= 0 )
            fKapZ = fRmz + fVal * fRate;
        else
            fKapZ = fRmz;

        nAnfang++;
    }

    for( sal_uInt32 i = nAnfang ; i <= nEnde ; i++ )
    {
        if( nPayType > 0 )
            fKapZ += fRmz - ( GetZw( fRate, double( i - 2 ), fRmz, fVal, 1 ) - fRmz ) * fRate;
        else
            fKapZ += fRmz - GetZw( fRate, double( i - 1 ), fRmz, fVal, 0 ) * fRate;
    }

    return fKapZ;
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

    sal_uInt32  nAnfang = sal_uInt32( nStartPer );
    sal_uInt32  nEnde = sal_uInt32( nEndPer );

    if( nAnfang == 1 )
    {
        if( nPayType <= 0 )
            fZinsZ = -fVal;

        nAnfang++;
    }

    for( sal_uInt32 i = nAnfang ; i <= nEnde ; i++ )
    {
        if( nPayType > 0 )
            fZinsZ += GetZw( fRate, double( i - 2 ), fRmz, fVal, 1 ) - fRmz;
        else
            fZinsZ += GetZw( fRate, double( i - 1 ), fRmz, fVal, 0 );
    }

    fZinsZ *= fRate;

    return fZinsZ;
}


double SAL_CALL AnalysisAddIn::getPrice( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fRate, double fYield, double fRedemp, sal_Int32 nFreq,
    const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( fYield < 0.0 || fRate < 0.0 || fRedemp <= 0 || CHK_Freq || nSettle >= nMat )
        THROW_IAE;

    return getPrice_( GetNullDate( xOpt ), nSettle, nMat, fRate, fYield, fRedemp, nFreq, GetOptBase( rOB ) );
}


double SAL_CALL AnalysisAddIn::getPricedisc( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fDisc, double fRedemp, const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( fDisc <= 0.0 || fRedemp <= 0 || nSettle >= nMat )
        THROW_IAE;

    return fRedemp * ( 1.0 - fDisc * GetYearDiff( GetNullDate( xOpt ), nSettle, nMat, GetOptBase( rOB ) ) );
}


double SAL_CALL AnalysisAddIn::getPricemat( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, double fRate, double fYield, const ANY& rOB )
    THROWDEF_RTE_IAE
{
    if( fRate < 0.0 || fYield < 0.0 || nSettle >= nMat )
        THROW_IAE;

    sal_Int32   nNullDate = GetNullDate( xOpt );
    sal_Int32   nBase = GetOptBase( rOB );

    double      fIssMat = GetYearFrac( nNullDate, nIssue, nMat, nBase );
    double      fIssSet = GetYearFrac( nNullDate, nIssue, nSettle, nBase );
    double      fSetMat = GetYearFrac( nNullDate, nSettle, nMat, nBase );

    double      fRet = 1.0 + fIssMat * fRate;
    fRet /= 1.0 + fSetMat * fYield;
    fRet -= fIssMat * fRate;
    fRet *= 100.0;

    return fRet;
}


double SAL_CALL AnalysisAddIn::getMduration( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fCoup, double fYield, sal_Int32 nFreq, const ANY& rOB )
    THROWDEF_RTE_IAE
{
    if( fCoup < 0.0 || fYield < 0.0 || CHK_Freq )
        THROW_IAE;

    sal_Int32   nBase = GetOptBase( rOB );

    double      fRet = GetDuration( GetNullDate( xOpt ),  nSettle, nMat, fCoup, fYield, nFreq, GetOptBase( rOB ) );
    fRet /= 1.0 + ( fYield / double( nFreq ) );
    return fRet;
}


double SAL_CALL AnalysisAddIn::getNominal( double fRate, sal_Int32 nPeriods ) THROWDEF_RTE_IAE
{
    if( fRate <= 0.0 || nPeriods < 0 )
        THROW_IAE;

    double  fPeriods = nPeriods;
    return ( pow( fRate + 1.0, 1.0 / fPeriods ) - 1.0 ) * fPeriods;
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

    return fRet;
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

    return fRet;
}


double SAL_CALL AnalysisAddIn::getYield( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fCoup, double fPrice, double fRedemp, sal_Int32 nFreq, const ANY& rOB )
    THROWDEF_RTE_IAE
{
    if( fCoup < 0.0 || fPrice <= 0.0 || fRedemp <= 0.0 || CHK_Freq || nSettle >= nMat )
        THROW_IAE;

    return getYield_( GetNullDate( xOpt ), nSettle, nMat, fCoup, fPrice, fRedemp, nFreq, GetOptBase( rOB ) );
}


double SAL_CALL AnalysisAddIn::getYielddisc( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fPrice, double fRedemp, const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( fPrice <= 0.0 || fRedemp <= 0.0 || nSettle >= nMat )
        THROW_IAE;

    sal_Int32   nBase = GetOptBase( rOB );
    sal_Int32   nNullDate = GetNullDate( xOpt );

    double      fRet = 1.0 - fPrice / fRedemp;
//  fRet /= GetYearDiff( nNullDate, nSettle, nMat, nBase );
    fRet /= GetYearFrac( nNullDate, nSettle, nMat, nBase );

    return fRet / 0.99795;  // don't know what this constant means in original
}


double SAL_CALL AnalysisAddIn::getYieldmat( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, double fRate, double fPrice, const ANY& rOB )
    THROWDEF_RTE_IAE
{
    if( fRate < 0.0 || fRate <= 0.0 || nSettle >= nMat )
        THROW_IAE;

    return GetYieldmat( GetNullDate( xOpt ),  nSettle, nMat, nIssue, fRate, fPrice, GetOptBase( rOB ) );
}


double SAL_CALL AnalysisAddIn::getTbilleq( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fDisc ) THROWDEF_RTE_IAE
{
    nMat++;

    sal_Int32   nDiff = GetDiffDate360( xOpt, nSettle, nMat, sal_True );

    if( fDisc <= 0.0 || nSettle >= nMat || nDiff > 360 )
        THROW_IAE;

    return ( 365 * fDisc ) / ( 360 - ( fDisc * double( nDiff ) ) );
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

    return 100.0 * ( 1.0 - fDisc * fFraction );
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

    return fRet;
}


double SAL_CALL AnalysisAddIn::getOddfprice( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, sal_Int32 nFirstCoup,
    double fRate, double fYield, double fRedemp, sal_Int32 nFreq, const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( fRate < 0 || fYield < 0 || CHK_Freq || nMat <= nFirstCoup || nFirstCoup <= nSettle || nSettle <= nIssue )
        THROW_IAE;

    sal_Int32   nBase = GetOptBase( rOB );
    sal_Int32   nNullDate = GetNullDate( xOpt );

    double      fN = GetCoupnum( nNullDate, nSettle, nMat, nFreq, nBase ) - 1.0;
    double      fA = nSettle - GetCouppcd( nNullDate, nSettle, nFirstCoup, nFreq, nBase );
    double      fE = GetCoupdays( nNullDate, nSettle, nMat, nFreq, nBase );
    double      fDSC = GetCoupdaysnc( nNullDate, nSettle, nMat, nFreq, nBase );
    double      fDSC_E = fDSC / fE;
    double      fDFC = GetCoupdaysnc( nNullDate, nIssue, nFirstCoup, nFreq, nBase );

    double      fT1 = 1.0 + fYield / double( nFreq );
    double      fT2 = 100.0 * fRate / double( nFreq );

    double      fRet = fRedemp / pow( fT1, fN + fDSC_E );
    fRet += fT2 * fDFC / ( pow( fT1, fDSC_E ) * fE );
    fRet -= fT2 * fA / fE;

    double      fC = 1.0 + fDSC / fE;
    sal_Int32   nN = sal_Int32( fN );
    for( sal_Int32 k = 1 ; k < nN ; k++, fC++ )
        fRet += fT2 / pow( fT1, fC );

    return fRet;
}


double SAL_CALL AnalysisAddIn::getOddfyield( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, sal_Int32 nFirstCoup,
    double fRate, double fPrice, double fRedemp, sal_Int32 nFreq, const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( fRate < 0 || fPrice <= 0 || CHK_Freq || nMat <= nFirstCoup || nFirstCoup <= nSettle || nSettle <= nIssue )
        THROW_IAE;

    return GetOddfyield( GetNullDate( xOpt ), nSettle, nMat, nIssue, nFirstCoup, fRate, fPrice, fRedemp, nFreq,
                        GetOptBase( rOB ) );
}


double SAL_CALL AnalysisAddIn::getOddlprice( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nLastInterest,
    double fRate, double fYield, double fRedemp, sal_Int32 nFreq, const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( fRate < 0 || fYield < 0 || CHK_Freq || nMat <= nSettle || nSettle <= nLastInterest )
        THROW_IAE;

    return GetOddlprice( GetNullDate( xOpt ), nSettle, nMat, nLastInterest, fRate, fYield, fRedemp, nFreq,
                        GetOptBase( rOB ) );
}


double SAL_CALL AnalysisAddIn::getOddlyield( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nLastInterest,
    double fRate, double fPrice, double fRedemp, sal_Int32 nFreq, const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( fRate < 0 || fPrice <= 0 || CHK_Freq || nMat <= nSettle || nSettle <= nLastInterest )
        THROW_IAE;

    return GetOddlyield( GetNullDate( xOpt ), nSettle, nMat, nLastInterest, fRate, fPrice, fRedemp, nFreq,
                        GetOptBase( rOB ) );
}


double SAL_CALL AnalysisAddIn::getXirr(
    const SEQSEQ( double )& rValues, const SEQSEQ( sal_Int32 )& rDates, const ANY& rGuess ) THROWDEF_RTE_IAE
{

    double              fGuess = GetOpt( rGuess, 0.1 );

    DoubleList          aValList;
    DoubleList          aDateList;

    aValList.Append( rValues );
    aDateList.Append( rDates );

    sal_Int32           nNum = aValList.Count();

    if( nNum != sal_Int32( aDateList.Count() ) || nNum < 2 )
        THROW_IAE;

    double              f, fG, fK;
    sal_Int32           nMax = 200;

    double              fYld = fGuess;
    double              fDiff = fYld;
    double              fOld = 0.0;
    sal_Int32           n = 1;
    sal_Int32           nNew = 0;
    double              fNull = *aDateList.Get( 0 );


    while( fabs( fDiff ) > 1E-10 && n <= nMax )
    {
        f = 0.0;
        n++;
        for( sal_Int32 i = 0 ; i < nNum ; i++ )
            f += *aValList.Get( i ) / pow( 1.0 + fYld, ( *aDateList.Get( i ) - fNull ) / 365.0 );

        if( ( ( fOld < 0.0 && f > 0.0 ) || ( fOld > 0.0 && f < 0.0 ) ) && nNew == 0 )
        {
            if( f < 0.0 )
            {
                fG = fYld;
                fK = fYld - 0.1;
            }
            else
            {
                fG = fYld - 0.1;
                fK = fYld;
            }
            fYld = 0.5 * ( fG + fK );
            nNew = 1;
        }
        else if( nNew == 0 )
        {
            fYld += 0.1;
            fOld = f;
        }
        else
        {
            if( f < 0.0 )
                fG = fYld;
            else
                fK = fYld;
            double      fTmp = 0.5 * ( fG + fK );
            fDiff = fYld - fTmp;
            fYld = fTmp;
        }
    }

    if( floor( f ) == 0.0 || fabs( fDiff ) <= 1e-10 )
        return fYld;
    else
        THROW_IAE;
}


double SAL_CALL AnalysisAddIn::getXnpv(
    double fRate, const SEQSEQ( double )& rValues, const SEQSEQ( sal_Int32 )& rDates ) THROWDEF_RTE_IAE
{
    DoubleList          aValList;
    DoubleList          aDateList;

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

    return fRet;
}


double SAL_CALL AnalysisAddIn::getIntrate( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, double fInvest, double fRedemp, const ANY& rOB ) THROWDEF_RTE_IAE
{
    if( fInvest <= 0.0 || fRedemp <= 0.0 || nSettle >= nMat )
        THROW_IAE;

    return ( ( fRedemp / fInvest ) - 1.0 ) / GetYearDiff( GetNullDate( xOpt ), nSettle, nMat, GetOptBase( rOB ) );
}


double SAL_CALL AnalysisAddIn::getCoupncd( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const ANY& rOB ) THROWDEF_RTE_IAE
{
    return GetCoupncd( GetNullDate( xOpt ), nSettle, nMat, nFreq, GetOptBase( rOB ) );
}


double SAL_CALL AnalysisAddIn::getCoupdays( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const ANY& rOB ) THROWDEF_RTE_IAE
{
    return GetCoupdays( GetNullDate( xOpt ), nSettle, nMat, nFreq, GetOptBase( rOB ) );
}


double SAL_CALL AnalysisAddIn::getCoupdaysnc( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const ANY& rOB ) THROWDEF_RTE_IAE
{
    return GetCoupdaysnc( GetNullDate( xOpt ), nSettle, nMat, nFreq, GetOptBase( rOB ) );
}


double SAL_CALL AnalysisAddIn::getCoupdaybs( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const ANY& rOB ) THROWDEF_RTE_IAE
{
    return GetCoupdaybs( GetNullDate( xOpt ), nSettle, nMat, nFreq, GetOptBase( rOB ) );
}


double SAL_CALL AnalysisAddIn::getCouppcd( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const ANY& rOB ) THROWDEF_RTE_IAE
{
    return GetCouppcd( GetNullDate( xOpt ), nSettle, nMat, nFreq, GetOptBase( rOB ) );
}


double SAL_CALL AnalysisAddIn::getCoupnum( constREFXPS& xOpt,
    sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const ANY& rOB ) THROWDEF_RTE_IAE
{
    return GetCoupnum( GetNullDate( xOpt ), nSettle, nMat, nFreq, GetOptBase( rOB ) );
}


double SAL_CALL AnalysisAddIn::getFvschedule( double fPrinc, const SEQSEQ( double )& rSchedule ) THROWDEF_RTE_IAE
{
    DoubleList          aSchedList;

    aSchedList.Append( rSchedule );

    for( const double* p = aSchedList.First() ; p ; p = aSchedList.Next() )
        fPrinc *= 1.0 + *p;

    return fPrinc;
}


