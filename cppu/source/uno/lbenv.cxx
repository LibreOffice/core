/*************************************************************************
 *
 *  $RCSfile: lbenv.cxx,v $
 *
 *  $Revision: 1.27 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 16:37:42 $
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

#include <stdio.h>
#ifdef SOLARIS
#include <alloca.h>
#elif defined MACOSX
#include <sys/malloc.h>
#else
#include <malloc.h>
#endif
#include <hash_map>
#include <vector>

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

#include "typelib/typedescription.h"
#include "uno/dispatcher.h"
#include "uno/environment.h"

#include "uno/lbnames.h"

#include "prim.hxx"
#include "destr.hxx"


using namespace ::std;
using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;


namespace cppu
{

//--------------------------------------------------------------------------------------------------
inline static sal_Bool td_equals( typelib_InterfaceTypeDescription * pTD1,
                                  typelib_InterfaceTypeDescription * pTD2 )
    SAL_THROW( () )
{
    return (pTD1 == pTD2 ||
            (((typelib_TypeDescription *)pTD1)->pTypeName->length ==
             ((typelib_TypeDescription *)pTD2)->pTypeName->length &&
             ::rtl_ustr_compare( ((typelib_TypeDescription *)pTD1)->pTypeName->buffer,
                                 ((typelib_TypeDescription *)pTD2)->pTypeName->buffer ) == 0));
}

struct ObjectEntry;
//--------------------------------------------------------------------------------------------------
struct InterfaceEntry
{
    void *              pInterface;
    uno_freeProxyFunc   fpFreeProxy;
    typelib_InterfaceTypeDescription * pTypeDescr;
    ObjectEntry *       pOEntry;

    inline bool supports( typelib_InterfaceTypeDescription * pTypeDescr_ ) const
        SAL_THROW( () );
};
//--------------------------------------------------------------------------------------------------
struct ObjectEntry
{
    uno_ExtEnvironment *        pEnv;
    OUString                    oid;
    sal_Int32                   nRef;
    vector< InterfaceEntry >    aInterfaces;

    inline ObjectEntry( uno_ExtEnvironment * pEnv, const OUString & rOId_ ) SAL_THROW( () );

    inline void append(
        void * pInterface, typelib_InterfaceTypeDescription * pTypeDescr,
        uno_freeProxyFunc fpFreeProxy )
        SAL_THROW( () );
    inline const InterfaceEntry * find(
        typelib_InterfaceTypeDescription * pTypeDescr ) const
        SAL_THROW( () );
};

//--------------------------------------------------------------------------------------------------
struct FctPtrHash : public unary_function< const void *, size_t >
{
    size_t operator () ( const void * pKey ) const SAL_THROW( () )
        { return (size_t)pKey; }
};
//--------------------------------------------------------------------------------------------------
struct FctOUStringHash : public unary_function< const OUString &, size_t >
{
    size_t operator () ( const OUString & rKey ) const SAL_THROW( () )
        { return rKey.hashCode(); }
};

// mapping from environment name to environment
typedef hash_map<
    OUString, uno_Environment *, FctOUStringHash, equal_to< OUString > > OUString2EnvironmentMap;

// mapping from ptr to object entry
typedef hash_map<
    void *, ObjectEntry *, FctPtrHash, equal_to< void * > > Ptr2ObjectMap;
// mapping from oid to object entry
typedef hash_map<
    OUString, ObjectEntry *, FctOUStringHash, equal_to< OUString > > OId2ObjectMap;

//==================================================================================================
struct EnvironmentsData
{
    Mutex                   aMutex;
    OUString2EnvironmentMap aName2EnvMap;

    ~EnvironmentsData() SAL_THROW( () );

    inline void getEnvironment(
        uno_Environment ** ppEnv, const OUString & rTypeName, void * pContext )
        SAL_THROW( () );
    inline void registerEnvironment( uno_Environment ** ppEnv )
        SAL_THROW( () );
    inline void getRegisteredEnvironments(
        uno_Environment *** pppEnvs, sal_Int32 * pnLen, uno_memAlloc memAlloc, const OUString & rEnvTypeName )
        SAL_THROW( () );
};
//--------------------------------------------------------------------------------------------------
static EnvironmentsData & getEnvironmentsData() SAL_THROW( () )
{
    static EnvironmentsData * s_p = 0;
    if (! s_p)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_p)
        {
            static EnvironmentsData s_obj;
            s_p = &s_obj;
        }
    }
    return *s_p;
}

//==================================================================================================
struct uno_DefaultEnvironment : public uno_ExtEnvironment
{
    sal_Int32           nRef;
    sal_Int32           nWeakRef;

    Mutex               aAccess;
    Ptr2ObjectMap       aPtr2ObjectMap;
    OId2ObjectMap       aOId2ObjectMap;

    uno_DefaultEnvironment(
        const OUString & rTypeName_, void * pContext_ )
        SAL_THROW( () );
    ~uno_DefaultEnvironment()
        SAL_THROW( () );
};

//__________________________________________________________________________________________________
inline ObjectEntry::ObjectEntry( uno_ExtEnvironment * pEnv_, const OUString & rOId_ )
    SAL_THROW( () )
    : pEnv( pEnv_ )
    , oid( rOId_ )
    , nRef( 0 )
{
    aInterfaces.reserve( 8 );
}
//__________________________________________________________________________________________________
inline void ObjectEntry::append(
    void * pInterface, typelib_InterfaceTypeDescription * pTypeDescr,
    uno_freeProxyFunc fpFreeProxy )
    SAL_THROW( () )
{
    InterfaceEntry aNewEntry;
    if (! fpFreeProxy)
    {
        (*pEnv->acquireInterface)( pEnv, pInterface );
    }
    aNewEntry.pInterface    = pInterface;
    aNewEntry.fpFreeProxy   = fpFreeProxy;
    aNewEntry.pOEntry       = this;
    typelib_typedescription_acquire( (typelib_TypeDescription *)(aNewEntry.pTypeDescr = pTypeDescr) );

    aInterfaces.push_back( aNewEntry );
    static_cast< uno_DefaultEnvironment * >( pEnv )->aPtr2ObjectMap[ pInterface ] = this;
}
//__________________________________________________________________________________________________
inline bool InterfaceEntry::supports(
    typelib_InterfaceTypeDescription * pTypeDescr_ ) const
    SAL_THROW( () )
{
    typelib_InterfaceTypeDescription * pITD = pTypeDescr;
    while (pITD)
    {
        if (td_equals( pITD, pTypeDescr_ ))
            return true;
        pITD = pITD->pBaseTypeDescription;
    }
    return false;
}
//__________________________________________________________________________________________________
inline const InterfaceEntry * ObjectEntry::find(
    typelib_InterfaceTypeDescription * pTypeDescr ) const
    SAL_THROW( () )
{
    OSL_ASSERT( ! aInterfaces.empty() );

    // shortcut common case
    OUString const & type_name =
        * reinterpret_cast< OUString const * >(
            &((typelib_TypeDescription *) pTypeDescr)->pTypeName );
    if (type_name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.uno.XInterface") ))
    {
        return &aInterfaces[ 0 ];
    }

    size_t nSize = aInterfaces.size();
    for ( size_t nPos = 0; nPos < nSize; ++nPos )
    {
        OSL_ASSERT( aInterfaces[nPos].pOEntry == this );
        if (aInterfaces[ nPos ].supports( pTypeDescr ))
            return &aInterfaces[nPos];
    }
    return 0;
}
extern "C"
{
//--------------------------------------------------------------------------------------------------
static void SAL_CALL defenv_registerInterface(
    uno_ExtEnvironment * pEnv, void ** ppInterface,
    rtl_uString * pOId, typelib_InterfaceTypeDescription * pTypeDescr )
    SAL_THROW( () )
{
    OSL_ENSURE( pEnv && ppInterface && pOId && pTypeDescr, "### null ptr!" );
    OUString const & rOId = * reinterpret_cast< OUString * >( &pOId );

    uno_DefaultEnvironment * that = static_cast< uno_DefaultEnvironment * >( pEnv );
    ClearableMutexGuard aGuard( that->aAccess );

    OId2ObjectMap::const_iterator const iFind( that->aOId2ObjectMap.find( rOId ) );
    if (iFind == that->aOId2ObjectMap.end())
    {
        ObjectEntry * pOEntry = new ObjectEntry( pEnv, rOId );
        pair< OId2ObjectMap::iterator, bool > insertion(
            that->aOId2ObjectMap.insert( OId2ObjectMap::value_type( rOId, pOEntry ) ) );
        OSL_ENSURE( insertion.second, "### inserting new object entry failed?!" );
        ++pOEntry->nRef; // another register call on object
        pOEntry->append( *ppInterface, pTypeDescr, 0 );
    }
    else // object entry exists
    {
        ObjectEntry * pOEntry = iFind->second;
        ++pOEntry->nRef; // another register call on object
        InterfaceEntry const * pIEntry = pOEntry->find( pTypeDescr );

        if (pIEntry) // type entry exists
        {
            if (pIEntry->pInterface != *ppInterface)
            {
                void * pInterface = pIEntry->pInterface;
                (*pEnv->acquireInterface)( pEnv, pInterface );
                aGuard.clear();
                (*pEnv->releaseInterface)( pEnv, *ppInterface );
                *ppInterface = pInterface;
            }
        }
        else
        {
            pOEntry->append( *ppInterface, pTypeDescr, 0 );
        }
    }
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL defenv_registerProxyInterface(
    uno_ExtEnvironment * pEnv, void ** ppInterface, uno_freeProxyFunc freeProxy,
    rtl_uString * pOId, typelib_InterfaceTypeDescription * pTypeDescr )
    SAL_THROW( () )
{
    OSL_ENSURE( pEnv && ppInterface && pOId && pTypeDescr, "### null ptr!" );
    OUString const & rOId = * reinterpret_cast< OUString * >( &pOId );

    uno_DefaultEnvironment * that = static_cast< uno_DefaultEnvironment * >( pEnv );
    ClearableMutexGuard aGuard( that->aAccess );

    OId2ObjectMap::const_iterator const iFind( that->aOId2ObjectMap.find( rOId ) );
    if (iFind == that->aOId2ObjectMap.end())
    {
        ObjectEntry * pOEntry = new ObjectEntry( pEnv, rOId );
        pair< OId2ObjectMap::iterator, bool > insertion(
            that->aOId2ObjectMap.insert( OId2ObjectMap::value_type( rOId, pOEntry ) ) );
        OSL_ENSURE( insertion.second, "### inserting new object entry failed?!" );
        ++pOEntry->nRef; // another register call on object
        pOEntry->append( *ppInterface, pTypeDescr, freeProxy );
    }
    else // object entry exists
    {
        ObjectEntry * pOEntry = iFind->second;
        ++pOEntry->nRef; // another register call on object
        InterfaceEntry const * pIEntry = pOEntry->find( pTypeDescr );

        if (pIEntry) // type entry exists
        {
            if (pIEntry->pInterface != *ppInterface)
            {
                void * pInterface = pIEntry->pInterface;
                (*pEnv->acquireInterface)( pEnv, pInterface );
                --pOEntry->nRef; // manual revoke of proxy to be freed
                aGuard.clear();
                (*freeProxy)( pEnv, *ppInterface );
                *ppInterface = pInterface;
            }
        }
        else
        {
            pOEntry->append( *ppInterface, pTypeDescr, freeProxy );
        }
    }
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL defenv_revokeInterface(
    uno_ExtEnvironment * pEnv, void * pInterface )
    SAL_THROW( () )
{
    OSL_ENSURE( pEnv && pInterface, "### null ptr!" );
    uno_DefaultEnvironment * that = static_cast< uno_DefaultEnvironment * >( pEnv );
    ClearableMutexGuard aGuard( that->aAccess );

    Ptr2ObjectMap::const_iterator const iFind( that->aPtr2ObjectMap.find( pInterface ) );
    OSL_ASSERT( iFind != that->aPtr2ObjectMap.end() );
    if (! --iFind->second->nRef)
    {
        ObjectEntry * pOEntry = iFind->second;
        OSL_ASSERT( pEnv == pOEntry->pEnv );
        // cleanup maps
        that->aOId2ObjectMap.erase( pOEntry->oid );
        sal_Int32 nPos;
        for ( nPos = pOEntry->aInterfaces.size(); nPos--; )
        {
            that->aPtr2ObjectMap.erase( pOEntry->aInterfaces[nPos].pInterface );
        }

        // the last proxy interface of the environment might kill this environment,
        // because of releasing its language binding!!!
        aGuard.clear();

        // release interfaces
        for ( nPos = pOEntry->aInterfaces.size(); nPos--; )
        {
            InterfaceEntry const & rEntry = pOEntry->aInterfaces[nPos];
            typelib_typedescription_release( (typelib_TypeDescription *)rEntry.pTypeDescr );
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
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL defenv_getObjectIdentifier(
    uno_ExtEnvironment * pEnv, rtl_uString ** ppOId, void * pInterface )
    SAL_THROW( () )
{
    OSL_ENSURE( pEnv && ppOId && pInterface, "### null ptr!" );
    if (*ppOId)
    {
        ::rtl_uString_release( *ppOId );
        *ppOId = 0;
    }

    uno_DefaultEnvironment * that = static_cast< uno_DefaultEnvironment * >( pEnv );
    ClearableMutexGuard aGuard( that->aAccess );

    Ptr2ObjectMap::const_iterator const iFind( that->aPtr2ObjectMap.find( pInterface ) );
    if (iFind == that->aPtr2ObjectMap.end())
    {
        aGuard.clear();
        (*pEnv->computeObjectIdentifier)( pEnv, ppOId, pInterface );
    }
    else
    {
        ::rtl_uString_acquire( *ppOId = iFind->second->oid.pData );
    }
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL defenv_getRegisteredInterface(
    uno_ExtEnvironment * pEnv, void ** ppInterface,
    rtl_uString * pOId, typelib_InterfaceTypeDescription * pTypeDescr )
    SAL_THROW( () )
{
    OSL_ENSURE( pEnv && ppInterface && pOId && pTypeDescr, "### null ptr!" );
    if (*ppInterface)
    {
        (*pEnv->releaseInterface)( pEnv, *ppInterface );
        *ppInterface = 0;
    }

    OUString const & rOId = * reinterpret_cast< OUString * >( &pOId );
    uno_DefaultEnvironment * that = static_cast< uno_DefaultEnvironment * >( pEnv );
    MutexGuard aGuard( that->aAccess );

    OId2ObjectMap::const_iterator const iFind( that->aOId2ObjectMap.find( rOId ) );
    if (iFind != that->aOId2ObjectMap.end())
    {
        InterfaceEntry const * pIEntry = iFind->second->find( pTypeDescr );
        if (pIEntry)
        {
            (*pEnv->acquireInterface)( pEnv, *ppInterface = pIEntry->pInterface );
        }
    }
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL defenv_getRegisteredInterfaces(
    uno_ExtEnvironment * pEnv, void *** pppInterfaces, sal_Int32 * pnLen, uno_memAlloc memAlloc )
    SAL_THROW( () )
{
    OSL_ENSURE( pEnv && pppInterfaces && pnLen && memAlloc, "### null ptr!" );
    uno_DefaultEnvironment * that = static_cast< uno_DefaultEnvironment * >( pEnv );
    MutexGuard aGuard( that->aAccess );

    sal_Int32 nLen = that->aPtr2ObjectMap.size();
    sal_Int32 nPos = 0;
    void ** ppInterfaces = (void **)(*memAlloc)( nLen * sizeof(void *) );

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
//--------------------------------------------------------------------------------------------------
static void SAL_CALL defenv_acquire( uno_Environment * pEnv )
    SAL_THROW( () )
{
    uno_DefaultEnvironment * that = (uno_DefaultEnvironment *)pEnv;
    ::osl_incrementInterlockedCount( &that->nWeakRef );
    ::osl_incrementInterlockedCount( &that->nRef );
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL defenv_release( uno_Environment * pEnv )
    SAL_THROW( () )
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
//--------------------------------------------------------------------------------------------------
static void SAL_CALL defenv_acquireWeak( uno_Environment * pEnv )
    SAL_THROW( () )
{
    uno_DefaultEnvironment * that = (uno_DefaultEnvironment *)pEnv;
    ::osl_incrementInterlockedCount( &that->nWeakRef );
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL defenv_releaseWeak( uno_Environment * pEnv )
    SAL_THROW( () )
{
    uno_DefaultEnvironment * that = (uno_DefaultEnvironment *)pEnv;
    if (! ::osl_decrementInterlockedCount( &that->nWeakRef ))
    {
        delete that;
    }
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL defenv_harden( uno_Environment ** ppHardEnv, uno_Environment * pEnv )
    SAL_THROW( () )
{
    if (*ppHardEnv)
    {
        (*(*ppHardEnv)->release)( *ppHardEnv );
        *ppHardEnv = 0;
    }

    uno_DefaultEnvironment * that = (uno_DefaultEnvironment *)pEnv;
    {
    MutexGuard aGuard( getEnvironmentsData().aMutex );
    if (1 == ::osl_incrementInterlockedCount( &that->nRef )) // is dead
    {
        that->nRef = 0;
        return;
    }
    }
    ::osl_incrementInterlockedCount( &that->nWeakRef );
    *ppHardEnv = pEnv;
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL defenv_dispose( uno_Environment * pEnv )
    SAL_THROW( () )
{
}
}

//__________________________________________________________________________________________________
uno_DefaultEnvironment::uno_DefaultEnvironment(
    const OUString & rTypeName_, void * pContext_ )
    SAL_THROW( () )
    : nRef( 0 )
    , nWeakRef( 0 )
{
    uno_Environment * that = (uno_Environment *)this;
    that->pReserved                 = 0;
    // functions
    that->acquire                   = defenv_acquire;
    that->release                   = defenv_release;
    that->acquireWeak               = defenv_acquireWeak;
    that->releaseWeak               = defenv_releaseWeak;
    that->harden                    = defenv_harden;
    that->dispose                   = defenv_dispose;
    that->pExtEnv                   = this;
    // identifier
    ::rtl_uString_acquire( rTypeName_.pData );
    that->pTypeName                 = rTypeName_.pData;
    that->pContext                  = pContext_;

    // will be late initialized
    that->environmentDisposing      = 0;

    uno_ExtEnvironment::registerInterface       = defenv_registerInterface;
    uno_ExtEnvironment::registerProxyInterface  = defenv_registerProxyInterface;
    uno_ExtEnvironment::revokeInterface         = defenv_revokeInterface;
    uno_ExtEnvironment::getObjectIdentifier     = defenv_getObjectIdentifier;
    uno_ExtEnvironment::getRegisteredInterface  = defenv_getRegisteredInterface;
    uno_ExtEnvironment::getRegisteredInterfaces = defenv_getRegisteredInterfaces;

}
//__________________________________________________________________________________________________
uno_DefaultEnvironment::~uno_DefaultEnvironment() SAL_THROW( () )
{
    ::rtl_uString_release( ((uno_Environment *)this)->pTypeName );
}

//==================================================================================================
static void writeLine( void * stream, const sal_Char * pLine, const sal_Char * pFilter )
    SAL_THROW( () )
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
//==================================================================================================
static void writeLine( void * stream, const OUString & rLine, const sal_Char * pFilter )
    SAL_THROW( () )
{
    OString aLine( OUStringToOString( rLine, RTL_TEXTENCODING_ASCII_US ) );
    writeLine( stream, aLine.getStr(), pFilter );
}

//##################################################################################################
extern "C" void SAL_CALL uno_dumpEnvironment(
    void * stream, uno_Environment * pEnv, const sal_Char * pFilter )
    SAL_THROW_EXTERN_C()
{
    OSL_ENSURE( pEnv, "### null ptr!" );
    OUStringBuffer buf;

    if (! pEnv->pExtEnv)
    {
        writeLine( stream, "##############################################################################", pFilter );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("environment: ") );
        buf.append( pEnv->pTypeName );
        writeLine( stream, buf.makeStringAndClear(), pFilter );
        writeLine( stream, "NO INTERFACE INFORMATION AVAILABLE!", pFilter );
        return;
    }

    writeLine( stream, "##############################################################################", pFilter );
    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("environment dump: ") );
    buf.append( pEnv->pTypeName );
    writeLine( stream, buf.makeStringAndClear(), pFilter );

    MutexGuard aGuard( ((uno_DefaultEnvironment *)pEnv)->aAccess );

    OId2ObjectMap::const_iterator iPos( ((uno_DefaultEnvironment *)pEnv)->aOId2ObjectMap.begin() );
    while (iPos != ((uno_DefaultEnvironment *)pEnv)->aOId2ObjectMap.end())
    {
        ObjectEntry * pOEntry = iPos->second;
        OSL_ASSERT( (uno_DefaultEnvironment *)pEnv == pOEntry->pEnv );

        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("+ object entry: nRef=") );
        buf.append( pOEntry->nRef, 10 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("; oid=\"") );
        buf.append( pOEntry->oid );
        buf.append( (sal_Unicode)'\"' );
        writeLine( stream, buf.makeStringAndClear(), pFilter );

        for ( sal_Int32 nPos = pOEntry->aInterfaces.size(); nPos--; )
        {
            const InterfaceEntry & rIEntry = pOEntry->aInterfaces[nPos];
            OSL_ASSERT( rIEntry.pOEntry == pOEntry );

            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("  - ") );
            buf.append( ((typelib_TypeDescription *)rIEntry.pTypeDescr)->pTypeName );
            if (rIEntry.fpFreeProxy)
            {
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("; proxy free=0x") );
                buf.append( (sal_Int64)rIEntry.fpFreeProxy, 16 );
            }
            else
            {
                buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("; original") );
            }
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("; ptr=0x") );
            buf.append( (sal_Int64)rIEntry.pInterface, 16 );
            writeLine( stream, buf.makeStringAndClear(), pFilter );
        }
        ++iPos;
    }
    writeLine( stream, "##############################################################################", pFilter );
}
//##################################################################################################
extern "C" void SAL_CALL uno_dumpEnvironmentByName(
    void * stream, rtl_uString * pEnvTypeName, const sal_Char * pFilter )
    SAL_THROW_EXTERN_C()
{
    uno_Environment * pEnv = 0;
    uno_getEnvironment( &pEnv, pEnvTypeName, 0 );
    if (pEnv)
    {
        ::uno_dumpEnvironment( stream, pEnv, pFilter );
        (*pEnv->release)( pEnv );
    }
    else
    {
        OUStringBuffer buf( 32 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("environment \"") );
        buf.append( pEnvTypeName );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" does not exist!") );
        writeLine( stream, buf.makeStringAndClear(), pFilter );
    }
}

//--------------------------------------------------------------------------------------------------
inline static const OUString & unoenv_getStaticOIdPart()
    SAL_THROW( () )
{
    static OUString * s_pStaticOidPart = 0;
    if (! s_pStaticOidPart)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_pStaticOidPart)
        {
            OUStringBuffer aRet( 64 );
            aRet.appendAscii( RTL_CONSTASCII_STRINGPARAM("];") );
            // pid
            oslProcessInfo info;
            info.Size = sizeof(oslProcessInfo);
            if (::osl_getProcessInfo( 0, osl_Process_IDENTIFIER, &info ) == osl_Process_E_None)
                aRet.append( (sal_Int64)info.Ident, 16 );
            else
                aRet.appendAscii( RTL_CONSTASCII_STRINGPARAM("unknown process id") );
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
//--------------------------------------------------------------------------------------------------
static void SAL_CALL unoenv_computeObjectIdentifier(
    uno_ExtEnvironment * pEnv, rtl_uString ** ppOId, void * pInterface )
    SAL_THROW( () )
{
    OSL_ENSURE( pEnv && ppOId && pInterface, "### null ptr!" );
    if (*ppOId)
    {
        ::rtl_uString_release( *ppOId );
        *ppOId = 0;
    }

    uno_Interface * pUnoI = (uno_Interface *)
        binuno_queryInterface(
            pInterface, * typelib_static_type_getByTypeClass( typelib_TypeClass_INTERFACE ) );
    if (0 != pUnoI)
    {
        (*pUnoI->release)( pUnoI );
        // interface
        OUStringBuffer oid( 64 );
        oid.append( reinterpret_cast< sal_Int64 >( pUnoI ), 16 );
        oid.append( (sal_Unicode)';' );
        // environment[context]
        oid.append( ((uno_Environment *) pEnv)->pTypeName );
        oid.append( (sal_Unicode)'[' );
        oid.append( (sal_Int64)((uno_Environment *) pEnv)->pContext, 16 );
        // process;good guid
        oid.append( unoenv_getStaticOIdPart() );
        OUString aStr( oid.makeStringAndClear() );
        ::rtl_uString_acquire( *ppOId = aStr.pData );
    }
}
//==================================================================================================
static void SAL_CALL unoenv_acquireInterface( uno_ExtEnvironment *, void * pUnoI )
    SAL_THROW( () )
{
    (*((uno_Interface *)pUnoI)->acquire)( (uno_Interface *)pUnoI );
}
//==================================================================================================
static void SAL_CALL unoenv_releaseInterface( uno_ExtEnvironment *, void * pUnoI )
    SAL_THROW( () )
{
    (*((uno_Interface *)pUnoI)->release)( (uno_Interface *)pUnoI );
}
}

//__________________________________________________________________________________________________
EnvironmentsData::~EnvironmentsData() SAL_THROW( () )
{
    MutexGuard aGuard( aMutex );

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
            uno_dumpEnvironment( 0, pHard, 0 );
#endif
#if defined CPPU_LEAK_STATIC_DATA
            pHard->environmentDisposing = 0; // set to null => wont be called
#else
            (*pHard->dispose)( pHard ); // send explicit dispose
#endif
            (*pHard->release)( pHard );
        }
    }
}
//__________________________________________________________________________________________________
inline void EnvironmentsData::getEnvironment(
    uno_Environment ** ppEnv, const OUString & rEnvTypeName, void * pContext )
    SAL_THROW( () )
{
    if (*ppEnv)
    {
        (*(*ppEnv)->release)( *ppEnv );
        *ppEnv = 0;
    }

    OUString aKey( OUString::valueOf( (sal_Int64)pContext ) );
    aKey += rEnvTypeName;

    // try to find registered mapping
    OUString2EnvironmentMap::const_iterator const iFind( aName2EnvMap.find( aKey ) );
    if (iFind != aName2EnvMap.end())
    {
        uno_Environment * pWeak = iFind->second;
        (*pWeak->harden)( ppEnv, pWeak );
    }
}
//__________________________________________________________________________________________________
inline void EnvironmentsData::registerEnvironment( uno_Environment ** ppEnv )
    SAL_THROW( () )
{
    OSL_ENSURE( ppEnv, "### null ptr!" );
    uno_Environment * pEnv =  *ppEnv;

    OUString aKey( OUString::valueOf( (sal_Int64)pEnv->pContext ) );
    aKey += pEnv->pTypeName;

    // try to find registered environment
    OUString2EnvironmentMap::const_iterator const iFind( aName2EnvMap.find( aKey ) );
    if (iFind == aName2EnvMap.end())
    {
        (*pEnv->acquireWeak)( pEnv );
        pair< OUString2EnvironmentMap::iterator, bool > insertion(
            aName2EnvMap.insert( OUString2EnvironmentMap::value_type( aKey, pEnv ) ) );
        OSL_ENSURE( insertion.second, "### insertion of env into map failed?!" );
    }
    else
    {
        uno_Environment * pHard = 0;
        uno_Environment * pWeak = iFind->second;
        (*pWeak->harden)( &pHard, pWeak );
        if (pHard)
        {
            if (pEnv)
            {
                (*pEnv->release)( pEnv );
            }
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
//__________________________________________________________________________________________________
inline void EnvironmentsData::getRegisteredEnvironments(
    uno_Environment *** pppEnvs, sal_Int32 * pnLen, uno_memAlloc memAlloc,
    const OUString & rEnvTypeName )
    SAL_THROW( () )
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
        if (!rEnvTypeName.getLength() || rEnvTypeName.equals( pWeak->pTypeName ))
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
        *pppEnvs = (uno_Environment **)(*memAlloc)( sizeof(uno_Environment *) * nSize );
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

extern "C"
{
//--------------------------------------------------------------------------------------------------
static uno_Environment * initDefaultEnvironment(
    const OUString & rEnvTypeName, void * pContext )
    SAL_THROW( () )
{
    uno_Environment * pEnv = 0;

    // create default environment
    if (rEnvTypeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_LB_UNO) ))
    {
        uno_DefaultEnvironment * that = new uno_DefaultEnvironment( rEnvTypeName, pContext );
        pEnv = (uno_Environment *)that;
        (*pEnv->acquire)( pEnv );
        that->computeObjectIdentifier = unoenv_computeObjectIdentifier;
        that->acquireInterface = unoenv_acquireInterface;
        that->releaseInterface = unoenv_releaseInterface;
    }
    else
    {
        // late init with some code from matching uno language binding
        OUStringBuffer aLibName( 16 );
#ifdef SAL_UNX
        aLibName.appendAscii( RTL_CONSTASCII_STRINGPARAM("lib") );
        aLibName.append( rEnvTypeName );
#ifdef MACOSX
        aLibName.appendAscii( RTL_CONSTASCII_STRINGPARAM("_uno.dylib") );
#else
        aLibName.appendAscii( RTL_CONSTASCII_STRINGPARAM("_uno.so") );
#endif
#else
        aLibName.append( rEnvTypeName );
        aLibName.appendAscii( RTL_CONSTASCII_STRINGPARAM("_uno.dll") );
#endif
        OUString aStr( aLibName.makeStringAndClear() );
        // will be unloaded by environment
        oslModule hMod = ::osl_loadModule( aStr.pData, SAL_LOADMODULE_GLOBAL | SAL_LOADMODULE_LAZY );
        if (hMod)
        {
            OUString aSymbolName( RTL_CONSTASCII_USTRINGPARAM(UNO_INIT_ENVIRONMENT) );
            uno_initEnvironmentFunc fpInit =
                (uno_initEnvironmentFunc)::osl_getSymbol( hMod, aSymbolName.pData );
            if (fpInit)
            {
                pEnv = (uno_Environment *)new uno_DefaultEnvironment(
                    rEnvTypeName, pContext );
                (*pEnv->acquire)( pEnv );
                (*fpInit)( pEnv ); // init of environment
                ::rtl_registerModuleForUnloading( hMod );
            }
            else
            {
                ::osl_unloadModule( hMod );
            }
        }
    }

    return pEnv;
}

//##################################################################################################
void SAL_CALL uno_createEnvironment(
    uno_Environment ** ppEnv, rtl_uString * pEnvTypeName, void * pContext )
    SAL_THROW_EXTERN_C()
{
    OSL_ENSURE( ppEnv, "### null ptr!" );
    if (*ppEnv)
    {
        (*(*ppEnv)->release)( *ppEnv );
    }

    OUString const & rEnvTypeName = * reinterpret_cast< OUString const * >( &pEnvTypeName );
    *ppEnv = initDefaultEnvironment( rEnvTypeName, pContext );
}

//##################################################################################################
void SAL_CALL uno_getEnvironment(
    uno_Environment ** ppEnv, rtl_uString * pEnvTypeName, void * pContext )
    SAL_THROW_EXTERN_C()
{
    OSL_ENSURE( ppEnv, "### null ptr!" );
    if (*ppEnv)
    {
        (*(*ppEnv)->release)( *ppEnv );
    }

    OUString const & rEnvTypeName = * reinterpret_cast< OUString const * >( &pEnvTypeName );

    EnvironmentsData & rData = getEnvironmentsData();

    MutexGuard aGuard( rData.aMutex );
    rData.getEnvironment( ppEnv, rEnvTypeName, pContext );
    if (! *ppEnv)
    {
        if (*ppEnv = initDefaultEnvironment( rEnvTypeName, pContext )) // register new environment
        {
            rData.registerEnvironment( ppEnv );
        }
    }
}

//##################################################################################################
void SAL_CALL uno_getRegisteredEnvironments(
    uno_Environment *** pppEnvs, sal_Int32 * pnLen, uno_memAlloc memAlloc,
    rtl_uString * pEnvTypeName )
    SAL_THROW_EXTERN_C()
{
    EnvironmentsData & rData = getEnvironmentsData();

    MutexGuard aGuard( rData.aMutex );
    rData.getRegisteredEnvironments(
        pppEnvs, pnLen, memAlloc,
        (pEnvTypeName ? OUString( pEnvTypeName ) : OUString()) );
}
} // extern "C"

}
