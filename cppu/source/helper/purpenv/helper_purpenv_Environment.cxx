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


#include "cppu/helper/purpenv/Environment.hxx"

#include "osl/diagnose.h"
#include "uno/lbnames.h"

#include "typelib/typedescription.h"
#include "osl/interlck.h"

#ifdef debug
# define LOG_LIFECYCLE_cppu_helper_purpenv_Base
#endif

#ifdef LOG_LIFECYCLE_cppu_helper_purpenv_Base
#  include <iostream>
#  define LOG_LIFECYCLE_cppu_helper_purpenv_Base_emit(x) x

#else
#  define LOG_LIFECYCLE_cppu_helper_purpenv_Base_emit(x)

#endif


extern "C" {
typedef void SAL_CALL EnvFun_P   (uno_Environment *);
typedef void SAL_CALL EnvFun_PP_P(uno_Environment ** ppHardEnv, uno_Environment *);
typedef void SAL_CALL ExtEnv_registerProxyInterface (uno_ExtEnvironment                 *,
                                                     void                              ** ppProxy,
                                                     uno_freeProxyFunc                    freeProxy,
                                                     rtl_uString * pOId,
                                                     typelib_InterfaceTypeDescription   * pTypeDescr);
typedef void SAL_CALL ExtEnv_revokeInterface        (uno_ExtEnvironment                 *,
                                                     void                               * pInterface);
typedef void SAL_CALL ExtEnv_getObjectIdentifier    (uno_ExtEnvironment                 *,
                                                     rtl_uString                       **,
                                                     void                               *);
typedef void SAL_CALL ExtEnv_getRegisteredInterface (uno_ExtEnvironment                 *,
                                                     void                              **,
                                                     rtl_uString                        *,
                                                     typelib_InterfaceTypeDescription   *);
typedef void SAL_CALL ExtEnv_getRegisteredInterfaces(uno_ExtEnvironment                 *,
                                                     void                             *** pppInterfaces,
                                                     sal_Int32                          * pnLen,
                                                     uno_memAlloc                         memAlloc);
typedef void SAL_CALL ExtEnv_computeObjectIdentifier(uno_ExtEnvironment                 *,
                                                     rtl_uString                       ** ppOId,
                                                     void                               * pInterface);
typedef void SAL_CALL ExtEnv_acquireInterface       (uno_ExtEnvironment                 *,
                                                     void                               * pInterface);
typedef void SAL_CALL ExtEnv_releaseInterface       (uno_ExtEnvironment                 *,
                                                     void                               * pInterface);
}

class Base : public cppu::Enterable
{
public:
    explicit Base(uno_Environment * pEnv, cppu::Enterable * pEnterable);

    void acquireWeak(void);
    void releaseWeak(void);
    void harden     (uno_Environment ** ppHardEnv);
    void acquire    (void);
    void release    (void);

    void registerProxyInterface (void                                  ** ppProxy,
                                 uno_freeProxyFunc                        freeProxy,
                                 rtl::OUString                    const & oid,
                                 typelib_InterfaceTypeDescription       * pTypeDescr);
    void revokeInterface        (void                                   * pInterface);
    void getObjectIdentifier    (void                                   * pInterface,
                                 rtl::OUString                          * pOid);
    void getRegisteredInterface (void                                  **,
                                 rtl::OUString                    const & oid,
                                 typelib_InterfaceTypeDescription       *);
    void getRegisteredInterfaces(void                                 ***,
                                 sal_Int32                              * pnLen,
                                 uno_memAlloc                             memAlloc);
    void computeObjectIdentifier(void                                   * pInterface,
                                 rtl::OUString                          * pOid);
    void acquireInterface       (void                                   * pInterface);
    void releaseInterface       (void                                   * pInterface);

    virtual void v_enter     (void);
    virtual void v_leave     (void);
    virtual void v_callInto_v(uno_EnvCallee * pCallee, va_list * pParam);
    virtual void v_callOut_v (uno_EnvCallee * pCallee, va_list * pParam);
    virtual int  v_isValid   (rtl::OUString * pReason);

protected:
    oslInterlockedCount    m_nRef;
    uno_Environment      * m_pEnv;
    cppu::Enterable      * m_pEnterable;

    EnvFun_P    * m_env_acquire;
    EnvFun_P    * m_env_release;
    EnvFun_PP_P * m_env_harden;
    EnvFun_P    * m_env_acquireWeak;
    EnvFun_P    * m_env_releaseWeak;

