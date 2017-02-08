/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppu.hxx"

#include "cppu/EnvDcp.hxx"

#include "sal/alloca.h"
#include "osl/diagnose.h"
#include "osl/interlck.h"
#include "osl/mutex.hxx"
#include "osl/module.h"
#include "osl/process.h"
#include "rtl/process.h"
#include "rtl/unload.h"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/instance.hxx"
#include "typelib/typedescription.h"
#include "uno/dispatcher.h"
#include "uno/environment.h"
#include "uno/lbnames.h"
#include "prim.hxx"
#include "destr.hxx"
#include "loadmodule.hxx"

#include <hash_map>
#include <vector>
#include <stdio.h>


using ::rtl::OUString;

namespace
{

//------------------------------------------------------------------------------
inline static bool td_equals( typelib_InterfaceTypeDescription * pTD1,
                              typelib_InterfaceTypeDescription * pTD2 )
{
    return (pTD1 == pTD2 ||
            (((typelib_TypeDescription *)pTD1)->pTypeName->length ==
             ((typelib_TypeDescription *)pTD2)->pTypeName->length &&
             ::rtl_ustr_compare(
                 ((typelib_TypeDescription *) pTD1)->pTypeName->buffer,
                 ((typelib_TypeDescription *) pTD2)->pTypeName->buffer ) == 0));
}

struct ObjectEntry;
struct uno_DefaultEnvironment;

//------------------------------------------------------------------------------
struct InterfaceEntry
{
    sal_Int32 refCount;
    void * pInterface;
    uno_freeProxyFunc fpFreeProxy;
    typelib_InterfaceTypeDescription * pTypeDescr;
};

struct ObjectEntry
{
    OUString oid;
    sal_Int32 nRef;
    ::std::vector< InterfaceEntry > aInterfaces;
    bool mixedObject;

    inline ObjectEntry( const OUString & rOId_ );

    inline void append(
        uno_DefaultEnvironment * pEnv,
        void * pInterface, typelib_InterfaceTypeDescription * pTypeDescr,
        uno_freeProxyFunc fpFreeProxy );
    inline InterfaceEntry * find(
        typelib_InterfaceTypeDescription * pTypeDescr );
    inline sal_Int32 find( void * iface_ptr, ::std::size_t pos );
};

//------------------------------------------------------------------------------
struct FctPtrHash :
    public ::std::unary_function< const void *, ::std::size_t >
{
    ::std::size_t operator () ( const void * pKey ) const
        { return (::std::size_t) pKey; }
};

//------------------------------------------------------------------------------
struct FctOUStringHash :
    public ::std::unary_function< const OUString &, ::std::size_t >
{
    ::std::size_t operator () ( const OUString & rKey ) const
        { return rKey.hashCode(); }
};

// mapping from environment name to environment
typedef ::std::hash_map<
    OUString, uno_Environment *, FctOUStringHash,
    ::std::equal_to< OUString > > OUString2EnvironmentMap;

// mapping from ptr to object entry
typedef ::std::hash_map<
    void *, ObjectEntry *, FctPtrHash,
    ::std::equal_to< void * > > Ptr2ObjectMap;
// mapping from oid to object entry
typedef ::std::hash_map<
    OUString, ObjectEntry *, FctOUStringHash,
    ::std::equal_to< OUString > > OId2ObjectMap;


//==============================================================================
struct EnvironmentsData
{
    ::osl::Mutex mutex;
    OUString2EnvironmentMap aName2EnvMap;

    EnvironmentsData() : isDisposing(false) {}
    ~EnvironmentsData();

    inline void getEnvironment(
        uno_Environment ** ppEnv, const OUString & rEnvDcp, void * pContext );
    inline void registerEnvironment( uno_Environment ** ppEnv );
    inline void getRegisteredEnvironments(
        uno_Environment *** pppEnvs, sal_Int32 * pnLen,
        uno_memAlloc memAlloc, const OUString & rEnvDcp );

    bool isDisposing;
};

namespace
{
    struct theEnvironmentsData : public rtl::Static< EnvironmentsData, theEnvironmentsData > {};
}

//==============================================================================
struct uno_DefaultEnvironment : public uno_ExtEnvironment
{
    sal_Int32 nRef;
    sal_Int32 nWeakRef;

    ::osl::Mutex mutex;
    Ptr2ObjectMap aPtr2ObjectMap;
    OId2ObjectMap aOId2ObjectMap;

