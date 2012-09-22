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


#include "bridges/cpp_uno/shared/unointerfaceproxy.hxx"

#include "bridges/cpp_uno/shared/bridge.hxx"

#include "com/sun/star/uno/XInterface.hpp"
#include "osl/diagnose.h"
#include "osl/interlck.h"
#include "typelib/typedescription.h"
#include "uno/dispatcher.h"

namespace bridges { namespace cpp_uno { namespace shared {

void freeUnoInterfaceProxy(uno_ExtEnvironment * pEnv, void * pProxy)
{
    UnoInterfaceProxy * pThis =
        static_cast< UnoInterfaceProxy * >(
            reinterpret_cast< uno_Interface * >( pProxy ) );
    if (pEnv != pThis->pBridge->getUnoEnv()) {
        OSL_ASSERT(false);
    }

    (*pThis->pBridge->getCppEnv()->revokeInterface)(
        pThis->pBridge->getCppEnv(), pThis->pCppI );
    pThis->pCppI->release();
    ::typelib_typedescription_release(
        (typelib_TypeDescription *)pThis->pTypeDescr );
    pThis->pBridge->release();

#if OSL_DEBUG_LEVEL > 1
    *(int *)pProxy = 0xdeadbabe;
#endif
    delete pThis;
}

void acquireProxy(uno_Interface * pUnoI)
{
    if (1 == osl_atomic_increment(
            & static_cast< UnoInterfaceProxy * >( pUnoI )->nRef ))
    {
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
        OSL_ASSERT( pThis == pUnoI );
#endif
    }
}

void releaseProxy(uno_Interface * pUnoI)
{
    if (! osl_atomic_decrement(
            & static_cast< UnoInterfaceProxy * >( pUnoI )->nRef ))
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
    rtl::OUString const & rOId) SAL_THROW(())
{
    return new UnoInterfaceProxy(pBridge, pCppI, pTypeDescr, rOId);
}

UnoInterfaceProxy::UnoInterfaceProxy(
    bridges::cpp_uno::shared::Bridge * pBridge_,
    com::sun::star::uno::XInterface * pCppI_,
    typelib_InterfaceTypeDescription * pTypeDescr_, rtl::OUString const & rOId_)
    SAL_THROW(())
    : nRef( 1 )
    , pBridge( pBridge_ )
    , pCppI( pCppI_ )
    , pTypeDescr( pTypeDescr_ )
    , oid( rOId_ )
{
    pBridge->acquire();
    ::typelib_typedescription_acquire( (typelib_TypeDescription *)pTypeDescr );
    if (! ((typelib_TypeDescription *)pTypeDescr)->bComplete)
        ::typelib_typedescription_complete(
            (typelib_TypeDescription **)&pTypeDescr );
    OSL_ENSURE(
        ((typelib_TypeDescription *)pTypeDescr)->bComplete,
        "### type is incomplete!" );
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

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
