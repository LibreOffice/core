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
#include "cppuhelper/implbase1.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/weak.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "uno/lbnames.h"

namespace css = com::sun::star;

namespace {

class Server:
    public cppu::WeakImplHelper1< test::javauno::nativethreadpool::XSource >
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
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "no component context service manager" )),
            static_cast< cppu::OWeakObject * >(this));
    }
    css::uno::Reference< test::javauno::nativethreadpool::XSource > source;
    try {
        // Use "127.0.0.1" instead of "localhost", see #i32281#:
        source
            = css::uno::Reference< test::javauno::nativethreadpool::XSource >(
                css::bridge::UnoUrlResolver::create(context)->resolve(
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "uno:socket,host=127.0.0.1,port=3831;urp;test" ))),
                css::uno::UNO_QUERY_THROW);
    } catch (css::connection::NoConnectException & e) {
        throw css::lang::WrappedTargetRuntimeException(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.uno.UnoUrlResolver.resolve" )),
            static_cast< cppu::OWeakObject * >(this), css::uno::makeAny(e));
    } catch (css::connection::ConnectionSetupException & e) {
        throw css::lang::WrappedTargetRuntimeException(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.uno.UnoUrlResolver.resolve" )),
            static_cast< cppu::OWeakObject * >(this), css::uno::makeAny(e));
    } catch (css::lang::IllegalArgumentException & e) {
        throw css::lang::WrappedTargetRuntimeException(
            rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.uno.UnoUrlResolver.resolve" )),
            static_cast< cppu::OWeakObject * >(this), css::uno::makeAny(e));
    }
    return source->get();
}

css::uno::Reference< css::uno::XInterface > SAL_CALL create(
    css::uno::Reference< css::uno::XComponentContext > const & context)
    SAL_THROW((css::uno::Exception))
{
    return static_cast< cppu::OWeakObject * >(new Server(context));
}

rtl::OUString SAL_CALL getImplementationName() {
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "test.javauno.nativethreadpool.server" ));
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

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    char const * implName, void * serviceManager, void * registryKey)
{
    return cppu::component_getFactoryHelper(
        implName, serviceManager, registryKey, entries);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
