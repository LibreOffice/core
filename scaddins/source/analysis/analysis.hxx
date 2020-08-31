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

#ifndef INCLUDED_SCADDINS_SOURCE_ANALYSIS_ANALYSIS_HXX
#define INCLUDED_SCADDINS_SOURCE_ANALYSIS_ANALYSIS_HXX


#include <com/sun/star/sheet/XAddIn.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sheet/addin/XAnalysis.hpp>
#include <com/sun/star/sheet/XCompatibilityNames.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include "analysishelper.hxx"

#include <memory>

namespace com::sun::star::lang { class XMultiServiceFactory; }
namespace com::sun::star::sheet { struct LocalizedName; }

typedef cppu::WeakComponentImplHelper<
                            css::sheet::XAddIn,
                            css::sheet::XCompatibilityNames,
                            css::sheet::addin::XAnalysis,
                            css::lang::XServiceName,
                            css::lang::XServiceInfo > AnalysisAddIn_Base;

class AnalysisAddIn : private cppu::BaseMutex, public AnalysisAddIn_Base
{
private:
    css::lang::Locale           aFuncLoc;
    std::unique_ptr<css::lang::Locale[]> pDefLocales;
    std::unique_ptr<sca::analysis::FuncDataList> pFD;
    std::unique_ptr<double[]>   pFactDoubles;
    std::unique_ptr<sca::analysis::ConvertDataList> pCDL;
    std::locale                 aResLocale;

    sca::analysis::ScaAnyConverter aAnyConv;

    /// @throws css::uno::RuntimeException
    OUString                    GetFuncDescrStr(const char** pResId, sal_uInt16 nStrIndex);
    void                        InitDefLocales();
    inline const css::lang::Locale& GetLocale( sal_uInt32 nInd );
    void                        InitData();

                                /// Converts an Any to sal_Int32 in the range from 0 to 4 (date calculation mode).
                                ///
                                /// @throws css::uno::RuntimeException
                                /// @throws css::lang::IllegalArgumentException
    sal_Int32                   getDateMode(
                                    const css::uno::Reference< css::beans::XPropertySet >& xPropSet,
                                    const css::uno::Any& rAny );

public:
    explicit                    AnalysisAddIn(
                                    const css::uno::Reference< css::uno::XComponentContext >& xContext );

    OUString AnalysisResId(const char* pId);

    virtual                     ~AnalysisAddIn() override;

    // XComponent
    virtual void SAL_CALL       dispose() override;

    /// @throws css::uno::RuntimeException
    /// @throws css::lang::IllegalArgumentException
    double                      FactDouble( sal_Int32 nNum );

                                // XAddIn
    virtual OUString SAL_CALL     getProgrammaticFunctionName( const OUString& aDisplayName ) override;
    virtual OUString SAL_CALL     getDisplayFunctionName( const OUString& aProgrammaticName ) override;
    virtual OUString SAL_CALL     getFunctionDescription( const OUString& aProgrammaticName ) override;
    virtual OUString SAL_CALL     getDisplayArgumentName( const OUString& aProgrammaticFunctionName, sal_Int32 nArgument ) override;
    virtual OUString SAL_CALL     getArgumentDescription( const OUString& aProgrammaticFunctionName, sal_Int32 nArgument ) override;
    virtual OUString SAL_CALL     getProgrammaticCategoryName( const OUString& aProgrammaticFunctionName ) override;
    virtual OUString SAL_CALL     getDisplayCategoryName( const OUString& aProgrammaticFunctionName ) override;

                                // XCompatibilityNames
    virtual css::uno::Sequence< css::sheet::LocalizedName > SAL_CALL getCompatibilityNames( const OUString& aProgrammaticName ) override;

                                // XLocalizable
    virtual void SAL_CALL       setLocale( const css::lang::Locale& eLocale ) override;
    virtual css::lang::Locale SAL_CALL getLocale(  ) override;

                                // XServiceName
    virtual OUString SAL_CALL     getServiceName(  ) override;

                                // XServiceInfo
    virtual OUString SAL_CALL     getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL   supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL  getSupportedServiceNames(  ) override;

    //  methods from own interfaces start here

