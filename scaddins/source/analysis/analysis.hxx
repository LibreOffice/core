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

#ifndef ANALYSIS_HXX
#define ANALYSIS_HXX


#include <com/sun/star/sheet/XAddIn.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/addin/XAnalysis.hpp>
#include <com/sun/star/sheet/LocalizedName.hpp>
#include <com/sun/star/sheet/XCompatibilityNames.hpp>
#include <com/sun/star/sheet/NoConvergenceException.hpp>

#include <cppuhelper/implbase5.hxx> // helper for implementations

#include "analysisdefs.hxx"
#include "analysishelper.hxx"


class FuncDataList;
class ConvertDataList;
class ResMgr;


REF( ::com::sun::star::uno::XInterface ) SAL_CALL AnalysisAddIn_CreateInstance( const REF( ::com::sun::star::lang::XMultiServiceFactory )& );


class AnalysisAddIn : public cppu::WeakImplHelper5<
                            ::com::sun::star::sheet::XAddIn,
                            ::com::sun::star::sheet::XCompatibilityNames,
                            ::com::sun::star::sheet::addin::XAnalysis,
                            ::com::sun::star::lang::XServiceName,
                            ::com::sun::star::lang::XServiceInfo >
{
private:
    ::com::sun::star::lang::Locale           aFuncLoc;
    ::com::sun::star::lang::Locale*          pDefLocales;
    FuncDataList*               pFD;
    double*                     pFactDoubles;
    ConvertDataList*            pCDL;
    ResMgr*                     pResMgr;

    ScaAnyConverter             aAnyConv;

    ResMgr&                     GetResMgr( void ) THROWDEF_RTE;
    OUString                      GetDisplFuncStr( sal_uInt16 nFuncNum ) THROWDEF_RTE;
    OUString                      GetFuncDescrStr( sal_uInt16 nResId, sal_uInt16 nStrIndex ) THROWDEF_RTE;
    void                        InitDefLocales( void );
    inline const ::com::sun::star::lang::Locale& GetLocale( sal_uInt32 nInd );
    void                        InitData( void );

                                /// Converts an Any to sal_Int32 in the range from 0 to 4 (date calculation mode).
    sal_Int32                   getDateMode(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xPropSet,
                                    const ::com::sun::star::uno::Any& rAny )
                                throw( ::com::sun::star::uno::RuntimeException, ::com::sun::star::lang::IllegalArgumentException );

public:
                                AnalysisAddIn(
                                    const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext );
    virtual                     ~AnalysisAddIn();

    double                      FactDouble( sal_Int32 nNum ) THROWDEF_RTE_IAE;

    static OUString               getImplementationName_Static();
    static SEQ( OUString )        getSupportedServiceNames_Static();

                                // XAddIn
    virtual OUString SAL_CALL     getProgrammaticFuntionName( const OUString& aDisplayName ) THROWDEF_RTE;
    virtual OUString SAL_CALL     getDisplayFunctionName( const OUString& aProgrammaticName ) THROWDEF_RTE;
    virtual OUString SAL_CALL     getFunctionDescription( const OUString& aProgrammaticName ) THROWDEF_RTE;
    virtual OUString SAL_CALL     getDisplayArgumentName( const OUString& aProgrammaticFunctionName, sal_Int32 nArgument ) THROWDEF_RTE;
    virtual OUString SAL_CALL     getArgumentDescription( const OUString& aProgrammaticFunctionName, sal_Int32 nArgument ) THROWDEF_RTE;
    virtual OUString SAL_CALL     getProgrammaticCategoryName( const OUString& aProgrammaticFunctionName ) THROWDEF_RTE;
    virtual OUString SAL_CALL     getDisplayCategoryName( const OUString& aProgrammaticFunctionName ) THROWDEF_RTE;

                                // XCompatibilityNames
    virtual SEQofLocName SAL_CALL getCompatibilityNames( const OUString& aProgrammaticName ) THROWDEF_RTE;

                                // XLocalizable
    virtual void SAL_CALL       setLocale( const ::com::sun::star::lang::Locale& eLocale ) THROWDEF_RTE;
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) THROWDEF_RTE;

                                // XServiceName
    virtual OUString SAL_CALL     getServiceName(  ) THROWDEF_RTE;

                                // XServiceInfo
    virtual OUString SAL_CALL     getImplementationName(  ) THROWDEF_RTE;
    virtual sal_Bool SAL_CALL   supportsService( const OUString& ServiceName ) THROWDEF_RTE;
    virtual SEQ( OUString ) SAL_CALL  getSupportedServiceNames(  ) THROWDEF_RTE;

    //  methods from own interfaces start here

                            // XAnalysis
