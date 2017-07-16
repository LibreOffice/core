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


#include "osl/thread.hxx"
#include "osl/conditn.hxx"
#include "osl/mutex.hxx"
#include <osl/diagnose.h>

#include <cppu/Enterable.hxx>
#include "cppu/helper/purpenv/Environment.hxx"
#include "cppu/helper/purpenv/Mapping.hxx"


class InnerThread;
class OuterThread;

class AffineBridge : public cppu::Enterable
{
public:
    enum Msg
    {
        CB_DONE,
        CB_FPOINTER
    };

    Msg                   m_message;
    uno_EnvCallee       * m_pCallee;
    va_list             * m_pParam;

    osl::Mutex            m_innerMutex;
    oslThreadIdentifier   m_innerThreadId;
    InnerThread         * m_pInnerThread;
    osl::Condition        m_innerCondition;
    sal_Int32             m_enterCount;

    osl::Mutex            m_outerMutex;
    oslThreadIdentifier   m_outerThreadId;
    osl::Condition        m_outerCondition;
    OuterThread         * m_pOuterThread;

    explicit  AffineBridge();
    virtual  ~AffineBridge() override;

    virtual void  v_callInto_v(uno_EnvCallee * pCallee, va_list * pParam) override;
    virtual void  v_callOut_v (uno_EnvCallee * pCallee, va_list * pParam) override;

    virtual void  v_enter() override;
    virtual void  v_leave() override;

    virtual bool v_isValid(rtl::OUString * pReason) override;

    void innerDispatch();
    void outerDispatch(bool loop);
};

class InnerThread : public osl::Thread
{
    virtual void SAL_CALL run() override;

    AffineBridge * m_pAffineBridge;

public:
    explicit InnerThread(AffineBridge * threadEnvironment)
        : m_pAffineBridge(threadEnvironment)
        {
            create();
        }
};

void InnerThread::run()
{
    osl_setThreadName("UNO AffineBridge InnerThread");

    m_pAffineBridge->enter();
    m_pAffineBridge->innerDispatch();
    m_pAffineBridge->leave();
}

class OuterThread : public osl::Thread
{
    virtual void SAL_CALL run() override;

    AffineBridge * m_pAffineBridge;

public:
    explicit OuterThread(AffineBridge * threadEnvironment);
};

OuterThread::OuterThread(AffineBridge * threadEnvironment)
    : m_pAffineBridge(threadEnvironment)
{
    create();
}

void OuterThread::run()
{
    osl_setThreadName("UNO AffineBridge OuterThread");

    osl::MutexGuard guard(m_pAffineBridge->m_outerMutex);

    m_pAffineBridge->m_outerThreadId = getIdentifier();
    m_pAffineBridge->outerDispatch(false);
    m_pAffineBridge->m_outerThreadId = 0;

    m_pAffineBridge->m_pOuterThread = nullptr;
    m_pAffineBridge = nullptr;
}


AffineBridge::AffineBridge()
    : m_message      (CB_DONE),
      m_pCallee      (nullptr),
      m_pParam       (nullptr),
      m_innerThreadId(0),
      m_pInnerThread (nullptr),
      m_enterCount   (0),
      m_outerThreadId(0),
      m_pOuterThread (nullptr)
{
    SAL_INFO("cppu.affinebridge", "LIFE: AffineBridge::AffineBridge(uno_Environment * pEnv) -> " << this);
}

AffineBridge::~AffineBridge()
{
    SAL_INFO("cppu.affinebridge", "LIFE: AffineBridge::~AffineBridge() -> " << this);

    if (m_pInnerThread && osl::Thread::getCurrentIdentifier() != m_innerThreadId)
    {
        m_message = CB_DONE;
        m_innerCondition.set();

        m_pInnerThread->join();
    }

    delete m_pInnerThread;

    if (m_pOuterThread)
    {
        m_pOuterThread->join();
        delete m_pOuterThread;
    }
}


void AffineBridge::outerDispatch(bool loop)
{
    OSL_ASSERT(m_outerThreadId == osl::Thread::getCurrentIdentifier());
    OSL_ASSERT(m_innerThreadId != m_outerThreadId);

    Msg mm;

    do
    {
        // FIXME: created outer thread must not wait
        // in case of no message
        // note: no message can happen in case newly created
        // outer thread acquire outerMutex after a real outer
        // thread enters outerDispatch!
        m_outerCondition.wait();
        m_outerCondition.reset();

        mm = m_message;

        switch(mm)
        {
        case CB_DONE:
            break;

        case CB_FPOINTER:
        {
            m_pCallee(m_pParam);

            m_message = CB_DONE;
            m_innerCondition.set();
            break;
        }
        default:
            abort();
        }
    }
    while(mm != CB_DONE && loop);
}

