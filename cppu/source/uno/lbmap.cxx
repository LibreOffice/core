/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: lbmap.cxx,v $
 * $Revision: 1.30 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppu.hxx"

#include "IdentityMapping.hxx"

#include <hash_map>
#include <set>
#include <algorithm>

#include "rtl/unload.h"
#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"
#include "osl/module.h"
#include "osl/diagnose.h"
#include "osl/mutex.hxx"
#include "osl/interlck.h"

#include "uno/dispatcher.h"
#include "uno/mapping.h"
#include "uno/lbnames.h"
#include "uno/environment.hxx"

#include "typelib/typedescription.h"

#include "cppu/EnvDcp.hxx"
#include "cascade_mapping.hxx"
#include "IdentityMapping.hxx"
#include "loadmodule.hxx"

using namespace std;
using namespace osl;
using namespace rtl;
using namespace com::sun::star::uno;


namespace cppu
{

class Mapping
{
    uno_Mapping * _pMapping;

public:
    inline Mapping( uno_Mapping * pMapping = 0 ) SAL_THROW( () );
    inline Mapping( const Mapping & rMapping ) SAL_THROW( () );
    inline ~Mapping() SAL_THROW( () );
    inline Mapping & SAL_CALL operator = ( uno_Mapping * pMapping ) SAL_THROW( () );
    inline Mapping & SAL_CALL operator = ( const Mapping & rMapping ) SAL_THROW( () )
        { return operator = ( rMapping._pMapping ); }
    inline uno_Mapping * SAL_CALL get() const SAL_THROW( () )
        { return _pMapping; }
    inline sal_Bool SAL_CALL is() const SAL_THROW( () )
        { return (_pMapping != 0); }
};
//__________________________________________________________________________________________________
inline Mapping::Mapping( uno_Mapping * pMapping ) SAL_THROW( () )
    : _pMapping( pMapping )
{
    if (_pMapping)
        (*_pMapping->acquire)( _pMapping );
}
//__________________________________________________________________________________________________
inline Mapping::Mapping( const Mapping & rMapping ) SAL_THROW( () )
    : _pMapping( rMapping._pMapping )
{
    if (_pMapping)
        (*_pMapping->acquire)( _pMapping );
}
//__________________________________________________________________________________________________
inline Mapping::~Mapping() SAL_THROW( () )
{
    if (_pMapping)
        (*_pMapping->release)( _pMapping );
}
//__________________________________________________________________________________________________
inline Mapping & Mapping::operator = ( uno_Mapping * pMapping ) SAL_THROW( () )
{
    if (pMapping)
        (*pMapping->acquire)( pMapping );
    if (_pMapping)
        (*_pMapping->release)( _pMapping );
    _pMapping = pMapping;
    return *this;
}

//==================================================================================================
struct MappingEntry
{
    sal_Int32           nRef;
    uno_Mapping *       pMapping;
    uno_freeMappingFunc freeMapping;
    OUString            aMappingName;

    MappingEntry(
        uno_Mapping * pMapping_, uno_freeMappingFunc freeMapping_,
        const OUString & rMappingName_ )
        SAL_THROW( () )
        : nRef( 1 )
        , pMapping( pMapping_ )
        , freeMapping( freeMapping_ )
        , aMappingName( rMappingName_ )
        {}
};
//--------------------------------------------------------------------------------------------------
struct FctOUStringHash : public unary_function< const OUString &, size_t >
{
    size_t operator()( const OUString & rKey ) const SAL_THROW( () )
        { return (size_t)rKey.hashCode(); }
};
//--------------------------------------------------------------------------------------------------
struct FctPtrHash : public unary_function< uno_Mapping *, size_t >
{
    size_t operator()( uno_Mapping * pKey ) const SAL_THROW( () )
        { return (size_t)pKey; }
};

typedef hash_map<
    OUString, MappingEntry *, FctOUStringHash, equal_to< OUString > > t_OUString2Entry;
typedef hash_map<
    uno_Mapping *, MappingEntry *, FctPtrHash, equal_to< uno_Mapping * > > t_Mapping2Entry;

typedef set< uno_getMappingFunc > t_CallbackSet;
typedef set< OUString > t_OUStringSet;

//==================================================================================================
struct MappingsData
{
    Mutex               aMappingsMutex;
    t_OUString2Entry    aName2Entry;
    t_Mapping2Entry     aMapping2Entry;

    Mutex               aCallbacksMutex;
    t_CallbackSet       aCallbacks;

