/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _BRIDGES_CPP_UNO_BRIDGE_HXX_
#define _BRIDGES_CPP_UNO_BRIDGE_HXX_

#include <bridges/cpp_uno/bridge.h>
#include <osl/mutex.hxx>
#include <rtl/process.h>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/genfunc.hxx>
#include <com/sun/star/uno/XInterface.hpp>


namespace CPPU_CURRENT_NAMESPACE
{

//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_cppInterfaceProxy_free( uno_ExtEnvironment * pEnv, void * pProxy ) SAL_THROW(())
{
    cppu_cppInterfaceProxy * pThis =
        static_cast< cppu_cppInterfaceProxy * >(
            reinterpret_cast< ::com::sun::star::uno::XInterface * >( pProxy ) );
    OSL_ASSERT( pEnv == pThis->pBridge->pCppEnv );

    (*pThis->pBridge->pUnoEnv->revokeInterface)( pThis->pBridge->pUnoEnv, pThis->pUnoI );
    (*pThis->pUnoI->release)( pThis->pUnoI );
    ::typelib_typedescription_release( (typelib_TypeDescription *)pThis->pTypeDescr );
    pThis->pBridge->release();

#if OSL_DEBUG_LEVEL > 1
    *(int *)pProxy = 0xdeadbabe;
#endif
    delete pThis;
}
//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_Mapping_uno2cpp(
    uno_Mapping * pMapping, void ** ppCppI,
    void * pUnoI, typelib_InterfaceTypeDescription * pTypeDescr ) SAL_THROW(())
{
    OSL_ASSERT( ppCppI && pTypeDescr );
    if (*ppCppI)
    {
        reinterpret_cast< ::com::sun::star::uno::XInterface * >( *ppCppI )->release();
        *ppCppI = 0;
    }
    if (pUnoI)
    {
        cppu_Bridge * pBridge = static_cast< cppu_Mapping * >( pMapping )->pBridge;

        // get object id of uno interface to be wrapped
        rtl_uString * pOId = 0;
        (*pBridge->pUnoEnv->getObjectIdentifier)( pBridge->pUnoEnv, &pOId, pUnoI );
        OSL_ASSERT( pOId );

        // try to get any known interface from target environment
        (*pBridge->pCppEnv->getRegisteredInterface)(
            pBridge->pCppEnv, ppCppI, pOId, pTypeDescr );

        if (! *ppCppI) // no existing interface, register new proxy interface
        {
            // try to publish a new proxy (ref count initially 1)
            cppu_cppInterfaceProxy * pProxy = new cppu_cppInterfaceProxy(
                pBridge, reinterpret_cast< uno_Interface * >( pUnoI ), pTypeDescr, pOId );
            ::com::sun::star::uno::XInterface * pSurrogate = pProxy;
            cppu_cppInterfaceProxy_patchVtable( pSurrogate, pProxy->pTypeDescr );

            // proxy may be exchanged during registration
            (*pBridge->pCppEnv->registerProxyInterface)(
                pBridge->pCppEnv, reinterpret_cast< void ** >( &pSurrogate ),
                (uno_freeProxyFunc)cppu_cppInterfaceProxy_free, pOId, pTypeDescr );

            *ppCppI = pSurrogate;
        }
        ::rtl_uString_release( pOId );
    }
}
//__________________________________________________________________________________________________
inline void cppu_cppInterfaceProxy::acquireProxy() SAL_THROW(())
{
    if (1 == osl_atomic_increment( &nRef ))
    {
        // rebirth of proxy zombie
        // register at cpp env
        void * pThis = static_cast< ::com::sun::star::uno::XInterface * >( this );
        (*pBridge->pCppEnv->registerProxyInterface)(
            pBridge->pCppEnv, &pThis, (uno_freeProxyFunc)cppu_cppInterfaceProxy_free,
            oid.pData, pTypeDescr );
        OSL_ASSERT( pThis == static_cast< ::com::sun::star::uno::XInterface * >( this ) );
    }
}
//__________________________________________________________________________________________________
inline void cppu_cppInterfaceProxy::releaseProxy() SAL_THROW(())
{
    if (! osl_atomic_decrement( &nRef )) // last release
    {
        // revoke from cpp env
        (*pBridge->pCppEnv->revokeInterface)(
            pBridge->pCppEnv, static_cast< ::com::sun::star::uno::XInterface * >( this ) );
    }
}
//__________________________________________________________________________________________________
inline cppu_cppInterfaceProxy::cppu_cppInterfaceProxy(
    cppu_Bridge * pBridge_, uno_Interface * pUnoI_,
    typelib_InterfaceTypeDescription * pTypeDescr_, const ::rtl::OUString & rOId_ ) SAL_THROW(())
    : nRef( 1 )
    , pBridge( pBridge_ )
    , pUnoI( pUnoI_ )
    , pTypeDescr( pTypeDescr_ )
    , oid( rOId_ )
{
    pBridge->acquire();
    ::typelib_typedescription_acquire( (typelib_TypeDescription *)pTypeDescr );
    if (! ((typelib_TypeDescription *)pTypeDescr)->bComplete)
        ::typelib_typedescription_complete( (typelib_TypeDescription **)&pTypeDescr );
    OSL_ENSURE( ((typelib_TypeDescription *)pTypeDescr)->bComplete, "### type is incomplete!" );
    (*pUnoI->acquire)( pUnoI );
    (*pBridge->pUnoEnv->registerInterface)(
        pBridge->pUnoEnv, reinterpret_cast< void ** >( &pUnoI ), oid.pData, pTypeDescr );
}