    ExtEnv_registerProxyInterface  * m_env_registerProxyInterface;
    ExtEnv_revokeInterface         * m_env_revokeInterface;
    ExtEnv_getObjectIdentifier     * m_env_getObjectIdentifier;
    ExtEnv_getRegisteredInterface  * m_env_getRegisteredInterface;
    ExtEnv_getRegisteredInterfaces * m_env_getRegisteredInterfaces;
    ExtEnv_computeObjectIdentifier * m_env_computeObjectIdentifier;
    ExtEnv_acquireInterface        * m_env_acquireInterface;
    ExtEnv_releaseInterface        * m_env_releaseInterface;

    virtual  ~Base();
};

extern "C" {
static void SAL_CALL s_acquire(uno_Environment * pEnv) //SAL_THROW_EXTERN_C()
{
    Base * pBase = static_cast<Base *>(pEnv->pReserved);
    pBase->acquire();
}

static void SAL_CALL s_release(uno_Environment * pEnv) SAL_THROW_EXTERN_C()
{
    Base * pBase = static_cast<Base *>(pEnv->pReserved);
    pBase->release();
}

static void SAL_CALL s_harden(uno_Environment ** ppHardEnv, uno_Environment * pEnv) SAL_THROW_EXTERN_C()
{
    Base * pBase = static_cast<Base *>(pEnv->pReserved);
    pBase->harden(ppHardEnv);
}

static void SAL_CALL s_acquireWeak(uno_Environment * pEnv) SAL_THROW_EXTERN_C()
{
    Base * pBase = static_cast<Base *>(pEnv->pReserved);
    pBase->acquireWeak();
}

static void SAL_CALL s_releaseWeak(uno_Environment * pEnv) SAL_THROW_EXTERN_C()
{
    Base * pBase = static_cast<Base *>(pEnv->pReserved);
    pBase->releaseWeak();
}


static void SAL_CALL s_registerProxyInterface(uno_ExtEnvironment                * pExtEnv,
                                                         void                             ** ppProxy,
                                                         uno_freeProxyFunc                   freeProxy,
                                                         rtl_uString                       * pOId,
                                                         typelib_InterfaceTypeDescription  * pTypeDescr)
{
    Base * pBase = static_cast<Base *>(pExtEnv->aBase.pReserved);
    pBase->registerProxyInterface(ppProxy, freeProxy, pOId, pTypeDescr);
}

static void SAL_CALL s_revokeInterface(uno_ExtEnvironment * pExtEnv, void * pInterface)
{
    Base * pBase = static_cast<Base *>(pExtEnv->aBase.pReserved);
    pBase->revokeInterface(pInterface);
}

static void SAL_CALL s_getObjectIdentifier(uno_ExtEnvironment *  pExtEnv,
                                                      rtl_uString        ** ppOId,
                                                      void               *  pInterface)
{
    Base * pBase = static_cast<Base *>(pExtEnv->aBase.pReserved);
    pBase->getObjectIdentifier(pInterface, reinterpret_cast<rtl::OUString *>(ppOId));
}

static void SAL_CALL s_getRegisteredInterface(uno_ExtEnvironment *  pExtEnv,
                                                         void               ** ppInterface,
                                                         rtl_uString        *  pOId,
                                                         typelib_InterfaceTypeDescription * pTypeDescr)
{
    Base * pBase = static_cast<Base *>(pExtEnv->aBase.pReserved);
    pBase->getRegisteredInterface(ppInterface, pOId, pTypeDescr);
}

static void SAL_CALL s_getRegisteredInterfaces(uno_ExtEnvironment   * pExtEnv,
                                                          void               *** pppInterface,
                                                          sal_Int32            * pnLen,
                                                          uno_memAlloc           memAlloc)
{
    Base * pBase = static_cast<Base *>(pExtEnv->aBase.pReserved);
    pBase->getRegisteredInterfaces(pppInterface, pnLen, memAlloc);
}

static void SAL_CALL s_computeObjectIdentifier(uno_ExtEnvironment *  pExtEnv,
                                                          rtl_uString        ** ppOId,
                                                          void               *  pInterface)
{
    Base * pBase = static_cast<Base *>(pExtEnv->aBase.pReserved);
    pBase->computeObjectIdentifier(pInterface, reinterpret_cast<rtl::OUString *>(ppOId));
}

static void SAL_CALL s_acquireInterface(uno_ExtEnvironment * pExtEnv, void * pInterface) {
    Base * pBase = static_cast<Base *>(pExtEnv->aBase.pReserved);
    pBase->acquireInterface(pInterface);
}

static void SAL_CALL s_releaseInterface(uno_ExtEnvironment * pExtEnv, void * pInterface) {
    Base * pBase = static_cast<Base *>(pExtEnv->aBase.pReserved);
    pBase->releaseInterface(pInterface);
}

}

