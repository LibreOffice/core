/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AffineBridge.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:36:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "osl/thread.hxx"
#include "osl/conditn.hxx"
#include "osl/mutex.hxx"

#include "cppu/helper/purpenv/Environment.hxx"
#include "cppu/helper/purpenv/Mapping.hxx"


#ifdef debug
# define LOG_LIFECYCLE_AffineBridge
#endif

#ifdef LOG_LIFECYCLE_AffineBridge
#  include <iostream>
#  define LOG_LIFECYCLE_AffineBridge_emit(x) x

#else
#  define LOG_LIFECYCLE_AffineBridge_emit(x)

#endif

class InnerThread;
class OuterThread;

class SAL_DLLPRIVATE AffineBridge : public cppu::Enterable
{
public:
    enum Msg
    {
        CB_DONE,
        CB_FPOINTER
    };

    Msg                   m_message;
    uno_EnvCallee       * m_pCallee;
    va_list               m_param;

    osl::Mutex            m_innerMutex;
    oslThreadIdentifier   m_innerThreadId;
    InnerThread         * m_pInnerThread;
    osl::Condition        m_innerCondition;
    sal_Int32             m_enterCount;

    osl::Mutex            m_outerMutex;
    oslThreadIdentifier   m_outerThreadId;
    osl::Condition        m_outerCondition;
    OuterThread         * m_pOuterThread;

    explicit  AffineBridge(void);
    virtual  ~AffineBridge(void);

    virtual void  v_callInto_v(uno_EnvCallee * pCallee, va_list param);
    virtual void  v_callOut_v (uno_EnvCallee * pCallee, va_list param);

    virtual void  v_enter(void);
    virtual void  v_leave(void);

    virtual int  v_isValid(rtl::OUString * pReason);

    void innerDispatch(void);
    void outerDispatch(int loop);
};

class SAL_DLLPRIVATE InnerThread : public osl::Thread
{
    virtual void SAL_CALL run(void);

    AffineBridge * m_pAffineBridge;

public:
    InnerThread(AffineBridge * threadEnvironment)
        : m_pAffineBridge(threadEnvironment)
        {
            create();
        }
};

void InnerThread::run(void)
{
    m_pAffineBridge->enter();
    m_pAffineBridge->innerDispatch();
    m_pAffineBridge->leave();
}

class SAL_DLLPRIVATE OuterThread : public osl::Thread
{
    virtual void SAL_CALL run(void);

    AffineBridge * m_pAffineBridge;

public:
    OuterThread(AffineBridge * threadEnvironment);
};

OuterThread::OuterThread(AffineBridge * threadEnvironment)
    : m_pAffineBridge(threadEnvironment)
{
    create();
}

void OuterThread::run(void)
{
    osl::MutexGuard guard(m_pAffineBridge->m_outerMutex);

    m_pAffineBridge->m_outerThreadId = getIdentifier();
    m_pAffineBridge->outerDispatch(0);
    m_pAffineBridge->m_outerThreadId = 0;

    m_pAffineBridge->m_pOuterThread = NULL;
    m_pAffineBridge = NULL;
}


AffineBridge::AffineBridge(void)
    : m_innerThreadId(0),
      m_pInnerThread (NULL),
      m_enterCount   (0),
      m_outerThreadId(0),
      m_pOuterThread (NULL)
{
    LOG_LIFECYCLE_AffineBridge_emit(fprintf(stderr, "LIFE: %s -> %p\n", "AffineBridge::AffineBridge(uno_Environment * pEnv)", this));
}

AffineBridge::~AffineBridge(void)
{
    LOG_LIFECYCLE_AffineBridge_emit(fprintf(stderr, "LIFE: %s -> %p\n", "AffineBridge::~AffineBridge(void)", this));

    if (m_pInnerThread && osl_getThreadIdentifier(NULL) != m_innerThreadId)
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


void AffineBridge::outerDispatch(int loop)
{
    OSL_ASSERT(m_outerThreadId == osl_getThreadIdentifier(NULL));
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
            m_pCallee(m_param);

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

void AffineBridge::innerDispatch(void)
{
    OSL_ASSERT(m_innerThreadId == osl_getThreadIdentifier(NULL));
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
            m_pCallee(m_param);

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

void AffineBridge::v_callInto_v(uno_EnvCallee * pCallee, va_list param)
{
    osl::MutexGuard guard(m_outerMutex); // only one thread at a time can call into

    if (m_innerThreadId == 0) // no inner thread yet
    {
        m_pInnerThread  = new InnerThread(this);
        m_pInnerThread->resume();
    }

    bool resetId = false;
    if (!m_outerThreadId)
    {
        m_outerThreadId = osl_getThreadIdentifier(NULL);
        resetId = true;
    }

    m_message = CB_FPOINTER;
    m_pCallee = pCallee;
    m_param   = param;
    m_innerCondition.set();

    outerDispatch(1);

    if (resetId)
        m_outerThreadId = 0;
}

void AffineBridge::v_callOut_v(uno_EnvCallee * pCallee, va_list param)
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
    m_param   = param;
    m_outerCondition.set();

    innerDispatch();
}

void AffineBridge::v_enter(void)
{
    m_innerMutex.acquire();

    if (!m_enterCount)
        m_innerThreadId = osl_getThreadIdentifier(NULL);

    OSL_ASSERT(m_innerThreadId == osl_getThreadIdentifier(NULL));

    ++ m_enterCount;
}

void AffineBridge::v_leave(void)
{
    OSL_ASSERT(m_innerThreadId == osl_getThreadIdentifier(NULL));

    -- m_enterCount;
    if (!m_enterCount)
        m_innerThreadId = 0;

    m_innerMutex.release();
}

int  AffineBridge::v_isValid(rtl::OUString * pReason)
{
    int result = 1;

    result = m_enterCount > 0;
    if (!result)
        *pReason = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("not entered"));

    else
    {
        result = m_innerThreadId == osl_getThreadIdentifier(NULL);

        if (!result)
            *pReason = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("wrong thread"));
    }

    if (result)
        *pReason = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OK"));

    return result;
}

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

