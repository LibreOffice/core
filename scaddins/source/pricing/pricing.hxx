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

// option pricing functions add in

// most parts of this files are technical UNO details which are
// all copied from ../datefunc/datefunc.hxx
// to avoid having to rename all classes to do with UNO
// technicalities we use our own namespace

#ifndef INCLUDED_SCADDINS_SOURCE_PRICING_PRICING_HXX
#define INCLUDED_SCADDINS_SOURCE_PRICING_PRICING_HXX


#include <string.h>
#include <vector>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sheet/XAddIn.hpp>
#include <com/sun/star/sheet/XCompatibilityNames.hpp>
#include <com/sun/star/sheet/addin/XPricingFunctions.hpp>
#include <cppuhelper/implbase.hxx>
#include <tools/resid.hxx>
#include <tools/rc.hxx>
#include <tools/resary.hxx>

#define RETURN_FINITE(d)    if( ::rtl::math::isFinite( d ) ) return d; else throw css::lang::IllegalArgumentException()



namespace sca {
namespace pricing {

class ScaResId : public ResId
{
public:
                                ScaResId( sal_uInt16 nResId, ResMgr& rResMgr );
};


class ScaResStringLoader : public Resource
{
private:
    OUString                    aStr;

public:
    inline                      ScaResStringLoader( sal_uInt16 nResId, sal_uInt16 nStrId, ResMgr& rResMgr );

    inline const OUString&      GetString() const   { return aStr; }

};


inline ScaResStringLoader::ScaResStringLoader( sal_uInt16 nResId, sal_uInt16 nStrId, ResMgr& rResMgr ) :
    Resource( ScaResId( nResId, rResMgr ) ),
    aStr( ScaResId( nStrId, rResMgr ) )
{
    FreeResource();
}


class ScaResStringArrLoader : public Resource
{
private:
    ResStringArray              aStrArray;

public:
    inline                      ScaResStringArrLoader( sal_uInt16 nResId, sal_uInt16 nArrayId, ResMgr& rResMgr );

    inline const ResStringArray& GetStringArray() const { return aStrArray; }
};

inline ScaResStringArrLoader::ScaResStringArrLoader( sal_uInt16 nResId, sal_uInt16 nArrayId, ResMgr& rResMgr ) :
    Resource( ScaResId( nResId, rResMgr ) ),
    aStrArray( ScaResId( nArrayId, rResMgr ) )
{
    FreeResource();
}


class ScaResPublisher : public Resource
{
public:
    inline                      ScaResPublisher( const ScaResId& rResId ) : Resource( rResId ) {}

    inline bool             IsAvailableRes( const ResId& rResId ) const
                                    { return Resource::IsAvailableRes( rResId ); }
    inline void                 FreeResource()
                                    { Resource::FreeResource(); }
};


class ScaFuncRes : public Resource
{
public:
                                ScaFuncRes( ResId& rResId, ResMgr& rResMgr, sal_uInt16 nIndex, OUString& rRet );
};


enum ScaCategory
{
    ScaCat_AddIn,
    ScaCat_DateTime,
    ScaCat_Text,
    ScaCat_Finance,
    ScaCat_Inf,
    ScaCat_Math,
    ScaCat_Tech
};

struct ScaFuncDataBase
{
    const sal_Char*             pIntName;           // internal name (get***)
    sal_uInt16                  nUINameID;          // resource ID to UI name
    sal_uInt16                  nDescrID;           // resource ID to description, parameter names and ~ description
    sal_uInt16                  nCompListID;        // resource ID to list of valid names
    sal_uInt16                  nParamCount;        // number of named / described parameters
    ScaCategory                 eCat;               // function category
    bool                    bDouble;            // name already exist in Calc
    bool                    bWithOpt;           // first parameter is internal
};

class ScaFuncData
{
private:
    OUString             aIntName;           // internal name (get***)
    sal_uInt16                  nUINameID;          // resource ID to UI name
    sal_uInt16                  nDescrID;           // leads also to parameter descriptions!
    sal_uInt16                  nCompListID;        // resource ID to list of valid names
    sal_uInt16                  nParamCount;        // num of parameters
    std::vector<OUString>       aCompList;          // list of all valid names
    ScaCategory                 eCat;               // function category
    bool                    bDouble;            // name already exist in Calc
    bool                    bWithOpt;           // first parameter is internal

public:
                                ScaFuncData( const ScaFuncDataBase& rBaseData, ResMgr& rRscMgr );
    virtual                     ~ScaFuncData();

