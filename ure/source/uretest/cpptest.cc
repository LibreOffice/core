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
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "uno/environment.h"

#include "test/types/TestException.hpp"
#include "test/types/XTest.hpp"

namespace {

class Service: public cppu::WeakImplHelper < test::types::XTest >
{
public:
    Service() {}

    virtual void SAL_CALL throwException()
        throw (test::types::TestException, css::uno::RuntimeException)
    {
        throw test::types::TestException(
            rtl::OUString("test"),
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
{
    try {
        return static_cast< cppu::OWeakObject * >(new Service);
    } catch (std::bad_alloc &) {
        throw css::uno::RuntimeException("std::bad_alloc");
    }
}

rtl::OUString getImplementationName() {
    return rtl::OUString("test.cpp.cpptest.Component");
}

css::uno::Sequence< rtl::OUString > getSupportedServiceNames() {
    rtl::OUString s("test.types.CppTest");
    return css::uno::Sequence< rtl::OUString >(&s, 1);
}

}

cppu::ImplementationEntry entries[] = {
    { CppTest::create, CppTest::getImplementationName,
      CppTest::getSupportedServiceNames, cppu::createSingleComponentFactory, 0,
      0 },
    { 0, 0, 0, 0, 0, 0 } };

}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(
    char const * implName, void * serviceManager, void * registryKey)
{
    return cppu::component_getFactoryHelper(
        implName, serviceManager, registryKey, entries);
}