    Mutex               aNegativeLibsMutex;
    t_OUStringSet       aNegativeLibs;
    ~MappingsData() SAL_THROW( () );
};
//__________________________________________________________________________________________________
MappingsData::~MappingsData() SAL_THROW( () )
{
#if OSL_DEBUG_LEVEL > 1
    OSL_ENSURE( aName2Entry.empty() && aMapping2Entry.empty(), "### unrevoked mappings!" );
    t_OUString2Entry::const_iterator iPos( aName2Entry.begin() );
    while (iPos != aName2Entry.end())
    {
        MappingEntry * pEntry = (*iPos).second;
        OString aName( OUStringToOString( pEntry->aMappingName, RTL_TEXTENCODING_ASCII_US ) );
        OSL_TRACE( "### unrevoked mapping: %s", aName.getStr() );
        ++iPos;
    }
    OSL_ENSURE( aCallbacks.empty(), "### callbacks left!" );
    if (aCallbacks.size())
    {
        OString aSize( OString::valueOf( (sal_Int32)aCallbacks.size() ) );
        OSL_TRACE( "### %d unrevoked callbacks", aSize.getStr() );
    }
#endif
}
//--------------------------------------------------------------------------------------------------
static MappingsData & getMappingsData() SAL_THROW( () )
{
    static MappingsData * s_p = 0;
    if (! s_p)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_p)
        {
            //TODO  This memory is leaked; see #i63473# for when this should be
            // changed again:
            s_p = new MappingsData;
        }
    }
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
        const OUString & rAddPurpose )
        SAL_THROW( () );
};
extern "C"
{
//--------------------------------------------------------------------------------------------------
static void SAL_CALL mediate_free( uno_Mapping * pMapping )
    SAL_THROW( () )
{
    delete static_cast< uno_Mediate_Mapping * >( pMapping );
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL mediate_acquire( uno_Mapping * pMapping )
    SAL_THROW( () )
{
    if (1 == ::osl_incrementInterlockedCount(
        & static_cast< uno_Mediate_Mapping * >( pMapping )->nRef ))
    {
        uno_registerMapping(
            &pMapping, mediate_free,
            static_cast< uno_Mediate_Mapping * >( pMapping )->aFrom.get(),
            static_cast< uno_Mediate_Mapping * >( pMapping )->aTo.get(),
            static_cast< uno_Mediate_Mapping * >( pMapping )->aAddPurpose.pData );
    }
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL mediate_release( uno_Mapping * pMapping )
    SAL_THROW( () )
{
    if (! ::osl_decrementInterlockedCount(
        & static_cast< uno_Mediate_Mapping * >( pMapping )->nRef ))
    {
        uno_revokeMapping( pMapping );
    }
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL mediate_mapInterface(
    uno_Mapping * pMapping,
    void ** ppOut, void * pInterface,
    typelib_InterfaceTypeDescription * pInterfaceTypeDescr )
    SAL_THROW( () )
{
    OSL_ENSURE( pMapping && ppOut, "### null ptr!" );
    if (pMapping && ppOut)
    {
        uno_Mediate_Mapping * that = static_cast< uno_Mediate_Mapping * >( pMapping );
        uno_Mapping * pFrom2Uno = that->aFrom2Uno.get();

        uno_Interface * pUnoI = 0;
        (*pFrom2Uno->mapInterface)( pFrom2Uno, (void **) &pUnoI, pInterface, pInterfaceTypeDescr );
        if (0 == pUnoI)
        {
            void * pOut = *ppOut;
            if (0 != pOut)
            {
                uno_ExtEnvironment * pTo = that->aTo.get()->pExtEnv;
                OSL_ENSURE( 0 != pTo, "### cannot release out interface: leaking!" );
                if (0 != pTo)
                    (*pTo->releaseInterface)( pTo, pOut );
                *ppOut = 0; // set to 0 anyway, because mapping was not successfull!
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
//__________________________________________________________________________________________________
uno_Mediate_Mapping::uno_Mediate_Mapping(
    const Environment & rFrom_, const Environment & rTo_,
    const Mapping & rFrom2Uno_, const Mapping & rUno2To_,
    const OUString & rAddPurpose_ )
    SAL_THROW( () )
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

//==================================================================================================
static inline OUString getMappingName(
    const Environment & rFrom, const Environment & rTo, const OUString & rAddPurpose )
    SAL_THROW( () )
{
    OUStringBuffer aKey( 64 );
    aKey.append( rAddPurpose );
    aKey.append( (sal_Unicode)';' );
    aKey.append( rFrom.getTypeName() );
    aKey.append( (sal_Unicode)'[' );
    aKey.append( reinterpret_cast< sal_IntPtr >(rFrom.get()), 16 );
    aKey.appendAscii( RTL_CONSTASCII_STRINGPARAM("];") );
    aKey.append( rTo.getTypeName() );
    aKey.append( (sal_Unicode)'[' );
    aKey.append( reinterpret_cast< sal_IntPtr >(rTo.get()), 16 );
    aKey.append( (sal_Unicode)']' );
    return aKey.makeStringAndClear();
}
//==================================================================================================
static inline OUString getBridgeName(
    const Environment & rFrom, const Environment & rTo, const OUString & rAddPurpose )
    SAL_THROW( () )
{
    OUStringBuffer aBridgeName( 16 );
    if (rAddPurpose.getLength())
    {
        aBridgeName.append( rAddPurpose );
        aBridgeName.append( (sal_Unicode)'_' );
    }
    aBridgeName.append( EnvDcp::getTypeName(rFrom.getTypeName()) );
    aBridgeName.append( (sal_Unicode)'_' );
    aBridgeName.append( EnvDcp::getTypeName(rTo.getTypeName()) );
    return aBridgeName.makeStringAndClear();
}
//==================================================================================================
static inline void setNegativeBridge( const OUString & rBridgeName )
    SAL_THROW( () )
{
    MappingsData & rData = getMappingsData();
    MutexGuard aGuard( rData.aNegativeLibsMutex );
    rData.aNegativeLibs.insert( rBridgeName );
}
//==================================================================================================
static inline oslModule loadModule( const OUString & rBridgeName )
    SAL_THROW( () )
{
    sal_Bool bNeg;
    {
    MappingsData & rData = getMappingsData();
    MutexGuard aGuard( rData.aNegativeLibsMutex );
    const t_OUStringSet::const_iterator iFind( rData.aNegativeLibs.find( rBridgeName ) );
    bNeg = (iFind != rData.aNegativeLibs.end());
    }

    if (! bNeg)
    {
        oslModule hModule = cppu::detail::loadModule( rBridgeName );

        if (hModule)
            return hModule;

        setNegativeBridge( rBridgeName ); // no load again
    }
    return 0;
}
//==================================================================================================
static Mapping loadExternalMapping(
    const Environment & rFrom, const Environment & rTo, const OUString & rAddPurpose )
    SAL_THROW( () )
{
    OSL_ASSERT( rFrom.is() && rTo.is() );
    if (rFrom.is() && rTo.is())
    {
        // find proper lib
        oslModule hModule = 0;
        OUString aName;

        if (EnvDcp::getTypeName(rFrom.getTypeName()).equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_LB_UNO) ))
            hModule = loadModule( aName = getBridgeName( rTo, rFrom, rAddPurpose ) );
        if (! hModule)
            hModule = loadModule( aName = getBridgeName( rFrom, rTo, rAddPurpose ) );
        if (! hModule)
            hModule = loadModule( aName = getBridgeName( rTo, rFrom, rAddPurpose ) );

        if (hModule)
        {
            OUString aSymbolName( RTL_CONSTASCII_USTRINGPARAM(UNO_EXT_GETMAPPING) );
            uno_ext_getMappingFunc fpGetMapFunc =
                (uno_ext_getMappingFunc)::osl_getFunctionSymbol(
                    hModule, aSymbolName.pData );

            if (fpGetMapFunc)
            {
                Mapping aExt;
                (*fpGetMapFunc)( (uno_Mapping **)&aExt, rFrom.get(), rTo.get() );
                OSL_ASSERT( aExt.is() );
                if (aExt.is())
                {
                    ::rtl_registerModuleForUnloading( hModule );
                    return aExt;
                }
            }
            ::osl_unloadModule( hModule );
            setNegativeBridge( aName );
        }
    }
    return Mapping();
}

//==================================================================================================
static Mapping getDirectMapping(
    const Environment & rFrom, const Environment & rTo, const OUString & rAddPurpose = OUString() )
    SAL_THROW( () )
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
        else
        {
            return Mapping( (*iFind).second->pMapping );
        }
    }
    return Mapping();
}

//--------------------------------------------------------------------------------------------------
static inline Mapping createMediateMapping(
    const Environment & rFrom, const Environment & rTo,
    const Mapping & rFrom2Uno, const Mapping & rUno2To,
    const OUString & rAddPurpose )
    SAL_THROW( () )
{
    uno_Mapping * pRet = new uno_Mediate_Mapping(
        rFrom, rTo, rFrom2Uno, rUno2To, rAddPurpose ); // ref count initially 1
    uno_registerMapping(
        &pRet, mediate_free, rFrom.get(), rTo.get(), rAddPurpose.pData );
    Mapping aRet( pRet );
    (*pRet->release)( pRet );
    return aRet;
}
//==================================================================================================
static Mapping getMediateMapping(
    const Environment & rFrom, const Environment & rTo, const OUString & rAddPurpose )
    SAL_THROW( () )
{
    Environment aUno;
    Mapping aUno2To;

    // backwards: from dest to source of mapping chain

    // connect to uno
    OUString aUnoEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO) );
    if (rTo.getTypeName() == aUnoEnvTypeName) // to is uno
    {
        aUno = rTo;
        // no Uno2To mapping necessary
    }
    else
    {
        // get registered uno env
        ::uno_getEnvironment( (uno_Environment **)&aUno, aUnoEnvTypeName.pData, 0 );

        aUno2To = getDirectMapping( aUno, rTo );
        // : uno <-> to
        if (! aUno2To.is())
            return Mapping();
    }

    // connect to uno
    if (rAddPurpose.getLength()) // insert purpose mapping between new ano_uno <-> uno
    {
        // create anonymous uno env
        Environment aAnUno;
        ::uno_createEnvironment( (uno_Environment **)&aAnUno, aUnoEnvTypeName.pData, 0 );

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
//##################################################################################################
void SAL_CALL uno_getMapping(
    uno_Mapping ** ppMapping, uno_Environment * pFrom, uno_Environment * pTo,
    rtl_uString * pAddPurpose )
    SAL_THROW_EXTERN_C()
{
    OSL_ENSURE( ppMapping && pFrom && pTo, "### null ptr!" );
    if (*ppMapping)
    {
        (*(*ppMapping)->release)( *ppMapping );
        *ppMapping = 0;
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
    if (!aRet.is() && pFrom == pTo && !aAddPurpose.getLength())
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
        for ( t_CallbackSet::const_iterator iPos( rData.aCallbacks.begin() );
              iPos != rData.aCallbacks.end(); ++iPos )
        {
            (**iPos)( ppMapping, pFrom, pTo, aAddPurpose.pData );
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
//##################################################################################################
void SAL_CALL uno_getMappingByName(
    uno_Mapping ** ppMapping, rtl_uString * pFrom, rtl_uString * pTo,
    rtl_uString * pAddPurpose )
    SAL_THROW_EXTERN_C()
{
    OSL_ENSURE( ppMapping && pFrom && pTo, "### null ptr!" );
    if (*ppMapping)
    {
        (*(*ppMapping)->release)( *ppMapping );
        *ppMapping = 0;
    }

    uno_Environment * pEFrom = 0;
    uno_getEnvironment( &pEFrom, pFrom, 0 );
    OSL_ENSURE( pEFrom, "### cannot get source environment!" );
    if (pEFrom)
    {
        uno_Environment * pETo = 0;
        uno_getEnvironment( &pETo, pTo, 0 );
        OSL_ENSURE( pETo, "### cannot get target environment!" );
        if (pETo)
        {
            ::uno_getMapping( ppMapping, pEFrom, pETo, pAddPurpose );
            (*pETo->release)( pETo );
        }
        (*pEFrom->release)( pEFrom );
    }
}

//##################################################################################################
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
#if OSL_DEBUG_LEVEL > 1
        OString cstr( OUStringToOString( aMappingName, RTL_TEXTENCODING_ASCII_US ) );
        OSL_TRACE( "> inserting new mapping: %s", cstr.getStr() );
#endif
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
//##################################################################################################
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
#if OSL_DEBUG_LEVEL > 1
        OString cstr( OUStringToOString( pEntry->aMappingName, RTL_TEXTENCODING_ASCII_US  ) );
        OSL_TRACE( "> revoking mapping %s", cstr.getStr() );
#endif
        (*pEntry->freeMapping)( pEntry->pMapping );
        delete pEntry;
    }
}

//##################################################################################################
void SAL_CALL uno_registerMappingCallback(
    uno_getMappingFunc pCallback )
    SAL_THROW_EXTERN_C()
{
    OSL_ENSURE( pCallback, "### null ptr!" );
    MappingsData & rData = getMappingsData();
    MutexGuard aGuard( rData.aCallbacksMutex );
    rData.aCallbacks.insert( pCallback );
}
//##################################################################################################
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

