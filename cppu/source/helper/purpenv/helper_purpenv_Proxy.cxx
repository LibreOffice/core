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


#include "Proxy.hxx"

#include <sal/log.hxx>
#include <uno/dispatcher.h>
#include <typelib/typedescription.hxx>

using namespace com::sun::star;

static bool relatesToInterface(typelib_TypeDescription * pTypeDescr)
{
    switch (pTypeDescr->eTypeClass)
    {
//      case typelib_TypeClass_TYPEDEF:
    case typelib_TypeClass_SEQUENCE:
    {
        switch (reinterpret_cast<typelib_IndirectTypeDescription *>(pTypeDescr)->pType->eTypeClass)
        {
        case typelib_TypeClass_INTERFACE:
        case typelib_TypeClass_ANY: // might relate to interface
            return true;
        case typelib_TypeClass_SEQUENCE:
        case typelib_TypeClass_STRUCT:
        case typelib_TypeClass_EXCEPTION:
        {
            typelib_TypeDescription * pTD = nullptr;
            TYPELIB_DANGER_GET( &pTD, reinterpret_cast<typelib_IndirectTypeDescription *>(pTypeDescr)->pType );
            bool bRel = relatesToInterface( pTD );
            TYPELIB_DANGER_RELEASE( pTD );
            return bRel;
        }
        default:
            ;
        }
        return false;
    }
    case typelib_TypeClass_STRUCT:
    case typelib_TypeClass_EXCEPTION:
    {
        // ...optimized... to avoid getDescription() calls!
        typelib_CompoundTypeDescription * pComp    = reinterpret_cast<typelib_CompoundTypeDescription *>(pTypeDescr);
        typelib_TypeDescriptionReference ** pTypes = pComp->ppTypeRefs;
        for ( sal_Int32 nPos = pComp->nMembers; nPos--; )
        {
            switch (pTypes[nPos]->eTypeClass)
            {
            case typelib_TypeClass_INTERFACE:
            case typelib_TypeClass_ANY: // might relate to interface
                return true;
//              case typelib_TypeClass_TYPEDEF:
            case typelib_TypeClass_SEQUENCE:
            case typelib_TypeClass_STRUCT:
            case typelib_TypeClass_EXCEPTION:
            {
                typelib_TypeDescription * pTD = nullptr;
                TYPELIB_DANGER_GET( &pTD, pTypes[nPos] );
                bool bRel = relatesToInterface( pTD );
                TYPELIB_DANGER_RELEASE( pTD );
                if (bRel)
                    return true;
                break;
            }
            default:
                break;
            }
        }
        if (pComp->pBaseTypeDescription)
            return relatesToInterface( &pComp->pBaseTypeDescription->aBase );
        break;
    }
    case typelib_TypeClass_ANY: // might relate to interface
    case typelib_TypeClass_INTERFACE:
        return true;

    default:
        ;
    }
    return false;
}

extern "C" { static void s_Proxy_dispatch(
    uno_Interface                 * pUnoI,
    typelib_TypeDescription const * pMemberType,
    void                          * pReturn,
    void                          * pArgs[],
    uno_Any                      ** ppException)
    SAL_THROW_EXTERN_C()
{
    Proxy * pThis = static_cast<Proxy *>(pUnoI);

    typelib_MethodParameter            param;
    sal_Int32                          nParams = 0;
    typelib_MethodParameter          * pParams = nullptr;
    typelib_TypeDescriptionReference * pReturnTypeRef = nullptr;
    // sal_Int32                          nOutParams = 0;

    switch (pMemberType->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
        if (pReturn)
        {
            pReturnTypeRef =
                reinterpret_cast<typelib_InterfaceAttributeTypeDescription const *>(
                 pMemberType)->pAttributeTypeRef;
            nParams = 0;
            pParams = nullptr;
        }
        else
        {
            param.pTypeRef = reinterpret_cast<typelib_InterfaceAttributeTypeDescription const *>(
                              pMemberType)->pAttributeTypeRef;
            param.bIn = true;
            param.bOut = false;
            nParams = 1;
            pParams = &param;
        }
        break;
    case typelib_TypeClass_INTERFACE_METHOD:
    {
        typelib_InterfaceMethodTypeDescription const * method_td =
            reinterpret_cast<typelib_InterfaceMethodTypeDescription const *>(pMemberType);
        pReturnTypeRef = method_td->pReturnTypeRef;
        nParams = method_td->nParams;
        pParams = method_td->pParams;
        break;
    }
    default:
        OSL_FAIL( "### illegal member typeclass!" );
        abort();
    }

    pThis->dispatch( pReturnTypeRef,
                     pParams,
                     nParams,
                     pMemberType,
                     pReturn,
                     pArgs,
                     ppException );
}}