                            // XAnalysis
//    virtual double SAL_CALL       get_Test( const css::uno::Reference< css::beans::XPropertySet >&, sal_Int32 nMode, double f1, double f2, double f3 ) throw( css::uno::RuntimeException );

    virtual sal_Int32 SAL_CALL  getWorkday( const css::uno::Reference< css::beans::XPropertySet >&, sal_Int32 nStartDate, sal_Int32 nDays, const css::uno::Any& aHDay ) override;
    virtual double SAL_CALL     getYearfrac( const css::uno::Reference< css::beans::XPropertySet >&, sal_Int32 nStartDate, sal_Int32 nEndDate, const css::uno::Any& aMode ) override;
    virtual sal_Int32 SAL_CALL  getEdate( const css::uno::Reference< css::beans::XPropertySet >&, sal_Int32 nStartDate, sal_Int32 nMonths ) override;
    virtual sal_Int32 SAL_CALL  getWeeknum( const css::uno::Reference< css::beans::XPropertySet >&, sal_Int32 nStartDate, sal_Int32 nMode ) override;
    virtual sal_Int32 SAL_CALL  getEomonth( const css::uno::Reference< css::beans::XPropertySet >&, sal_Int32 nStartDate, sal_Int32 nMonths ) override;
    virtual sal_Int32 SAL_CALL  getNetworkdays( const css::uno::Reference< css::beans::XPropertySet >&, sal_Int32 nStartDate, sal_Int32 nEndDate, const css::uno::Any& aHDay ) override;

    virtual sal_Int32 SAL_CALL  getIseven( sal_Int32 nVal ) override;
    virtual sal_Int32 SAL_CALL  getIsodd( sal_Int32 nVal ) override;

    virtual double SAL_CALL     getMultinomial( const css::uno::Reference< css::beans::XPropertySet >& xOpt, const css::uno::Sequence< css::uno::Sequence< sal_Int32 > >& aVLst, const css::uno::Sequence< css::uno::Any >& aOptVLst ) override;
    virtual double SAL_CALL     getSeriessum( double fX, double fN, double fM, const css::uno::Sequence< css::uno::Sequence< double > >& aCoeffList ) override;
    virtual double SAL_CALL     getQuotient( double fNum, double fDenum ) override;

    virtual double SAL_CALL     getMround( double fNum, double fMult ) override;
    virtual double SAL_CALL     getSqrtpi( double fNum ) override;

    virtual double SAL_CALL     getRandbetween( double fMin, double fMax ) override;

    virtual double SAL_CALL     getGcd( const css::uno::Reference< css::beans::XPropertySet >& xOpt, const css::uno::Sequence< css::uno::Sequence< double > >& aVLst, const css::uno::Sequence< css::uno::Any >& aOptVLst ) override;
    virtual double SAL_CALL     getLcm( const css::uno::Reference< css::beans::XPropertySet >& xOpt, const css::uno::Sequence< css::uno::Sequence< double > >& aVLst, const css::uno::Sequence< css::uno::Any >& aOptVLst ) override;

    virtual double SAL_CALL     getBesseli( double fNum, sal_Int32 nOrder ) override;
    virtual double SAL_CALL     getBesselj( double fNum, sal_Int32 nOrder ) override;
    virtual double SAL_CALL     getBesselk( double fNum, sal_Int32 nOrder ) override;
    virtual double SAL_CALL     getBessely( double fNum, sal_Int32 nOrder ) override;

    virtual OUString SAL_CALL     getBin2Oct( const css::uno::Reference< css::beans::XPropertySet >& xOpt, const OUString& aNum, const css::uno::Any& rPlaces ) override;
    virtual double SAL_CALL     getBin2Dec( const OUString& aNum ) override;
    virtual OUString SAL_CALL     getBin2Hex( const css::uno::Reference< css::beans::XPropertySet >& xOpt, const OUString& aNum, const css::uno::Any& rPlaces ) override;

    virtual OUString SAL_CALL     getOct2Bin( const css::uno::Reference< css::beans::XPropertySet >& xOpt, const OUString& aNum, const css::uno::Any& rPlaces ) override;
    virtual double SAL_CALL     getOct2Dec( const OUString& aNum ) override;
    virtual OUString SAL_CALL     getOct2Hex( const css::uno::Reference< css::beans::XPropertySet >& xOpt, const OUString& aNum, const css::uno::Any& rPlaces ) override;