    inline sal_uInt16           GetUINameID() const     { return nUINameID; }
    inline sal_uInt16           GetDescrID() const      { return nDescrID; }
    inline ScaCategory          GetCategory() const     { return eCat; }
    inline bool                 IsDouble() const        { return bDouble; }

    sal_uInt16                  GetStrIndex( sal_uInt16 nParam ) const;
    inline bool                 Is( const OUString& rCompare ) const
                                                    { return aIntName == rCompare; }

    inline const std::vector<OUString>& GetCompNameList() const { return aCompList; }
};


typedef std::vector<ScaFuncData> ScaFuncDataList;

void InitScaFuncDataList ( ScaFuncDataList& rMap, ResMgr& rResMgr );

// Predicate for use with std::find_if
struct FindScaFuncData
{
    const OUString& m_rId;
    explicit FindScaFuncData( const OUString& rId ) : m_rId(rId) {}
    bool operator() ( ScaFuncData& rCandidate ) const { return rCandidate.Is(m_rId); }
};

} // namespace pricing
} // namespace sca




css::uno::Reference< css::uno::XInterface > SAL_CALL PricingFunctionAddIn_CreateInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory >& );


// AddIn class for pricing functions

class ScaPricingAddIn : public ::cppu::WeakImplHelper<
                                css::sheet::XAddIn,
                                css::sheet::XCompatibilityNames,
                                css::sheet::addin::XPricingFunctions,
                                css::lang::XServiceName,
                                css::lang::XServiceInfo >
{
private:
    css::lang::Locale  aFuncLoc;
    css::lang::Locale* pDefLocales;
    ResMgr*                     pResMgr;
    sca::pricing::ScaFuncDataList*            pFuncDataList;


    void                        InitDefLocales();
    const css::lang::Locale& GetLocale( sal_uInt32 nIndex );
    ResMgr&                     GetResMgr() throw( css::uno::RuntimeException, std::exception );
    void                        InitData();

    OUString             GetDisplFuncStr( sal_uInt16 nResId ) throw( css::uno::RuntimeException, std::exception );
    OUString             GetFuncDescrStr( sal_uInt16 nResId, sal_uInt16 nStrIndex ) throw( css::uno::RuntimeException, std::exception );

public:
                                ScaPricingAddIn();
    virtual                     ~ScaPricingAddIn();

    static OUString      getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

                                // XAddIn
    virtual OUString SAL_CALL getProgrammaticFuntionName( const OUString& aDisplayName ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual OUString SAL_CALL getDisplayFunctionName( const OUString& aProgrammaticName ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual OUString SAL_CALL getFunctionDescription( const OUString& aProgrammaticName ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual OUString SAL_CALL getDisplayArgumentName( const OUString& aProgrammaticName, sal_Int32 nArgument ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual OUString SAL_CALL getArgumentDescription( const OUString& aProgrammaticName, sal_Int32 nArgument ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual OUString SAL_CALL getProgrammaticCategoryName( const OUString& aProgrammaticName ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual OUString SAL_CALL getDisplayCategoryName( const OUString& aProgrammaticName ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

                                // XCompatibilityNames
    virtual css::uno::Sequence< css::sheet::LocalizedName > SAL_CALL getCompatibilityNames( const OUString& aProgrammaticName ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

                                // XLocalizable
    virtual void SAL_CALL       setLocale( const css::lang::Locale& eLocale ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual css::lang::Locale SAL_CALL getLocale() throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

                                // XServiceName
    virtual OUString SAL_CALL getServiceName() throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

                                // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL   supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;


    //  methods from own interfaces start here


    virtual double SAL_CALL getOptBarrier( double spot, double vol,
            double r, double rf, double T, double strike,
            double barrier_low, double barrier_up, double rebate,
            const OUString& put_call, const OUString& in_out,
            const OUString& continuous, const css::uno::Any& greek ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception ) SAL_OVERRIDE;

   virtual double SAL_CALL getOptTouch( double spot, double vol,
            double r, double rf, double T,
            double barrier_low, double barrier_up,
            const OUString& for_dom, const OUString& in_out,
            const OUString& barriercont, const css::uno::Any& greekstr ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception ) SAL_OVERRIDE;

   virtual double SAL_CALL getOptProbHit( double spot, double vol,
            double mu, double T,
            double barrier_low, double barrier_up ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception ) SAL_OVERRIDE;

   virtual double SAL_CALL getOptProbInMoney( double spot, double vol,
            double mu, double T,
            double barrier_low, double barrier_up,
            const css::uno::Any& strikeval, const css::uno::Any& put_call ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException, std::exception ) SAL_OVERRIDE;

};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