    uno_DefaultEnvironment(
        const OUString & rEnvDcp_, void * pContext_ );
    ~uno_DefaultEnvironment();
};

//______________________________________________________________________________
inline ObjectEntry::ObjectEntry( OUString const & rOId_ )
    : oid( rOId_ ),
      nRef( 0 ),
      mixedObject( false )
{
    aInterfaces.reserve( 2 );
}

//______________________________________________________________________________
inline void ObjectEntry::append(
    uno_DefaultEnvironment * pEnv,
    void * pInterface, typelib_InterfaceTypeDescription * pTypeDescr,
    uno_freeProxyFunc fpFreeProxy )
{
    InterfaceEntry aNewEntry;
    if (! fpFreeProxy)
        (*pEnv->acquireInterface)( pEnv, pInterface );
    aNewEntry.refCount = 1;
    aNewEntry.pInterface = pInterface;
    aNewEntry.fpFreeProxy = fpFreeProxy;
    typelib_typedescription_acquire( (typelib_TypeDescription *) pTypeDescr );
    aNewEntry.pTypeDescr = pTypeDescr;

    ::std::pair< Ptr2ObjectMap::iterator, bool > insertion(
        pEnv->aPtr2ObjectMap.insert( Ptr2ObjectMap::value_type(
                                         pInterface, this ) ) );
    OSL_ASSERT( insertion.second ||
                (find( pInterface, 0 ) >= 0 &&
                 // points to the same object entry:
                 insertion.first->second == this) );
    aInterfaces.push_back( aNewEntry );
}

//______________________________________________________________________________
inline InterfaceEntry * ObjectEntry::find(
    typelib_InterfaceTypeDescription * pTypeDescr_ )
{
    OSL_ASSERT( ! aInterfaces.empty() );
    if (aInterfaces.empty())
        return 0;

    // shortcut common case:
    OUString const & type_name =
        OUString::unacquired(
            &((typelib_TypeDescription *) pTypeDescr_)->pTypeName );
    if (type_name.equalsAsciiL(
            RTL_CONSTASCII_STRINGPARAM("com.sun.star.uno.XInterface") ))
    {
        return &aInterfaces[ 0 ];
    }

    ::std::size_t nSize = aInterfaces.size();
    for ( ::std::size_t nPos = 0; nPos < nSize; ++nPos )
    {
        typelib_InterfaceTypeDescription * pITD =
            aInterfaces[ nPos ].pTypeDescr;
        while (pITD)
        {
            if (td_equals( pITD, pTypeDescr_ ))
                return &aInterfaces[ nPos ];
            pITD = pITD->pBaseTypeDescription;
        }
    }
    return 0;
}

//______________________________________________________________________________
inline sal_Int32 ObjectEntry::find(
    void * iface_ptr, ::std::size_t pos )
{
    ::std::size_t size = aInterfaces.size();
    for ( ; pos < size; ++pos )
    {
        if (aInterfaces[ pos ].pInterface == iface_ptr)
            return pos;
    }
    return -1;
}

extern "C"
{

//------------------------------------------------------------------------------
static void SAL_CALL defenv_registerInterface(
    uno_ExtEnvironment * pEnv, void ** ppInterface,
    rtl_uString * pOId, typelib_InterfaceTypeDescription * pTypeDescr )
{
    OSL_ENSURE( pEnv && ppInterface && pOId && pTypeDescr, "### null ptr!" );
    OUString sOId( pOId );

    uno_DefaultEnvironment * that =
        static_cast< uno_DefaultEnvironment * >( pEnv );
    ::osl::ClearableMutexGuard guard( that->mutex );

    // try to insert dummy 0:
    std::pair<OId2ObjectMap::iterator, bool> const insertion(
        that->aOId2ObjectMap.insert( OId2ObjectMap::value_type( sOId, 0 ) ) );
    if (insertion.second)
    {
        ObjectEntry * pOEntry = new ObjectEntry( sOId );
        insertion.first->second = pOEntry;
        ++pOEntry->nRef; // another register call on object
        pOEntry->append( that, *ppInterface, pTypeDescr, 0 );
    }
    else // object entry exists
    {
        ObjectEntry * pOEntry = insertion.first->second;
        ++pOEntry->nRef; // another register call on object
        InterfaceEntry * pIEntry = pOEntry->find( pTypeDescr );

        if (pIEntry) // type entry exists
        {
            ++pIEntry->refCount;
            if (pIEntry->pInterface != *ppInterface)
            {
                void * pInterface = pIEntry->pInterface;
                (*pEnv->acquireInterface)( pEnv, pInterface );
                guard.clear();
                (*pEnv->releaseInterface)( pEnv, *ppInterface );
                *ppInterface = pInterface;
            }
        }
        else
        {
            pOEntry->append( that, *ppInterface, pTypeDescr, 0 );
        }
    }
}

//------------------------------------------------------------------------------
static void SAL_CALL defenv_registerProxyInterface(
    uno_ExtEnvironment * pEnv, void ** ppInterface, uno_freeProxyFunc freeProxy,
    rtl_uString * pOId, typelib_InterfaceTypeDescription * pTypeDescr )
{
    OSL_ENSURE( pEnv && ppInterface && pOId && pTypeDescr && freeProxy,
                "### null ptr!" );
    OUString sOId( pOId );

    uno_DefaultEnvironment * that =
        static_cast< uno_DefaultEnvironment * >( pEnv );
    ::osl::ClearableMutexGuard guard( that->mutex );

    // try to insert dummy 0:
    std::pair<OId2ObjectMap::iterator, bool> const insertion(
        that->aOId2ObjectMap.insert( OId2ObjectMap::value_type( sOId, 0 ) ) );
    if (insertion.second)
    {
        ObjectEntry * pOEntry = new ObjectEntry( sOId );
        insertion.first->second = pOEntry;
        ++pOEntry->nRef; // another register call on object
        pOEntry->append( that, *ppInterface, pTypeDescr, freeProxy );
    }
    else // object entry exists
    {
        ObjectEntry * pOEntry = insertion.first->second;

        // first registration was an original, then registerProxyInterface():
        pOEntry->mixedObject |=
            (!pOEntry->aInterfaces.empty() &&
             pOEntry->aInterfaces[ 0 ].fpFreeProxy == 0);

        ++pOEntry->nRef; // another register call on object
        InterfaceEntry * pIEntry = pOEntry->find( pTypeDescr );

        if (pIEntry) // type entry exists
        {
            if (pIEntry->pInterface == *ppInterface)
            {
                ++pIEntry->refCount;
            }
            else
            {
                void * pInterface = pIEntry->pInterface;
                (*pEnv->acquireInterface)( pEnv, pInterface );
                --pOEntry->nRef; // manual revoke of proxy to be freed
                guard.clear();
                (*freeProxy)( pEnv, *ppInterface );
                *ppInterface = pInterface;
            }
        }
        else
        {
            pOEntry->append( that, *ppInterface, pTypeDescr, freeProxy );
        }
    }
}

//------------------------------------------------------------------------------
static void SAL_CALL s_stub_defenv_revokeInterface(va_list * pParam)
{
    uno_ExtEnvironment * pEnv       = va_arg(*pParam, uno_ExtEnvironment *);
    void               * pInterface = va_arg(*pParam, void *);

    OSL_ENSURE( pEnv && pInterface, "### null ptr!" );
    uno_DefaultEnvironment * that =
        static_cast< uno_DefaultEnvironment * >( pEnv );
    ::osl::ClearableMutexGuard guard( that->mutex );

    Ptr2ObjectMap::const_iterator const iFind(
        that->aPtr2ObjectMap.find( pInterface ) );
    OSL_ASSERT( iFind != that->aPtr2ObjectMap.end() );
    ObjectEntry * pOEntry = iFind->second;
    if (! --pOEntry->nRef)
    {
        // cleanup maps
        that->aOId2ObjectMap.erase( pOEntry->oid );
        sal_Int32 nPos;
        for ( nPos = pOEntry->aInterfaces.size(); nPos--; )
        {
            that->aPtr2ObjectMap.erase( pOEntry->aInterfaces[nPos].pInterface );
        }

        // the last proxy interface of the environment might kill this
        // environment, because of releasing its language binding!!!
        guard.clear();

        // release interfaces
        for ( nPos = pOEntry->aInterfaces.size(); nPos--; )
        {
            InterfaceEntry const & rEntry = pOEntry->aInterfaces[nPos];
            typelib_typedescription_release(
                (typelib_TypeDescription *) rEntry.pTypeDescr );
            if (rEntry.fpFreeProxy) // is proxy or used interface?
            {
                (*rEntry.fpFreeProxy)( pEnv, rEntry.pInterface );
            }
            else
            {
                (*pEnv->releaseInterface)( pEnv, rEntry.pInterface );
            }
        }

        delete pOEntry;
    }
    else if (pOEntry->mixedObject)
    {
        OSL_ASSERT( !pOEntry->aInterfaces.empty() &&
                    pOEntry->aInterfaces[ 0 ].fpFreeProxy == 0 );

        sal_Int32 index = pOEntry->find( pInterface, 1 );
        OSL_ASSERT( index > 0 );
        if (index > 0)
        {
            InterfaceEntry & entry = pOEntry->aInterfaces[ index ];
            OSL_ASSERT( entry.pInterface == pInterface );
            if (entry.fpFreeProxy != 0)
            {
                --entry.refCount;
                if (entry.refCount == 0)
                {
                    uno_freeProxyFunc fpFreeProxy = entry.fpFreeProxy;
                    typelib_TypeDescription * pTypeDescr =
                        reinterpret_cast< typelib_TypeDescription * >(
                            entry.pTypeDescr );

                    pOEntry->aInterfaces.erase(
                        pOEntry->aInterfaces.begin() + index );
                    if (pOEntry->find( pInterface, index ) < 0)
                    {
                        // proxy ptr not registered for another interface:
                        // remove from ptr map
#if OSL_DEBUG_LEVEL > 0
                        ::std::size_t erased =
#endif
                              that->aPtr2ObjectMap.erase( pInterface );
                        OSL_ASSERT( erased == 1 );
                    }

                    guard.clear();

                    typelib_typedescription_release( pTypeDescr );
                    (*fpFreeProxy)( pEnv, pInterface );
                }
            }
        }
    }
}

static void SAL_CALL defenv_revokeInterface(uno_ExtEnvironment * pEnv, void * pInterface)
{
    uno_Environment_invoke(&pEnv->aBase, s_stub_defenv_revokeInterface, pEnv, pInterface);
}

//------------------------------------------------------------------------------
static void SAL_CALL defenv_getObjectIdentifier(
    uno_ExtEnvironment * pEnv, rtl_uString ** ppOId, void * pInterface )
{
    OSL_ENSURE( pEnv && ppOId && pInterface, "### null ptr!" );
    if (*ppOId)
    {
        ::rtl_uString_release( *ppOId );
        *ppOId = 0;
    }

    uno_DefaultEnvironment * that =
        static_cast< uno_DefaultEnvironment * >( pEnv );
    ::osl::ClearableMutexGuard guard( that->mutex );

    Ptr2ObjectMap::const_iterator const iFind(
        that->aPtr2ObjectMap.find( pInterface ) );
    if (iFind == that->aPtr2ObjectMap.end())
    {
        guard.clear();
        (*pEnv->computeObjectIdentifier)( pEnv, ppOId, pInterface );
    }
    else
    {
        rtl_uString * hstr = iFind->second->oid.pData;
        rtl_uString_acquire( hstr );
        *ppOId = hstr;
    }
}

//------------------------------------------------------------------------------
static void SAL_CALL defenv_getRegisteredInterface(
    uno_ExtEnvironment * pEnv, void ** ppInterface,
    rtl_uString * pOId, typelib_InterfaceTypeDescription * pTypeDescr )
{
    OSL_ENSURE( pEnv && ppInterface && pOId && pTypeDescr, "### null ptr!" );
    if (*ppInterface)
    {
        (*pEnv->releaseInterface)( pEnv, *ppInterface );
        *ppInterface = 0;
    }

    OUString sOId( pOId );
    uno_DefaultEnvironment * that =
        static_cast< uno_DefaultEnvironment * >( pEnv );
    ::osl::MutexGuard guard( that->mutex );

    OId2ObjectMap::const_iterator const iFind
        ( that->aOId2ObjectMap.find( sOId ) );
    if (iFind != that->aOId2ObjectMap.end())
    {
        InterfaceEntry const * pIEntry = iFind->second->find( pTypeDescr );
        if (pIEntry)
        {
            (*pEnv->acquireInterface)( pEnv, pIEntry->pInterface );
            *ppInterface = pIEntry->pInterface;
        }
    }
}

//------------------------------------------------------------------------------
static void SAL_CALL defenv_getRegisteredInterfaces(
    uno_ExtEnvironment * pEnv, void *** pppInterfaces, sal_Int32 * pnLen,
    uno_memAlloc memAlloc )
{
    OSL_ENSURE( pEnv && pppInterfaces && pnLen && memAlloc, "### null ptr!" );
    uno_DefaultEnvironment * that =
        static_cast< uno_DefaultEnvironment * >( pEnv );
    ::osl::MutexGuard guard( that->mutex );

    sal_Int32 nLen = that->aPtr2ObjectMap.size();
    sal_Int32 nPos = 0;
    void ** ppInterfaces = (void **) (*memAlloc)( nLen * sizeof (void *) );

    Ptr2ObjectMap::const_iterator iPos( that->aPtr2ObjectMap.begin() );
    Ptr2ObjectMap::const_iterator const iEnd( that->aPtr2ObjectMap.end() );
    while (iPos != iEnd)
    {
        (*pEnv->acquireInterface)( pEnv, ppInterfaces[nPos++] = (*iPos).first );
        ++iPos;
    }

    *pppInterfaces = ppInterfaces;
    *pnLen = nLen;
}

//------------------------------------------------------------------------------
static void SAL_CALL defenv_acquire( uno_Environment * pEnv )
{
    uno_DefaultEnvironment * that = (uno_DefaultEnvironment *)pEnv;
    ::osl_incrementInterlockedCount( &that->nWeakRef );
    ::osl_incrementInterlockedCount( &that->nRef );
}

//------------------------------------------------------------------------------
static void SAL_CALL defenv_release( uno_Environment * pEnv )
{
    uno_DefaultEnvironment * that = (uno_DefaultEnvironment *)pEnv;
    if (! ::osl_decrementInterlockedCount( &that->nRef ))
    {
        // invoke dispose callback
        if (pEnv->environmentDisposing)
        {
            (*pEnv->environmentDisposing)( pEnv );
        }

        OSL_ENSURE( that->aOId2ObjectMap.empty(), "### object entries left!" );
    }
    // free memory if no weak refs left
    if (! ::osl_decrementInterlockedCount( &that->nWeakRef ))
    {
        delete that;
    }
}

//------------------------------------------------------------------------------
static void SAL_CALL defenv_acquireWeak( uno_Environment * pEnv )
{
    uno_DefaultEnvironment * that = (uno_DefaultEnvironment *)pEnv;
    ::osl_incrementInterlockedCount( &that->nWeakRef );
}

//------------------------------------------------------------------------------
static void SAL_CALL defenv_releaseWeak( uno_Environment * pEnv )
{
    uno_DefaultEnvironment * that = (uno_DefaultEnvironment *)pEnv;
    if (! ::osl_decrementInterlockedCount( &that->nWeakRef ))
    {
        delete that;
    }
}

//------------------------------------------------------------------------------
static void SAL_CALL defenv_harden(
    uno_Environment ** ppHardEnv, uno_Environment * pEnv )
{
    if (*ppHardEnv)
    {
        (*(*ppHardEnv)->release)( *ppHardEnv );
        *ppHardEnv = 0;
    }

    EnvironmentsData & rData = theEnvironmentsData::get();

    if (rData.isDisposing)
        return;

    uno_DefaultEnvironment * that = (uno_DefaultEnvironment *)pEnv;
    {
    ::osl::MutexGuard guard( rData.mutex );
    if (1 == ::osl_incrementInterlockedCount( &that->nRef )) // is dead
    {
        that->nRef = 0;
        return;
    }
    }
    ::osl_incrementInterlockedCount( &that->nWeakRef );
    *ppHardEnv = pEnv;
}

//------------------------------------------------------------------------------
static void SAL_CALL defenv_dispose( uno_Environment * )
{
}
}

//______________________________________________________________________________
uno_DefaultEnvironment::uno_DefaultEnvironment(
    const OUString & rEnvDcp_, void * pContext_ )
    : nRef( 0 ),
      nWeakRef( 0 )
{
    uno_Environment * that = reinterpret_cast< uno_Environment * >(this);
    that->pReserved = 0;
    // functions
    that->acquire = defenv_acquire;
    that->release = defenv_release;
    that->acquireWeak = defenv_acquireWeak;
    that->releaseWeak = defenv_releaseWeak;
    that->harden = defenv_harden;
    that->dispose = defenv_dispose;
    that->pExtEnv = this;
    // identifier
    ::rtl_uString_acquire( rEnvDcp_.pData );
    that->pTypeName = rEnvDcp_.pData;
    that->pContext = pContext_;

    // will be late initialized
    that->environmentDisposing = 0;

    uno_ExtEnvironment::registerInterface = defenv_registerInterface;
    uno_ExtEnvironment::registerProxyInterface = defenv_registerProxyInterface;
    uno_ExtEnvironment::revokeInterface = defenv_revokeInterface;
    uno_ExtEnvironment::getObjectIdentifier = defenv_getObjectIdentifier;
    uno_ExtEnvironment::getRegisteredInterface = defenv_getRegisteredInterface;
    uno_ExtEnvironment::getRegisteredInterfaces =
        defenv_getRegisteredInterfaces;

}

//______________________________________________________________________________
uno_DefaultEnvironment::~uno_DefaultEnvironment()
{
    ::rtl_uString_release( ((uno_Environment *) this)->pTypeName );
}

//==============================================================================
static void writeLine(
    void * stream, const sal_Char * pLine, const sal_Char * pFilter )
{
    if (pFilter && *pFilter)
    {
        // lookup pFilter in pLine
        while (*pLine)
        {
            if (*pLine == *pFilter)
            {
                sal_Int32 nPos = 1;
                while (pLine[nPos] && pFilter[nPos] == pLine[nPos])
                {
                    ++nPos;
                }
                if (! pFilter[nPos])
                {
                    if (stream)
                    {
                        fprintf( (FILE *) stream, "%s\n", pLine );
                    }
                    else
                    {
                        OSL_TRACE( pLine );
                        OSL_TRACE( "\n" );
                    }
                }
            }
            ++pLine;
        }
    }
    else
    {
        if (stream)
        {
            fprintf( (FILE *) stream, "%s\n", pLine );
        }
        else
        {
            fprintf( stderr, "%s\n", pLine );
        }
    }
}

//==============================================================================
static void writeLine(
    void * stream, const OUString & rLine, const sal_Char * pFilter )
{
    ::rtl::OString aLine( ::rtl::OUStringToOString(
                              rLine, RTL_TEXTENCODING_ASCII_US ) );
    writeLine( stream, aLine.getStr(), pFilter );
}

//##############################################################################
extern "C" void SAL_CALL uno_dumpEnvironment(
    void * stream, uno_Environment * pEnv, const sal_Char * pFilter )
    SAL_THROW_EXTERN_C()
{
    OSL_ENSURE( pEnv, "### null ptr!" );
    ::rtl::OUStringBuffer buf;

    if (! pEnv->pExtEnv)
    {
        writeLine( stream, "###################################"
                   "###########################################", pFilter );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("environment: ") );
        buf.append( pEnv->pTypeName );
        writeLine( stream, buf.makeStringAndClear(), pFilter );
        writeLine( stream, "NO INTERFACE INFORMATION AVAILABLE!", pFilter );
        return;
    }

