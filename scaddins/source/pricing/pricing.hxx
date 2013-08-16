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
// option pricing functions add in
//
// most parts of this files are technical UNO details which are
// all copied from ../datefunc/datefunc.hxx
// to avoid having to rename all classes to do with UNO
// technicalities we use our own namespace
//
//------------------------------------------------------------------

#ifndef _SCA_PRICING_HXX
#define _SCA_PRICING_HXX


#include <string.h>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/sheet/XAddIn.hpp>
#include <com/sun/star/sheet/XCompatibilityNames.hpp>
#include <com/sun/star/sheet/addin/XPricingFunctions.hpp>
#include <cppuhelper/implbase5.hxx>             // helper for implementations
#include <tools/resid.hxx>
#include <tools/rc.hxx>
#include <tools/resary.hxx>

#define RETURN_FINITE(d)    if( ::rtl::math::isFinite( d ) ) return d; else throw css::lang::IllegalArgumentException()



//------------------------------------------------------------------

namespace sca {
namespace pricing {

class ScaList
{
private:
    static const sal_uInt32     nStartSize;
    static const sal_uInt32     nIncrSize;

    void**                      pData;          // pointer array
    sal_uInt32                  nSize;          // array size
    sal_uInt32                  nCount;         // next index to be inserted at
    sal_uInt32                  nCurr;          // current pos for iterations

    void                        _Grow();
    inline void                 Grow();

public:
                                ScaList();
    virtual                     ~ScaList();

    inline sal_uInt32           Count() const       { return nCount; }

    inline const void*          GetObject( sal_uInt32 nIndex ) const
                                    { return (nIndex < nCount) ? pData[ nIndex ] : NULL; }

    inline void*                First() { return nCount ? pData[ nCurr = 0 ] : NULL; }
    inline void*                Next()  { return (nCurr + 1 < nCount) ? pData[ ++nCurr ] : NULL; }

    inline void                 Append( void* pNew );
};


inline void ScaList::Grow()
{
    if( nCount >= nSize )
        _Grow();
}

inline void ScaList::Append( void* pNew )
{
    Grow();
    pData[ nCount++ ] = pNew;
}


//------------------------------------------------------------------

class ScaStringList : protected ScaList
{
public:
    inline                      ScaStringList() : ScaList() {};
    virtual                     ~ScaStringList();

                                using ScaList::Count;

    inline const OUString* Get( sal_uInt32 nIndex ) const;

    inline OUString*     First();
    inline OUString*     Next();

    using ScaList::Append;
    inline void                 Append( OUString* pNew );
    inline void                 Append( const OUString& rNew );
};


inline const OUString* ScaStringList::Get( sal_uInt32 nIndex ) const
{
    return static_cast< const OUString* >( ScaList::GetObject( nIndex ) );
}

inline OUString* ScaStringList::First()
{
    return static_cast< OUString* >( ScaList::First() );
}

inline OUString* ScaStringList::Next()
{
    return static_cast< OUString* >( ScaList::Next() );
}

inline void ScaStringList::Append( OUString* pNew )
{
    ScaList::Append( pNew );
}

inline void ScaStringList::Append( const OUString& rNew )
{
    ScaList::Append( new OUString( rNew ) );
}


//------------------------------------------------------------------

class ScaResId : public ResId
{
public:
                                ScaResId( sal_uInt16 nResId, ResMgr& rResMgr );
};


//------------------------------------------------------------------

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


//------------------------------------------------------------------

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


//------------------------------------------------------------------

class ScaResPublisher : public Resource
{
public:
    inline                      ScaResPublisher( const ScaResId& rResId ) : Resource( rResId ) {}

    inline sal_Bool             IsAvailableRes( const ResId& rResId ) const
                                    { return Resource::IsAvailableRes( rResId ); }
    inline void                 FreeResource()
                                    { Resource::FreeResource(); }
};


//------------------------------------------------------------------

class ScaFuncRes : public Resource
{
public:
                                ScaFuncRes( ResId& rResId, ResMgr& rResMgr, sal_uInt16 nIndex, OUString& rRet );
};


//------------------------------------------------------------------

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
    sal_Bool                    bDouble;            // name already exist in Calc
    sal_Bool                    bWithOpt;           // first parameter is internal
};

class ScaFuncData
{
private:
    OUString             aIntName;           // internal name (get***)
    sal_uInt16                  nUINameID;          // resource ID to UI name
    sal_uInt16                  nDescrID;           // leads also to parameter descriptions!
    sal_uInt16                  nCompListID;        // resource ID to list of valid names
    sal_uInt16                  nParamCount;        // num of parameters
    ScaStringList               aCompList;          // list of all valid names
    ScaCategory                 eCat;               // function category
    sal_Bool                    bDouble;            // name already exist in Calc
    sal_Bool                    bWithOpt;           // first parameter is internal

public:
                                ScaFuncData( const ScaFuncDataBase& rBaseData, ResMgr& rRscMgr );
    virtual                     ~ScaFuncData();

    inline sal_uInt16           GetUINameID() const     { return nUINameID; }
    inline sal_uInt16           GetDescrID() const      { return nDescrID; }
    inline ScaCategory          GetCategory() const     { return eCat; }
    inline sal_Bool             IsDouble() const        { return bDouble; }
    inline sal_Bool             HasIntParam() const     { return bWithOpt; }

