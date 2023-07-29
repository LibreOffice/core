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
#include <osl/mutex.hxx>
#include <salhelper/condition.hxx>
#include <salhelper/dynload.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <memory>

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

    void testConditionWaiterTimedout();

    void testORealDynamicLoader();

    void testSimpleReferenceObject();

    void testDerivedCondition();

    void testDerivedConditionWaiterTimedout();

    void testDerivedSimpleReferenceObject();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testCondition);
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
    [[maybe_unused]] volatile auto const rtti = &typeid(salhelper::Condition);
}

void Test::testConditionWaiterTimedout() {
    salhelper::ConditionWaiter::timedout x;
    [[maybe_unused]] volatile auto const rtti = &typeid(salhelper::ConditionWaiter::timedout);
    try {
        throw salhelper::ConditionWaiter::timedout();
    } catch (salhelper::ConditionWaiter::timedout &) {
    } catch (...) {
        CPPUNIT_FAIL("not caught");
    }
}

void Test::testORealDynamicLoader() {
    [[maybe_unused]] volatile auto const rtti = &typeid(salhelper::ORealDynamicLoader);
}

void Test::testSimpleReferenceObject() {
    salhelper::SimpleReferenceObject * p = new DerivedSimpleReferenceObject;
    delete static_cast< DerivedSimpleReferenceObject * >(p);
    [[maybe_unused]] volatile auto const rtti = &typeid(salhelper::SimpleReferenceObject);
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
