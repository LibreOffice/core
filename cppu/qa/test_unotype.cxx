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

#include <cppunit/TestFixture.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <cppunit/extensions/HelperMacros.h>

#include <typeinfo>

#include <com/sun/star/beans/Optional.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/uno/TypeClass.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <cppu/unotype.hxx>
#include <rtl/ustring.hxx>

namespace com { namespace sun { namespace star { namespace uno {
    class Any;
} } } }

namespace com { namespace sun { namespace star { namespace uno { template <class interface_type> class Reference; } } } }

namespace {

struct DerivedStruct1: css::lang::EventObject {};

struct DerivedStruct2: css::beans::PropertyChangeEvent {};

struct DerivedException1: css::uno::Exception {};

struct DerivedException2: css::uno::RuntimeException {};

struct DerivedInterface1: css::uno::XInterface {
private:
    ~DerivedInterface1() {}
        // avoid warnings about virtual members and non-virtual dtor

public:
    static void dummy(DerivedInterface1 * p)
    { p->DerivedInterface1::~DerivedInterface1(); }
        // ...and avoid warnings about unused ~DerivedInterface1 (see below)
};

struct DerivedInterface2: css::uno::XComponentContext {
private:
    ~DerivedInterface2() {}
        // avoid warnings about virtual members and non-virtual dtor

public:
    static void dummy(DerivedInterface2 * p)
    { p->DerivedInterface2::~DerivedInterface2(); }
        // ...and avoid warnings about unused ~DerivedInterface2 (see below)
};

class Test: public CppUnit::TestFixture {
public:
    void testUnoType();

    void testGetTypeFavourUnsigned();

    void testGetTypeFavourChar();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testUnoType);
    CPPUNIT_TEST(testGetTypeFavourUnsigned);
    CPPUNIT_TEST(testGetTypeFavourChar);
    CPPUNIT_TEST_SUITE_END();
};

