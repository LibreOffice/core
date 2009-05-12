/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: purpenv.test.cxx,v $
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
#include "uno/dispatcher.h"

#include "typelib/typedescription.h"

#include "com/sun/star/uno/Any.h"

#include "../ObjectFactory/ObjectFactory.hxx"


using namespace com::sun::star;


static rtl::OUString    g_usret(RTL_CONSTASCII_USTRINGPARAM("\n"));
static rtl::OUString    g_ustab(RTL_CONSTASCII_USTRINGPARAM("\t"));
static rtl::OUString    g_comment;
static uno::Environment g_env;
static bool             g_check = 0;

extern "C" { static void s_callee(rtl_uString * pMethod_name)
{
    if (g_check)
    {
        g_comment += g_ustab;
        g_comment += g_ustab;
        g_comment += rtl::OUString(pMethod_name);

        rtl::OUString reason;
        int valid = g_env.isValid(&reason);

        g_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("["));
        g_comment += rtl::OUString::valueOf((sal_Int32)valid);
        g_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
        g_comment += reason;
        g_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("]"));

        if (!valid)
            g_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" - FAILURE"));

        g_comment += g_usret;
    }
}}


static rtl::OUString s_test_registerInterface(rtl::OUString const & envDcp)
{
    g_env = uno::Environment(envDcp);

    g_comment = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_registerInterface"));
    g_comment += g_usret;

    rtl::OUString id(RTL_CONSTASCII_USTRINGPARAM("blabla"));

    typelib_TypeDescriptionReference * type_XInterface =
        * typelib_static_type_getByTypeClass( typelib_TypeClass_INTERFACE );

    typelib_TypeDescription * pRet = NULL;
    typelib_typedescriptionreference_getDescription(&pRet, type_XInterface);

    void * pObject = createObject(envDcp, s_callee);
    g_check = 1;
    g_env.get()->pExtEnv->registerInterface(g_env.get()->pExtEnv,
                                          &pObject,
                                          id.pData,
                                          (struct _typelib_InterfaceTypeDescription *)pRet);
    g_check = 0;

    g_env.get()->pExtEnv->revokeInterface(g_env.get()->pExtEnv, pObject);
    g_env.get()->pExtEnv->releaseInterface(g_env.get()->pExtEnv, pObject);

    g_env.clear();

    return g_comment;
}

extern "C" { static void s_freeFunc(struct _uno_ExtEnvironment * /*pEnv*/, void * /*pProxy*/ )
{
    if (g_check)
    {
        g_comment += g_ustab;
        g_comment += g_ustab;
        g_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("s_freeFunc"));

        rtl::OUString reason;
        int valid = g_env.isValid(&reason);

        g_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("["));
        g_comment += rtl::OUString::valueOf((sal_Int32)valid);
        g_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
        g_comment += reason;
        g_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("]"));

        if (!valid)
            g_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" - FAILURE"));

        g_comment += g_usret;
    }
}}

static rtl::OUString s_test_registerProxyInterface(rtl::OUString const & envDcp)
{
    g_env = uno::Environment(envDcp);

    g_comment = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_registerProxyInterface"));
    g_comment += g_usret;

    rtl::OUString id(RTL_CONSTASCII_USTRINGPARAM("blabla"));

    typelib_TypeDescriptionReference * type_XInterface =
        * typelib_static_type_getByTypeClass( typelib_TypeClass_INTERFACE );

    typelib_TypeDescription * pRet = NULL;
    typelib_typedescriptionreference_getDescription(&pRet, type_XInterface);

    void * pObject = createObject(envDcp, s_callee);
    g_check = 1;
    g_env.get()->pExtEnv->registerProxyInterface(g_env.get()->pExtEnv,
                                               &pObject,
                                               s_freeFunc,
                                               id.pData,
                                               (typelib_InterfaceTypeDescription *)pRet);
    g_check = 0;

    g_env.get()->pExtEnv->releaseInterface(g_env.get()->pExtEnv, pObject);
    g_check = 1;
    g_env.get()->pExtEnv->revokeInterface(g_env.get()->pExtEnv, pObject);
    g_check = 0;

    g_env.clear();


    return g_comment;
}

