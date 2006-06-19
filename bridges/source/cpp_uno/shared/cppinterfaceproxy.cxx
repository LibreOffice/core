/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cppinterfaceproxy.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:45:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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

namespace {

struct InitVtableFactory {
    bridges::cpp_uno::shared::VtableFactory * operator()() {
        static bridges::cpp_uno::shared::VtableFactory instance;
        return &instance;
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