extern "C" void Proxy_free(SAL_UNUSED_PARAMETER uno_ExtEnvironment * /*pEnv*/, void * pProxy) SAL_THROW_EXTERN_C()
{
    Proxy * pThis = static_cast<Proxy * >(static_cast<uno_Interface *>(pProxy));
    delete pThis;
}

extern "C" {
static void s_Proxy_acquire(uno_Interface * pUnoI) SAL_THROW_EXTERN_C()
{
    Proxy * pProxy = static_cast<Proxy *>(pUnoI);
    pProxy->acquire();
}

static void s_Proxy_release(uno_Interface * pUnoI) SAL_THROW_EXTERN_C()
{
    Proxy * pProxy = static_cast<Proxy *>(pUnoI);
    pProxy->release();
}

static void s_acquireAndRegister_v(va_list * pParam)
{
    uno_Interface                    * pUnoI      = va_arg(*pParam, uno_Interface *);
    rtl_uString                      * pOid       = va_arg(*pParam, rtl_uString *);
    typelib_InterfaceTypeDescription * pTypeDescr = va_arg(*pParam, typelib_InterfaceTypeDescription *);
    uno_ExtEnvironment               * pEnv       = va_arg(*pParam, uno_ExtEnvironment *);

    pUnoI->acquire(pUnoI);
    pEnv->registerInterface(pEnv, reinterpret_cast<void **>(&pUnoI), pOid, pTypeDescr);
}
}

Proxy::Proxy(uno::Mapping                  const & to_from,
             uno_Environment                     * pTo,
             uno_Environment                     * pFrom,
             uno_Interface                       * pUnoI,
             typelib_InterfaceTypeDescription    * pTypeDescr,
             OUString                      const & rOId,
             cppu::helper::purpenv::ProbeFun     * probeFun,
             void                                * pProbeContext
)
        : m_nRef         (1),
          m_from         (pFrom),
          m_to           (pTo),
          m_from_to      (pFrom, pTo),
          m_to_from      (to_from),
          m_pUnoI        (pUnoI),
          m_pTypeDescr   (pTypeDescr),
          m_aOId         (rOId),
          m_probeFun     (probeFun),
          m_pProbeContext(pProbeContext)
{
    SAL_INFO("cppu.purpenv", "LIFE: Proxy::Proxy(<>) -> " << this);

    typelib_typedescription_acquire(&m_pTypeDescr->aBase);
    if (!m_pTypeDescr->aBase.bComplete)
        typelib_typedescription_complete(reinterpret_cast<typelib_TypeDescription **>(&m_pTypeDescr));

    OSL_ENSURE(m_pTypeDescr->aBase.bComplete, "### type is incomplete!");

    uno_Environment_invoke(m_to.get(), s_acquireAndRegister_v, m_pUnoI, rOId.pData, pTypeDescr, m_to.get());

    // uno_Interface
    uno_Interface::acquire     = s_Proxy_acquire;
    uno_Interface::release     = s_Proxy_release;
    uno_Interface::pDispatcher = s_Proxy_dispatch;
}

extern "C" { static void s_releaseAndRevoke_v(va_list * pParam)
{
    uno_ExtEnvironment * pEnv  = va_arg(*pParam, uno_ExtEnvironment *);
    uno_Interface      * pUnoI = va_arg(*pParam, uno_Interface *);

    pEnv->revokeInterface(pEnv, pUnoI);
    pUnoI->release(pUnoI);
}}

Proxy::~Proxy()
{
    SAL_INFO("cppu.purpenv", "LIFE: Proxy::~Proxy() -> " << this);

    uno_Environment_invoke(m_to.get(), s_releaseAndRevoke_v, m_to.get(), m_pUnoI);

    typelib_typedescription_release(&m_pTypeDescr->aBase);
}

