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

#ifdef DISABLE_DYNLOADING
#include <config_java.h>
#endif

#include "IdentityMapping.hxx"

#include <cassert>
#include <set>
#include <unordered_map>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/module.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <osl/interlck.h>
#include <sal/log.hxx>

#include <uno/dispatcher.h>
#include <uno/mapping.h>
#include <uno/lbnames.h>
#include <uno/environment.hxx>

#include <typelib/typedescription.h>

#include <cppu/EnvDcp.hxx>
#include "cascade_mapping.hxx"
#include "loadmodule.hxx"

using namespace std;
using namespace osl;
using namespace com::sun::star::uno;

namespace cppu
{

class Mapping
{
    uno_Mapping * _pMapping;

public:
    inline explicit Mapping( uno_Mapping * pMapping = nullptr );
    inline Mapping( const Mapping & rMapping );
    Mapping(Mapping && other): _pMapping(other._pMapping)
    { other._pMapping = nullptr; }
    inline ~Mapping();
    inline Mapping & operator = ( uno_Mapping * pMapping );
    Mapping & operator = ( const Mapping & rMapping )
        { return operator = ( rMapping._pMapping ); }
    Mapping & operator =(Mapping && other) {
        if (_pMapping != nullptr) {
            (*_pMapping->release)(_pMapping);
        }
        _pMapping = other._pMapping;
        other._pMapping = nullptr;
        return *this;
    }
    uno_Mapping * get() const
        { return _pMapping; }
    bool is() const
        { return (_pMapping != nullptr); }
};

inline Mapping::Mapping( uno_Mapping * pMapping )
    : _pMapping( pMapping )
{
    if (_pMapping)
        (*_pMapping->acquire)( _pMapping );
}

inline Mapping::Mapping( const Mapping & rMapping )
    : _pMapping( rMapping._pMapping )
{
    if (_pMapping)
        (*_pMapping->acquire)( _pMapping );
}

inline Mapping::~Mapping()
{
    if (_pMapping)
        (*_pMapping->release)( _pMapping );
}

inline Mapping & Mapping::operator = ( uno_Mapping * pMapping )
{
    if (pMapping)
        (*pMapping->acquire)( pMapping );
    if (_pMapping)
        (*_pMapping->release)( _pMapping );
    _pMapping = pMapping;
    return *this;
}


struct MappingEntry
{
    sal_Int32           nRef;
    uno_Mapping *       pMapping;
    uno_freeMappingFunc freeMapping;
    OUString            aMappingName;

    MappingEntry(
        uno_Mapping * pMapping_, uno_freeMappingFunc freeMapping_,
        const OUString & rMappingName_ )
        : nRef( 1 )
        , pMapping( pMapping_ )
        , freeMapping( freeMapping_ )
        , aMappingName( rMappingName_ )
        {}
};

struct FctPtrHash
{
    size_t operator()( uno_Mapping * pKey ) const
        { return reinterpret_cast<size_t>(pKey); }
};

typedef std::unordered_map<
    OUString, MappingEntry * > t_OUString2Entry;
typedef std::unordered_map<
    uno_Mapping *, MappingEntry *, FctPtrHash > t_Mapping2Entry;

typedef set< uno_getMappingFunc > t_CallbackSet;


struct MappingsData
{
    Mutex               aMappingsMutex;
    t_OUString2Entry    aName2Entry;
    t_Mapping2Entry     aMapping2Entry;

    Mutex               aCallbacksMutex;
    t_CallbackSet       aCallbacks;

    Mutex               aNegativeLibsMutex;
    set<OUString>       aNegativeLibs;
};

static MappingsData & getMappingsData()
{
    //TODO  This memory is leaked; see #i63473# for when this should be
    // changed again:
    static MappingsData * s_p(new MappingsData);

    return *s_p;
}

/**
 * This class mediates two different mapping via uno, e.g. form any language to uno,
 * then from uno to any other language.
 */
struct uno_Mediate_Mapping : public uno_Mapping
{
    sal_Int32   nRef;

