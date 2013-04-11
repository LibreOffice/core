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

#include "sal/config.h"
#include "sal/macros.h"

#include <cstddef>
#include <memory>
#include <new>

#include "com/sun/star/lang/XMain.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/util/theMacroExpander.hpp"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/interfacecontainer.hxx"
#include "cppuhelper/unourl.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/thread.h"
#include "rtl/malformeduriexception.hxx"
#include "rtl/string.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "salhelper/simplereferenceobject.hxx"
#include "uno/current_context.hxx"
#include "uno/environment.h"
#include "uno/lbnames.h"

#include "test/types/CppTest.hpp"
#include "test/types/JavaTest.hpp"
#include "test/types/TestException.hpp"
#include "test/types/XTest.hpp"

namespace {

class Service: public ::cppu::WeakImplHelper1< css::lang::XMain > {
public:
    explicit Service(
        css::uno::Reference< css::uno::XComponentContext > const & context):
        context_(context) {}

    virtual ::sal_Int32 SAL_CALL run(
        css::uno::Sequence< ::rtl::OUString > const &)
        throw (css::uno::RuntimeException);

private:
    Service(Service &); // not defined
    void operator =(Service &); // not defined

    virtual ~Service() {}

    void test(
        css::uno::Reference< test::types::XTest > const & test,
        ::rtl::OUString const & name);

    css::uno::Reference< css::uno::XComponentContext > context_;
};

::sal_Int32 Service::run(css::uno::Sequence< ::rtl::OUString > const &)
    throw (css::uno::RuntimeException)
{
    osl_getThreadIdentifier(0); // check for sal
    (new salhelper::SimpleReferenceObject)->release(); // check for salhelper
    css::uno::getCurrentContext(); // check for cppu
    try { // check for cppuhelper
        std::auto_ptr< cppu::UnoUrl > dummy(new cppu::UnoUrl(rtl::OUString()));
    } catch (rtl::MalformedUriException &) {}
    static char const * const services[] = {
        "com.sun.star.beans.Introspection",
        "com.sun.star.bridge.BridgeFactory",
        "com.sun.star.bridge.UnoUrlResolver",
        "com.sun.star.connection.Acceptor",
        "com.sun.star.connection.Connector",
        "com.sun.star.io.DataInputStream",
        "com.sun.star.io.DataOutputStream",
        "com.sun.star.io.MarkableInputStream",
        "com.sun.star.io.MarkableOutputStream",
        "com.sun.star.io.ObjectInputStream",
        "com.sun.star.io.ObjectOutputStream",
        "com.sun.star.io.Pipe",
        "com.sun.star.io.Pump",
        "com.sun.star.io.TextInputStream",
        "com.sun.star.io.TextOutputStream",
        "com.sun.star.java.JavaVirtualMachine",
        "com.sun.star.lang.RegistryServiceManager",
        "com.sun.star.lang.ServiceManager",
        "com.sun.star.loader.Java",
        "com.sun.star.loader.Java2",
        "com.sun.star.loader.SharedLibrary",
        "com.sun.star.reflection.CoreReflection",
        "com.sun.star.reflection.ProxyFactory",
        "com.sun.star.reflection.TypeDescriptionManager",
        "com.sun.star.reflection.TypeDescriptionProvider",
        "com.sun.star.registry.ImplementationRegistration",
        "com.sun.star.registry.NestedRegistry",
        "com.sun.star.registry.SimpleRegistry",
        "com.sun.star.script.Converter",
        "com.sun.star.script.Invocation",
        "com.sun.star.script.InvocationAdapterFactory",
        "com.sun.star.security.AccessController",
        "com.sun.star.security.Policy",
        "com.sun.star.uno.NamingService",
        "com.sun.star.uri.ExternalUriReferenceTranslator",
        "com.sun.star.uri.UriReferenceFactory",
        "com.sun.star.uri.UriSchemeParser_vndDOTsunDOTstarDOTscript",
        "com.sun.star.uri.VndSunStarPkgUrlReferenceFactory"
    };
    for (::std::size_t i = 0; i < SAL_N_ELEMENTS(services); ++i) {
        ::rtl::OUString name(::rtl::OUString::createFromAscii(services[i]));
        css::uno::Reference< css::uno::XInterface > instance;
        try {
            instance = context_->getServiceManager()->createInstanceWithContext(
                name, context_);
        } catch (css::uno::RuntimeException &) {
            throw;
        } catch (css::uno::Exception &) {
            throw css::uno::RuntimeException(
                ::rtl::OUString("error creating instance"),
                static_cast< ::cppu::OWeakObject * >(this));
        }
        if (!instance.is()) {
            throw css::uno::RuntimeException(
                "no instance: " + name,
                static_cast< ::cppu::OWeakObject * >(this));
        }
    }
    css::util::theMacroExpander::get(context_);
    test(
        ::test::types::CppTest::create(context_),
        ::rtl::OUString("test.types.CppTest"));
    test(
        ::test::types::JavaTest::create(context_),
        ::rtl::OUString("test.types.JavaTest"));
    return 0;
}

void Service::test(
    css::uno::Reference< test::types::XTest > const & test,
    ::rtl::OUString const & name)
{
    bool ok = false;
    try {
        test->throwException();
    } catch (::test::types::TestException &) {
        ok = true;
    }
    if (!ok) {
        throw css::uno::RuntimeException(
            (name
             + ::rtl::OUString(".throwException failed")),
            static_cast< ::cppu::OWeakObject * >(this));
    }
}

namespace CppMain {

css::uno::Reference< css::uno::XInterface > create(
    css::uno::Reference< css::uno::XComponentContext > const & context)
    SAL_THROW((css::uno::Exception))
{
    try {
        return static_cast< ::cppu::OWeakObject * >(new Service(context));
    } catch (::std::bad_alloc &) {
        throw css::uno::RuntimeException(
            ::rtl::OUString("std::bad_alloc"),
            css::uno::Reference< css::uno::XInterface >());
    }
}

rtl::OUString getImplementationName() {
    return rtl::OUString("test.cpp.cppmain.Component");
}

css::uno::Sequence< ::rtl::OUString > getSupportedServiceNames() {
    return css::uno::Sequence< ::rtl::OUString >();
}

}

::cppu::ImplementationEntry entries[] = {
    { CppMain::create, CppMain::getImplementationName,
      CppMain::getSupportedServiceNames, ::cppu::createSingleComponentFactory,
      0, 0 },
    { 0, 0, 0, 0, 0, 0 } };

}

extern "C" SAL_DLLPUBLIC_EXPORT ::sal_Bool SAL_CALL component_writeInfo(
    void * serviceManager, void * registryKey)
{
    return ::cppu::component_writeInfoHelper(
        serviceManager, registryKey, entries);
}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    char const * implName, void * serviceManager, void * registryKey)
{
    return ::cppu::component_getFactoryHelper(
        implName, serviceManager, registryKey, entries);
}
