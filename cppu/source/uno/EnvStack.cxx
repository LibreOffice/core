/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#include "uno/environment.hxx"

#include "cppu/EnvDcp.hxx"
#include "cppu/Enterable.hxx"

#include "osl/thread.h"
#include "osl/mutex.hxx"

#include <hash_map>


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

typedef ::std::hash_map<oslThreadIdentifier,
                        uno_Environment *,
                        oslThreadIdentifier_hash,
                        oslThreadIdentifier_equal>  ThreadMap;

static osl::Mutex s_threadMap_mutex;
static ThreadMap  s_threadMap;


static rtl::OUString s_uno_envDcp(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO));

static void s_setCurrent(uno_Environment * pEnv)
{
    oslThreadIdentifier threadId = osl_getThreadIdentifier(NULL);

    osl::MutexGuard guard(s_threadMap_mutex);
    if (pEnv)
        s_threadMap[threadId] = pEnv;
    else
    {
        ThreadMap::iterator iEnv = s_threadMap.find(threadId);
        if( iEnv != s_threadMap.end())
            s_threadMap.erase(iEnv);
    }
}

static uno_Environment * s_getCurrent(void)
{
    uno_Environment * pEnv = NULL;

    oslThreadIdentifier threadId = osl_getThreadIdentifier(NULL);

    osl::MutexGuard guard(s_threadMap_mutex);
    ThreadMap::iterator iEnv = s_threadMap.find(threadId);
    if(iEnv != s_threadMap.end())
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
