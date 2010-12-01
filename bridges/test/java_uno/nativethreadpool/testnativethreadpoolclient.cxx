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
#include "precompiled_bridges.hxx"

#include "test/javauno/nativethreadpool/XRelay.hpp"
#include "test/javauno/nativethreadpool/XSource.hpp"

#include "com/sun/star/bridge/UnoUrlResolver.hpp"
#include "com/sun/star/bridge/XUnoUrlResolver.hpp"
#include "com/sun/star/connection/ConnectionSetupException.hpp"
#include "com/sun/star/connection/NoConnectException.hpp"
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/lang/XMain.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implbase2.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/thread.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "uno/lbnames.h"

#include <iostream>

namespace css = com::sun::star;

namespace {

class Client: public cppu::WeakImplHelper2<
    css::lang::XMain, test::javauno::nativethreadpool::XSource >
{
public:
    explicit Client(
        css::uno::Reference< css::uno::XComponentContext > const & theContext):
        context(theContext) {}

private:
    virtual ~Client() {}

    virtual sal_Int32 SAL_CALL run(css::uno::Sequence< rtl::OUString > const &)
        throw (css::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL get() throw (css::uno::RuntimeException);

    css::uno::Reference< css::uno::XComponentContext > context;
    osl::ThreadData data;
};

sal_Int32 Client::run(css::uno::Sequence< rtl::OUString > const &)
    throw (css::uno::RuntimeException)
{
    css::uno::Reference< css::lang::XMultiComponentFactory > factory(
        context->getServiceManager());
    if (!factory.is()) {
        throw new css::uno::RuntimeException(
            rtl::OUString::createFromAscii(
                "no component context service manager"),
            static_cast< cppu::OWeakObject * >(this));
    }
    css::uno::Reference< test::javauno::nativethreadpool::XRelay > relay;
    try {
        relay = css::uno::Reference< test::javauno::nativethreadpool::XRelay >(
            factory->createInstanceWithContext(
                rtl::OUString::createFromAscii(
                    "test.javauno.nativethreadpool.Relay"),
                context),
            css::uno::UNO_QUERY_THROW);
    } catch (css::uno::RuntimeException &) {
        throw;
    } catch (css::uno::Exception & e) {
        throw css::lang::WrappedTargetRuntimeException(
            rtl::OUString::createFromAscii(
                "creating test.javauno.nativethreadpool.Relay service"),
            static_cast< cppu::OWeakObject * >(this), css::uno::makeAny(e));
    }
    relay->start(this);
    if (!data.setData(reinterpret_cast< void * >(12345))) {
        throw new css::uno::RuntimeException(
            rtl::OUString::createFromAscii("osl::ThreadData::setData failed"),
            static_cast< cppu::OWeakObject * >(this));
    }
    css::uno::Reference< test::javauno::nativethreadpool::XSource > source;
    try {
        source
            = css::uno::Reference< test::javauno::nativethreadpool::XSource >(
                css::bridge::UnoUrlResolver::create(context)->resolve(
                    rtl::OUString::createFromAscii(
                        "uno:socket,host=localhost,port=3830;urp;test")),
                css::uno::UNO_QUERY_THROW);
    } catch (css::connection::NoConnectException & e) {
        throw css::lang::WrappedTargetRuntimeException(
            rtl::OUString::createFromAscii(
                "com.sun.star.uno.UnoUrlResolver.resolve"),
            static_cast< cppu::OWeakObject * >(this), css::uno::makeAny(e));
    } catch (css::connection::ConnectionSetupException & e) {
        throw css::lang::WrappedTargetRuntimeException(
            rtl::OUString::createFromAscii(
                "com.sun.star.uno.UnoUrlResolver.resolve"),
            static_cast< cppu::OWeakObject * >(this), css::uno::makeAny(e));
    } catch (css::lang::IllegalArgumentException & e) {
        throw css::lang::WrappedTargetRuntimeException(
            rtl::OUString::createFromAscii(
                "com.sun.star.uno.UnoUrlResolver.resolve"),
            static_cast< cppu::OWeakObject * >(this), css::uno::makeAny(e));
    }
    bool success = source->get() == 12345;
    std::cout << "success? " << (success ? "yes" : "no") << '\n';
    return success ? 0 : 1;
}

sal_Int32 Client::get() throw (css::uno::RuntimeException) {
    return reinterpret_cast< sal_Int32 >(data.getData());
}

css::uno::Reference< css::uno::XInterface > SAL_CALL create(
    css::uno::Reference< css::uno::XComponentContext > const & context)
    SAL_THROW((css::uno::Exception))
{
    return static_cast< cppu::OWeakObject * >(new Client(context));
}

rtl::OUString SAL_CALL getImplementationName() {
    return rtl::OUString::createFromAscii(
        "test.javauno.nativethreadpool.client");
}

css::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames() {
    return css::uno::Sequence< rtl::OUString >();
}

cppu::ImplementationEntry entries[] = {
    { &create, &getImplementationName, &getSupportedServiceNames,
      &cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
};

}

extern "C" void * SAL_CALL component_getFactory(
    char const * implName, void * serviceManager, void * registryKey)
{
    return cppu::component_getFactoryHelper(
        implName, serviceManager, registryKey, entries);
}

extern "C" void SAL_CALL component_getImplementationEnvironment(
    char const ** envTypeName, uno_Environment **)
{
    *envTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