//##################################################################################################
//##################################################################################################
//##################################################################################################


//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_unoInterfaceProxy_free( uno_ExtEnvironment * pEnv, void * pProxy ) SAL_THROW(())
{
    cppu_unoInterfaceProxy * pThis =
        static_cast< cppu_unoInterfaceProxy * >(
            reinterpret_cast< uno_Interface * >( pProxy ) );
    OSL_ASSERT( pEnv == pThis->pBridge->pUnoEnv );

    (*pThis->pBridge->pCppEnv->revokeInterface)( pThis->pBridge->pCppEnv, pThis->pCppI );
    pThis->pCppI->release();
    ::typelib_typedescription_release( (typelib_TypeDescription *)pThis->pTypeDescr );
    pThis->pBridge->release();

#if OSL_DEBUG_LEVEL > 1
    *(int *)pProxy = 0xdeadbabe;
#endif
    delete pThis;
}
//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_unoInterfaceProxy_acquire( uno_Interface * pUnoI ) SAL_THROW(())
{
    if (1 == osl_atomic_increment( & static_cast< cppu_unoInterfaceProxy * >( pUnoI )->nRef ))
    {
        // rebirth of proxy zombie
        // register at uno env
#if OSL_DEBUG_LEVEL > 1
        void * pThis = pUnoI;
#endif
        (*static_cast< cppu_unoInterfaceProxy * >( pUnoI )->pBridge->pUnoEnv->registerProxyInterface)(
            static_cast< cppu_unoInterfaceProxy * >( pUnoI )->pBridge->pUnoEnv,
            reinterpret_cast< void ** >( &pUnoI ),
            (uno_freeProxyFunc)cppu_unoInterfaceProxy_free,
            static_cast< cppu_unoInterfaceProxy * >( pUnoI )->oid.pData,
            static_cast< cppu_unoInterfaceProxy * >( pUnoI )->pTypeDescr );
#if OSL_DEBUG_LEVEL > 1
        OSL_ASSERT( pThis == pUnoI );
#endif
    }
}
//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_unoInterfaceProxy_release( uno_Interface * pUnoI ) SAL_THROW(())
{
    if (! osl_atomic_decrement( & static_cast< cppu_unoInterfaceProxy * >( pUnoI )->nRef ))
    {
        // revoke from uno env on last release
        (*static_cast< cppu_unoInterfaceProxy * >( pUnoI )->pBridge->pUnoEnv->revokeInterface)(
            static_cast< cppu_unoInterfaceProxy * >( pUnoI )->pBridge->pUnoEnv, pUnoI );
    }
}
//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_Mapping_cpp2uno(
    uno_Mapping * pMapping, void ** ppUnoI,
    void * pCppI, typelib_InterfaceTypeDescription * pTypeDescr ) SAL_THROW(())
{
    OSL_ENSURE( ppUnoI && pTypeDescr, "### null ptr!" );
    if (*ppUnoI)
    {
        (*reinterpret_cast< uno_Interface * >( *ppUnoI )->release)(
            reinterpret_cast< uno_Interface * >( *ppUnoI ) );
        *ppUnoI = 0;
    }
    if (pCppI)
    {
        cppu_Bridge * pBridge = static_cast< cppu_Mapping * >( pMapping )->pBridge;

        // get object id of interface to be wrapped
        rtl_uString * pOId = 0;
        (*pBridge->pCppEnv->getObjectIdentifier)( pBridge->pCppEnv, &pOId, pCppI );
        OSL_ASSERT( pOId );

        // try to get any known interface from target environment
        (*pBridge->pUnoEnv->getRegisteredInterface)(
            pBridge->pUnoEnv, ppUnoI, pOId, pTypeDescr );

        if (! *ppUnoI) // no existing interface, register new proxy interface
        {
            // try to publish a new proxy (refcount initially 1)
            uno_Interface * pSurrogate = new cppu_unoInterfaceProxy(
                pBridge, reinterpret_cast< ::com::sun::star::uno::XInterface * >( pCppI ),
                pTypeDescr, pOId );

            // proxy may be exchanged during registration
            (*pBridge->pUnoEnv->registerProxyInterface)(
                pBridge->pUnoEnv, reinterpret_cast< void ** >( &pSurrogate ),
                (uno_freeProxyFunc)cppu_unoInterfaceProxy_free, pOId, pTypeDescr );

            *ppUnoI = pSurrogate;
        }
        ::rtl_uString_release( pOId );
    }
}
//__________________________________________________________________________________________________
inline cppu_unoInterfaceProxy::cppu_unoInterfaceProxy(
    cppu_Bridge * pBridge_, ::com::sun::star::uno::XInterface * pCppI_,
    typelib_InterfaceTypeDescription * pTypeDescr_, const ::rtl::OUString & rOId_ ) SAL_THROW(())
    : nRef( 1 )
    , pBridge( pBridge_ )
    , pCppI( pCppI_ )
    , pTypeDescr( pTypeDescr_ )
    , oid( rOId_ )
{
    pBridge->acquire();
    ::typelib_typedescription_acquire( (typelib_TypeDescription *)pTypeDescr );
    if (! ((typelib_TypeDescription *)pTypeDescr)->bComplete)
        ::typelib_typedescription_complete( (typelib_TypeDescription **)&pTypeDescr );
    OSL_ENSURE( ((typelib_TypeDescription *)pTypeDescr)->bComplete, "### type is incomplete!" );
    pCppI->acquire();
    (*pBridge->pCppEnv->registerInterface)(
        pBridge->pCppEnv, reinterpret_cast< void ** >( &pCppI ), oid.pData, pTypeDescr );

    // uno_Interface
    uno_Interface::acquire = cppu_unoInterfaceProxy_acquire;
    uno_Interface::release = cppu_unoInterfaceProxy_release;
    uno_Interface::pDispatcher = (uno_DispatchMethod)cppu_unoInterfaceProxy_dispatch;
}