static rtl::OUString s_test_revokeInterface(rtl::OUString const & envDcp)
{
    g_env = uno::Environment(envDcp);

    g_comment = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_revokeInterface"));
    g_comment += g_usret;

    rtl::OUString id(RTL_CONSTASCII_USTRINGPARAM("blabla"));

    typelib_TypeDescriptionReference * type_XInterface =
        * typelib_static_type_getByTypeClass( typelib_TypeClass_INTERFACE );

    typelib_TypeDescription * pRet = NULL;
    typelib_typedescriptionreference_getDescription(&pRet, type_XInterface);

    void * pObject = createObject(envDcp, s_callee);
    g_env.get()->pExtEnv->registerInterface(g_env.get()->pExtEnv,
                                          &pObject,
                                          id.pData,
                                          (struct _typelib_InterfaceTypeDescription *)pRet);

    g_check = 1;
    g_env.get()->pExtEnv->revokeInterface(g_env.get()->pExtEnv, pObject);
    g_check = 0;
    g_env.get()->pExtEnv->releaseInterface(g_env.get()->pExtEnv, pObject);

    g_env.clear();

    return g_comment;
}

static rtl::OUString s_test_getObjectIdentifier(rtl::OUString const & envDcp)
{
    g_env = uno::Environment(envDcp);

    g_comment = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_getObjectIdentifier"));
    g_comment += g_usret;

    rtl::OUString oId;
    void * pObject = createObject(envDcp, s_callee);
    g_check = 1;
    g_env.get()->pExtEnv->getObjectIdentifier(g_env.get()->pExtEnv, &oId.pData, pObject);
    g_check = 0;
    g_env.get()->pExtEnv->releaseInterface(g_env.get()->pExtEnv, pObject);

    g_env.clear();

    return g_comment;
}

static rtl::OUString s_test_getRegisteredInterface(rtl::OUString const & envDcp)
{
    g_env = uno::Environment(envDcp);

    g_comment = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_getRegisteredInterface"));
    g_comment += g_usret;

    rtl::OUString id(RTL_CONSTASCII_USTRINGPARAM("blabla"));


    typelib_TypeDescriptionReference * type_XInterface =
        * typelib_static_type_getByTypeClass( typelib_TypeClass_INTERFACE );

    typelib_TypeDescription * pRet = NULL;
    typelib_typedescriptionreference_getDescription(&pRet, type_XInterface);

    void * pObject = createObject(envDcp, s_callee);
    g_env.get()->pExtEnv->registerInterface(g_env.get()->pExtEnv,
                                            &pObject,
                                            id.pData,
                                            (typelib_InterfaceTypeDescription *)pRet);


    g_check = 1;
    g_env.get()->pExtEnv->getRegisteredInterface(g_env.get()->pExtEnv, &pObject, id.pData,
                                                 (typelib_InterfaceTypeDescription *)pRet);
    g_check = 0;

    g_env.get()->pExtEnv->releaseInterface(g_env.get()->pExtEnv, pObject);
    g_env.get()->pExtEnv->revokeInterface(g_env.get()->pExtEnv, pObject);

    g_env.clear();

    return g_comment;
}

