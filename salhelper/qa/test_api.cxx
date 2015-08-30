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

#include "osl/mutex.hxx"
#include "salhelper/condition.hxx"
#include "salhelper/dynload.hxx"
#include "salhelper/simplereferenceobject.hxx"
#include "gtest/gtest.h"

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

class Test: public ::testing::Test {
public:
};

TEST_F(Test, testCondition) {
    osl::Mutex mutex;
    std::auto_ptr< salhelper::Condition > p(new DerivedCondition(mutex));
    ASSERT_TRUE(typeid (*p.get()) != typeid (salhelper::Condition));
    ASSERT_TRUE(typeid (p.get()) == typeid (salhelper::Condition *));
    ASSERT_TRUE(
        typeid (const_cast< salhelper::Condition const * >(p.get()))
        == typeid (salhelper::Condition const *));
    ASSERT_TRUE(
        typeid (const_cast< salhelper::Condition volatile * >(p.get()))
        == typeid (salhelper::Condition volatile *));
    ASSERT_TRUE(typeid (salhelper::Condition *) == getConditionTypeInfo());
}

TEST_F(Test, testConditionModifier) {
    salhelper::ConditionModifier * p = 0;
    ASSERT_TRUE(typeid (*p) == typeid (salhelper::ConditionModifier));
    ASSERT_TRUE(typeid (p) == typeid (salhelper::ConditionModifier *));
    ASSERT_TRUE(
        typeid (const_cast< salhelper::ConditionModifier const * >(p))
        == typeid (salhelper::ConditionModifier const *));
    ASSERT_TRUE(
        typeid (const_cast< salhelper::ConditionModifier volatile * >(p))
        == typeid (salhelper::ConditionModifier volatile *));
    ASSERT_TRUE(
        typeid (salhelper::ConditionModifier *)
        == getConditionModifierTypeInfo());
}

TEST_F(Test, testConditionWaiter) {
    salhelper::ConditionWaiter * p = 0;
    ASSERT_TRUE(typeid (*p) == typeid (salhelper::ConditionWaiter));
    ASSERT_TRUE(typeid (p) == typeid (salhelper::ConditionWaiter *));
    ASSERT_TRUE(
        typeid (const_cast< salhelper::ConditionWaiter const * >(p))
        == typeid (salhelper::ConditionWaiter const *));
    ASSERT_TRUE(
        typeid (const_cast< salhelper::ConditionWaiter volatile * >(p))
        == typeid (salhelper::ConditionWaiter volatile *));
    ASSERT_TRUE(
        typeid (salhelper::ConditionWaiter *) == getConditionWaiterTypeInfo());
}

TEST_F(Test, testConditionWaiterTimedout) {
    salhelper::ConditionWaiter::timedout x;
    ASSERT_TRUE(typeid (x) == typeid (salhelper::ConditionWaiter::timedout));
    ASSERT_TRUE(
        typeid (&x) == typeid (salhelper::ConditionWaiter::timedout *));
    ASSERT_TRUE(
        typeid (const_cast< salhelper::ConditionWaiter::timedout const * >(&x))
        == typeid (salhelper::ConditionWaiter::timedout const *));
    ASSERT_TRUE(
        (typeid
         (const_cast< salhelper::ConditionWaiter::timedout volatile * >(&x)))
        == typeid (salhelper::ConditionWaiter::timedout volatile *));
    try {
        throw salhelper::ConditionWaiter::timedout();
    } catch (salhelper::ConditionWaiter::timedout &) {
    } catch (...) {
        FAIL() << "not caught";
    }
}

TEST_F(Test, testORealDynamicLoader) {
    salhelper::ORealDynamicLoader * p = 0;
    ASSERT_TRUE(typeid (p) != typeid (salhelper::ORealDynamicLoader));
    ASSERT_TRUE(typeid (p) == typeid (salhelper::ORealDynamicLoader *));
    ASSERT_TRUE(
        typeid (const_cast< salhelper::ORealDynamicLoader const * >(p))
        == typeid (salhelper::ORealDynamicLoader const *));
    ASSERT_TRUE(
        typeid (const_cast< salhelper::ORealDynamicLoader volatile * >(p))
        == typeid (salhelper::ORealDynamicLoader volatile *));
    ASSERT_TRUE(
        typeid (salhelper::ORealDynamicLoader *)
        == getORealDynamicLoaderTypeInfo());
}

TEST_F(Test, testSimpleReferenceObject) {
    salhelper::SimpleReferenceObject * p = new DerivedSimpleReferenceObject;
    try {
        ASSERT_TRUE(
            typeid (*p) != typeid (salhelper::SimpleReferenceObject));
        ASSERT_TRUE(
            typeid (p) == typeid (salhelper::SimpleReferenceObject *));
        ASSERT_TRUE(
            typeid (const_cast< salhelper::SimpleReferenceObject const * >(p))
            == typeid (salhelper::SimpleReferenceObject const *));
        ASSERT_TRUE(
            (typeid
             (const_cast< salhelper::SimpleReferenceObject volatile * >(p)))
            == typeid (salhelper::SimpleReferenceObject volatile *));
        ASSERT_TRUE(
            typeid (salhelper::SimpleReferenceObject *)
            == getSimpleReferenceObjectTypeInfo());
    } catch (...) {
        delete static_cast< DerivedSimpleReferenceObject * >(p);
        throw;
    }
}

TEST_F(Test, testDerivedCondition) {
    osl::Mutex mutex;
    std::auto_ptr< salhelper::Condition > p(new DerivedCondition(mutex));
    ASSERT_TRUE(dynamic_cast< DerivedCondition * >(p.get()) != 0);
}

TEST_F(Test, testDerivedConditionWaiterTimedout) {
    std::auto_ptr< salhelper::ConditionWaiter::timedout > p(
        new DerivedConditionWaiterTimedout);
    ASSERT_TRUE(
        dynamic_cast< DerivedConditionWaiterTimedout * >(p.get()) != 0);
    try {
        throw DerivedConditionWaiterTimedout();
    } catch (salhelper::ConditionWaiter::timedout &) {
    } catch (...) {
        FAIL() << "not caught";
    }
}

TEST_F(Test, testDerivedSimpleReferenceObject) {
    salhelper::SimpleReferenceObject * p = new DerivedSimpleReferenceObject;
    try {
        ASSERT_TRUE(dynamic_cast< DerivedSimpleReferenceObject * >(p) != 0);
    } catch (...) {
        delete static_cast< DerivedSimpleReferenceObject * >(p);
        throw;
    }
}


}