    writeLine( stream, "########################################"
               "######################################", pFilter );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("environment dump: ") );
    buf.append( pEnv->pTypeName );
    writeLine( stream, buf.makeStringAndClear(), pFilter );

    uno_DefaultEnvironment * that =
        reinterpret_cast< uno_DefaultEnvironment * >(pEnv);
    ::osl::MutexGuard guard( that->mutex );

    Ptr2ObjectMap ptr2obj( that->aPtr2ObjectMap );
    OId2ObjectMap::const_iterator iPos( that->aOId2ObjectMap.begin() );
    while (iPos != that->aOId2ObjectMap.end())
    {
        ObjectEntry * pOEntry = iPos->second;

        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("+ ") );
        if (pOEntry->mixedObject)
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("mixed ") );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("object entry: nRef=") );
        buf.append( pOEntry->nRef, 10 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("; oid=\"") );
        buf.append( pOEntry->oid );
        buf.append( (sal_Unicode) '\"' );
        writeLine( stream, buf.makeStringAndClear(), pFilter );

        for ( ::std::size_t nPos = 0;
              nPos < pOEntry->aInterfaces.size(); ++nPos )
        {
            const InterfaceEntry & rIEntry = pOEntry->aInterfaces[nPos];

            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("  - ") );
            buf.append(
                ((typelib_TypeDescription *) rIEntry.pTypeDescr)->pTypeName );
            if (rIEntry.fpFreeProxy)
            {
                buf.appendAscii(
                    RTL_CONSTASCII_STRINGPARAM("; proxy free=0x") );
                buf.append(
                    reinterpret_cast< sal_IntPtr >(rIEntry.fpFreeProxy), 16 );
            }
            else
            {
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("; original") );
            }
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("; ptr=0x") );
            buf.append(
                reinterpret_cast< sal_IntPtr >(rIEntry.pInterface), 16 );

            if (pOEntry->find( rIEntry.pInterface, nPos + 1 ) < 0)
            {
                ::std::size_t erased = ptr2obj.erase( rIEntry.pInterface );
                if (erased != 1)
                {
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
                                         " (ptr not found in map!)") );
                }
            }
            writeLine( stream, buf.makeStringAndClear(), pFilter );
        }
        ++iPos;
    }
    if (! ptr2obj.empty())
        writeLine( stream, "ptr map inconsistency!!!", pFilter );
    writeLine( stream, "#####################################"
               "#########################################", pFilter );
}

