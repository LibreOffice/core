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


#include "com/sun/star/bridge/UnoUrlResolver.hpp"
#include "com/sun/star/bridge/XUnoUrlResolver.hpp"
#include "com/sun/star/lang/XMain.hpp"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/lang/XSingleComponentFactory.hpp"
#include "com/sun/star/registry/InvalidRegistryException.hpp"
#include "com/sun/star/registry/XRegistryKey.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/Type.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/uno/XInterface.hpp"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implbase3.hxx"
#include "cppuhelper/weak.hxx"
#include "osl/conditn.hxx"
#include "osl/interlck.h"
#include "rtl/string.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "test/javauno/acquire/XBase.hpp"
#include "test/javauno/acquire/XDerived.hpp"
#include "test/javauno/acquire/XTest.hpp"
#include "uno/environment.h"
#include "uno/lbnames.h"

#include <iostream>
#include <cstdlib>

namespace css = com::sun::star;

namespace {

class WaitCondition {
public:
    WaitCondition() {}

    ~WaitCondition();

    osl::Condition & get() { return m_condition; }

private:
    WaitCondition(WaitCondition &); // not implemented
    void operator =(WaitCondition); // not implemented

    osl::Condition m_condition;
};

}

WaitCondition::~WaitCondition() {
    std::cout << "waiting for condition\n";
    if (m_condition.wait() != osl::Condition::result_ok) {
       std::cerr << "osl::Condition::wait failed\n";
       std::abort()
    }
}

namespace {

class Interface: public css::uno::XInterface {
public:
    explicit Interface(osl::Condition & condition):
        m_condition(condition), m_refCount(0) {}

    virtual css::uno::Any SAL_CALL queryInterface(css::uno::Type const & type)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL acquire() throw ()
    { osl_atomic_increment(&m_refCount); }

    virtual void SAL_CALL release() throw ();

protected:
    virtual ~Interface() { m_condition.set(); }

private:
    Interface(Interface &); // not implemented
    void operator =(Interface); // not implemented

    osl::Condition & m_condition;
    oslInterlockedCount m_refCount;
};

}

css::uno::Any Interface::queryInterface(css::uno::Type const & type)
    throw (css::uno::RuntimeException)
{
    return type.getTypeName() == "com.sun.star.uno.XInterface"
        ? css::uno::makeAny(css::uno::Reference< css::uno::XInterface >(this))
        : css::uno::Any();
}

void Interface::release() throw () {
    if (osl_atomic_decrement(&m_refCount) == 0) {
        delete this;
    }
}

namespace {

class Base: public Interface, public test::javauno::acquire::XBase {
public:
    explicit Base(osl::Condition & condition): Interface(condition) {}

    virtual css::uno::Any SAL_CALL queryInterface(css::uno::Type const & type)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL acquire() throw () { Interface::acquire(); }

    virtual void SAL_CALL release() throw () { Interface::release(); }

protected:
    virtual ~Base() {}
};

}

css::uno::Any Base::queryInterface(css::uno::Type const & type)
    throw (css::uno::RuntimeException)
{
    return type.getTypeName() == "test.javauno.acquire.XBase"
        ? css::uno::makeAny(css::uno::Reference< test::javauno::acquire::XBase >(this))
        : Interface::queryInterface(type);
}

namespace {

class Derived: public Base, public test::javauno::acquire::XDerived {
public:
    explicit Derived(osl::Condition & condition): Base(condition) {}

    virtual css::uno::Any SAL_CALL queryInterface(css::uno::Type const & type)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL acquire() throw () { Base::acquire(); }

    virtual void SAL_CALL release() throw () { Base::release(); }

private:
    virtual ~Derived() {}
};

}

css::uno::Any Derived::queryInterface(css::uno::Type const & type)
    throw (css::uno::RuntimeException)
{
    return ( type.getTypeName() == "test.javauno.acquire.XDerived" )
        ? css::uno::makeAny(
            css::uno::Reference< test::javauno::acquire::XDerived >(this))
        : Interface::queryInterface(type);
}

