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

#include <cppu/EnvDcp.hxx>

#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <osl/interlck.h>
#include <osl/mutex.hxx>
#include <osl/module.hxx>
#include <osl/process.h>
#include <rtl/process.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/instance.hxx>
#include <typelib/typedescription.h>
#include <uno/dispatcher.h>
#include <uno/environment.h>
#include <uno/lbnames.h>
#include "prim.hxx"
#include "loadmodule.hxx"

#include <unordered_map>
#include <vector>
#include <stdio.h>


namespace
{


bool td_equals( typelib_InterfaceTypeDescription const * pTD1,
                       typelib_InterfaceTypeDescription const * pTD2 )
{
    return (pTD1 == pTD2 ||
            (pTD1->aBase.pTypeName->length == pTD2->aBase.pTypeName->length &&
             ::rtl_ustr_compare(
                 pTD1->aBase.pTypeName->buffer,
                 pTD2->aBase.pTypeName->buffer ) == 0));
}

struct uno_DefaultEnvironment;


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
    std::vector< InterfaceEntry > aInterfaces;
    bool mixedObject;

    explicit ObjectEntry( const OUString & rOId_ );

    void append(
        uno_DefaultEnvironment * pEnv,
        void * pInterface, typelib_InterfaceTypeDescription * pTypeDescr,
        uno_freeProxyFunc fpFreeProxy );
    InterfaceEntry * find(
        typelib_InterfaceTypeDescription * pTypeDescr );
    sal_Int32 find( void const * iface_ptr, std::size_t pos ) const;
};


struct FctPtrHash
{
    std::size_t operator () ( const void * pKey ) const
        { return reinterpret_cast< std::size_t>( pKey ); }
};


// mapping from environment name to environment
typedef std::unordered_map<
    OUString, uno_Environment * > OUString2EnvironmentMap;

// mapping from ptr to object entry
typedef std::unordered_map<
    void *, ObjectEntry *, FctPtrHash > Ptr2ObjectMap;
// mapping from oid to object entry
typedef std::unordered_map<
    OUString, ObjectEntry * > OId2ObjectMap;

struct EnvironmentsData
{
    ::osl::Mutex mutex;
    OUString2EnvironmentMap aName2EnvMap;

    EnvironmentsData() : isDisposing(false) {}
    ~EnvironmentsData();

    void getEnvironment(
        uno_Environment ** ppEnv, const OUString & rEnvDcp, void * pContext );
    void registerEnvironment( uno_Environment ** ppEnv );
    void getRegisteredEnvironments(
        uno_Environment *** pppEnvs, sal_Int32 * pnLen,
        uno_memAlloc memAlloc, const OUString & rEnvDcp );

    bool isDisposing;
};

struct theEnvironmentsData : public rtl::Static< EnvironmentsData, theEnvironmentsData > {};

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


ObjectEntry::ObjectEntry( OUString const & rOId_ )
    : oid( rOId_ ),
      nRef( 0 ),
      mixedObject( false )
{
    aInterfaces.reserve( 2 );
}


void ObjectEntry::append(
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
    typelib_typedescription_acquire( &pTypeDescr->aBase );
    aNewEntry.pTypeDescr = pTypeDescr;

    std::pair< Ptr2ObjectMap::iterator, bool > i(
        pEnv->aPtr2ObjectMap.emplace( pInterface, this ) );
    SAL_WARN_IF(
        !i.second && (find(pInterface, 0) == -1 || i.first->second != this),
        "cppu",
        "map already contains " << i.first->second << " != " << this << " for "
            << pInterface);
    aInterfaces.push_back( aNewEntry );
}


InterfaceEntry * ObjectEntry::find(
    typelib_InterfaceTypeDescription * pTypeDescr_ )
{
    OSL_ASSERT( ! aInterfaces.empty() );
    if (aInterfaces.empty())
        return nullptr;

    // shortcut common case:
    OUString const & type_name =
        OUString::unacquired( &pTypeDescr_->aBase.pTypeName );
    if ( type_name == "com.sun.star.uno.XInterface" )
    {
        return &aInterfaces[ 0 ];
    }

    std::size_t nSize = aInterfaces.size();
    for ( std::size_t nPos = 0; nPos < nSize; ++nPos )
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
    return nullptr;
}


