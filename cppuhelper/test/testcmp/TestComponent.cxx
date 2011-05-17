/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_cppuhelper.hxx"


#define CPPUHELPER_TEST_COMPONENT_IMPL
#include "TestComponent.hxx"


#include <string.h>

#include "osl/thread.h"

#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/implementationentry.hxx"

#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"

#include "com/sun/star/uno/XComponentContext.hpp"


#include "cppu/EnvDcp.hxx"

#include <uno/environment.hxx>

using namespace ::com::sun::star;


#define LOG_LIFECYCLE_TestComponent
#ifdef LOG_LIFECYCLE_TestComponent
#  include <iostream>
#  define LOG_LIFECYCLE_TestComponent_emit(x) x

#else
#  define LOG_LIFECYCLE_TestComponent_emit(x)

#endif


class TestComponent: public cppu::WeakImplHelper1<lang::XServiceInfo>
{
    rtl::OUString m_implName;

public:
    static uno::Reference<uno::XInterface> create(
        uno::Reference<uno::XComponentContext> const & xCtx
    )
    SAL_THROW((uno::Exception));


    static uno::Sequence<rtl::OUString> SAL_CALL getSupportedServiceNames_Static();

    explicit TestComponent(uno::Reference<uno::XComponentContext> const & xCtx);
    virtual ~TestComponent();

    uno::Any SAL_CALL queryInterface(uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException);
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
    uno::Reference<uno::XComponentContext> const & xCtx
)
    SAL_THROW((uno::Exception))
{
    try
    {
        return static_cast<cppu::OWeakObject *>(new TestComponent(xCtx));
    }
    catch (std::bad_alloc &)
    {
        throw uno::RuntimeException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("std::bad_alloc")),
                                     uno::Reference<uno::XInterface>());
    }
}

uno::Sequence<rtl::OUString> SAL_CALL TestComponent::getSupportedServiceNames_Static()
{
    uno::Sequence<rtl::OUString> serviceNames(1);
    serviceNames[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.lang.ServiceInfo"));

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
     cppu::WeakImplHelper1<lang::XServiceInfo>::acquire();
}

void SAL_CALL TestComponent::release() throw ()
{
    cppu::WeakImplHelper1<lang::XServiceInfo>::release();
}

uno::Any SAL_CALL TestComponent::queryInterface(uno::Type const & rType ) throw (::com::sun::star::uno::RuntimeException)
{
    return cppu::WeakImplHelper1<lang::XServiceInfo>::queryInterface(rType);
}

sal_Bool SAL_CALL TestComponent::supportsService(rtl::OUString const & ServiceName)
    throw (uno::RuntimeException)
{
    uno::Sequence<rtl::OUString> serviceNames = getSupportedServiceNames_Static();

    for (sal_Int32 n = 0; n < serviceNames.getLength(); ++n)
    {
        if (serviceNames[n] == ServiceName)
            return true;
    }

    return false;
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
    rtl::OUString cmpName(RTL_CONSTASCII_USTRINGPARAM("impl.test.TestComponent"));

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
        strcpy(buff, CPPU_STRINGIFY(CPPU_ENV));
        strcat(buff, purpose.getStr());

        *envTypeName = strdup(buff);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
