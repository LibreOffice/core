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

#include "sal/config.h"

#include <typeinfo>

namespace salhelper {
    class Condition;
    class ConditionModifier;
    class ConditionWaiter;
    class ORealDynamicLoader;
    class SimpleReferenceObject;
}

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

#include "testshl/simpleheader.hxx"
#include "osl/mutex.hxx"
#include "salhelper/condition.hxx"
#include "salhelper/dynload.hxx"
#include "salhelper/simplereferenceobject.hxx"

#include <memory>

namespace {

class DerivedCondition: public salhelper::Condition {
public:
    explicit DerivedCondition(osl::Mutex & mutex): Condition(mutex) {}

protected:
    virtual bool applies() const { return false; }
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
    std::auto_ptr< salhelper::Condition > p(new DerivedCondition(mutex));
    CPPUNIT_ASSERT(typeid (*p.get()) != typeid (salhelper::Condition));
    CPPUNIT_ASSERT(typeid (p.get()) == typeid (salhelper::Condition *));
    CPPUNIT_ASSERT(
        typeid (const_cast< salhelper::Condition const * >(p.get()))
        == typeid (salhelper::Condition const *));
    CPPUNIT_ASSERT(
        typeid (const_cast< salhelper::Condition volatile * >(p.get()))
        == typeid (salhelper::Condition volatile *));
    CPPUNIT_ASSERT(typeid (salhelper::Condition *) == getConditionTypeInfo());
}

void Test::testConditionModifier() {
    salhelper::ConditionModifier * p = 0;
    CPPUNIT_ASSERT(typeid (*p) == typeid (salhelper::ConditionModifier));
    CPPUNIT_ASSERT(typeid (p) == typeid (salhelper::ConditionModifier *));
    CPPUNIT_ASSERT(
        typeid (const_cast< salhelper::ConditionModifier const * >(p))
        == typeid (salhelper::ConditionModifier const *));
    CPPUNIT_ASSERT(
        typeid (const_cast< salhelper::ConditionModifier volatile * >(p))
        == typeid (salhelper::ConditionModifier volatile *));
    CPPUNIT_ASSERT(
        typeid (salhelper::ConditionModifier *)
        == getConditionModifierTypeInfo());
}

void Test::testConditionWaiter() {
    salhelper::ConditionWaiter * p = 0;
    CPPUNIT_ASSERT(typeid (*p) == typeid (salhelper::ConditionWaiter));
    CPPUNIT_ASSERT(typeid (p) == typeid (salhelper::ConditionWaiter *));
    CPPUNIT_ASSERT(
        typeid (const_cast< salhelper::ConditionWaiter const * >(p))
        == typeid (salhelper::ConditionWaiter const *));
    CPPUNIT_ASSERT(
        typeid (const_cast< salhelper::ConditionWaiter volatile * >(p))
        == typeid (salhelper::ConditionWaiter volatile *));
    CPPUNIT_ASSERT(
        typeid (salhelper::ConditionWaiter *) == getConditionWaiterTypeInfo());
}

void Test::testConditionWaiterTimedout() {
    salhelper::ConditionWaiter::timedout x;
    CPPUNIT_ASSERT(typeid (x) == typeid (salhelper::ConditionWaiter::timedout));
    CPPUNIT_ASSERT(
        typeid (&x) == typeid (salhelper::ConditionWaiter::timedout *));
    CPPUNIT_ASSERT(
        typeid (const_cast< salhelper::ConditionWaiter::timedout const * >(&x))
        == typeid (salhelper::ConditionWaiter::timedout const *));
    CPPUNIT_ASSERT(
        (typeid
         (const_cast< salhelper::ConditionWaiter::timedout volatile * >(&x)))
        == typeid (salhelper::ConditionWaiter::timedout volatile *));
    try {
        throw salhelper::ConditionWaiter::timedout();
    } catch (salhelper::ConditionWaiter::timedout &) {
    } catch (...) {
        CPPUNIT_FAIL("not caught");
    }
}

void Test::testORealDynamicLoader() {
    salhelper::ORealDynamicLoader * p = 0;
    CPPUNIT_ASSERT(typeid (p) != typeid (salhelper::ORealDynamicLoader));
    CPPUNIT_ASSERT(typeid (p) == typeid (salhelper::ORealDynamicLoader *));
    CPPUNIT_ASSERT(
        typeid (const_cast< salhelper::ORealDynamicLoader const * >(p))
        == typeid (salhelper::ORealDynamicLoader const *));
    CPPUNIT_ASSERT(
        typeid (const_cast< salhelper::ORealDynamicLoader volatile * >(p))
        == typeid (salhelper::ORealDynamicLoader volatile *));
    CPPUNIT_ASSERT(
        typeid (salhelper::ORealDynamicLoader *)
        == getORealDynamicLoaderTypeInfo());
}

void Test::testSimpleReferenceObject() {
    salhelper::SimpleReferenceObject * p = new DerivedSimpleReferenceObject;
    try {
        CPPUNIT_ASSERT(
            typeid (*p) != typeid (salhelper::SimpleReferenceObject));
        CPPUNIT_ASSERT(
            typeid (p) == typeid (salhelper::SimpleReferenceObject *));
        CPPUNIT_ASSERT(
            typeid (const_cast< salhelper::SimpleReferenceObject const * >(p))
            == typeid (salhelper::SimpleReferenceObject const *));
        CPPUNIT_ASSERT(
            (typeid
             (const_cast< salhelper::SimpleReferenceObject volatile * >(p)))
            == typeid (salhelper::SimpleReferenceObject volatile *));
        CPPUNIT_ASSERT(
            typeid (salhelper::SimpleReferenceObject *)
            == getSimpleReferenceObjectTypeInfo());
    } catch (...) {
        delete static_cast< DerivedSimpleReferenceObject * >(p);
        throw;
    }
}

void Test::testDerivedCondition() {
    osl::Mutex mutex;
    std::auto_ptr< salhelper::Condition > p(new DerivedCondition(mutex));
    CPPUNIT_ASSERT(dynamic_cast< DerivedCondition * >(p.get()) != 0);
}

void Test::testDerivedConditionWaiterTimedout() {
    std::auto_ptr< salhelper::ConditionWaiter::timedout > p(
        new DerivedConditionWaiterTimedout);
    CPPUNIT_ASSERT(
        dynamic_cast< DerivedConditionWaiterTimedout * >(p.get()) != 0);
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
        CPPUNIT_ASSERT(dynamic_cast< DerivedSimpleReferenceObject * >(p) != 0);
    } catch (...) {
        delete static_cast< DerivedSimpleReferenceObject * >(p);
        throw;
    }
}

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(Test, "alltests");

}

NOADDITIONAL;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
