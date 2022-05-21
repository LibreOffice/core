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

#include <unointerfaceproxy.hxx>

#include <bridge.hxx>

#include <com/sun/star/uno/XInterface.hpp>
#include <utility>
#include <typelib/typedescription.h>
#include <uno/dispatcher.h>

namespace bridges::cpp_uno::shared {

void freeUnoInterfaceProxy(uno_ExtEnvironment * pEnv, void * pProxy)
{
    UnoInterfaceProxy * pThis =
        static_cast< UnoInterfaceProxy * >(
            static_cast< uno_Interface * >( pProxy ) );
    if (pEnv != pThis->pBridge->getUnoEnv()) {
        assert(false);
    }

    (*pThis->pBridge->getCppEnv()->revokeInterface)(
        pThis->pBridge->getCppEnv(), pThis->pCppI );
    pThis->pCppI->release();
    ::typelib_typedescription_release(&pThis->pTypeDescr->aBase);
    pThis->pBridge->release();

#if OSL_DEBUG_LEVEL > 1
    *(int *)pProxy = 0xdeadbabe;
#endif
    delete pThis;
}

void acquireProxy(uno_Interface * pUnoI)
{
    if (++static_cast< UnoInterfaceProxy * >( pUnoI )->nRef != 1)
        return;

    // rebirth of proxy zombie
    // register at uno env
#if OSL_DEBUG_LEVEL > 1
    void * pThis = pUnoI;
#endif
    (*static_cast< UnoInterfaceProxy * >( pUnoI )->pBridge->getUnoEnv()->
     registerProxyInterface)(
         static_cast< UnoInterfaceProxy * >( pUnoI )->pBridge->getUnoEnv(),
         reinterpret_cast< void ** >( &pUnoI ), freeUnoInterfaceProxy,
         static_cast< UnoInterfaceProxy * >( pUnoI )->oid.pData,
         static_cast< UnoInterfaceProxy * >( pUnoI )->pTypeDescr );
#if OSL_DEBUG_LEVEL > 1
    assert(pThis == pUnoI);
#endif
}

void releaseProxy(uno_Interface * pUnoI)
{
    if (! --static_cast< UnoInterfaceProxy * >( pUnoI )->nRef )
    {
        // revoke from uno env on last release
        (*static_cast< UnoInterfaceProxy * >( pUnoI )->pBridge->getUnoEnv()->
         revokeInterface)(
             static_cast< UnoInterfaceProxy * >( pUnoI )->pBridge->getUnoEnv(),
             pUnoI );
    }
}

UnoInterfaceProxy * UnoInterfaceProxy::create(
    bridges::cpp_uno::shared::Bridge * pBridge,
    com::sun::star::uno::XInterface * pCppI,
    typelib_InterfaceTypeDescription * pTypeDescr,
    OUString const & rOId)
{
    return new UnoInterfaceProxy(pBridge, pCppI, pTypeDescr, rOId);
}

UnoInterfaceProxy::UnoInterfaceProxy(
    bridges::cpp_uno::shared::Bridge * pBridge_,
    com::sun::star::uno::XInterface * pCppI_,
    typelib_InterfaceTypeDescription * pTypeDescr_, OUString aOId_)
    : nRef( 1 )
    , pBridge( pBridge_ )
    , pCppI( pCppI_ )
    , pTypeDescr( pTypeDescr_ )
    , oid(std::move( aOId_ ))
{
    pBridge->acquire();
    ::typelib_typedescription_acquire(&pTypeDescr->aBase);
    if (!pTypeDescr->aBase.bComplete)
        ::typelib_typedescription_complete(
            reinterpret_cast<typelib_TypeDescription **>(&pTypeDescr));
    assert(pTypeDescr->aBase.bComplete);
    pCppI->acquire();
    (*pBridge->getCppEnv()->registerInterface)(
        pBridge->getCppEnv(), reinterpret_cast< void ** >( &pCppI ), oid.pData,
        pTypeDescr );

    // uno_Interface
    acquire = acquireProxy;
    release = releaseProxy;
    pDispatcher = unoInterfaceProxyDispatch;
}

UnoInterfaceProxy::~UnoInterfaceProxy()
{}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
