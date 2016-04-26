/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <sal/types.h>
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"

#include <o3tl/lru_map.hxx>

#include <boost/functional/hash.hpp>

using namespace ::o3tl;

class lru_map_test : public CppUnit::TestFixture
{
public:
    void testBaseUsage();
    void testReplaceKey();
    void testReplaceValue();
    void testLruRemoval();
    void testCustomHash();

    CPPUNIT_TEST_SUITE(lru_map_test);
    CPPUNIT_TEST(testBaseUsage);
    CPPUNIT_TEST(testReplaceKey);
    CPPUNIT_TEST(testReplaceValue);
    CPPUNIT_TEST(testLruRemoval);
    CPPUNIT_TEST(testCustomHash);
    CPPUNIT_TEST_SUITE_END();
};

void lru_map_test::testBaseUsage()
{
    o3tl::lru_map<int, int> lru(10);
    lru.insert(std::make_pair<int, int>(1, 1));

    std::pair<int, int> pair;
    for (int i = 2; i < 7; i++)
    {
        pair.first = pair.second = i;
        lru.insert(pair);
    }

    CPPUNIT_ASSERT_EQUAL(size_t(6), lru.size());

    o3tl::lru_map<int, int>::const_iterator it;

    it = lru.find(2);
    CPPUNIT_ASSERT(it != lru.end());
    CPPUNIT_ASSERT_EQUAL(2, it->second);

    it = lru.find(5);
    CPPUNIT_ASSERT(it != lru.end());
    CPPUNIT_ASSERT_EQUAL(5, it->second);

    it = lru.find(0);
    CPPUNIT_ASSERT(it == lru.end());
}

void lru_map_test::testReplaceValue()
{
    o3tl::lru_map<int, int> lru(2);
    // check if map is empty
    CPPUNIT_ASSERT_EQUAL(size_t(0), lru.size());

    // check if inserting entry with same key replaces the value

    // inserting new entry
    lru.insert(std::make_pair<int, int>(1, 2));
    CPPUNIT_ASSERT_EQUAL(size_t(1), lru.size());
    CPPUNIT_ASSERT_EQUAL(2, lru.find(1)->second);

    // inserting new entry with key that alreay exists
    lru.insert(std::make_pair<int, int>(1, 4));
    CPPUNIT_ASSERT_EQUAL(size_t(1), lru.size());
    CPPUNIT_ASSERT_EQUAL(4, lru.find(1)->second);

    // inserting new entry
    lru.insert(std::make_pair<int, int>(2, 200));
    CPPUNIT_ASSERT_EQUAL(size_t(2), lru.size());
    CPPUNIT_ASSERT_EQUAL(4, lru.find(1)->second);
    CPPUNIT_ASSERT_EQUAL(200, lru.find(2)->second);

    // check if insert with same key, moves the entry back of the lru queu

    // inserting new entry with key that alreay exists
    lru.insert(std::make_pair<int, int>(1, 6));
    // inserting new entry, lru removed
    lru.insert(std::make_pair<int, int>(3, 300));

    CPPUNIT_ASSERT_EQUAL(size_t(2), lru.size());
    CPPUNIT_ASSERT_EQUAL(6, lru.find(1)->second);
    CPPUNIT_ASSERT_EQUAL(300, lru.find(3)->second);

}

void lru_map_test::testReplaceKey()
{
    o3tl::lru_map<int, int> lru(2);

    // inserting new entry
    lru.insert(std::make_pair<int, int>(1, 100));
    CPPUNIT_ASSERT_EQUAL(size_t(1), lru.size());
    CPPUNIT_ASSERT_EQUAL(100, lru.find(1)->second);
    CPPUNIT_ASSERT(lru.find(2) == lru.end());
    CPPUNIT_ASSERT(lru.find(3) == lru.end());

    // inserting new entry
    lru.insert(std::make_pair<int, int>(2, 200));
    CPPUNIT_ASSERT_EQUAL(size_t(2), lru.size());
    CPPUNIT_ASSERT_EQUAL(100, lru.find(1)->second);
    CPPUNIT_ASSERT_EQUAL(200, lru.find(2)->second);
    CPPUNIT_ASSERT(lru.find(3) == lru.end());

    // inserting new entry, lru entry is removed
    lru.insert(std::make_pair<int, int>(3, 300));
    CPPUNIT_ASSERT_EQUAL(size_t(2), lru.size());
    CPPUNIT_ASSERT(lru.find(1) == lru.end());
    CPPUNIT_ASSERT_EQUAL(200, lru.find(2)->second);
    CPPUNIT_ASSERT_EQUAL(300, lru.find(3)->second);

    // inserting new entry, lru entry is removed
    std::pair<int, int> pair(4, 400);
    lru.insert(pair);
    CPPUNIT_ASSERT_EQUAL(size_t(2), lru.size());
    CPPUNIT_ASSERT(lru.find(1) == lru.end());
    CPPUNIT_ASSERT(lru.find(2) == lru.end());
    CPPUNIT_ASSERT_EQUAL(300, lru.find(3)->second);
    CPPUNIT_ASSERT_EQUAL(400, lru.find(4)->second);
}

