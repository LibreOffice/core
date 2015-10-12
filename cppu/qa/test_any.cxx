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

#include <sal/types.h>

#include <stdlib.h>

#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestCase.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <cppunit/extensions/HelperMacros.h>

#include "Enum1.hpp"
#include "Enum2.hpp"
#include "Exception1.hpp"
#include "Exception2.hpp"
#include "Exception2a.hpp"
#include "Exception2b.hpp"
#include "Interface1.hpp"
#include "Interface2.hpp"
#include "Interface2a.hpp"
#include "Interface2b.hpp"
#include "Interface3.hpp"
#include "Poly.hpp"
#include "Struct1.hpp"
#include "Struct2.hpp"
#include "Struct2a.hpp"
#include "Struct2b.hpp"
#include "boost/noncopyable.hpp"
#include "boost/type_traits/is_same.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/uno/Type.hxx"
#include "com/sun/star/uno/XInterface.hpp"
#include "osl/diagnose.h"
#include "osl/interlck.h"
#include "rtl/string.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

namespace {

class Base: private boost::noncopyable {
public:
    Base(): m_count(0) {}

    void acquire() {
        if (osl_atomic_increment(&m_count) == SAL_MAX_INT32) {
            abort();
        }
    }

    void release() {
        if (osl_atomic_decrement(&m_count) == 0) {
            delete this;
        }
    }

protected:
    virtual ~Base() {}

private:
    oslInterlockedCount m_count;
};

class Impl1: public Interface1, private Base {
public:
    virtual css::uno::Any SAL_CALL queryInterface(css::uno::Type const & type)
        throw (css::uno::RuntimeException, std::exception) override
    {
        if (type == cppu::UnoType<css::uno::XInterface>::get()) {
            css::uno::Reference< css::uno::XInterface > ref(
                static_cast< css::uno::XInterface * >(this));
            return css::uno::Any(&ref, type);
        } else if (type == cppu::UnoType<Interface1>::get()) {
            css::uno::Reference< Interface1 > ref(this);
            return css::uno::Any(&ref, type);
        } else {
            return css::uno::Any();
        }
    }

    virtual void SAL_CALL acquire() throw () override {
        Base::acquire();
    }

    virtual void SAL_CALL release() throw () override {
        Base::release();
    }
};

class Impl2: public Interface2a, public Interface3, private Base {
public:
    virtual css::uno::Any SAL_CALL queryInterface(css::uno::Type const & type)
        throw (css::uno::RuntimeException, std::exception) override
    {
        if (type == cppu::UnoType<css::uno::XInterface>::get()) {
            css::uno::Reference< css::uno::XInterface > ref(
                static_cast< css::uno::XInterface * >(
                    static_cast< Interface2a * >(this)));
            return css::uno::Any(&ref, type);
        } else if (type == cppu::UnoType<Interface2>::get()) {
            css::uno::Reference< Interface2 > ref(this);
            return css::uno::Any(&ref, type);
        } else if (type == cppu::UnoType<Interface2a>::get()) {
            css::uno::Reference< Interface2a > ref(this);
            return css::uno::Any(&ref, type);
        } else if (type == cppu::UnoType<Interface3>::get()) {
            css::uno::Reference< Interface3 > ref(this);
            return css::uno::Any(&ref, type);
        } else {
            return css::uno::Any();
        }
    }

    virtual void SAL_CALL acquire() throw () override {
        Base::acquire();
    }

    virtual void SAL_CALL release() throw () override {
        Base::release();
    }
};

class Impl2b: public Interface2b, private Base {
public:
    virtual css::uno::Any SAL_CALL queryInterface(css::uno::Type const & type)
        throw (css::uno::RuntimeException, std::exception) override
    {
        if (type == cppu::UnoType<css::uno::XInterface>::get()) {
            css::uno::Reference< css::uno::XInterface > ref(
                static_cast< css::uno::XInterface * >(
                    static_cast< Interface2a * >(this)));
            return css::uno::Any(&ref, type);
        } else if (type == cppu::UnoType<Interface2>::get()) {
            css::uno::Reference< Interface2 > ref(this);
            return css::uno::Any(&ref, type);
        } else if (type == cppu::UnoType<Interface2a>::get()) {
            css::uno::Reference< Interface2a > ref(this);
            return css::uno::Any(&ref, type);
        } else if (type == cppu::UnoType<Interface2b>::get()) {
            css::uno::Reference< Interface2b > ref(this);
            return css::uno::Any(&ref, type);
        } else {
            return css::uno::Any();
        }
    }

    virtual void SAL_CALL acquire() throw () override {
        Base::acquire();
    }

    virtual void SAL_CALL release() throw () override {
        Base::release();
    }
};

class Test: public CppUnit::TestFixture {
public:
    void testVoid();
    void testBoolean();
    void testByte();
    void testShort();
    void testUnsignedShort();
    void testLong();
    void testUnsignedLong();
    void testHyper();
    void testUnsignedHyper();
    void testFloat();
    void testDouble();
    void testChar();
    void testString();
    void testType();
    void testSequence();
    void testEnum();
    void testStruct();
    void testPoly();
    void testException();
    void testInterface();
    void testNull();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testVoid);
    CPPUNIT_TEST(testBoolean);
    CPPUNIT_TEST(testByte);
    CPPUNIT_TEST(testShort);
    CPPUNIT_TEST(testUnsignedShort);
    CPPUNIT_TEST(testLong);
    CPPUNIT_TEST(testUnsignedLong);
    CPPUNIT_TEST(testHyper);
    CPPUNIT_TEST(testUnsignedHyper);
    CPPUNIT_TEST(testFloat);
    CPPUNIT_TEST(testDouble);
    CPPUNIT_TEST(testChar);
    CPPUNIT_TEST(testString);
    CPPUNIT_TEST(testType);
    CPPUNIT_TEST(testSequence);
    CPPUNIT_TEST(testEnum);
    CPPUNIT_TEST(testStruct);
    CPPUNIT_TEST(testPoly);
    CPPUNIT_TEST(testException);
    CPPUNIT_TEST(testInterface);
    CPPUNIT_TEST(testNull);
    CPPUNIT_TEST_SUITE_END();
};

void Test::testVoid() {
    css::uno::Any a;
    CPPUNIT_ASSERT(a.getValueType() == cppu::UnoType<void>::get());
    {
        bool b = true;
        CPPUNIT_ASSERT_MESSAGE("bool", !(a >>= b) && b);
    }
    {
        sal_Bool b = true;
        CPPUNIT_ASSERT_MESSAGE("sal_Bool", !(a >>= b) && b);
    }
    {
        sal_Int8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int8", !(a >>= b) && b == 2);
    }
    {
        sal_uInt8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt8", !(a >>= b) && b == 2);
    }
    {
        sal_Int16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int16", !(a >>= b) && b == 2);
    }
    {
        sal_uInt16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt16", !(a >>= b) && b == 2);
    }
    {
        sal_Int32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int32", !(a >>= b) && b == 2);
    }
    {
        sal_uInt32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt32", !(a >>= b) && b == 2);
    }
    {
        sal_Int64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int64", !(a >>= b) && b == 2);
    }
    {
        sal_uInt64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt64", !(a >>= b) && b == 2);
    }
    {
        float b = 2;
        CPPUNIT_ASSERT_MESSAGE("float", !(a >>= b) && b == 2);
    }
    {
        double b = 2;
        CPPUNIT_ASSERT_MESSAGE("double", !(a >>= b) && b == 2);
    }
    {
        sal_Unicode b = '2';
        CPPUNIT_ASSERT_MESSAGE("sal_Unicode", !(a >>= b) && b == '2');
    }
    {
        rtl::OUString b("2");
        CPPUNIT_ASSERT_MESSAGE( "rtl::OUString", !(a >>= b) && b == "2" );
    }
    {
        css::uno::Type b(cppu::UnoType<OUString>::get());
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Type",
            !(a >>= b) && b == cppu::UnoType<OUString>::get());
    }
    {
        css::uno::Any b(rtl::OUString("2"));
        CPPUNIT_ASSERT_MESSAGE("css::uno::Any", (a >>= b) && b == a);
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Sequence<rtl::OUString>", !(a >>= b) && b.getLength() == 2);
    }
    {
        Enum1 b = Enum1_M2;
        CPPUNIT_ASSERT_MESSAGE("Enum1", !(a >>= b) && b == Enum1_M2);
    }
    {
        Struct1 b(2);
        CPPUNIT_ASSERT_MESSAGE("Struct1", !(a >>= b) && b.member == 2);
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        CPPUNIT_ASSERT_MESSAGE("Exception1", !(a >>= b) && b.member == 2);
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        CPPUNIT_ASSERT_MESSAGE("Interface1", !(a >>= b) && b == i);
    }
}

