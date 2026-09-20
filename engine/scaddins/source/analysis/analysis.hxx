/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once


#include <com/sun/star/sheet/XAddIn.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sheet/addin/XAnalysis.hpp>
#include <com/sun/star/sheet/XCompatibilityNames.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <unotools/resmgr.hxx>

#include "analysishelper.hxx"

#include <memory>

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

    /// @throws cpo::uno::RuntimeException
    OUString                    GetFuncDescrStr(const TranslateId* pResId, sal_uInt16 nStrIndex);
    void                        InitDefLocales();
    inline const css::lang::Locale& GetLocale( sal_uInt32 nInd );
    void                        InitData();

                                /// Converts an Any to sal_Int32 in the range from 0 to 4 (date calculation mode).
                                ///
                                /// @throws cpo::uno::RuntimeException
                                /// @throws css::lang::IllegalArgumentException
    sal_Int32                   getDateMode(
                                    const cpo::uno::Reference< css::beans::XPropertySet >& xPropSet,
                                    const cpo::uno::Any& rAny );

public:
    explicit                    AnalysisAddIn(
                                    const cpo::uno::Reference< cpo::uno::XComponentContext >& xContext );

    OUString AnalysisResId(TranslateId aId);

    virtual                     ~AnalysisAddIn() override;

    /// @throws cpo::uno::RuntimeException
    /// @throws css::lang::IllegalArgumentException
    double                      FactDouble( sal_Int32 nNum );

                                // XAddIn
    virtual OUString     getProgrammaticFuntionName( const OUString& aDisplayName ) override;
    virtual OUString     getDisplayFunctionName( const OUString& aProgrammaticName ) override;
    virtual OUString     getFunctionDescription( const OUString& aProgrammaticName ) override;
    virtual OUString     getDisplayArgumentName( const OUString& aProgrammaticFunctionName, sal_Int32 nArgument ) override;
    virtual OUString     getArgumentDescription( const OUString& aProgrammaticFunctionName, sal_Int32 nArgument ) override;
    virtual OUString     getProgrammaticCategoryName( const OUString& aProgrammaticFunctionName ) override;
    virtual OUString     getDisplayCategoryName( const OUString& aProgrammaticFunctionName ) override;

                                // XCompatibilityNames
    virtual cpo::uno::Sequence< css::sheet::LocalizedName > getCompatibilityNames( const OUString& aProgrammaticName ) override;

                                // XLocalizable
    virtual void       setLocale( const css::lang::Locale& eLocale ) override;
    virtual css::lang::Locale getLocale(  ) override;

                                // XServiceName
    virtual OUString     getServiceName(  ) override;

                                // XServiceInfo
    virtual OUString     getImplementationName(  ) override;
    virtual bool   supportsService( const OUString& ServiceName ) override;
    virtual cpo::uno::Sequence< OUString >  getSupportedServiceNames(  ) override;

    //  methods from own interfaces start here

                            // XAnalysis