    virtual OUString SAL_CALL     getDec2Bin( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 fNum, const css::uno::Any& rPlaces ) override;
    virtual OUString SAL_CALL     getDec2Oct( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 fNum, const css::uno::Any& rPlaces ) override;
    virtual OUString SAL_CALL     getDec2Hex( const css::uno::Reference< css::beans::XPropertySet >& xOpt, double fNum, const css::uno::Any& rPlaces ) override;

    virtual OUString SAL_CALL     getHex2Bin( const css::uno::Reference< css::beans::XPropertySet >& xOpt, const OUString& aNum, const css::uno::Any& rPlaces ) override;
    virtual double SAL_CALL     getHex2Dec( const OUString& aNum ) override;
    virtual OUString SAL_CALL     getHex2Oct( const css::uno::Reference< css::beans::XPropertySet >& xOpt, const OUString& aNum, const css::uno::Any& rPlaces ) override;

    virtual sal_Int32 SAL_CALL  getDelta( const css::uno::Reference< css::beans::XPropertySet >& xOpt, double fNum1, const css::uno::Any& rNum2 ) override;

    virtual double SAL_CALL     getErf( const css::uno::Reference< css::beans::XPropertySet >& xOpt, double fLowerLimit, const css::uno::Any& rUpperLimit ) override;
    virtual double SAL_CALL     getErfc( double fLowerLimit ) override;

    virtual sal_Int32 SAL_CALL  getGestep( const css::uno::Reference< css::beans::XPropertySet >& xOpt, double fNum, const css::uno::Any& rStep ) override;

    virtual double SAL_CALL     getFactdouble( sal_Int32 nNum ) override;

    virtual double SAL_CALL     getImabs( const OUString& aNum ) override;
    virtual double SAL_CALL     getImaginary( const OUString& aNum ) override;
    virtual OUString SAL_CALL     getImpower( const OUString& aNum, double fPower ) override;
    virtual double SAL_CALL     getImargument( const OUString& aNum ) override;
    virtual OUString SAL_CALL     getImcos( const OUString& aNum ) override;
    virtual OUString SAL_CALL     getImdiv( const OUString& aDivident, const OUString& aDivisor ) override;
    virtual OUString SAL_CALL     getImexp( const OUString& aNum ) override;
    virtual OUString SAL_CALL     getImconjugate( const OUString& aNum ) override;
    virtual OUString SAL_CALL     getImln( const OUString& aNum ) override;
    virtual OUString SAL_CALL     getImlog10( const OUString& aNum ) override;
    virtual OUString SAL_CALL     getImlog2( const OUString& aNum ) override;
    virtual OUString SAL_CALL     getImproduct( const css::uno::Reference< css::beans::XPropertySet >& xOpt, const css::uno::Sequence< css::uno::Sequence< OUString > >& aNum1, const css::uno::Sequence< css::uno::Any >& aNumList ) override;
    virtual double SAL_CALL     getImreal( const OUString& aNum ) override;
    virtual OUString SAL_CALL     getImsin( const OUString& aNum ) override;
    virtual OUString SAL_CALL     getImsub( const OUString& aNum1, const OUString& aNum2 ) override;
    virtual OUString SAL_CALL     getImsum( const css::uno::Reference< css::beans::XPropertySet >& xOpt, const css::uno::Sequence< css::uno::Sequence< OUString > >& aNum1, const css::uno::Sequence< css::uno::Any >& aFollowingPars ) override;

    virtual OUString SAL_CALL     getImsqrt( const OUString& aNum ) override;
    virtual OUString SAL_CALL     getImtan( const OUString& aNum ) override;
    virtual OUString SAL_CALL     getImsec( const OUString& aNum ) override;
    virtual OUString SAL_CALL     getImcsc( const OUString& aNum ) override;
    virtual OUString SAL_CALL     getImcot( const OUString& aNum ) override;
    virtual OUString SAL_CALL     getImsinh( const OUString& aNum ) override;
    virtual OUString SAL_CALL     getImcosh( const OUString& aNum ) override;
    virtual OUString SAL_CALL     getImsech( const OUString& aNum ) override;
    virtual OUString SAL_CALL     getImcsch( const OUString& aNum ) override;
    virtual OUString SAL_CALL     getComplex( double fReal, double fImaginary, const css::uno::Any& rSuffix ) override;

