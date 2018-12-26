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

#include <typeinfo>

#include <osl/mutex.hxx>
#include <salhelper/condition.hxx>
#include <salhelper/dynload.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <memory>

namespace {

std::type_info const & getConditionTypeInfo()
{ return typeid (salhelper::Condition *); }

std::type_info const & getConditionModifierTypeInfo()
{ return typeid (salhelper::ConditionModifier *); }

std::type_info const & getConditionWaiterTypeInfo()
{ return typeid (salhelper::ConditionWaiter *); }

std::type_info const & getORealDynamicLoaderTypeInfo()
{ return typeid (salhelper::ORealDynamicLoader *); }

std::type_info const & getSimpleReferenceObjectTypeInfo()
{ return typeid (salhelper::SimpleReferenceObject *); }

}

namespace {

class DerivedCondition: public salhelper::Condition {
public:
    explicit DerivedCondition(osl::Mutex & mutex): Condition(mutex) {}

protected:
    virtual bool applies() const override { return false; }
};

class DerivedConditionWaiterTimedout:
    public salhelper::ConditionWaiter::timedout
{};

class DerivedSimpleReferenceObject: public salhelper::SimpleReferenceObject {};

class Test: public CppUnit::TestFixture {
public:
    void testCondition();

    void testConditionModifier();

    void testConditionWaiter();

    void testConditionWaiterTimedout();

    void testORealDynamicLoader();

    void testSimpleReferenceObject();

    void testDerivedCondition();

    void testDerivedConditionWaiterTimedout();

    void testDerivedSimpleReferenceObject();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testCondition);
    CPPUNIT_TEST(testConditionModifier);
    CPPUNIT_TEST(testConditionWaiter);
    CPPUNIT_TEST(testConditionWaiterTimedout);
    CPPUNIT_TEST(testORealDynamicLoader);
    CPPUNIT_TEST(testSimpleReferenceObject);
    CPPUNIT_TEST(testDerivedCondition);
    CPPUNIT_TEST(testDerivedConditionWaiterTimedout);
    CPPUNIT_TEST(testDerivedSimpleReferenceObject);
    CPPUNIT_TEST_SUITE_END();
};

void Test::testCondition() {
    osl::Mutex mutex;
    std::unique_ptr< salhelper::Condition > p(new DerivedCondition(mutex));
    CPPUNIT_ASSERT(typeid(*p) != typeid(salhelper::Condition));
    CPPUNIT_ASSERT(bool(typeid (p.get()) == typeid (salhelper::Condition *)));
    CPPUNIT_ASSERT(bool(
        typeid (const_cast< salhelper::Condition const * >(p.get()))
        == typeid (salhelper::Condition const *)));
    CPPUNIT_ASSERT(bool(
        typeid (const_cast< salhelper::Condition volatile * >(p.get()))
        == typeid (salhelper::Condition volatile *)));
    CPPUNIT_ASSERT(bool(typeid (salhelper::Condition *) == getConditionTypeInfo()));
}

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4189) // 'p': local variable is initialized but not referenced
#endif


void Test::testConditionModifier() {
    salhelper::ConditionModifier * p = nullptr;
    CPPUNIT_ASSERT(bool(typeid (*p) == typeid (salhelper::ConditionModifier)));
    CPPUNIT_ASSERT(bool(typeid (p) == typeid (salhelper::ConditionModifier *)));
    CPPUNIT_ASSERT(bool(
        typeid (const_cast< salhelper::ConditionModifier const * >(p))
        == typeid (salhelper::ConditionModifier const *)));
    CPPUNIT_ASSERT(bool(
        typeid (const_cast< salhelper::ConditionModifier volatile * >(p))
        == typeid (salhelper::ConditionModifier volatile *)));
    CPPUNIT_ASSERT(bool(
        typeid (salhelper::ConditionModifier *)
        == getConditionModifierTypeInfo()));
}

void Test::testConditionWaiter() {
    salhelper::ConditionWaiter * p = nullptr;
    CPPUNIT_ASSERT(bool(typeid (*p) == typeid (salhelper::ConditionWaiter)));
    CPPUNIT_ASSERT(bool(typeid (p) == typeid (salhelper::ConditionWaiter *)));
    CPPUNIT_ASSERT(bool(
        typeid (const_cast< salhelper::ConditionWaiter const * >(p))
        == typeid (salhelper::ConditionWaiter const *)));
    CPPUNIT_ASSERT(bool(
        typeid (const_cast< salhelper::ConditionWaiter volatile * >(p))
        == typeid (salhelper::ConditionWaiter volatile *)));
    CPPUNIT_ASSERT(bool(
        typeid (salhelper::ConditionWaiter *) == getConditionWaiterTypeInfo()));
}