//##############################################################################
extern "C" void SAL_CALL uno_dumpEnvironmentByName(
    void * stream, rtl_uString * pEnvDcp, const sal_Char * pFilter )
    SAL_THROW_EXTERN_C()
{
    uno_Environment * pEnv = 0;
    uno_getEnvironment( &pEnv, pEnvDcp, 0 );
    if (pEnv)
    {
        ::uno_dumpEnvironment( stream, pEnv, pFilter );
        (*pEnv->release)( pEnv );
    }
    else
    {
        ::rtl::OUStringBuffer buf( 32 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("environment \"") );
        buf.append( pEnvDcp );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" does not exist!") );
        writeLine( stream, buf.makeStringAndClear(), pFilter );
    }
}

//------------------------------------------------------------------------------
inline static const OUString & unoenv_getStaticOIdPart()
{
    static OUString * s_pStaticOidPart = 0;
    if (! s_pStaticOidPart)
    {
        ::osl::MutexGuard guard( ::osl::Mutex::getGlobalMutex() );
        if (! s_pStaticOidPart)
        {
            ::rtl::OUStringBuffer aRet( 64 );
            aRet.appendAscii( RTL_CONSTASCII_STRINGPARAM("];") );
            // pid
            oslProcessInfo info;
            info.Size = sizeof(oslProcessInfo);
            if (::osl_getProcessInfo( 0, osl_Process_IDENTIFIER, &info ) ==
                osl_Process_E_None)
            {
                aRet.append( (sal_Int64)info.Ident, 16 );
            }
            else
            {
                aRet.appendAscii(
                    RTL_CONSTASCII_STRINGPARAM("unknown process id") );
            }
            // good guid
            sal_uInt8 ar[16];
            ::rtl_getGlobalProcessId( ar );
            aRet.append( (sal_Unicode)';' );
            for ( sal_Int32 i = 0; i < 16; ++i )
                aRet.append( (sal_Int32)ar[i], 16 );

            static OUString s_aStaticOidPart( aRet.makeStringAndClear() );
            s_pStaticOidPart = &s_aStaticOidPart;
        }
    }
    return *s_pStaticOidPart;
}

