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

//------------------------------------------------------------------
//
// pricing functions add in
//
// all of the UNO add-in technical details have been copied from
// ../datefunc/datefunc.cxx
//
//------------------------------------------------------------------

#include "pricing.hxx"
#include "black_scholes.hxx"
#include "pricing.hrc"
#include <cppuhelper/factory.hxx>
#include <osl/diagnose.h>
#include <rtl/ustrbuf.hxx>
#include <rtl/math.hxx>
#include <tools/resmgr.hxx>
#include <tools/rcid.h>

#include <iostream>

using namespace ::com::sun::star;
using namespace sca::pricing;


//------------------------------------------------------------------

#define ADDIN_SERVICE           "com.sun.star.sheet.AddIn"
#define MY_SERVICE              "com.sun.star.sheet.addin.PricingFunctions"
#define MY_IMPLNAME             "com.sun.star.sheet.addin.PricingFunctionsImpl"

//------------------------------------------------------------------

#define STR_FROM_ANSI( s )      OUString( s, strlen( s ), RTL_TEXTENCODING_MS_1252 )

//------------------------------------------------------------------

const sal_uInt32 ScaList::nStartSize = 16;
const sal_uInt32 ScaList::nIncrSize = 16;

ScaList::ScaList() :
    pData( new void*[ nStartSize ] ),
    nSize( nStartSize ),
    nCount( 0 ),
    nCurr( 0 )
{
}

ScaList::~ScaList()
{
    delete[] pData;
}

void ScaList::_Grow()
{
    nSize += nIncrSize;

    void** pNewData = new void*[ nSize ];
    memcpy( pNewData, pData, nCount * sizeof( void* ) );

    delete[] pData;
    pData = pNewData;
}

//------------------------------------------------------------------

ScaStringList::~ScaStringList()
{
    for( OUString* pStr = First(); pStr; pStr = Next() )
        delete pStr;
}

//------------------------------------------------------------------

ScaResId::ScaResId( sal_uInt16 nId, ResMgr& rResMgr ) :
    ResId( nId, rResMgr )
{
}


//------------------------------------------------------------------

#define UNIQUE              sal_False   // function name does not exist in Calc

#define STDPAR              sal_False   // all parameters are described