Base::Base(uno_Environment * pEnv, cppu::Enterable * pEnterable)
    :m_nRef(1),
     m_pEnv(pEnv),
     m_pEnterable     (pEnterable),
     m_env_acquire    (pEnv->acquire),
     m_env_release    (pEnv->release),
     m_env_harden     (pEnv->harden),
     m_env_acquireWeak(pEnv->acquireWeak),
     m_env_releaseWeak(pEnv->releaseWeak),
     m_env_registerProxyInterface (pEnv->pExtEnv->registerProxyInterface),
     m_env_revokeInterface        (pEnv->pExtEnv->revokeInterface),
     m_env_getObjectIdentifier    (pEnv->pExtEnv->getObjectIdentifier),
     m_env_getRegisteredInterface (pEnv->pExtEnv->getRegisteredInterface),
     m_env_getRegisteredInterfaces(pEnv->pExtEnv->getRegisteredInterfaces),
     m_env_computeObjectIdentifier(pEnv->pExtEnv->computeObjectIdentifier),
     m_env_acquireInterface       (pEnv->pExtEnv->acquireInterface),
     m_env_releaseInterface       (pEnv->pExtEnv->releaseInterface)
{
    LOG_LIFECYCLE_cppu_helper_purpenv_Base_emit(fprintf(stderr, "LIFE: %s -> %p\n", "cppu::helper::purpenv::Base::Base(uno_Environment * pEnv)", this));
    OSL_ENSURE(
        rtl_ustr_ascii_compare_WithLength(pEnv->pTypeName->buffer, rtl_str_getLength(UNO_LB_UNO), UNO_LB_UNO)
            == 0,
            "### wrong environment type!");

    pEnv->acquire     = s_acquire;
    pEnv->release     = s_release;
    pEnv->harden      = s_harden;
    pEnv->acquireWeak = s_acquireWeak;
    pEnv->releaseWeak = s_releaseWeak;

    pEnv->pExtEnv->registerProxyInterface  = s_registerProxyInterface;
    pEnv->pExtEnv->revokeInterface         = s_revokeInterface;
    pEnv->pExtEnv->getObjectIdentifier     = s_getObjectIdentifier;
    pEnv->pExtEnv->getRegisteredInterface  = s_getRegisteredInterface;
    pEnv->pExtEnv->getRegisteredInterfaces = s_getRegisteredInterfaces;
    pEnv->pExtEnv->computeObjectIdentifier = s_computeObjectIdentifier;
    pEnv->pExtEnv->acquireInterface        = s_acquireInterface;
    pEnv->pExtEnv->releaseInterface        = s_releaseInterface;

    pEnv->pReserved = this;
}

Base::~Base()
{
    LOG_LIFECYCLE_cppu_helper_purpenv_Base_emit(fprintf(stderr, "LIFE: %s -> %p\n", "cppu::helper::purpenv::Base::~Base()", this));

    m_pEnv->acquire     = m_env_acquire;
    m_pEnv->release     = m_env_release;
    m_pEnv->harden      = m_env_harden;
    m_pEnv->acquireWeak = m_env_acquireWeak;
    m_pEnv->releaseWeak = m_env_releaseWeak;

    m_pEnv->pReserved = NULL;

    delete m_pEnterable;
    m_pEnv->release(m_pEnv);
}

void Base::acquire(void)
{
    m_env_acquire(m_pEnv);

    osl_atomic_increment(&m_nRef);
}

void Base::release(void)
{
    if (osl_atomic_decrement(&m_nRef) == 0)
        delete this;

    else
        m_env_release(m_pEnv);
}

void Base::harden(uno_Environment ** ppHardEnv)
{
    m_env_harden(ppHardEnv, m_pEnv);
    osl_atomic_increment(&m_nRef);
}

