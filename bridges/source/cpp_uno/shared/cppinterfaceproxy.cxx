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

#include <cppinterfaceproxy.hxx>

#include <bridge.hxx>
#include <utility>
#include <vtablefactory.hxx>

#include <com/sun/star/uno/XInterface.hpp>
#include <typelib/typedescription.h>

#include <cstddef>
#include <memory>
#include <new>

namespace bridges::cpp_uno::shared {

void freeCppInterfaceProxy(uno_ExtEnvironment * pEnv, void * pInterface)
{
    CppInterfaceProxy * pThis = CppInterfaceProxy::castInterfaceToProxy(
        pInterface);
    if (pEnv != pThis->pBridge->getCppEnv()) {
        assert(false);
    }

    (*pThis->pBridge->getUnoEnv()->revokeInterface)(
        pThis->pBridge->getUnoEnv(), pThis->pUnoI );
    (*pThis->pUnoI->release)( pThis->pUnoI );
    ::typelib_typedescription_release(
        &pThis->pTypeDescr->aBase );
    pThis->pBridge->release();

#if OSL_DEBUG_LEVEL > 1
    *(int *)pInterface = 0xdeadbabe;
#endif
    pThis->~CppInterfaceProxy();
    delete[] reinterpret_cast< char * >(pThis);
}

com::sun::star::uno::XInterface * CppInterfaceProxy::create(
    bridges::cpp_uno::shared::Bridge * pBridge, uno_Interface * pUnoI,
    typelib_InterfaceTypeDescription * pTypeDescr, OUString const & rOId)
{
    typelib_typedescription_complete(
        reinterpret_cast< typelib_TypeDescription ** >(&pTypeDescr));
    static bridges::cpp_uno::shared::VtableFactory factory;
    const bridges::cpp_uno::shared::VtableFactory::Vtables& rVtables(
        factory.getVtables(pTypeDescr));
    std::unique_ptr< char[] > pMemory(
        new char[
            sizeof (CppInterfaceProxy)
            + (rVtables.count - 1) * sizeof (void **)]);
    new(pMemory.get()) CppInterfaceProxy(pBridge, pUnoI, pTypeDescr, rOId);
    CppInterfaceProxy * pProxy = reinterpret_cast< CppInterfaceProxy * >(
        pMemory.release());
    for (sal_Int32 i = 0; i < rVtables.count; ++i) {
        pProxy->vtables[i] = VtableFactory::mapBlockToVtable(
            rVtables.blocks[i].start);
    }
    return castProxyToInterface(pProxy);
}

void CppInterfaceProxy::acquireProxy()
{
    if (++nRef == 1)
    {
        // rebirth of proxy zombie
        // register at cpp env
        void * pThis = castProxyToInterface( this );
        (*pBridge->getCppEnv()->registerProxyInterface)(
            pBridge->getCppEnv(), &pThis, freeCppInterfaceProxy, oid.pData,
            pTypeDescr );
        assert(pThis == castProxyToInterface(this));
    }
}

void CppInterfaceProxy::releaseProxy()
{
    if (! --nRef ) // last release
    {
        // revoke from cpp env
        (*pBridge->getCppEnv()->revokeInterface)(
            pBridge->getCppEnv(), castProxyToInterface( this ) );
    }
}

CppInterfaceProxy::CppInterfaceProxy(
    bridges::cpp_uno::shared::Bridge * pBridge_, uno_Interface * pUnoI_,
    typelib_InterfaceTypeDescription * pTypeDescr_, OUString aOId_)
    : nRef( 1 )
    , pBridge( pBridge_ )
    , pUnoI( pUnoI_ )
    , pTypeDescr( pTypeDescr_ )
    , oid(std::move( aOId_ ))
{
    pBridge->acquire();
    ::typelib_typedescription_acquire( &pTypeDescr->aBase );
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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
