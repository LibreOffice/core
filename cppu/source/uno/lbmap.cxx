/*************************************************************************
 *
 *  $RCSfile: lbmap.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: dbo $ $Date: 2000-09-21 11:40:40 $
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

#ifdef CPPU_ASSERTIONS
#define CPPU_TRACE(x) OSL_TRACE(x)
#else
#define CPPU_TRACE(x)
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif

#ifndef _UNO_DISPATCHER_H_
#include <uno/dispatcher.h>
#endif
#ifndef _UNO_MAPPING_HXX_
#include <uno/mapping.hxx>
#endif
#ifndef _UNO_ENVIRONMENT_HXX_
#include <uno/environment.hxx>
#endif

#ifndef _TYPELIB_TYPEDESCRIPTION_H_
#include <typelib/typedescription.h>
#endif

#include <com/sun/star/uno/XInterface.hpp>

#include <stl/hash_map>
#include <stl/set>
#include <stl/algorithm>


using namespace std;
using namespace osl;
using namespace rtl;
using namespace com::sun::star::uno;


//==================================================================================================
struct MappingEntry
{
    sal_Int32           nRef;
    uno_Mapping *       pMapping;
    uno_freeMappingFunc freeMapping;
    OUString            aMappingName;

    MappingEntry( uno_Mapping * pMapping_, uno_freeMappingFunc freeMapping_,
                  const OUString & rMappingName_ )
        : nRef( 1 )
        , pMapping( pMapping_ )
        , freeMapping( freeMapping_ )
        , aMappingName( rMappingName_ )
        {}
};
//--------------------------------------------------------------------------------------------------
struct FctOUStringHash : public unary_function< const OUString &, size_t >
{
    size_t operator()( const OUString & rKey ) const
        { return (size_t)rKey.hashCode(); }
};
//--------------------------------------------------------------------------------------------------
struct FctPtrHash : public unary_function< uno_Mapping *, size_t >
{
    size_t operator()( uno_Mapping * pKey ) const
        { return (size_t)pKey; }
};

typedef hash_map< OUString, MappingEntry *, FctOUStringHash, equal_to< OUString > > t_OUString2Entry;
typedef hash_map< uno_Mapping *, MappingEntry *, FctPtrHash, equal_to< uno_Mapping * > > t_Mapping2Entry;

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
    ~MappingsData();
};
//__________________________________________________________________________________________________
MappingsData::~MappingsData()
{
#ifdef CPPU_ASSERTIONS
    OSL_ENSHURE( aName2Entry.empty() && aMapping2Entry.empty(), "### unrevoked mappings!" );
    t_OUString2Entry::const_iterator iPos( aName2Entry.begin() );
    while (iPos != aName2Entry.end())
    {
        CPPU_TRACE( "\n### unrevoked mapping: " );
        MappingEntry * pEntry = (*iPos).second;
        OString aName( OUStringToOString( pEntry->aMappingName, RTL_TEXTENCODING_ASCII_US ) );
        CPPU_TRACE( aName.getStr() );
        ++iPos;
    }
    OSL_ENSHURE( aCallbacks.empty(), "### callbacks left!" );
    if (aCallbacks.size())
    {
        CPPU_TRACE( "\n### " );
        OString aSize( OString::valueOf( (sal_Int32)aCallbacks.size() ) );
        CPPU_TRACE( aSize.getStr() );
        CPPU_TRACE( " unrevoked callbacks" );
    }
#endif
}
//--------------------------------------------------------------------------------------------------
static MappingsData & getMappingsData()
{
    static MappingsData * s_p = 0;
    if (! s_p)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_p)
        {
            static MappingsData s_obj;
            s_p = &s_obj;
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

    uno_Mediate_Mapping( const Environment & rFrom_, const Environment & rTo_,
                         const Mapping & rFrom2Uno_, const Mapping & rUno2To_,
                         const OUString & rAddPurpose );
};
//--------------------------------------------------------------------------------------------------
static void SAL_CALL mediate_free( uno_Mapping * pMapping )
{
    delete static_cast< uno_Mediate_Mapping * >( pMapping );
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL mediate_acquire( uno_Mapping * pMapping )
{
    if (1 == osl_incrementInterlockedCount( & static_cast< uno_Mediate_Mapping * >( pMapping )->nRef ))
    {
        uno_registerMapping( &pMapping, mediate_free,
                             static_cast< uno_Mediate_Mapping * >( pMapping )->aFrom.get(),
                             static_cast< uno_Mediate_Mapping * >( pMapping )->aTo.get(),
                             static_cast< uno_Mediate_Mapping * >( pMapping )->aAddPurpose.pData );
    }
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL mediate_release( uno_Mapping * pMapping )
{
    if (! osl_decrementInterlockedCount( & static_cast< uno_Mediate_Mapping * >( pMapping )->nRef ))
    {
        uno_revokeMapping( pMapping );
    }
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL mediate_mapInterface( uno_Mapping * pMapping,
                                           void ** ppOut, void * pInterface,
                                           typelib_InterfaceTypeDescription * pInterfaceTypeDescr )
{
    OSL_ENSHURE( pMapping && ppOut, "### null ptr!" );
    if (pMapping && ppOut)
    {
        uno_Interface * pUnoI = 0;

        uno_Mapping * pFrom2Uno = static_cast< uno_Mediate_Mapping * >( pMapping )->aFrom2Uno.get();
        uno_Mapping * pUno2To   = static_cast< uno_Mediate_Mapping * >( pMapping )->aUno2To.get();

        (*pFrom2Uno->mapInterface)( pFrom2Uno, (void **)&pUnoI, pInterface, pInterfaceTypeDescr );
        if (pUnoI)
        {
            (*pUno2To->mapInterface)( pUno2To, ppOut, pUnoI, pInterfaceTypeDescr );
            (*pUnoI->release)( pUnoI );
        }
    }
}
//__________________________________________________________________________________________________
uno_Mediate_Mapping::uno_Mediate_Mapping( const Environment & rFrom_, const Environment & rTo_,
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

//==================================================================================================
static inline OUString getMappingName(
    const Environment & rFrom, const Environment & rTo, const OUString & rAddPurpose )
{
    OUStringBuffer aKey( 64 );
    aKey.append( rAddPurpose );
    aKey.append( (sal_Unicode)';' );
    aKey.append( rFrom.getTypeName() );
    aKey.append( (sal_Unicode)'[' );
    aKey.append( (sal_Int64)rFrom.get(), 16 );
    aKey.appendAscii( RTL_CONSTASCII_STRINGPARAM("];") );
    aKey.append( rTo.getTypeName() );
    aKey.append( (sal_Unicode)'[' );
    aKey.append( (sal_Int64)rTo.get(), 16 );
    aKey.append( (sal_Unicode)']' );
    return aKey.makeStringAndClear();
}
//==================================================================================================
static inline OUString getLibName(
    const Environment & rFrom, const Environment & rTo, const OUString & rAddPurpose )
{
    OUStringBuffer aLibName( 16 );
#ifdef SAL_UNX
    aLibName.appendAscii( RTL_CONSTASCII_STRINGPARAM("lib") );
    if (rAddPurpose.getLength())
    {
        aLibName.append( rAddPurpose );
        aLibName.append( (sal_Unicode)'_' );
    }
    aLibName.append( rFrom.getTypeName() );
    aLibName.append( (sal_Unicode)'_' );
    aLibName.append( rTo.getTypeName() );
    aLibName.appendAscii( RTL_CONSTASCII_STRINGPARAM(".so") );
#else
    if (rAddPurpose.getLength())
    {
        aLibName.append( rAddPurpose );
        aLibName.append( (sal_Unicode)'_' );
    }
    aLibName.append( rFrom.getTypeName() );
    aLibName.append( (sal_Unicode)'_' );
    aLibName.append( rTo.getTypeName() );
#ifndef OS2
    aLibName.appendAscii( RTL_CONSTASCII_STRINGPARAM(".dll") );
#endif
#endif
    return aLibName.makeStringAndClear();
}

//--------------------------------------------------------------------------------------------------
static inline void setNegativeModule( const OUString & rName )
{
    MappingsData & rData = getMappingsData();
    MutexGuard aGuard( rData.aNegativeLibsMutex );
    rData.aNegativeLibs.insert( rName );
}
//--------------------------------------------------------------------------------------------------
static inline oslModule loadModule( const OUString & rName )
{
    sal_Bool bNeg;
    {
    MappingsData & rData = getMappingsData();
    MutexGuard aGuard( rData.aNegativeLibsMutex );
    const t_OUStringSet::const_iterator iFind( rData.aNegativeLibs.find( rName ) );
    bNeg = (iFind != rData.aNegativeLibs.end());
    }

    if (! bNeg)
    {
        oslModule hModule = osl_loadModule( rName.pData, SAL_LOADMODULE_GLOBAL | SAL_LOADMODULE_LAZY );
        if (hModule)
            return hModule;
        setNegativeModule( rName ); // no load again
    }
    return 0;
}
//==================================================================================================
static Mapping loadExternalMapping(
    const Environment & rFrom, const Environment & rTo, const OUString & rAddPurpose )
{
    OSL_ASSERT( rFrom.is() && rTo.is() );
    if (rFrom.is() && rTo.is())
    {
        // find proper lib
        oslModule hModule = 0;
        OUString aName;

        if (rFrom.getTypeName().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_LB_UNO) ))
            hModule = loadModule( aName = getLibName( rTo, rFrom, rAddPurpose ) );
        if (! hModule)
            hModule = loadModule( aName = getLibName( rFrom, rTo, rAddPurpose ) );
        if (! hModule)
            hModule = loadModule( aName = getLibName( rTo, rFrom, rAddPurpose ) );

        if (hModule)
        {
            OUString aSymbolName( RTL_CONSTASCII_USTRINGPARAM(UNO_EXT_GETMAPPING) );
            uno_ext_getMappingFunc fpGetMapFunc =
                (uno_ext_getMappingFunc)osl_getSymbol( hModule, aSymbolName.pData );
            if (fpGetMapFunc)
            {
                Mapping aExt;
                (*fpGetMapFunc)( (uno_Mapping **)&aExt, rFrom.get(), rTo.get() );
                if (aExt.is())
                    return aExt;
            }
            osl_unloadModule( hModule );
            setNegativeModule( aName );
        }
    }
    return Mapping();
}

//==================================================================================================
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
{
    Environment aUno;
    Mapping aUno2To;

    // connect to uno
    if (rTo.getTypeName().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_LB_UNO ) )) // to is uno
    {
        aUno = rTo;
        // no Uno2To mapping necessary
    }
    else
    {
        // get registered uno env
        OUString aEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO) );
        uno_getEnvironment( (uno_Environment **)&aUno, aEnvTypeName.pData, 0 );

        aUno2To = getDirectMapping( aUno, rTo );
        // : uno <-> to
    }

    // connect to uno
    if (rAddPurpose.getLength()) // insert purpose mapping between new ano_uno <-> uno
    {
        // create anonymous uno env
        Environment aAnUno;
        OUString aEnvTypeName( RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO) );
        uno_createEnvironment( (uno_Environment **)&aAnUno, aEnvTypeName.pData, 0 );

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

//##################################################################################################
extern "C" SAL_DLLEXPORT void SAL_CALL uno_getMapping(
    uno_Mapping ** ppMapping, uno_Environment * pFrom, uno_Environment * pTo, rtl_uString * pAddPurpose )
{
    OSL_ENSHURE( ppMapping && pFrom && pTo, "### null ptr!" );
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
extern "C" SAL_DLLEXPORT void SAL_CALL uno_getMappingByName(
    uno_Mapping ** ppMapping, rtl_uString * pFrom, rtl_uString * pTo, rtl_uString * pAddPurpose )
{
    OSL_ENSHURE( ppMapping && pFrom && pTo, "### null ptr!" );
    if (*ppMapping)
    {
        (*(*ppMapping)->release)( *ppMapping );
        *ppMapping = 0;
    }

    uno_Environment * pEFrom = 0;
    uno_getEnvironment( &pEFrom, pFrom, 0 );
    OSL_ENSHURE( pEFrom, "### cannot get source environment!" );
    if (pEFrom)
    {
        uno_Environment * pETo = 0;
        uno_getEnvironment( &pETo, pTo, 0 );
        OSL_ENSHURE( pETo, "### cannot get target environment!" );
        if (pETo)
        {
            uno_getMapping( ppMapping, pEFrom, pETo, pAddPurpose );
            (*pETo->release)( pETo );
        }
        (*pEFrom->release)( pEFrom );
    }
}

//##################################################################################################
extern "C" SAL_DLLEXPORT void SAL_CALL uno_registerMapping(
    uno_Mapping ** ppMapping, uno_freeMappingFunc freeMapping,
    uno_Environment * pFrom, uno_Environment * pTo, rtl_uString * pAddPurpose )
{
    MappingsData & rData = getMappingsData();
    ClearableMutexGuard aGuard( rData.aMappingsMutex );

    const t_Mapping2Entry::const_iterator iFind( rData.aMapping2Entry.find( *ppMapping ) );
    if (iFind == rData.aMapping2Entry.end())
    {
        OUString aMappingName(
            getMappingName( pFrom, pTo, pAddPurpose ? OUString(pAddPurpose) : OUString() ) );
#ifdef CPPU_ASSERTIONS
        CPPU_TRACE( "> inserting new mapping " );
        OString aMappingName8( OUStringToOString( aMappingName, RTL_TEXTENCODING_ASCII_US ) );
        CPPU_TRACE( aMappingName8.getStr() );
        CPPU_TRACE( " <\n" );
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
extern "C" SAL_DLLEXPORT void SAL_CALL uno_revokeMapping( uno_Mapping * pMapping )
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
#ifdef CPPU_ASSERTIONS
        CPPU_TRACE( "> revoking mapping " );
        OString aMappingName( OUStringToOString( pEntry->aMappingName, RTL_TEXTENCODING_ASCII_US  ) );
        CPPU_TRACE( aMappingName.getStr() );
        CPPU_TRACE( " <\n" );
#endif
        (*pEntry->freeMapping)( pEntry->pMapping );
        delete pEntry;
    }
}

//##################################################################################################
extern "C" SAL_DLLEXPORT void SAL_CALL uno_registerMappingCallback( uno_getMappingFunc pCallback )
{
    OSL_ENSHURE( pCallback, "### null ptr!" );
    MappingsData & rData = getMappingsData();
    MutexGuard aGuard( rData.aCallbacksMutex );
    rData.aCallbacks.insert( pCallback );
}
//##################################################################################################
extern "C" SAL_DLLEXPORT void SAL_CALL uno_revokeMappingCallback( uno_getMappingFunc pCallback )
{
    OSL_ENSHURE( pCallback, "### null ptr!" );
    MappingsData & rData = getMappingsData();
    MutexGuard aGuard( rData.aCallbacksMutex );
    rData.aCallbacks.erase( pCallback );
}