extern "C"
{

//------------------------------------------------------------------------------
static void SAL_CALL unoenv_computeObjectIdentifier(
    uno_ExtEnvironment * pEnv, rtl_uString ** ppOId, void * pInterface )
{
    OSL_ENSURE( pEnv && ppOId && pInterface, "### null ptr!" );
    if (*ppOId)
    {
        ::rtl_uString_release( *ppOId );
        *ppOId = 0;
    }

    uno_Interface * pUnoI = (uno_Interface *)
        ::cppu::binuno_queryInterface(
            pInterface, *typelib_static_type_getByTypeClass(
                typelib_TypeClass_INTERFACE ) );
    if (0 != pUnoI)
    {
        (*pUnoI->release)( pUnoI );
        // interface
        ::rtl::OUStringBuffer oid( 64 );
        oid.append( reinterpret_cast< sal_Int64 >(pUnoI), 16 );
        oid.append( static_cast< sal_Unicode >(';') );
        // environment[context]
        oid.append( ((uno_Environment *) pEnv)->pTypeName );
        oid.append( static_cast< sal_Unicode >('[') );
        oid.append( reinterpret_cast< sal_Int64 >(
                        reinterpret_cast<
                        uno_Environment * >(pEnv)->pContext ), 16 );
        // process;good guid
        oid.append( unoenv_getStaticOIdPart() );
        OUString aStr( oid.makeStringAndClear() );
        ::rtl_uString_acquire( *ppOId = aStr.pData );
    }
}

//==============================================================================
static void SAL_CALL unoenv_acquireInterface(
    uno_ExtEnvironment *, void * pUnoI_ )
{
    uno_Interface * pUnoI = reinterpret_cast< uno_Interface * >(pUnoI_);
    (*pUnoI->acquire)( pUnoI );
}

//==============================================================================
static void SAL_CALL unoenv_releaseInterface(
    uno_ExtEnvironment *, void * pUnoI_ )
{
    uno_Interface * pUnoI = reinterpret_cast< uno_Interface * >(pUnoI_);
    (*pUnoI->release)( pUnoI );
}
}