namespace {

class Service: public cppu::WeakImplHelper3<
    css::lang::XServiceInfo, css::lang::XMain, test::javauno::acquire::XTest >
{
public:
    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException)
    { return getImplementationName_static(); }

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const & serviceName)
        throw (css::uno::RuntimeException);

    virtual css::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames()  throw (css::uno::RuntimeException)
    { return getSupportedServiceNames_static(); }

    virtual sal_Int32 SAL_CALL
    run(css::uno::Sequence< rtl::OUString > const & arguments)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL setInterfaceToInterface(
        css::uno::Reference< css::uno::XInterface > const & obj)
        throw (css::uno::RuntimeException)
    { m_interface = obj; }

    virtual void SAL_CALL setBaseToInterface(
        css::uno::Reference< test::javauno::acquire::XBase > const & obj)
        throw (css::uno::RuntimeException)
    { m_interface = obj; }

    virtual void SAL_CALL setDerivedToInterface(
        css::uno::Reference< test::javauno::acquire::XDerived > const & obj)
        throw (css::uno::RuntimeException)
    { m_interface = obj; }

    virtual css::uno::Reference< css::uno::XInterface >
    SAL_CALL getInterfaceFromInterface() throw (css::uno::RuntimeException)
    { return m_interface; }

    virtual void SAL_CALL clearInterface() throw (css::uno::RuntimeException)
    { m_interface.clear(); }

    virtual void SAL_CALL setBaseToBase(
        css::uno::Reference< test::javauno::acquire::XBase > const & obj)
        throw (css::uno::RuntimeException)
    { m_base = obj; }

    virtual void SAL_CALL setDerivedToBase(
        css::uno::Reference< test::javauno::acquire::XDerived > const & obj)
        throw (css::uno::RuntimeException)
    { m_base = obj.get(); }

    virtual css::uno::Reference< css::uno::XInterface >
    SAL_CALL getInterfaceFromBase() throw (css::uno::RuntimeException)
    { return m_base; }

    virtual css::uno::Reference< test::javauno::acquire::XBase >
    SAL_CALL getBaseFromBase() throw (css::uno::RuntimeException)
    { return m_base; }

    virtual void SAL_CALL clearBase() throw (css::uno::RuntimeException)
    { m_base.clear(); }

    virtual void SAL_CALL setDerivedToDerived(
        css::uno::Reference< test::javauno::acquire::XDerived > const & obj)
        throw (css::uno::RuntimeException)
    { m_derived = obj; }

    virtual css::uno::Reference< css::uno::XInterface >
    SAL_CALL getInterfaceFromDerived() throw (css::uno::RuntimeException)
    { return m_derived; }

    virtual css::uno::Reference< test::javauno::acquire::XBase >
    SAL_CALL getBaseFromDerived() throw (css::uno::RuntimeException)
    { return m_derived.get(); }

    virtual css::uno::Reference< test::javauno::acquire::XDerived >
    SAL_CALL getDerivedFromDerived() throw (css::uno::RuntimeException)
    { return m_derived; }

    virtual void SAL_CALL clearDerived() throw (css::uno::RuntimeException)
    { m_derived.clear(); }

    virtual css::uno::Reference< css::uno::XInterface >
    SAL_CALL roundTripInterfaceToInterface(
        css::uno::Reference< css::uno::XInterface > const & obj)
        throw (css::uno::RuntimeException)
    { return obj; }

    virtual css::uno::Reference< css::uno::XInterface >
    SAL_CALL roundTripBaseToInterface(
        css::uno::Reference< test::javauno::acquire::XBase > const & obj)
        throw (css::uno::RuntimeException)
    { return obj; }

    virtual css::uno::Reference< css::uno::XInterface >
    SAL_CALL roundTripDerivedToInterface(
        css::uno::Reference< test::javauno::acquire::XDerived > const & obj)
        throw (css::uno::RuntimeException)
    { return obj; }

    virtual css::uno::Reference< test::javauno::acquire::XBase >
    SAL_CALL roundTripBaseToBase(
        css::uno::Reference< test::javauno::acquire::XBase > const & obj)
        throw (css::uno::RuntimeException)
    { return obj; }

    virtual css::uno::Reference< test::javauno::acquire::XBase >
    SAL_CALL roundTripDerivedToBase(
        css::uno::Reference< test::javauno::acquire::XDerived > const & obj)
        throw (css::uno::RuntimeException)
    { return obj.get(); }

    virtual css::uno::Reference< test::javauno::acquire::XDerived >
    SAL_CALL roundTripDerivedToDerived(
        css::uno::Reference< test::javauno::acquire::XDerived > const & obj)
        throw (css::uno::RuntimeException)
    { return obj; }

    static rtl::OUString getImplementationName_static();

    static css::uno::Sequence< rtl::OUString >
    getSupportedServiceNames_static();

    static css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance(
        css::uno::Reference< css::uno::XComponentContext > const & context)
        throw (css::uno::Exception);