void Test::testConditionWaiterTimedout() {
    salhelper::ConditionWaiter::timedout x;
    CPPUNIT_ASSERT(bool(typeid (x) == typeid (salhelper::ConditionWaiter::timedout)));
    CPPUNIT_ASSERT(bool(
        typeid (&x) == typeid (salhelper::ConditionWaiter::timedout *)));
    CPPUNIT_ASSERT(bool(
        typeid (const_cast< salhelper::ConditionWaiter::timedout const * >(&x))
        == typeid (salhelper::ConditionWaiter::timedout const *)));
    CPPUNIT_ASSERT(bool(
        (typeid
         (const_cast< salhelper::ConditionWaiter::timedout volatile * >(&x)))
        == typeid (salhelper::ConditionWaiter::timedout volatile *)));
    try {
        throw salhelper::ConditionWaiter::timedout();
    } catch (salhelper::ConditionWaiter::timedout &) {
    } catch (...) {
        CPPUNIT_FAIL("not caught");
    }
}

void Test::testORealDynamicLoader() {
    salhelper::ORealDynamicLoader * p = nullptr;
    CPPUNIT_ASSERT(typeid (p) != typeid (salhelper::ORealDynamicLoader));
    CPPUNIT_ASSERT(bool(typeid (p) == typeid (salhelper::ORealDynamicLoader *)));
    CPPUNIT_ASSERT(bool(
        typeid (const_cast< salhelper::ORealDynamicLoader const * >(p))
        == typeid (salhelper::ORealDynamicLoader const *)));
    CPPUNIT_ASSERT(bool(
        typeid (const_cast< salhelper::ORealDynamicLoader volatile * >(p))
        == typeid (salhelper::ORealDynamicLoader volatile *)));
    CPPUNIT_ASSERT(bool(
        typeid (salhelper::ORealDynamicLoader *)
        == getORealDynamicLoaderTypeInfo()));
}

#ifdef _MSC_VER
#pragma warning (pop)
#endif

void Test::testSimpleReferenceObject() {
    salhelper::SimpleReferenceObject * p = new DerivedSimpleReferenceObject;
    try {
        CPPUNIT_ASSERT(
            typeid (*p) != typeid (salhelper::SimpleReferenceObject));
        CPPUNIT_ASSERT(bool(
            typeid (p) == typeid (salhelper::SimpleReferenceObject *)));
        CPPUNIT_ASSERT(bool(
            typeid (const_cast< salhelper::SimpleReferenceObject const * >(p))
            == typeid (salhelper::SimpleReferenceObject const *)));
        CPPUNIT_ASSERT(bool(
            (typeid
             (const_cast< salhelper::SimpleReferenceObject volatile * >(p)))
            == typeid (salhelper::SimpleReferenceObject volatile *)));
        CPPUNIT_ASSERT(bool(
            typeid (salhelper::SimpleReferenceObject *)
            == getSimpleReferenceObjectTypeInfo()));
    } catch (...) {
        delete static_cast< DerivedSimpleReferenceObject * >(p);
        throw;
    }
    delete static_cast< DerivedSimpleReferenceObject * >(p);
}

void Test::testDerivedCondition() {
    osl::Mutex mutex;
    // Next line tests that new doesn't throw
    std::unique_ptr< salhelper::Condition > p(new DerivedCondition(mutex));
}

void Test::testDerivedConditionWaiterTimedout() {
    // Next line tests that new doesn't throw
    std::unique_ptr< salhelper::ConditionWaiter::timedout > p(
        new DerivedConditionWaiterTimedout);
    try {
        throw DerivedConditionWaiterTimedout();
    } catch (salhelper::ConditionWaiter::timedout &) {
    } catch (...) {
        CPPUNIT_FAIL("not caught");
    }
}

void Test::testDerivedSimpleReferenceObject() {
    salhelper::SimpleReferenceObject * p = new DerivedSimpleReferenceObject;
    try {
        CPPUNIT_ASSERT(dynamic_cast< DerivedSimpleReferenceObject * >(p) != nullptr);
    } catch (...) {
        delete static_cast< DerivedSimpleReferenceObject * >(p);
        throw;
    }
    delete static_cast< DerivedSimpleReferenceObject * >(p);
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