void Test::testBoolean() {
    css::uno::Any a(false);
    CPPUNIT_ASSERT(a.getValueType() == cppu::UnoType<bool>::get());
    {
        bool b = true;
        CPPUNIT_ASSERT_MESSAGE("bool", (a >>= b) && !b);
    }
    {
        sal_Bool b = true;
        CPPUNIT_ASSERT_MESSAGE("sal_Bool", (a >>= b) && !b);
    }
    {
        sal_Int8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int8", !(a >>= b) && b == 2);
    }
    {
        sal_uInt8 b = 2;
        if (boost::is_same< sal_uInt8, sal_Bool >::value) {
            CPPUNIT_ASSERT_MESSAGE("@sal_uInt8", (a >>= b) && b == 0);
        } else {
            CPPUNIT_ASSERT_MESSAGE("sal_uInt8", !(a >>= b) && b == 2);
        }
    }
    {
        sal_Int16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int16", !(a >>= b) && b == 2);
    }
    {
        sal_uInt16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt16", !(a >>= b) && b == 2);
    }
    {
        sal_Int32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int32", !(a >>= b) && b == 2);
    }
    {
        sal_uInt32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt32", !(a >>= b) && b == 2);
    }
    {
        sal_Int64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int64", !(a >>= b) && b == 2);
    }
    {
        sal_uInt64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt64", !(a >>= b) && b == 2);
    }
    {
        float b = 2;
        CPPUNIT_ASSERT_MESSAGE("float", !(a >>= b) && b == 2);
    }
    {
        double b = 2;
        CPPUNIT_ASSERT_MESSAGE("double", !(a >>= b) && b == 2);
    }
    {
        sal_Unicode b = '2';
        CPPUNIT_ASSERT_MESSAGE("sal_Unicode", !(a >>= b) && b == '2');
    }
    {
        rtl::OUString b("2");
        CPPUNIT_ASSERT_MESSAGE( "rtl::OUString", !(a >>= b) && b == "2" );
    }
    {
        css::uno::Type b(cppu::UnoType<OUString>::get());
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Type",
            !(a >>= b) && b == cppu::UnoType<OUString>::get());
    }
    {
        css::uno::Any b(rtl::OUString("2"));
        CPPUNIT_ASSERT_MESSAGE("css::uno::Any", (a >>= b) && b == a);
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Sequence<rtl::OUString>",
            !(a >>= b) && b.getLength() == 2);
    }
    {
        Enum1 b = Enum1_M2;
        CPPUNIT_ASSERT_MESSAGE("Enum1", !(a >>= b) && b == Enum1_M2);
    }
    {
        Struct1 b(2);
        CPPUNIT_ASSERT_MESSAGE("Struct1", !(a >>= b) && b.member == 2);
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        CPPUNIT_ASSERT_MESSAGE("Exception1", !(a >>= b) && b.member == 2);
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        CPPUNIT_ASSERT_MESSAGE("Interface1", !(a >>= b) && b == i);
    }
}

void Test::testByte() {
    css::uno::Any a(static_cast< sal_Int8 >(1));
    CPPUNIT_ASSERT(a.getValueType() == cppu::UnoType<sal_Int8>::get());
    {
        bool b = true;
        CPPUNIT_ASSERT_MESSAGE("bool", !(a >>= b) && b);
    }
    {
        sal_Bool b = true;
        CPPUNIT_ASSERT_MESSAGE("sal_Bool", !(a >>= b) && b);
    }
    {
        sal_Int8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int8", (a >>= b) && b == 1);
    }
    {
        sal_uInt8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt8", !(a >>= b) && b == 2);
    }
    {
        sal_Int16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int16", (a >>= b) && b == 1);
    }
    {
        sal_uInt16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt16", (a >>= b) && b == 1);
    }
    {
        sal_Int32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int32", (a >>= b) && b == 1);
    }
    {
        sal_uInt32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt32", (a >>= b) && b == 1);
    }
    {
        sal_Int64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int64", (a >>= b) && b == 1);
    }
    {
        sal_uInt64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt64", (a >>= b) && b == 1);
    }
    {
        float b = 2;
        CPPUNIT_ASSERT_MESSAGE("float", (a >>= b) && b == 1);
    }
    {
        double b = 2;
        CPPUNIT_ASSERT_MESSAGE("double", (a >>= b) && b == 1);
    }
    {
        sal_Unicode b = '2';
        if (boost::is_same< sal_Unicode, sal_uInt16 >::value) {
            CPPUNIT_ASSERT_MESSAGE("@sal_Unicode", (a >>= b) && b == 1);
        } else {
            CPPUNIT_ASSERT_MESSAGE("sal_Unicode", !(a >>= b) && b == '2');
        }
    }
    {
        rtl::OUString b("2");
        CPPUNIT_ASSERT_MESSAGE( "rtl::OUString", !(a >>= b) && b == "2" );
    }
    {
        css::uno::Type b(cppu::UnoType<OUString>::get());
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Type",
            !(a >>= b) && b == cppu::UnoType<OUString>::get());
    }
    {
        css::uno::Any b(rtl::OUString("2"));
        CPPUNIT_ASSERT_MESSAGE("css::uno::Any", (a >>= b) && b == a);
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Sequence<rtl::OUString>",
            !(a >>= b) && b.getLength() == 2);
    }
    {
        Enum1 b = Enum1_M2;
        CPPUNIT_ASSERT_MESSAGE("Enum1", !(a >>= b) && b == Enum1_M2);
    }
    {
        Struct1 b(2);
        CPPUNIT_ASSERT_MESSAGE("Struct1", !(a >>= b) && b.member == 2);
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        CPPUNIT_ASSERT_MESSAGE("Exception1", !(a >>= b) && b.member == 2);
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        CPPUNIT_ASSERT_MESSAGE("Interface1", !(a >>= b) && b == i);
    }
}

void Test::testShort() {
    css::uno::Any a(static_cast< sal_Int16 >(1));
    CPPUNIT_ASSERT(a.getValueType() == cppu::UnoType<sal_Int16>::get());
    {
        bool b = true;
        CPPUNIT_ASSERT_MESSAGE("bool", !(a >>= b) && b);
    }
    {
        sal_Bool b = true;
        CPPUNIT_ASSERT_MESSAGE("sal_Bool", !(a >>= b) && b);
    }
    {
        sal_Int8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int8", !(a >>= b) && b == 2);
    }
    {
        sal_uInt8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt8", !(a >>= b) && b == 2);
    }
    {
        sal_Int16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int16", (a >>= b) && b == 1);
    }
    {
        sal_uInt16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt16", (a >>= b) && b == 1);
    }
    {
        sal_Int32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int32", (a >>= b) && b == 1);
    }
    {
        sal_uInt32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt32", (a >>= b) && b == 1);
    }
    {
        sal_Int64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int64", (a >>= b) && b == 1);
    }
    {
        sal_uInt64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt64", (a >>= b) && b == 1);
    }
    {
        float b = 2;
        CPPUNIT_ASSERT_MESSAGE("float", (a >>= b) && b == 1);
    }
    {
        double b = 2;
        CPPUNIT_ASSERT_MESSAGE("double", (a >>= b) && b == 1);
    }
    {
        sal_Unicode b = '2';
        if (boost::is_same< sal_Unicode, sal_uInt16 >::value) {
            CPPUNIT_ASSERT_MESSAGE("@sal_Unicode", (a >>= b) && b == 1);
        } else {
            CPPUNIT_ASSERT_MESSAGE("sal_Unicode", !(a >>= b) && b == '2');
        }
    }
    {
        rtl::OUString b("2");
        CPPUNIT_ASSERT_MESSAGE( "rtl::OUString", !(a >>= b) && b == "2" );
    }
    {
        css::uno::Type b(cppu::UnoType<OUString>::get());
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Type",
            !(a >>= b) && b == cppu::UnoType<OUString>::get());
    }
    {
        css::uno::Any b(rtl::OUString("2"));
        CPPUNIT_ASSERT_MESSAGE("css::uno::Any", (a >>= b) && b == a);
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Sequence<rtl::OUString>",
            !(a >>= b) && b.getLength() == 2);
    }
    {
        Enum1 b = Enum1_M2;
        CPPUNIT_ASSERT_MESSAGE("Enum1", !(a >>= b) && b == Enum1_M2);
    }
    {
        Struct1 b(2);
        CPPUNIT_ASSERT_MESSAGE("Struct1", !(a >>= b) && b.member == 2);
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        CPPUNIT_ASSERT_MESSAGE("Exception1", !(a >>= b) && b.member == 2);
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        CPPUNIT_ASSERT_MESSAGE("Interface1", !(a >>= b) && b == i);
    }
}