    Environment aFrom;
    Environment aTo;

    Mapping     aFrom2Uno;
    Mapping     aUno2To;

    OUString    aAddPurpose;

    uno_Mediate_Mapping(
        const Environment & rFrom_, const Environment & rTo_,
        const Mapping & rFrom2Uno_, const Mapping & rUno2To_,
        const OUString & rAddPurpose );
};
extern "C"
{

static void mediate_free( uno_Mapping * pMapping )
{
    delete static_cast< uno_Mediate_Mapping * >( pMapping );
}

static void mediate_acquire( uno_Mapping * pMapping )
{
    if (1 == osl_atomic_increment(
        & static_cast< uno_Mediate_Mapping * >( pMapping )->nRef ))
    {
        uno_registerMapping(
            &pMapping, mediate_free,
            static_cast< uno_Mediate_Mapping * >( pMapping )->aFrom.get(),
            static_cast< uno_Mediate_Mapping * >( pMapping )->aTo.get(),
            static_cast< uno_Mediate_Mapping * >( pMapping )->aAddPurpose.pData );
    }
}

static void mediate_release( uno_Mapping * pMapping )
{
    if (! osl_atomic_decrement(
        & static_cast< uno_Mediate_Mapping * >( pMapping )->nRef ))
    {
        uno_revokeMapping( pMapping );
    }
}

static void mediate_mapInterface(
    uno_Mapping * pMapping,
    void ** ppOut, void * pInterface,
    typelib_InterfaceTypeDescription * pInterfaceTypeDescr )
{
    OSL_ENSURE( pMapping && ppOut, "### null ptr!" );
    if (pMapping && ppOut)
    {
        uno_Mediate_Mapping * that = static_cast< uno_Mediate_Mapping * >( pMapping );
        uno_Mapping * pFrom2Uno = that->aFrom2Uno.get();

        uno_Interface * pUnoI = nullptr;
        (*pFrom2Uno->mapInterface)( pFrom2Uno, reinterpret_cast<void **>(&pUnoI), pInterface, pInterfaceTypeDescr );
        if (nullptr == pUnoI)
        {
            void * pOut = *ppOut;
            if (nullptr != pOut)
            {
                uno_ExtEnvironment * pTo = that->aTo.get()->pExtEnv;
                OSL_ENSURE( nullptr != pTo, "### cannot release out interface: leaking!" );
                if (nullptr != pTo)
                    (*pTo->releaseInterface)( pTo, pOut );
                *ppOut = nullptr; // set to 0 anyway, because mapping was not successful!
            }
        }
        else
        {
            uno_Mapping * pUno2To = that->aUno2To.get();
            (*pUno2To->mapInterface)( pUno2To, ppOut, pUnoI, pInterfaceTypeDescr );
            (*pUnoI->release)( pUnoI );
        }
    }
}
}

uno_Mediate_Mapping::uno_Mediate_Mapping(
    const Environment & rFrom_, const Environment & rTo_,
    const Mapping & rFrom2Uno_, const Mapping & rUno2To_,
    const OUString & rAddPurpose_ )
    : nRef( 1 )
    , aFrom( rFrom_ )
    , aTo( rTo_ )
    , aFrom2Uno( rFrom2Uno_ )
    , aUno2To( rUno2To_ )
    , aAddPurpose( rAddPurpose_ )
{
    uno_Mapping::acquire        = mediate_acquire;
    uno_Mapping::release        = mediate_release;
    uno_Mapping::mapInterface   = mediate_mapInterface;
}


static OUString getMappingName(
    const Environment & rFrom, const Environment & rTo, const OUString & rAddPurpose )
{
    OUStringBuffer aKey( 64 );
    aKey.append( rAddPurpose );
    aKey.append( ';' );
    aKey.append( rFrom.getTypeName() );
    aKey.append( '[' );
    aKey.append( reinterpret_cast< sal_IntPtr >(rFrom.get()), 16 );
    aKey.append( "];" );
    aKey.append( rTo.getTypeName() );
    aKey.append( '[' );
    aKey.append( reinterpret_cast< sal_IntPtr >(rTo.get()), 16 );
    aKey.append( ']' );
    return aKey.makeStringAndClear();
}

static OUString getBridgeName(
    const Environment & rFrom, const Environment & rTo, const OUString & rAddPurpose )
{
    OUStringBuffer aBridgeName( 16 );
    if (!rAddPurpose.isEmpty())
    {
        aBridgeName.append( rAddPurpose );
        aBridgeName.append( '_' );
    }
    aBridgeName.append( EnvDcp::getTypeName(rFrom.getTypeName()) );
    aBridgeName.append( '_' );
    aBridgeName.append( EnvDcp::getTypeName(rTo.getTypeName()) );
    return aBridgeName.makeStringAndClear();
}

#ifndef DISABLE_DYNLOADING

static void setNegativeBridge( const OUString & rBridgeName )
{
    MappingsData & rData = getMappingsData();
    MutexGuard aGuard( rData.aNegativeLibsMutex );
    rData.aNegativeLibs.insert( rBridgeName );
}

#endif

#ifdef DISABLE_DYNLOADING

static uno_ext_getMappingFunc selectMapFunc( const OUString & rBridgeName )
{
    if (rBridgeName.equalsAscii( CPPU_CURRENT_LANGUAGE_BINDING_NAME "_uno" ))
        return CPPU_ENV_uno_ext_getMapping;
#if HAVE_FEATURE_JAVA
    if (rBridgeName.equalsAscii( "java" "_uno" ))
        return java_uno_ext_getMapping;
#endif

#if 0
    // I don't think the affine or log bridges will be needed on any
    // DISABLE_DYNLOADING platform (iOS at least, possibly Android), but if
    // somebody wants to experiment, need to find out then whether these are
    // needed.
    if (rBridgeName.equalsAscii( "affine_uno_uno" ))
        return affine_uno_uno_ext_getMapping;
    if (rBridgeName.equalsAscii( "log_uno_uno" ))
        return log_uno_uno_ext_getMapping;
#endif
    return 0;
}

#else

static bool loadModule(osl::Module & rModule, const OUString & rBridgeName)
{
    bool bNeg;
    {
    MappingsData & rData = getMappingsData();
    MutexGuard aGuard( rData.aNegativeLibsMutex );
    const auto iFind( rData.aNegativeLibs.find( rBridgeName ) );
    bNeg = (iFind != rData.aNegativeLibs.end());
    }

    if (!bNeg)
    {
        bool bModule;
        try {
            bModule = cppu::detail::loadModule(rModule, rBridgeName);
        }
        catch(...) {
            // convert throw to return false
            bModule = false;
        }

        if (bModule)
            return true;

        setNegativeBridge( rBridgeName ); // no load again
    }
    return false;
}

#endif


static Mapping loadExternalMapping(
    const Environment & rFrom, const Environment & rTo, const OUString & rAddPurpose )
{
    OSL_ASSERT( rFrom.is() && rTo.is() );
    if (rFrom.is() && rTo.is())
    {
#ifdef DISABLE_DYNLOADING
        OUString aName;
        uno_ext_getMappingFunc fpGetMapFunc = 0;

        if (EnvDcp::getTypeName(rFrom.getTypeName()) == UNO_LB_UNO)
        {
            aName = getBridgeName( rTo, rFrom, rAddPurpose );
            fpGetMapFunc = selectMapFunc( aName );
        }
        if (! fpGetMapFunc)
        {
            aName = getBridgeName( rFrom, rTo, rAddPurpose );
            fpGetMapFunc = selectMapFunc( aName );
        }
        if (! fpGetMapFunc)
        {
            aName = getBridgeName( rTo, rFrom, rAddPurpose );
            fpGetMapFunc = selectMapFunc( aName );
        }

        if (! fpGetMapFunc)
        {
            SAL_INFO("cppu", "Could not find mapfunc for " << aName);
            return Mapping();
        }

        if (fpGetMapFunc)
        {
            Mapping aExt;
            (*fpGetMapFunc)( (uno_Mapping **)&aExt, rFrom.get(), rTo.get() );
            OSL_ASSERT( aExt.is() );
            if (aExt.is())
                return aExt;
        }
#else
        // find proper lib
        osl::Module aModule;
        bool bModule(false);
        OUString aName;

        if ( EnvDcp::getTypeName(rFrom.getTypeName()) == UNO_LB_UNO )
        {
            aName = getBridgeName( rTo, rFrom, rAddPurpose );
            bModule = loadModule( aModule, aName );
        }
        if (!bModule)
        {
            aName = getBridgeName( rFrom, rTo, rAddPurpose );
            bModule = loadModule( aModule, aName );
        }
        if (!bModule)
        {
            aName = getBridgeName( rTo, rFrom, rAddPurpose );
            bModule = loadModule( aModule, aName );
        }

        if (bModule)
        {
            uno_ext_getMappingFunc fpGetMapFunc =
                reinterpret_cast<uno_ext_getMappingFunc>(aModule.getSymbol( UNO_EXT_GETMAPPING ));

            if (fpGetMapFunc)
            {
                Mapping aExt;
                (*fpGetMapFunc)( reinterpret_cast<uno_Mapping **>(&aExt), rFrom.get(), rTo.get() );
                OSL_ASSERT( aExt.is() );
                if (aExt.is())
                {
                    aModule.release();
                    return aExt;
                }
            }
            aModule.unload();
            setNegativeBridge( aName );
        }
#endif
    }
    return Mapping();
}


static Mapping getDirectMapping(
    const Environment & rFrom, const Environment & rTo, const OUString & rAddPurpose = OUString() )

{
    OSL_ASSERT( rFrom.is() && rTo.is() );
    if (rFrom.is() && rTo.is())
    {
        MappingsData & rData = getMappingsData();
        ClearableMutexGuard aGuard( rData.aMappingsMutex );

        // try to find registered mapping
        const t_OUString2Entry::const_iterator iFind( rData.aName2Entry.find(
            getMappingName( rFrom, rTo, rAddPurpose ) ) );

        if (iFind == rData.aName2Entry.end())
        {
            aGuard.clear();
            return loadExternalMapping( rFrom, rTo, rAddPurpose );
        }
        return Mapping( (*iFind).second->pMapping );
    }
    return Mapping();
}


static Mapping createMediateMapping(
    const Environment & rFrom, const Environment & rTo,
    const Mapping & rFrom2Uno, const Mapping & rUno2To,
    const OUString & rAddPurpose )
{
    uno_Mapping * pRet = new uno_Mediate_Mapping(
        rFrom, rTo, rFrom2Uno, rUno2To, rAddPurpose ); // ref count initially 1
    uno_registerMapping(
        &pRet, mediate_free, rFrom.get(), rTo.get(), rAddPurpose.pData );
    Mapping aRet( pRet );
    (*pRet->release)( pRet );
    return aRet;
}

static Mapping getMediateMapping(
    const Environment & rFrom, const Environment & rTo, const OUString & rAddPurpose )
{
    Environment aUno;
    Mapping aUno2To;

    // backwards: from dest to source of mapping chain

    // connect to uno
    OUString aUnoEnvTypeName( UNO_LB_UNO );
    if (rTo.getTypeName() == aUnoEnvTypeName) // to is uno
    {
        aUno = rTo;
        // no Uno2To mapping necessary
    }
    else
    {
        // get registered uno env
        ::uno_getEnvironment( reinterpret_cast<uno_Environment **>(&aUno), aUnoEnvTypeName.pData, nullptr );

        aUno2To = getDirectMapping( aUno, rTo );
        // : uno <-> to
        if (! aUno2To.is())
            return Mapping();
    }

    // connect to uno
    if (!rAddPurpose.isEmpty()) // insert purpose mapping between new ano_uno <-> uno
    {
        // create anonymous uno env
        Environment aAnUno;
        ::uno_createEnvironment( reinterpret_cast<uno_Environment **>(&aAnUno), aUnoEnvTypeName.pData, nullptr );

        Mapping aAnUno2Uno( getDirectMapping( aAnUno, aUno, rAddPurpose ) );
        if (! aAnUno2Uno.is())
            return Mapping();

        if (aUno2To.is()) // to is not uno
        {
            // create another purposed mediate mapping
            aUno2To = createMediateMapping( aAnUno, rTo, aAnUno2Uno, aUno2To, rAddPurpose );
            // : ano_uno <-> uno <-> to
        }
        else
        {
            aUno2To = aAnUno2Uno;
            // : ano_uno <-> to (i.e., uno)
        }
        aUno = aAnUno;
    }

    Mapping aFrom2Uno( getDirectMapping( rFrom, aUno ) );
    if (aFrom2Uno.is() && aUno2To.is())
    {
        return createMediateMapping( rFrom, rTo, aFrom2Uno, aUno2To, rAddPurpose );
        // : from <-> some uno ...
    }

    return Mapping();
}
}

