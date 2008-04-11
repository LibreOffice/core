/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FreeReference.test.cxx,v $
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

#include "cppu/FreeReference.hxx"
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

    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\" env: "));
    s_comment += uno::Environment::getCurrent().getTypeName();

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

    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\" env: "));
    s_comment += uno::Environment::getCurrent().getTypeName();

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
    {
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" - FAILURE: is in\n"));
    }
}
}

static cppu::FreeReference<uno::XInterface> s_get_envObject(void)
{
    cppu::EnvGuard envGuard(s_env);

    uno::XInterface * pObject = reinterpret_cast<uno::XInterface *>(
        createObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV))),
                     s_callee_in));

    cppu::FreeReference<uno::XInterface> env_obj(pObject, SAL_NO_ACQUIRE);

    return env_obj;
}

static cppu::FreeReference<uno::XInterface> s_get_flatObject(void)
{
    uno::XInterface * pObject = reinterpret_cast<uno::XInterface *>(
        createObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV))),
                     s_callee_out));

    return cppu::FreeReference<uno::XInterface>(pObject, SAL_NO_ACQUIRE);
}


static void s_test_operator_arrow(void)
{
    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_operator_arrow:\n"));

    cppu::FreeReference<uno::XInterface> env_obj(s_get_envObject());

    env_obj->acquire();
    env_obj->release();
}


static void s_test_operator_assign_empty_reference(void)
{
    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_operator_assign_empty_reference\n"));

    cppu::FreeReference<uno::XInterface> flat_obj(s_get_flatObject());
    cppu::FreeReference<uno::XInterface> empty_ref;

    flat_obj = empty_ref;
}

static void s_test_operator_assign_inner_reference_inside(void)
{
    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_operator_assign_inner_reference_inside\n"));

    cppu::FreeReference<uno::XInterface> env_obj(s_get_envObject());
    cppu::FreeReference<uno::XInterface> flat_obj(s_get_flatObject());

    {
        cppu::EnvGuard envGuard(s_env);
        env_obj = flat_obj;
    }
}

static void s_test_method_clear_inner_reference_inside(void)
{
    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_method_clear_inner_reference_inside\n"));

    cppu::FreeReference<uno::XInterface> env_obj(s_get_envObject());

    {
        cppu::EnvGuard envGuard(s_env);
        env_obj.clear();
    }
}

static void s_test_method_clear_inner_reference_outside(void)
{
    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_method_clear_inner_reference_outside\n"));

    cppu::FreeReference<uno::XInterface> env_obj(s_get_envObject());

    env_obj.clear();
}

static void s_test_method_clear_outer_reference_inside(void)
{
    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_method_clear_outer_reference_inside\n"));

    cppu::FreeReference<uno::XInterface> flat_obj(s_get_flatObject());

    {
        cppu::EnvGuard envGuard(s_env);
        flat_obj.clear();
    }
}

static void s_test_method_clear_outer_reference_outside(void)
{
    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_method_clear_outer_reference_outside\n"));

    cppu::FreeReference<uno::XInterface> flat_obj(s_get_flatObject());

    flat_obj.clear();
}

static void s_test_method_set_inside_with_inner_reference(void)
{
    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_method_set_inside_with_inner_reference\n"));

    cppu::FreeReference<uno::XInterface> env_obj(s_get_envObject());

    {
        cppu::EnvGuard envGuard(s_env);
        uno::XInterface * pObject2 = reinterpret_cast<uno::XInterface *>(
            createObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV))),
                         s_callee_in));
        {
            uno::Reference<uno::XInterface> ref(pObject2, SAL_NO_ACQUIRE);

            env_obj.set(ref);
        }
    }
}

static void s_test_method_set_inside_with_outer_reference(void)
{
    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_method_set_inside_with_outer_reference\n"));

    cppu::FreeReference<uno::XInterface> env_obj(s_get_envObject());

    uno::XInterface * pObject = reinterpret_cast<uno::XInterface *>(
        createObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV))),
                     s_callee_out));
    {
        uno::Reference<uno::XInterface> ref(pObject, SAL_NO_ACQUIRE);

        env_obj.set(ref);
    }
}

static void s_test_method_set_with_empty_reference(void)
{
    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_method_set_outside_with_empty_reference\n"));

    cppu::FreeReference<uno::XInterface> flat_obj(s_get_flatObject());

    {
        uno::Reference<uno::XInterface> ref;

        flat_obj.set(ref);
    }
}

