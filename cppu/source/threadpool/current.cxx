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
#include <osl/mutex.hxx>

#include <uno/current_context.h>
#include <uno/environment.hxx>
#include <uno/mapping.hxx>
#include <typelib/typedescription.h>

#include "current.hxx"


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

IdContainer::IdContainer()
    : pCurrentContext(nullptr)
    , pCurrentContextEnv(nullptr)
    , pLocalThreadId(nullptr)
    , pCurrentId(nullptr)
    , nRefCountOfCurrentId(0)
    , bInit(false)
{
}

IdContainer::~IdContainer()
{
    if (pCurrentContext)
    {
        (*pCurrentContextEnv->releaseInterface)(
            pCurrentContextEnv, pCurrentContext );
        (*pCurrentContextEnv->aBase.release)(
            &pCurrentContextEnv->aBase );
    }
    if (bInit)
    {
        ::rtl_byte_sequence_release( pLocalThreadId );
        ::rtl_byte_sequence_release( pCurrentId );
    }
}

IdContainer& getIdContainer()
{
    static thread_local IdContainer aId;
    return aId;
}

}

extern "C" sal_Bool SAL_CALL uno_setCurrentContext(
    void * pCurrentContext,
    rtl_uString * pEnvTypeName, void * pEnvContext )
    SAL_THROW_EXTERN_C()
{
    IdContainer& id = getIdContainer();

    // free old one
    if (id.pCurrentContext)
    {
        (*id.pCurrentContextEnv->releaseInterface)(
            id.pCurrentContextEnv, id.pCurrentContext );
        (*id.pCurrentContextEnv->aBase.release)(
            &id.pCurrentContextEnv->aBase );
        id.pCurrentContextEnv = nullptr;

        id.pCurrentContext = nullptr;
    }

    if (!pCurrentContext)
        return true;

    uno_Environment * pEnv = nullptr;
    ::uno_getEnvironment( &pEnv, pEnvTypeName, pEnvContext );
    OSL_ASSERT( pEnv && pEnv->pExtEnv );
    if (pEnv)
    {
        if (pEnv->pExtEnv)
        {
            id.pCurrentContextEnv = pEnv->pExtEnv;
            (*id.pCurrentContextEnv->acquireInterface)(
                id.pCurrentContextEnv, pCurrentContext );
            id.pCurrentContext = pCurrentContext;
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
    return true;
}

extern "C" sal_Bool SAL_CALL uno_getCurrentContext(
    void ** ppCurrentContext, rtl_uString * pEnvTypeName, void * pEnvContext )
    SAL_THROW_EXTERN_C()
{
    IdContainer& id = getIdContainer();

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
    if (nullptr == id.pCurrentContext)
        return true;

    if (! target_env.is())
    {
        target_env = Environment(OUString(pEnvTypeName), pEnvContext);
        OSL_ASSERT( target_env.is() );
        if (! target_env.is())
            return false;
    }

    Mapping mapping(&id.pCurrentContextEnv->aBase, target_env.get());
    OSL_ASSERT( mapping.is() );
    if (! mapping.is())
        return false;

    mapping.mapInterface(ppCurrentContext, id.pCurrentContext, ::cppu::get_type_XCurrentContext());

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