using namespace ::cppu;

extern "C"
{

void SAL_CALL uno_getMapping(
    uno_Mapping ** ppMapping, uno_Environment * pFrom, uno_Environment * pTo,
    rtl_uString * pAddPurpose )
    SAL_THROW_EXTERN_C()
{
    assert(ppMapping != nullptr);
    assert(pFrom != nullptr);
    assert(pTo != nullptr);
    if (*ppMapping)
    {
        (*(*ppMapping)->release)( *ppMapping );
        *ppMapping = nullptr;
    }

    Mapping aRet;
    Environment aFrom( pFrom ), aTo( pTo );

    OUString aAddPurpose;
    if (pAddPurpose)
        aAddPurpose = pAddPurpose;

    MappingsData & rData = getMappingsData();

    // try registered mapping
    {
    MutexGuard aGuard( rData.aMappingsMutex );
    const t_OUString2Entry::const_iterator iFind( rData.aName2Entry.find(
        getMappingName( aFrom, aTo, aAddPurpose ) ) );
    if (iFind != rData.aName2Entry.end())
        aRet = (*iFind).second->pMapping;
    }

    // See if an identity mapping does fit.
    if (!aRet.is() && pFrom == pTo && aAddPurpose.isEmpty())
        aRet = createIdentityMapping(pFrom);

    if (!aRet.is())
    {
        getCascadeMapping(ppMapping, pFrom, pTo, pAddPurpose);

        if (*ppMapping)
            return;
    }

    if (! aRet.is()) // try callback chain
    {
        MutexGuard aGuard( rData.aCallbacksMutex );
        for ( const auto& rCallback : rData.aCallbacks )
        {
            (*rCallback)( ppMapping, pFrom, pTo, aAddPurpose.pData );
            if (*ppMapping)
                return;
        }
    }

    if (! aRet.is())
    {
        aRet = loadExternalMapping( aFrom, aTo, aAddPurpose ); // direct try
        if (! aRet.is())
            aRet = getMediateMapping( aFrom, aTo, aAddPurpose ); // try via uno
    }

    if (aRet.is())
    {
        (*aRet.get()->acquire)( aRet.get() );
        *ppMapping = aRet.get();
    }
}

void SAL_CALL uno_getMappingByName(
    uno_Mapping ** ppMapping, rtl_uString * pFrom, rtl_uString * pTo,
    rtl_uString * pAddPurpose )
    SAL_THROW_EXTERN_C()
{
    assert(ppMapping && pFrom && pTo && "### null ptr!");
    if (*ppMapping)
    {
        (*(*ppMapping)->release)( *ppMapping );
        *ppMapping = nullptr;
    }

    uno_Environment * pEFrom = nullptr;
    uno_getEnvironment( &pEFrom, pFrom, nullptr );
    OSL_ENSURE( pEFrom, "### cannot get source environment!" );
    if (pEFrom)
    {
        uno_Environment * pETo = nullptr;
        uno_getEnvironment( &pETo, pTo, nullptr );
        OSL_ENSURE( pETo, "### cannot get target environment!" );
        if (pETo)
        {
            ::uno_getMapping( ppMapping, pEFrom, pETo, pAddPurpose );
            (*pETo->release)( pETo );
        }
        (*pEFrom->release)( pEFrom );
    }
}


void SAL_CALL uno_registerMapping(
    uno_Mapping ** ppMapping, uno_freeMappingFunc freeMapping,
    uno_Environment * pFrom, uno_Environment * pTo, rtl_uString * pAddPurpose )
    SAL_THROW_EXTERN_C()
{
    MappingsData & rData = getMappingsData();
    ClearableMutexGuard aGuard( rData.aMappingsMutex );

    const t_Mapping2Entry::const_iterator iFind( rData.aMapping2Entry.find( *ppMapping ) );
    if (iFind == rData.aMapping2Entry.end())
    {
        OUString aMappingName(
            getMappingName( pFrom, pTo, pAddPurpose ? OUString(pAddPurpose) : OUString() ) );
        SAL_INFO("cppu", "> inserting new mapping: " << aMappingName);
        // count initially 1
        MappingEntry * pEntry = new MappingEntry( *ppMapping, freeMapping, aMappingName );
        rData.aName2Entry[ aMappingName ] = pEntry;
        rData.aMapping2Entry[ *ppMapping ] = pEntry;
    }
    else
    {
        MappingEntry * pEntry = (*iFind).second;
        ++pEntry->nRef;

        if (pEntry->pMapping != *ppMapping) // exchange mapping to be registered
        {
            (*pEntry->pMapping->acquire)( pEntry->pMapping );
            --pEntry->nRef; // correct count; kill mapping to be registered
            aGuard.clear();
            (*freeMapping)( *ppMapping );
            *ppMapping = pEntry->pMapping;
        }
    }
}

void SAL_CALL uno_revokeMapping(
    uno_Mapping * pMapping )
    SAL_THROW_EXTERN_C()
{
    MappingsData & rData = getMappingsData();
    ClearableMutexGuard aGuard( rData.aMappingsMutex );

    const t_Mapping2Entry::const_iterator iFind( rData.aMapping2Entry.find( pMapping ) );
    OSL_ASSERT( iFind != rData.aMapping2Entry.end() );
    MappingEntry * pEntry = (*iFind).second;
    if (! --pEntry->nRef)
    {
        rData.aMapping2Entry.erase( pEntry->pMapping );
        rData.aName2Entry.erase( pEntry->aMappingName );
        aGuard.clear();
        SAL_INFO("cppu", "> revoking mapping " << pEntry->aMappingName);
        (*pEntry->freeMapping)( pEntry->pMapping );
        delete pEntry;
    }
}


void SAL_CALL uno_registerMappingCallback(
    uno_getMappingFunc pCallback )
    SAL_THROW_EXTERN_C()
{
    OSL_ENSURE( pCallback, "### null ptr!" );
    MappingsData & rData = getMappingsData();
    MutexGuard aGuard( rData.aCallbacksMutex );
    rData.aCallbacks.insert( pCallback );
}

void SAL_CALL uno_revokeMappingCallback(
    uno_getMappingFunc pCallback )
    SAL_THROW_EXTERN_C()
{
    OSL_ENSURE( pCallback, "### null ptr!" );
    MappingsData & rData = getMappingsData();
    MutexGuard aGuard( rData.aCallbacksMutex );
    rData.aCallbacks.erase( pCallback );
}
} // extern "C"

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
