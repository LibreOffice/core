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


#include "sal/main.h"
#include "osl/file.hxx"

#include "typelib/typedescription.hxx"

#include "cppuhelper/bootstrap.hxx"
#include "cppuhelper/shlib.hxx"

#include <com/sun/star/lang/XComponent.hpp>

#include "uno/environment.hxx"
#include "cppu/EnvDcp.hxx"
#include "cppu/EnvGuards.hxx"

#include <iostream>


#ifndef SAL_DLLPREFIX
# define SAL_DLLPREFIX  ""
#endif


using namespace com::sun::star;


static rtl::OUString s_comment;

static bool s_check_object_is_in(void * pObject)
{
    uno::Environment currentEnv(uno::Environment::getCurrent());

    rtl_uString * pOId = NULL;
    currentEnv.get()->pExtEnv->getObjectIdentifier(currentEnv.get()->pExtEnv, &pOId, pObject);


    uno::TypeDescription typeDescription(rtl::OUString("com.sun.star.uno.XInterface"));

    void * pRegisteredObject = NULL;
    currentEnv.get()->pExtEnv->getRegisteredInterface(currentEnv.get()->pExtEnv,
                                                      &pRegisteredObject,
                                                      pOId,
                                                      (typelib_InterfaceTypeDescription *)typeDescription.get());


    if (pOId) rtl_uString_release(pOId);

    bool result = pRegisteredObject != NULL;

    if (result)
        currentEnv.get()->pExtEnv->releaseInterface(currentEnv.get()->pExtEnv, pRegisteredObject);

     return result;
}

static void s_test__loadSharedLibComponentFactory(rtl::OUString const & clientPurpose,
                                                  rtl::OUString const & servicePurpose)
{
    cppu::EnvGuard envGuard(uno::Environment(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO))
                                               + clientPurpose, NULL));
    if (clientPurpose.getLength() && !envGuard.is())
    {
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\tcouldn't get purpose env: \""));
        s_comment += clientPurpose;
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\" - FAILURE\n"));

        return;
    }

    rtl::OString os_clientPurpose(rtl::OUStringToOString(clientPurpose, RTL_TEXTENCODING_ASCII_US));

    uno::Reference<uno::XInterface> xItf(
        cppu::loadSharedLibComponentFactory(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SAL_DLLPREFIX "TestComponent.uno" SAL_DLLEXTENSION)),
#ifdef WIN32
            rtl::OUString(""),
#else
            rtl::OUString("file://../lib/"),
#endif
            rtl::OUString("impl.test.TestComponent") + servicePurpose,
            uno::Reference<lang::XMultiServiceFactory>(),
            uno::Reference<registry::XRegistryKey>()
            )
        );

    if (!xItf.is())
    {
        s_comment += rtl::OUString("\t\tgot no object - FAILURE\n");
        return;
    }

    if (!clientPurpose.equals(servicePurpose) && !s_check_object_is_in(xItf.get()))
    {
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\tcouldn't find object in current purpose \""));
        s_comment += clientPurpose;
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\" - FAILURE\n"));
    }

    if (!cppu::EnvDcp::getPurpose(uno::Environment::getCurrent().getTypeName()).equals(clientPurpose))
    {
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\tdid not enter client purpose \""));
        s_comment += clientPurpose;
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\" - FAILURE\n"));
    }
}

static void s_test__loadSharedLibComponentFactory__free_free()
{
    s_comment += rtl::OUString("\ts_test__loadSharedLibComponentFactory__free_free\n");

    s_test__loadSharedLibComponentFactory(rtl::OUString(), rtl::OUString());
}

static void s_test__loadSharedLibComponentFactory__free_purpose()
{
    s_comment += rtl::OUString("\ts_test__loadSharedLibComponentFactory__free_purpose\n");

    s_test__loadSharedLibComponentFactory(rtl::OUString(),
                                          rtl::OUString(":testenv"));
}

static void s_test__loadSharedLibComponentFactory__purpose_free()
{
    s_comment += rtl::OUString("\ts_test__loadSharedLibComponentFactory__purpose_free\n");

    s_test__loadSharedLibComponentFactory(rtl::OUString(":testenv"),
                                          rtl::OUString());
}

static void s_test__loadSharedLibComponentFactory__purpose_purpose()
{
    s_comment += rtl::OUString("\ts_test__loadSharedLibComponentFactory__purpose_purpose\n");

    s_test__loadSharedLibComponentFactory(rtl::OUString(":testenv"),
                                          rtl::OUString(":testenv"));
}