void Base::acquireWeak(void)
{
    m_env_acquireWeak(m_pEnv);
}

void Base::releaseWeak(void)
{
    m_env_releaseWeak(m_pEnv);
}


extern "C" { static void s_registerProxyInterface_v(va_list * pParam)
{
    uno_ExtEnvironment                   * pExtEnv    = va_arg(*pParam, uno_ExtEnvironment *);
    void                                ** ppProxy    = va_arg(*pParam, void **);
    uno_freeProxyFunc                      freeProxy  = va_arg(*pParam, uno_freeProxyFunc);
    rtl_uString                          * pOId       = va_arg(*pParam, rtl_uString *);
    typelib_InterfaceTypeDescription     * pTypeDescr = va_arg(*pParam, typelib_InterfaceTypeDescription *);
    ExtEnv_registerProxyInterface  * pRegisterProxyInterface
        = va_arg(*pParam, ExtEnv_registerProxyInterface *);

    pRegisterProxyInterface(pExtEnv, ppProxy, freeProxy, pOId, pTypeDescr);
}}

void Base::registerProxyInterface(void                                  ** ppProxy,
                                  uno_freeProxyFunc                        freeProxy,
                                  rtl::OUString                    const & oid,
                                  typelib_InterfaceTypeDescription       * pTypeDescr)
{
     uno_Environment_invoke(m_pEnv,
                           s_registerProxyInterface_v,
                           m_pEnv->pExtEnv,
                           ppProxy,
                           freeProxy,
                           oid.pData,
                           pTypeDescr,
                           m_env_registerProxyInterface);
}


extern "C" { static void s_revokeInterface_v(va_list * pParam)
{
    uno_ExtEnvironment * pExtEnv    = va_arg(*pParam, uno_ExtEnvironment *);
    void               * pInterface = va_arg(*pParam, void *);
    ExtEnv_revokeInterface * pRevokeInterface = va_arg(*pParam, ExtEnv_revokeInterface *);

    pRevokeInterface(pExtEnv, pInterface);
}}

void Base::revokeInterface(void * pInterface)
{
     uno_Environment_invoke(m_pEnv,
                           s_revokeInterface_v,
                           m_pEnv->pExtEnv,
                           pInterface,
                           m_env_revokeInterface);
}


extern "C" { static void s_getObjectIdentifier_v(va_list * pParam)
{
    uno_ExtEnvironment * pExtEnv    = va_arg(*pParam, uno_ExtEnvironment *);
    void               * pInterface = va_arg(*pParam, void *);
    rtl::OUString      * pOId       = va_arg(*pParam, rtl::OUString *);
    ExtEnv_getObjectIdentifier * pGetObjectIdentifier
        = va_arg(*pParam, ExtEnv_getObjectIdentifier *);

    pGetObjectIdentifier(pExtEnv, reinterpret_cast<rtl_uString **>(pOId), pInterface);
}}

void Base::getObjectIdentifier(void * pInterface, rtl::OUString * pOid)
{
     uno_Environment_invoke(m_pEnv,
                           s_getObjectIdentifier_v,
                           m_pEnv->pExtEnv,
                           pInterface,
                           pOid,
                           m_env_getObjectIdentifier);
}


extern "C" { static void s_getRegisteredInterface_v(va_list * pParam)
{
    uno_ExtEnvironment                   * pExtEnv     = va_arg(*pParam, uno_ExtEnvironment *);
    void                                ** ppInterface = va_arg(*pParam, void **);
    rtl_uString                          * pOId        = va_arg(*pParam, rtl_uString *);
    typelib_InterfaceTypeDescription     * pTypeDescr  = va_arg(*pParam, typelib_InterfaceTypeDescription *);
    ExtEnv_getRegisteredInterface  * pGetRegisteredInterface
        = va_arg(*pParam, ExtEnv_getRegisteredInterface *);

    pGetRegisteredInterface(pExtEnv, ppInterface, pOId, pTypeDescr);
}}

void Base::getRegisteredInterface(void ** ppInterface,
                                  rtl::OUString const & oid,
                                  typelib_InterfaceTypeDescription * pTypeDescr)
{
     uno_Environment_invoke(m_pEnv,
                           s_getRegisteredInterface_v,
                           m_pEnv->pExtEnv,
                           ppInterface,
                           oid.pData,
                           pTypeDescr,
                           m_env_getRegisteredInterface);
}