sal_Int32 ObjectEntry::find(
    void const * iface_ptr, std::size_t pos ) const
{
    std::size_t size = aInterfaces.size();
    for ( ; pos < size; ++pos )
    {
        if (aInterfaces[ pos ].pInterface == iface_ptr)
            return pos;
    }
    return -1;
}

extern "C"
{


static void defenv_registerInterface(
    uno_ExtEnvironment * pEnv, void ** ppInterface,
    rtl_uString * pOId, typelib_InterfaceTypeDescription * pTypeDescr )
{
    OSL_ENSURE( pEnv && ppInterface && pOId && pTypeDescr, "### null ptr!" );
    OUString const & rOId = OUString::unacquired( &pOId );

    uno_DefaultEnvironment * that =
        static_cast< uno_DefaultEnvironment * >( pEnv );
    ::osl::ClearableMutexGuard guard( that->mutex );

    // try to insert dummy 0:
    std::pair<OId2ObjectMap::iterator, bool> const insertion(
        that->aOId2ObjectMap.emplace(  rOId, nullptr ) );
    if (insertion.second)
    {
        ObjectEntry * pOEntry = new ObjectEntry( rOId );
        insertion.first->second = pOEntry;
        ++pOEntry->nRef; // another register call on object
        pOEntry->append( that, *ppInterface, pTypeDescr, nullptr );
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
            pOEntry->append( that, *ppInterface, pTypeDescr, nullptr );
        }
    }
}


