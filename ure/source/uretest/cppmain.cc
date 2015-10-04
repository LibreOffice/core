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

#include "com/sun/star/beans/Introspection.hpp"
#include "com/sun/star/beans/theIntrospection.hpp"
#include "com/sun/star/bridge/BridgeFactory.hpp"
#include "com/sun/star/bridge/UnoUrlResolver.hpp"
#include "com/sun/star/connection/Acceptor.hpp"
#include "com/sun/star/connection/Connector.hpp"
#include "com/sun/star/io/Pipe.hpp"
#include "com/sun/star/io/TextInputStream.hpp"
#include "com/sun/star/io/TextOutputStream.hpp"
#include "com/sun/star/java/JavaVirtualMachine.hpp"
#include "com/sun/star/lang/XMain.hpp"
#include "com/sun/star/loader/Java.hpp"
#include "com/sun/star/loader/SharedLibrary.hpp"
#include "com/sun/star/reflection/ProxyFactory.hpp"
#include "com/sun/star/registry/ImplementationRegistration.hpp"
#include "com/sun/star/registry/SimpleRegistry.hpp"
#include "com/sun/star/script/Converter.hpp"
#include "com/sun/star/script/Invocation.hpp"
#include "com/sun/star/security/AccessController.hpp"
#include "com/sun/star/security/Policy.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/NamingService.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "com/sun/star/uri/ExternalUriReferenceTranslator.hpp"
#include "com/sun/star/uri/UriReferenceFactory.hpp"
#include "com/sun/star/uri/VndSunStarPkgUrlReferenceFactory.hpp"
#include "com/sun/star/util/theMacroExpander.hpp"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implbase.hxx"
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

#include "test/types/CppTest.hpp"
#include "test/types/JavaTest.hpp"
#include "test/types/TestException.hpp"
#include "test/types/XTest.hpp"

namespace {

class Service: public ::cppu::WeakImplHelper < css::lang::XMain >
{
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
        "com.sun.star.io.DataInputStream",
        "com.sun.star.io.DataOutputStream",
        "com.sun.star.io.MarkableInputStream",
        "com.sun.star.io.MarkableOutputStream",
        "com.sun.star.io.ObjectInputStream",
        "com.sun.star.io.ObjectOutputStream",
        "com.sun.star.io.Pump",
        "com.sun.star.lang.RegistryServiceManager",
        "com.sun.star.lang.ServiceManager",
        "com.sun.star.reflection.CoreReflection",
        "com.sun.star.registry.NestedRegistry",
        "com.sun.star.script.InvocationAdapterFactory",
        "com.sun.star.uri.UriSchemeParser_vndDOTsunDOTstarDOTscript"
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
    css::beans::Introspection::create(context_);
    css::beans::theIntrospection::get(context_);
    css::bridge::BridgeFactory::create(context_);
    css::bridge::UnoUrlResolver::create(context_);
    css::connection::Acceptor::create(context_);
    css::connection::Connector::create(context_);
    css::io::Pipe::create(context_);
    css::io::TextInputStream::create(context_);
    css::io::TextOutputStream::create(context_);
    css::java::JavaVirtualMachine::create(context_);
    css::loader::Java::create(context_);
    css::loader::SharedLibrary::create(context_);
    css::reflection::ProxyFactory::create(context_);
    css::registry::ImplementationRegistration::create(context_);
    css::registry::SimpleRegistry::create(context_);
    css::script::Converter::create(context_);
    css::script::Invocation::create(context_);
    css::security::AccessController::create(context_);
    css::security::Policy::create(context_);
    css::uno::NamingService::create(context_);
    css::uri::ExternalUriReferenceTranslator::create(context_);
    css::uri::UriReferenceFactory::create(context_);
    css::uri::VndSunStarPkgUrlReferenceFactory::create(context_);
    static char const * const singletons[] = {
        "com.sun.star.reflection.theTypeDescriptionManager"
    };
    for (std::size_t i = 0; i != SAL_N_ELEMENTS(singletons); ++i) {
        css::uno::Reference< css::uno::XInterface > instance(
            context_->getValueByName(
                "/singletons/" + rtl::OUString::createFromAscii(singletons[i])),
            css::uno::UNO_QUERY_THROW);
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
{
    try {
        return static_cast< ::cppu::OWeakObject * >(new Service(context));
    } catch (::std::bad_alloc &) {
        throw css::uno::RuntimeException("std::bad_alloc");
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