static rtl::OUString s_getSDrive(void)
{
    rtl::OUString path;//(RTL_CONSTASCII_USTRINGPARAM("file://"));

    char const * tmp = getenv("SOLARVER");
    path += rtl::OUString(tmp, rtl_str_getLength(tmp), RTL_TEXTENCODING_ASCII_US);
    path += rtl::OUString(SAL_PATHDELIMITER);

    tmp = getenv("INPATH");
    path += rtl::OUString(tmp, rtl_str_getLength(tmp), RTL_TEXTENCODING_ASCII_US);
    path += rtl::OUString(SAL_PATHDELIMITER);
#ifdef WIN32
    path += rtl::OUString("bin");

#else
    path += rtl::OUString("lib");
#endif

    tmp = getenv("UPDMINOREXT");
    if (tmp)
        path += rtl::OUString(tmp, rtl_str_getLength(tmp), RTL_TEXTENCODING_ASCII_US);

    osl::FileBase::getFileURLFromSystemPath(path, path);

    return path;
}

static void s_test__createSimpleRegistry(rtl::OUString const & clientPurpose)
{
    cppu::EnvGuard envGuard(uno::Environment(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO))
                                               + clientPurpose, NULL));
    if (clientPurpose.getLength() && !envGuard.is())
    {
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\tcouldn't get purpose env: \""));
        s_comment += clientPurpose;
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\" - FAILURE\n"));

        return;
    }

    uno::Reference<registry::XSimpleRegistry> registry(cppu::createSimpleRegistry(
                                                           s_getSDrive()));

    if (!registry.is())
    {
        s_comment += rtl::OUString("\t\tgot no object - FAILURE\n");
        return;
    }

    if (clientPurpose.getLength() != 0 && !s_check_object_is_in(registry.get()))
    {
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\tcouldn't find object in current purpose \""));
        s_comment += clientPurpose;
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\" - FAILURE\n"));
    }
}

static void s_test__createSimpleRegistry__free(void)
{
    s_comment += rtl::OUString("\ts_test__createSimpleRegistry__free\n");

    s_test__createSimpleRegistry(rtl::OUString());
}

static void s_test__createSimpleRegistry__purpose(void)
{
    s_comment += rtl::OUString("\ts_test__createSimpleRegistry__purpose\n");

    s_test__createSimpleRegistry(rtl::OUString(":testenv"));
}


static void s_test__bootstrap_InitialComponentContext(rtl::OUString const & clientPurpose)
{
    cppu::EnvGuard envGuard(uno::Environment(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(UNO_LB_UNO))
                                               + clientPurpose, NULL));
    if (clientPurpose.getLength() && !envGuard.is())
    {
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\tcouldn't get purpose env: \""));
        s_comment += clientPurpose;
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\" - FAILURE\n"));

        return;
    }

    uno::Reference<uno::XComponentContext> xContext(
        cppu::bootstrap_InitialComponentContext(
            uno::Reference<registry::XSimpleRegistry>(),
            s_getSDrive())
        );

    if (!xContext.is())
    {
        s_comment += rtl::OUString("\t\tgot no object - FAILURE\n");
        return;
    }

    if (clientPurpose.getLength() != 0 && !s_check_object_is_in(xContext.get()))
    {
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\t\tcouldn't find object in current purpose \""));
        s_comment += clientPurpose;
        s_comment += rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\" - FAILURE\n"));
    }

    uno::Reference<lang::XComponent> xComponent(xContext, uno::UNO_QUERY_THROW);
    xComponent->dispose();
}

static void s_test__bootstrap_InitialComponentContext__free(void)
{
    s_comment += rtl::OUString("\ts_test__bootstrap_InitialComponentContext__free\n");

    s_test__bootstrap_InitialComponentContext(rtl::OUString());
}

static void s_test__bootstrap_InitialComponentContext__purpose(void)
{
    s_comment += rtl::OUString("\ts_test__bootstrap_InitialComponentContext__purpose\n");

    s_test__bootstrap_InitialComponentContext(rtl::OUString(":testenv"));
}


SAL_IMPLEMENT_MAIN_WITH_ARGS(/*argc*/, argv)
{
    s_test__createSimpleRegistry__free();
    s_test__createSimpleRegistry__purpose();

    s_test__loadSharedLibComponentFactory__free_free();
    s_test__loadSharedLibComponentFactory__free_purpose();
    s_test__loadSharedLibComponentFactory__purpose_free();
    s_test__loadSharedLibComponentFactory__purpose_purpose();

    s_test__bootstrap_InitialComponentContext__free();
    s_test__bootstrap_InitialComponentContext__purpose();

    int ret;
    if (s_comment.indexOf("FAILURE") == -1)
    {
        s_comment += rtl::OUString("TESTS PASSED\n");
        ret = 0;
    }
    else
    {
        s_comment += rtl::OUString("TESTS _NOT_ PASSED\n");
        ret = -1;
    }

    std::cerr
        << argv[0]
        << std::endl
        << rtl::OUStringToOString(s_comment, RTL_TEXTENCODING_ASCII_US).getStr()
        << std::endl;

    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