#define FUNCDATA( FuncName, ParamCount, Category, Double, IntPar )  \
    { "get" #FuncName, PRICING_FUNCNAME_##FuncName, PRICING_FUNCDESC_##FuncName, PRICING_DEFFUNCNAME_##FuncName, ParamCount, Category, Double, IntPar }

const ScaFuncDataBase pFuncDataArr[] =
{
   FUNCDATA( OptBarrier,       13, ScaCat_Finance,    UNIQUE,  STDPAR),
   FUNCDATA( OptTouch,         11, ScaCat_Finance,    UNIQUE,  STDPAR),
   FUNCDATA( OptProbHit,        6, ScaCat_Finance,    UNIQUE,  STDPAR),
   FUNCDATA( OptProbInMoney,    8, ScaCat_Finance,    UNIQUE,  STDPAR)
};

#undef FUNCDATA


//------------------------------------------------------------------

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

    for( sal_uInt16 nIndex = 0; nIndex < rArr.Count(); nIndex++ )
        aCompList.Append( rArr.GetString( nIndex ) );
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


//------------------------------------------------------------------

ScaFuncDataList::ScaFuncDataList( ResMgr& rResMgr ) :
    nLast( 0xFFFFFFFF )
{
    for( sal_uInt16 nIndex = 0; nIndex < SAL_N_ELEMENTS(pFuncDataArr); nIndex++ )
        Append( new ScaFuncData( pFuncDataArr[ nIndex ], rResMgr ) );
}

ScaFuncDataList::~ScaFuncDataList()
{
    for( ScaFuncData* pFData = First(); pFData; pFData = Next() )
        delete pFData;
}

const ScaFuncData* ScaFuncDataList::Get( const OUString& rProgrammaticName ) const
{
    if( aLastName == rProgrammaticName ){
        return Get( nLast );
    }

    for( sal_uInt32 nIndex = 0; nIndex < Count(); nIndex++ )
    {
        const ScaFuncData* pCurr = Get( nIndex );
        if( pCurr->Is( rProgrammaticName ) )
        {
            const_cast< ScaFuncDataList* >( this )->aLastName = rProgrammaticName;
            const_cast< ScaFuncDataList* >( this )->nLast = nIndex;
            return pCurr;
        }
    }
    return NULL;
}


//------------------------------------------------------------------

ScaFuncRes::ScaFuncRes( ResId& rResId, ResMgr& rResMgr, sal_uInt16 nIndex, OUString& rRet ) :
    Resource( rResId )
{
    rRet = ScaResId(nIndex, rResMgr).toString();
    FreeResource();
}


//------------------------------------------------------------------
//
//  entry points for service registration / instantiation
//
//------------------------------------------------------------------

uno::Reference< uno::XInterface > SAL_CALL ScaPricingAddIn_CreateInstance(
        const uno::Reference< lang::XMultiServiceFactory >& )
{
    static uno::Reference< uno::XInterface > xInst = (cppu::OWeakObject*) new ScaPricingAddIn();
    return xInst;
}


//------------------------------------------------------------------------

extern "C" {

SAL_DLLPUBLIC_EXPORT void * SAL_CALL pricing_component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * /*pRegistryKey*/ )
{
    void* pRet = 0;

    if ( pServiceManager &&
            OUString::createFromAscii( pImplName ) == ScaPricingAddIn::getImplementationName_Static() )
    {
        uno::Reference< lang::XSingleServiceFactory > xFactory( cppu::createOneInstanceFactory(
                reinterpret_cast< lang::XMultiServiceFactory* >( pServiceManager ),
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

//------------------------------------------------------------------------
//
//  "normal" service implementation
//
//------------------------------------------------------------------------

ScaPricingAddIn::ScaPricingAddIn() :
    pDefLocales( NULL ),
    pResMgr( NULL ),
    pFuncDataList( NULL )
{
}

ScaPricingAddIn::~ScaPricingAddIn()
{
    if( pFuncDataList )
        delete pFuncDataList;
    if( pDefLocales )
        delete[] pDefLocales;

    // pResMgr already deleted (_all_ resource managers are deleted _before_ this dtor is called)
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

ResMgr& ScaPricingAddIn::GetResMgr() throw( uno::RuntimeException )
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

    if( pResMgr )
        delete pResMgr;

    OString aModName( "pricing" );
    pResMgr = ResMgr::CreateResMgr( aModName.getStr(), LanguageTag( aFuncLoc) );

    if( pFuncDataList )
        delete pFuncDataList;

    pFuncDataList = pResMgr ? new ScaFuncDataList( *pResMgr ) : NULL;

    if( pDefLocales )
    {
        delete pDefLocales;
        pDefLocales = NULL;
    }
}

OUString ScaPricingAddIn::GetDisplFuncStr( sal_uInt16 nResId ) throw( uno::RuntimeException )
{
    return ScaResStringLoader( RID_PRICING_FUNCTION_NAMES, nResId, GetResMgr() ).GetString();
}

OUString ScaPricingAddIn::GetFuncDescrStr( sal_uInt16 nResId, sal_uInt16 nStrIndex ) throw( uno::RuntimeException )
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


//------------------------------------------------------------------------

OUString ScaPricingAddIn::getImplementationName_Static()
{
    return OUString( MY_IMPLNAME );
}

uno::Sequence< OUString > ScaPricingAddIn::getSupportedServiceNames_Static()
{
    uno::Sequence< OUString > aRet( 2 );
    OUString* pArray = aRet.getArray();
    pArray[0] = OUString( ADDIN_SERVICE );
    pArray[1] = OUString( MY_SERVICE );
    return aRet;
}

// XServiceName

OUString SAL_CALL ScaPricingAddIn::getServiceName() throw( uno::RuntimeException )
{
    // name of specific AddIn service
    return OUString( MY_SERVICE );
}

// XServiceInfo

OUString SAL_CALL ScaPricingAddIn::getImplementationName() throw( uno::RuntimeException )
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL ScaPricingAddIn::supportsService( const OUString& aServiceName ) throw( uno::RuntimeException )
{
    return aServiceName == ADDIN_SERVICE || aServiceName == MY_SERVICE;
}

uno::Sequence< OUString > SAL_CALL ScaPricingAddIn::getSupportedServiceNames() throw( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

// XLocalizable

void SAL_CALL ScaPricingAddIn::setLocale( const lang::Locale& eLocale ) throw( uno::RuntimeException )
{
    aFuncLoc = eLocale;
    InitData();     // change of locale invalidates resources!
}

lang::Locale SAL_CALL ScaPricingAddIn::getLocale() throw( uno::RuntimeException )
{
    return aFuncLoc;
}

//------------------------------------------------------------------
//
//  function descriptions start here
//
//------------------------------------------------------------------

// XAddIn
OUString SAL_CALL ScaPricingAddIn::getProgrammaticFuntionName( const OUString& ) throw( uno::RuntimeException )
{
    //  not used by calc
    //  (but should be implemented for other uses of the AddIn service)
    return OUString();
}

OUString SAL_CALL ScaPricingAddIn::getDisplayFunctionName( const OUString& aProgrammaticName ) throw( uno::RuntimeException )
{
    OUString aRet;

    const ScaFuncData* pFData = pFuncDataList->Get( aProgrammaticName );
    if( pFData )
    {
        aRet = GetDisplFuncStr( pFData->GetUINameID() );
        if( pFData->IsDouble() )
            aRet += STR_FROM_ANSI( "_ADD" );
    }
    else
    {
        aRet = STR_FROM_ANSI( "UNKNOWNFUNC_" );
        aRet += aProgrammaticName;
    }

    return aRet;
}

OUString SAL_CALL ScaPricingAddIn::getFunctionDescription( const OUString& aProgrammaticName ) throw( uno::RuntimeException )
{
    OUString aRet;

    const ScaFuncData* pFData = pFuncDataList->Get( aProgrammaticName );
    if( pFData )
        aRet = GetFuncDescrStr( pFData->GetDescrID(), 1 );

    return aRet;
}

OUString SAL_CALL ScaPricingAddIn::getDisplayArgumentName(
        const OUString& aProgrammaticName, sal_Int32 nArgument ) throw( uno::RuntimeException )
{
    OUString aRet;

    const ScaFuncData* pFData = pFuncDataList->Get( aProgrammaticName );
    if( pFData && (nArgument <= 0xFFFF) )
    {
        sal_uInt16 nStr = pFData->GetStrIndex( static_cast< sal_uInt16 >( nArgument ) );
        if( nStr )
            aRet = GetFuncDescrStr( pFData->GetDescrID(), nStr );
        else
            aRet = STR_FROM_ANSI( "internal" );
    }

    return aRet;
}

OUString SAL_CALL ScaPricingAddIn::getArgumentDescription(
        const OUString& aProgrammaticName, sal_Int32 nArgument ) throw( uno::RuntimeException )
{
    OUString aRet;

    const ScaFuncData* pFData = pFuncDataList->Get( aProgrammaticName );
    if( pFData && (nArgument <= 0xFFFF) )
    {
        sal_uInt16 nStr = pFData->GetStrIndex( static_cast< sal_uInt16 >( nArgument ) );
        if( nStr )
            aRet = GetFuncDescrStr( pFData->GetDescrID(), nStr + 1 );
        else
            aRet = STR_FROM_ANSI( "for internal use only" );
    }

    return aRet;
}

OUString SAL_CALL ScaPricingAddIn::getProgrammaticCategoryName(
        const OUString& aProgrammaticName ) throw( uno::RuntimeException )
{
    OUString aRet;

    const ScaFuncData* pFData = pFuncDataList->Get( aProgrammaticName );
    if( pFData )
    {
        switch( pFData->GetCategory() )
        {
            case ScaCat_DateTime:   aRet = STR_FROM_ANSI( "Date&Time" );    break;
            case ScaCat_Text:       aRet = STR_FROM_ANSI( "Text" );         break;
            case ScaCat_Finance:    aRet = STR_FROM_ANSI( "Financial" );    break;
            case ScaCat_Inf:        aRet = STR_FROM_ANSI( "Information" );  break;
            case ScaCat_Math:       aRet = STR_FROM_ANSI( "Mathematical" ); break;
            case ScaCat_Tech:       aRet = STR_FROM_ANSI( "Technical" );    break;
            default:    // to prevent compiler warnings
                break;
        }
    }

    if( aRet.isEmpty() )
        aRet = STR_FROM_ANSI( "Add-In" );
    return aRet;
}

OUString SAL_CALL ScaPricingAddIn::getDisplayCategoryName(
        const OUString& aProgrammaticName ) throw( uno::RuntimeException )
{
    return getProgrammaticCategoryName( aProgrammaticName );
}


// XCompatibilityNames

uno::Sequence< sheet::LocalizedName > SAL_CALL ScaPricingAddIn::getCompatibilityNames(
        const OUString& aProgrammaticName ) throw( uno::RuntimeException )
{
    const ScaFuncData* pFData = pFuncDataList->Get( aProgrammaticName );
    if( !pFData )
        return uno::Sequence< sheet::LocalizedName >( 0 );

    const ScaStringList& rStrList = pFData->GetCompNameList();
    sal_uInt32 nCount = rStrList.Count();

    uno::Sequence< sheet::LocalizedName > aRet( nCount );
    sheet::LocalizedName* pArray = aRet.getArray();

    for( sal_uInt32 nIndex = 0; nIndex < nCount; nIndex++ )
        pArray[ nIndex ] = sheet::LocalizedName( GetLocale( nIndex ), *rStrList.Get( nIndex ) );

    return aRet;
}



// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
// actual function implementation starts here
//

// auxillary input handling functions
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

    if(str.compareToAscii("value")==0 || str.compareToAscii("price")==0 ||
       str.compareToAscii("v")==0 || str.compareToAscii("p")==0 ) {
        greek=bs::types::Value;
    } else if(str.compareToAscii("delta")==0||str.compareToAscii("d")==0) {
        greek=bs::types::Delta;
    } else if(str.compareToAscii("gamma")==0||str.compareToAscii("g")==0) {
        greek=bs::types::Gamma;
    } else if(str.compareToAscii("theta")==0||str.compareToAscii("t")==0) {
        greek=bs::types::Theta;
    } else if(str.compareToAscii("vega")==0||str.compareToAscii("e")==0) {
        greek=bs::types::Vega;
    } else if(str.compareToAscii("volga")==0||str.compareToAscii("o")==0) {
        greek=bs::types::Volga;
    } else if(str.compareToAscii("vanna")==0||str.compareToAscii("a")==0) {
        greek=bs::types::Vanna;
    } else if(str.compareToAscii("rho")==0||str.compareToAscii("r")==0) {
        greek=bs::types::Rho_d;
    } else if(str.compareToAscii("rhof")==0||str.compareToAscii("f")==0) {
        greek=bs::types::Rho_f;
    } else {
        return false;
    }
    return true;
}

} // namespace for auxillary functions


// OPT_BARRIER(...)
double SAL_CALL ScaPricingAddIn::getOptBarrier( double spot, double vol,
            double r, double rf, double T, double strike,
            double barrier_low, double barrier_up, double rebate,
            const OUString& put_call, const OUString& in_out,
            const OUString& barriercont, const uno::Any& greekstr ) throw( uno::RuntimeException, lang::IllegalArgumentException )
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
            const OUString& barriercont, const uno::Any& greekstr ) throw( uno::RuntimeException, lang::IllegalArgumentException )
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
            double barrier_low, double barrier_up ) throw( uno::RuntimeException, lang::IllegalArgumentException )
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
            const uno::Any& strikeval, const uno::Any& put_call ) throw( uno::RuntimeException, lang::IllegalArgumentException )
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



//------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