//##################################################################################################
//##################################################################################################
//##################################################################################################


//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_Mapping_acquire( uno_Mapping * pMapping ) SAL_THROW(())
{
    static_cast< cppu_Mapping * >( pMapping )->pBridge->acquire();
}
//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_Mapping_release( uno_Mapping * pMapping ) SAL_THROW(())
{
    static_cast< cppu_Mapping * >( pMapping )->pBridge->release();
}
//__________________________________________________________________________________________________
inline cppu_Bridge::cppu_Bridge(
    uno_ExtEnvironment * pCppEnv_, uno_ExtEnvironment * pUnoEnv_,
    sal_Bool bExportCpp2Uno_ ) SAL_THROW(())
    : nRef( 1 )
    , pCppEnv( pCppEnv_ )
    , pUnoEnv( pUnoEnv_ )
    , bExportCpp2Uno( bExportCpp2Uno_ )
{
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );

    aCpp2Uno.pBridge = this;
    aCpp2Uno.acquire = cppu_Mapping_acquire;
    aCpp2Uno.release = cppu_Mapping_release;
    aCpp2Uno.mapInterface = cppu_Mapping_cpp2uno;

    aUno2Cpp.pBridge = this;
    aUno2Cpp.acquire = cppu_Mapping_acquire;
    aUno2Cpp.release = cppu_Mapping_release;
    aUno2Cpp.mapInterface = cppu_Mapping_uno2cpp;

    (*((uno_Environment *)pCppEnv)->acquire)( (uno_Environment *)pCppEnv );
    (*((uno_Environment *)pUnoEnv)->acquire)( (uno_Environment *)pUnoEnv );
}
//__________________________________________________________________________________________________
inline cppu_Bridge::~cppu_Bridge() SAL_THROW(())
{
    (*((uno_Environment *)pUnoEnv)->release)( (uno_Environment *)pUnoEnv );
    (*((uno_Environment *)pCppEnv)->release)( (uno_Environment *)pCppEnv );
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}
//__________________________________________________________________________________________________
inline void SAL_CALL cppu_Bridge_free( uno_Mapping * pMapping ) SAL_THROW(())
{
    delete static_cast< cppu_Mapping * >( pMapping )->pBridge;
}
//__________________________________________________________________________________________________
inline void cppu_Bridge::acquire() SAL_THROW(())
{
    if (1 == osl_atomic_increment( &nRef ))
    {
        if (bExportCpp2Uno)
        {
            uno_Mapping * pMapping = &aCpp2Uno;
            ::uno_registerMapping(
                &pMapping, cppu_Bridge_free,
                (uno_Environment *)pCppEnv, (uno_Environment *)pUnoEnv, 0 );
        }
        else
        {
            uno_Mapping * pMapping = &aUno2Cpp;
            ::uno_registerMapping(
                &pMapping, cppu_Bridge_free,
                (uno_Environment *)pUnoEnv, (uno_Environment *)pCppEnv, 0 );
        }
    }
}
//__________________________________________________________________________________________________
inline void cppu_Bridge::release() SAL_THROW(())
{
    if (! osl_atomic_decrement( &nRef ))
    {
        ::uno_revokeMapping( bExportCpp2Uno ? &aCpp2Uno : &aUno2Cpp );
    }
}