static uno::TypeDescription getAcquireMethod()
{
    typelib_TypeDescriptionReference * type_XInterface =
        * typelib_static_type_getByTypeClass(typelib_TypeClass_INTERFACE);

    typelib_TypeDescription * pTXInterfaceDescr = nullptr;
    TYPELIB_DANGER_GET    (&pTXInterfaceDescr, type_XInterface);
    uno::TypeDescription acquire(
        reinterpret_cast< typelib_InterfaceTypeDescription * >(
            pTXInterfaceDescr)->ppAllMembers[1]);
    TYPELIB_DANGER_RELEASE(pTXInterfaceDescr);

    return acquire;
}

static uno::TypeDescription getReleaseMethod()
{
    typelib_TypeDescriptionReference * type_XInterface =
        * typelib_static_type_getByTypeClass(typelib_TypeClass_INTERFACE);

    typelib_TypeDescription * pTXInterfaceDescr = nullptr;
    TYPELIB_DANGER_GET    (&pTXInterfaceDescr, type_XInterface);
    uno::TypeDescription release(
        reinterpret_cast< typelib_InterfaceTypeDescription * >(
            pTXInterfaceDescr)->ppAllMembers[2]);
    TYPELIB_DANGER_RELEASE(pTXInterfaceDescr);

    return release;
}

static uno::TypeDescription s_acquireMethod(getAcquireMethod());
static uno::TypeDescription s_releaseMethod(getReleaseMethod());

void Proxy::acquire()
{
    if (m_probeFun)
        m_probeFun(true,
                   this,
                   m_pProbeContext,
                   *typelib_static_type_getByTypeClass(typelib_TypeClass_VOID),
                   nullptr,
                   0,
                   s_acquireMethod.get(),
                   nullptr,
                   nullptr,
                   nullptr);

    if (osl_atomic_increment(&m_nRef) == 1)
    {
        // rebirth of proxy zombie
        void * pThis = this;
        m_from.get()->pExtEnv->registerProxyInterface(m_from.get()->pExtEnv,
                                                      &pThis,
                                                      Proxy_free,
                                                      m_aOId.pData,
                                                      m_pTypeDescr);
        OSL_ASSERT(pThis == this);
    }

    if (m_probeFun)
        m_probeFun(false,
                   this,
                   m_pProbeContext,
                   *typelib_static_type_getByTypeClass(typelib_TypeClass_VOID),
                   nullptr,
                   0,
                   s_acquireMethod.get(),
                   nullptr,
                   nullptr,
                   nullptr);

}

void Proxy::release()
{
    cppu::helper::purpenv::ProbeFun * probeFun = m_probeFun;
    void                            * pProbeContext = m_pProbeContext;

    if (m_probeFun)
        m_probeFun(true,
                   this,
                   m_pProbeContext,
                   *typelib_static_type_getByTypeClass(typelib_TypeClass_VOID),
                   nullptr,
                   0,
                   s_releaseMethod.get(),
                   nullptr,
                   nullptr,
                   nullptr);

    if (osl_atomic_decrement(&m_nRef) == 0)
        m_from.get()->pExtEnv->revokeInterface(m_from.get()->pExtEnv, this);

    if (probeFun)
        probeFun(false,
                 this,
                 pProbeContext,
                 *typelib_static_type_getByTypeClass(typelib_TypeClass_VOID),
                 nullptr,
                 0,
                 s_releaseMethod.get(),
                 nullptr,
                 nullptr,
                 nullptr);

}


extern "C" {
static void s_type_destructData_v(va_list * pParam)
{
    void * ret = va_arg(*pParam, void *);
    typelib_TypeDescriptionReference * pReturnTypeRef = va_arg(*pParam, typelib_TypeDescriptionReference *);

    uno_type_destructData(ret, pReturnTypeRef, nullptr);
}

static void s_dispatcher_v(va_list * pParam)
{
    uno_Interface                 * pUnoI       = va_arg(*pParam, uno_Interface *);
    typelib_TypeDescription const * pMemberType = va_arg(*pParam, typelib_TypeDescription const *);
    void                          * pReturn     = va_arg(*pParam, void *);
    void                         ** pArgs       = va_arg(*pParam, void **);
    uno_Any                      ** ppException = va_arg(*pParam, uno_Any **);

    pUnoI->pDispatcher(pUnoI, pMemberType, pReturn, pArgs, ppException);
}
}

