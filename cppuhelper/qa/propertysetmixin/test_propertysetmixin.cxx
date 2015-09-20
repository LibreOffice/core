/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cppuhelper.hxx"

#include "sal/config.h"

#include "test/cppuhelper/propertysetmixin/CppSupplier.hpp"
#include "test/cppuhelper/propertysetmixin/JavaSupplier.hpp"
#include "test/cppuhelper/propertysetmixin/XSupplier.hpp"
#include "test/cppuhelper/propertysetmixin/XTest3.hpp"

#include "com/sun/star/beans/Ambiguous.hpp"
#include "com/sun/star/beans/Defaulted.hpp"
#include "com/sun/star/beans/Optional.hpp"
#include "com/sun/star/beans/Property.hpp"
#include "com/sun/star/beans/PropertyAttribute.hpp"
#include "com/sun/star/beans/PropertyChangeEvent.hpp"
#include "com/sun/star/beans/PropertyState.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/beans/PropertyVetoException.hpp"
#include "com/sun/star/beans/UnknownPropertyException.hpp"
#include "com/sun/star/beans/XFastPropertySet.hpp"
#include "com/sun/star/beans/XPropertyAccess.hpp"
#include "com/sun/star/beans/XPropertyChangeListener.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/beans/XPropertySetInfo.hpp"
#include "com/sun/star/beans/XVetoableChangeListener.hpp"
#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/Type.hxx"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "cppuhelper/bootstrap.hxx"
#include "cppuhelper/implbase1.hxx"
#include "gtest/gtest.h"
#include "osl/mutex.hxx"
#include "rtl/ref.hxx"
#include "rtl/string.h"
#include "rtl/textenc.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "osl/process.h"

#include <limits>
#include <ostream>

namespace com { namespace sun { namespace star {
    struct EventObject;
} } }

namespace css = com::sun::star;

namespace {

std::ostream & operator <<(std::ostream & out, rtl::OUString const & value) {
    return out << rtl::OUStringToOString(value, RTL_TEXTENCODING_UTF8).getStr();
}

std::ostream & operator <<(std::ostream & out, css::uno::Type const & value) {
    return out << "com::sun::star::uno::Type[" << value.getTypeName() << ']';
}

std::ostream & operator <<(std::ostream & out, css::uno::Any const & value) {
    return
        out << "com::sun::star::uno::Any[" << value.getValueType() << ", ...]";
}

class BoundListener:
    public cppu::WeakImplHelper1< css::beans::XPropertyChangeListener >
{
public:
    BoundListener(): m_count(0) {}

    int count() const {
        osl::MutexGuard g(m_mutex);
        return m_count;
    }

    virtual void SAL_CALL disposing(css::lang::EventObject const &)
        throw (css::uno::RuntimeException)
    {
        osl::MutexGuard g(m_mutex);
        ASSERT_TRUE(m_count < std::numeric_limits< int >::max());
        ++m_count;
    }

    virtual void SAL_CALL propertyChange(
        css::beans::PropertyChangeEvent const &)
        throw (css::uno::RuntimeException)
    { FAIL() << "BoundListener::propertyChange called"; }

private:
    BoundListener(BoundListener &); // not defined
    void operator =(BoundListener &); // not defined

    virtual ~BoundListener() {}

    mutable osl::Mutex m_mutex;
    int m_count;
};

class VetoListener:
    public cppu::WeakImplHelper1< css::beans::XVetoableChangeListener >
{
public:
    VetoListener(): m_count(0) {}

    int count() const {
        osl::MutexGuard g(m_mutex);
        return m_count;
    }

    virtual void SAL_CALL disposing(css::lang::EventObject const &)
        throw (css::uno::RuntimeException)
    {
        osl::MutexGuard g(m_mutex);
        ASSERT_TRUE(m_count < std::numeric_limits< int >::max());
        ++m_count;
    }

    virtual void SAL_CALL vetoableChange(
        css::beans::PropertyChangeEvent const &)
        throw (css::beans::PropertyVetoException, css::uno::RuntimeException)
    { FAIL() << "VetoListener::vetoableChange called"; }

private:
    VetoListener(VetoListener &); // not defined
    void operator =(VetoListener &); // not defined

    virtual ~VetoListener() {}

    mutable osl::Mutex m_mutex;
    int m_count;
};

class Test: public ::testing::Test {
public:
    virtual void SetUp();

    virtual void TearDown();

protected:
    css::uno::Reference< test::cppuhelper::propertysetmixin::XSupplier >
    getCppSupplier() const;

