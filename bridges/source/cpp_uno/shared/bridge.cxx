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


#include "bridges/cpp_uno/shared/bridge.hxx"

#include "component.hxx"

#include "bridges/cpp_uno/shared/cppinterfaceproxy.hxx"
#include "bridges/cpp_uno/shared/unointerfaceproxy.hxx"

#include "com/sun/star/uno/XInterface.hpp"
#include "osl/diagnose.h"
#include "osl/interlck.h"
#include "rtl/ustring.h"
#include "sal/types.h"
#include "typelib/typedescription.h"
#include "uno/dispatcher.h"
#include "uno/environment.h"
#include "uno/mapping.h"

namespace bridges { namespace cpp_uno { namespace shared {

void freeMapping(uno_Mapping * pMapping)
{
    delete static_cast< Bridge::Mapping * >( pMapping )->pBridge;
}

void acquireMapping(uno_Mapping * pMapping)
{
    static_cast< Bridge::Mapping * >( pMapping )->pBridge->acquire();
}

void releaseMapping(uno_Mapping * pMapping)
{
    static_cast< Bridge::Mapping * >( pMapping )->pBridge->release();
}

void cpp2unoMapping(
    uno_Mapping * pMapping, void ** ppUnoI, void * pCppI,
    typelib_InterfaceTypeDescription * pTypeDescr)
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
        Bridge * pBridge = static_cast< Bridge::Mapping * >( pMapping )->pBridge;

        // get object id of interface to be wrapped
        rtl_uString * pOId = 0;
        (*pBridge->pCppEnv->getObjectIdentifier)(
            pBridge->pCppEnv, &pOId, pCppI );
        OSL_ASSERT( pOId );

        // try to get any known interface from target environment
        (*pBridge->pUnoEnv->getRegisteredInterface)(
            pBridge->pUnoEnv, ppUnoI, pOId, pTypeDescr );

        if (! *ppUnoI) // no existing interface, register new proxy interface
        {
            // try to publish a new proxy (refcount initially 1)
            uno_Interface * pSurrogate
                = bridges::cpp_uno::shared::UnoInterfaceProxy::create(
                    pBridge,
                    static_cast< ::com::sun::star::uno::XInterface * >( pCppI ),
                    pTypeDescr, pOId );

            // proxy may be exchanged during registration
            (*pBridge->pUnoEnv->registerProxyInterface)(
                pBridge->pUnoEnv, reinterpret_cast< void ** >( &pSurrogate ),
                freeUnoInterfaceProxy, pOId,
                pTypeDescr );

            *ppUnoI = pSurrogate;
        }
        ::rtl_uString_release( pOId );
    }
}

void uno2cppMapping(
    uno_Mapping * pMapping, void ** ppCppI, void * pUnoI,
    typelib_InterfaceTypeDescription * pTypeDescr)
{
    OSL_ASSERT( ppCppI && pTypeDescr );
    if (*ppCppI)
    {
        static_cast< ::com::sun::star::uno::XInterface * >( *ppCppI )->
            release();
        *ppCppI = 0;
    }
    if (pUnoI)
    {
        Bridge * pBridge = static_cast< Bridge::Mapping * >( pMapping )->pBridge;

        // get object id of uno interface to be wrapped
        rtl_uString * pOId = 0;
        (*pBridge->pUnoEnv->getObjectIdentifier)(
            pBridge->pUnoEnv, &pOId, pUnoI );
        OSL_ASSERT( pOId );

        // try to get any known interface from target environment
        (*pBridge->pCppEnv->getRegisteredInterface)(
            pBridge->pCppEnv, ppCppI, pOId, pTypeDescr );

        if (! *ppCppI) // no existing interface, register new proxy interface
        {
            // try to publish a new proxy (ref count initially 1)
            com::sun::star::uno::XInterface * pProxy
                = bridges::cpp_uno::shared::CppInterfaceProxy::create(
                    pBridge, static_cast< uno_Interface * >( pUnoI ),
                    pTypeDescr, pOId );

            // proxy may be exchanged during registration
            (*pBridge->pCppEnv->registerProxyInterface)(
                pBridge->pCppEnv, reinterpret_cast< void ** >( &pProxy ),
                freeCppInterfaceProxy, pOId,
                pTypeDescr );

            *ppCppI = pProxy;
        }
        ::rtl_uString_release( pOId );
    }
}

uno_Mapping * Bridge::createMapping(
    uno_ExtEnvironment * pCppEnv, uno_ExtEnvironment * pUnoEnv,
    bool bExportCpp2Uno) SAL_THROW(())
{
    Bridge * bridge = new Bridge(pCppEnv, pUnoEnv, bExportCpp2Uno);
    return bExportCpp2Uno ? &bridge->aCpp2Uno : &bridge->aUno2Cpp;
}

void Bridge::acquire() SAL_THROW(())
{
    if (1 == osl_atomic_increment( &nRef ))
    {
        if (bExportCpp2Uno)
        {
            uno_Mapping * pMapping = &aCpp2Uno;
            ::uno_registerMapping(
                &pMapping, freeMapping, (uno_Environment *)pCppEnv,
                (uno_Environment *)pUnoEnv, 0 );
        }
        else
        {
            uno_Mapping * pMapping = &aUno2Cpp;
            ::uno_registerMapping(
                &pMapping, freeMapping, (uno_Environment *)pUnoEnv,
                (uno_Environment *)pCppEnv, 0 );
        }
    }
}

void Bridge::release() SAL_THROW(())
{
    if (! osl_atomic_decrement( &nRef ))
    {
        ::uno_revokeMapping( bExportCpp2Uno ? &aCpp2Uno : &aUno2Cpp );
    }
}

Bridge::Bridge(
    uno_ExtEnvironment * pCppEnv_, uno_ExtEnvironment * pUnoEnv_,
    bool bExportCpp2Uno_) SAL_THROW(())
    : nRef( 1 )
    , pCppEnv( pCppEnv_ )
    , pUnoEnv( pUnoEnv_ )
    , bExportCpp2Uno( bExportCpp2Uno_ )
{
    bridges::cpp_uno::shared::g_moduleCount.modCnt.acquire(
        &bridges::cpp_uno::shared::g_moduleCount.modCnt );

    aCpp2Uno.pBridge = this;
    aCpp2Uno.acquire = acquireMapping;
    aCpp2Uno.release = releaseMapping;
    aCpp2Uno.mapInterface = cpp2unoMapping;

    aUno2Cpp.pBridge = this;
    aUno2Cpp.acquire = acquireMapping;
    aUno2Cpp.release = releaseMapping;
    aUno2Cpp.mapInterface = uno2cppMapping;

    (*((uno_Environment *)pCppEnv)->acquire)( (uno_Environment *)pCppEnv );
    (*((uno_Environment *)pUnoEnv)->acquire)( (uno_Environment *)pUnoEnv );
}

Bridge::~Bridge() SAL_THROW(())
{
    (*((uno_Environment *)pUnoEnv)->release)( (uno_Environment *)pUnoEnv );
    (*((uno_Environment *)pCppEnv)->release)( (uno_Environment *)pCppEnv );
    bridges::cpp_uno::shared::g_moduleCount.modCnt.release(
        &bridges::cpp_uno::shared::g_moduleCount.modCnt );
}

} } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