    virtual double SAL_CALL     getConvert( double fVal, const OUString& aFromUnit, const OUString& aToUnit ) override;

    virtual double SAL_CALL     getAmordegrc( const css::uno::Reference< css::beans::XPropertySet >&, double fCost, sal_Int32 nDate, sal_Int32 nFirstPer, double fRestVal, double fPer, double fRate, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getAmorlinc( const css::uno::Reference< css::beans::XPropertySet >&, double fCost, sal_Int32 nDate, sal_Int32 nFirstPer, double fRestVal, double fPer, double fRate, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getAccrint( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nIssue, sal_Int32 nFirstInter, sal_Int32 nSettle, double fRate, const css::uno::Any& rVal, sal_Int32 nFreq, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getAccrintm( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nIssue, sal_Int32 nSettle, double fRate, const css::uno::Any& rVal, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getReceived( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fInvest, double fDisc, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getDisc( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fPrice, double fRedemp, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getDuration( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fCoup, double fYield, sal_Int32 nFreq, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getEffect( double fNominal, sal_Int32 nPeriods ) override;
    virtual double SAL_CALL     getCumprinc( double fRate, sal_Int32 nNumPeriods, double fVal, sal_Int32 nStartPer, sal_Int32 nEndPer, sal_Int32 nPayType ) override;
    virtual double SAL_CALL     getCumipmt( double fRate, sal_Int32 nNumPeriods, double fVal, sal_Int32 nStartPer, sal_Int32 nEndPer, sal_Int32 nPayType ) override;
    virtual double SAL_CALL     getPrice( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fRate, double fYield, double fRedemp, sal_Int32 nFreq, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getPricedisc( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fDisc, double fRedemp, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getPricemat( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, double fRate, double fYield, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getMduration( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fCoup, double fYield, sal_Int32 nFreq, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getNominal( double fRate, sal_Int32 nPeriods ) override;
    virtual double SAL_CALL     getDollarfr( double fDollarDec, sal_Int32 nFrac ) override;
    virtual double SAL_CALL     getDollarde( double fDollarFrac, sal_Int32 nFrac ) override;
    virtual double SAL_CALL     getYield( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fCoup, double fPrice, double fRedemp, sal_Int32 nFreq, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getYielddisc( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fPrice, double fRedemp, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getYieldmat( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, double fRate, double fPrice, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getTbilleq( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fDisc ) override;
    virtual double SAL_CALL     getTbillprice( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fDisc ) override;
    virtual double SAL_CALL     getTbillyield( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fPrice ) override;
    virtual double SAL_CALL     getOddfprice( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, sal_Int32 nFirstCoup, double fRate, double fYield, double fRedemp, sal_Int32 nFreq, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getOddfyield( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, sal_Int32 nFirstCoup, double fRate, double fPrice, double fRedemp, sal_Int32 nFreq, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getOddlprice( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nLastInterest, double fRate, double fYield, double fRedemp, sal_Int32 nFreq, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getOddlyield( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nLastInterest, double fRate, double fPrice, double fRedemp, sal_Int32 nFreq, const css::uno::Any& rOptBase) override;
    virtual double SAL_CALL     getXirr( const css::uno::Reference< css::beans::XPropertySet >& xOpt, const css::uno::Sequence< css::uno::Sequence< double > >& rValues, const css::uno::Sequence< css::uno::Sequence< sal_Int32 > >& rDates, const css::uno::Any& rGuess ) override;
    virtual double SAL_CALL     getXnpv( double fRate, const css::uno::Sequence< css::uno::Sequence< double > >& rValues, const css::uno::Sequence< css::uno::Sequence< sal_Int32 > >& rDates ) override;
    virtual double SAL_CALL     getIntrate( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fInvest, double fRedemp, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getCoupncd( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getCoupdays( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getCoupdaysnc( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getCoupdaybs( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getCouppcd( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getCoupnum( const css::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const css::uno::Any& rOptBase ) override;
    virtual double SAL_CALL     getFvschedule( double fPrinc, const css::uno::Sequence< css::uno::Sequence< double > >& rSchedule ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