static void s_test_method_set_outside_with_inner_reference(void)
{
    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_method_set_outside_with_inner_reference\n"));

    cppu::FreeReference<uno::XInterface> flat_obj(s_get_flatObject());

    {
        cppu::EnvGuard envGuard(s_env);

        uno::XInterface * pObject2 = reinterpret_cast<uno::XInterface *>(
            createObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV))),
                         s_callee_in));
        {
            uno::Reference<uno::XInterface> ref(pObject2, SAL_NO_ACQUIRE);

            flat_obj.set(ref);
        }
    }
}

static void s_test_method_set_outside_with_outer_reference(void)
{
    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_method_set_outside_with_outer_reference\n"));

    cppu::FreeReference<uno::XInterface> flat_obj(s_get_flatObject());

    uno::XInterface * pObject2 = reinterpret_cast<uno::XInterface *>(
        createObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV))),
                     s_callee_out));
    {
        uno::Reference<uno::XInterface> ref(pObject2, SAL_NO_ACQUIRE);

        flat_obj.set(ref);
    }
}

static void s_test_operator_equal(void)
{
    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_operator_equal\n"));


    cppu::FreeReference<uno::XInterface> env_obj;
    uno::XInterface * pObject = NULL;
    {
        cppu::EnvGuard envGuard(s_env);
        pObject = reinterpret_cast<uno::XInterface *>(
            createObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV))),
                         s_callee_in));

        env_obj = cppu::FreeReference<uno::XInterface>(pObject, SAL_NO_ACQUIRE);
    }


    uno::Mapping mapping(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV) ":unsafe")),
                         rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV))));
    uno::Reference<uno::XInterface> tmp;
    uno::XInterface * pMappedObject = reinterpret_cast<uno::XInterface *>(mapping.mapInterface(pObject, ::getCppuType(&tmp)));


    cppu::FreeReference<uno::XInterface> flat_obj(pMappedObject, SAL_NO_ACQUIRE);

    {
        cppu::EnvGuard envGuard(s_env);

        if (!(env_obj == flat_obj))
            s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\trefs are not equal inside - FAILURE\n"));
    }

    if (!(env_obj == flat_obj))
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\trefs are not equal outside - FAILURE\n"));
}

static void s_test_operator_unequal(void)
{
    s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_operator_unequal\n"));


    uno::XInterface * pObject = NULL;
    cppu::FreeReference<uno::XInterface> env_obj;
    {
        cppu::EnvGuard envGuard(s_env);

        pObject = reinterpret_cast<uno::XInterface *>(
            createObject(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV))),
                         s_callee_in));

        env_obj = cppu::FreeReference<uno::XInterface>(pObject, SAL_NO_ACQUIRE);
    }


    uno::Mapping mapping(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV) ":unsafe")),
                         rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(CPPU_STRINGIFY(CPPU_ENV))));
    uno::Reference<uno::XInterface> tmp;
    uno::XInterface * pMappedObject = reinterpret_cast<uno::XInterface *>(mapping.mapInterface(pObject, ::getCppuType(&tmp)));


    cppu::FreeReference<uno::XInterface> flat_obj(pMappedObject, SAL_NO_ACQUIRE);

    {
        cppu::EnvGuard envGuard(s_env);

        if (env_obj != flat_obj)
            s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\trefs are not equal inside - FAILURE\n"));
    }

    if (env_obj != flat_obj)
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\trefs are not equal outside - FAILURE\n"));
}


SAL_IMPLEMENT_MAIN_WITH_ARGS(/*argc*/, argv)
{
    uno::Environment(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO))).enter();

    s_env = uno::Environment(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("uno:unsafe")));


    rtl::OUString result;
    s_test_operator_arrow();
    s_test_operator_equal();
    s_test_operator_unequal();

    s_test_operator_assign_empty_reference();
    s_test_operator_assign_inner_reference_inside();

    s_test_method_set_with_empty_reference();
    s_test_method_set_inside_with_inner_reference();
    s_test_method_set_inside_with_outer_reference();
    s_test_method_set_outside_with_inner_reference();
    s_test_method_set_outside_with_outer_reference();

    s_test_method_clear_inner_reference_inside();
    s_test_method_clear_inner_reference_outside();
    s_test_method_clear_outer_reference_inside();
    s_test_method_clear_outer_reference_outside();

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