static void defenv_registerProxyInterface(
    uno_ExtEnvironment * pEnv, void ** ppInterface, uno_freeProxyFunc freeProxy,
    rtl_uString * pOId, typelib_InterfaceTypeDescription * pTypeDescr )
{
    OSL_ENSURE( pEnv && ppInterface && pOId && pTypeDescr && freeProxy,
                "### null ptr!" );
    OUString const & rOId = OUString::unacquired( &pOId );

    uno_DefaultEnvironment * that =
        static_cast< uno_DefaultEnvironment * >( pEnv );
    ::osl::ClearableMutexGuard guard( that->mutex );

    // try to insert dummy 0:
    std::pair<OId2ObjectMap::iterator, bool> const insertion(
        that->aOId2ObjectMap.emplace(  rOId, nullptr ) );
    if (insertion.second)
    {
        ObjectEntry * pOEntry = new ObjectEntry( rOId );
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
             pOEntry->aInterfaces[ 0 ].fpFreeProxy == nullptr);

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


static void s_stub_defenv_revokeInterface(va_list * pParam)
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
            typelib_typedescription_release( &rEntry.pTypeDescr->aBase );
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
                    pOEntry->aInterfaces[ 0 ].fpFreeProxy == nullptr );

        sal_Int32 index = pOEntry->find( pInterface, 1 );
        OSL_ASSERT( index > 0 );
        if (index > 0)
        {
            InterfaceEntry & entry = pOEntry->aInterfaces[ index ];
            OSL_ASSERT( entry.pInterface == pInterface );
            if (entry.fpFreeProxy != nullptr)
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
                        std::size_t erased =
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

static void defenv_revokeInterface(uno_ExtEnvironment * pEnv, void * pInterface)
{
    uno_Environment_invoke(&pEnv->aBase, s_stub_defenv_revokeInterface, pEnv, pInterface);
}


static void defenv_getObjectIdentifier(
    uno_ExtEnvironment * pEnv, rtl_uString ** ppOId, void * pInterface )
{
    OSL_ENSURE( pEnv && ppOId && pInterface, "### null ptr!" );
    if (*ppOId)
    {
        ::rtl_uString_release( *ppOId );
        *ppOId = nullptr;
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


static void defenv_getRegisteredInterface(
    uno_ExtEnvironment * pEnv, void ** ppInterface,
    rtl_uString * pOId, typelib_InterfaceTypeDescription * pTypeDescr )
{
    OSL_ENSURE( pEnv && ppInterface && pOId && pTypeDescr, "### null ptr!" );
    if (*ppInterface)
    {
        (*pEnv->releaseInterface)( pEnv, *ppInterface );
        *ppInterface = nullptr;
    }

    OUString const & rOId = OUString::unacquired( &pOId );
    uno_DefaultEnvironment * that =
        static_cast< uno_DefaultEnvironment * >( pEnv );
    ::osl::MutexGuard guard( that->mutex );

    OId2ObjectMap::const_iterator const iFind
        ( that->aOId2ObjectMap.find( rOId ) );
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


static void defenv_getRegisteredInterfaces(
    uno_ExtEnvironment * pEnv, void *** pppInterfaces, sal_Int32 * pnLen,
    uno_memAlloc memAlloc )
{
    assert(pEnv && pppInterfaces && pnLen && memAlloc && "### null ptr!");
    uno_DefaultEnvironment * that =
        static_cast< uno_DefaultEnvironment * >( pEnv );
    ::osl::MutexGuard guard( that->mutex );

    sal_Int32 nLen = that->aPtr2ObjectMap.size();
    sal_Int32 nPos = 0;
    void ** ppInterfaces = static_cast<void **>((*memAlloc)( nLen * sizeof (void *) ));

    for (const auto& rEntry : that->aPtr2ObjectMap)
    {
        (*pEnv->acquireInterface)( pEnv, ppInterfaces[nPos++] = rEntry.first );
    }

    *pppInterfaces = ppInterfaces;
    *pnLen = nLen;
}


static void defenv_acquire( uno_Environment * pEnv )
{
    uno_DefaultEnvironment * that = reinterpret_cast<uno_DefaultEnvironment *>(pEnv);
    osl_atomic_increment( &that->nWeakRef );
    osl_atomic_increment( &that->nRef );
}


static void defenv_release( uno_Environment * pEnv )
{
    uno_DefaultEnvironment * that = reinterpret_cast<uno_DefaultEnvironment *>(pEnv);
    if (! osl_atomic_decrement( &that->nRef ))
    {
        // invoke dispose callback
        if (pEnv->environmentDisposing)
        {
            (*pEnv->environmentDisposing)( pEnv );
        }

        OSL_ENSURE( that->aOId2ObjectMap.empty(), "### object entries left!" );
    }
    // free memory if no weak refs left
    if (! osl_atomic_decrement( &that->nWeakRef ))
    {
        delete that;
    }
}


static void defenv_acquireWeak( uno_Environment * pEnv )
{
    uno_DefaultEnvironment * that = reinterpret_cast<uno_DefaultEnvironment *>(pEnv);
    osl_atomic_increment( &that->nWeakRef );
}


static void defenv_releaseWeak( uno_Environment * pEnv )
{
    uno_DefaultEnvironment * that = reinterpret_cast<uno_DefaultEnvironment *>(pEnv);
    if (! osl_atomic_decrement( &that->nWeakRef ))
    {
        delete that;
    }
}


static void defenv_harden(
    uno_Environment ** ppHardEnv, uno_Environment * pEnv )
{
    if (*ppHardEnv)
    {
        (*(*ppHardEnv)->release)( *ppHardEnv );
        *ppHardEnv = nullptr;
    }

    EnvironmentsData & rData = theEnvironmentsData::get();

    if (rData.isDisposing)
        return;

    uno_DefaultEnvironment * that = reinterpret_cast<uno_DefaultEnvironment *>(pEnv);
    {
    ::osl::MutexGuard guard( rData.mutex );
    if (1 == osl_atomic_increment( &that->nRef )) // is dead
    {
        that->nRef = 0;
        return;
    }
    }
    osl_atomic_increment( &that->nWeakRef );
    *ppHardEnv = pEnv;
}


static void defenv_dispose( SAL_UNUSED_PARAMETER uno_Environment * )
{
}
}


uno_DefaultEnvironment::uno_DefaultEnvironment(
    const OUString & rEnvDcp_, void * pContext_ )
    : nRef( 0 ),
      nWeakRef( 0 )
{
    uno_Environment * that = reinterpret_cast< uno_Environment * >(this);
    that->pReserved = nullptr;
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
    that->environmentDisposing = nullptr;

    uno_ExtEnvironment::registerInterface = defenv_registerInterface;
    uno_ExtEnvironment::registerProxyInterface = defenv_registerProxyInterface;
    uno_ExtEnvironment::revokeInterface = defenv_revokeInterface;
    uno_ExtEnvironment::getObjectIdentifier = defenv_getObjectIdentifier;
    uno_ExtEnvironment::getRegisteredInterface = defenv_getRegisteredInterface;
    uno_ExtEnvironment::getRegisteredInterfaces =
        defenv_getRegisteredInterfaces;

}


uno_DefaultEnvironment::~uno_DefaultEnvironment()
{
    ::rtl_uString_release( aBase.pTypeName );
}


void writeLine(
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
                        fprintf( static_cast<FILE *>(stream), "%s\n", pLine );
                    }
                    else
                    {
                        SAL_WARN("cppu", pLine );
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
            fprintf( static_cast<FILE *>(stream), "%s\n", pLine );
        }
        else
        {
            fprintf( stderr, "%s\n", pLine );
        }
    }
}


void writeLine(
    void * stream, const OUString & rLine, const sal_Char * pFilter )
{
    OString aLine( OUStringToOString(
                              rLine, RTL_TEXTENCODING_ASCII_US ) );
    writeLine( stream, aLine.getStr(), pFilter );
}

}