void Proxy::dispatch(typelib_TypeDescriptionReference * pReturnTypeRef,
                     typelib_MethodParameter          * pParams,
                     sal_Int32                          nParams,
                     typelib_TypeDescription    const * pMemberType,
                     void                             * pReturn,
                     void                             * pArgs[],
                     uno_Any                         ** ppException)
{
    if (m_probeFun)
        m_probeFun(true,
                   this,
                   m_pProbeContext,
                   pReturnTypeRef,
                   pParams,
                   nParams,
                   pMemberType,
                   pReturn,
                   pArgs,
                   ppException);

    void ** args = static_cast<void **>(alloca( sizeof (void *) * nParams ));

    typelib_TypeDescription * return_td = nullptr;
    void * ret = pReturn;
    if (pReturnTypeRef)
    {
        TYPELIB_DANGER_GET(&return_td, pReturnTypeRef);

        if (relatesToInterface(return_td))
            ret = alloca(return_td->nSize);

        TYPELIB_DANGER_RELEASE(return_td);
    }

    for (sal_Int32 nPos = 0; nPos < nParams; ++ nPos)
    {
        typelib_MethodParameter const & param = pParams[nPos];
        typelib_TypeDescription * td = nullptr;
        TYPELIB_DANGER_GET( &td, param.pTypeRef );
        if (relatesToInterface(td))
        {
            args[nPos] = alloca(td->nSize);
            if (param.bIn)
            {
                uno_copyAndConvertData(args[nPos], pArgs[nPos], td, m_from_to.get());
            }
        }
        else
        {
            args[nPos] = pArgs[nPos];
        }
        TYPELIB_DANGER_RELEASE( td );
    }

    uno_Any exc_data;
    uno_Any * exc = &exc_data;

    // do the UNO call...
    uno_Environment_invoke(m_to.get(), s_dispatcher_v, m_pUnoI, pMemberType, ret, args, &exc);

    if (exc == nullptr)
    {
        for (sal_Int32 nPos = 0; nPos < nParams; ++ nPos)
        {
            if (args[nPos] != pArgs[nPos])
            {
                typelib_MethodParameter const & param = pParams[nPos];
                if (param.bOut)
                {
                    if (param.bIn) // is inout
                    {
                        uno_type_destructData(pArgs[nPos], param.pTypeRef, nullptr);
                    }
                    uno_type_copyAndConvertData(pArgs[ nPos ],
                                                args[ nPos ],
                                                param.pTypeRef,
                                                m_to_from.get());
                }
                uno_Environment_invoke(m_to.get(), s_type_destructData_v, args[nPos], param.pTypeRef, 0);
            }
        }
        if (ret != pReturn)
        {
            uno_type_copyAndConvertData(pReturn,
                                        ret,
                                        pReturnTypeRef,
                                        m_to_from.get());

            uno_Environment_invoke(m_to.get(), s_type_destructData_v, ret, pReturnTypeRef, 0);
        }

        *ppException = nullptr;
    }
    else // exception occurred
    {
        for (sal_Int32 nPos = 0; nPos < nParams; ++ nPos)
        {
            if (args[nPos] != pArgs[nPos])
            {
                typelib_MethodParameter const & param = pParams[nPos];
                if (param.bIn)
                {
                    uno_Environment_invoke(m_to.get(), s_type_destructData_v, args[nPos], param.pTypeRef, 0);
                }
            }
        }

        uno_type_any_constructAndConvert(*ppException,
                                         exc->pData,
                                         exc->pType,
                                         m_to_from.get());

        // FIXME: need to destruct in m_to
        uno_any_destruct(exc, nullptr);
    }

    if (m_probeFun)
        m_probeFun(false,
                   this,
                   m_pProbeContext,
                   pReturnTypeRef,
                   pParams,
                   nParams,
                   pMemberType,
                   pReturn,
                   pArgs,
                   ppException);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
