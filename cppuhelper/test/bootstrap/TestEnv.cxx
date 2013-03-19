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

    virtual  ~TestEnv();

public:
    explicit  TestEnv();

protected:
    virtual void v_enter(void);
    virtual void v_leave(void);

    virtual void v_callInto_v(uno_EnvCallee * pCallee, va_list * pParam);
    virtual void v_callOut_v (uno_EnvCallee * pCallee, va_list * pParam);

    virtual int  v_isValid   (rtl::OUString * pReason);
};

TestEnv::TestEnv()
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
        *pReason = rtl::OUString("OK");

    else
        *pReason = rtl::OUString("not entered/invoked");

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