//    virtual double SAL_CALL       get_Test( constREFXPS&, sal_Int32 nMode, double f1, double f2, double f3 ) THROWDEF_RTE;

    virtual sal_Int32 SAL_CALL  getWorkday( constREFXPS&, sal_Int32 nStartDate, sal_Int32 nDays, const ANY& aHDay ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getYearfrac( constREFXPS&, sal_Int32 nStartDate, sal_Int32 nEndDate, const ANY& aMode ) THROWDEF_RTE_IAE;
    virtual sal_Int32 SAL_CALL  getEdate( constREFXPS&, sal_Int32 nStartDate, sal_Int32 nMonths ) THROWDEF_RTE_IAE;
    virtual sal_Int32 SAL_CALL  getWeeknum( constREFXPS&, sal_Int32 nStartDate, sal_Int32 nMode ) THROWDEF_RTE_IAE;
    virtual sal_Int32 SAL_CALL  getEomonth( constREFXPS&, sal_Int32 nStartDate, sal_Int32 nMonths ) THROWDEF_RTE_IAE;
    virtual sal_Int32 SAL_CALL  getNetworkdays( constREFXPS&, sal_Int32 nStartDate, sal_Int32 nEndDate, const ANY& aHDay ) THROWDEF_RTE_IAE;

    virtual sal_Int32 SAL_CALL  getIseven( sal_Int32 nVal ) THROWDEF_RTE_IAE;
    virtual sal_Int32 SAL_CALL  getIsodd( sal_Int32 nVal ) THROWDEF_RTE_IAE;

    virtual double SAL_CALL     getMultinomial( constREFXPS& xOpt, const SEQSEQ( sal_Int32 )& aVLst, const SEQ( com::sun::star::uno::Any )& aOptVLst ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getSeriessum( double fX, double fN, double fM, const SEQSEQ( double )& aCoeffList ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getQuotient( double fNum, double fDenum ) THROWDEF_RTE_IAE;

    virtual double SAL_CALL     getMround( double fNum, double fMult ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getSqrtpi( double fNum ) THROWDEF_RTE_IAE;

    virtual double SAL_CALL     getRandbetween( double fMin, double fMax ) THROWDEF_RTE_IAE;

    virtual double SAL_CALL     getGcd( constREFXPS& xOpt, const SEQSEQ( double )& aVLst, const SEQ( ANY )& aOptVLst ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getLcm( constREFXPS& xOpt, const SEQSEQ( double )& aVLst, const SEQ( ANY )& aOptVLst ) THROWDEF_RTE_IAE;

    virtual double SAL_CALL     getBesseli( double fNum, sal_Int32 nOrder ) THROWDEF_RTE_IAE_NCE;
    virtual double SAL_CALL     getBesselj( double fNum, sal_Int32 nOrder ) THROWDEF_RTE_IAE_NCE;
    virtual double SAL_CALL     getBesselk( double fNum, sal_Int32 nOrder ) THROWDEF_RTE_IAE_NCE;
    virtual double SAL_CALL     getBessely( double fNum, sal_Int32 nOrder ) THROWDEF_RTE_IAE_NCE;

    virtual OUString SAL_CALL     getBin2Oct( constREFXPS& xOpt, const OUString& aNum, const ANY& rPlaces ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getBin2Dec( const OUString& aNum ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getBin2Hex( constREFXPS& xOpt, const OUString& aNum, const ANY& rPlaces ) THROWDEF_RTE_IAE;

    virtual OUString SAL_CALL     getOct2Bin( constREFXPS& xOpt, const OUString& aNum, const ANY& rPlaces ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getOct2Dec( const OUString& aNum ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getOct2Hex( constREFXPS& xOpt, const OUString& aNum, const ANY& rPlaces ) THROWDEF_RTE_IAE;

    virtual OUString SAL_CALL     getDec2Bin( constREFXPS& xOpt, sal_Int32 fNum, const ANY& rPlaces ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getDec2Oct( constREFXPS& xOpt, sal_Int32 fNum, const ANY& rPlaces ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getDec2Hex( constREFXPS& xOpt, double fNum, const ANY& rPlaces ) THROWDEF_RTE_IAE;

    virtual OUString SAL_CALL     getHex2Bin( constREFXPS& xOpt, const OUString& aNum, const ANY& rPlaces ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getHex2Dec( const OUString& aNum ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getHex2Oct( constREFXPS& xOpt, const OUString& aNum, const ANY& rPlaces ) THROWDEF_RTE_IAE;

    virtual sal_Int32 SAL_CALL  getDelta( constREFXPS& xOpt, double fNum1, const ANY& rNum2 ) THROWDEF_RTE_IAE;

    virtual double SAL_CALL     getErf( constREFXPS& xOpt, double fLowerLimit, const ANY& rUpperLimit ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getErfc( double fLowerLimit ) THROWDEF_RTE_IAE;

    virtual sal_Int32 SAL_CALL  getGestep( constREFXPS& xOpt, double fNum, const ANY& rStep ) THROWDEF_RTE_IAE;

    virtual double SAL_CALL     getFactdouble( sal_Int32 nNum ) THROWDEF_RTE_IAE;

    virtual double SAL_CALL     getImabs( const OUString& aNum ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getImaginary( const OUString& aNum ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getImpower( const OUString& aNum, double fPower ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getImargument( const OUString& aNum ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getImcos( const OUString& aNum ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getImdiv( const OUString& aDivident, const OUString& aDivisor ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getImexp( const OUString& aNum ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getImconjugate( const OUString& aNum ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getImln( const OUString& aNum ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getImlog10( const OUString& aNum ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getImlog2( const OUString& aNum ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getImproduct( constREFXPS& xOpt, const SEQSEQ( OUString )& aNum1, const SEQ_ANY& aNumList ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getImreal( const OUString& aNum ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getImsin( const OUString& aNum ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getImsub( const OUString& aNum1, const OUString& aNum2 ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getImsum( constREFXPS& xOpt, const SEQSEQ( OUString )& aNum1, const SEQ( ANY )& aFollowingPars ) THROWDEF_RTE_IAE;

    virtual OUString SAL_CALL     getImsqrt( const OUString& aNum ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getImtan( const OUString& aNum ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getImsec( const OUString& aNum ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getImcsc( const OUString& aNum ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getImcot( const OUString& aNum ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getImsinh( const OUString& aNum ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getImcosh( const OUString& aNum ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getImsech( const OUString& aNum ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getImcsch( const OUString& aNum ) THROWDEF_RTE_IAE;
    virtual OUString SAL_CALL     getComplex( double fReal, double fImaginary, const ANY& rSuffix ) THROWDEF_RTE_IAE;

    virtual double SAL_CALL     getConvert( double fVal, const OUString& aFromUnit, const OUString& aToUnit ) THROWDEF_RTE_IAE;

    virtual double SAL_CALL     getAmordegrc( constREFXPS&, double fCost, sal_Int32 nDate, sal_Int32 nFirstPer, double fRestVal, double fPer, double fRate, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getAmorlinc( constREFXPS&, double fCost, sal_Int32 nDate, sal_Int32 nFirstPer, double fRestVal, double fPer, double fRate, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getAccrint( constREFXPS& xOpt, sal_Int32 nIssue, sal_Int32 nFirstInter, sal_Int32 nSettle, double fRate, const ANY& rVal, sal_Int32 nFreq, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getAccrintm( constREFXPS& xOpt, sal_Int32 nIssue, sal_Int32 nSettle, double fRate, const ANY& rVal, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getReceived( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fInvest, double fDisc, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getDisc( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fPrice, double fRedemp, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getDuration( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fCoup, double fYield, sal_Int32 nFreq, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getEffect( double fNominal, sal_Int32 nPeriods ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getCumprinc( double fRate, sal_Int32 nNumPeriods, double fVal, sal_Int32 nStartPer, sal_Int32 nEndPer, sal_Int32 nPayType ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getCumipmt( double fRate, sal_Int32 nNumPeriods, double fVal, sal_Int32 nStartPer, sal_Int32 nEndPer, sal_Int32 nPayType ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getPrice( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fRate, double fYield, double fRedemp, sal_Int32 nFreq, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getPricedisc( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fDisc, double fRedemp, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getPricemat( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, double fRate, double fYield, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getMduration( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fCoup, double fYield, sal_Int32 nFreq, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getNominal( double fRate, sal_Int32 nPeriods ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getDollarfr( double fDollarDec, sal_Int32 nFrac ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getDollarde( double fDollarFrac, sal_Int32 nFrac ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getYield( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fCoup, double fPrice, double fRedemp, sal_Int32 nFreq, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getYielddisc( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fPrice, double fRedemp, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getYieldmat( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, double fRate, double fPrice, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getTbilleq( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fDisc ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getTbillprice( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fDisc ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getTbillyield( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fPrice ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getOddfprice( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, sal_Int32 nFirstCoup, double fRate, double fYield, double fRedemp, sal_Int32 nFreq, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getOddfyield( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, sal_Int32 nFirstCoup, double fRate, double fPrice, double fRedemp, sal_Int32 nFreq, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getOddlprice( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nLastInterest, double fRate, double fYield, double fRedemp, sal_Int32 nFreq, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getOddlyield( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nLastInterest, double fRate, double fPrice, double fRedemp, sal_Int32 nFreq, const ANY& rOptBase) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getXirr( constREFXPS& xOpt, const SEQSEQ( double )& rValues, const SEQSEQ( sal_Int32 )& rDates, const ANY& rGuess ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getXnpv( double fRate, const SEQSEQ( double )& rValues, const SEQSEQ( sal_Int32 )& rDates ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getIntrate( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fInvest, double fRedemp, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getCoupncd( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getCoupdays( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getCoupdaysnc( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getCoupdaybs( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getCouppcd( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getCoupnum( constREFXPS& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const ANY& rOptBase ) THROWDEF_RTE_IAE;
    virtual double SAL_CALL     getFvschedule( double fPrinc, const SEQSEQ( double )& rSchedule ) THROWDEF_RTE_IAE;
};

//------------------------------------------------------------------

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