void Test::testUnsignedShort() {
    sal_uInt16 n = 1;
    css::uno::Any a(&n, cppu::UnoType<cppu::UnoUnsignedShortType>::get());
    CPPUNIT_ASSERT(
        a.getValueType() == cppu::UnoType<cppu::UnoUnsignedShortType>::get());
    {
        bool b = true;
        CPPUNIT_ASSERT_MESSAGE("bool", !(a >>= b) && b);
    }
    {
        sal_Bool b = true;
        CPPUNIT_ASSERT_MESSAGE("sal_Bool", !(a >>= b) && b);
    }
    {
        sal_Int8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int8", !(a >>= b) && b == 2);
    }
    {
        sal_uInt8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt8", !(a >>= b) && b == 2);
    }
    {
        sal_Int16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int16", (a >>= b) && b == 1);
    }
    {
        sal_uInt16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt16", (a >>= b) && b == 1);
    }
    {
        sal_Int32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int32", (a >>= b) && b == 1);
    }
    {
        sal_uInt32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt32", (a >>= b) && b == 1);
    }
    {
        sal_Int64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int64", (a >>= b) && b == 1);
    }
    {
        sal_uInt64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt64", (a >>= b) && b == 1);
    }
    {
        float b = 2;
        CPPUNIT_ASSERT_MESSAGE("float", (a >>= b) && b == 1);
    }
    {
        double b = 2;
        CPPUNIT_ASSERT_MESSAGE("double", (a >>= b) && b == 1);
    }
    {
        sal_Unicode b = '2';
        if (boost::is_same< sal_Unicode, sal_uInt16 >::value) {
            CPPUNIT_ASSERT_MESSAGE("@sal_Unicode", (a >>= b) && b == 1);
        } else {
            CPPUNIT_ASSERT_MESSAGE("sal_Unicode", !(a >>= b) && b == '2');
        }
    }
    {
        rtl::OUString b("2");
        CPPUNIT_ASSERT_MESSAGE( "rtl::OUString", !(a >>= b) && b == "2" );
    }
    {
        css::uno::Type b(cppu::UnoType<OUString>::get());
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Type",
            !(a >>= b) && b == cppu::UnoType<OUString>::get());
    }
    {
        css::uno::Any b(rtl::OUString("2"));
        CPPUNIT_ASSERT_MESSAGE("css::uno::Any", (a >>= b) && b == a);
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Sequence<rtl::OUString>",
            !(a >>= b) && b.getLength() == 2);
    }
    {
        Enum1 b = Enum1_M2;
        CPPUNIT_ASSERT_MESSAGE("Enum1", !(a >>= b) && b == Enum1_M2);
    }
    {
        Struct1 b(2);
        CPPUNIT_ASSERT_MESSAGE("Struct1", !(a >>= b) && b.member == 2);
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        CPPUNIT_ASSERT_MESSAGE("Exception1", !(a >>= b) && b.member == 2);
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        CPPUNIT_ASSERT_MESSAGE("Interface1", !(a >>= b) && b == i);
    }
}

void Test::testLong() {
    css::uno::Any a(static_cast< sal_Int32 >(1));
    CPPUNIT_ASSERT(a.getValueType() == cppu::UnoType<sal_Int32>::get());
    {
        bool b = true;
        CPPUNIT_ASSERT_MESSAGE("bool", !(a >>= b) && b);
    }
    {
        sal_Bool b = true;
        CPPUNIT_ASSERT_MESSAGE("sal_Bool", !(a >>= b) && b);
    }
    {
        sal_Int8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int8", !(a >>= b) && b == 2);
    }
    {
        sal_uInt8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt8", !(a >>= b) && b == 2);
    }
    {
        sal_Int16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int16", !(a >>= b) && b == 2);
    }
    {
        sal_uInt16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt16", !(a >>= b) && b == 2);
    }
    {
        sal_Int32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int32", (a >>= b) && b == 1);
    }
    {
        sal_uInt32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt32", (a >>= b) && b == 1);
    }
    {
        sal_Int64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int64", (a >>= b) && b == 1);
    }
    {
        sal_uInt64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt64", (a >>= b) && b == 1);
    }
    {
        float b = 2;
        CPPUNIT_ASSERT_MESSAGE("float", !(a >>= b) && b == 2);
    }
    {
        double b = 2;
        CPPUNIT_ASSERT_MESSAGE("double", (a >>= b) && b == 1);
    }
    {
        sal_Unicode b = '2';
        CPPUNIT_ASSERT_MESSAGE("sal_Unicode", !(a >>= b) && b == '2');
    }
    {
        rtl::OUString b("2");
        CPPUNIT_ASSERT_MESSAGE( "rtl::OUString", !(a >>= b) && b == "2" );
    }
    {
        css::uno::Type b(cppu::UnoType<OUString>::get());
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Type",
            !(a >>= b) && b == cppu::UnoType<OUString>::get());
    }
    {
        css::uno::Any b(rtl::OUString("2"));
        CPPUNIT_ASSERT_MESSAGE("css::uno::Any", (a >>= b) && b == a);
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Sequence<rtl::OUString>",
            !(a >>= b) && b.getLength() == 2);
    }
    {
        Enum1 b = Enum1_M2;
        CPPUNIT_ASSERT_MESSAGE("Enum1", !(a >>= b) && b == Enum1_M2);
    }
    {
        Struct1 b(2);
        CPPUNIT_ASSERT_MESSAGE("Struct1", !(a >>= b) && b.member == 2);
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        CPPUNIT_ASSERT_MESSAGE("Exception1", !(a >>= b) && b.member == 2);
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        CPPUNIT_ASSERT_MESSAGE("Interface1", !(a >>= b) && b == i);
    }
}

