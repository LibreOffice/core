/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cppinterfaceproxy.cxx,v $
 * $Revision: 1.8 $
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
#include "precompiled_bridges.hxx"

#include "bridges/cpp_uno/shared/cppinterfaceproxy.hxx"

#include "guardedarray.hxx"

#include "bridges/cpp_uno/shared/bridge.hxx"
#include "bridges/cpp_uno/shared/vtablefactory.hxx"

#include "com/sun/star/uno/XInterface.hpp"
#include "osl/diagnose.h"
#include "osl/getglobalmutex.hxx"
#include "osl/interlck.h"
#include "osl/mutex.hxx"
#include "rtl/instance.hxx"
#include "typelib/typedescription.h"

#include <cstddef>
#include <new>


static bridges::cpp_uno::shared::VtableFactory * pInstance;

#ifdef __GNUG__
void dso_init(void) __attribute__((constructor));
void dso_exit(void) __attribute__((destructor));
#endif

void dso_init(void) {
    if (!pInstance)
        pInstance = new bridges::cpp_uno::shared::VtableFactory();
}

void dso_exit(void) {
    if (pInstance)
    {
        delete pInstance;
        pInstance = NULL;
    }
}

#ifdef __SUNPRO_CC
# pragma init(dso_init)
# pragma fini(dso_exit)
#endif



namespace {

struct InitVtableFactory {
    bridges::cpp_uno::shared::VtableFactory * operator()() {
        return pInstance;
    }
};

bridges::cpp_uno::shared::VtableFactory * getVtableFactory() {
    return rtl_Instance<
        bridges::cpp_uno::shared::VtableFactory, InitVtableFactory,
        osl::MutexGuard, osl::GetGlobalMutex >::create(
            InitVtableFactory(), osl::GetGlobalMutex());
}

}

namespace bridges { namespace cpp_uno { namespace shared {

void freeCppInterfaceProxy(uno_ExtEnvironment * pEnv, void * pInterface)
{
    CppInterfaceProxy * pThis = CppInterfaceProxy::castInterfaceToProxy(
        pInterface);
    if (pEnv != pThis->pBridge->getCppEnv()) {
        OSL_ASSERT(false);
    }

    (*pThis->pBridge->getUnoEnv()->revokeInterface)(
        pThis->pBridge->getUnoEnv(), pThis->pUnoI );
    (*pThis->pUnoI->release)( pThis->pUnoI );
    ::typelib_typedescription_release(
        (typelib_TypeDescription *)pThis->pTypeDescr );
    pThis->pBridge->release();

#if OSL_DEBUG_LEVEL > 1
    *(int *)pInterface = 0xdeadbabe;
#endif
    pThis->~CppInterfaceProxy();
    delete[] reinterpret_cast< char * >(pThis);
}

com::sun::star::uno::XInterface * CppInterfaceProxy::create(
    bridges::cpp_uno::shared::Bridge * pBridge, uno_Interface * pUnoI,
    typelib_InterfaceTypeDescription * pTypeDescr, rtl::OUString const & rOId)
    SAL_THROW(())
{
    typelib_typedescription_complete(
        reinterpret_cast< typelib_TypeDescription ** >(&pTypeDescr));
    bridges::cpp_uno::shared::VtableFactory::Vtables aVtables(
        getVtableFactory()->getVtables(pTypeDescr));
    bridges::cpp_uno::shared::GuardedArray< char > pMemory(
        new char[
            sizeof (CppInterfaceProxy)
            + (aVtables.count - 1) * sizeof (void **)]);
    new(pMemory.get()) CppInterfaceProxy(pBridge, pUnoI, pTypeDescr, rOId);
    CppInterfaceProxy * pProxy = reinterpret_cast< CppInterfaceProxy * >(
        pMemory.release());
    for (sal_Int32 i = 0; i < aVtables.count; ++i) {
        pProxy->vtables[i] = VtableFactory::mapBlockToVtable(
            aVtables.blocks[i].start);
    }
    return castProxyToInterface(pProxy);
}

void CppInterfaceProxy::acquireProxy() SAL_THROW(())
{
    if (1 == osl_incrementInterlockedCount( &nRef ))
    {
        // rebirth of proxy zombie
        // register at cpp env
        void * pThis = castProxyToInterface( this );
        (*pBridge->getCppEnv()->registerProxyInterface)(
            pBridge->getCppEnv(), &pThis, freeCppInterfaceProxy, oid.pData,
            pTypeDescr );
        OSL_ASSERT( pThis == castProxyToInterface( this ) );
    }
}

void CppInterfaceProxy::releaseProxy() SAL_THROW(())
{
    if (! osl_decrementInterlockedCount( &nRef )) // last release
    {
        // revoke from cpp env
        (*pBridge->getCppEnv()->revokeInterface)(
            pBridge->getCppEnv(), castProxyToInterface( this ) );
    }
}

CppInterfaceProxy::CppInterfaceProxy(
    bridges::cpp_uno::shared::Bridge * pBridge_, uno_Interface * pUnoI_,
    typelib_InterfaceTypeDescription * pTypeDescr_, rtl::OUString const & rOId_)
    SAL_THROW(())
    : nRef( 1 )
    , pBridge( pBridge_ )
    , pUnoI( pUnoI_ )
    , pTypeDescr( pTypeDescr_ )
    , oid( rOId_ )
{
    pBridge->acquire();
    ::typelib_typedescription_acquire( (typelib_TypeDescription *)pTypeDescr );
    (*pUnoI->acquire)( pUnoI );
    (*pBridge->getUnoEnv()->registerInterface)(
        pBridge->getUnoEnv(), reinterpret_cast< void ** >( &pUnoI ), oid.pData,
        pTypeDescr );
}

CppInterfaceProxy::~CppInterfaceProxy()
{}

com::sun::star::uno::XInterface * CppInterfaceProxy::castProxyToInterface(
    CppInterfaceProxy * pProxy)
{
    return reinterpret_cast< com::sun::star::uno::XInterface * >(
        &pProxy->vtables);
}

CppInterfaceProxy * CppInterfaceProxy::castInterfaceToProxy(void * pInterface)
{
    // pInterface == &pProxy->vtables (this emulated offsetof is not truly
    // portable):
    char const * const base = reinterpret_cast< char const * >(16);
    std::ptrdiff_t const offset = reinterpret_cast< char const * >(
        &reinterpret_cast< CppInterfaceProxy const * >(base)->vtables) - base;
    return reinterpret_cast< CppInterfaceProxy * >(
        static_cast< char * >(pInterface) - offset);
}

} } }