extern "C" { static void s_getRegisteredInterfaces_v(va_list * pParam)
{
    uno_ExtEnvironment   * pExtEnv      = va_arg(*pParam, uno_ExtEnvironment *);
    void               *** pppInterface = va_arg(*pParam, void ***);
    sal_Int32            * pnLen        = va_arg(*pParam, sal_Int32 *);
    uno_memAlloc           memAlloc     = va_arg(*pParam, uno_memAlloc);
    ExtEnv_getRegisteredInterfaces * pGetRegisteredInterfaces
        = va_arg(*pParam, ExtEnv_getRegisteredInterfaces *);

    pGetRegisteredInterfaces(pExtEnv, pppInterface, pnLen, memAlloc);
}}

void Base::getRegisteredInterfaces(void         *** pppInterface,
                                   sal_Int32      * pnLen,
                                   uno_memAlloc     memAlloc)
{
     uno_Environment_invoke(m_pEnv,
                           s_getRegisteredInterfaces_v,
                           m_pEnv->pExtEnv,
                           pppInterface,
                           pnLen,
                           memAlloc,
                           m_env_getRegisteredInterfaces);
}


extern "C" { static void s_computeObjectIdentifier_v(va_list * pParam)
{
    uno_ExtEnvironment * pExtEnv    = va_arg(*pParam, uno_ExtEnvironment *);
    void               * pInterface = va_arg(*pParam, void *);
    rtl::OUString      * pOId       = va_arg(*pParam, rtl::OUString *);
    ExtEnv_computeObjectIdentifier * pComputeObjectIdentifier
        = va_arg(*pParam, ExtEnv_computeObjectIdentifier *);

    pComputeObjectIdentifier(pExtEnv, reinterpret_cast<rtl_uString **>(pOId), pInterface);
}}

void Base::computeObjectIdentifier(void * pInterface, rtl::OUString * pOid)
{
     uno_Environment_invoke(m_pEnv,
                           s_computeObjectIdentifier_v,
                           m_pEnv->pExtEnv,
                           pInterface,
                           pOid,
                           m_env_computeObjectIdentifier);
}


extern "C" { static void s_acquireInterface_v(va_list * pParam)
{
    uno_ExtEnvironment * pExtEnv    = va_arg(*pParam, uno_ExtEnvironment *);
    void               * pInterface = va_arg(*pParam, void *);
    ExtEnv_acquireInterface * pAcquireInterface
        = va_arg(*pParam, ExtEnv_acquireInterface *);

    pAcquireInterface(pExtEnv, pInterface);
}}

void Base::acquireInterface(void * pInterface)
{
     uno_Environment_invoke(m_pEnv, s_acquireInterface_v, m_pEnv->pExtEnv, pInterface, m_env_acquireInterface);
}


extern "C" { static void s_releaseInterface_v(va_list * pParam)
{
    uno_ExtEnvironment * pExtEnv    = va_arg(*pParam, uno_ExtEnvironment *);
    void               * pInterface = va_arg(*pParam, void *);
    ExtEnv_releaseInterface * pReleaseInterface
        = va_arg(*pParam, ExtEnv_releaseInterface *);

    pReleaseInterface(pExtEnv, pInterface);
}}

void Base::releaseInterface(void * pInterface)
{
     uno_Environment_invoke(m_pEnv,
                           s_releaseInterface_v,
                           m_pEnv->pExtEnv,
                           pInterface,
                           m_env_releaseInterface);
}

void Base::v_enter(void)
{
    m_pEnterable->enter();
}

void Base::v_leave(void)
{
    m_pEnterable->leave();
}

void Base::v_callInto_v(uno_EnvCallee * pCallee, va_list * pParam)
{
    m_pEnterable->callInto_v(pCallee, pParam);
}

void Base::v_callOut_v(uno_EnvCallee * pCallee, va_list * pParam)
{
    m_pEnterable->callOut_v(pCallee, pParam);
}

int Base::v_isValid(rtl::OUString * pReason)
{
    return m_pEnterable->isValid(pReason);
}

namespace cppu { namespace helper { namespace purpenv {

void Environment_initWithEnterable(uno_Environment * pEnvironment, cppu::Enterable * pEnterable)
{
    new Base(pEnvironment, pEnterable);
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