void Test::testUnsignedLong() {
    css::uno::Any a(static_cast< sal_uInt32 >(1));
    CPPUNIT_ASSERT(a.getValueType() == cppu::UnoType<sal_uInt32>::get());
    {
        bool b = true;
        CPPUNIT_ASSERT_MESSAGE("bool", !(a >>= b) && b);
    }
    {
        sal_Bool b = true;
        CPPUNIT_ASSERT_MESSAGE("sal_Bool", !(a >>= b) && b);
    }
    {
        sal_Int8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int8", !(a >>= b) && b == 2);
    }
    {
        sal_uInt8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt8", !(a >>= b) && b == 2);
    }
    {
        sal_Int16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int16", !(a >>= b) && b == 2);
    }
    {
        sal_uInt16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt16", !(a >>= b) && b == 2);
    }
    {
        sal_Int32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int32", (a >>= b) && b == 1);
    }
    {
        sal_uInt32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt32", (a >>= b) && b == 1);
    }
    {
        sal_Int64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int64", (a >>= b) && b == 1);
    }
    {
        sal_uInt64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt64", (a >>= b) && b == 1);
    }
    {
        float b = 2;
        CPPUNIT_ASSERT_MESSAGE("float", !(a >>= b) && b == 2);
    }
    {
        double b = 2;
        CPPUNIT_ASSERT_MESSAGE("double", (a >>= b) && b == 1);
    }
    {
        sal_Unicode b = '2';
        CPPUNIT_ASSERT_MESSAGE("sal_Unicode", !(a >>= b) && b == '2');
    }
    {
        rtl::OUString b("2");
        CPPUNIT_ASSERT_MESSAGE( "rtl::OUString", !(a >>= b) && b == "2" );
    }
    {
        css::uno::Type b(cppu::UnoType<OUString>::get());
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Type",
            !(a >>= b) && b == cppu::UnoType<OUString>::get());
    }
    {
        css::uno::Any b(rtl::OUString("2"));
        CPPUNIT_ASSERT_MESSAGE("css::uno::Any", (a >>= b) && b == a);
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Sequence<rtl::OUString>",
            !(a >>= b) && b.getLength() == 2);
    }
    {
        Enum1 b = Enum1_M2;
        CPPUNIT_ASSERT_MESSAGE("Enum1", !(a >>= b) && b == Enum1_M2);
    }
    {
        Struct1 b(2);
        CPPUNIT_ASSERT_MESSAGE("Struct1", !(a >>= b) && b.member == 2);
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        CPPUNIT_ASSERT_MESSAGE("Exception1", !(a >>= b) && b.member == 2);
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        CPPUNIT_ASSERT_MESSAGE("Interface1", !(a >>= b) && b == i);
    }
}

void Test::testHyper() {
    css::uno::Any a(static_cast< sal_Int64 >(1));
    CPPUNIT_ASSERT(a.getValueType() == cppu::UnoType<sal_Int64>::get());
    {
        bool b = true;
        CPPUNIT_ASSERT_MESSAGE("bool", !(a >>= b) && b);
    }
    {
        sal_Bool b = true;
        CPPUNIT_ASSERT_MESSAGE("sal_Bool", !(a >>= b) && b);
    }
    {
        sal_Int8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int8", !(a >>= b) && b == 2);
    }
    {
        sal_uInt8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt8", !(a >>= b) && b == 2);
    }
    {
        sal_Int16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int16", !(a >>= b) && b == 2);
    }
    {
        sal_uInt16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt16", !(a >>= b) && b == 2);
    }
    {
        sal_Int32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int32", !(a >>= b) && b == 2);
    }
    {
        sal_uInt32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt32", !(a >>= b) && b == 2);
    }
    {
        sal_Int64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int64", (a >>= b) && b == 1);
    }
    {
        sal_uInt64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt64", (a >>= b) && b == 1);
    }
    {
        float b = 2;
        CPPUNIT_ASSERT_MESSAGE("float", !(a >>= b) && b == 2);
    }
    {
        double b = 2;
        CPPUNIT_ASSERT_MESSAGE("double", !(a >>= b) && b == 2);
    }
    {
        sal_Unicode b = '2';
        CPPUNIT_ASSERT_MESSAGE("sal_Unicode", !(a >>= b) && b == '2');
    }
    {
        rtl::OUString b("2");
        CPPUNIT_ASSERT_MESSAGE( "rtl::OUString", !(a >>= b) && b == "2" );
    }
    {
        css::uno::Type b(cppu::UnoType<OUString>::get());
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Type",
            !(a >>= b) && b == cppu::UnoType<OUString>::get());
    }
    {
        css::uno::Any b(rtl::OUString("2"));
        CPPUNIT_ASSERT_MESSAGE("css::uno::Any", (a >>= b) && b == a);
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Sequence<rtl::OUString>",
            !(a >>= b) && b.getLength() == 2);
    }
    {
        Enum1 b = Enum1_M2;
        CPPUNIT_ASSERT_MESSAGE("Enum1", !(a >>= b) && b == Enum1_M2);
    }
    {
        Struct1 b(2);
        CPPUNIT_ASSERT_MESSAGE("Struct1", !(a >>= b) && b.member == 2);
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        CPPUNIT_ASSERT_MESSAGE("Exception1", !(a >>= b) && b.member == 2);
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        CPPUNIT_ASSERT_MESSAGE("Interface1", !(a >>= b) && b == i);
    }
}

void Test::testUnsignedHyper() {
    css::uno::Any a(static_cast< sal_uInt64 >(1));
    CPPUNIT_ASSERT(a.getValueType() == cppu::UnoType<sal_uInt64>::get());
    {
        bool b = true;
        CPPUNIT_ASSERT_MESSAGE("bool", !(a >>= b) && b);
    }
    {
        sal_Bool b = true;
        CPPUNIT_ASSERT_MESSAGE("sal_Bool", !(a >>= b) && b);
    }
    {
        sal_Int8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int8", !(a >>= b) && b == 2);
    }
    {
        sal_uInt8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt8", !(a >>= b) && b == 2);
    }
    {
        sal_Int16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int16", !(a >>= b) && b == 2);
    }
    {
        sal_uInt16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt16", !(a >>= b) && b == 2);
    }
    {
        sal_Int32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int32", !(a >>= b) && b == 2);
    }
    {
        sal_uInt32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt32", !(a >>= b) && b == 2);
    }
    {
        sal_Int64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int64", (a >>= b) && b == 1);
    }
    {
        sal_uInt64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt64", (a >>= b) && b == 1);
    }
    {
        float b = 2;
        CPPUNIT_ASSERT_MESSAGE("float", !(a >>= b) && b == 2);
    }
    {
        double b = 2;
        CPPUNIT_ASSERT_MESSAGE("double", !(a >>= b) && b == 2);
    }
    {
        sal_Unicode b = '2';
        CPPUNIT_ASSERT_MESSAGE("sal_Unicode", !(a >>= b) && b == '2');
    }
    {
        rtl::OUString b("2");
        CPPUNIT_ASSERT_MESSAGE( "rtl::OUString", !(a >>= b) && b == "2" );
    }
    {
        css::uno::Type b(cppu::UnoType<OUString>::get());
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Type",
            !(a >>= b) && b == cppu::UnoType<OUString>::get());
    }
    {
        css::uno::Any b(rtl::OUString("2"));
        CPPUNIT_ASSERT_MESSAGE("css::uno::Any", (a >>= b) && b == a);
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Sequence<rtl::OUString>",
            !(a >>= b) && b.getLength() == 2);
    }
    {
        Enum1 b = Enum1_M2;
        CPPUNIT_ASSERT_MESSAGE("Enum1", !(a >>= b) && b == Enum1_M2);
    }
    {
        Struct1 b(2);
        CPPUNIT_ASSERT_MESSAGE("Struct1", !(a >>= b) && b.member == 2);
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        CPPUNIT_ASSERT_MESSAGE("Exception1", !(a >>= b) && b.member == 2);
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        CPPUNIT_ASSERT_MESSAGE("Interface1", !(a >>= b) && b == i);
    }
}