private:
    explicit Service(
        css::uno::Reference< css::uno::XComponentContext > const & context):
        m_context(context) {}

    css::uno::Reference< css::uno::XComponentContext > m_context;
    css::uno::Reference< css::uno::XInterface > m_interface;
    css::uno::Reference< test::javauno::acquire::XBase > m_base;
    css::uno::Reference< test::javauno::acquire::XDerived > m_derived;
};

}

sal_Bool Service::supportsService(rtl::OUString const & serviceName)
    throw (css::uno::RuntimeException)
{
    css::uno::Sequence< rtl::OUString > names(
        getSupportedServiceNames_static());
    for (sal_Int32 i = 0; i< names.getLength(); ++i) {
        if (names[i] == serviceName) {
            return true;
        }
    }
    return false;
}

namespace {

template< typename T > void assertNotNull(css::uno::Reference< T > const & ref)
{
    if (!ref.is()) {
        std::cerr << "assertNotNull failed\n";
        std::abort();
    }
}

}

sal_Int32 Service::run(css::uno::Sequence< rtl::OUString > const & arguments)
    throw (css::uno::RuntimeException)
{
    // - arguments[0] must be the UNO URL to connect to:
    css::uno::Reference< XTest > test(
        css::bridge::UnoUrlResolver::create(m_context)->resolve(arguments[0]),
        css::uno::UNO_QUERY_THROW);

    {
        WaitCondition c;
        test->setInterfaceToInterface(new Interface(c.get()));
        assertNotNull(test->getInterfaceFromInterface());
        test->clearInterface();
    }
    {
        WaitCondition c;
        test->setInterfaceToInterface(
            static_cast< Interface * >(new Base(c.get())));
        assertNotNull(test->getInterfaceFromInterface());
        test->clearInterface();
    }
    {
        WaitCondition c;
        test->setInterfaceToInterface(
            static_cast< Interface * >(new Derived(c.get())));
        assertNotNull(test->getInterfaceFromInterface());
        test->clearInterface();
    }

    {
        WaitCondition c;
        test->setBaseToInterface(new Base(c.get()));
        assertNotNull(test->getInterfaceFromInterface());
        test->clearInterface();
    }
    {
        WaitCondition c;
        test->setBaseToInterface(static_cast< Base * >(new Derived(c.get())));
        assertNotNull(test->getInterfaceFromInterface());
        test->clearInterface();
    }

    {
        WaitCondition c;
        test->setDerivedToInterface(new Derived(c.get()));
        assertNotNull(test->getInterfaceFromInterface());
        test->clearInterface();
    }

    {
        WaitCondition c;
        test->setBaseToBase(new Base(c.get()));
        assertNotNull(test->getInterfaceFromBase());
        assertNotNull(test->getBaseFromBase());
        test->clearBase();
    }
    {
        WaitCondition c;
        test->setBaseToBase(static_cast< Base * >(new Derived(c.get())));
        assertNotNull(test->getInterfaceFromBase());
        assertNotNull(test->getBaseFromBase());
        test->clearBase();
    }

    {
        WaitCondition c;
        test->setDerivedToBase(new Derived(c.get()));
        assertNotNull(test->getInterfaceFromBase());
        assertNotNull(test->getBaseFromBase());
        test->clearBase();
    }

    {
        WaitCondition c;
        test->setDerivedToDerived(new Derived(c.get()));
        assertNotNull(test->getInterfaceFromDerived());
        assertNotNull(test->getBaseFromDerived());
        assertNotNull(test->getDerivedFromDerived());
        test->clearDerived();
    }

    {
        WaitCondition c;
        assertNotNull(
            test->roundTripInterfaceToInterface(new Interface(c.get())));
    }
    {
        WaitCondition c;
        assertNotNull(test->roundTripInterfaceToInterface(
                          static_cast< Interface * >(new Base(c.get()))));
    }
    {
        WaitCondition c;
        assertNotNull(test->roundTripInterfaceToInterface(
                          static_cast< Interface * >(new Derived(c.get()))));
    }

    {
        WaitCondition c;
        assertNotNull(test->roundTripBaseToInterface(new Base(c.get())));
    }
    {
        WaitCondition c;
        assertNotNull(test->roundTripBaseToInterface(
                          static_cast< Base * >(new Derived(c.get()))));
    }

    {
        WaitCondition c;
        assertNotNull(test->roundTripDerivedToInterface(new Derived(c.get())));
    }

    {
        WaitCondition c;
        assertNotNull(test->roundTripBaseToBase(new Base(c.get())));
    }
    {
        WaitCondition c;
        assertNotNull(test->roundTripBaseToBase(
                          static_cast< Base * >(new Derived(c.get()))));
    }

    {
        WaitCondition c;
        assertNotNull(test->roundTripDerivedToBase(new Derived(c.get())));
    }

    {
        WaitCondition c;
        assertNotNull(test->roundTripDerivedToDerived(new Derived(c.get())));
    }

    std::cout << "Client and server both cleanly terminate now: Success\n";
    return 0;
}

