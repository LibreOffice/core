/*************************************************************************
 *
 *  $RCSfile: lbenv.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:25:53 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _OSL_MODULE_H_
#include <osl/module.h>
#endif
#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _RTL_PROCESS_H_
#include <rtl/process.h>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include <stdio.h>
#ifdef SOLARIS
#include <alloca.h>
#else
#include <malloc.h>
#endif
#include <stl/hash_map>
#include <stl/vector>

#ifndef _TYPELIB_TYPEDESCRIPTION_H_
#include <typelib/typedescription.h>
#endif
#ifndef _UNO_LBNAMES_H_
#include <uno/lbnames.h>
#endif
#ifndef _UNO_DISPATCHER_H_
#include <uno/dispatcher.h>
#endif
#ifndef _UNO_ENVIRONMENT_H_
#include <uno/environment.h>
#endif

#include "prim.hxx"
#include "destr.hxx"

#include <com/sun/star/uno/XInterface.hpp>


using namespace std;
using namespace osl;
using namespace rtl;
using namespace cppu;
using namespace com::sun::star::uno;


//--------------------------------------------------------------------------------------------------
inline static sal_Bool td_equals( typelib_InterfaceTypeDescription * pTD1,
                                  typelib_InterfaceTypeDescription * pTD2 )
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

    inline sal_Bool supports( typelib_InterfaceTypeDescription * pTypeDescr_ ) const;
};
//--------------------------------------------------------------------------------------------------
struct ObjectEntry
{
    uno_ExtEnvironment *        pEnv;
    OUString                    oid;
    sal_Int32                   nRef;
    vector< InterfaceEntry >    aInterfaces;

    inline ObjectEntry( uno_ExtEnvironment * pEnv, const OUString & rOId_ );

    inline void append( void * pInterface, typelib_InterfaceTypeDescription * pTypeDescr,
                        uno_freeProxyFunc fpFreeProxy );
    inline const InterfaceEntry * find( typelib_InterfaceTypeDescription * pTypeDescr ) const;
};

//--------------------------------------------------------------------------------------------------
struct FctPtrHash : public unary_function< const void *, size_t >
{
    size_t operator()( const void * pKey ) const
        { return (size_t)pKey; }
};
//--------------------------------------------------------------------------------------------------
struct FctOUStringHash : public unary_function< const OUString &, size_t >
{
    size_t operator()( const OUString & rKey ) const
        { return rKey.hashCode(); }
};

// mapping from environment name to environment
typedef hash_map< OUString, uno_Environment *, FctOUStringHash, equal_to< OUString > > OUString2EnvironmentMap;

// mapping from ptr to object entry
typedef hash_map< void *, ObjectEntry *, FctPtrHash, equal_to< void * > > Ptr2ObjectMap;
// mapping from oid to object entry
typedef hash_map< OUString, ObjectEntry *, FctOUStringHash, equal_to< OUString > > OId2ObjectMap;

//==================================================================================================
struct EnvironmentsData
{
    Mutex                   aMutex;
    OUString2EnvironmentMap aName2EnvMap;

    ~EnvironmentsData();

    inline uno_Environment * getEnvironment( const OUString & rTypeName, void * pContext );
    inline sal_Bool registerEnvironment( uno_Environment * pEnv );
    inline sal_Bool revokeEnvironment( uno_Environment * pEnv );
    inline void getRegisteredEnvironments(
        uno_Environment *** pppEnvs, sal_Int32 * pnLen, uno_memAlloc memAlloc,
        const OUString & rEnvTypeName );
};
//--------------------------------------------------------------------------------------------------
static EnvironmentsData & getEnvironmentsData()
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
    oslModule           hModule;

    Mutex               aAccess;
    Ptr2ObjectMap       aPtr2ObjectMap;
    OId2ObjectMap       aOId2ObjectMap;

    uno_DefaultEnvironment( const OUString & rTypeName_, oslModule hMod_, void * pContext_ );
    ~uno_DefaultEnvironment();
};

//__________________________________________________________________________________________________
inline ObjectEntry::ObjectEntry( uno_ExtEnvironment * pEnv_, const OUString & rOId_ )
    : pEnv( pEnv_ )
    , oid( rOId_ )
    , nRef( 0 )
{
    aInterfaces.reserve( 8 );
}
//__________________________________________________________________________________________________
inline void ObjectEntry::append( void * pInterface, typelib_InterfaceTypeDescription * pTypeDescr,
                                 uno_freeProxyFunc fpFreeProxy )
{
    InterfaceEntry aNewEntry;
    if (! fpFreeProxy)
        (*pEnv->acquireInterface)( pEnv, pInterface );
    aNewEntry.pInterface    = pInterface;
    aNewEntry.fpFreeProxy   = fpFreeProxy;
    aNewEntry.pOEntry       = this;
    typelib_typedescription_acquire( (typelib_TypeDescription *)(aNewEntry.pTypeDescr = pTypeDescr) );

    aInterfaces.push_back( aNewEntry );
    static_cast< uno_DefaultEnvironment * >( pEnv )->aPtr2ObjectMap[ pInterface ] = this;
}
//__________________________________________________________________________________________________
inline sal_Bool InterfaceEntry::supports( typelib_InterfaceTypeDescription * pTypeDescr_ ) const
{
    typelib_InterfaceTypeDescription * pITD = pTypeDescr;
    while (pITD)
    {
        if (td_equals( pITD, pTypeDescr_ ))
            return sal_True;
        pITD = pITD->pBaseTypeDescription;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
inline const InterfaceEntry * ObjectEntry::find( typelib_InterfaceTypeDescription * pTypeDescr ) const
{
    // shortcut common case
    if (((typelib_TypeDescription *)pTypeDescr)->pTypeName->length == sizeof("com.sun.star.uno.XInterface") &&
        rtl_ustr_ascii_compare( ((typelib_TypeDescription *)pTypeDescr)->pTypeName->buffer,
                                "com.sun.star.uno.XInterface" ) == 0)
    {
        return &aInterfaces[0];
    }

    sal_Int32 nSize = aInterfaces.size();
    for ( sal_Int32 nPos = 0; nPos < nSize; ++nPos )
    {
        OSL_ASSERT( aInterfaces[nPos].pOEntry == this );
        if (aInterfaces[nPos].supports( pTypeDescr ))
            return &aInterfaces[nPos];
    }
    return 0;
}

//--------------------------------------------------------------------------------------------------
static void SAL_CALL defenv_registerInterface(
    uno_ExtEnvironment * pEnv, void ** ppInterface,
    rtl_uString * pOId, typelib_InterfaceTypeDescription * pTypeDescr )
{
    OSL_ENSHURE( pEnv && ppInterface && pOId && pTypeDescr, "### null ptr!" );
    const OUString & rOId = * reinterpret_cast< OUString * >( &pOId );

    ClearableMutexGuard aGuard( static_cast< uno_DefaultEnvironment * >( pEnv )->aAccess );

    const OId2ObjectMap::const_iterator iFind(
        static_cast< uno_DefaultEnvironment * >( pEnv )->aOId2ObjectMap.find( rOId ) );
    if (iFind == static_cast< uno_DefaultEnvironment * >( pEnv )->aOId2ObjectMap.end())
    {
        ObjectEntry * pOEntry = new ObjectEntry( pEnv, rOId );
        static_cast< uno_DefaultEnvironment * >( pEnv )->aOId2ObjectMap[ rOId ] = pOEntry;
        ++pOEntry->nRef; // another register call on object
        pOEntry->append( *ppInterface, pTypeDescr, 0 );
    }
    else // object entry exists
    {
        ObjectEntry * pOEntry = (*iFind).second;
        ++pOEntry->nRef; // another register call on object
        const InterfaceEntry * pIEntry = pOEntry->find( pTypeDescr );

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
{
    OSL_ENSHURE( pEnv && ppInterface && pOId && pTypeDescr, "### null ptr!" );
    const OUString & rOId = * reinterpret_cast< OUString * >( &pOId );

    ClearableMutexGuard aGuard( static_cast< uno_DefaultEnvironment * >( pEnv )->aAccess );

    const OId2ObjectMap::const_iterator iFind(
        static_cast< uno_DefaultEnvironment * >( pEnv )->aOId2ObjectMap.find( rOId ) );
    if (iFind == static_cast< uno_DefaultEnvironment * >( pEnv )->aOId2ObjectMap.end())
    {
        ObjectEntry * pOEntry = new ObjectEntry( pEnv, rOId );
        static_cast< uno_DefaultEnvironment * >( pEnv )->aOId2ObjectMap[ rOId ] = pOEntry;
        ++pOEntry->nRef; // another register call on object
        pOEntry->append( *ppInterface, pTypeDescr, freeProxy );
    }
    else // object entry exists
    {
        ObjectEntry * pOEntry = (*iFind).second;
        ++pOEntry->nRef; // another register call on object
        const InterfaceEntry * pIEntry = pOEntry->find( pTypeDescr );

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
{
    OSL_ENSHURE( pEnv && pInterface, "### null ptr!" );
    ClearableMutexGuard aGuard( static_cast< uno_DefaultEnvironment * >( pEnv )->aAccess );

    const Ptr2ObjectMap::const_iterator iFind(
        static_cast< uno_DefaultEnvironment * >( pEnv )->aPtr2ObjectMap.find( pInterface ) );
    OSL_ASSERT( iFind != static_cast< uno_DefaultEnvironment * >( pEnv )->aPtr2ObjectMap.end() );
    if (! --(*iFind).second->nRef)
    {
        ObjectEntry * pOEntry = (*iFind).second;
        OSL_ASSERT( pEnv == pOEntry->pEnv );
        // cleanup maps
        static_cast< uno_DefaultEnvironment * >( pEnv )->aOId2ObjectMap.erase( pOEntry->oid );
        sal_Int32 nPos;
        for ( nPos = pOEntry->aInterfaces.size(); nPos--; )
        {
            static_cast< uno_DefaultEnvironment * >( pEnv )->aPtr2ObjectMap.erase(
                pOEntry->aInterfaces[nPos].pInterface );
        }

        // the last proxy interface of the environment might kill this environment,
        // because of releasing its language binding!!!
        aGuard.clear();

        // release interfaces
        for ( nPos = pOEntry->aInterfaces.size(); nPos--; )
        {
            const InterfaceEntry & rEntry = pOEntry->aInterfaces[nPos];
            typelib_typedescription_release( (typelib_TypeDescription *)rEntry.pTypeDescr );
            if (rEntry.fpFreeProxy) // is proxy or used interface?
                (*rEntry.fpFreeProxy)( pEnv, rEntry.pInterface );
            else
                (*pEnv->releaseInterface)( pEnv, rEntry.pInterface );
        }

        delete pOEntry;
    }
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL defenv_getObjectIdentifier(
    uno_ExtEnvironment * pEnv, rtl_uString ** ppOId, void * pInterface )
{
    OSL_ENSHURE( pEnv && ppOId && pInterface, "### null ptr!" );
    if (*ppOId)
    {
        rtl_uString_release( *ppOId );
        *ppOId = 0;
    }

    ClearableMutexGuard aGuard( static_cast< uno_DefaultEnvironment * >( pEnv )->aAccess );

    const Ptr2ObjectMap::const_iterator iFind(
        static_cast< uno_DefaultEnvironment * >( pEnv )->aPtr2ObjectMap.find( pInterface ) );
    if (iFind != static_cast< uno_DefaultEnvironment * >( pEnv )->aPtr2ObjectMap.end())
    {
        rtl_uString_acquire( *ppOId = (*iFind).second->oid.pData );
    }
    else
    {
        aGuard.clear();
        (*pEnv->computeObjectIdentifier)( pEnv, ppOId, pInterface );
    }
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL defenv_getRegisteredInterface(
    uno_ExtEnvironment * pEnv, void ** ppInterface,
    rtl_uString * pOId, typelib_InterfaceTypeDescription * pTypeDescr )
{
    OSL_ENSHURE( pEnv && ppInterface && pOId && pTypeDescr, "### null ptr!" );
    if (*ppInterface)
    {
        (*pEnv->releaseInterface)( pEnv, *ppInterface );
        *ppInterface = 0;
    }

    const OUString & rOId = * reinterpret_cast< OUString * >( &pOId );

    MutexGuard aGuard( static_cast< uno_DefaultEnvironment * >( pEnv )->aAccess );

    const OId2ObjectMap::const_iterator iFind(
        static_cast< uno_DefaultEnvironment * >( pEnv )->aOId2ObjectMap.find( rOId ) );
    if (iFind != static_cast< uno_DefaultEnvironment * >( pEnv )->aOId2ObjectMap.end())
    {
        const InterfaceEntry * pIEntry = (*iFind).second->find( pTypeDescr );
        if (pIEntry)
            (*pEnv->acquireInterface)( pEnv, *ppInterface = pIEntry->pInterface );
    }
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL defenv_getRegisteredInterfaces(
    uno_ExtEnvironment * pEnv, void *** pppInterfaces, sal_Int32 * pnLen, uno_memAlloc memAlloc )
{
    OSL_ENSHURE( pEnv && pppInterfaces && pnLen && memAlloc, "### null ptr!" );
    MutexGuard aGuard( static_cast< uno_DefaultEnvironment * >( pEnv )->aAccess );

    sal_Int32 nLen = static_cast< uno_DefaultEnvironment * >( pEnv )->aPtr2ObjectMap.size();
    sal_Int32 nPos = 0;
    void ** ppInterfaces = (void **)(*memAlloc)( nLen * sizeof(void *) );

    Ptr2ObjectMap::const_iterator iPos(
        static_cast< uno_DefaultEnvironment * >( pEnv )->aPtr2ObjectMap.begin() );
    while (iPos != static_cast< uno_DefaultEnvironment * >( pEnv )->aPtr2ObjectMap.end())
    {
        (*pEnv->acquireInterface)( pEnv, ppInterfaces[nPos++] = (*iPos).first );
        ++iPos;
    }

    *pppInterfaces = ppInterfaces;
    *pnLen = nLen;
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL defenv_acquire( uno_Environment * pEnv )
{
    osl_incrementInterlockedCount( &((uno_DefaultEnvironment *)pEnv)->nRef );
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL defenv_release( uno_Environment * pEnv )
{
    MutexGuard aGuard( getEnvironmentsData().aMutex );
    if (! osl_decrementInterlockedCount( &((uno_DefaultEnvironment *)pEnv)->nRef ))
    {
        getEnvironmentsData().revokeEnvironment( pEnv );
        delete (uno_DefaultEnvironment *)pEnv;
    }
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL defenv_dispose( uno_Environment * pEnv )
{
}

//__________________________________________________________________________________________________
uno_DefaultEnvironment::uno_DefaultEnvironment(
    const OUString & rTypeName_, void * pContext_, oslModule hMod_ )
    : nRef( 0 )
    , hModule( hMod_ )
{
    ((uno_Environment *)this)->pReserved                = 0;
    // functions
    ((uno_Environment *)this)->acquire                  = defenv_acquire;
    ((uno_Environment *)this)->release                  = defenv_release;
    ((uno_Environment *)this)->dispose                  = defenv_dispose;
    ((uno_Environment *)this)->pExtEnv                  = this;
    // identifier
    rtl_uString_acquire( rTypeName_.pData );
    ((uno_Environment *)this)->pTypeName                = rTypeName_.pData;
    ((uno_Environment *)this)->pContext                 = pContext_;

    // will be late initialized
    ((uno_Environment *)this)->environmentDisposing     = 0;

    uno_ExtEnvironment::registerInterface       = defenv_registerInterface;
    uno_ExtEnvironment::registerProxyInterface  = defenv_registerProxyInterface;
    uno_ExtEnvironment::revokeInterface         = defenv_revokeInterface;
    uno_ExtEnvironment::getObjectIdentifier     = defenv_getObjectIdentifier;
    uno_ExtEnvironment::getRegisteredInterface  = defenv_getRegisteredInterface;
    uno_ExtEnvironment::getRegisteredInterfaces = defenv_getRegisteredInterfaces;

}
//__________________________________________________________________________________________________
uno_DefaultEnvironment::~uno_DefaultEnvironment()
{
    OSL_ENSHURE( aOId2ObjectMap.empty(), "### object entries left!" );

    if (((uno_Environment *)this)->environmentDisposing)
        (*((uno_Environment *)this)->environmentDisposing)( (uno_Environment *)this );
    if (hModule)
        osl_unloadModule( hModule );

    rtl_uString_release( ((uno_Environment *)this)->pTypeName );
}

//==================================================================================================
static void writeLine( void * stream, const sal_Char * pLine, const sal_Char * pFilter )
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
                    ++nPos;
                if (! pFilter[nPos])
                {
                    if (stream)
                    {
                        fprintf( (FILE *)stream, "%s\n", pLine );
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
            fprintf( (FILE *)stream, "%s\n", pLine );
        }
        else
        {
            OSL_TRACE( pLine );
            OSL_TRACE( "\n" );
        }
    }
}
//==================================================================================================
static void writeLine( void * stream, const OUString & rLine, const sal_Char * pFilter )
{
    OString aLine( OUStringToOString( rLine, RTL_TEXTENCODING_ASCII_US ) );
    writeLine( stream, aLine.getStr(), pFilter );
}

//##################################################################################################
extern "C" SAL_DLLEXPORT void SAL_CALL uno_dumpEnvironment(
    void * stream, uno_Environment * pEnv, const sal_Char * pFilter )
{
    OSL_ENSHURE( pEnv, "### null ptr!" );
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
        ObjectEntry * pOEntry = (*iPos).second;
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
extern "C" SAL_DLLEXPORT void SAL_CALL uno_dumpEnvironmentByName(
    void * stream, rtl_uString * pEnvTypeName, const sal_Char * pFilter )
{
    uno_Environment * pEnv = 0;
    uno_getEnvironment( &pEnv, pEnvTypeName, 0 );
    if (pEnv)
    {
        uno_dumpEnvironment( stream, pEnv, pFilter );
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
            if (osl_getProcessInfo( 0, osl_Process_IDENTIFIER, &info ) == osl_Process_E_None)
                aRet.append( (sal_Int64)info.Ident, 16 );
            else
                aRet.appendAscii( RTL_CONSTASCII_STRINGPARAM("unknown process id") );
            // good guid
            sal_uInt8 ar[16];
            rtl_getGlobalProcessId( ar );
            aRet.append( (sal_Unicode)';' );
            for ( sal_Int32 i = 0; i < 16; ++i )
                aRet.append( (sal_Int32)ar[i], 16 );

            static OUString s_aStaticOidPart( aRet.makeStringAndClear() );
            s_pStaticOidPart = &s_aStaticOidPart;
        }
    }
    return *s_pStaticOidPart;
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL unoenv_computeObjectIdentifier(
    uno_ExtEnvironment * pEnv, rtl_uString ** ppOId, void * pInterface )
{
    OSL_ENSHURE( pEnv && ppOId && pInterface, "### null ptr!" );
    if (*ppOId)
    {
        rtl_uString_release( *ppOId );
        *ppOId = 0;
    }

    typelib_InterfaceTypeDescription * pTXInterfaceDescr = 0;
    const Type & rXIType = ::getCppuType( (const Reference< XInterface > *)0 );
    typelib_TypeDescription * pMTqueryInterface = __getQueryInterfaceTypeDescr();

    void * pArgs[1];
    pArgs[0] = const_cast< Type * >( &rXIType );
    uno_Any aRet, aExc;
    uno_Any * pExc = &aExc;

    (*((uno_Interface *)pInterface)->pDispatcher)(
        (uno_Interface *)pInterface, pMTqueryInterface, &aRet, pArgs, &pExc );

    OSL_ENSHURE( !pExc, "### Exception occured during queryInterface()!" );
    if (pExc) // cleanup exception object
    {
        __destructAny( pExc, 0 );
    }
    else
    {
        OSL_ENSHURE( aRet.pType->eTypeClass == typelib_TypeClass_INTERFACE,
                     "### cannot query for XInterface!" );
        if (aRet.pType->eTypeClass == typelib_TypeClass_INTERFACE)
        {
            // interface
            OUStringBuffer oid( 64 );
            oid.append( (sal_Int64)*(void **)aRet.pData, 16 );
            oid.append( (sal_Unicode)';' );
            // environment[context]
            oid.append( ((uno_Environment *)pEnv)->pTypeName );
            oid.append( (sal_Unicode)'[' );
            oid.append( (sal_Int64)((uno_Environment *)pEnv)->pContext, 16 );
            // process;good guid
            oid.append( unoenv_getStaticOIdPart() );
            OUString aStr( oid.makeStringAndClear() );
            rtl_uString_acquire( *ppOId = aStr.pData );
        }
        __destructAny( &aRet, 0 );
    }

    typelib_typedescription_release( pMTqueryInterface );
}
//==================================================================================================
static void SAL_CALL unoenv_acquireInterface( uno_ExtEnvironment *, void * pUnoI )
{
    (*((uno_Interface *)pUnoI)->acquire)( (uno_Interface *)pUnoI );
}
//==================================================================================================
static void SAL_CALL unoenv_releaseInterface( uno_ExtEnvironment *, void * pUnoI )
{
    (*((uno_Interface *)pUnoI)->release)( (uno_Interface *)pUnoI );
}

//__________________________________________________________________________________________________
EnvironmentsData::~EnvironmentsData()
{
    MutexGuard aGuard( aMutex );

    OUString2EnvironmentMap::const_iterator iPos( aName2EnvMap.begin() );
#ifdef CPPU_ALLOW_ASSERTIONS
    OSL_ENSHURE( aName2EnvMap.empty(), "### unrevoked environments!  living proxies?" );
    while (iPos != aName2EnvMap.end())
    {
        ::uno_dumpEnvironment( 0, (*iPos).second, 0 );
        ++iPos;
    }
    iPos = aName2EnvMap.begin();
#endif
    while (iPos != aName2EnvMap.end())
    {
        (*(*iPos).second->dispose)( (*iPos).second );
        ++iPos;
    }
}
//__________________________________________________________________________________________________
inline uno_Environment * EnvironmentsData::getEnvironment(
    const OUString & rEnvTypeName, void * pContext )
{
    uno_Environment * pEnv = 0;

    OUString aKey( OUString::valueOf( (sal_Int32)pContext ) );
    aKey += rEnvTypeName;

    // try to find registered mapping
    MutexGuard aGuard( aMutex );
    const OUString2EnvironmentMap::const_iterator iFind( aName2EnvMap.find( aKey ) );
    if (iFind != aName2EnvMap.end())
    {
        pEnv = (*iFind).second;
        (*pEnv->acquire)( pEnv );
    }

    return pEnv;
}
//__________________________________________________________________________________________________
inline sal_Bool EnvironmentsData::registerEnvironment( uno_Environment * pEnv )
{
    OSL_ENSHURE( pEnv, "### null ptr!" );
    OUString aKey( OUString::valueOf( (sal_Int64)pEnv->pContext ) );
    aKey += pEnv->pTypeName;

    // try to find environment
    MutexGuard aGuard( aMutex );
    const OUString2EnvironmentMap::const_iterator iFind( aName2EnvMap.find( aKey ) );
    OSL_ENSHURE( iFind == aName2EnvMap.end(), "### environment already registered!" );
    if (iFind == aName2EnvMap.end())
    {
        aName2EnvMap[ aKey ] = pEnv;
        return sal_True;
    }
    return sal_False;
}
//__________________________________________________________________________________________________
inline sal_Bool EnvironmentsData::revokeEnvironment( uno_Environment * pEnv )
{
    OSL_ENSHURE( pEnv, "### null ptr!" );
    OUString aKey( OUString::valueOf( (sal_Int64)pEnv->pContext ) );
    aKey += pEnv->pTypeName;

    MutexGuard aGuard( aMutex );
    OSL_ENSHURE( aName2EnvMap.find( aKey ) != aName2EnvMap.end(), "### env reg error!" );
    aName2EnvMap.erase( aKey );
    return sal_True;
}
//__________________________________________________________________________________________________
inline void EnvironmentsData::getRegisteredEnvironments(
    uno_Environment *** pppEnvs, sal_Int32 * pnLen, uno_memAlloc memAlloc,
    const OUString & rEnvTypeName )
{
    OSL_ENSHURE( pppEnvs && pnLen && memAlloc, "### null ptr!" );

    uno_Environment ** ppFound;
    sal_Int32 nSize = 0;

    {
    MutexGuard aGuard( aMutex );
    // max size
    ppFound = (uno_Environment **)alloca( sizeof(uno_Environment *) * aName2EnvMap.size() );
    // find matching environment
    for ( OUString2EnvironmentMap::const_iterator iPos( aName2EnvMap.begin() );
          iPos != aName2EnvMap.end(); ++iPos )
    {
        uno_Environment * pEnv = (*iPos).second;
        if (!rEnvTypeName.getLength() || rEnvTypeName.equals( pEnv->pTypeName ))
        {
            (*pEnv->acquire)( pEnv );
            ppFound[nSize++] = pEnv;
        }
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

//--------------------------------------------------------------------------------------------------
static uno_Environment * initDefaultEnvironment( const OUString & rEnvTypeName, void * pContext )
{
    uno_Environment * pEnv = 0;

    // create default environment
    if (rEnvTypeName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM(UNO_LB_UNO) ))
    {
        uno_DefaultEnvironment * pDefEnv = new uno_DefaultEnvironment( rEnvTypeName, pContext, 0 );
        pEnv = (uno_Environment *)pDefEnv;
        (*pEnv->acquire)( pEnv );
        pDefEnv->computeObjectIdentifier = unoenv_computeObjectIdentifier;
        pDefEnv->acquireInterface        = unoenv_acquireInterface;
        pDefEnv->releaseInterface        = unoenv_releaseInterface;
    }
    else
    {
        // late init with some code from matching uno language binding
        OUStringBuffer aLibName( 16 );
#ifdef SAL_UNX
        aLibName.appendAscii( RTL_CONSTASCII_STRINGPARAM("lib") );
        aLibName.append( rEnvTypeName );
        aLibName.appendAscii( RTL_CONSTASCII_STRINGPARAM("_uno.so") );
#else
        aLibName.append( rEnvTypeName );
        aLibName.appendAscii( RTL_CONSTASCII_STRINGPARAM("_uno.dll") );
#endif
        OUString aStr( aLibName.makeStringAndClear() );
        // will be unloaded by environment
        oslModule hMod = osl_loadModule( aStr.pData, SAL_LOADMODULE_GLOBAL | SAL_LOADMODULE_LAZY );
        if (hMod)
        {
            OUString aSymbolName( RTL_CONSTASCII_USTRINGPARAM(UNO_INIT_ENVIRONMENT) );
            uno_initEnvironmentFunc fpInit =
                (uno_initEnvironmentFunc)osl_getSymbol( hMod, aSymbolName.pData );
            if (fpInit)
            {
                pEnv = (uno_Environment *)new uno_DefaultEnvironment( rEnvTypeName, pContext, hMod );
                (*pEnv->acquire)( pEnv );
                (*fpInit)( pEnv ); // init of environment
            }
            else
            {
                osl_unloadModule( hMod );
            }
        }
    }

    return pEnv;
}

//--------------------------------------------------------------------------------------------------
static void SAL_CALL anonymous_defenv_release( uno_Environment * pEnv )
{
    if (! osl_decrementInterlockedCount( &((uno_DefaultEnvironment *)pEnv)->nRef ))
        delete (uno_DefaultEnvironment *)pEnv;
}

//##################################################################################################
extern "C" SAL_DLLEXPORT void SAL_CALL uno_createEnvironment(
    uno_Environment ** ppEnv, rtl_uString * pEnvTypeName, void * pContext )
{
    OSL_ENSHURE( ppEnv, "### null ptr!" );
    if (*ppEnv)
        (*(*ppEnv)->release)( *ppEnv );

    *ppEnv = initDefaultEnvironment(
        * reinterpret_cast< OUString * >( &pEnvTypeName ), pContext );
    (*ppEnv)->release = anonymous_defenv_release; // patch release func
}

//##################################################################################################
extern "C" SAL_DLLEXPORT void SAL_CALL uno_getEnvironment(
    uno_Environment ** ppEnv, rtl_uString * pEnvTypeName, void * pContext )
{
    OSL_ENSHURE( ppEnv, "### null ptr!" );
    if (*ppEnv)
        (*(*ppEnv)->release)( *ppEnv );

    const OUString & rEnvTypeName = * reinterpret_cast< OUString * >( &pEnvTypeName );

    MutexGuard aGuard( getEnvironmentsData().aMutex );
    *ppEnv = getEnvironmentsData().getEnvironment( rEnvTypeName, pContext );

    if (! *ppEnv)
    {
        if (*ppEnv = initDefaultEnvironment( rEnvTypeName, pContext )) // register new environment
            getEnvironmentsData().registerEnvironment( *ppEnv );
    }
}

//##################################################################################################
extern "C" SAL_DLLEXPORT void SAL_CALL uno_getRegisteredEnvironments(
    uno_Environment *** pppEnvs, sal_Int32 * pnLen, uno_memAlloc memAlloc,
    rtl_uString * pEnvTypeName )
{
    getEnvironmentsData().getRegisteredEnvironments(
        pppEnvs, pnLen, memAlloc,
        (pEnvTypeName ? OUString( pEnvTypeName ) : OUString()) );
}