void Test::testFloat() {
    css::uno::Any a(1.f);
    CPPUNIT_ASSERT(a.getValueType() == cppu::UnoType<float>::get());
    {
        bool b = true;
        CPPUNIT_ASSERT_MESSAGE("bool", !(a >>= b) && b);
    }
    {
        sal_Bool b = true;
        CPPUNIT_ASSERT_MESSAGE("sal_Bool", !(a >>= b) && b);
    }
    {
        sal_Int8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int8", !(a >>= b) && b == 2);
    }
    {
        sal_uInt8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt8", !(a >>= b) && b == 2);
    }
    {
        sal_Int16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int16", !(a >>= b) && b == 2);
    }
    {
        sal_uInt16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt16", !(a >>= b) && b == 2);
    }
    {
        sal_Int32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int32", !(a >>= b) && b == 2);
    }
    {
        sal_uInt32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt32", !(a >>= b) && b == 2);
    }
    {
        sal_Int64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int64", !(a >>= b) && b == 2);
    }
    {
        sal_uInt64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt64", !(a >>= b) && b == 2);
    }
    {
        float b = 2;
        CPPUNIT_ASSERT_MESSAGE("float", (a >>= b) && b == 1);
    }
    {
        double b = 2;
        CPPUNIT_ASSERT_MESSAGE("double", (a >>= b) && b == 1);
    }
    {
        sal_Unicode b = '2';
        CPPUNIT_ASSERT_MESSAGE("sal_Unicode", !(a >>= b) && b == '2');
    }
    {
        rtl::OUString b("2");
        CPPUNIT_ASSERT_MESSAGE( "rtl::OUString", !(a >>= b) && b == "2" );
    }
    {
        css::uno::Type b(cppu::UnoType<OUString>::get());
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Type",
            !(a >>= b) && b == cppu::UnoType<OUString>::get());
    }
    {
        css::uno::Any b(rtl::OUString("2"));
        CPPUNIT_ASSERT_MESSAGE("css::uno::Any", (a >>= b) && b == a);
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Sequence<rtl::OUString>",
            !(a >>= b) && b.getLength() == 2);
    }
    {
        Enum1 b = Enum1_M2;
        CPPUNIT_ASSERT_MESSAGE("Enum1", !(a >>= b) && b == Enum1_M2);
    }
    {
        Struct1 b(2);
        CPPUNIT_ASSERT_MESSAGE("Struct1", !(a >>= b) && b.member == 2);
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        CPPUNIT_ASSERT_MESSAGE("Exception1", !(a >>= b) && b.member == 2);
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        CPPUNIT_ASSERT_MESSAGE("Interface1", !(a >>= b) && b == i);
    }
}

void Test::testDouble() {
    css::uno::Any a(1.);
    CPPUNIT_ASSERT(a.getValueType() == cppu::UnoType<double>::get());
    {
        bool b = true;
        CPPUNIT_ASSERT_MESSAGE("bool", !(a >>= b) && b);
    }
    {
        sal_Bool b = true;
        CPPUNIT_ASSERT_MESSAGE("sal_Bool", !(a >>= b) && b);
    }
    {
        sal_Int8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int8", !(a >>= b) && b == 2);
    }
    {
        sal_uInt8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt8", !(a >>= b) && b == 2);
    }
    {
        sal_Int16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int16", !(a >>= b) && b == 2);
    }
    {
        sal_uInt16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt16", !(a >>= b) && b == 2);
    }
    {
        sal_Int32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int32", !(a >>= b) && b == 2);
    }
    {
        sal_uInt32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt32", !(a >>= b) && b == 2);
    }
    {
        sal_Int64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int64", !(a >>= b) && b == 2);
    }
    {
        sal_uInt64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt64", !(a >>= b) && b == 2);
    }
    {
        float b = 2;
        CPPUNIT_ASSERT_MESSAGE("float", !(a >>= b) && b == 2);
    }
    {
        double b = 2;
        CPPUNIT_ASSERT_MESSAGE("double", (a >>= b) && b == 1);
    }
    {
        sal_Unicode b = '2';
        CPPUNIT_ASSERT_MESSAGE("sal_Unicode", !(a >>= b) && b == '2');
    }
    {
        rtl::OUString b("2");
        CPPUNIT_ASSERT_MESSAGE( "rtl::OUString", !(a >>= b) && b == "2" );
    }
    {
        css::uno::Type b(cppu::UnoType<OUString>::get());
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Type",
            !(a >>= b) && b == cppu::UnoType<OUString>::get());
    }
    {
        css::uno::Any b(rtl::OUString("2"));
        CPPUNIT_ASSERT_MESSAGE("css::uno::Any", (a >>= b) && b == a);
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Sequence<rtl::OUString>",
            !(a >>= b) && b.getLength() == 2);
    }
    {
        Enum1 b = Enum1_M2;
        CPPUNIT_ASSERT_MESSAGE("Enum1", !(a >>= b) && b == Enum1_M2);
    }
    {
        Struct1 b(2);
        CPPUNIT_ASSERT_MESSAGE("Struct1", !(a >>= b) && b.member == 2);
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        CPPUNIT_ASSERT_MESSAGE("Exception1", !(a >>= b) && b.member == 2);
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        CPPUNIT_ASSERT_MESSAGE("Interface1", !(a >>= b) && b == i);
    }
}

void Test::testChar() {
    sal_Unicode c = '1';
    css::uno::Any a(&c, cppu::UnoType<cppu::UnoCharType>::get());
    CPPUNIT_ASSERT(a.getValueType() == cppu::UnoType<cppu::UnoCharType>::get());
    {
        bool b = true;
        CPPUNIT_ASSERT_MESSAGE("bool", !(a >>= b) && b);
    }
    {
        sal_Bool b = true;
        CPPUNIT_ASSERT_MESSAGE("sal_Bool", !(a >>= b) && b);
    }
    {
        sal_Int8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int8", !(a >>= b) && b == 2);
    }
    {
        sal_uInt8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt8", !(a >>= b) && b == 2);
    }
    {
        sal_Int16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int16", !(a >>= b) && b == 2);
    }
    {
        sal_uInt16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt16", !(a >>= b) && b == 2);
    }
    {
        sal_Int32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int32", !(a >>= b) && b == 2);
    }
    {
        sal_uInt32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt32", !(a >>= b) && b == 2);
    }
    {
        sal_Int64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int64", !(a >>= b) && b == 2);
    }
    {
        sal_uInt64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt64", !(a >>= b) && b == 2);
    }
    {
        float b = 2;
        CPPUNIT_ASSERT_MESSAGE("float", !(a >>= b) && b == 2);
    }
    {
        double b = 2;
        CPPUNIT_ASSERT_MESSAGE("double", !(a >>= b) && b == 2);
    }
    {
        sal_Unicode b = '2';
        if (boost::is_same< sal_Unicode, sal_uInt16 >::value) {
            CPPUNIT_ASSERT_MESSAGE("@sal_Unicode", !(a >>= b) && b == '2');
        } else {
            CPPUNIT_ASSERT_MESSAGE("sal_Unicode", (a >>= b) && b == '1');
        }
    }
    {
        rtl::OUString b("2");
        CPPUNIT_ASSERT_MESSAGE( "rtl::OUString", !(a >>= b) && b == "2" );
    }
    {
        css::uno::Type b(cppu::UnoType<OUString>::get());
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Type",
            !(a >>= b) && b == cppu::UnoType<OUString>::get());
    }
    {
        css::uno::Any b(rtl::OUString("2"));
        CPPUNIT_ASSERT_MESSAGE("css::uno::Any", (a >>= b) && b == a);
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Sequence<rtl::OUString>",
            !(a >>= b) && b.getLength() == 2);
    }
    {
        Enum1 b = Enum1_M2;
        CPPUNIT_ASSERT_MESSAGE("Enum1", !(a >>= b) && b == Enum1_M2);
    }
    {
        Struct1 b(2);
        CPPUNIT_ASSERT_MESSAGE("Struct1", !(a >>= b) && b.member == 2);
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        CPPUNIT_ASSERT_MESSAGE("Exception1", !(a >>= b) && b.member == 2);
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        CPPUNIT_ASSERT_MESSAGE("Interface1", !(a >>= b) && b == i);
    }
}

