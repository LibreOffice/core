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



#define CPPUHELPER_TEST_COMPONENT_IMPL
#include "TestComponent.hxx"


#include <string.h>

#include <osl/thread.h>

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <com/sun/star/uno/XComponentContext.hpp>


#include <cppu/EnvDcp.hxx>

#include <uno/environment.hxx>
#include <uno/lbnames.h>

using namespace ::com::sun::star;


#define LOG_LIFECYCLE_TestComponent
#ifdef LOG_LIFECYCLE_TestComponent
#  include <iostream>
#  define LOG_LIFECYCLE_TestComponent_emit(x) x

#else
#  define LOG_LIFECYCLE_TestComponent_emit(x)

#endif


class TestComponent: public cppu::WeakImplHelper <lang::XServiceInfo>
{
    rtl::OUString m_implName;

public:
    static uno::Reference<uno::XInterface> create(
        uno::Reference<uno::XComponentContext> const & xCtx
    );


    static uno::Sequence<rtl::OUString> SAL_CALL getSupportedServiceNames_Static();

    explicit TestComponent(uno::Reference<uno::XComponentContext> const & xCtx);
    virtual ~TestComponent();

    uno::Any SAL_CALL queryInterface(uno::Type const & rType ) throw (css::uno::RuntimeException);
    void SAL_CALL release() throw ();
    void SAL_CALL acquire() throw ();

    // lang::XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw (uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const & ServiceName)
        throw (uno::RuntimeException);
    virtual uno::Sequence<rtl::OUString> SAL_CALL getSupportedServiceNames()
        throw (uno::RuntimeException);

protected:
    uno::Reference<uno::XComponentContext> m_xComponentContext;
};


uno::Reference<uno::XInterface> SAL_CALL TestComponent::create(
    uno::Reference<uno::XComponentContext> const & xCtx)
{
    try
    {
        return static_cast<cppu::OWeakObject *>(new TestComponent(xCtx));
    }
    catch (std::bad_alloc &)
    {
        throw uno::RuntimeException("std::bad_alloc");
    }
}

uno::Sequence<rtl::OUString> SAL_CALL TestComponent::getSupportedServiceNames_Static()
{
    uno::Sequence<rtl::OUString> serviceNames(1);
    serviceNames[0] = rtl::OUString("com.sun.star.lang.ServiceInfo");

    return serviceNames;
}


TestComponent::TestComponent(uno::Reference<uno::XComponentContext> const & xCtx)
    : m_xComponentContext(xCtx)
{
    LOG_LIFECYCLE_TestComponent_emit(fprintf(stderr, "LIFE: %s -> %p\n", "TestComponent::TestComponent()", this));
}

TestComponent::~TestComponent()
{
    LOG_LIFECYCLE_TestComponent_emit(fprintf(stderr, "LIFE: %s -> %p\n", "TestComponent::~TestComponent", this));
}

rtl::OUString SAL_CALL TestComponent::getImplementationName()
    throw (uno::RuntimeException)
{
    return m_implName;
}

void SAL_CALL TestComponent::acquire() throw ()
{
     cppu::WeakImplHelper<lang::XServiceInfo>::acquire();
}

void SAL_CALL TestComponent::release() throw ()
{
    cppu::WeakImplHelper<lang::XServiceInfo>::release();
}

uno::Any SAL_CALL TestComponent::queryInterface(uno::Type const & rType ) throw (css::uno::RuntimeException)
{
    return cppu::WeakImplHelper<lang::XServiceInfo>::queryInterface(rType);
}

sal_Bool SAL_CALL TestComponent::supportsService(rtl::OUString const & ServiceName)
    throw (uno::RuntimeException)
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence<rtl::OUString> SAL_CALL TestComponent::getSupportedServiceNames()
    throw (uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}

extern "C" sal_Bool SAL_CALL component_writeInfo(
    void * /*serviceManager*/,
    void * /*registryKey*/
)
{
    g_envDcp = uno::Environment::getCurrent().getTypeName();

    return true;
}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    char const * pImplName,
    void       * /*serviceManager*/,
    void       * /*registryKey*/
)
{
    g_envDcp = uno::Environment::getCurrent().getTypeName();

    uno::Reference< lang::XSingleComponentFactory > xFactory;

    rtl::OUString uTmp(pImplName, rtl_str_getLength(pImplName), RTL_TEXTENCODING_ASCII_US);

    rtl::OUString uImplName(cppu::EnvDcp::getTypeName(uTmp));
    rtl::OUString cmpName("impl.test.TestComponent");

    if (uImplName.equals(cmpName))
    {
        xFactory = cppu::createSingleComponentFactory(
            TestComponent::create,
            uImplName,
            TestComponent::getSupportedServiceNames_Static());

        xFactory->acquire();
    }

    return xFactory.get();
}

extern "C" SAL_DLLPUBLIC_EXPORT  void SAL_CALL component_getImplementationEnvironmentExt(
    sal_Char        const ** envTypeName,
    uno_Environment       ** /*ppEnv*/,
    sal_Char        const  * pImplName,
    uno_Environment        * /*pSrcEnv*/
)
{
    rtl::OString purpose;

    if (pImplName) // this is the purpose for a specified impl
    {
        rtl::OUString uImplName(pImplName, rtl_str_getLength(pImplName), RTL_TEXTENCODING_ASCII_US);
        purpose = rtl::OUStringToOString(cppu::EnvDcp::getPurpose(uImplName), RTL_TEXTENCODING_ASCII_US);
    }

    if (!purpose.getLength())
    {
        char * pPurpose = getenv("TestComponent.uno");
        if (pPurpose)
            purpose = rtl::OString(pPurpose);
    }

    if (purpose.getLength() == 0)
        *envTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;

    else
    {
        char buff[256];
        strcpy(buff, CPPU_CURRENT_LANGUAGE_BINDING_NAME);
        strcat(buff, purpose.getStr());

        *envTypeName = strdup(buff);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
