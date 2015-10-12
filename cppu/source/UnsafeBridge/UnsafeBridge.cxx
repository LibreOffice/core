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


#include "osl/mutex.hxx"
#include "osl/thread.h"
#include "osl/thread.hxx"
#include "sal/log.hxx"

#include <cppu/Enterable.hxx>
#include "cppu/helper/purpenv/Environment.hxx"
#include "cppu/helper/purpenv/Mapping.hxx"


#ifdef debug
# define LOG_LIFECYCLE_UnsafeBridge
#endif

#ifdef LOG_LIFECYCLE_UnsafeBridge
#  include <iostream>
#  define LOG_LIFECYCLE_UnsafeBridge_emit(x) x

#else
#  define LOG_LIFECYCLE_UnsafeBridge_emit(x)

#endif


class UnsafeBridge : public cppu::Enterable
{
    osl::Mutex          m_mutex;
    sal_Int32           m_count;
    oslThreadIdentifier m_threadId;

    virtual  ~UnsafeBridge();

public:
    explicit UnsafeBridge();

    virtual void v_callInto_v(uno_EnvCallee * pCallee, va_list * pParam) override;
    virtual void v_callOut_v (uno_EnvCallee * pCallee, va_list * pParam) override;

    virtual void v_enter() override;
    virtual void v_leave() override;

    virtual bool v_isValid(rtl::OUString * pReason) override;
};

UnsafeBridge::UnsafeBridge()
    : m_count   (0),
      m_threadId(0)
{
    LOG_LIFECYCLE_UnsafeBridge_emit(fprintf(stderr, "LIFE: %s -> %p\n", "UnsafeBridge::UnsafeBridge(uno_Environment * pEnv)", this));
}

UnsafeBridge::~UnsafeBridge()
{
    LOG_LIFECYCLE_UnsafeBridge_emit(fprintf(stderr, "LIFE: %s -> %p\n", "UnsafeBridge::~UnsafeBridge()", this));

    SAL_WARN_IF(m_count < 0, "cppu.unsafebridge", "m_count is less than 0");
}

void UnsafeBridge::v_callInto_v(uno_EnvCallee * pCallee, va_list * pParam)
{
    enter();
    pCallee(pParam);
    leave();
}

void UnsafeBridge::v_callOut_v(uno_EnvCallee * pCallee, va_list * pParam)
{
    SAL_WARN_IF(m_count <= 0, "cppu.unsafebridge", "m_count is less than or equal to 0");

    -- m_count;
    pCallee(pParam);
    ++ m_count;

    if (!m_threadId)
        m_threadId = osl::Thread::getCurrentIdentifier();
}

void UnsafeBridge::v_enter()
{
    m_mutex.acquire();

    SAL_WARN_IF(m_count < 0, "cppu.unsafebridge", "m_count is less than 0");

    if (m_count == 0)
        m_threadId = osl::Thread::getCurrentIdentifier();

    ++ m_count;
}

void UnsafeBridge::v_leave()
{
    SAL_WARN_IF(m_count <= 0, "cppu.unsafebridge", "m_count is less than or equal to 0");

    -- m_count;
    if (!m_count)
        m_threadId = 0;


    m_mutex.release();
}

bool UnsafeBridge::v_isValid(rtl::OUString * pReason)
{
    bool result = m_count > 0;
    if (!result)
    {
        *pReason = "not entered";
    }
    else
    {
        result = m_threadId == osl::Thread::getCurrentIdentifier();

        if (!result)
            *pReason = "wrong thread";
    }

    if (result)
        *pReason = "OK";

    return result;
}

extern "C" void SAL_DLLPUBLIC_EXPORT SAL_CALL uno_initEnvironment(uno_Environment * pEnv)
    SAL_THROW_EXTERN_C()
{
    cppu::helper::purpenv::Environment_initWithEnterable(pEnv, new UnsafeBridge());
}

extern "C" void SAL_DLLPUBLIC_EXPORT SAL_CALL uno_ext_getMapping(uno_Mapping     ** ppMapping,
                                   uno_Environment  * pFrom,
                                   uno_Environment  * pTo )
{
    cppu::helper::purpenv::createMapping(ppMapping, pFrom, pTo);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
