/*************************************************************************
 *
 *  $RCSfile: bridge.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-12 14:36:58 $
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
#ifndef _BRIDGES_CPP_UNO_BRIDGE_HXX_
#define _BRIDGES_CPP_UNO_BRIDGE_HXX_

#ifndef _BRIDGES_CPP_UNO_BRIDGE_H_
#include <bridges/cpp_uno/bridge.h>
#endif

#ifndef _OSL_PROCESS_H_
#include <osl/process.h>
#endif
#ifndef _RTL_PROCESS_H_
#include <rtl/process.h>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_GENFUNC_HXX_
#include <com/sun/star/uno/genfunc.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif


namespace CPPU_CURRENT_NAMESPACE
{
extern "C"
{

//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_cppInterfaceProxy_free( uno_ExtEnvironment * pEnv, void * pProxy ) throw ()
{
    cppu_cppInterfaceProxy * pThis =
        static_cast< cppu_cppInterfaceProxy * >(
            reinterpret_cast< ::com::sun::star::uno::XInterface * >( pProxy ) );
    OSL_ASSERT( pEnv == pThis->pBridge->pCppEnv );

    (*pThis->pBridge->pUnoEnv->revokeInterface)( pThis->pBridge->pUnoEnv, pThis->pUnoI );
    (*pThis->pUnoI->release)( pThis->pUnoI );
    ::typelib_typedescription_release( (typelib_TypeDescription *)pThis->pTypeDescr );
    pThis->pBridge->release();

#ifdef DEBUG
    *(int *)pProxy = 0xdeadbabe;
#endif
    delete pThis;
}
//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_Mapping_uno2cpp(
    uno_Mapping * pMapping, void ** ppCppI,
    void * pUnoI, typelib_InterfaceTypeDescription * pTypeDescr ) throw ()
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
                cppu_cppInterfaceProxy_free, pOId, pTypeDescr );

            *ppCppI = pSurrogate;
        }
        ::rtl_uString_release( pOId );
    }
}
//__________________________________________________________________________________________________
inline void cppu_cppInterfaceProxy::acquireProxy() throw ()
{
    if (1 == osl_incrementInterlockedCount( &nRef ))
    {
        // rebirth of proxy zombie
        // register at cpp env
        void * pThis = static_cast< ::com::sun::star::uno::XInterface * >( this );
        (*pBridge->pCppEnv->registerProxyInterface)(
            pBridge->pCppEnv, &pThis, cppu_cppInterfaceProxy_free, oid.pData, pTypeDescr );
        OSL_ASSERT( pThis == static_cast< ::com::sun::star::uno::XInterface * >( this ) );
    }
}
//__________________________________________________________________________________________________
inline void cppu_cppInterfaceProxy::releaseProxy() throw ()
{
    if (! osl_decrementInterlockedCount( &nRef )) // last release
    {
        // revoke from cpp env
        (*pBridge->pCppEnv->revokeInterface)(
            pBridge->pCppEnv, static_cast< ::com::sun::star::uno::XInterface * >( this ) );
    }
}
//__________________________________________________________________________________________________
inline cppu_cppInterfaceProxy::cppu_cppInterfaceProxy(
    cppu_Bridge * pBridge_, uno_Interface * pUnoI_,
    typelib_InterfaceTypeDescription * pTypeDescr_, const ::rtl::OUString & rOId_ ) throw ()
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
    (*pBridge->pUnoEnv->registerInterface)(
        pBridge->pUnoEnv, reinterpret_cast< void ** >( &pUnoI ), oid.pData, pTypeDescr );
    (*pUnoI->acquire)( pUnoI );
}


//##################################################################################################
//##################################################################################################
//##################################################################################################


//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_unoInterfaceProxy_free( uno_ExtEnvironment * pEnv, void * pProxy ) throw ()
{
    cppu_unoInterfaceProxy * pThis =
        static_cast< cppu_unoInterfaceProxy * >(
            reinterpret_cast< uno_Interface * >( pProxy ) );
    OSL_ASSERT( pEnv == pThis->pBridge->pUnoEnv );

    (*pThis->pBridge->pCppEnv->revokeInterface)( pThis->pBridge->pCppEnv, pThis->pCppI );
    pThis->pCppI->release();
    ::typelib_typedescription_release( (typelib_TypeDescription *)pThis->pTypeDescr );
    pThis->pBridge->release();

#ifdef DEBUG
    *(int *)pProxy = 0xdeadbabe;
#endif
    delete pThis;
}
//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_unoInterfaceProxy_acquire( uno_Interface * pUnoI ) throw ()
{
    if (1 == osl_incrementInterlockedCount( & static_cast< cppu_unoInterfaceProxy * >( pUnoI )->nRef ))
    {
        // rebirth of proxy zombie
        // register at uno env
#ifdef DEBUG
        void * pThis = pUnoI;
#endif
        (*static_cast< cppu_unoInterfaceProxy * >( pUnoI )->pBridge->pUnoEnv->registerProxyInterface)(
            static_cast< cppu_unoInterfaceProxy * >( pUnoI )->pBridge->pUnoEnv,
            reinterpret_cast< void ** >( &pUnoI ), cppu_unoInterfaceProxy_free,
            static_cast< cppu_unoInterfaceProxy * >( pUnoI )->oid.pData,
            static_cast< cppu_unoInterfaceProxy * >( pUnoI )->pTypeDescr );
#ifdef DEBUG
        OSL_ASSERT( pThis == pUnoI );
#endif
    }
}
//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_unoInterfaceProxy_release( uno_Interface * pUnoI ) throw ()
{
    if (! osl_decrementInterlockedCount( & static_cast< cppu_unoInterfaceProxy * >( pUnoI )->nRef ))
    {
        // revoke from uno env on last release
        (*static_cast< cppu_unoInterfaceProxy * >( pUnoI )->pBridge->pUnoEnv->revokeInterface)(
            static_cast< cppu_unoInterfaceProxy * >( pUnoI )->pBridge->pUnoEnv, pUnoI );
    }
}
//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_Mapping_cpp2uno(
    uno_Mapping * pMapping, void ** ppUnoI,
    void * pCppI, typelib_InterfaceTypeDescription * pTypeDescr ) throw ()
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
                cppu_unoInterfaceProxy_free, pOId, pTypeDescr );

            *ppUnoI = pSurrogate;
        }
        ::rtl_uString_release( pOId );
    }
}
//__________________________________________________________________________________________________
inline cppu_unoInterfaceProxy::cppu_unoInterfaceProxy(
    cppu_Bridge * pBridge_, ::com::sun::star::uno::XInterface * pCppI_,
    typelib_InterfaceTypeDescription * pTypeDescr_, const ::rtl::OUString & rOId_ ) throw ()
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
    (*pBridge->pCppEnv->registerInterface)(
        pBridge->pCppEnv, reinterpret_cast< void ** >( &pCppI ), oid.pData, pTypeDescr );
    pCppI->acquire();

    // uno_Interface
    uno_Interface::acquire = CPPU_CURRENT_NAMESPACE::cppu_unoInterfaceProxy_acquire;
    uno_Interface::release = CPPU_CURRENT_NAMESPACE::cppu_unoInterfaceProxy_release;
    uno_Interface::pDispatcher = CPPU_CURRENT_NAMESPACE::cppu_unoInterfaceProxy_dispatch;
}


//##################################################################################################
//##################################################################################################
//##################################################################################################


//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_Mapping_acquire( uno_Mapping * pMapping ) throw ()
{
    static_cast< cppu_Mapping * >( pMapping )->pBridge->acquire();
}
//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_Mapping_release( uno_Mapping * pMapping ) throw ()
{
    static_cast< cppu_Mapping * >( pMapping )->pBridge->release();
}
//__________________________________________________________________________________________________
inline cppu_Mapping::cppu_Mapping( cppu_Bridge * pBridge_, uno_MapInterfaceFunc fpMap ) throw ()
    : pBridge( pBridge_ )
{
    uno_Mapping::acquire = cppu_Mapping_acquire;
    uno_Mapping::release = cppu_Mapping_release;
    uno_Mapping::mapInterface = fpMap;
}
//__________________________________________________________________________________________________
inline cppu_Bridge::cppu_Bridge(
    uno_ExtEnvironment * pCppEnv_, uno_ExtEnvironment * pUnoEnv_,
    sal_Bool bExportCpp2Uno_ ) throw ()
    : nRef( 1 )
    , pCppEnv( pCppEnv_ )
    , pUnoEnv( pUnoEnv_ )
    , aCpp2Uno( this, cppu_Mapping_cpp2uno )
    , aUno2Cpp( this, cppu_Mapping_uno2cpp )
    , bExportCpp2Uno( bExportCpp2Uno_ )
{
    (*((uno_Environment *)pCppEnv)->acquire)( (uno_Environment *)pCppEnv );
    (*((uno_Environment *)pUnoEnv)->acquire)( (uno_Environment *)pUnoEnv );
}
//__________________________________________________________________________________________________
inline void SAL_CALL cppu_Bridge_free( uno_Mapping * pMapping ) throw ()
{
    cppu_Bridge * pThis = static_cast< cppu_Mapping * >( pMapping )->pBridge;
    (*((uno_Environment *)pThis->pUnoEnv)->release)( (uno_Environment *)pThis->pUnoEnv );
    (*((uno_Environment *)pThis->pCppEnv)->release)( (uno_Environment *)pThis->pCppEnv );
    delete pThis;
}
//__________________________________________________________________________________________________
inline void cppu_Bridge::acquire() throw ()
{
    if (1 == osl_incrementInterlockedCount( &nRef ))
    {
        if (bExportCpp2Uno)
        {
            uno_Mapping * pMapping = &aCpp2Uno;
            uno_registerMapping( &pMapping, cppu_Bridge_free,
                                 (uno_Environment *)pCppEnv, (uno_Environment *)pUnoEnv, 0 );
        }
        else
        {
            uno_Mapping * pMapping = &aUno2Cpp;
            uno_registerMapping( &pMapping, cppu_Bridge_free,
                                 (uno_Environment *)pUnoEnv, (uno_Environment *)pCppEnv, 0 );
        }
    }
}
//__________________________________________________________________________________________________
inline void cppu_Bridge::release() throw ()
{
    if (! osl_decrementInterlockedCount( &nRef ))
    {
        uno_revokeMapping( bExportCpp2Uno ? &aCpp2Uno : &aUno2Cpp );
    }
}

//##################################################################################################
inline void SAL_CALL cppu_ext_getMapping(
    uno_Mapping ** ppMapping, uno_Environment * pFrom, uno_Environment * pTo ) throw ()
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
            ::uno_registerMapping( &pMapping, cppu_Bridge_free,
                                   (uno_Environment *)pFrom->pExtEnv,
                                   (uno_Environment *)pTo->pExtEnv, 0 );
        }
        if (0 == rtl_ustr_ascii_compare( pTo->pTypeName->buffer, CPPU_CURRENT_LANGUAGE_BINDING_NAME ) &&
            0 == rtl_ustr_ascii_compare( pFrom->pTypeName->buffer, UNO_LB_UNO ))
        {
            // ref count initially 1
            pMapping = &(new cppu_Bridge( pTo->pExtEnv, pFrom->pExtEnv, sal_False ))->aUno2Cpp;
            ::uno_registerMapping( &pMapping, cppu_Bridge_free,
                                   (uno_Environment *)pFrom->pExtEnv,
                                   (uno_Environment *)pTo->pExtEnv, 0 );
        }

        if (*ppMapping)
            (*(*ppMapping)->release)( *ppMapping );
        *ppMapping = pMapping;
    }
}


//##################################################################################################
//##################################################################################################
//##################################################################################################

#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__))
static ::rtl::OUString * s_pStaticOidPart = 0;
#endif

// environment init stuff
//--------------------------------------------------------------------------------------------------
inline const ::rtl::OUString & SAL_CALL cppu_cppenv_getStaticOIdPart() throw ()
{
#if ! ((defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__)))
    static ::rtl::OUString * s_pStaticOidPart = 0;
#endif
    if (! s_pStaticOidPart)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! s_pStaticOidPart)
        {
            ::rtl::OUStringBuffer aRet( 64 );
            aRet.appendAscii( RTL_CONSTASCII_STRINGPARAM("];") );
            // pid
            oslProcessInfo info;
            info.Size = sizeof(oslProcessInfo);
            if (::osl_getProcessInfo( 0, osl_Process_IDENTIFIER, &info ) == osl_Process_E_None)
            {
                aRet.append( (sal_Int64)info.Ident, 16 );
            }
            else
            {
                aRet.appendAscii( RTL_CONSTASCII_STRINGPARAM("unknown process id") );
            }
            // good guid
            sal_uInt8 ar[16];
            ::rtl_getGlobalProcessId( ar );
            aRet.append( (sal_Unicode)';' );
            for ( sal_Int32 i = 0; i < 16; ++i )
            {
                aRet.append( (sal_Int32)ar[i], 16 );
            }
#if (defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)) || (defined(__GNUC__) && defined(__APPLE__))
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
    uno_ExtEnvironment * pEnv, rtl_uString ** ppOId, void * pInterface ) throw ()
{
    OSL_ENSURE( pEnv && ppOId && pInterface, "### null ptr!" );
    if (pEnv && ppOId && pInterface)
    {
        if (*ppOId)
        {
            rtl_uString_release( *ppOId );
            *ppOId = 0;
        }

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
            // environment[context]
            oid.append( ((uno_Environment *)pEnv)->pTypeName );
            oid.append( (sal_Unicode)'[' );
            oid.append( (sal_Int64)((uno_Environment *)pEnv)->pContext, 16 );
            // process;good guid
            oid.append( cppu_cppenv_getStaticOIdPart() );
            ::rtl::OUString aRet( oid.makeStringAndClear() );
            ::rtl_uString_acquire( *ppOId = aRet.pData );
        }
    }
}
//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_cppenv_acquireInterface( uno_ExtEnvironment *, void * pCppI ) throw ()
{
    reinterpret_cast< ::com::sun::star::uno::XInterface * >( pCppI )->acquire();
}
//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_cppenv_releaseInterface( uno_ExtEnvironment *, void * pCppI ) throw ()
{
    reinterpret_cast< ::com::sun::star::uno::XInterface * >( pCppI )->release();
}
//--------------------------------------------------------------------------------------------------
inline void SAL_CALL cppu_cppenv_initEnvironment( uno_Environment * pCppEnv ) throw ()
{
    OSL_ENSURE( pCppEnv->pExtEnv, "### expected extended environment!" );
    OSL_ENSURE( rtl_ustr_ascii_compare( pCppEnv->pTypeName->buffer, CPPU_CURRENT_LANGUAGE_BINDING_NAME ) == 0,
                 "### wrong environment type!" );
    ((uno_ExtEnvironment *)pCppEnv)->computeObjectIdentifier = CPPU_CURRENT_NAMESPACE::cppu_cppenv_computeObjectIdentifier;
    ((uno_ExtEnvironment *)pCppEnv)->acquireInterface        = CPPU_CURRENT_NAMESPACE::cppu_cppenv_acquireInterface;
    ((uno_ExtEnvironment *)pCppEnv)->releaseInterface        = CPPU_CURRENT_NAMESPACE::cppu_cppenv_releaseInterface;
}

}
}

#endif