//##################################################################################################
inline void SAL_CALL cppu_ext_getMapping(
    uno_Mapping ** ppMapping, uno_Environment * pFrom, uno_Environment * pTo ) SAL_THROW(())
{
    OSL_ASSERT( ppMapping && pFrom && pTo );
    if (ppMapping && pFrom && pTo && pFrom->pExtEnv && pTo->pExtEnv)
    {
        uno_Mapping * pMapping = 0;

        if (0 == rtl_ustr_ascii_compare( pFrom->pTypeName->buffer, CPPU_CURRENT_LANGUAGE_BINDING_NAME ) &&
            0 == rtl_ustr_ascii_compare( pTo->pTypeName->buffer, UNO_LB_UNO ))
        {
            // ref count initially 1
            pMapping = &(new cppu_Bridge( pFrom->pExtEnv, pTo->pExtEnv, sal_True ))->aCpp2Uno;
            ::uno_registerMapping(
                &pMapping, cppu_Bridge_free,
                (uno_Environment *)pFrom->pExtEnv,
                (uno_Environment *)pTo->pExtEnv, 0 );
        }
        else if (0 == rtl_ustr_ascii_compare( pTo->pTypeName->buffer, CPPU_CURRENT_LANGUAGE_BINDING_NAME ) &&
                 0 == rtl_ustr_ascii_compare( pFrom->pTypeName->buffer, UNO_LB_UNO ))
        {
            // ref count initially 1
            pMapping = &(new cppu_Bridge( pTo->pExtEnv, pFrom->pExtEnv, sal_False ))->aUno2Cpp;
            ::uno_registerMapping(
                &pMapping, cppu_Bridge_free,
                (uno_Environment *)pFrom->pExtEnv,
                (uno_Environment *)pTo->pExtEnv, 0 );
        }

        if (*ppMapping)
        {
            (*(*ppMapping)->release)( *ppMapping );
        }
        if (pMapping)
        *ppMapping = pMapping;
    }
}


//##################################################################################################
//##################################################################################################
//##################################################################################################

#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500))
static ::rtl::OUString * s_pStaticOidPart = 0;
#endif