void Test::testString() {
    css::uno::Any a(rtl::OUString("1"));
    CPPUNIT_ASSERT(a.getValueType() == cppu::UnoType<OUString>::get());
    {
        bool b = true;
        CPPUNIT_ASSERT_MESSAGE("bool", !(a >>= b) && b);
    }
    {
        sal_Bool b = true;
        CPPUNIT_ASSERT_MESSAGE("sal_Bool", !(a >>= b) && b);
    }
    {
        sal_Int8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int8", !(a >>= b) && b == 2);
    }
    {
        sal_uInt8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt8", !(a >>= b) && b == 2);
    }
    {
        sal_Int16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int16", !(a >>= b) && b == 2);
    }
    {
        sal_uInt16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt16", !(a >>= b) && b == 2);
    }
    {
        sal_Int32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int32", !(a >>= b) && b == 2);
    }
    {
        sal_uInt32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt32", !(a >>= b) && b == 2);
    }
    {
        sal_Int64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int64", !(a >>= b) && b == 2);
    }
    {
        sal_uInt64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt64", !(a >>= b) && b == 2);
    }
    {
        float b = 2;
        CPPUNIT_ASSERT_MESSAGE("float", !(a >>= b) && b == 2);
    }
    {
        double b = 2;
        CPPUNIT_ASSERT_MESSAGE("double", !(a >>= b) && b == 2);
    }
    {
        sal_Unicode b = '2';
        CPPUNIT_ASSERT_MESSAGE("sal_Unicode", !(a >>= b) && b == '2');
    }
    {
        rtl::OUString b("2");
        CPPUNIT_ASSERT_MESSAGE( "rtl::OUString", (a >>= b) && b == "1" );
    }
    {
        css::uno::Type b(cppu::UnoType<OUString>::get());
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Type",
            !(a >>= b) && b == cppu::UnoType<OUString>::get());
    }
    {
        css::uno::Any b(rtl::OUString("2"));
        CPPUNIT_ASSERT_MESSAGE("css::uno::Any", (a >>= b) && b == a);
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Sequence<rtl::OUString>",
            !(a >>= b) && b.getLength() == 2);
    }
    {
        Enum1 b = Enum1_M2;
        CPPUNIT_ASSERT_MESSAGE("Enum1", !(a >>= b) && b == Enum1_M2);
    }
    {
        Struct1 b(2);
        CPPUNIT_ASSERT_MESSAGE("Struct1", !(a >>= b) && b.member == 2);
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        CPPUNIT_ASSERT_MESSAGE("Exception1", !(a >>= b) && b.member == 2);
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        CPPUNIT_ASSERT_MESSAGE("Interface1", !(a >>= b) && b == i);
    }
}

void Test::testType() {
    css::uno::Any a(cppu::UnoType<sal_Int32>::get());
    CPPUNIT_ASSERT(a.getValueType() == cppu::UnoType<css::uno::Type>::get());
    {
        bool b = true;
        CPPUNIT_ASSERT_MESSAGE("bool", !(a >>= b) && b);
    }
    {
        sal_Bool b = true;
        CPPUNIT_ASSERT_MESSAGE("sal_Bool", !(a >>= b) && b);
    }
    {
        sal_Int8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int8", !(a >>= b) && b == 2);
    }
    {
        sal_uInt8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt8", !(a >>= b) && b == 2);
    }
    {
        sal_Int16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int16", !(a >>= b) && b == 2);
    }
    {
        sal_uInt16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt16", !(a >>= b) && b == 2);
    }
    {
        sal_Int32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int32", !(a >>= b) && b == 2);
    }
    {
        sal_uInt32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt32", !(a >>= b) && b == 2);
    }
    {
        sal_Int64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int64", !(a >>= b) && b == 2);
    }
    {
        sal_uInt64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt64", !(a >>= b) && b == 2);
    }
    {
        float b = 2;
        CPPUNIT_ASSERT_MESSAGE("float", !(a >>= b) && b == 2);
    }
    {
        double b = 2;
        CPPUNIT_ASSERT_MESSAGE("double", !(a >>= b) && b == 2);
    }
    {
        sal_Unicode b = '2';
        CPPUNIT_ASSERT_MESSAGE("sal_Unicode", !(a >>= b) && b == '2');
    }
    {
        rtl::OUString b("2");
        CPPUNIT_ASSERT_MESSAGE( "rtl::OUString", !(a >>= b) && b == "2" );
    }
    {
        css::uno::Type b(cppu::UnoType<OUString>::get());
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Type", (a >>= b) && b == cppu::UnoType<sal_Int32>::get());
    }
    {
        css::uno::Any b(rtl::OUString("2"));
        CPPUNIT_ASSERT_MESSAGE("css::uno::Any", (a >>= b) && b == a);
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Sequence<rtl::OUString>",
            !(a >>= b) && b.getLength() == 2);
    }
    {
        Enum1 b = Enum1_M2;
        CPPUNIT_ASSERT_MESSAGE("Enum1", !(a >>= b) && b == Enum1_M2);
    }
    {
        Struct1 b(2);
        CPPUNIT_ASSERT_MESSAGE("Struct1", !(a >>= b) && b.member == 2);
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        CPPUNIT_ASSERT_MESSAGE("Exception1", !(a >>= b) && b.member == 2);
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        CPPUNIT_ASSERT_MESSAGE("Interface1", !(a >>= b) && b == i);
    }
}

void Test::testSequence() {
    sal_Int32 n = 1;
    css::uno::Any a(css::uno::Sequence< sal_Int32 >(&n, 1));
    CPPUNIT_ASSERT(
        a.getValueType()
        == cppu::UnoType<css::uno::Sequence<sal_Int32>>::get());
    {
        bool b = true;
        CPPUNIT_ASSERT_MESSAGE("bool", !(a >>= b) && b);
    }
    {
        sal_Bool b = true;
        CPPUNIT_ASSERT_MESSAGE("sal_Bool", !(a >>= b) && b);
    }
    {
        sal_Int8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int8", !(a >>= b) && b == 2);
    }
    {
        sal_uInt8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt8", !(a >>= b) && b == 2);
    }
    {
        sal_Int16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int16", !(a >>= b) && b == 2);
    }
    {
        sal_uInt16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt16", !(a >>= b) && b == 2);
    }
    {
        sal_Int32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int32", !(a >>= b) && b == 2);
    }
    {
        sal_uInt32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt32", !(a >>= b) && b == 2);
    }
    {
        sal_Int64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int64", !(a >>= b) && b == 2);
    }
    {
        sal_uInt64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt64", !(a >>= b) && b == 2);
    }
    {
        float b = 2;
        CPPUNIT_ASSERT_MESSAGE("float", !(a >>= b) && b == 2);
    }
    {
        double b = 2;
        CPPUNIT_ASSERT_MESSAGE("double", !(a >>= b) && b == 2);
    }
    {
        sal_Unicode b = '2';
        CPPUNIT_ASSERT_MESSAGE("sal_Unicode", !(a >>= b) && b == '2');
    }
    {
        rtl::OUString b("2");
        CPPUNIT_ASSERT_MESSAGE( "rtl::OUString", !(a >>= b) && b == "2" );
    }
    {
        css::uno::Type b(cppu::UnoType<OUString>::get());
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Type",
            !(a >>= b) && b == cppu::UnoType<OUString>::get());
    }
    {
        css::uno::Any b(rtl::OUString("2"));
        CPPUNIT_ASSERT_MESSAGE("css::uno::Any", (a >>= b) && b == a);
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Sequence<rtl::OUString>",
            !(a >>= b) && b.getLength() == 2);
    }
    {
        css::uno::Sequence< sal_Int32 > b(2);
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Sequence<sal_Int32>",
            (a >>= b) && b.getLength() == 1 && b[0] == 1);
    }
    {
        Enum1 b = Enum1_M2;
        CPPUNIT_ASSERT_MESSAGE("Enum1", !(a >>= b) && b == Enum1_M2);
    }
    {
        Struct1 b(2);
        CPPUNIT_ASSERT_MESSAGE("Struct1", !(a >>= b) && b.member == 2);
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        CPPUNIT_ASSERT_MESSAGE("Exception1", !(a >>= b) && b.member == 2);
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        CPPUNIT_ASSERT_MESSAGE("Interface1", !(a >>= b) && b == i);
    }
}