void Test::testUnoType() {
    // Avoid warnings about unused ~DerivedInterface1/2 (see above):
    if ((false)) {
        DerivedInterface1::dummy(nullptr);
        DerivedInterface2::dummy(nullptr);
    }

    css::uno::Type t;
    t = cppu::UnoType<cppu::UnoVoidType>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_VOID, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(OUString("void"), t.getTypeName());
    CPPUNIT_ASSERT(bool(cppu::UnoType<void>::get() == t));
    t = cppu::UnoType<bool>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_BOOLEAN, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(OUString("boolean"), t.getTypeName());
    CPPUNIT_ASSERT(bool(cppu::UnoType<sal_Bool>::get() == t));
    t = cppu::UnoType<sal_Int8>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_BYTE, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(OUString("byte"), t.getTypeName());
    t = cppu::UnoType<sal_Int16>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_SHORT, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(OUString("short"), t.getTypeName());
    t = cppu::UnoType<cppu::UnoUnsignedShortType>::get();
    CPPUNIT_ASSERT_EQUAL(
        css::uno::TypeClass_UNSIGNED_SHORT, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(OUString("unsigned short"), t.getTypeName());
    t = cppu::UnoType<sal_Int32>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_LONG, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(OUString("long"), t.getTypeName());
    t = cppu::UnoType<sal_uInt32>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_UNSIGNED_LONG, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(OUString("unsigned long"), t.getTypeName());
    t = cppu::UnoType<sal_Int64>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_HYPER, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(OUString("hyper"), t.getTypeName());
    t = cppu::UnoType<sal_uInt64>::get();
    CPPUNIT_ASSERT_EQUAL(
        css::uno::TypeClass_UNSIGNED_HYPER, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(OUString("unsigned hyper"), t.getTypeName());
    t = cppu::UnoType<float>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_FLOAT, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(OUString("float"), t.getTypeName());
    t = cppu::UnoType<double>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_DOUBLE, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(OUString("double"), t.getTypeName());
    t = cppu::UnoType<cppu::UnoCharType>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_CHAR, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(OUString("char"), t.getTypeName());
    t = cppu::UnoType<OUString>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_STRING, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(OUString("string"), t.getTypeName());
    t = cppu::UnoType<css::uno::Type>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_TYPE, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(OUString("type"), t.getTypeName());
    t = cppu::UnoType<css::uno::Any>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_ANY, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(OUString("any"), t.getTypeName());
    t = cppu::UnoType<cppu::UnoSequenceType<sal_Int8>>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_SEQUENCE, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(OUString("[]byte"), t.getTypeName());
    CPPUNIT_ASSERT(bool(cppu::UnoType<css::uno::Sequence<sal_Int8>>::get() == t));
    t = cppu::UnoType<cppu::UnoSequenceType<cppu::UnoUnsignedShortType>>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_SEQUENCE, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(OUString("[]unsigned short"), t.getTypeName());
    t = cppu::UnoType<cppu::UnoSequenceType<cppu::UnoCharType>>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_SEQUENCE, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(OUString("[]char"), t.getTypeName());
    t = cppu::UnoType<
        cppu::UnoSequenceType<cppu::UnoSequenceType<sal_Int8>>>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_SEQUENCE, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(OUString("[][]byte"), t.getTypeName());
    CPPUNIT_ASSERT_EQUAL(
        cppu::UnoType<css::uno::Sequence<css::uno::Sequence<sal_Int8>>>::get(),
        t);
    t = cppu::UnoType<
        cppu::UnoSequenceType<
            cppu::UnoSequenceType<cppu::UnoUnsignedShortType>>>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_SEQUENCE, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(OUString("[][]unsigned short"), t.getTypeName());
    t = cppu::UnoType<
        cppu::UnoSequenceType<cppu::UnoSequenceType<cppu::UnoCharType>>>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_SEQUENCE, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(OUString("[][]char"), t.getTypeName());
    t = cppu::UnoType<css::uno::TypeClass>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_ENUM, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(
        OUString("com.sun.star.uno.TypeClass"), t.getTypeName());
    t = cppu::UnoType<css::lang::EventObject>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_STRUCT, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(
        OUString("com.sun.star.lang.EventObject"), t.getTypeName());
    CPPUNIT_ASSERT_EQUAL(cppu::UnoType<DerivedStruct1>::get(), t);
    t = cppu::UnoType<css::beans::PropertyChangeEvent>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_STRUCT, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(
        OUString("com.sun.star.beans.PropertyChangeEvent"), t.getTypeName());
    CPPUNIT_ASSERT_EQUAL(cppu::UnoType<DerivedStruct2>::get(), t);
    t = cppu::UnoType<css::beans::Optional<sal_Int8>>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_STRUCT, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(
        OUString("com.sun.star.beans.Optional<byte>"), t.getTypeName());
    t = cppu::UnoType<css::uno::Exception>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_EXCEPTION, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(
        OUString("com.sun.star.uno.Exception"), t.getTypeName());
    CPPUNIT_ASSERT_EQUAL(cppu::UnoType<DerivedException1>::get(), t);
    t = cppu::UnoType<css::uno::RuntimeException>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_EXCEPTION, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(
        OUString("com.sun.star.uno.RuntimeException"), t.getTypeName());
    CPPUNIT_ASSERT_EQUAL(cppu::UnoType<DerivedException2>::get(), t);
    t = cppu::UnoType<css::uno::XInterface>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_INTERFACE, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(
        OUString("com.sun.star.uno.XInterface"), t.getTypeName());
    CPPUNIT_ASSERT_EQUAL(
        cppu::UnoType<css::uno::Reference<css::uno::XInterface>>::get(), t);
    CPPUNIT_ASSERT_EQUAL(cppu::UnoType<DerivedInterface1>::get(), t);
    CPPUNIT_ASSERT_EQUAL(
        cppu::UnoType<css::uno::Reference<DerivedInterface1>>::get(), t);
    t = cppu::UnoType<css::uno::XComponentContext>::get();
    CPPUNIT_ASSERT_EQUAL(css::uno::TypeClass_INTERFACE, t.getTypeClass());
    CPPUNIT_ASSERT_EQUAL(
        OUString("com.sun.star.uno.XComponentContext"), t.getTypeName());
    CPPUNIT_ASSERT_EQUAL(
        cppu::UnoType<css::uno::Reference<css::uno::XComponentContext>>::get(),
        t);
    CPPUNIT_ASSERT_EQUAL(cppu::UnoType<DerivedInterface2>::get(), t);
    CPPUNIT_ASSERT_EQUAL(
        cppu::UnoType<css::uno::Reference<DerivedInterface2>>::get(), t);
}

void Test::testGetTypeFavourUnsigned() {
    CPPUNIT_ASSERT(typeid(sal_Unicode) != typeid(sal_uInt16));
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(static_cast<cppu::UnoVoidType *>(nullptr)),
        cppu::UnoType<cppu::UnoVoidType>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(static_cast<bool *>(nullptr)),
        cppu::UnoType<bool>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(static_cast<sal_Bool *>(nullptr)),
        cppu::UnoType<bool>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(static_cast<sal_Int8 *>(nullptr)),
        cppu::UnoType<sal_Int8>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(static_cast<sal_Int16 *>(nullptr)),
        cppu::UnoType<sal_Int16>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(
            static_cast<cppu::UnoUnsignedShortType *>(nullptr)),
        cppu::UnoType<cppu::UnoUnsignedShortType>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(static_cast<sal_uInt16 *>(nullptr)),
        cppu::UnoType<cppu::UnoUnsignedShortType>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(static_cast<sal_Int32 *>(nullptr)),
        cppu::UnoType<sal_Int32>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(static_cast<sal_uInt32 *>(nullptr)),
        cppu::UnoType<sal_uInt32>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(static_cast<sal_Int64 *>(nullptr)),
        cppu::UnoType<sal_Int64>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(static_cast<sal_uInt64 *>(nullptr)),
        cppu::UnoType<sal_uInt64>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(static_cast<float *>(nullptr)),
        cppu::UnoType<float>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(static_cast<double *>(nullptr)),
        cppu::UnoType<double>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(static_cast<cppu::UnoCharType *>(nullptr)),
        cppu::UnoType<cppu::UnoCharType>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(static_cast<sal_Unicode *>(nullptr)),
        cppu::UnoType<cppu::UnoCharType>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(static_cast<OUString *>(nullptr)),
        cppu::UnoType<OUString>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(static_cast<css::uno::Type *>(nullptr)),
        cppu::UnoType<css::uno::Type>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(static_cast<css::uno::Any *>(nullptr)),
        cppu::UnoType<css::uno::Any>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(
            static_cast<
                cppu::UnoSequenceType<cppu::UnoUnsignedShortType> *>(nullptr)),
        cppu::UnoType<
            cppu::UnoSequenceType<cppu::UnoUnsignedShortType>>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(
            static_cast<css::uno::Sequence<sal_uInt16> *>(nullptr)),
        cppu::UnoType<
            cppu::UnoSequenceType<cppu::UnoUnsignedShortType>>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(
            static_cast<
                cppu::UnoSequenceType<
                    cppu::UnoSequenceType<
                        cppu::UnoUnsignedShortType>> *>(nullptr)),
        cppu::UnoType<
            cppu::UnoSequenceType<
                cppu::UnoSequenceType<cppu::UnoUnsignedShortType>>>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(
            static_cast<
                css::uno::Sequence<css::uno::Sequence<sal_uInt16>> *>(nullptr)),
        cppu::UnoType<
            cppu::UnoSequenceType<
                cppu::UnoSequenceType<cppu::UnoUnsignedShortType>>>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(
            static_cast<css::uno::Sequence<sal_Unicode> *>(nullptr)),
        cppu::UnoType<cppu::UnoSequenceType<cppu::UnoCharType>>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(
            static_cast<
                css::uno::Sequence<
                    css::uno::Sequence<sal_Unicode>> *>(nullptr)),
        cppu::UnoType<
            cppu::UnoSequenceType<
                cppu::UnoSequenceType<cppu::UnoCharType>>>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(
            static_cast<css::uno::TypeClass *>(nullptr)),
        cppu::UnoType<css::uno::TypeClass>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(
            static_cast<css::lang::EventObject *>(nullptr)),
        cppu::UnoType<css::lang::EventObject>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(static_cast<DerivedStruct1 *>(nullptr)),
        cppu::UnoType<css::lang::EventObject>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(
            static_cast<css::beans::PropertyChangeEvent *>(nullptr)),
        cppu::UnoType<css::beans::PropertyChangeEvent>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(static_cast<DerivedStruct2 *>(nullptr)),
        cppu::UnoType<css::beans::PropertyChangeEvent>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(
            static_cast<css::beans::Optional<sal_Int8> *>(nullptr)),
        cppu::UnoType<css::beans::Optional<sal_Int8>>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(
            static_cast<css::uno::Exception *>(nullptr)),
        cppu::UnoType<css::uno::Exception>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(static_cast<DerivedException1 *>(nullptr)),
        cppu::UnoType<css::uno::Exception>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(
            static_cast<css::uno::RuntimeException *>(nullptr)),
        cppu::UnoType<css::uno::RuntimeException>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(static_cast<DerivedException2 *>(nullptr)),
        cppu::UnoType<css::uno::RuntimeException>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(
            static_cast<css::uno::XInterface *>(nullptr)),
        cppu::UnoType<css::uno::XInterface>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(
            static_cast<css::uno::Reference<css::uno::XInterface> *>(nullptr)),
        cppu::UnoType<css::uno::XInterface>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(static_cast<DerivedInterface1 *>(nullptr)),
        cppu::UnoType<css::uno::XInterface>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(
            static_cast<css::uno::Reference<DerivedInterface1> *>(nullptr)),
        cppu::UnoType<css::uno::XInterface>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(
            static_cast<css::uno::XComponentContext *>(nullptr)),
        cppu::UnoType<css::uno::XComponentContext>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(
            static_cast<
                css::uno::Reference<css::uno::XComponentContext> *>(nullptr)),
        cppu::UnoType<css::uno::XComponentContext>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(static_cast<DerivedInterface2 *>(nullptr)),
        cppu::UnoType<css::uno::XComponentContext>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourUnsigned(
            static_cast<css::uno::Reference<DerivedInterface2> *>(nullptr)),
        cppu::UnoType<css::uno::XComponentContext>::get());
}

void Test::testGetTypeFavourChar() {
    CPPUNIT_ASSERT(typeid(sal_Unicode) != typeid(sal_uInt16));
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<cppu::UnoVoidType *>(nullptr)),
        cppu::UnoType<cppu::UnoVoidType>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<bool *>(nullptr)),
        cppu::UnoType<bool>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<sal_Bool *>(nullptr)),
        cppu::UnoType<bool>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<sal_Int8 *>(nullptr)),
        cppu::UnoType<sal_Int8>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<sal_Int16 *>(nullptr)),
        cppu::UnoType<sal_Int16>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(
            static_cast<cppu::UnoUnsignedShortType *>(nullptr)),
        cppu::UnoType<cppu::UnoUnsignedShortType>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<sal_uInt16 *>(nullptr)),
        cppu::UnoType<cppu::UnoUnsignedShortType>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<sal_Int32 *>(nullptr)),
        cppu::UnoType<sal_Int32>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<sal_uInt32 *>(nullptr)),
        cppu::UnoType<sal_uInt32>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<sal_Int64 *>(nullptr)),
        cppu::UnoType<sal_Int64>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<sal_uInt64 *>(nullptr)),
        cppu::UnoType<sal_uInt64>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<float *>(nullptr)),
        cppu::UnoType<float>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<double *>(nullptr)),
        cppu::UnoType<double>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<cppu::UnoCharType *>(nullptr)),
        cppu::UnoType<cppu::UnoCharType>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<sal_Unicode *>(nullptr)),
        cppu::UnoType<cppu::UnoCharType>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<OUString *>(nullptr)),
        cppu::UnoType<OUString>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<css::uno::Type *>(nullptr)),
        cppu::UnoType<css::uno::Type>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<css::uno::Any *>(nullptr)),
        cppu::UnoType<css::uno::Any>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(
            static_cast<
                cppu::UnoSequenceType<cppu::UnoUnsignedShortType> *>(nullptr)),
        cppu::UnoType<
            cppu::UnoSequenceType<cppu::UnoUnsignedShortType>>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(
            static_cast<css::uno::Sequence<sal_uInt16> *>(nullptr)),
        cppu::UnoType<
            cppu::UnoSequenceType<cppu::UnoUnsignedShortType>>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(
            static_cast<
                cppu::UnoSequenceType<
                    cppu::UnoSequenceType<
                        cppu::UnoUnsignedShortType>> *>(nullptr)),
        cppu::UnoType<
            cppu::UnoSequenceType<
                cppu::UnoSequenceType<cppu::UnoUnsignedShortType>>>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(
            static_cast<
                css::uno::Sequence<css::uno::Sequence<sal_uInt16>> *>(nullptr)),
        cppu::UnoType<
            cppu::UnoSequenceType<
                cppu::UnoSequenceType<cppu::UnoUnsignedShortType>>>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(
            static_cast<css::uno::Sequence<sal_Unicode> *>(nullptr)),
        cppu::UnoType<cppu::UnoSequenceType<cppu::UnoCharType>>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(
            static_cast<
                css::uno::Sequence<
                    css::uno::Sequence<sal_Unicode>> *>(nullptr)),
        cppu::UnoType<
            cppu::UnoSequenceType<
                cppu::UnoSequenceType<cppu::UnoCharType>>>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<css::uno::TypeClass *>(nullptr)),
        cppu::UnoType<css::uno::TypeClass>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<css::lang::EventObject *>(nullptr)),
        cppu::UnoType<css::lang::EventObject>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<DerivedStruct1 *>(nullptr)),
        cppu::UnoType<css::lang::EventObject>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(
            static_cast<css::beans::PropertyChangeEvent *>(nullptr)),
        cppu::UnoType<css::beans::PropertyChangeEvent>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<DerivedStruct2 *>(nullptr)),
        cppu::UnoType<css::beans::PropertyChangeEvent>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(
            static_cast<css::beans::Optional<sal_Int8> *>(nullptr)),
        cppu::UnoType<css::beans::Optional<sal_Int8>>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<css::uno::Exception *>(nullptr)),
        cppu::UnoType<css::uno::Exception>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<DerivedException1 *>(nullptr)),
        cppu::UnoType<css::uno::Exception>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(
            static_cast<css::uno::RuntimeException *>(nullptr)),
        cppu::UnoType<css::uno::RuntimeException>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<DerivedException2 *>(nullptr)),
        cppu::UnoType<css::uno::RuntimeException>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<css::uno::XInterface *>(nullptr)),
        cppu::UnoType<css::uno::XInterface>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(
            static_cast<css::uno::Reference<css::uno::XInterface> *>(nullptr)),
        cppu::UnoType<css::uno::XInterface>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<DerivedInterface1 *>(nullptr)),
        cppu::UnoType<css::uno::XInterface>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(
            static_cast<css::uno::Reference<DerivedInterface1> *>(nullptr)),
        cppu::UnoType<css::uno::XInterface>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(
            static_cast<css::uno::XComponentContext *>(nullptr)),
        cppu::UnoType<css::uno::XComponentContext>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(
            static_cast<
                css::uno::Reference<css::uno::XComponentContext> *>(nullptr)),
        cppu::UnoType<css::uno::XComponentContext>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(static_cast<DerivedInterface2 *>(nullptr)),
        cppu::UnoType<css::uno::XComponentContext>::get());
    CPPUNIT_ASSERT_EQUAL(
        cppu::getTypeFavourChar(
            static_cast<css::uno::Reference<DerivedInterface2> *>(nullptr)),
        cppu::UnoType<css::uno::XComponentContext>::get());
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
