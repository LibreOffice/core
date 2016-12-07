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


#include <sal/config.h>

#include <test/cppuhelper/propertysetmixin/XSupplier.hpp>
#include <test/cppuhelper/propertysetmixin/XTest3.hpp>

#include <com/sun/star/beans/Ambiguous.hpp>
#include <com/sun/star/beans/Defaulted.hpp>
#include <com/sun/star/beans/Optional.hpp>
#include <com/sun/star/beans/PropertyVetoException.hpp>
#include <com/sun/star/beans/UnknownPropertyException.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/propertysetmixin.hxx>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implementationentry.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <osl/mutex.hxx>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace com { namespace sun { namespace star {
    class XEventListener;
} } }

namespace {

class Empty1:
    public cppu::OWeakObject, public css::lang::XComponent,
    public cppu::PropertySetMixin< css::lang::XComponent >
{
public:
    explicit Empty1(
        css::uno::Reference< css::uno::XComponentContext > const & context):
        cppu::PropertySetMixin< css::lang::XComponent >(
            context, static_cast< Implements >(0),
            css::uno::Sequence< rtl::OUString >())
    {}

    virtual css::uno::Any SAL_CALL queryInterface(css::uno::Type const & type)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL acquire() throw () { OWeakObject::acquire(); }

    virtual void SAL_CALL release() throw () { OWeakObject::release(); }

    virtual void SAL_CALL dispose() throw (css::uno::RuntimeException) {
        cppu::PropertySetMixin< css::lang::XComponent >::dispose();
    }

    virtual void SAL_CALL addEventListener(
        css::uno::Reference< css::lang::XEventListener > const &)
        throw (css::uno::RuntimeException)
    {}

    virtual void SAL_CALL removeEventListener(
        css::uno::Reference< css::lang::XEventListener > const &)
        throw (css::uno::RuntimeException)
    {}

private:
    Empty1(Empty1 &); // not defined
    void operator =(Empty1 &); // not defined

    virtual ~Empty1() {}
};

css::uno::Any Empty1::queryInterface(css::uno::Type const & type)
    throw (css::uno::RuntimeException)
{
    css::uno::Any a(OWeakObject::queryInterface(type));
    if (a.hasValue()) {
        return a;
    }
    a = cppu::queryInterface(
        type, static_cast< css::lang::XComponent * >(this));
    return a.hasValue()
        ? a
        : cppu::PropertySetMixin< css::lang::XComponent >::queryInterface(
            type);
}

class Empty2:
    public cppu::OWeakObject, public css::lang::XComponent,
    public cppu::PropertySetMixin< css::lang::XComponent >
{
public:
    explicit Empty2(
        css::uno::Reference< css::uno::XComponentContext > const & context):
        cppu::PropertySetMixin< css::lang::XComponent >(
            context,
            static_cast< Implements >(
                IMPLEMENTS_PROPERTY_SET | IMPLEMENTS_FAST_PROPERTY_SET
                | IMPLEMENTS_PROPERTY_ACCESS),
            css::uno::Sequence< rtl::OUString >())
    {}

    virtual css::uno::Any SAL_CALL queryInterface(css::uno::Type const & type)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL acquire() throw () { OWeakObject::acquire(); }

    virtual void SAL_CALL release() throw () { OWeakObject::release(); }

    virtual void SAL_CALL dispose() throw (css::uno::RuntimeException) {
        cppu::PropertySetMixin< css::lang::XComponent >::dispose();
    }

    virtual void SAL_CALL addEventListener(
        css::uno::Reference< css::lang::XEventListener > const &)
        throw (css::uno::RuntimeException)
    {}

    virtual void SAL_CALL removeEventListener(
        css::uno::Reference< css::lang::XEventListener > const &)
        throw (css::uno::RuntimeException)
    {}

private:
    Empty2(Empty2 &); // not defined
    void operator =(Empty2 &); // not defined

    virtual ~Empty2() {}
};

css::uno::Any Empty2::queryInterface(css::uno::Type const & type)
    throw (css::uno::RuntimeException)
{
    css::uno::Any a(OWeakObject::queryInterface(type));
    if (a.hasValue()) {
        return a;
    }
    a = cppu::queryInterface(
        type, static_cast< css::lang::XComponent * >(this));
    return a.hasValue()
        ? a
        : cppu::PropertySetMixin< css::lang::XComponent >::queryInterface(
            type);
}

css::uno::Sequence< rtl::OUString > sequenceThird() {
    css::uno::Sequence<OUString> s { rtl::OUString("Third") };
    return s;
}

class Full:
    public cppu::OWeakObject, public test::cppuhelper::propertysetmixin::XTest3,
    public cppu::PropertySetMixin<
    test::cppuhelper::propertysetmixin::XTest3 >
{
public:
    explicit Full(
        css::uno::Reference< css::uno::XComponentContext > const & context):
        cppu::PropertySetMixin<
        test::cppuhelper::propertysetmixin::XTest3 >(
            context,
            static_cast< Implements >(
                IMPLEMENTS_PROPERTY_SET | IMPLEMENTS_FAST_PROPERTY_SET
                | IMPLEMENTS_PROPERTY_ACCESS),
            sequenceThird()),
        m_a1(0),
        m_a2(
            css::beans::Defaulted< css::beans::Optional< sal_Int32 > >(
                css::beans::Optional< sal_Int32 >(), true),
            false)
    {}

    virtual css::uno::Any SAL_CALL queryInterface(css::uno::Type const & type)
        throw (css::uno::RuntimeException);

    virtual void SAL_CALL acquire() throw () { OWeakObject::acquire(); }

    virtual void SAL_CALL release() throw () { OWeakObject::release(); }

    virtual sal_Int32 SAL_CALL getFirst() throw (css::uno::RuntimeException);

    virtual void SAL_CALL setFirst(sal_Int32 value)
        throw (css::uno::RuntimeException);

    virtual
    css::beans::Ambiguous<
        css::beans::Defaulted< css::beans::Optional< sal_Int32 > > >
    SAL_CALL getSecond()
        throw (
            css::beans::UnknownPropertyException, css::uno::RuntimeException);

    virtual void SAL_CALL setSecond(
        css::beans::Ambiguous<
        css::beans::Defaulted< css::beans::Optional< ::sal_Int32 > > > const &
        value)
        throw (
            css::beans::PropertyVetoException,
            css::beans::UnknownPropertyException, css::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getThird()
        throw (
            css::beans::UnknownPropertyException, css::uno::RuntimeException);

    virtual void SAL_CALL setThird(sal_Int32 value)
        throw (
            css::beans::UnknownPropertyException, css::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL getFourth()
        throw (
            css::beans::UnknownPropertyException, css::uno::RuntimeException);

    virtual void SAL_CALL setFourth(sal_Int32 value)
        throw (
            css::beans::UnknownPropertyException, css::uno::RuntimeException);

private:
    Full(Full &); // not defined
    void operator =(Full &); // not defined

    virtual ~Full() {}

    osl::Mutex m_mutex;
    sal_Int32 m_a1;
    css::beans::Ambiguous<
        css::beans::Defaulted< css::beans::Optional< sal_Int32 > > > m_a2;
};

css::uno::Any Full::queryInterface(css::uno::Type const & type)
    throw (css::uno::RuntimeException)
{
    css::uno::Any a(OWeakObject::queryInterface(type));
    if (a.hasValue()) {
        return a;
    }
    a = cppu::queryInterface(
        type,
        static_cast< test::cppuhelper::propertysetmixin::XTest3 * >(this));
    return a.hasValue()
        ? a
        : (cppu::PropertySetMixin<
           test::cppuhelper::propertysetmixin::XTest3 >::queryInterface(
               type));
}

sal_Int32 Full::getFirst() throw (css::uno::RuntimeException) {
    osl::MutexGuard g(m_mutex);
    return m_a1;
}

void Full::setFirst(sal_Int32 value) throw (css::uno::RuntimeException) {
    prepareSet(
        rtl::OUString("First"), css::uno::Any(),
        css::uno::Any(), 0);
    osl::MutexGuard g(m_mutex);
    m_a1 = value;
}

css::beans::Ambiguous<
    css::beans::Defaulted< css::beans::Optional< sal_Int32 > > >
Full::getSecond()
    throw (css::beans::UnknownPropertyException, css::uno::RuntimeException)
{
    osl::MutexGuard g(m_mutex);
    return m_a2;
}

void Full::setSecond(
    css::beans::Ambiguous<
    css::beans::Defaulted< css::beans::Optional< ::sal_Int32 > > > const &
    value)
    throw (
        css::beans::PropertyVetoException, css::beans::UnknownPropertyException,
        css::uno::RuntimeException)
{
    css::uno::Any v;
    if (value.Value.Value.IsPresent) {
        v <<= value.Value.Value.Value;
    }
    BoundListeners l;
    prepareSet(
        rtl::OUString("Second"), css::uno::Any(),
        v, &l);
    {
        osl::MutexGuard g(m_mutex);
        m_a2 = value;
    }
    l.notify();
}

sal_Int32 Full::getThird()
    throw (css::beans::UnknownPropertyException, css::uno::RuntimeException)
{
    throw css::beans::UnknownPropertyException(
        rtl::OUString("Third"),
        static_cast< cppu::OWeakObject * >(this));
}

void Full::setThird(sal_Int32)
    throw (css::beans::UnknownPropertyException, css::uno::RuntimeException)
{
    throw css::beans::UnknownPropertyException(
        rtl::OUString("Third"),
        static_cast< cppu::OWeakObject * >(this));
}

sal_Int32 Full::getFourth()
    throw (css::beans::UnknownPropertyException, css::uno::RuntimeException)
{
    throw css::beans::UnknownPropertyException(
        rtl::OUString("Fourth"),
        static_cast< cppu::OWeakObject * >(this));
}

void Full::setFourth(sal_Int32)
    throw (css::beans::UnknownPropertyException, css::uno::RuntimeException)
{
    throw css::beans::UnknownPropertyException(
        rtl::OUString("Fourth"),
        static_cast< cppu::OWeakObject * >(this));
}

class Supplier:
    public cppu::WeakImplHelper<
    test::cppuhelper::propertysetmixin::XSupplier >
{
public:
    explicit Supplier(
        css::uno::Reference< css::uno::XComponentContext > const & context):
        m_context(context) {}

    virtual css::uno::Reference< css::lang::XComponent > SAL_CALL getEmpty1()
        throw (css::uno::RuntimeException)
    { return new Empty1(m_context); }

    virtual css::uno::Reference< css::lang::XComponent > SAL_CALL getEmpty2()
        throw (css::uno::RuntimeException)
    { return new Empty2(m_context); }

    virtual css::uno::Reference< test::cppuhelper::propertysetmixin::XTest3 >
    SAL_CALL getFull() throw (css::uno::RuntimeException)
    { return new Full(m_context); }

private:
    Supplier(Supplier &); // not defined
    void operator =(Supplier &); // not defined

    virtual ~Supplier() {}

    css::uno::Reference< css::uno::XComponentContext > m_context;
};

css::uno::Reference< css::uno::XInterface > SAL_CALL create(
    css::uno::Reference< css::uno::XComponentContext > const & context)
{
    return static_cast< cppu::OWeakObject * >(new Supplier(context));
}

rtl::OUString SAL_CALL getImplementationName() {
    return rtl::OUString(
            "test.cppuhelper.propertysetmixin.comp.CppSupplier");
}

css::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames()
{
    css::uno::Sequence< rtl::OUString > s { "test.cppuhelper.propertysetmixin.CppSupplier") };
    return s;
}

cppu::ImplementationEntry entries[] = {
    { &create, &getImplementationName, &getSupportedServiceNames,
      &cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 } };

}

extern "C" SAL_DLLPUBLIC_EXPORT void * SAL_CALL mixin_component_getFactory(
    char const * implName, void * serviceManager, void * registryKey)
{
    return cppu::component_getFactoryHelper(
        implName, serviceManager, registryKey, entries);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
