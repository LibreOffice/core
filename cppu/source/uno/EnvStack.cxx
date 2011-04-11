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

#include "uno/environment.hxx"

#include "cppu/EnvDcp.hxx"
#include "cppu/Enterable.hxx"

#include "rtl/instance.hxx"

#include "osl/thread.h"
#include "osl/mutex.hxx"

#include <boost/unordered_map.hpp>


using namespace com::sun::star;


struct SAL_DLLPRIVATE oslThreadIdentifier_equal
{
    bool operator()(oslThreadIdentifier s1, oslThreadIdentifier s2) const;
};

bool oslThreadIdentifier_equal::operator()(oslThreadIdentifier s1, oslThreadIdentifier s2) const
{
    bool result = s1 == s2;

    return result;
}


struct SAL_DLLPRIVATE oslThreadIdentifier_hash
{
    size_t operator()(oslThreadIdentifier s1) const;
};

size_t oslThreadIdentifier_hash::operator()(oslThreadIdentifier s1) const
{
    return s1;
}

typedef ::boost::unordered_map<oslThreadIdentifier,
                        uno_Environment *,
                        oslThreadIdentifier_hash,
                        oslThreadIdentifier_equal>  ThreadMap;

namespace
{
    struct s_threadMap_mutex : public rtl::Static< osl::Mutex, s_threadMap_mutex > {};
    struct s_threadMap : public rtl::Static< ThreadMap, s_threadMap > {};
}

static rtl::OUString s_uno_envDcp(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO));

static void s_setCurrent(uno_Environment * pEnv)
{
    oslThreadIdentifier threadId = osl_getThreadIdentifier(NULL);

    osl::MutexGuard guard(s_threadMap_mutex::get());
    ThreadMap &rThreadMap = s_threadMap::get();
    if (pEnv)
        rThreadMap[threadId] = pEnv;

    else
        rThreadMap.erase(threadId);
}

static uno_Environment * s_getCurrent(void)
{
    uno_Environment * pEnv = NULL;

    oslThreadIdentifier threadId = osl_getThreadIdentifier(NULL);

    osl::MutexGuard guard(s_threadMap_mutex::get());
    ThreadMap &rThreadMap = s_threadMap::get();
    ThreadMap::iterator iEnv = rThreadMap.find(threadId);
    if(iEnv != rThreadMap.end())
        pEnv = iEnv->second;

    return pEnv;
}


extern "C" void SAL_CALL uno_getCurrentEnvironment(uno_Environment ** ppEnv, rtl_uString * pTypeName)
    SAL_THROW_EXTERN_C()
{
    if (*ppEnv)
    {
        (*ppEnv)->release(*ppEnv);
        *ppEnv = NULL;
    }

    rtl::OUString currPurpose;

    uno_Environment * pCurrEnv = s_getCurrent();
    if (pCurrEnv) // no environment means no purpose
        currPurpose = cppu::EnvDcp::getPurpose(pCurrEnv->pTypeName);

    if (pTypeName && rtl_uString_getLength(pTypeName))
    {
        rtl::OUString envDcp(pTypeName);
        envDcp += currPurpose;

        uno_getEnvironment(ppEnv, envDcp.pData, NULL);
    }
    else
    {
        if (pCurrEnv)
        {
            *ppEnv = pCurrEnv;
            (*ppEnv)->acquire(*ppEnv);
        }
        else
            uno_getEnvironment(ppEnv, s_uno_envDcp.pData, NULL);

    }
}

static rtl::OUString s_getPrefix(rtl::OUString const & str1, rtl::OUString const & str2)
{
    sal_Int32 nIndex1 = 0;
    sal_Int32 nIndex2 = 0;
    sal_Int32 sim = 0;

    rtl::OUString token1;
    rtl::OUString token2;

    do
    {
        token1 = str1.getToken(0, ':', nIndex1);
        token2 = str2.getToken(0, ':', nIndex2);

        if (token1.equals(token2))
            sim += token1.getLength() + 1;
    }
    while(nIndex1 == nIndex2 && nIndex1 >= 0 && token1.equals(token2));

    rtl::OUString result;

    if (sim)
        result = str1.copy(0, sim - 1);

    return result;
}

static int s_getNextEnv(uno_Environment ** ppEnv, uno_Environment * pCurrEnv, uno_Environment * pTargetEnv)
{
    int res = 0;

    rtl::OUString nextPurpose;

    rtl::OUString currPurpose;
    if (pCurrEnv)
        currPurpose = cppu::EnvDcp::getPurpose(pCurrEnv->pTypeName);

    rtl::OUString targetPurpose;
    if (pTargetEnv)
        targetPurpose = cppu::EnvDcp::getPurpose(pTargetEnv->pTypeName);

    rtl::OUString intermPurpose(s_getPrefix(currPurpose, targetPurpose));
    if (currPurpose.getLength() > intermPurpose.getLength())
    {
        sal_Int32 idx = currPurpose.lastIndexOf(':');
        nextPurpose = currPurpose.copy(0, idx);

        res = -1;
    }
    else if (intermPurpose.getLength() < targetPurpose.getLength())
    {
        sal_Int32 idx = targetPurpose.indexOf(':', intermPurpose.getLength() + 1);
        if (idx == -1)
            nextPurpose = targetPurpose;

        else
            nextPurpose = targetPurpose.copy(0, idx);

        res = 1;
    }

    if (nextPurpose.getLength())
    {
        rtl::OUString next_envDcp(s_uno_envDcp);
        next_envDcp += nextPurpose;

        uno_getEnvironment(ppEnv, next_envDcp.pData, NULL);
    }
    else
    {
        if (*ppEnv)
            (*ppEnv)->release(*ppEnv);

        *ppEnv = NULL;
    }

    return res;
}