//    virtual double       get_Test( const cpo::uno::Reference< css::beans::XPropertySet >&, sal_Int32 nMode, double f1, double f2, double f3 ) throw( cpo::uno::RuntimeException );

    virtual sal_Int32  getWorkday( const cpo::uno::Reference< css::beans::XPropertySet >&, sal_Int32 nStartDate, sal_Int32 nDays, const cpo::uno::Any& aHDay ) override;
    virtual double     getYearfrac( const cpo::uno::Reference< css::beans::XPropertySet >&, sal_Int32 nStartDate, sal_Int32 nEndDate, const cpo::uno::Any& aMode ) override;
    virtual sal_Int32  getEdate( const cpo::uno::Reference< css::beans::XPropertySet >&, sal_Int32 nStartDate, sal_Int32 nMonths ) override;
    virtual sal_Int32  getWeeknum( const cpo::uno::Reference< css::beans::XPropertySet >&, sal_Int32 nStartDate, sal_Int32 nMode ) override;
    virtual sal_Int32  getEomonth( const cpo::uno::Reference< css::beans::XPropertySet >&, sal_Int32 nStartDate, sal_Int32 nMonths ) override;
    virtual sal_Int32  getNetworkdays( const cpo::uno::Reference< css::beans::XPropertySet >&, sal_Int32 nStartDate, sal_Int32 nEndDate, const cpo::uno::Any& aHDay ) override;

    virtual sal_Int32  getIseven( sal_Int32 nVal ) override;
    virtual sal_Int32  getIsodd( sal_Int32 nVal ) override;

    virtual double     getMultinomial( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, const cpo::uno::Sequence< cpo::uno::Sequence< sal_Int32 > >& aVLst, const cpo::uno::Sequence< cpo::uno::Any >& aOptVLst ) override;
    virtual double     getSeriessum( double fX, double fN, double fM, const cpo::uno::Sequence< cpo::uno::Sequence< double > >& aCoeffList ) override;
    virtual double     getQuotient( double fNum, double fDenum ) override;

    virtual double     getMround( double fNum, double fMult ) override;
    virtual double     getSqrtpi( double fNum ) override;

    virtual double     getRandbetween( double fMin, double fMax ) override;

    virtual double     getGcd( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, const cpo::uno::Sequence< cpo::uno::Sequence< double > >& aVLst, const cpo::uno::Sequence< cpo::uno::Any >& aOptVLst ) override;
    virtual double     getLcm( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, const cpo::uno::Sequence< cpo::uno::Sequence< double > >& aVLst, const cpo::uno::Sequence< cpo::uno::Any >& aOptVLst ) override;

    virtual double     getBesseli( double fNum, sal_Int32 nOrder ) override;
    virtual double     getBesselj( double fNum, sal_Int32 nOrder ) override;
    virtual double     getBesselk( double fNum, sal_Int32 nOrder ) override;
    virtual double     getBessely( double fNum, sal_Int32 nOrder ) override;

    virtual OUString     getBin2Oct( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, const OUString& aNum, const cpo::uno::Any& rPlaces ) override;
    virtual double     getBin2Dec( const OUString& aNum ) override;
    virtual OUString     getBin2Hex( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, const OUString& aNum, const cpo::uno::Any& rPlaces ) override;

    virtual OUString     getOct2Bin( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, const OUString& aNum, const cpo::uno::Any& rPlaces ) override;
    virtual double     getOct2Dec( const OUString& aNum ) override;
    virtual OUString     getOct2Hex( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, const OUString& aNum, const cpo::uno::Any& rPlaces ) override;

    virtual OUString     getDec2Bin( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 fNum, const cpo::uno::Any& rPlaces ) override;
    virtual OUString     getDec2Oct( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 fNum, const cpo::uno::Any& rPlaces ) override;
    virtual OUString     getDec2Hex( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, double fNum, const cpo::uno::Any& rPlaces ) override;

    virtual OUString     getHex2Bin( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, const OUString& aNum, const cpo::uno::Any& rPlaces ) override;
    virtual double     getHex2Dec( const OUString& aNum ) override;
    virtual OUString     getHex2Oct( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, const OUString& aNum, const cpo::uno::Any& rPlaces ) override;

    virtual sal_Int32  getDelta( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, double fNum1, const cpo::uno::Any& rNum2 ) override;

    virtual double     getErf( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, double fLowerLimit, const cpo::uno::Any& rUpperLimit ) override;
    virtual double     getErfc( double fLowerLimit ) override;

    virtual sal_Int32  getGestep( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, double fNum, const cpo::uno::Any& rStep ) override;

    virtual double     getFactdouble( sal_Int32 nNum ) override;

    virtual double     getImabs( const OUString& aNum ) override;
    virtual double     getImaginary( const OUString& aNum ) override;
    virtual OUString     getImpower( const OUString& aNum, double fPower ) override;
    virtual double     getImargument( const OUString& aNum ) override;
    virtual OUString     getImcos( const OUString& aNum ) override;
    virtual OUString     getImdiv( const OUString& aDivident, const OUString& aDivisor ) override;
    virtual OUString     getImexp( const OUString& aNum ) override;
    virtual OUString     getImconjugate( const OUString& aNum ) override;
    virtual OUString     getImln( const OUString& aNum ) override;
    virtual OUString     getImlog10( const OUString& aNum ) override;
    virtual OUString     getImlog2( const OUString& aNum ) override;
    virtual OUString     getImproduct( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, const cpo::uno::Sequence< cpo::uno::Sequence< OUString > >& aNum1, const cpo::uno::Sequence< cpo::uno::Any >& aNumList ) override;
    virtual double     getImreal( const OUString& aNum ) override;
    virtual OUString     getImsin( const OUString& aNum ) override;
    virtual OUString     getImsub( const OUString& aNum1, const OUString& aNum2 ) override;
    virtual OUString     getImsum( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, const cpo::uno::Sequence< cpo::uno::Sequence< OUString > >& aNum1, const cpo::uno::Sequence< cpo::uno::Any >& aFollowingPars ) override;

    virtual OUString     getImsqrt( const OUString& aNum ) override;
    virtual OUString     getImtan( const OUString& aNum ) override;
    virtual OUString     getImsec( const OUString& aNum ) override;
    virtual OUString     getImcsc( const OUString& aNum ) override;
    virtual OUString     getImcot( const OUString& aNum ) override;
    virtual OUString     getImsinh( const OUString& aNum ) override;
    virtual OUString     getImcosh( const OUString& aNum ) override;
    virtual OUString     getImsech( const OUString& aNum ) override;
    virtual OUString     getImcsch( const OUString& aNum ) override;
    virtual OUString     getComplex( double fReal, double fImaginary, const cpo::uno::Any& rSuffix ) override;

    virtual double     getConvert( double fVal, const OUString& aFromUnit, const OUString& aToUnit ) override;

    virtual double     getAmordegrc( const cpo::uno::Reference< css::beans::XPropertySet >&, double fCost, sal_Int32 nDate, sal_Int32 nFirstPer, double fRestVal, double fPer, double fRate, const cpo::uno::Any& rOptBase ) override;
    virtual double     getAmorlinc( const cpo::uno::Reference< css::beans::XPropertySet >&, double fCost, sal_Int32 nDate, sal_Int32 nFirstPer, double fRestVal, double fPer, double fRate, const cpo::uno::Any& rOptBase ) override;
    virtual double     getAccrint( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nIssue, sal_Int32 nFirstInter, sal_Int32 nSettle, double fRate, const cpo::uno::Any& rVal, sal_Int32 nFreq, const cpo::uno::Any& rOptBase ) override;
    virtual double     getAccrintm( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nIssue, sal_Int32 nSettle, double fRate, const cpo::uno::Any& rVal, const cpo::uno::Any& rOptBase ) override;
    virtual double     getReceived( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fInvest, double fDisc, const cpo::uno::Any& rOptBase ) override;
    virtual double     getDisc( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fPrice, double fRedemp, const cpo::uno::Any& rOptBase ) override;
    virtual double     getDuration( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fCoup, double fYield, sal_Int32 nFreq, const cpo::uno::Any& rOptBase ) override;
    virtual double     getEffect( double fNominal, sal_Int32 nPeriods ) override;
    virtual double     getCumprinc( double fRate, sal_Int32 nNumPeriods, double fVal, sal_Int32 nStartPer, sal_Int32 nEndPer, sal_Int32 nPayType ) override;
    virtual double     getCumipmt( double fRate, sal_Int32 nNumPeriods, double fVal, sal_Int32 nStartPer, sal_Int32 nEndPer, sal_Int32 nPayType ) override;
    virtual double     getPrice( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fRate, double fYield, double fRedemp, sal_Int32 nFreq, const cpo::uno::Any& rOptBase ) override;
    virtual double     getPricedisc( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fDisc, double fRedemp, const cpo::uno::Any& rOptBase ) override;
    virtual double     getPricemat( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, double fRate, double fYield, const cpo::uno::Any& rOptBase ) override;
    virtual double     getMduration( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fCoup, double fYield, sal_Int32 nFreq, const cpo::uno::Any& rOptBase ) override;
    virtual double     getNominal( double fRate, sal_Int32 nPeriods ) override;
    virtual double     getDollarfr( double fDollarDec, sal_Int32 nFrac ) override;
    virtual double     getDollarde( double fDollarFrac, sal_Int32 nFrac ) override;
    virtual double     getYield( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fCoup, double fPrice, double fRedemp, sal_Int32 nFreq, const cpo::uno::Any& rOptBase ) override;
    virtual double     getYielddisc( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fPrice, double fRedemp, const cpo::uno::Any& rOptBase ) override;
    virtual double     getYieldmat( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, double fRate, double fPrice, const cpo::uno::Any& rOptBase ) override;
    virtual double     getTbilleq( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fDisc ) override;
    virtual double     getTbillprice( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fDisc ) override;
    virtual double     getTbillyield( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fPrice ) override;
    virtual double     getOddfprice( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, sal_Int32 nFirstCoup, double fRate, double fYield, double fRedemp, sal_Int32 nFreq, const cpo::uno::Any& rOptBase ) override;
    virtual double     getOddfyield( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nIssue, sal_Int32 nFirstCoup, double fRate, double fPrice, double fRedemp, sal_Int32 nFreq, const cpo::uno::Any& rOptBase ) override;
    virtual double     getOddlprice( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nLastInterest, double fRate, double fYield, double fRedemp, sal_Int32 nFreq, const cpo::uno::Any& rOptBase ) override;
    virtual double     getOddlyield( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nLastInterest, double fRate, double fPrice, double fRedemp, sal_Int32 nFreq, const cpo::uno::Any& rOptBase) override;
    virtual double     getXirr( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, const cpo::uno::Sequence< cpo::uno::Sequence< double > >& rValues, const cpo::uno::Sequence< cpo::uno::Sequence< sal_Int32 > >& rDates, const cpo::uno::Any& rGuess ) override;
    virtual double     getXnpv( double fRate, const cpo::uno::Sequence< cpo::uno::Sequence< double > >& rValues, const cpo::uno::Sequence< cpo::uno::Sequence< sal_Int32 > >& rDates ) override;
    virtual double     getIntrate( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, double fInvest, double fRedemp, const cpo::uno::Any& rOptBase ) override;
    virtual double     getCoupncd( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const cpo::uno::Any& rOptBase ) override;
    virtual double     getCoupdays( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const cpo::uno::Any& rOptBase ) override;
    virtual double     getCoupdaysnc( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const cpo::uno::Any& rOptBase ) override;
    virtual double     getCoupdaybs( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const cpo::uno::Any& rOptBase ) override;
    virtual double     getCouppcd( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const cpo::uno::Any& rOptBase ) override;
    virtual double     getCoupnum( const cpo::uno::Reference< css::beans::XPropertySet >& xOpt, sal_Int32 nSettle, sal_Int32 nMat, sal_Int32 nFreq, const cpo::uno::Any& rOptBase ) override;
    virtual double     getFvschedule( double fPrinc, const cpo::uno::Sequence< cpo::uno::Sequence< double > >& rSchedule ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