rtl::OUString Service::getImplementationName_static() {
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.test.bridges.testacquire.impl" ));
}

css::uno::Sequence< rtl::OUString > Service::getSupportedServiceNames_static() {
    css::uno::Sequence< rtl::OUString > names(1);
    names[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.test.bridges.testacquire" ));
    return names;
}

css::uno::Reference< css::uno::XInterface > Service::createInstance(
    css::uno::Reference< css::uno::XComponentContext > const & context)
    throw (css::uno::Exception)
{
    return static_cast< cppu::OWeakObject * >(new Service(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL component_getFactory(char const * implName,
                                                void * serviceManager, void *) {
    void * p = 0;
    if (serviceManager != 0) {
        css::uno::Reference< css::lang::XSingleComponentFactory > f;
        if (Service::getImplementationName_static().equalsAscii(implName)) {
            f = cppu::createSingleComponentFactory(
                &Service::createInstance,
                Service::getImplementationName_static(),
                Service::getSupportedServiceNames_static());
        }
        if (f.is()) {
            f->acquire();
            p = f.get();
        }
    }
    return p;
}

namespace {

bool writeInfo(void * registryKey, rtl::OUString const & implementationName,
               css::uno::Sequence< rtl::OUString > const & serviceNames) {
    rtl::OUString keyName( RTL_CONSTASCII_USTRINGPARAM( "/" ));
    keyName += implementationName;
    keyName += rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/UNO/SERVICES" ));
    css::uno::Reference< css::registry::XRegistryKey > key;
    try {
        key = static_cast< css::registry::XRegistryKey * >(registryKey)->
            createKey(keyName);
    } catch (css::registry::InvalidRegistryException &) {}
    if (!key.is()) {
        return false;
    }
    bool success = true;
    for (sal_Int32 i = 0; i < serviceNames.getLength(); ++i) {
        try {
            key->createKey(serviceNames[i]);
        } catch (css::registry::InvalidRegistryException &) {
            success = false;
            break;
        }
    }
    return success;
}

}

extern "C" sal_Bool SAL_CALL component_writeInfo(void *, void * registryKey) {
    return registryKey
        && writeInfo(registryKey, Service::getImplementationName_static(),
                     Service::getSupportedServiceNames_static());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
