/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <stdexcept>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <o3tl/array_view.hxx>

namespace {

class Test: public CppUnit::TestFixture {
private:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testOperations);
    CPPUNIT_TEST_SUITE_END();


    void testOperations() {
        int const some_data[] { 1, 2, 3 };
        o3tl::array_view<int> v(some_data);

        CPPUNIT_ASSERT_EQUAL(1, *v.begin());
        CPPUNIT_ASSERT_EQUAL(
            o3tl::array_view<int>::difference_type(3), v.end() - v.begin());
        CPPUNIT_ASSERT_EQUAL(1, *v.cbegin());
        CPPUNIT_ASSERT_EQUAL(
            o3tl::array_view<int>::difference_type(3), v.cend() - v.cbegin());
        CPPUNIT_ASSERT_EQUAL(3, *v.rbegin());
        CPPUNIT_ASSERT_EQUAL(
            o3tl::array_view<int>::difference_type(3), v.rend() - v.rbegin());
        CPPUNIT_ASSERT_EQUAL(3, *v.crbegin());
        CPPUNIT_ASSERT_EQUAL(
            o3tl::array_view<int>::difference_type(3), v.crend() - v.crbegin());
        CPPUNIT_ASSERT_EQUAL(o3tl::array_view<int>::size_type(3), v.size());
        CPPUNIT_ASSERT_EQUAL(o3tl::array_view<int>::size_type(3), v.length());
        CPPUNIT_ASSERT_EQUAL(o3tl::array_view<int>::npos - 1, v.max_size());
        CPPUNIT_ASSERT(!v.empty());
        CPPUNIT_ASSERT_EQUAL(2, v[1]);
        try {
            v.at(o3tl::array_view<int>::npos);
            CPPUNIT_FAIL("missing exception");
        } catch (std::out_of_range &) {}
        CPPUNIT_ASSERT_EQUAL(1, v.at(0));
        CPPUNIT_ASSERT_EQUAL(3, v.at(2));
        try {
            v.at(3);
            CPPUNIT_FAIL("missing exception");
        } catch (std::out_of_range &) {}
        CPPUNIT_ASSERT_EQUAL(1, v.front());
        CPPUNIT_ASSERT_EQUAL(3, v.back());
        CPPUNIT_ASSERT_EQUAL(1, *v.data());
        {
            int const d1[] { 1, 2 };
            int const d2[] { 3, 4, 5, 6 };
            o3tl::array_view<int> v1( d1 );
            o3tl::array_view<int> v2( d2 );
            v1.swap(v2);
            CPPUNIT_ASSERT_EQUAL(o3tl::array_view<int>::size_type(4), v1.size());
            CPPUNIT_ASSERT_EQUAL(o3tl::array_view<int>::size_type(2), v2.size());
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
