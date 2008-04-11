/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: UnsafeBridge.cxx,v $
 * $Revision: 1.5 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppu.hxx"

#include "osl/mutex.hxx"
#include "osl/thread.h"

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


class SAL_DLLPRIVATE UnsafeBridge : public cppu::Enterable
{
    osl::Mutex          m_mutex;
    sal_Int32           m_count;
    oslThreadIdentifier m_threadId;

    virtual  ~UnsafeBridge(void);

public:
    explicit UnsafeBridge(void);

    virtual void v_callInto_v(uno_EnvCallee * pCallee, va_list * pParam);
    virtual void v_callOut_v (uno_EnvCallee * pCallee, va_list * pParam);

    virtual void v_enter(void);
    virtual void v_leave(void);

    virtual int  v_isValid(rtl::OUString * pReason);
};

UnsafeBridge::UnsafeBridge(void)
    : m_count   (0),
      m_threadId(0)
{
    LOG_LIFECYCLE_UnsafeBridge_emit(fprintf(stderr, "LIFE: %s -> %p\n", "UnsafeBridge::UnsafeBridge(uno_Environment * pEnv)", this));
}

UnsafeBridge::~UnsafeBridge(void)
{
    LOG_LIFECYCLE_UnsafeBridge_emit(fprintf(stderr, "LIFE: %s -> %p\n", "UnsafeBridge::~UnsafeBridge(void)", this));

    OSL_ASSERT(m_count >= 0);
}

void UnsafeBridge::v_callInto_v(uno_EnvCallee * pCallee, va_list * pParam)
{
    enter();
    pCallee(pParam);
    leave();
}

void UnsafeBridge::v_callOut_v(uno_EnvCallee * pCallee, va_list * pParam)
{
    OSL_ASSERT(m_count > 0);

    -- m_count;
    pCallee(pParam);
    ++ m_count;

    if (!m_threadId)
        m_threadId = osl_getThreadIdentifier(NULL);
}

void UnsafeBridge::v_enter(void)
{
    m_mutex.acquire();

    OSL_ASSERT(m_count >= 0);

    if (m_count == 0)
        m_threadId = osl_getThreadIdentifier(NULL);

    ++ m_count;
}

void UnsafeBridge::v_leave(void)
{
    OSL_ASSERT(m_count > 0);

    -- m_count;
    if (!m_count)
        m_threadId = 0;


    m_mutex.release();
}

int UnsafeBridge::v_isValid(rtl::OUString * pReason)
{
    int result = 1;

    result = m_count > 0;
    if (!result)
        *pReason = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("not entered"));

    else
    {
        result = m_threadId == osl_getThreadIdentifier(NULL);

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
    cppu::helper::purpenv::Environment_initWithEnterable(pEnv, new UnsafeBridge());
}

extern "C" void SAL_DLLPUBLIC_EXPORT SAL_CALL uno_ext_getMapping(uno_Mapping     ** ppMapping,
                                   uno_Environment  * pFrom,
                                   uno_Environment  * pTo )
{
    cppu::helper::purpenv::createMapping(ppMapping, pFrom, pTo);
}