// environment init stuff
//--------------------------------------------------------------------------------------------------
inline const ::rtl::OUString & SAL_CALL cppu_cppenv_getStaticOIdPart() SAL_THROW(())
{
#if ! (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500))
    static ::rtl::OUString * s_pStaticOidPart = 0;
#endif
    if (! s_pStaticOidPart)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! s_pStaticOidPart)
        {
            ::rtl::OUStringBuffer aRet( 64 );
            aRet.appendAscii( RTL_CONSTASCII_STRINGPARAM("];") );
            // good guid
            sal_uInt8 ar[16];
            ::rtl_getGlobalProcessId( ar );
            for ( sal_Int32 i = 0; i < 16; ++i )
            {
                aRet.append( (sal_Int32)ar[i], 16 );
            }
#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500))
            s_pStaticOidPart = new ::rtl::OUString( aRet.makeStringAndClear() );
#else
            static ::rtl::OUString s_aStaticOidPart( aRet.makeStringAndClear() );
            s_pStaticOidPart = &s_aStaticOidPart;
#endif
        }
    }
    return *s_pStaticOidPart;
}
// functions set at environment init
//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_cppenv_computeObjectIdentifier(
    uno_ExtEnvironment * pEnv, rtl_uString ** ppOId, void * pInterface ) SAL_THROW(())
{
    OSL_ENSURE( pEnv && ppOId && pInterface, "### null ptr!" );
    if (pEnv && ppOId && pInterface)
    {
        if (*ppOId)
        {
            rtl_uString_release( *ppOId );
            *ppOId = 0;
        }

        try
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xHome(
                reinterpret_cast< ::com::sun::star::uno::XInterface * >( pInterface ),
                ::com::sun::star::uno::UNO_QUERY );
            OSL_ENSURE( xHome.is(), "### query to XInterface failed!" );
            if (xHome.is())
            {
                // interface
                ::rtl::OUStringBuffer oid( 64 );
                oid.append( (sal_Int64)xHome.get(), 16 );
                oid.append( (sal_Unicode)';' );
                // ;environment[context]
                oid.append(
                    *reinterpret_cast< ::rtl::OUString const * >(
                        &((uno_Environment *) pEnv)->pTypeName ) );
                oid.append( (sal_Unicode)'[' );
                oid.append( (sal_Int64)((uno_Environment *)pEnv)->pContext, 16 );
                // ];good guid
                oid.append( cppu_cppenv_getStaticOIdPart() );
                ::rtl::OUString aRet( oid.makeStringAndClear() );
                ::rtl_uString_acquire( *ppOId = aRet.pData );
            }
        }
        catch (const ::com::sun::star::uno::RuntimeException &)
        {
            OSL_FAIL( "### RuntimeException occurred during queryInterface()!" );
        }
    }
}
//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_cppenv_acquireInterface( uno_ExtEnvironment *, void * pCppI ) SAL_THROW(())
{
    reinterpret_cast< ::com::sun::star::uno::XInterface * >( pCppI )->acquire();
}
//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_cppenv_releaseInterface( uno_ExtEnvironment *, void * pCppI ) SAL_THROW(())
{
    reinterpret_cast< ::com::sun::star::uno::XInterface * >( pCppI )->release();
}
//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_cppenv_environmentDisposing( uno_Environment * ) SAL_THROW(())
{
    g_moduleCount.modCnt.release( &g_moduleCount.modCnt );
}
//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_cppenv_initEnvironment( uno_Environment * pCppEnv ) SAL_THROW(())
{
    OSL_ENSURE( pCppEnv->pExtEnv, "### expected extended environment!" );
    OSL_ENSURE( ::rtl_ustr_ascii_compare( pCppEnv->pTypeName->buffer, CPPU_CURRENT_LANGUAGE_BINDING_NAME ) == 0, "### wrong environment type!" );
    g_moduleCount.modCnt.acquire( &g_moduleCount.modCnt );
    ((uno_ExtEnvironment *)pCppEnv)->computeObjectIdentifier = cppu_cppenv_computeObjectIdentifier;
    ((uno_ExtEnvironment *)pCppEnv)->acquireInterface = cppu_cppenv_acquireInterface;
    ((uno_ExtEnvironment *)pCppEnv)->releaseInterface = cppu_cppenv_releaseInterface;
    pCppEnv->environmentDisposing = cppu_cppenv_environmentDisposing;
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