static rtl::OUString s_test_getRegisteredInterfaces(rtl::OUString const & envDcp)
{
    g_env = uno::Environment(envDcp);

    g_comment = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_getRegisteredInterfaces"));
    g_comment += g_usret;

    rtl::OUString id(RTL_CONSTASCII_USTRINGPARAM("blabla"));


    typelib_TypeDescriptionReference * type_XInterface =
        * typelib_static_type_getByTypeClass( typelib_TypeClass_INTERFACE );

    typelib_TypeDescription * pRet = NULL;
    typelib_typedescriptionreference_getDescription(&pRet, type_XInterface);

    void * pObject = createObject(envDcp, s_callee);
    g_env.get()->pExtEnv->registerInterface(g_env.get()->pExtEnv,
                                            &pObject,
                                            id.pData,
                                            (typelib_InterfaceTypeDescription *)pRet);

    g_check = 1;
    void ** ppObject;
    sal_Int32 nCount;
    g_env.get()->pExtEnv->getRegisteredInterfaces(g_env.get()->pExtEnv, &ppObject, &nCount, rtl_allocateMemory);
    g_check = 0;


    if (nCount != 1)
    {
        g_comment += g_ustab;
        g_comment += g_ustab;
        g_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("more than one object registered -> FAILURE"));
        g_comment += g_usret;
    }

    if (*ppObject != pObject)
    {
        g_comment += g_ustab;
        g_comment += g_ustab;
        g_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("returned object is not the registerd one -> FAILURE"));
        g_comment += g_usret;
    }

    g_env.get()->pExtEnv->releaseInterface(g_env.get()->pExtEnv, *ppObject);

    g_env.get()->pExtEnv->revokeInterface(g_env.get()->pExtEnv, pObject);
    g_env.get()->pExtEnv->releaseInterface(g_env.get()->pExtEnv, pObject);

    rtl_freeMemory(ppObject);

    g_env.clear();

    return g_comment;
}

static rtl::OUString s_test_computeObjectIdentifier(rtl::OUString const & envDcp)
{
    g_env = uno::Environment(envDcp);

    g_comment = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_computeObjectIdentifier"));
    g_comment += g_usret;

    rtl::OUString oId;
    void * pObject = createObject(envDcp, s_callee);
    g_check = 1;
    g_env.get()->pExtEnv->computeObjectIdentifier(g_env.get()->pExtEnv, &oId.pData, pObject);
    g_check = 0;
    g_env.get()->pExtEnv->releaseInterface(g_env.get()->pExtEnv, pObject);

    g_env.clear();

    return g_comment;
}

static rtl::OUString s_test_acquire(rtl::OUString const & envDcp)
{
    g_env = uno::Environment(envDcp);

    g_comment = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_acquire"));
    g_comment += g_usret;
    void * pObject = createObject(envDcp, s_callee);

    g_check = 1;
    g_env.get()->pExtEnv->acquireInterface(g_env.get()->pExtEnv, pObject);
    g_check = 0;

    g_env.get()->pExtEnv->releaseInterface(g_env.get()->pExtEnv, pObject);
    g_env.get()->pExtEnv->releaseInterface(g_env.get()->pExtEnv, pObject);
    g_env.clear();

    return g_comment;
}

static rtl::OUString s_test_release(rtl::OUString const & envDcp)
{
    g_env = uno::Environment(envDcp);

    g_comment = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\ts_test_release"));
    g_comment += g_usret;
    void * pObject = createObject(envDcp, s_callee);

    g_check = 1;
    g_env.get()->pExtEnv->releaseInterface(g_env.get()->pExtEnv, pObject);
    g_check = 0;

    g_env.clear();

    return g_comment;
}


rtl::OUString purpenv_test(rtl::OUString const & envDcp)
{
    g_env = uno::Environment(envDcp);

    rtl::OUString result;
    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("purpenv_test"));
    result += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n"));

    result += s_test_registerInterface(envDcp);
    result += g_usret;

    result += s_test_registerProxyInterface(envDcp);
    result += g_usret;

    result += s_test_revokeInterface(envDcp);
    result += g_usret;

    result += s_test_getObjectIdentifier(envDcp);
    result += g_usret;

    result += s_test_getRegisteredInterface(envDcp);
    result += g_usret;

    result += s_test_getRegisteredInterfaces(envDcp);
    result += g_usret;

    result += s_test_computeObjectIdentifier(envDcp);
    result += g_usret;

    result += s_test_acquire(envDcp);
    result += g_usret;

    result += s_test_release(envDcp);
    result += g_usret;

    return result;
}