    css::uno::Reference< test::cppuhelper::propertysetmixin::XSupplier >
    getJavaSupplier() const;

    void testEmpty1(
        css::uno::Reference< test::cppuhelper::propertysetmixin::XSupplier >
        const & supplier) const;

    void testEmpty2(
        css::uno::Reference< test::cppuhelper::propertysetmixin::XSupplier >
        const & supplier) const;

    void testFull(
        css::uno::Reference< test::cppuhelper::propertysetmixin::XSupplier >
        const & supplier) const;

    css::uno::Reference< css::uno::XComponentContext > m_context;
};

void Test::SetUp() {
    m_context = cppu::defaultBootstrap_InitialComponentContext();
    ASSERT_TRUE(m_context.is());
}

void Test::TearDown() {
    css::uno::Reference< css::lang::XComponent >(
        m_context, css::uno::UNO_QUERY_THROW)->dispose();
}

css::uno::Reference< test::cppuhelper::propertysetmixin::XSupplier >
Test::getCppSupplier() const
{
    return test::cppuhelper::propertysetmixin::CppSupplier::create(m_context);
}

css::uno::Reference< test::cppuhelper::propertysetmixin::XSupplier >
Test::getJavaSupplier() const
{
    return test::cppuhelper::propertysetmixin::JavaSupplier::create(m_context);
}

void Test::testEmpty1(
    css::uno::Reference< test::cppuhelper::propertysetmixin::XSupplier >
    const & supplier) const
{
    css::uno::Reference< css::lang::XComponent > empty1(
        supplier->getEmpty1(), css::uno::UNO_QUERY_THROW);
    ASSERT_TRUE(
        !css::uno::Reference< css::beans::XPropertySet >(
            empty1, css::uno::UNO_QUERY).is());
    ASSERT_TRUE(
        !css::uno::Reference< css::beans::XFastPropertySet >(
            empty1, css::uno::UNO_QUERY).is());
    ASSERT_TRUE(
        !css::uno::Reference< css::beans::XPropertyAccess >(
            empty1, css::uno::UNO_QUERY).is());
    empty1->dispose();
}

void Test::testEmpty2(
    css::uno::Reference< test::cppuhelper::propertysetmixin::XSupplier >
    const & supplier) const
{
    css::uno::Reference< css::lang::XComponent > empty2(
        supplier->getEmpty2(), css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::beans::XPropertySet > empty2p(
        empty2, css::uno::UNO_QUERY);
    ASSERT_TRUE(empty2p.is());
    css::uno::Reference< css::beans::XPropertySetInfo > info(
        empty2p->getPropertySetInfo());
    ASSERT_TRUE(info.is());
    ASSERT_EQ(
        static_cast< sal_Int32 >(0), info->getProperties().getLength());
    try {
        info->getPropertyByName(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("any")));
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
    ASSERT_TRUE(
        !info->hasPropertyByName(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("any"))));
    try {
        empty2p->setPropertyValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("any")), css::uno::Any());
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
    try {
        empty2p->getPropertyValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("any")));
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
    rtl::Reference< BoundListener > boundListener1(new BoundListener);
    empty2p->addPropertyChangeListener(rtl::OUString(), boundListener1.get());
    empty2p->addPropertyChangeListener(rtl::OUString(), boundListener1.get());
    rtl::Reference< BoundListener > boundListener2(new BoundListener);
    empty2p->removePropertyChangeListener(
        rtl::OUString(), boundListener2.get());
    rtl::Reference< VetoListener > vetoListener1(new VetoListener);
    empty2p->addVetoableChangeListener(rtl::OUString(), vetoListener1.get());
    empty2p->addVetoableChangeListener(rtl::OUString(), vetoListener1.get());
    rtl::Reference< VetoListener > vetoListener2(new VetoListener);
    empty2p->addVetoableChangeListener(rtl::OUString(), vetoListener2.get());
    empty2p->removeVetoableChangeListener(rtl::OUString(), vetoListener2.get());
    css::uno::Reference< css::beans::XFastPropertySet > empty2f(
        empty2, css::uno::UNO_QUERY);
    ASSERT_TRUE(empty2f.is());
    try {
        empty2f->setFastPropertyValue(-1, css::uno::Any());
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
    try {
        empty2f->setFastPropertyValue(0, css::uno::Any());
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
    try {
        empty2f->getFastPropertyValue(-1);
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
    try {
        empty2f->getFastPropertyValue(0);
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
    css::uno::Reference< css::beans::XPropertyAccess > empty2a(
        empty2, css::uno::UNO_QUERY);
    ASSERT_TRUE(empty2a.is());
    ASSERT_EQ(
        static_cast< sal_Int32 >(0), empty2a->getPropertyValues().getLength());
    empty2a->setPropertyValues(
        css::uno::Sequence< css::beans::PropertyValue >());
    css::uno::Sequence< css::beans::PropertyValue > vs(2);
    vs[0].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("any1"));
    vs[0].Handle = -1;
    vs[0].State = css::beans::PropertyState_DIRECT_VALUE;
    vs[0].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("any2"));
    vs[0].Handle = -1;
    vs[0].State = css::beans::PropertyState_DIRECT_VALUE;
    try {
        empty2a->setPropertyValues(vs);
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
    ASSERT_EQ(0, boundListener1->count());
    ASSERT_EQ(0, boundListener2->count());
    ASSERT_EQ(0, vetoListener1->count());
    ASSERT_EQ(0, vetoListener2->count());
    empty2->dispose();
    ASSERT_EQ(2, boundListener1->count());
    ASSERT_EQ(0, boundListener2->count());
    ASSERT_EQ(2, vetoListener1->count());
    ASSERT_EQ(0, vetoListener2->count());
    empty2p->removePropertyChangeListener(
        rtl::OUString(), boundListener1.get());
    empty2p->removePropertyChangeListener(
        rtl::OUString(), boundListener2.get());
    empty2p->removeVetoableChangeListener(rtl::OUString(), vetoListener1.get());
    empty2p->removeVetoableChangeListener(rtl::OUString(), vetoListener2.get());
    empty2p->addPropertyChangeListener(rtl::OUString(), boundListener1.get());
    empty2p->addPropertyChangeListener(rtl::OUString(), boundListener2.get());
    empty2p->addVetoableChangeListener(rtl::OUString(), vetoListener1.get());
    empty2p->addVetoableChangeListener(rtl::OUString(), vetoListener2.get());
    try {
        empty2p->addPropertyChangeListener(
            rtl::OUString(),
            css::uno::Reference< css::beans::XPropertyChangeListener >());
    } catch (css::uno::RuntimeException &) {}
    try {
        empty2p->addVetoableChangeListener(
            rtl::OUString(),
            css::uno::Reference< css::beans::XVetoableChangeListener >());
    } catch (css::uno::RuntimeException &) {}
    ASSERT_EQ(3, boundListener1->count());
    ASSERT_EQ(1, boundListener2->count());
    ASSERT_EQ(3, vetoListener1->count());
    ASSERT_EQ(1, vetoListener2->count());
}

void Test::testFull(
    css::uno::Reference< test::cppuhelper::propertysetmixin::XSupplier >
    const & supplier) const
{
    css::uno::Reference< test::cppuhelper::propertysetmixin::XTest3 > full(
        supplier->getFull(), css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::beans::XPropertySet > fullp(
        full, css::uno::UNO_QUERY);
    ASSERT_TRUE(fullp.is());
    css::uno::Reference< css::beans::XPropertySetInfo > info(
        fullp->getPropertySetInfo());
    ASSERT_TRUE(info.is());
    ASSERT_EQ(
        static_cast< sal_Int32 >(3), info->getProperties().getLength());
    css::beans::Property prop(
        info->getPropertyByName(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("First"))));
    ASSERT_EQ(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("First")), prop.Name);
    ASSERT_EQ(static_cast< sal_Int32 >(0), prop.Handle);
    ASSERT_EQ(getCppuType(static_cast< sal_Int32 * >(0)), prop.Type);
    ASSERT_EQ(static_cast< sal_Int16 >(0), prop.Attributes);
    prop = info->getPropertyByName(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Second")));
    ASSERT_EQ(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Second")), prop.Name);
    ASSERT_EQ(static_cast< sal_Int32 >(1), prop.Handle);
    ASSERT_EQ(getCppuType(static_cast< sal_Int32 * >(0)), prop.Type);
    ASSERT_EQ(
        static_cast< sal_Int16 >(
            css::beans::PropertyAttribute::MAYBEVOID
            | css::beans::PropertyAttribute::BOUND
            | css::beans::PropertyAttribute::CONSTRAINED
            | css::beans::PropertyAttribute::MAYBEAMBIGUOUS
            | css::beans::PropertyAttribute::MAYBEDEFAULT
            | css::beans::PropertyAttribute::OPTIONAL),
        prop.Attributes);
    try {
        info->getPropertyByName(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Third")));
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
    prop = info->getPropertyByName(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Fourth")));
    ASSERT_EQ(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Fourth")), prop.Name);
    ASSERT_EQ(static_cast< sal_Int32 >(3), prop.Handle);
    ASSERT_EQ(getCppuType(static_cast< sal_Int32 * >(0)), prop.Type);
    ASSERT_EQ(
        static_cast< sal_Int16 >(css::beans::PropertyAttribute::OPTIONAL),
        prop.Attributes);
    try {
        info->getPropertyByName(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("first")));
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
    ASSERT_TRUE(
        info->hasPropertyByName(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("First"))));
    ASSERT_TRUE(
        info->hasPropertyByName(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Second"))));
    ASSERT_TRUE(
        !info->hasPropertyByName(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Third"))));
    ASSERT_TRUE(
        info->hasPropertyByName(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Fourth"))));
    ASSERT_TRUE(
        !info->hasPropertyByName(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("first"))));
    ASSERT_EQ(
        css::uno::makeAny(static_cast< sal_Int32 >(0)),
        fullp->getPropertyValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("First"))));
    fullp->setPropertyValue(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("First")),
        css::uno::makeAny(static_cast< sal_Int32 >(-100)));
    ASSERT_EQ(
        css::uno::makeAny(static_cast< sal_Int32 >(-100)),
        fullp->getPropertyValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("First"))));
    css::uno::Any voidAny;
    ASSERT_EQ(
        voidAny,
        fullp->getPropertyValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Second"))));
    fullp->setPropertyValue(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Second")),
        css::uno::makeAny(static_cast< sal_Int32 >(100)));
    ASSERT_EQ(
        css::uno::makeAny(static_cast< sal_Int32 >(100)),
        fullp->getPropertyValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Second"))));
    ASSERT_TRUE(full->getSecond().Value.Value.IsPresent);
    ASSERT_EQ(
        static_cast< sal_Int32 >(100), full->getSecond().Value.Value.Value);
    ASSERT_TRUE(!full->getSecond().Value.IsDefaulted);
    ASSERT_TRUE(!full->getSecond().IsAmbiguous);
    fullp->setPropertyValue(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Second")),
        css::uno::Any());
    ASSERT_EQ(
        voidAny,
        fullp->getPropertyValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Second"))));
    ASSERT_TRUE(!full->getSecond().Value.Value.IsPresent);
    ASSERT_TRUE(!full->getSecond().Value.IsDefaulted);
    ASSERT_TRUE(!full->getSecond().IsAmbiguous);
    try {
        fullp->setPropertyValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Third")),
            css::uno::makeAny(static_cast< sal_Int32 >(100)));
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
    try {
        fullp->getPropertyValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Third")));
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
    try {
        fullp->setPropertyValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Fourth")),
            css::uno::makeAny(static_cast< sal_Int32 >(100)));
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
    try {
        fullp->getPropertyValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Fourth")));
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
    try {
        fullp->setPropertyValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("first")),
            css::uno::Any());
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
    try {
        fullp->getPropertyValue(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("first")));
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
    css::uno::Reference< css::beans::XFastPropertySet > fullf(
        full, css::uno::UNO_QUERY);
    ASSERT_TRUE(fullf.is());
    ASSERT_EQ(
        css::uno::makeAny(static_cast< sal_Int32 >(-100)),
        fullf->getFastPropertyValue(0));
    fullf->setFastPropertyValue(
        0, css::uno::makeAny(static_cast< sal_Int32 >(0)));
    ASSERT_EQ(
        css::uno::makeAny(static_cast< sal_Int32 >(0)),
        fullf->getFastPropertyValue(0));
    try {
        fullf->getFastPropertyValue(-1);
    } catch (css::beans::UnknownPropertyException &) {}
    try {
        fullf->setFastPropertyValue(-1, css::uno::Any());
    } catch (css::beans::UnknownPropertyException &) {}
    css::uno::Reference< css::beans::XPropertyAccess > fulla(
        full, css::uno::UNO_QUERY);
    ASSERT_TRUE(fulla.is());
    css::uno::Sequence< css::beans::PropertyValue > vs(
        fulla->getPropertyValues());
    ASSERT_EQ(static_cast< sal_Int32 >(2), vs.getLength());
    ASSERT_EQ(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("First")), vs[0].Name);
    ASSERT_EQ(static_cast< sal_Int32 >(0), vs[0].Handle);
    ASSERT_EQ(
        css::uno::makeAny(static_cast< sal_Int32 >(0)), vs[0].Value);
    ASSERT_EQ(css::beans::PropertyState_DIRECT_VALUE, vs[0].State);
    ASSERT_EQ(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Second")), vs[1].Name);
    ASSERT_EQ(static_cast< sal_Int32 >(1), vs[1].Handle);
    ASSERT_EQ(voidAny, vs[1].Value);
    ASSERT_EQ(css::beans::PropertyState_DIRECT_VALUE, vs[1].State);
    vs[0].Value <<= static_cast< sal_Int32 >(-100);
    vs[1].Value <<= static_cast< sal_Int32 >(100);
    vs[1].State = css::beans::PropertyState_AMBIGUOUS_VALUE;
    fulla->setPropertyValues(vs);
    vs = fulla->getPropertyValues();
    ASSERT_EQ(static_cast< sal_Int32 >(2), vs.getLength());
    ASSERT_EQ(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("First")), vs[0].Name);
    ASSERT_EQ(
        css::uno::makeAny(static_cast< sal_Int32 >(-100)), vs[0].Value);
    ASSERT_EQ(css::beans::PropertyState_DIRECT_VALUE, vs[0].State);
    ASSERT_EQ(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Second")), vs[1].Name);
    ASSERT_EQ(
        css::uno::makeAny(static_cast< sal_Int32 >(100)), vs[1].Value);
    ASSERT_EQ(
        css::beans::PropertyState_AMBIGUOUS_VALUE, vs[1].State);
    ASSERT_TRUE(full->getSecond().Value.Value.IsPresent);
    ASSERT_EQ(
        static_cast< sal_Int32 >(100), full->getSecond().Value.Value.Value);
    ASSERT_TRUE(!full->getSecond().Value.IsDefaulted);
    ASSERT_TRUE(full->getSecond().IsAmbiguous);
    css::uno::Reference< css::beans::XPropertyChangeListener > boundListener(
        new BoundListener);
    fullp->addPropertyChangeListener(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("First")), boundListener);
    fullp->removePropertyChangeListener(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("First")), boundListener);
    fullp->addPropertyChangeListener(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Second")), boundListener);
    fullp->removePropertyChangeListener(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Second")), boundListener);
    try {
        fullp->addPropertyChangeListener(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Third")),
            boundListener);
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
    try {
        fullp->removePropertyChangeListener(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Third")),
            boundListener);
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
    fullp->addPropertyChangeListener(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Fourth")), boundListener);
    fullp->removePropertyChangeListener(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Fourth")), boundListener);
    try {
        fullp->addPropertyChangeListener(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Fifth")),
            boundListener);
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
    try {
        fullp->removePropertyChangeListener(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Fifth")),
            boundListener);
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
    css::uno::Reference< css::beans::XVetoableChangeListener > vetoListener(
        new VetoListener);
    fullp->addVetoableChangeListener(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("First")), vetoListener);
    fullp->removeVetoableChangeListener(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("First")), vetoListener);
    fullp->addVetoableChangeListener(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Second")), vetoListener);
    fullp->removeVetoableChangeListener(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Second")), vetoListener);
    try {
        fullp->addVetoableChangeListener(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Third")),
            vetoListener);
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
    try {
        fullp->removeVetoableChangeListener(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Third")),
            vetoListener);
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
    fullp->addVetoableChangeListener(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Fourth")), vetoListener);
    fullp->removeVetoableChangeListener(
        rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Fourth")), vetoListener);
    try {
        fullp->addVetoableChangeListener(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Fifth")),
            vetoListener);
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
    try {
        fullp->removeVetoableChangeListener(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Fifth")),
            vetoListener);
        FAIL() << "exception expected";
    } catch (css::beans::UnknownPropertyException &) {}
}

TEST_F(Test, testCppEmpty1) { testEmpty1(getCppSupplier()); }

TEST_F(Test, testCppEmpty2) { testEmpty2(getCppSupplier()); }

TEST_F(Test, testCppFull) { testFull(getCppSupplier()); }

TEST_F(Test, testJavaEmpty1) { testEmpty1(getJavaSupplier()); }

TEST_F(Test, testJavaEmpty2) { testEmpty2(getJavaSupplier()); }

TEST_F(Test, testJavaFull) { testFull(getJavaSupplier()); }



}

int main(int argc, char **argv)
{
    osl_setCommandArgs(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

