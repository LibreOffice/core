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


#include <cppu/helper/purpenv/Mapping.hxx>

#include "Proxy.hxx"

#include <osl/interlck.h>
#include <sal/log.hxx>
#include <uno/environment.hxx>
#include <uno/dispatcher.h>

using namespace com::sun::star;

class Mapping : public uno_Mapping
{
    uno::Environment   m_from;
    uno::Environment   m_to;

    oslInterlockedCount m_nCount;

    cppu::helper::purpenv::ProbeFun * m_probeFun;
    void                            * m_pContext;

public:
    explicit  Mapping(uno_Environment                 * pFrom,
                      uno_Environment                 * pTo,
                      cppu::helper::purpenv::ProbeFun * probeFun,
                      void                            * pProbeContext);
    virtual  ~Mapping();

    void mapInterface(
        uno_Interface                    ** ppOut,
        uno_Interface                     * pUnoI,
        typelib_InterfaceTypeDescription  * pTypeDescr);

    void acquire();
    void release();
};

static void s_mapInterface(
    uno_Mapping                       * puno_Mapping,
    void                             ** ppOut,
    void                              * pUnoI,
    typelib_InterfaceTypeDescription  * pTypeDescr )
    SAL_THROW_EXTERN_C()
{
    Mapping * pMapping = static_cast<Mapping *>(puno_Mapping);
    pMapping->mapInterface(
        reinterpret_cast<uno_Interface **>(ppOut),
        static_cast<uno_Interface *>(pUnoI), pTypeDescr);
}

extern "C" {
static void s_acquire(uno_Mapping * puno_Mapping)
    SAL_THROW_EXTERN_C()
{
    Mapping * pMapping = static_cast<Mapping *>(puno_Mapping);
    pMapping->acquire();
}

static void s_release(uno_Mapping * puno_Mapping)
    SAL_THROW_EXTERN_C()
{
    Mapping * pMapping = static_cast<Mapping * >(puno_Mapping);
    pMapping->release();
}


static void s_getIdentifier_v(va_list * pParam)
{
    uno_ExtEnvironment *  pEnv  = va_arg(*pParam, uno_ExtEnvironment *);
    rtl_uString        ** ppOid = va_arg(*pParam, rtl_uString **);
    uno_Interface      *  pUnoI = va_arg(*pParam, uno_Interface *);

    pEnv->getObjectIdentifier(pEnv, ppOid, pUnoI);
}

static void s_free(uno_Mapping * puno_Mapping)
    SAL_THROW_EXTERN_C()
{
    Mapping * pMapping = static_cast<Mapping *>(puno_Mapping);
    delete pMapping;
}
}

Mapping::Mapping(uno_Environment                 * pFrom,
                 uno_Environment                 * pTo,
                 cppu::helper::purpenv::ProbeFun * probeFun,
                 void                            * pProbeContext
)
    : m_from    (pFrom),
      m_to      (pTo),
      m_nCount  (1),
      m_probeFun(probeFun),
      m_pContext(pProbeContext)
{
    SAL_INFO("cppu.purpenv", "LIFE: Mapping::Mapping(uno_Environment * pFrom, uno_Environment * pTo -> " << this);

    uno_Mapping::acquire      = s_acquire;
    uno_Mapping::release      = s_release;
    uno_Mapping::mapInterface = s_mapInterface;
}

Mapping::~Mapping()
{
    SAL_INFO("cppu.purpenv", "LIFE: Mapping:~Mapping() -> " << this);
}


void Mapping::mapInterface(
    uno_Interface                    ** ppOut,
    uno_Interface                     * pUnoI,
    typelib_InterfaceTypeDescription  * pTypeDescr)
{
    OSL_ASSERT(ppOut && pTypeDescr);
    if (*ppOut)
    {
        (*ppOut)->release(*ppOut);
        *ppOut = nullptr;
    }

    if (!pUnoI)
        return;

    // get object id of uno interface to be wrapped
    // need to enter environment because of potential "queryInterface" call
    rtl_uString * pOId = nullptr;
    uno_Environment_invoke(m_from.get(), s_getIdentifier_v, m_from.get(), &pOId, pUnoI);
    OSL_ASSERT(pOId);

     // try to get any known interface from target environment
    m_to.get()->pExtEnv->getRegisteredInterface(m_to.get()->pExtEnv, reinterpret_cast<void **>(ppOut), pOId, pTypeDescr);

    if (!*ppOut) // not yet there, register new proxy interface
    {
        // try to publish a new proxy (ref count initially 1)
        uno_Interface * pProxy = new Proxy(this,
                                           m_from.get(),
                                           m_to.get(),
                                           pUnoI,
                                           pTypeDescr,
                                           pOId,
                                           m_probeFun,
                                           m_pContext);

        // proxy may be exchanged during registration
        m_to.get()->pExtEnv->registerProxyInterface(m_to.get()->pExtEnv,
                                                    reinterpret_cast<void **>(&pProxy),
                                                    Proxy_free,
                                                    pOId,
                                                    pTypeDescr);

        *ppOut = pProxy;
    }

    rtl_uString_release(pOId);
}


void Mapping::acquire()
{
    if (osl_atomic_increment(&m_nCount) == 1)
    {
        uno_Mapping * pMapping = this;

        ::uno_registerMapping(&pMapping, s_free, m_from.get(), m_to.get(), nullptr);
    }
}

void Mapping::release()
{
    if (osl_atomic_decrement(&m_nCount) == 0)
        ::uno_revokeMapping(this);
}


namespace cppu { namespace helper { namespace purpenv {

void createMapping(uno_Mapping     ** ppMapping,
                   uno_Environment  * pFrom,
                   uno_Environment  * pTo,
                   ProbeFun         * probeFun,
                   void             * pContext
 )
{
    *ppMapping = new Mapping(pFrom, pTo, probeFun, pContext);

    ::uno_registerMapping(ppMapping, s_free, pFrom, pTo, nullptr);
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