    sal_uInt16                  GetStrIndex( sal_uInt16 nParam ) const;
    inline sal_Bool             Is( const OUString& rCompare ) const
                                                    { return aIntName == rCompare; }

    inline const ScaStringList& GetCompNameList() const { return aCompList; }
};


//------------------------------------------------------------------

class ScaFuncDataList : private ScaList
{
    OUString             aLastName;
    sal_uInt32                  nLast;

public:
                                ScaFuncDataList( ResMgr& rResMgr );
    virtual                     ~ScaFuncDataList();

                                using ScaList::Count;

    inline const ScaFuncData*   Get( sal_uInt32 nIndex ) const;
    const ScaFuncData*          Get( const OUString& rProgrammaticName ) const;
    inline ScaFuncData*         First();
    inline ScaFuncData*         Next();

    using ScaList::Append;
    inline void                 Append( ScaFuncData* pNew ) { ScaList::Append( pNew ); }
};


inline const ScaFuncData* ScaFuncDataList::Get( sal_uInt32 nIndex ) const
{
    return static_cast< const ScaFuncData* >( ScaList::GetObject( nIndex ) );
}

inline ScaFuncData* ScaFuncDataList::First()
{
    return static_cast< ScaFuncData* >( ScaList::First() );
}

inline ScaFuncData* ScaFuncDataList::Next()
{
    return static_cast< ScaFuncData* >( ScaList::Next() );
}

} // namespace pricing
} // namespace sca



//------------------------------------------------------------------
//------------------------------------------------------------------

css::uno::Reference< css::uno::XInterface > SAL_CALL PricingFunctionAddIn_CreateInstance(
    const css::uno::Reference< css::lang::XMultiServiceFactory >& );


// AddIn class for pricing functions

class ScaPricingAddIn : public ::cppu::WeakImplHelper5<
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
    ResMgr&                     GetResMgr() throw( css::uno::RuntimeException );
    void                        InitData();

    OUString             GetDisplFuncStr( sal_uInt16 nResId ) throw( css::uno::RuntimeException );
    OUString             GetFuncDescrStr( sal_uInt16 nResId, sal_uInt16 nStrIndex ) throw( css::uno::RuntimeException );

public:
                                ScaPricingAddIn();
    virtual                     ~ScaPricingAddIn();

    static OUString      getImplementationName_Static();
    static css::uno::Sequence< OUString > getSupportedServiceNames_Static();

                                // XAddIn
    virtual OUString SAL_CALL getProgrammaticFuntionName( const OUString& aDisplayName ) throw( css::uno::RuntimeException );
    virtual OUString SAL_CALL getDisplayFunctionName( const OUString& aProgrammaticName ) throw( css::uno::RuntimeException );
    virtual OUString SAL_CALL getFunctionDescription( const OUString& aProgrammaticName ) throw( css::uno::RuntimeException );
    virtual OUString SAL_CALL getDisplayArgumentName( const OUString& aProgrammaticName, sal_Int32 nArgument ) throw( css::uno::RuntimeException );
    virtual OUString SAL_CALL getArgumentDescription( const OUString& aProgrammaticName, sal_Int32 nArgument ) throw( css::uno::RuntimeException );
    virtual OUString SAL_CALL getProgrammaticCategoryName( const OUString& aProgrammaticName ) throw( css::uno::RuntimeException );
    virtual OUString SAL_CALL getDisplayCategoryName( const OUString& aProgrammaticName ) throw( css::uno::RuntimeException );

                                // XCompatibilityNames
    virtual css::uno::Sequence< css::sheet::LocalizedName > SAL_CALL getCompatibilityNames( const OUString& aProgrammaticName ) throw( css::uno::RuntimeException );

                                // XLocalizable
    virtual void SAL_CALL       setLocale( const css::lang::Locale& eLocale ) throw( css::uno::RuntimeException );
    virtual css::lang::Locale SAL_CALL getLocale() throw( css::uno::RuntimeException );

                                // XServiceName
    virtual OUString SAL_CALL getServiceName() throw( css::uno::RuntimeException );

                                // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw( css::uno::RuntimeException );
    virtual sal_Bool SAL_CALL   supportsService( const OUString& ServiceName ) throw( css::uno::RuntimeException );
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw( css::uno::RuntimeException );

    // ----------------------------------------
    //  methods from own interfaces start here
    // ----------------------------------------

    virtual double SAL_CALL getOptBarrier( double spot, double vol,
            double r, double rf, double T, double strike,
            double barrier_low, double barrier_up, double rebate,
            const OUString& put_call, const OUString& in_out,
            const OUString& continuous, const css::uno::Any& greek ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );

   virtual double SAL_CALL getOptTouch( double spot, double vol,
            double r, double rf, double T,
            double barrier_low, double barrier_up,
            const OUString& for_dom, const OUString& in_out,
            const OUString& barriercont, const css::uno::Any& greekstr ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );

   virtual double SAL_CALL getOptProbHit( double spot, double vol,
            double mu, double T,
            double barrier_low, double barrier_up ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );

   virtual double SAL_CALL getOptProbInMoney( double spot, double vol,
            double mu, double T,
            double barrier_low, double barrier_up,
            const css::uno::Any& strikeval, const css::uno::Any& put_call ) throw( css::uno::RuntimeException, css::lang::IllegalArgumentException );

};
//------------------------------------------------------------------

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