void Test::testEnum() {
    css::uno::Any a(Enum2_M1);
    CPPUNIT_ASSERT(a.getValueType() == cppu::UnoType<Enum2>::get());
    {
        bool b = true;
        CPPUNIT_ASSERT_MESSAGE("bool", !(a >>= b) && b);
    }
    {
        sal_Bool b = true;
        CPPUNIT_ASSERT_MESSAGE("sal_Bool", !(a >>= b) && b);
    }
    {
        sal_Int8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int8", !(a >>= b) && b == 2);
    }
    {
        sal_uInt8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt8", !(a >>= b) && b == 2);
    }
    {
        sal_Int16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int16", !(a >>= b) && b == 2);
    }
    {
        sal_uInt16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt16", !(a >>= b) && b == 2);
    }
    {
        sal_Int32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int32", !(a >>= b) && b == 2);
    }
    {
        sal_uInt32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt32", !(a >>= b) && b == 2);
    }
    {
        sal_Int64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int64", !(a >>= b) && b == 2);
    }
    {
        sal_uInt64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt64", !(a >>= b) && b == 2);
    }
    {
        float b = 2;
        CPPUNIT_ASSERT_MESSAGE("float", !(a >>= b) && b == 2);
    }
    {
        double b = 2;
        CPPUNIT_ASSERT_MESSAGE("double", !(a >>= b) && b == 2);
    }
    {
        sal_Unicode b = '2';
        CPPUNIT_ASSERT_MESSAGE("sal_Unicode", !(a >>= b) && b == '2');
    }
    {
        rtl::OUString b("2");
        CPPUNIT_ASSERT_MESSAGE( "rtl::OUString", !(a >>= b) && b == "2" );
    }
    {
        css::uno::Type b(cppu::UnoType<OUString>::get());
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Type",
            !(a >>= b) && b == cppu::UnoType<OUString>::get());
    }
    {
        css::uno::Any b(rtl::OUString("2"));
        CPPUNIT_ASSERT_MESSAGE("css::uno::Any", (a >>= b) && b == a);
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Sequence<rtl::OUString>",
            !(a >>= b) && b.getLength() == 2);
    }
    {
        Enum1 b = Enum1_M2;
        CPPUNIT_ASSERT_MESSAGE("Enum1", !(a >>= b) && b == Enum1_M2);
    }
    {
        Enum2 b = Enum2_M2;
        CPPUNIT_ASSERT_MESSAGE("Enum2", (a >>= b) && b == Enum2_M1);
    }
    {
        Struct1 b(2);
        CPPUNIT_ASSERT_MESSAGE("Struct1", !(a >>= b) && b.member == 2);
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        CPPUNIT_ASSERT_MESSAGE("Exception1", !(a >>= b) && b.member == 2);
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        CPPUNIT_ASSERT_MESSAGE("Interface1", !(a >>= b) && b == i);
    }
}

void Test::testStruct() {
    css::uno::Any a(Struct2a(1, 3));
    CPPUNIT_ASSERT(a.getValueType() == cppu::UnoType<Struct2a>::get());
    {
        bool b = true;
        CPPUNIT_ASSERT_MESSAGE("bool", !(a >>= b) && b);
    }
    {
        sal_Bool b = true;
        CPPUNIT_ASSERT_MESSAGE("sal_Bool", !(a >>= b) && b);
    }
    {
        sal_Int8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int8", !(a >>= b) && b == 2);
    }
    {
        sal_uInt8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt8", !(a >>= b) && b == 2);
    }
    {
        sal_Int16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int16", !(a >>= b) && b == 2);
    }
    {
        sal_uInt16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt16", !(a >>= b) && b == 2);
    }
    {
        sal_Int32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int32", !(a >>= b) && b == 2);
    }
    {
        sal_uInt32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt32", !(a >>= b) && b == 2);
    }
    {
        sal_Int64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int64", !(a >>= b) && b == 2);
    }
    {
        sal_uInt64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt64", !(a >>= b) && b == 2);
    }
    {
        float b = 2;
        CPPUNIT_ASSERT_MESSAGE("float", !(a >>= b) && b == 2);
    }
    {
        double b = 2;
        CPPUNIT_ASSERT_MESSAGE("double", !(a >>= b) && b == 2);
    }
    {
        sal_Unicode b = '2';
        CPPUNIT_ASSERT_MESSAGE("sal_Unicode", !(a >>= b) && b == '2');
    }
    {
        rtl::OUString b("2");
        CPPUNIT_ASSERT_MESSAGE( "rtl::OUString", !(a >>= b) && b == "2" );
    }
    {
        css::uno::Type b(cppu::UnoType<OUString>::get());
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Type",
            !(a >>= b) && b == cppu::UnoType<OUString>::get());
    }
    {
        css::uno::Any b(rtl::OUString("2"));
        CPPUNIT_ASSERT_MESSAGE("css::uno::Any", (a >>= b) && b == a);
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Sequence<rtl::OUString>",
            !(a >>= b) && b.getLength() == 2);
    }
    {
        Enum1 b = Enum1_M2;
        CPPUNIT_ASSERT_MESSAGE("Enum1", !(a >>= b) && b == Enum1_M2);
    }
    {
        Struct1 b(2);
        CPPUNIT_ASSERT_MESSAGE("Struct1", !(a >>= b) && b.member == 2);
    }
    {
        Struct2 b(2);
        CPPUNIT_ASSERT_MESSAGE("Struct2", (a >>= b) && b.member == 1);
    }
    {
        Struct2a b(2, 2);
        CPPUNIT_ASSERT_MESSAGE(
            "Struct2a", (a >>= b) && b.member == 1 && b.member2 == 3);
    }
    {
        Struct2b b(2, 2, 2);
        CPPUNIT_ASSERT_MESSAGE("Struct2b", !(a >>= b) && b.member == 2);
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        CPPUNIT_ASSERT_MESSAGE("Exception1", !(a >>= b) && b.member == 2);
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        CPPUNIT_ASSERT_MESSAGE("Interface1", !(a >>= b) && b == i);
    }
}

void Test::testPoly() {
    css::uno::Any a;
    a <<= Poly< css::uno::Sequence< ::sal_Unicode > >();
    CPPUNIT_ASSERT_MESSAGE( "type name", a.getValueType().getTypeName() == "Poly<[]char>" );
    CPPUNIT_ASSERT_MESSAGE(
        "constructor",
        a == css::uno::Any(Poly< css::uno::Sequence< ::sal_Unicode > >()));
}

void Test::testException() {
    css::uno::Any a(
        Exception2a(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 1,
            3));
    CPPUNIT_ASSERT(a.getValueType() == cppu::UnoType<Exception2a>::get());
    {
        bool b = true;
        CPPUNIT_ASSERT_MESSAGE("bool", !(a >>= b) && b);
    }
    {
        sal_Bool b = true;
        CPPUNIT_ASSERT_MESSAGE("sal_Bool", !(a >>= b) && b);
    }
    {
        sal_Int8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int8", !(a >>= b) && b == 2);
    }
    {
        sal_uInt8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt8", !(a >>= b) && b == 2);
    }
    {
        sal_Int16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int16", !(a >>= b) && b == 2);
    }
    {
        sal_uInt16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt16", !(a >>= b) && b == 2);
    }
    {
        sal_Int32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int32", !(a >>= b) && b == 2);
    }
    {
        sal_uInt32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt32", !(a >>= b) && b == 2);
    }
    {
        sal_Int64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int64", !(a >>= b) && b == 2);
    }
    {
        sal_uInt64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt64", !(a >>= b) && b == 2);
    }
    {
        float b = 2;
        CPPUNIT_ASSERT_MESSAGE("float", !(a >>= b) && b == 2);
    }
    {
        double b = 2;
        CPPUNIT_ASSERT_MESSAGE("double", !(a >>= b) && b == 2);
    }
    {
        sal_Unicode b = '2';
        CPPUNIT_ASSERT_MESSAGE("sal_Unicode", !(a >>= b) && b == '2');
    }
    {
        rtl::OUString b("2");
        CPPUNIT_ASSERT_MESSAGE( "rtl::OUString", !(a >>= b) && b == "2" );
    }
    {
        css::uno::Type b(cppu::UnoType<OUString>::get());
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Type",
            !(a >>= b) && b == cppu::UnoType<OUString>::get());
    }
    {
        css::uno::Any b(rtl::OUString("2"));
        CPPUNIT_ASSERT_MESSAGE("css::uno::Any", (a >>= b) && b == a);
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Sequence<rtl::OUString>",
            !(a >>= b) && b.getLength() == 2);
    }
    {
        Enum1 b = Enum1_M2;
        CPPUNIT_ASSERT_MESSAGE("Enum1", !(a >>= b) && b == Enum1_M2);
    }
    {
        Struct1 b(2);
        CPPUNIT_ASSERT_MESSAGE("Struct1", !(a >>= b) && b.member == 2);
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        CPPUNIT_ASSERT_MESSAGE("Exception1", !(a >>= b) && b.member == 2);
    }
    {
        Exception2 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        CPPUNIT_ASSERT_MESSAGE("Exception2", (a >>= b) && b.member == 1);
    }
    {
        Exception2a b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2,
            2);
        CPPUNIT_ASSERT_MESSAGE(
            "Exception2a", (a >>= b) && b.member == 1 && b.member2 == 3);
    }
    {
        Exception2b b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2,
            2);
        CPPUNIT_ASSERT_MESSAGE("Exception2b", !(a >>= b) && b.member == 2);
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        CPPUNIT_ASSERT_MESSAGE("Interface1", !(a >>= b) && b == i);
    }
}