//______________________________________________________________________________
EnvironmentsData::~EnvironmentsData()
{
    ::osl::MutexGuard guard( mutex );
    isDisposing = true;

    for ( OUString2EnvironmentMap::const_iterator iPos( aName2EnvMap.begin() );
          iPos != aName2EnvMap.end(); ++iPos )
    {
        uno_Environment * pWeak = iPos->second;
        uno_Environment * pHard = 0;
        (*pWeak->harden)( &pHard, pWeak );
        (*pWeak->releaseWeak)( pWeak );

        if (pHard)
        {
#if OSL_DEBUG_LEVEL > 1
            ::uno_dumpEnvironment( 0, pHard, 0 );
#endif
            (*pHard->dispose)( pHard ); // send explicit dispose
            (*pHard->release)( pHard );
        }
    }
}

//______________________________________________________________________________
inline void EnvironmentsData::getEnvironment(
    uno_Environment ** ppEnv, const OUString & rEnvDcp, void * pContext )
{
    if (*ppEnv)
    {
        (*(*ppEnv)->release)( *ppEnv );
        *ppEnv = 0;
    }

    OUString aKey(
        OUString::valueOf( reinterpret_cast< sal_IntPtr >(pContext) ) );
    aKey += rEnvDcp;

    // try to find registered mapping
    OUString2EnvironmentMap::const_iterator const iFind(
        aName2EnvMap.find( aKey ) );
    if (iFind != aName2EnvMap.end())
    {
        uno_Environment * pWeak = iFind->second;
        (*pWeak->harden)( ppEnv, pWeak );
    }
}