extern "C" { static void s_pull(va_list * pParam)
{
    uno_EnvCallee * pCallee = va_arg(*pParam, uno_EnvCallee *);
    va_list       * pXparam = va_arg(*pParam, va_list *);

    pCallee(pXparam);
}}

static void s_callInto_v(uno_Environment * pEnv, uno_EnvCallee * pCallee, va_list * pParam)
{
    cppu::Enterable * pEnterable = reinterpret_cast<cppu::Enterable *>(pEnv->pReserved);
    if (pEnterable)
        pEnterable->callInto(s_pull, pCallee, pParam);

    else
        pCallee(pParam);
}

static void s_callInto(uno_Environment * pEnv, uno_EnvCallee * pCallee, ...)
{
    va_list param;

    va_start(param, pCallee);
    s_callInto_v(pEnv, pCallee, &param);
    va_end(param);
}

static void s_callOut_v(uno_Environment * pEnv, uno_EnvCallee * pCallee, va_list * pParam)
{
    cppu::Enterable * pEnterable = reinterpret_cast<cppu::Enterable *>(pEnv->pReserved);
    if (pEnterable)
        pEnterable->callOut_v(pCallee, pParam);

    else
        pCallee(pParam);
}

static void s_callOut(uno_Environment * pEnv, uno_EnvCallee * pCallee, ...)
{
    va_list param;

    va_start(param, pCallee);
    s_callOut_v(pEnv, pCallee, &param);
    va_end(param);
}

static void s_environment_invoke_v(uno_Environment *, uno_Environment *, uno_EnvCallee *, va_list *);

extern "C" { static void s_environment_invoke_vv(va_list * pParam)
{
    uno_Environment * pCurrEnv    = va_arg(*pParam, uno_Environment *);
    uno_Environment * pTargetEnv  = va_arg(*pParam, uno_Environment *);
    uno_EnvCallee   * pCallee     = va_arg(*pParam, uno_EnvCallee *);
    va_list         * pXparam     = va_arg(*pParam, va_list *);

    s_environment_invoke_v(pCurrEnv, pTargetEnv, pCallee, pXparam);
}}

static void s_environment_invoke_v(uno_Environment * pCurrEnv, uno_Environment * pTargetEnv, uno_EnvCallee * pCallee, va_list * pParam)
{
    uno_Environment * pNextEnv = NULL;
    switch(s_getNextEnv(&pNextEnv, pCurrEnv, pTargetEnv))
    {
    case -1:
        s_setCurrent(pNextEnv);
        s_callOut(pCurrEnv, s_environment_invoke_vv, pNextEnv, pTargetEnv, pCallee, pParam);
        s_setCurrent(pCurrEnv);
        break;

    case 0: {
        uno_Environment * hld = s_getCurrent();
        s_setCurrent(pCurrEnv);
        pCallee(pParam);
        s_setCurrent(hld);
    }
        break;

    case 1:
        s_setCurrent(pNextEnv);
        s_callInto(pNextEnv, s_environment_invoke_vv, pNextEnv, pTargetEnv, pCallee, pParam);
        s_setCurrent(pCurrEnv);
        break;
    }

    if (pNextEnv)
        pNextEnv->release(pNextEnv);
}

extern "C" void SAL_CALL uno_Environment_invoke_v(uno_Environment * pTargetEnv, uno_EnvCallee * pCallee, va_list * pParam)
    SAL_THROW_EXTERN_C()
{
    s_environment_invoke_v(s_getCurrent(), pTargetEnv, pCallee, pParam);
}

extern "C" void SAL_CALL uno_Environment_invoke(uno_Environment * pEnv, uno_EnvCallee * pCallee, ...)
    SAL_THROW_EXTERN_C()
{
    va_list param;

    va_start(param, pCallee);
    uno_Environment_invoke_v(pEnv, pCallee, &param);
    va_end(param);
}

extern "C" void SAL_CALL uno_Environment_enter(uno_Environment * pTargetEnv)
    SAL_THROW_EXTERN_C()
{
    uno_Environment * pNextEnv = NULL;
    uno_Environment * pCurrEnv = s_getCurrent();

    int res;
    while ( (res = s_getNextEnv(&pNextEnv, pCurrEnv, pTargetEnv)) != 0)
    {
        cppu::Enterable * pEnterable;

        switch(res)
        {
        case -1:
            pEnterable = reinterpret_cast<cppu::Enterable *>(pCurrEnv->pReserved);
            if (pEnterable)
                pEnterable->leave();
            pCurrEnv->release(pCurrEnv);
            break;

        case 1:
            pNextEnv->acquire(pNextEnv);
            pEnterable = reinterpret_cast<cppu::Enterable *>(pNextEnv->pReserved);
            if (pEnterable)
                pEnterable->enter();
            break;
        }

        s_setCurrent(pNextEnv);
        pCurrEnv = pNextEnv;
    }
}

int SAL_CALL uno_Environment_isValid(uno_Environment * pEnv, rtl_uString ** pReason)
    SAL_THROW_EXTERN_C()
{
    int result = 1;

    rtl::OUString typeName(cppu::EnvDcp::getTypeName(pEnv->pTypeName));
    if (typeName.equals(s_uno_envDcp))
    {
        cppu::Enterable * pEnterable = reinterpret_cast<cppu::Enterable *>(pEnv->pReserved);
        if (pEnterable)
            result = pEnterable->isValid((rtl::OUString *)pReason);
    }
    else
    {
        rtl::OUString envDcp(s_uno_envDcp);
        envDcp += cppu::EnvDcp::getPurpose(pEnv->pTypeName);

        uno::Environment env(envDcp);

        result = env.isValid((rtl::OUString *)pReason);
    }

    return result;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