extern "C" void SAL_CALL uno_dumpEnvironment(
    void * stream, uno_Environment * pEnv, const sal_Char * pFilter )
    SAL_THROW_EXTERN_C()
{
    OSL_ENSURE( pEnv, "### null ptr!" );
    OUStringBuffer buf;

    if (! pEnv->pExtEnv)
    {
        writeLine( stream, "###################################"
                   "###########################################", pFilter );
        buf.append( "environment: " );
        buf.append( pEnv->pTypeName );
        writeLine( stream, buf.makeStringAndClear(), pFilter );
        writeLine( stream, "NO INTERFACE INFORMATION AVAILABLE!", pFilter );
        return;
    }

    writeLine( stream, "########################################"
               "######################################", pFilter );
    buf.append( "environment dump: " );
    buf.append( pEnv->pTypeName );
    writeLine( stream, buf.makeStringAndClear(), pFilter );

    uno_DefaultEnvironment * that =
        reinterpret_cast< uno_DefaultEnvironment * >(pEnv);
    ::osl::MutexGuard guard( that->mutex );

    Ptr2ObjectMap ptr2obj( that->aPtr2ObjectMap );
    for (const auto& rEntry : that->aOId2ObjectMap)
    {
        ObjectEntry * pOEntry = rEntry.second;

        buf.append( "+ " );
        if (pOEntry->mixedObject)
            buf.append( "mixed " );
        buf.append( "object entry: nRef=" );
        buf.append( pOEntry->nRef );
        buf.append( "; oid=\"" );
        buf.append( pOEntry->oid );
        buf.append( '\"' );
        writeLine( stream, buf.makeStringAndClear(), pFilter );

        for ( std::size_t nPos = 0;
              nPos < pOEntry->aInterfaces.size(); ++nPos )
        {
            const InterfaceEntry & rIEntry = pOEntry->aInterfaces[nPos];

            buf.append( "  - " );
            buf.append( rIEntry.pTypeDescr->aBase.pTypeName );
            if (rIEntry.fpFreeProxy)
            {
                buf.append( "; proxy free=0x" );
                buf.append(
                    reinterpret_cast< sal_IntPtr >(rIEntry.fpFreeProxy), 16 );
            }
            else
            {
                buf.append( "; original" );
            }
            buf.append( "; ptr=0x" );
            buf.append(
                reinterpret_cast< sal_IntPtr >(rIEntry.pInterface), 16 );

            if (pOEntry->find( rIEntry.pInterface, nPos + 1 ) < 0)
            {
                std::size_t erased = ptr2obj.erase( rIEntry.pInterface );
                if (erased != 1)
                {
                    buf.append( " (ptr not found in map!)" );
                }
            }
            writeLine( stream, buf.makeStringAndClear(), pFilter );
        }
    }
    if (! ptr2obj.empty())
        writeLine( stream, "ptr map inconsistency!!!", pFilter );
    writeLine( stream, "#####################################"
               "#########################################", pFilter );
}