void Test::testInterface() {
    css::uno::Reference< Interface2a > i2(new Impl2);
    css::uno::Any a(i2);
    CPPUNIT_ASSERT(a.getValueType() == cppu::UnoType<Interface2a>::get());
    {
        bool b = true;
        CPPUNIT_ASSERT_MESSAGE("bool", !(a >>= b) && b);
    }
    {
        sal_Bool b = true;
        CPPUNIT_ASSERT_MESSAGE("sal_Bool", !(a >>= b) && b);
    }
    {
        sal_Int8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int8", !(a >>= b) && b == 2);
    }
    {
        sal_uInt8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt8", !(a >>= b) && b == 2);
    }
    {
        sal_Int16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int16", !(a >>= b) && b == 2);
    }
    {
        sal_uInt16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt16", !(a >>= b) && b == 2);
    }
    {
        sal_Int32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int32", !(a >>= b) && b == 2);
    }
    {
        sal_uInt32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt32", !(a >>= b) && b == 2);
    }
    {
        sal_Int64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int64", !(a >>= b) && b == 2);
    }
    {
        sal_uInt64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt64", !(a >>= b) && b == 2);
    }
    {
        float b = 2;
        CPPUNIT_ASSERT_MESSAGE("float", !(a >>= b) && b == 2);
    }
    {
        double b = 2;
        CPPUNIT_ASSERT_MESSAGE("double", !(a >>= b) && b == 2);
    }
    {
        sal_Unicode b = '2';
        CPPUNIT_ASSERT_MESSAGE("sal_Unicode", !(a >>= b) && b == '2');
    }
    {
        rtl::OUString b("2");
        CPPUNIT_ASSERT_MESSAGE( "rtl::OUString", !(a >>= b) && b == "2" );
    }
    {
        css::uno::Type b(cppu::UnoType<OUString>::get());
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Type",
            !(a >>= b) && b == cppu::UnoType<OUString>::get());
    }
    {
        css::uno::Any b(rtl::OUString("2"));
        CPPUNIT_ASSERT_MESSAGE("css::uno::Any", (a >>= b) && b == a);
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Sequence<rtl::OUString>",
            !(a >>= b) && b.getLength() == 2);
    }
    {
        Enum1 b = Enum1_M2;
        CPPUNIT_ASSERT_MESSAGE("Enum1", !(a >>= b) && b == Enum1_M2);
    }
    {
        Struct1 b(2);
        CPPUNIT_ASSERT_MESSAGE("Struct1", !(a >>= b) && b.member == 2);
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        CPPUNIT_ASSERT_MESSAGE("Exception1", !(a >>= b) && b.member == 2);
    }
    {
        css::uno::Reference< Interface1 > i(new Impl1);
        css::uno::Reference< Interface1 > b(i);
        CPPUNIT_ASSERT_MESSAGE("Interface1", !(a >>= b) && b == i);
    }
    {
        css::uno::Reference< Interface2 > b(new Impl2);
        CPPUNIT_ASSERT_MESSAGE("Interface2", (a >>= b) && b == i2);
    }
    {
        css::uno::Reference< Interface2a > b(new Impl2);
        CPPUNIT_ASSERT_MESSAGE("Interface2a", (a >>= b) && b == i2);
    }
    {
        css::uno::Reference< Interface2b > i(new Impl2b);
        css::uno::Reference< Interface2b > b(i);
        CPPUNIT_ASSERT_MESSAGE("Interface2b", !(a >>= b) && b == i);
    }
    {
        css::uno::Reference< Interface3 > b(new Impl2);
        CPPUNIT_ASSERT_MESSAGE("Interface3", (a >>= b) && b == i2);
    }
}

void Test::testNull() {
    css::uno::Any a = css::uno::Any(css::uno::Reference< Interface2a >());
    CPPUNIT_ASSERT(a.getValueType() == cppu::UnoType<Interface2a>::get());
    {
        bool b = true;
        CPPUNIT_ASSERT_MESSAGE("bool", !(a >>= b) && b);
    }
    {
        sal_Bool b = true;
        CPPUNIT_ASSERT_MESSAGE("sal_Bool", !(a >>= b) && b);
    }
    {
        sal_Int8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int8", !(a >>= b) && b == 2);
    }
    {
        sal_uInt8 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt8", !(a >>= b) && b == 2);
    }
    {
        sal_Int16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int16", !(a >>= b) && b == 2);
    }
    {
        sal_uInt16 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt16", !(a >>= b) && b == 2);
    }
    {
        sal_Int32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int32", !(a >>= b) && b == 2);
    }
    {
        sal_uInt32 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt32", !(a >>= b) && b == 2);
    }
    {
        sal_Int64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_Int64", !(a >>= b) && b == 2);
    }
    {
        sal_uInt64 b = 2;
        CPPUNIT_ASSERT_MESSAGE("sal_uInt64", !(a >>= b) && b == 2);
    }
    {
        float b = 2;
        CPPUNIT_ASSERT_MESSAGE("float", !(a >>= b) && b == 2);
    }
    {
        double b = 2;
        CPPUNIT_ASSERT_MESSAGE("double", !(a >>= b) && b == 2);
    }
    {
        sal_Unicode b = '2';
        CPPUNIT_ASSERT_MESSAGE("sal_Unicode", !(a >>= b) && b == '2');
    }
    {
        rtl::OUString b("2");
        CPPUNIT_ASSERT_MESSAGE( "rtl::OUString", !(a >>= b) && b == "2" );
    }
    {
        css::uno::Type b(cppu::UnoType<OUString>::get());
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Type",
            !(a >>= b) && b == cppu::UnoType<OUString>::get());
    }
    {
        css::uno::Any b(rtl::OUString("2"));
        CPPUNIT_ASSERT_MESSAGE("css::uno::Any", (a >>= b) && b == a);
    }
    {
        css::uno::Sequence< rtl::OUString > b(2);
        CPPUNIT_ASSERT_MESSAGE(
            "css::uno::Sequence<rtl::OUString>",
            !(a >>= b) && b.getLength() == 2);
    }
    {
        Enum1 b = Enum1_M2;
        CPPUNIT_ASSERT_MESSAGE("Enum1", !(a >>= b) && b == Enum1_M2);
    }
    {
        Struct1 b(2);
        CPPUNIT_ASSERT_MESSAGE("Struct1", !(a >>= b) && b.member == 2);
    }
    {
        Exception1 b(
            rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), 2);
        CPPUNIT_ASSERT_MESSAGE("Exception1", !(a >>= b) && b.member == 2);
    }
    {
        css::uno::Reference< Interface1 > b(new Impl1);
        CPPUNIT_ASSERT_MESSAGE(
            "Interface1", (a >>= b) && !b.is());
    }
    {
        css::uno::Reference< Interface2 > b(new Impl2);
        CPPUNIT_ASSERT_MESSAGE(
            "Interface2", (a >>= b) && !b.is());
    }
    {
        css::uno::Reference< Interface2a > b(new Impl2);
        CPPUNIT_ASSERT_MESSAGE("Interface2a", (a >>= b) && !b.is());
    }
    {
        css::uno::Reference< Interface2b > b(new Impl2b);
        CPPUNIT_ASSERT_MESSAGE(
            "Interface2b", (a >>= b) && !b.is());
    }
    {
        css::uno::Reference< Interface3 > b(new Impl2);
        CPPUNIT_ASSERT_MESSAGE(
            "Interface3", (a >>= b) && !b.is());
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