//______________________________________________________________________________
inline void EnvironmentsData::registerEnvironment( uno_Environment ** ppEnv )
{
    OSL_ENSURE( ppEnv, "### null ptr!" );
    uno_Environment * pEnv =  *ppEnv;

    OUString aKey(
        OUString::valueOf( reinterpret_cast< sal_IntPtr >(pEnv->pContext) ) );
    aKey += pEnv->pTypeName;

    // try to find registered environment
    OUString2EnvironmentMap::const_iterator const iFind(
        aName2EnvMap.find( aKey ) );
    if (iFind == aName2EnvMap.end())
    {
        (*pEnv->acquireWeak)( pEnv );
        ::std::pair< OUString2EnvironmentMap::iterator, bool > insertion(
            aName2EnvMap.insert(
                OUString2EnvironmentMap::value_type( aKey, pEnv ) ) );
        OSL_ENSURE(
            insertion.second, "### insertion of env into map failed?!" );
    }
    else
    {
        uno_Environment * pHard = 0;
        uno_Environment * pWeak = iFind->second;
        (*pWeak->harden)( &pHard, pWeak );
        if (pHard)
        {
            if (pEnv)
                (*pEnv->release)( pEnv );
            *ppEnv = pHard;
        }
        else // registered one is dead
        {
            (*pWeak->releaseWeak)( pWeak );
            (*pEnv->acquireWeak)( pEnv );
            aName2EnvMap[ aKey ] = pEnv;
        }
    }
}

//______________________________________________________________________________
inline void EnvironmentsData::getRegisteredEnvironments(
    uno_Environment *** pppEnvs, sal_Int32 * pnLen, uno_memAlloc memAlloc,
    const OUString & rEnvDcp )
{
    OSL_ENSURE( pppEnvs && pnLen && memAlloc, "### null ptr!" );

    // max size
    uno_Environment ** ppFound = (uno_Environment **)alloca(
        sizeof(uno_Environment *) * aName2EnvMap.size() );
    sal_Int32 nSize = 0;

    // find matching environment
    for ( OUString2EnvironmentMap::const_iterator iPos( aName2EnvMap.begin() );
          iPos != aName2EnvMap.end(); ++iPos )
    {
        uno_Environment * pWeak = iPos->second;
        if (!rEnvDcp.getLength() ||
            rEnvDcp.equals( pWeak->pTypeName ))
        {
            ppFound[nSize] = 0;
            (*pWeak->harden)( &ppFound[nSize], pWeak );
            if (ppFound[nSize])
                ++nSize;
        }
    }

    *pnLen = nSize;
    if (nSize)
    {
        *pppEnvs = (uno_Environment **) (*memAlloc)(
            sizeof (uno_Environment *) * nSize );
        OSL_ASSERT( *pppEnvs );
        while (nSize--)
        {
            (*pppEnvs)[nSize] = ppFound[nSize];
        }
    }
    else
    {
        *pppEnvs = 0;
    }
}

