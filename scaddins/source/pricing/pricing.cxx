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

// pricing functions add in

// all of the UNO add-in technical details have been copied from
// ../datefunc/datefunc.cxx

#include "pricing.hxx"
#include "black_scholes.hxx"
#include "pricing.hrc"

#include <cppuhelper/factory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <iostream>
#include <algorithm>
#include <osl/diagnose.h>
#include <rtl/math.hxx>
#include <rtl/ustrbuf.hxx>
#include <tools/rcid.h>
#include <tools/resmgr.hxx>

using namespace ::com::sun::star;
using namespace sca::pricing;


#define ADDIN_SERVICE           "com.sun.star.sheet.AddIn"
#define MY_SERVICE              "com.sun.star.sheet.addin.PricingFunctions"
#define MY_IMPLNAME             "com.sun.star.sheet.addin.PricingFunctionsImpl"

ScaResId::ScaResId( sal_uInt16 nId, ResMgr& rResMgr ) :
    ResId( nId, rResMgr )
{
}

#define UNIQUE              false   // function name does not exist in Calc

#define STDPAR              false   // all parameters are described

#define FUNCDATA( FuncName, ParamCount, Category, Double, IntPar )  \
    { "get" #FuncName, PRICING_FUNCNAME_##FuncName, PRICING_FUNCDESC_##FuncName, PRICING_DEFFUNCNAME_##FuncName, ParamCount, Category, Double, IntPar }

const ScaFuncDataBase pFuncDataArr[] =
{
   FUNCDATA( OptBarrier,       13, ScaCategory::Finance,    UNIQUE,  STDPAR),
   FUNCDATA( OptTouch,         11, ScaCategory::Finance,    UNIQUE,  STDPAR),
   FUNCDATA( OptProbHit,        6, ScaCategory::Finance,    UNIQUE,  STDPAR),
   FUNCDATA( OptProbInMoney,    8, ScaCategory::Finance,    UNIQUE,  STDPAR)
};

#undef FUNCDATA

ScaFuncData::ScaFuncData( const ScaFuncDataBase& rBaseData, ResMgr& rResMgr ) :
    aIntName( OUString::createFromAscii( rBaseData.pIntName ) ),
    nUINameID( rBaseData.nUINameID ),
    nDescrID( rBaseData.nDescrID ),
    nCompListID( rBaseData.nCompListID ),
    nParamCount( rBaseData.nParamCount ),
    eCat( rBaseData.eCat ),
    bDouble( rBaseData.bDouble ),
    bWithOpt( rBaseData.bWithOpt )
{
    ScaResStringArrLoader aArrLoader( RID_PRICING_DEFFUNCTION_NAMES, nCompListID, rResMgr );
    const ResStringArray& rArr = aArrLoader.GetStringArray();

    for( sal_uInt32 nIndex = 0; nIndex < rArr.Count(); nIndex++ )
        aCompList.push_back( rArr.GetString( nIndex ) );
}

ScaFuncData::~ScaFuncData()
{
}

sal_uInt16 ScaFuncData::GetStrIndex( sal_uInt16 nParam ) const
{
    if( !bWithOpt )
        nParam++;
    return (nParam > nParamCount) ? (nParamCount * 2) : (nParam * 2);
}

void sca::pricing::InitScaFuncDataList( ScaFuncDataList& rList, ResMgr& rResMgr )
{
    for(const auto & nIndex : pFuncDataArr)
        rList.push_back( ScaFuncData( nIndex, rResMgr ) ) ;
}

ScaFuncRes::ScaFuncRes( ResId& rResId, ResMgr& rResMgr, sal_uInt16 nIndex, OUString& rRet ) :
    Resource( rResId )
{
    rRet = ScaResId(nIndex, rResMgr).toString();
    FreeResource();
}

// entry points for service registration / instantiation
uno::Reference< uno::XInterface > SAL_CALL ScaPricingAddIn_CreateInstance(
        const uno::Reference< lang::XMultiServiceFactory >& )
{
    return static_cast<cppu::OWeakObject*>(new ScaPricingAddIn());
}

extern "C" {

SAL_DLLPUBLIC_EXPORT void * SAL_CALL pricing_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void* pRet = nullptr;

    if ( pServiceManager &&
            OUString::createFromAscii( pImplName ) == ScaPricingAddIn::getImplementationName_Static() )
    {
        uno::Reference< lang::XSingleServiceFactory > xFactory( cppu::createOneInstanceFactory(
                static_cast< lang::XMultiServiceFactory* >( pServiceManager ),
                ScaPricingAddIn::getImplementationName_Static(),
                ScaPricingAddIn_CreateInstance,
                ScaPricingAddIn::getSupportedServiceNames_Static() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}

}   // extern C

//  "normal" service implementation
ScaPricingAddIn::ScaPricingAddIn() :
    pDefLocales( nullptr ),
    pResMgr( nullptr ),
    pFuncDataList( nullptr )
{
}

ScaPricingAddIn::~ScaPricingAddIn()
{
    delete pFuncDataList;
    delete pResMgr;
    delete[] pDefLocales;
}

static const sal_Char*  pLang[] = { "de", "en" };
static const sal_Char*  pCoun[] = { "DE", "US" };
static const sal_uInt32 nNumOfLoc = SAL_N_ELEMENTS( pLang );

void ScaPricingAddIn::InitDefLocales()
{
    pDefLocales = new lang::Locale[ nNumOfLoc ];

    for( sal_uInt32 nIndex = 0; nIndex < nNumOfLoc; nIndex++ )
    {
        pDefLocales[ nIndex ].Language = OUString::createFromAscii( pLang[ nIndex ] );
        pDefLocales[ nIndex ].Country = OUString::createFromAscii( pCoun[ nIndex ] );
    }
}

const lang::Locale& ScaPricingAddIn::GetLocale( sal_uInt32 nIndex )
{
    if( !pDefLocales )
        InitDefLocales();

    return (nIndex < sizeof( pLang )) ? pDefLocales[ nIndex ] : aFuncLoc;
}

ResMgr& ScaPricingAddIn::GetResMgr()
{
    if( !pResMgr )
    {
        InitData();     // try to get resource manager
        if( !pResMgr )
            throw uno::RuntimeException();
    }
    return *pResMgr;
}

void ScaPricingAddIn::InitData()
{
    delete pResMgr;
    pResMgr = ResMgr::CreateResMgr("pricing", LanguageTag( aFuncLoc) );
    delete pFuncDataList;

    if(pResMgr)
    {
        pFuncDataList = new ScaFuncDataList;
        InitScaFuncDataList( *pFuncDataList, *pResMgr );
    }
    else
    {
        pFuncDataList = nullptr;
    }

    if( pDefLocales )
    {
        delete pDefLocales;
        pDefLocales = nullptr;
    }
}

OUString ScaPricingAddIn::GetFuncDescrStr( sal_uInt16 nResId, sal_uInt16 nStrIndex )
{
    OUString aRet;

    ScaResPublisher aResPubl( ScaResId( RID_PRICING_FUNCTION_DESCRIPTIONS, GetResMgr() ) );
    ScaResId aResId( nResId, GetResMgr() );
    aResId.SetRT( RSC_RESOURCE );

    if( aResPubl.IsAvailableRes( aResId ) )
        ScaFuncRes aSubRes( aResId, GetResMgr(), nStrIndex, aRet );

    aResPubl.FreeResource();
    return aRet;
}

OUString ScaPricingAddIn::getImplementationName_Static()
{
    return OUString( MY_IMPLNAME );
}

uno::Sequence< OUString > ScaPricingAddIn::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aRet( 2 );
    OUString* pArray = aRet.getArray();
    pArray[0] = ADDIN_SERVICE;
    pArray[1] = MY_SERVICE;
    return aRet;
}

// XServiceName
OUString SAL_CALL ScaPricingAddIn::getServiceName()
{
    // name of specific AddIn service
    return OUString( MY_SERVICE );
}

// XServiceInfo
OUString SAL_CALL ScaPricingAddIn::getImplementationName()
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL ScaPricingAddIn::supportsService( const OUString& aServiceName )
{
    return cppu::supportsService(this, aServiceName);
}

uno::Sequence< OUString > SAL_CALL ScaPricingAddIn::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}

// XLocalizable
void SAL_CALL ScaPricingAddIn::setLocale( const lang::Locale& eLocale )
{
    aFuncLoc = eLocale;
    InitData();     // change of locale invalidates resources!
}

lang::Locale SAL_CALL ScaPricingAddIn::getLocale()
{
    return aFuncLoc;
}

// function descriptions start here
// XAddIn
OUString SAL_CALL ScaPricingAddIn::getProgrammaticFuntionName( const OUString& )
{
    //  not used by calc
    //  (but should be implemented for other uses of the AddIn service)
    return OUString();
}

OUString SAL_CALL ScaPricingAddIn::getDisplayFunctionName( const OUString& aProgrammaticName )
{
    OUString aRet;

    auto fDataIt = std::find_if(pFuncDataList->begin(), pFuncDataList->end(),
                                FindScaFuncData( aProgrammaticName ) );
    if(fDataIt != pFuncDataList->end() )
    {
        aRet = ScaResStringLoader( RID_PRICING_FUNCTION_NAMES, fDataIt->GetUINameID(), GetResMgr() ).GetString();
        if( fDataIt->IsDouble() )
            aRet += "_ADD";
    }
    else
    {
        aRet = "UNKNOWNFUNC_";
        aRet += aProgrammaticName;
    }

    return aRet;
}

OUString SAL_CALL ScaPricingAddIn::getFunctionDescription( const OUString& aProgrammaticName )
{
    OUString aRet;

    auto fDataIt = std::find_if( pFuncDataList->begin(), pFuncDataList->end(),
                                 FindScaFuncData( aProgrammaticName ) );
    if( fDataIt != pFuncDataList->end() )
        aRet = GetFuncDescrStr( fDataIt->GetDescrID(), 1 );

    return aRet;
}

OUString SAL_CALL ScaPricingAddIn::getDisplayArgumentName(
        const OUString& aProgrammaticName, sal_Int32 nArgument )
{
    OUString aRet;

    auto fDataIt = std::find_if( pFuncDataList->begin(), pFuncDataList->end(),
                                 FindScaFuncData( aProgrammaticName ) );
    if( fDataIt != pFuncDataList->end() && (nArgument <= 0xFFFF) )
    {
        sal_uInt16 nStr = fDataIt->GetStrIndex( static_cast< sal_uInt16 >( nArgument ) );
        if( nStr )
            aRet = GetFuncDescrStr( fDataIt->GetDescrID(), nStr );
        else
            aRet = "internal";
    }

    return aRet;
}

OUString SAL_CALL ScaPricingAddIn::getArgumentDescription(
        const OUString& aProgrammaticName, sal_Int32 nArgument )
{
    OUString aRet;

    auto fDataIt = std::find_if( pFuncDataList->begin(), pFuncDataList->end(),
                                 FindScaFuncData( aProgrammaticName ) );
    if( fDataIt != pFuncDataList->end() && (nArgument <= 0xFFFF) )
    {
        sal_uInt16 nStr = fDataIt->GetStrIndex( static_cast< sal_uInt16 >( nArgument ) );
        if( nStr )
            aRet = GetFuncDescrStr( fDataIt->GetDescrID(), nStr + 1 );
        else
            aRet = "for internal use only";
    }

    return aRet;
}

OUString SAL_CALL ScaPricingAddIn::getProgrammaticCategoryName(
        const OUString& aProgrammaticName )
{
    OUString aRet;

    auto fDataIt = std::find_if( pFuncDataList->begin(), pFuncDataList->end(),
                                 FindScaFuncData( aProgrammaticName ) );
    if( fDataIt != pFuncDataList->end() )
    {
        switch( fDataIt->GetCategory() )
        {
            case ScaCategory::DateTime:   aRet = "Date&Time";    break;
            case ScaCategory::Text:       aRet = "Text";         break;
            case ScaCategory::Finance:    aRet = "Financial";    break;
            case ScaCategory::Inf:        aRet = "Information";  break;
            case ScaCategory::Math:       aRet = "Mathematical"; break;
            case ScaCategory::Tech:       aRet = "Technical";    break;
        }
    }

    if( aRet.isEmpty() )
        aRet = "Add-In";
    return aRet;
}

OUString SAL_CALL ScaPricingAddIn::getDisplayCategoryName(
        const OUString& aProgrammaticName )
{
    return getProgrammaticCategoryName( aProgrammaticName );
}

// XCompatibilityNames
uno::Sequence< sheet::LocalizedName > SAL_CALL ScaPricingAddIn::getCompatibilityNames(
        const OUString& aProgrammaticName )
{
    auto fDataIt = std::find_if( pFuncDataList->begin(), pFuncDataList->end(),
                                 FindScaFuncData( aProgrammaticName ) );
    if( fDataIt == pFuncDataList->end() )
        return uno::Sequence< sheet::LocalizedName >( 0 );

    const std::vector<OUString>& rStrList = fDataIt->GetCompNameList();
    sal_uInt32 nCount = rStrList.size();

    uno::Sequence< sheet::LocalizedName > aRet( nCount );
    sheet::LocalizedName* pArray = aRet.getArray();

    for( sal_uInt32 nIndex = 0; nIndex < nCount; nIndex++ )
        pArray[ nIndex ] = sheet::LocalizedName( GetLocale( nIndex ), rStrList[nIndex] );

    return aRet;
}

// actual function implementation starts here
// auxiliary input handling functions
namespace {

bool getinput_putcall(bs::types::PutCall& pc, const OUString& str) {
    if(str.startsWith("c")) {
        pc=bs::types::Call;
    } else if(str.startsWith("p")) {
        pc=bs::types::Put;
    } else {
        return false;
    }
    return true;
}

bool getinput_putcall(bs::types::PutCall& pc, const uno::Any& anyval) {
    OUString str;
    if(anyval.getValueTypeClass() == uno::TypeClass_STRING) {
        anyval >>= str;
    } else if(anyval.getValueTypeClass() == uno::TypeClass_VOID) {
        str="c";        // call as default
    } else {
        return false;
    }
    return getinput_putcall(pc, str);
}

bool getinput_strike(double& strike, const uno::Any& anyval) {
    if(anyval.getValueTypeClass() == uno::TypeClass_DOUBLE) {
        anyval >>= strike;
    } else if(anyval.getValueTypeClass() == uno::TypeClass_VOID) {
        strike=-1.0;        // -1 as default (means not set)
    } else {
        return false;
    }
    return true;
}

bool getinput_inout(bs::types::BarrierKIO& kio, const OUString& str) {
    if(str.startsWith("i")) {
        kio=bs::types::KnockIn;
    } else if(str.startsWith("o")) {
        kio=bs::types::KnockOut;
    } else {
        return false;
    }
    return true;
}

bool getinput_barrier(bs::types::BarrierActive& cont, const OUString& str) {
    if(str.startsWith("c")) {
        cont=bs::types::Continuous;
    } else if(str.startsWith("e")) {
        cont=bs::types::Maturity;
    } else {
        return false;
    }
    return true;
}

bool getinput_fordom(bs::types::ForDom& fd, const OUString& str) {
    if(str.startsWith("f")) {
        fd=bs::types::Foreign;
    } else if(str.startsWith("d")) {
        fd=bs::types::Domestic;
    } else {
        return false;
    }
    return true;
}

bool getinput_greek(bs::types::Greeks& greek, const uno::Any& anyval) {
    OUString str;
    if(anyval.getValueTypeClass() == uno::TypeClass_STRING) {
        anyval >>= str;
    } else if(anyval.getValueTypeClass() == uno::TypeClass_VOID) {
        str="value";
    } else {
        return false;
    }

    if(str == "value" || str == "price" || str == "v" || str == "p") {
        greek=bs::types::Value;
    } else if(str == "delta" || str == "d") {
        greek=bs::types::Delta;
    } else if(str == "gamma" || str == "g") {
        greek=bs::types::Gamma;
    } else if(str == "theta" || str == "t") {
        greek=bs::types::Theta;
    } else if(str == "vega" || str == "e") {
        greek=bs::types::Vega;
    } else if(str == "volga" || str == "o") {
        greek=bs::types::Volga;
    } else if(str == "vanna" || str == "a") {
        greek=bs::types::Vanna;
    } else if(str == "rho" || str == "r") {
        greek=bs::types::Rho_d;
    } else if(str == "rhof" || str == "f") {
        greek=bs::types::Rho_f;
    } else {
        return false;
    }
    return true;
}

} // namespace for auxiliary functions

// OPT_BARRIER(...)
double SAL_CALL ScaPricingAddIn::getOptBarrier( double spot, double vol,
            double r, double rf, double T, double strike,
            double barrier_low, double barrier_up, double rebate,
            const OUString& put_call, const OUString& in_out,
            const OUString& barriercont, const uno::Any& greekstr )
{
    bs::types::PutCall pc;
    bs::types::BarrierKIO kio;
    bs::types::BarrierActive bcont;
    bs::types::Greeks greek;
    // read and check input values
    if( spot<=0.0 || vol<=0.0 || T<0.0 || strike<0.0 ||
                !getinput_putcall(pc,put_call) ||
                !getinput_inout(kio,in_out) ||
                !getinput_barrier(bcont,barriercont) ||
                !getinput_greek(greek,greekstr) ){
        throw lang::IllegalArgumentException();
    }

    double fRet=bs::barrier(spot,vol,r,rf,T,strike, barrier_low,barrier_up,
                            rebate,pc,kio,bcont,greek);

    RETURN_FINITE( fRet );
}

// OPT_TOUCH(...)
double SAL_CALL ScaPricingAddIn::getOptTouch( double spot, double vol,
            double r, double rf, double T,
            double barrier_low, double barrier_up,
            const OUString& for_dom, const OUString& in_out,
            const OUString& barriercont, const uno::Any& greekstr )
{
    bs::types::ForDom fd;
    bs::types::BarrierKIO kio;
    bs::types::BarrierActive bcont;
    bs::types::Greeks greek;
    // read and check input values
    if( spot<=0.0 || vol<=0.0 || T<0.0 ||
                !getinput_fordom(fd,for_dom) ||
                !getinput_inout(kio,in_out) ||
                !getinput_barrier(bcont,barriercont) ||
                !getinput_greek(greek,greekstr) ){
        throw lang::IllegalArgumentException();
    }

    double fRet=bs::touch(spot,vol,r,rf,T,barrier_low,barrier_up,
                            fd,kio,bcont,greek);

    RETURN_FINITE( fRet );
}

// OPT_PRB_HIT(...)
double SAL_CALL ScaPricingAddIn::getOptProbHit( double spot, double vol,
            double mu, double T,
            double barrier_low, double barrier_up )
{
    // read and check input values
    if( spot<=0.0 || vol<=0.0 || T<0.0 ) {
        throw lang::IllegalArgumentException();
    }

    double fRet=bs::prob_hit(spot,vol,mu,T,barrier_low,barrier_up);

    RETURN_FINITE( fRet );
}

// OPT_PROB_INMONEY(...)
double SAL_CALL ScaPricingAddIn::getOptProbInMoney( double spot, double vol,
            double mu, double T,
            double barrier_low, double barrier_up,
            const uno::Any& strikeval, const uno::Any& put_call )
{
    bs::types::PutCall pc=bs::types::Call;
    double  K;

    // read and check input values
    if( spot<=0.0 || vol<=0.0 || T<0.0 ||
            !getinput_putcall(pc,put_call) ||
            !getinput_strike(K,strikeval) ) {
        throw lang::IllegalArgumentException();
    }

    double fRet=bs::prob_in_money(spot,vol,mu,T,K,barrier_low,barrier_up,pc);

    RETURN_FINITE( fRet );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