extern "C" void SAL_CALL uno_dumpEnvironmentByName(
    void * stream, rtl_uString * pEnvDcp, const sal_Char * pFilter )
    SAL_THROW_EXTERN_C()
{
    uno_Environment * pEnv = nullptr;
    uno_getEnvironment( &pEnv, pEnvDcp, nullptr );
    if (pEnv)
    {
        ::uno_dumpEnvironment( stream, pEnv, pFilter );
        (*pEnv->release)( pEnv );
    }
    else
    {
        OUStringBuffer buf( 32 );
        buf.append( "environment \"" );
        buf.append( pEnvDcp );
        buf.append( "\" does not exist!" );
        writeLine( stream, buf.makeStringAndClear(), pFilter );
    }
}

namespace
{
    class makeOIdPart
    {
    private:
        OUString m_sOidPart;
    public:
        makeOIdPart()
        {
            OUStringBuffer aRet( 64 );
            aRet.append( "];" );
            // pid
            oslProcessInfo info;
            info.Size = sizeof(oslProcessInfo);
            if (::osl_getProcessInfo( nullptr, osl_Process_IDENTIFIER, &info ) ==
                osl_Process_E_None)
            {
                aRet.append( static_cast<sal_Int64>(info.Ident), 16 );
            }
            else
            {
                aRet.append( "unknown process id" );
            }
            // good guid
            sal_uInt8 ar[16];
            ::rtl_getGlobalProcessId( ar );
            aRet.append( ';' );
            for (unsigned char i : ar)
                aRet.append( static_cast<sal_Int32>(i), 16 );

            m_sOidPart = aRet.makeStringAndClear();
        }
        const OUString& getOIdPart() const { return m_sOidPart; }
    };

    class theStaticOIdPart : public rtl::Static<makeOIdPart, theStaticOIdPart> {};

const OUString & unoenv_getStaticOIdPart()
{
    return theStaticOIdPart::get().getOIdPart();
}

}

extern "C"
{


static void unoenv_computeObjectIdentifier(
    uno_ExtEnvironment * pEnv, rtl_uString ** ppOId, void * pInterface )
{
    assert(pEnv && ppOId && pInterface && "### null ptr!");
    if (*ppOId)
    {
        ::rtl_uString_release( *ppOId );
        *ppOId = nullptr;
    }

    uno_Interface * pUnoI = static_cast<uno_Interface *>(
        ::cppu::binuno_queryInterface(
            pInterface, *typelib_static_type_getByTypeClass(
                typelib_TypeClass_INTERFACE ) ));
    if (nullptr != pUnoI)
    {
        (*pUnoI->release)( pUnoI );
        // interface
        OUStringBuffer oid( 64 );
        oid.append( reinterpret_cast< sal_Int64 >(pUnoI), 16 );
        oid.append( ';' );
        // environment[context]
        oid.append( pEnv->aBase.pTypeName );
        oid.append( '[' );
        oid.append( reinterpret_cast< sal_Int64 >(
                        reinterpret_cast<
                        uno_Environment * >(pEnv)->pContext ), 16 );
        // process;good guid
        oid.append( unoenv_getStaticOIdPart() );
        OUString aStr( oid.makeStringAndClear() );
        ::rtl_uString_acquire( *ppOId = aStr.pData );
    }
}


static void unoenv_acquireInterface(
    SAL_UNUSED_PARAMETER uno_ExtEnvironment *, void * pUnoI_ )
{
    uno_Interface * pUnoI = static_cast< uno_Interface * >(pUnoI_);
    (*pUnoI->acquire)( pUnoI );
}


static void unoenv_releaseInterface(
    SAL_UNUSED_PARAMETER uno_ExtEnvironment *, void * pUnoI_ )
{
    uno_Interface * pUnoI = static_cast< uno_Interface * >(pUnoI_);
    (*pUnoI->release)( pUnoI );
}
}

