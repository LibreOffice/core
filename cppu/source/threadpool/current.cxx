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

#include <sal/config.h>

#include <rtl/byteseq.h>
#include <osl/thread.h>
#include <osl/mutex.hxx>

#include <uno/current_context.h>
#include <uno/environment.hxx>
#include <uno/mapping.hxx>
#include <typelib/typedescription.h>

#include "current.hxx"


using namespace ::osl;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::com::sun::star::uno;

namespace cppu
{

static typelib_InterfaceTypeDescription * get_type_XCurrentContext()
{
    static typelib_InterfaceTypeDescription* s_type_XCurrentContext = []() {
        OUString sTypeName("com.sun.star.uno.XCurrentContext");
        typelib_InterfaceTypeDescription* pTD = nullptr;
        typelib_TypeDescriptionReference* pMembers[1] = { nullptr };
        OUString sMethodName0("com.sun.star.uno.XCurrentContext::getValueByName");
        typelib_typedescriptionreference_new(&pMembers[0], typelib_TypeClass_INTERFACE_METHOD,
                                             sMethodName0.pData);
        typelib_typedescription_newInterface(
            &pTD, sTypeName.pData, 0, 0, 0, 0, 0,
            *typelib_static_type_getByTypeClass(typelib_TypeClass_INTERFACE), 1, pMembers);

        typelib_typedescription_register(reinterpret_cast<typelib_TypeDescription**>(&pTD));
        typelib_typedescriptionreference_release(pMembers[0]);

        typelib_InterfaceMethodTypeDescription* pMethod = nullptr;
        typelib_Parameter_Init aParameters[1];
        OUString sParamName0("Name");
        OUString sParamType0("string");
        aParameters[0].pParamName = sParamName0.pData;
        aParameters[0].eTypeClass = typelib_TypeClass_STRING;
        aParameters[0].pTypeName = sParamType0.pData;
        aParameters[0].bIn = true;
        aParameters[0].bOut = false;
        rtl_uString* pExceptions[1];
        OUString sExceptionName0("com.sun.star.uno.RuntimeException");
        pExceptions[0] = sExceptionName0.pData;
        OUString sReturnType0("any");
        typelib_typedescription_newInterfaceMethod(&pMethod, 3, false, sMethodName0.pData,
                                                   typelib_TypeClass_ANY, sReturnType0.pData, 1,
                                                   aParameters, 1, pExceptions);
        typelib_typedescription_register(reinterpret_cast<typelib_TypeDescription**>(&pMethod));
        typelib_typedescription_release(&pMethod->aBase.aBase);
        // another static ref:
        ++reinterpret_cast<typelib_TypeDescription*>(pTD)->nStaticRefCount;
        return pTD;
    }();

    return s_type_XCurrentContext;
}


class ThreadKey
{
    bool     _bInit;
    oslThreadKey _hThreadKey;
    oslThreadKeyCallbackFunction _pCallback;

public:
    oslThreadKey getThreadKey()
    {
        if (! _bInit)
        {
            MutexGuard aGuard( Mutex::getGlobalMutex() );
            if (! _bInit)
            {
                _hThreadKey = ::osl_createThreadKey( _pCallback );
                _bInit = true;
            }
        }
        return _hThreadKey;
    }

    explicit ThreadKey( oslThreadKeyCallbackFunction pCallback )
        : _bInit(false)
        , _hThreadKey(nullptr)
        , _pCallback(pCallback)
    {
    }

    ~ThreadKey()
    {
        if (_bInit)
        {
            ::osl_destroyThreadKey( _hThreadKey );
        }
    }
};

extern "C" {

static void delete_IdContainer( void * p )
{
    if (p)
    {
        IdContainer * pId = static_cast< IdContainer * >( p );
        if (pId->pCurrentContext)
        {
            (*pId->pCurrentContextEnv->releaseInterface)(
                pId->pCurrentContextEnv, pId->pCurrentContext );
            (*pId->pCurrentContextEnv->aBase.release)(
                &pId->pCurrentContextEnv->aBase );
        }
        if (pId->bInit)
        {
            ::rtl_byte_sequence_release( pId->pLocalThreadId );
            ::rtl_byte_sequence_release( pId->pCurrentId );
        }
        delete pId;
    }
}

}

IdContainer * getIdContainer()
{
    static ThreadKey s_key( delete_IdContainer );
    oslThreadKey aKey = s_key.getThreadKey();

    IdContainer * pId = static_cast< IdContainer * >( ::osl_getThreadKeyData( aKey ) );
    if (! pId)
    {
        pId = new IdContainer;
        pId->pCurrentContext = nullptr;
        pId->pCurrentContextEnv = nullptr;
        pId->bInit = false;
        ::osl_setThreadKeyData( aKey, pId );
    }
    return pId;
}

}


extern "C" sal_Bool SAL_CALL uno_setCurrentContext(
    void * pCurrentContext,
    rtl_uString * pEnvTypeName, void * pEnvContext )
    SAL_THROW_EXTERN_C()
{
    IdContainer * pId = getIdContainer();
    OSL_ASSERT( pId );

    // free old one
    if (pId->pCurrentContext)
    {
        (*pId->pCurrentContextEnv->releaseInterface)(
            pId->pCurrentContextEnv, pId->pCurrentContext );
        (*pId->pCurrentContextEnv->aBase.release)(
            &pId->pCurrentContextEnv->aBase );
        pId->pCurrentContextEnv = nullptr;

        pId->pCurrentContext = nullptr;
    }

    if (pCurrentContext)
    {
        uno_Environment * pEnv = nullptr;
        ::uno_getEnvironment( &pEnv, pEnvTypeName, pEnvContext );
        OSL_ASSERT( pEnv && pEnv->pExtEnv );
        if (pEnv)
        {
            if (pEnv->pExtEnv)
            {
                pId->pCurrentContextEnv = pEnv->pExtEnv;
                (*pId->pCurrentContextEnv->acquireInterface)(
                    pId->pCurrentContextEnv, pCurrentContext );
                pId->pCurrentContext = pCurrentContext;
            }
            else
            {
                (*pEnv->release)( pEnv );
                return false;
            }
        }
        else
        {
            return false;
        }
    }
    return true;
}

extern "C" sal_Bool SAL_CALL uno_getCurrentContext(
    void ** ppCurrentContext, rtl_uString * pEnvTypeName, void * pEnvContext )
    SAL_THROW_EXTERN_C()
{
    IdContainer * pId = getIdContainer();
    OSL_ASSERT( pId );

    Environment target_env;

    // release inout parameter
    if (*ppCurrentContext)
    {
        target_env = Environment(OUString(pEnvTypeName), pEnvContext);
        OSL_ASSERT( target_env.is() );
        if (! target_env.is())
            return false;
        uno_ExtEnvironment * pEnv = target_env.get()->pExtEnv;
        OSL_ASSERT( nullptr != pEnv );
        if (nullptr == pEnv)
            return false;
        (*pEnv->releaseInterface)( pEnv, *ppCurrentContext );

        *ppCurrentContext = nullptr;
    }

    // case: null-ref
    if (nullptr == pId->pCurrentContext)
        return true;

    if (! target_env.is())
    {
        target_env = Environment(OUString(pEnvTypeName), pEnvContext);
        OSL_ASSERT( target_env.is() );
        if (! target_env.is())
            return false;
    }

    Mapping mapping(&pId->pCurrentContextEnv->aBase, target_env.get());
    OSL_ASSERT( mapping.is() );
    if (! mapping.is())
        return false;

    mapping.mapInterface(ppCurrentContext, pId->pCurrentContext, ::cppu::get_type_XCurrentContext() );

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
