/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ProbeEnv.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-18 12:23:21 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppu.hxx"

#include "cppu/EnvDcp.hxx"

#include "cppu/helper/purpenv/Environment.hxx"
#include "cppu/helper/purpenv/Mapping.hxx"

#include "EnvStack.tester.hxx"


#define LOG_LIFECYCLE_TestEnv
#ifdef LOG_LIFECYCLE_TestEnv
#  include <iostream>
#  define LOG_LIFECYCLE_TestEnv_emit(x) x

#else
#  define LOG_LIFECYCLE_TestEnv_emit(x)

#endif


class SAL_DLLPRIVATE TestEnv : public cppu::Enterable
{
    virtual  ~TestEnv(void);

public:
    explicit  TestEnv(uno_Environment * pEnv);

    uno_Environment * m_pEnv;

protected:
    virtual void v_enter(void);
    virtual void v_leave(void);

    virtual void v_callInto_v(uno_EnvCallee * pCallee, va_list * pParam);
    virtual void v_callOut_v (uno_EnvCallee * pCallee, va_list * pParam);

    virtual int  v_isValid   (rtl::OUString * pReason);
};

TestEnv::TestEnv(uno_Environment * pEnv)
    : m_pEnv(pEnv)
{
    LOG_LIFECYCLE_TestEnv_emit(fprintf(stderr, "LIFE: %s -> %p\n", "TestEnv::TestEnv(...)", this));
}

TestEnv::~TestEnv(void)
{
    LOG_LIFECYCLE_TestEnv_emit(fprintf(stderr, "LIFE: %s -> %p\n", "TestEnv::~TestEnv(void)", this));
}


static void s_checkGEnvValidity(void)
{
    if (g_env.is())
    {
        rtl::OUString reason;
        int result = g_env.isValid(&reason);

        g_commentStack += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("["));
        g_commentStack += rtl::OUString::valueOf((sal_Int32)result);
        g_commentStack += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
        g_commentStack += reason;
        g_commentStack += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("]"));
    }
}

void TestEnv::v_callInto_v(uno_EnvCallee * pCallee, va_list * pParam)
{
    g_commentStack += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-into"));
    g_commentStack += cppu::EnvDcp::getPurpose(m_pEnv->pTypeName);

    s_checkGEnvValidity();

    pCallee(pParam);
}

void TestEnv::v_callOut_v(uno_EnvCallee * pCallee, va_list * pParam)
{
    g_commentStack += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-out"));
    g_commentStack += cppu::EnvDcp::getPurpose(m_pEnv->pTypeName);

    s_checkGEnvValidity();

    pCallee(pParam);
}

void TestEnv::v_enter(void)
{
    g_commentStack += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-enter"));
    g_commentStack += cppu::EnvDcp::getPurpose(m_pEnv->pTypeName);

    s_checkGEnvValidity();
}

void TestEnv::v_leave(void)
{
    g_commentStack += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("-leave"));
    g_commentStack += cppu::EnvDcp::getPurpose(m_pEnv->pTypeName);

    s_checkGEnvValidity();
}

int  TestEnv::v_isValid(rtl::OUString * /*pReason*/)
{
    return 1;
}

extern "C" void SAL_CALL uno_initEnvironment(uno_Environment * pEnv) SAL_THROW_EXTERN_C()
{
    cppu::helper::purpenv::Environment_initWithEnterable(pEnv, new TestEnv(pEnv));
}

extern "C" void uno_ext_getMapping(uno_Mapping     ** ppMapping,
                                   uno_Environment  * pFrom,
                                   uno_Environment  * pTo )
{
    cppu::helper::purpenv::createMapping(ppMapping, pFrom, pTo);
}

