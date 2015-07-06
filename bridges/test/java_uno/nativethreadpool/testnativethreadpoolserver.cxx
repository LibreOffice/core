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


#include "test/javauno/nativethreadpool/XSource.hpp"

#include "com/sun/star/bridge/UnoUrlResolver.hpp"
#include "com/sun/star/bridge/XUnoUrlResolver.hpp"
#include "com/sun/star/connection/ConnectionSetupException.hpp"
#include "com/sun/star/connection/NoConnectException.hpp"
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/lang/XMultiComponentFactory.hpp"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implbase.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/weak.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

namespace {

class Server:
    public cppu::WeakImplHelper< test::javauno::nativethreadpool::XSource >
{
public:
    explicit Server(
        css::uno::Reference< css::uno::XComponentContext > const & theContext):
        context(theContext) {}

private:
    virtual ~Server() {}

    virtual sal_Int32 SAL_CALL get() throw (css::uno::RuntimeException);

    css::uno::Reference< css::uno::XComponentContext > context;
};

sal_Int32 Server::get() throw (css::uno::RuntimeException) {
    css::uno::Reference< css::lang::XMultiComponentFactory > factory(
        context->getServiceManager());
    if (!factory.is()) {
        throw css::uno::RuntimeException(
            OUString( "no component context service manager" ),
            static_cast< cppu::OWeakObject * >(this));
    }
    css::uno::Reference< test::javauno::nativethreadpool::XSource > source;
    try {
        // Use "127.0.0.1" instead of "localhost", see #i32281#:
        source
            = css::uno::Reference< test::javauno::nativethreadpool::XSource >(
                css::bridge::UnoUrlResolver::create(context)->resolve(
                    OUString( "uno:socket,host=127.0.0.1,port=3831;urp;test" )),
                css::uno::UNO_QUERY_THROW);
    } catch (css::connection::NoConnectException & e) {
        throw css::lang::WrappedTargetRuntimeException(
            OUString( "com.sun.star.uno.UnoUrlResolver.resolve" ),
            static_cast< cppu::OWeakObject * >(this), css::uno::makeAny(e));
    } catch (css::connection::ConnectionSetupException & e) {
        throw css::lang::WrappedTargetRuntimeException(
            OUString( "com.sun.star.uno.UnoUrlResolver.resolve" ),
            static_cast< cppu::OWeakObject * >(this), css::uno::makeAny(e));
    } catch (css::lang::IllegalArgumentException & e) {
        throw css::lang::WrappedTargetRuntimeException(
            OUString( "com.sun.star.uno.UnoUrlResolver.resolve" ),
            static_cast< cppu::OWeakObject * >(this), css::uno::makeAny(e));
    }
    return source->get();
}

css::uno::Reference< css::uno::XInterface > SAL_CALL create(
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    return static_cast< cppu::OWeakObject * >(new Server(context));
}

OUString SAL_CALL getImplementationName() {
    return OUString( "test.javauno.nativethreadpool.server" );
}

css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() {
    return css::uno::Sequence< OUString >();
}

cppu::ImplementationEntry entries[] = {
    { &create, &getImplementationName, &getSupportedServiceNames,
      &cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
};

}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    char const * implName, void * serviceManager, void * registryKey)
{
    return cppu::component_getFactoryHelper(
        implName, serviceManager, registryKey, entries);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
