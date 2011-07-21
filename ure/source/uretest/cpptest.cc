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

#include "sal/config.h"

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
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "uno/environment.h"
#include "uno/lbnames.h"

#include "test/types/TestException.hpp"
#include "test/types/XTest.hpp"

namespace css = com::sun::star;

namespace {

class Service: public cppu::WeakImplHelper1< test::types::XTest > {
public:
    Service() {}

    virtual void SAL_CALL throwException()
        throw (test::types::TestException, css::uno::RuntimeException)
    {
        throw test::types::TestException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("test")),
            static_cast< cppu::OWeakObject * >(this));
    }

private:
    Service(Service &); // not defined
    void operator =(Service &); // not defined

    virtual ~Service() {}
};

namespace CppTest {

css::uno::Reference< css::uno::XInterface > create(
    css::uno::Reference< css::uno::XComponentContext > const &)
    SAL_THROW((css::uno::Exception))
{
    try {
        return static_cast< cppu::OWeakObject * >(new Service);
    } catch (std::bad_alloc &) {
        throw css::uno::RuntimeException(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("std::bad_alloc")),
            css::uno::Reference< css::uno::XInterface >());
    }
}

rtl::OUString getImplementationName() {
    return rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("test.cpp.cpptest.Component"));
}

css::uno::Sequence< rtl::OUString > getSupportedServiceNames() {
    rtl::OUString s(RTL_CONSTASCII_USTRINGPARAM("test.types.CppTest"));
    return css::uno::Sequence< rtl::OUString >(&s, 1);
}

}

cppu::ImplementationEntry entries[] = {
    { CppTest::create, CppTest::getImplementationName,
      CppTest::getSupportedServiceNames, cppu::createSingleComponentFactory, 0,
      0 },
    { 0, 0, 0, 0, 0, 0 } };

}

extern "C" sal_Bool SAL_CALL component_writeInfo(
    void * serviceManager, void * registryKey)
{
    return cppu::component_writeInfoHelper(
        serviceManager, registryKey, entries);
}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    char const * implName, void * serviceManager, void * registryKey)
{
    return cppu::component_getFactoryHelper(
        implName, serviceManager, registryKey, entries);
}