void AffineBridge::innerDispatch()
{
    OSL_ASSERT(m_innerThreadId == osl::Thread::getCurrentIdentifier());
    OSL_ASSERT(m_innerThreadId != m_outerThreadId);

    Msg mm;

    do
    {
        m_innerCondition.wait();
        m_innerCondition.reset();

        mm = m_message;

        switch(mm)
        {
        case CB_DONE:
            break;

        case CB_FPOINTER:
        {
            m_pCallee(m_pParam);

            m_message = CB_DONE;
            m_outerCondition.set();
            break;
        }
        default:
            abort();
        }
    }
    while(mm != CB_DONE);
}

void AffineBridge::v_callInto_v(uno_EnvCallee * pCallee, va_list * pParam)
{
    osl::MutexGuard guard(m_outerMutex); // only one thread at a time can call into

    if (m_innerThreadId == 0) // no inner thread yet
    {
        m_pInnerThread  = new InnerThread(this);
        m_pInnerThread->resume();
    }

    bool bResetId = false;
    if (!m_outerThreadId)
    {
        m_outerThreadId = osl::Thread::getCurrentIdentifier();
        bResetId = true;
    }

    m_message = CB_FPOINTER;
    m_pCallee = pCallee;
    m_pParam  = pParam;
    m_innerCondition.set();

    outerDispatch(true);

    if (bResetId)
        m_outerThreadId = 0;
}

void AffineBridge::v_callOut_v(uno_EnvCallee * pCallee, va_list * pParam)
{
    OSL_ASSERT(m_innerThreadId);

    osl::MutexGuard guard(m_innerMutex);

    if (m_outerThreadId == 0) // no outer thread yet
    {
        osl::MutexGuard guard_m_outerMutex(m_outerMutex);

        if (m_outerThreadId == 0)
        {
            if (m_pOuterThread)
            {
                m_pOuterThread->join();
                delete m_pOuterThread;
            }

            m_pOuterThread = new OuterThread(this);
        }
    }

    m_message = CB_FPOINTER;
    m_pCallee = pCallee;
    m_pParam  = pParam;
    m_outerCondition.set();

    innerDispatch();
}

void AffineBridge::v_enter()
{
    m_innerMutex.acquire();

    if (!m_enterCount)
        m_innerThreadId = osl::Thread::getCurrentIdentifier();

    OSL_ASSERT(m_innerThreadId == osl::Thread::getCurrentIdentifier());

    ++ m_enterCount;
}

void AffineBridge::v_leave()
{
    OSL_ASSERT(m_innerThreadId == osl::Thread::getCurrentIdentifier());

    -- m_enterCount;
    if (!m_enterCount)
        m_innerThreadId = 0;

    m_innerMutex.release();
}

bool AffineBridge::v_isValid(rtl::OUString * pReason)
{
    bool result = m_enterCount > 0;
    if (!result)
        *pReason = "not entered";

    else
    {
        result = m_innerThreadId == osl::Thread::getCurrentIdentifier();

        if (!result)
            *pReason = "wrong thread";
    }

    if (result)
        *pReason = "OK";

    return result;
}

#ifdef DISABLE_DYNLOADING

#define uno_initEnvironment affine_uno_uno_initEnvironment
#define uno_ext_getMapping affine_uno_uno_ext_getMapping

#endif

extern "C" void SAL_DLLPUBLIC_EXPORT SAL_CALL uno_initEnvironment(uno_Environment * pEnv)
    SAL_THROW_EXTERN_C()
{
    cppu::helper::purpenv::Environment_initWithEnterable(pEnv, new AffineBridge());
}

extern "C" void SAL_DLLPUBLIC_EXPORT SAL_CALL uno_ext_getMapping(uno_Mapping     ** ppMapping,
                                                        uno_Environment  * pFrom,
                                                        uno_Environment  * pTo )
{
    cppu::helper::purpenv::createMapping(ppMapping, pFrom, pTo);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
