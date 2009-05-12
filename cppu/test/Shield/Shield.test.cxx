/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Shield.test.cxx,v $
 * $Revision: 1.3 $
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

#include <iostream>

#include "sal/main.h"

#include "uno/environment.hxx"

#include "cppu/Shield.hxx"
#include "cppu/EnvGuards.hxx"

#include "../ObjectFactory/ObjectFactory.hxx"


using namespace com::sun::star;


static rtl::OUString    s_comment;
static uno::Environment s_env;

extern "C" {
static void s_callee_in(rtl_uString *  pMethod_name)
{
    rtl::OUString method_name(pMethod_name);

    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\ts_callee_in  method:\""));
    s_comment += method_name;

    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\" env: \""));
    s_comment += uno::Environment::getCurrent().getTypeName();
    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\""));

    if (!s_env.is())
    {
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" - FAILURE: s_env not set"));
        return;
    }

    rtl::OUString reason;
    int valid = s_env.isValid(&reason);

    if (valid)
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));

    else
    {
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" - FAILURE: "));
        s_comment += reason;
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));
    }
}

static void s_callee_out(rtl_uString *  pMethod_name)
{
    rtl::OUString method_name(pMethod_name);

    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\ts_callee_out method:\""));
    s_comment += method_name;

    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\" env: \""));
    s_comment += uno::Environment::getCurrent().getTypeName();
    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\""));

    if (!s_env.is())
    {
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" - FAILURE: s_env not set"));
        return;
    }

    rtl::OUString reason;
    int valid = s_env.isValid(&reason);

    if (!valid)
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));

    else
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" - FAILURE: is in\n"));
}
}

static uno::Reference<uno::XInterface> s_get_envObject(void)
{
    cppu::EnvGuard envGuard(s_env);

    uno::XInterface * pObject = reinterpret_cast<uno::XInterface *>(
        createObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV))),
                     s_callee_in));

    return uno::Reference<uno::XInterface>(pObject, SAL_NO_ACQUIRE);
}

static uno::XInterface * s_x_get_flatObject(void)
{
    uno::XInterface * pObject = reinterpret_cast<uno::XInterface *>(
        createObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV))),
                     s_callee_out));

    return pObject;
}

static uno::Reference<uno::XInterface> s_get_flatObject(void)
{
    return uno::Reference<uno::XInterface>(s_x_get_flatObject(), SAL_NO_ACQUIRE);
}


static void s_test__shield(void)
{
    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test__shield:\n"));

    uno::Reference<uno::XInterface> obj;

    {
        cppu::EnvGuard envGuard(s_env);
        obj.set(cppu::shield(s_get_envObject().get()), SAL_NO_ACQUIRE);
    }

     obj->acquire();
     obj->release();
}

static void s_test__unshield(void)
{
    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test__unshield:\n"));

    uno::Reference<uno::XInterface> obj;

    {
        cppu::EnvGuard envGuard(s_env);
        uno::XInterface * pObj = s_x_get_flatObject();
        obj.set(cppu::unshield(pObj), SAL_NO_ACQUIRE);

        envGuard.clear();
        pObj->release();
    }

    {
        cppu::EnvGuard envGuard(s_env);
        obj.clear();
    }
}

static void s_test__shieldAny(void)
{
    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test__shieldAny:\n"));

    uno::Any out;

    {
        cppu::EnvGuard envGuard(s_env);
        uno::Any any;
        any <<= s_get_envObject();
        cppu::shieldAny(any, &out);
    }

    uno::Reference<uno::XInterface> obj;
    out >>= obj;

     obj->acquire();
     obj->release();
}

static void s_test__unshieldAny(void)
{
    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test__unshieldAny:\n"));

    uno::Any out;
    {
        uno::Any any(s_get_flatObject());

        {
            cppu::EnvGuard envGuard(s_env);
            cppu::unshieldAny(any, &out);
        }
    }

    {
        cppu::EnvGuard envGuard(s_env);

        uno::Reference<uno::XInterface> obj;
        out >>= obj;

        obj.clear();
    }
}


SAL_IMPLEMENT_MAIN_WITH_ARGS(/*argc*/, argv)
{
    uno::Environment(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO))).enter();

    s_env = uno::Environment(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("uno:unsafe")));

    s_test__shield();
      s_test__shieldAny();
       s_test__unshield();
       s_test__unshieldAny();

    s_env.clear();


    uno_Environment_enter(NULL);


    int ret;
    if (s_comment.indexOf(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FAILURE"))) == -1)
    {
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TESTS PASSED\n"));
        ret = 0;
    }
    else
    {
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TESTS _NOT_ PASSED\n"));
        ret = -1;
    }

    std::cerr
        << argv[0]
        << std::endl
        << rtl::OUStringToOString(s_comment, RTL_TEXTENCODING_ASCII_US).getStr()
        << std::endl;

    return ret;
}
