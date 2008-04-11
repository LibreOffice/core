/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TestEnv.cxx,v $
 * $Revision: 1.4 $
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


#include "cppu/EnvDcp.hxx"

#include "cppu/helper/purpenv/Environment.hxx"
#include "cppu/helper/purpenv/Mapping.hxx"



#define LOG_LIFECYCLE_TestEnv
#ifdef LOG_LIFECYCLE_TestEnv
#  include <iostream>
#  define LOG_LIFECYCLE_TestEnv_emit(x) x

#else
#  define LOG_LIFECYCLE_TestEnv_emit(x)

#endif


class SAL_DLLPRIVATE TestEnv : public cppu::Enterable
{
    int m_inCount;

    virtual  ~TestEnv(void);

public:
    explicit  TestEnv(void);

protected:
    virtual void v_enter(void);
    virtual void v_leave(void);

    virtual void v_callInto_v(uno_EnvCallee * pCallee, va_list * pParam);
    virtual void v_callOut_v (uno_EnvCallee * pCallee, va_list * pParam);

    virtual int  v_isValid   (rtl::OUString * pReason);
};

TestEnv::TestEnv(void)
    : m_inCount(0)
{
    LOG_LIFECYCLE_TestEnv_emit(fprintf(stderr, "LIFE: %s -> %p\n", "TestEnv::TestEnv(...)", this));
}

TestEnv::~TestEnv(void)
{
    LOG_LIFECYCLE_TestEnv_emit(fprintf(stderr, "LIFE: %s -> %p\n", "TestEnv::~TestEnv(void)", this));
}


void TestEnv::v_callInto_v(uno_EnvCallee * pCallee, va_list * pParam)
{
    ++ m_inCount;
    pCallee(pParam);
    -- m_inCount;
}

void TestEnv::v_callOut_v(uno_EnvCallee * pCallee, va_list * pParam)
{
    -- m_inCount;
    pCallee(pParam);
    ++ m_inCount;
}

void TestEnv::v_enter(void)
{
    ++ m_inCount;
}

void TestEnv::v_leave(void)
{
    -- m_inCount;
}

int  TestEnv::v_isValid(rtl::OUString * pReason)
{
    int result = m_inCount & 1;

    if (result)
        *pReason = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OK"));

    else
        *pReason = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("not entered/invoked"));

    return result;
}

extern "C" void SAL_CALL uno_initEnvironment(uno_Environment * pEnv) SAL_THROW_EXTERN_C()
{
    cppu::helper::purpenv::Environment_initWithEnterable(pEnv, new TestEnv());
}

extern "C" void uno_ext_getMapping(uno_Mapping     ** ppMapping,
                                   uno_Environment  * pFrom,
                                   uno_Environment  * pTo )
{
    cppu::helper::purpenv::createMapping(ppMapping, pFrom, pTo);
}

