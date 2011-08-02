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

#include "mono_bridge.h"

#include "rtl/ustring.h"
#include "uno/dispatcher.h"
#include "uno/environment.h"
#include "typelib/typedescription.h"

extern "C" {
#include "mono/metadata/threads.h"
}

using namespace mono_uno;

extern "C" {

static void SAL_CALL mono_proxy_free( uno_ExtEnvironment * /* env */, void * proxy)
{
    MonoProxy * monoProxy = reinterpret_cast< MonoProxy * >( proxy );

    delete monoProxy;
}

uno_Interface * SAL_CALL cli_uno_environment_createMonoProxyAndRegister(
    uno_ExtEnvironment *pUnoEnv, void *pMonoProxy, rtl_uString *pOid,
    typelib_TypeDescription *pTD )
    SAL_THROW_EXTERN_C()
{
    uno_Interface * proxy = static_cast< uno_Interface * >(
        new MonoProxy( pUnoEnv,
                       static_cast< guint32 >( reinterpret_cast< sal_IntPtr >( pMonoProxy ) ),
                       pOid, pTD ) );

    pUnoEnv->registerProxyInterface(
        pUnoEnv,
        reinterpret_cast< void ** >( &proxy ),
        mono_proxy_free,
        pOid,
        (typelib_InterfaceTypeDescription*) pTD );

    return proxy;
}

static void SAL_CALL mono_proxy_acquire( uno_Interface * pUnoI )
{
    MonoProxy const * that = static_cast< MonoProxy const * >( pUnoI );
    that->acquire();
}

static void SAL_CALL mono_proxy_release( uno_Interface * pUnoI )
{
    MonoProxy const * that = static_cast< MonoProxy const * >( pUnoI );
    that->release();
}

static void SAL_CALL mono_proxy_dispatch(
    uno_Interface * pUnoI, typelib_TypeDescription const * member_td,
    void * uno_ret, void * uno_args [], uno_Any ** uno_exc )
    SAL_THROW_EXTERN_C()
{
    MonoProxy * that = static_cast< MonoProxy * >( pUnoI );
    that->dispatch( member_td, uno_ret, uno_args, uno_exc );
}

} // extern "C"

namespace mono_uno
{

MonoProxy::MonoProxy(uno_ExtEnvironment * pUnoEnv, guint32 managedProxy,
                     rtl_uString *pOid, typelib_TypeDescription * pTD):
    m_ref(1),
    m_managedProxy(managedProxy), // FIXME free this in the destructor?
    m_unoEnv(pUnoEnv),
    m_unoType(pTD),
    m_Oid(pOid)
{
    uno_Interface::acquire = mono_proxy_acquire;
    uno_Interface::release = mono_proxy_release;
    uno_Interface::pDispatcher = mono_proxy_dispatch;

    MonoObject * pObj = mono_gchandle_get_target( m_managedProxy );
    MonoClass * pClass = mono_object_get_class( pObj );
    MonoMethodDesc * pMethodDesc = mono_method_desc_new( "ManagedProxy:Dispatch", FALSE );
    m_managedDispatch = mono_method_desc_search_in_class( pMethodDesc, pClass );
    mono_method_desc_free( pMethodDesc );
    OSL_ASSERT( 0 != m_managedDispatch );
}

inline void MonoProxy::acquire() const
{
    if (1 == osl_incrementInterlockedCount( &m_ref ))
    {
        // rebirth of proxy zombie
        void * that = const_cast< MonoProxy * >( this );
        // register at uno env
        (*m_unoEnv->registerProxyInterface)(
            m_unoEnv, &that,
            mono_proxy_free, m_Oid.pData,
            (typelib_InterfaceTypeDescription *)m_unoType.get() );
#if OSL_DEBUG_LEVEL >= 2
        OSL_ASSERT( this == (void const * const)that );
#endif
    }
}

inline void MonoProxy::release() const
{
    if (0 == osl_decrementInterlockedCount( &m_ref ))
    {
        // revoke from uno env on last release,
        // The proxy can be resurrected if acquire is called before the uno
        // environment calls mono_proxy_free. mono_proxy_free will
        //delete the proxy. The environment does not acquire a registered
        //proxy.
        (*m_unoEnv->revokeInterface)(
            m_unoEnv, const_cast< MonoProxy * >( this ) );
    }
}

inline void MonoProxy::dispatch( typelib_TypeDescription const * member_td,
                                 void * uno_ret, void * uno_args [], uno_Any ** uno_exc )
{
    OSL_ASSERT( m_managedDispatch != 0 );

    gpointer pMonoParams[4];

    pMonoParams[0] = const_cast< typelib_TypeDescription * >(member_td);
    pMonoParams[1] = uno_ret;
    pMonoParams[2] = uno_args;
    pMonoParams[3] = uno_exc;

    MonoObject *obj = mono_gchandle_get_target( m_managedProxy );

    mono_uno::runtime_invoke( m_managedDispatch,
                              obj, pMonoParams, NULL,
                              mono_object_get_domain( obj ) );
}

}