static bool loadEnv(OUString const  & cLibStem,
                    uno_Environment * pEnv,
                    void            * /*pContext*/)
{
    // late init with some code from matching uno language binding
    // will be unloaded by environment
    oslModule hMod = cppu::detail::loadModule( cLibStem );

    if (!hMod)
        return false;

    OUString aSymbolName(RTL_CONSTASCII_USTRINGPARAM(UNO_INIT_ENVIRONMENT));
    uno_initEnvironmentFunc fpInit = (uno_initEnvironmentFunc)
        ::osl_getFunctionSymbol( hMod, aSymbolName.pData );
    if (!fpInit)
    {
        ::osl_unloadModule( hMod );
        return false;
    }

    (*fpInit)( pEnv ); // init of environment
    ::rtl_registerModuleForUnloading( hMod );

    return true;
}


extern "C"
{

//------------------------------------------------------------------------------
static uno_Environment * initDefaultEnvironment(
    const OUString & rEnvDcp, void * pContext )
{
    uno_Environment * pEnv = &(new uno_DefaultEnvironment( rEnvDcp, pContext ))->aBase;
    (*pEnv->acquire)( pEnv );

    OUString envTypeName = cppu::EnvDcp::getTypeName(rEnvDcp);

    // create default environment
    if (envTypeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_LB_UNO) ))
    {
        uno_DefaultEnvironment * that = (uno_DefaultEnvironment *)pEnv;
        that->computeObjectIdentifier = unoenv_computeObjectIdentifier;
        that->acquireInterface = unoenv_acquireInterface;
        that->releaseInterface = unoenv_releaseInterface;

        OUString envPurpose = cppu::EnvDcp::getPurpose(rEnvDcp);
        if (envPurpose.getLength())
        {
            rtl::OUString libStem = envPurpose.copy(envPurpose.lastIndexOf(':') + 1);
            libStem += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("_uno_uno") );

            if(!loadEnv(libStem, pEnv, pContext))
            {
                pEnv->release(pEnv);
                return NULL;
            }
        }
    }
    else
    {
        // late init with some code from matching uno language binding
        ::rtl::OUStringBuffer aLibName( 16 );
        aLibName.append( envTypeName );
        aLibName.appendAscii( RTL_CONSTASCII_STRINGPARAM("_uno" ) );
        OUString aStr( aLibName.makeStringAndClear() );

        if (!loadEnv(aStr, pEnv, pContext))
        {
            pEnv->release(pEnv);
            return NULL;
        }
    }

    return pEnv;
}

//##############################################################################
void SAL_CALL uno_createEnvironment(
    uno_Environment ** ppEnv, rtl_uString * pEnvDcp, void * pContext )
    SAL_THROW_EXTERN_C()
{
    OSL_ENSURE( ppEnv, "### null ptr!" );
    if (*ppEnv)
        (*(*ppEnv)->release)( *ppEnv );

    OUString const & rEnvDcp = OUString::unacquired( &pEnvDcp );
    *ppEnv = initDefaultEnvironment( rEnvDcp, pContext );
}

//##############################################################################
void SAL_CALL uno_direct_getEnvironment(
    uno_Environment ** ppEnv, rtl_uString * pEnvDcp, void * pContext )
    SAL_THROW_EXTERN_C()
{
    OSL_ENSURE( ppEnv, "### null ptr!" );
    OUString const & rEnvDcp = OUString::unacquired( &pEnvDcp );

    EnvironmentsData & rData = theEnvironmentsData::get();

    ::osl::MutexGuard guard( rData.mutex );
    rData.getEnvironment( ppEnv, rEnvDcp, pContext );
    if (! *ppEnv)
    {
        *ppEnv = initDefaultEnvironment( rEnvDcp, pContext );
        if (*ppEnv)
        {
            // register new environment:
            rData.registerEnvironment( ppEnv );
        }
    }
}

//##############################################################################
void SAL_CALL uno_getRegisteredEnvironments(
    uno_Environment *** pppEnvs, sal_Int32 * pnLen, uno_memAlloc memAlloc,
    rtl_uString * pEnvDcp )
    SAL_THROW_EXTERN_C()
{
    EnvironmentsData & rData = theEnvironmentsData::get();

    ::osl::MutexGuard guard( rData.mutex );
    rData.getRegisteredEnvironments(
        pppEnvs, pnLen, memAlloc,
        (pEnvDcp ? OUString(pEnvDcp) : OUString()) );
}

} // extern "C"

}