namespace {

EnvironmentsData::~EnvironmentsData()
{
    ::osl::MutexGuard guard( mutex );
    isDisposing = true;

    for ( const auto& rEntry : aName2EnvMap )
    {
        uno_Environment * pWeak = rEntry.second;
        uno_Environment * pHard = nullptr;
        (*pWeak->harden)( &pHard, pWeak );
        (*pWeak->releaseWeak)( pWeak );

        if (pHard)
        {
            (*pHard->dispose)( pHard ); // send explicit dispose
            (*pHard->release)( pHard );
        }
    }
}


void EnvironmentsData::getEnvironment(
    uno_Environment ** ppEnv, const OUString & rEnvDcp, void * pContext )
{
    if (*ppEnv)
    {
        (*(*ppEnv)->release)( *ppEnv );
        *ppEnv = nullptr;
    }

    OUString aKey = OUString::number( reinterpret_cast< sal_IntPtr >(pContext) ) + rEnvDcp;

    // try to find registered mapping
    OUString2EnvironmentMap::const_iterator const iFind(
        aName2EnvMap.find( aKey ) );
    if (iFind != aName2EnvMap.end())
    {
        uno_Environment * pWeak = iFind->second;
        (*pWeak->harden)( ppEnv, pWeak );
    }
}


void EnvironmentsData::registerEnvironment( uno_Environment ** ppEnv )
{
    OSL_ENSURE( ppEnv, "### null ptr!" );
    uno_Environment * pEnv =  *ppEnv;

    OUString aKey(
        OUString::number( reinterpret_cast< sal_IntPtr >(pEnv->pContext) ) );
    aKey += pEnv->pTypeName;

    // try to find registered environment
    OUString2EnvironmentMap::const_iterator const iFind(
        aName2EnvMap.find( aKey ) );
    if (iFind == aName2EnvMap.end())
    {
        (*pEnv->acquireWeak)( pEnv );
        std::pair< OUString2EnvironmentMap::iterator, bool > insertion (
            aName2EnvMap.emplace( aKey, pEnv ) );
        SAL_WARN_IF( !insertion.second, "cppu", "key " << aKey << " already in env map" );
    }
    else
    {
        uno_Environment * pHard = nullptr;
        uno_Environment * pWeak = iFind->second;
        (*pWeak->harden)( &pHard, pWeak );
        if (pHard)
        {
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


void EnvironmentsData::getRegisteredEnvironments(
    uno_Environment *** pppEnvs, sal_Int32 * pnLen, uno_memAlloc memAlloc,
    const OUString & rEnvDcp )
{
    assert(pppEnvs && pnLen && memAlloc && "### null ptr!");

    // max size
    std::vector<uno_Environment*> aFounds(aName2EnvMap.size());
    sal_Int32 nSize = 0;

    // find matching environment
    for ( const auto& rEntry : aName2EnvMap )
    {
        uno_Environment * pWeak = rEntry.second;
        if (rEnvDcp.isEmpty() ||
            rEnvDcp == pWeak->pTypeName )
        {
            aFounds[nSize] = nullptr;
            (*pWeak->harden)( &aFounds[nSize], pWeak );
            if (aFounds[nSize])
                ++nSize;
        }
    }

    *pnLen = nSize;
    if (nSize)
    {
        *pppEnvs = static_cast<uno_Environment **>((*memAlloc)(
            sizeof (uno_Environment *) * nSize ));
        OSL_ASSERT( *pppEnvs );
        while (nSize--)
        {
            (*pppEnvs)[nSize] = aFounds[nSize];
        }
    }
    else
    {
        *pppEnvs = nullptr;
    }
}

bool loadEnv(OUString const  & cLibStem,
                    uno_Environment * pEnv)
{
#ifdef DISABLE_DYNLOADING
    uno_initEnvironmentFunc fpInit;

    if ( cLibStem == CPPU_CURRENT_LANGUAGE_BINDING_NAME "_uno" )
        fpInit = CPPU_ENV_uno_initEnvironment;
#if HAVE_FEATURE_JAVA
    else if ( cLibStem == "java_uno" )
        fpInit = java_uno_initEnvironment;
#endif
    else
    {
        SAL_INFO("cppu", ": Unhandled env: " << cLibStem);
        return false;
    }
#else
    // late init with some code from matching uno language binding
    // will be unloaded by environment
    osl::Module aMod;
    try {
        bool bMod = cppu::detail::loadModule(aMod, cLibStem);
        if (!bMod)
            return false;
    }
    catch(...) {
        // Catch everything and convert to return false
        return false;
    }


    uno_initEnvironmentFunc fpInit = reinterpret_cast<uno_initEnvironmentFunc>(aMod.getSymbol(UNO_INIT_ENVIRONMENT));

    if (!fpInit)
        return false;

    aMod.release();
#endif

    (*fpInit)( pEnv ); // init of environment
    return true;
}

}

extern "C"
{


static uno_Environment * initDefaultEnvironment(
    const OUString & rEnvDcp, void * pContext )
{
    uno_Environment * pEnv = &(new uno_DefaultEnvironment( rEnvDcp, pContext ))->aBase;
    (*pEnv->acquire)( pEnv );

    OUString envTypeName = cppu::EnvDcp::getTypeName(rEnvDcp);

    // create default environment
    if ( envTypeName == UNO_LB_UNO )
    {
        uno_DefaultEnvironment * that = reinterpret_cast<uno_DefaultEnvironment *>(pEnv);
        that->computeObjectIdentifier = unoenv_computeObjectIdentifier;
        that->acquireInterface = unoenv_acquireInterface;
        that->releaseInterface = unoenv_releaseInterface;

        OUString envPurpose = cppu::EnvDcp::getPurpose(rEnvDcp);
        if (!envPurpose.isEmpty())
        {
            OUString libStem(
                envPurpose.copy(envPurpose.lastIndexOf(':') + 1) + "_uno_uno");
            if(!loadEnv(libStem, pEnv))
            {
                pEnv->release(pEnv);
                return nullptr;
            }
        }
    }
    else
    {
        // late init with some code from matching uno language binding
        OUString aStr( envTypeName + "_uno" );

        if (!loadEnv(aStr, pEnv))
        {
            pEnv->release(pEnv);
            return nullptr;
        }
    }

    return pEnv;
}


void SAL_CALL uno_createEnvironment(
    uno_Environment ** ppEnv, rtl_uString * pEnvDcp, void * pContext )
    SAL_THROW_EXTERN_C()
{
    assert(ppEnv && "### null ptr!");
    if (*ppEnv)
        (*(*ppEnv)->release)( *ppEnv );

    OUString const & rEnvDcp = OUString::unacquired( &pEnvDcp );
    *ppEnv = initDefaultEnvironment( rEnvDcp, pContext );
}

void SAL_CALL uno_getEnvironment(
    uno_Environment ** ppEnv, rtl_uString * pEnvDcp, void * pContext )
    SAL_THROW_EXTERN_C()
{
    assert(ppEnv && "### null ptr!");
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