void lru_map_test::testLruRemoval()
{
    o3tl::lru_map<int, int> lru(5);
    CPPUNIT_ASSERT_EQUAL(size_t(0), lru.size());

    // fill up..
    lru.insert(std::make_pair<int, int>(1, 100));
    lru.insert(std::make_pair<int, int>(2, 200));
    lru.insert(std::make_pair<int, int>(3, 300));
    lru.insert(std::make_pair<int, int>(4, 400));
    lru.insert(std::make_pair<int, int>(5, 500));
    CPPUNIT_ASSERT_EQUAL(size_t(5), lru.size());
    CPPUNIT_ASSERT_EQUAL(100, lru.find(1)->second);
    CPPUNIT_ASSERT_EQUAL(200, lru.find(2)->second);
    CPPUNIT_ASSERT_EQUAL(300, lru.find(3)->second);
    CPPUNIT_ASSERT_EQUAL(400, lru.find(4)->second);
    CPPUNIT_ASSERT_EQUAL(500, lru.find(5)->second);

    // add one more entry - lru entry should be removed
    lru.insert(std::make_pair<int, int>(6, 600));

    CPPUNIT_ASSERT_EQUAL(size_t(5), lru.size());
    CPPUNIT_ASSERT_EQUAL(200, lru.find(2)->second);
    CPPUNIT_ASSERT_EQUAL(300, lru.find(3)->second);
    CPPUNIT_ASSERT_EQUAL(400, lru.find(4)->second);
    CPPUNIT_ASSERT_EQUAL(500, lru.find(5)->second);
    CPPUNIT_ASSERT_EQUAL(600, lru.find(6)->second);

    // access the lru entry to put it at the back of the lru queue
    lru.find(2);
    // add new entry - lru entry should be removed
    lru.insert(std::make_pair<int, int>(7, 700));

    CPPUNIT_ASSERT_EQUAL(size_t(5), lru.size());
    CPPUNIT_ASSERT_EQUAL(200, lru.find(2)->second);
    CPPUNIT_ASSERT_EQUAL(400, lru.find(4)->second);
    CPPUNIT_ASSERT_EQUAL(500, lru.find(5)->second);
    CPPUNIT_ASSERT_EQUAL(600, lru.find(6)->second);
    CPPUNIT_ASSERT_EQUAL(700, lru.find(7)->second);
}

struct TestClassKey
{
    int mA;
    int mB;

    TestClassKey(int a, int b)
        : mA(a)
        , mB(b)
    {}

    bool operator==(TestClassKey const& aOther) const
    {
        return mA == aOther.mA
            && mB == aOther.mB;
    }
};

struct TestClassKeyHashFunction
{
    std::size_t operator()(TestClassKey const& aKey) const
    {
        std::size_t seed = 0;
        boost::hash_combine(seed, aKey.mA);
        boost::hash_combine(seed, aKey.mB);
        return seed;
    }
};

void lru_map_test::testCustomHash()
{
    // check lru_map with custom hash function
    o3tl::lru_map<TestClassKey, int, TestClassKeyHashFunction> lru(2);
    CPPUNIT_ASSERT_EQUAL(size_t(0), lru.size());

    lru.insert(std::make_pair<TestClassKey, int>(TestClassKey(1,1), 2));
    CPPUNIT_ASSERT_EQUAL(size_t(1), lru.size());

    lru.insert(std::make_pair<TestClassKey, int>(TestClassKey(1,1), 7));
    CPPUNIT_ASSERT_EQUAL(size_t(1), lru.size());

    lru.insert(std::make_pair<TestClassKey, int>(TestClassKey(1,2), 9));
    CPPUNIT_ASSERT_EQUAL(size_t(2), lru.size());

    CPPUNIT_ASSERT(lru.end() == lru.find(TestClassKey(0,0))); // non existent
    CPPUNIT_ASSERT_EQUAL(7, lru.find(TestClassKey(1,1))->second);
    CPPUNIT_ASSERT_EQUAL(9, lru.find(TestClassKey(1,2))->second);

    lru.insert(std::make_pair<TestClassKey, int>(TestClassKey(2,1), 13));

    CPPUNIT_ASSERT_EQUAL(size_t(2), lru.size());

    CPPUNIT_ASSERT(lru.end() == lru.find(TestClassKey(1,1)));
    CPPUNIT_ASSERT_EQUAL(9,  lru.find(TestClassKey(1,2))->second);
    CPPUNIT_ASSERT_EQUAL(13, lru.find(TestClassKey(2,1))->second);
}

CPPUNIT_TEST_SUITE_REGISTRATION(lru_map_test);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
