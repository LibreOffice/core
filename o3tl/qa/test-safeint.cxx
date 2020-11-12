/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <limits>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <o3tl/safeint.hxx>

namespace
{
class Test : public CppUnit::TestFixture
{
private:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testSignedSaturatingAdd);
    CPPUNIT_TEST(testUnsignedSaturatingAdd);
    CPPUNIT_TEST_SUITE_END();

    void testSignedSaturatingAdd()
    {
        auto const min = std::numeric_limits<int>::min();
        auto const max = std::numeric_limits<int>::max();

        CPPUNIT_ASSERT_EQUAL(min, o3tl::saturating_add(min, min));
        CPPUNIT_ASSERT_EQUAL(min, o3tl::saturating_add(min, min + 1));
        CPPUNIT_ASSERT_EQUAL(min, o3tl::saturating_add(min, -1));
        CPPUNIT_ASSERT_EQUAL(min, o3tl::saturating_add(min, 0));
        CPPUNIT_ASSERT_EQUAL(min + 1, o3tl::saturating_add(min, 1));
        CPPUNIT_ASSERT_EQUAL(min + max - 1, o3tl::saturating_add(min, max - 1));
        CPPUNIT_ASSERT_EQUAL(min + max, o3tl::saturating_add(min, max));

        CPPUNIT_ASSERT_EQUAL(min, o3tl::saturating_add(min + 1, min));
        CPPUNIT_ASSERT_EQUAL(min, o3tl::saturating_add(min + 1, min + 1));
        CPPUNIT_ASSERT_EQUAL(min, o3tl::saturating_add(min + 1, -1));
        CPPUNIT_ASSERT_EQUAL(min + 1, o3tl::saturating_add(min + 1, 0));
        CPPUNIT_ASSERT_EQUAL(min + 2, o3tl::saturating_add(min + 1, 1));
        CPPUNIT_ASSERT_EQUAL(min + max, o3tl::saturating_add(min + 1, max - 1));
        CPPUNIT_ASSERT_EQUAL(min + max + 1, o3tl::saturating_add(min + 1, max));

        CPPUNIT_ASSERT_EQUAL(min, o3tl::saturating_add(-1, min));
        CPPUNIT_ASSERT_EQUAL(min, o3tl::saturating_add(-1, min + 1));
        CPPUNIT_ASSERT_EQUAL(-2, o3tl::saturating_add(-1, -1));
        CPPUNIT_ASSERT_EQUAL(-1, o3tl::saturating_add(-1, 0));
        CPPUNIT_ASSERT_EQUAL(0, o3tl::saturating_add(-1, 1));
        CPPUNIT_ASSERT_EQUAL(max - 2, o3tl::saturating_add(-1, max - 1));
        CPPUNIT_ASSERT_EQUAL(max - 1, o3tl::saturating_add(-1, max));

        CPPUNIT_ASSERT_EQUAL(min, o3tl::saturating_add(0, min));
        CPPUNIT_ASSERT_EQUAL(min + 1, o3tl::saturating_add(0, min + 1));
        CPPUNIT_ASSERT_EQUAL(-1, o3tl::saturating_add(0, -1));
        CPPUNIT_ASSERT_EQUAL(0, o3tl::saturating_add(0, 0));
        CPPUNIT_ASSERT_EQUAL(1, o3tl::saturating_add(0, 1));
        CPPUNIT_ASSERT_EQUAL(max - 1, o3tl::saturating_add(0, max - 1));
        CPPUNIT_ASSERT_EQUAL(max, o3tl::saturating_add(0, max));

        CPPUNIT_ASSERT_EQUAL(min + 1, o3tl::saturating_add(1, min));
        CPPUNIT_ASSERT_EQUAL(min + 2, o3tl::saturating_add(1, min + 1));
        CPPUNIT_ASSERT_EQUAL(0, o3tl::saturating_add(1, -1));
        CPPUNIT_ASSERT_EQUAL(1, o3tl::saturating_add(1, 0));
        CPPUNIT_ASSERT_EQUAL(2, o3tl::saturating_add(1, 1));
        CPPUNIT_ASSERT_EQUAL(max, o3tl::saturating_add(1, max - 1));
        CPPUNIT_ASSERT_EQUAL(max, o3tl::saturating_add(1, max));

        CPPUNIT_ASSERT_EQUAL(min + max - 1, o3tl::saturating_add(max - 1, min));
        CPPUNIT_ASSERT_EQUAL(min + max, o3tl::saturating_add(max - 1, min + 1));
        CPPUNIT_ASSERT_EQUAL(max - 2, o3tl::saturating_add(max - 1, -1));
        CPPUNIT_ASSERT_EQUAL(max - 1, o3tl::saturating_add(max - 1, 0));
        CPPUNIT_ASSERT_EQUAL(max, o3tl::saturating_add(max - 1, 1));
        CPPUNIT_ASSERT_EQUAL(max, o3tl::saturating_add(max - 1, max - 1));
        CPPUNIT_ASSERT_EQUAL(max, o3tl::saturating_add(max - 1, max));

        CPPUNIT_ASSERT_EQUAL(min + max, o3tl::saturating_add(max, min));
        CPPUNIT_ASSERT_EQUAL(min + max + 1, o3tl::saturating_add(max, min + 1));
        CPPUNIT_ASSERT_EQUAL(max - 1, o3tl::saturating_add(max, -1));
        CPPUNIT_ASSERT_EQUAL(max, o3tl::saturating_add(max, 0));
        CPPUNIT_ASSERT_EQUAL(max, o3tl::saturating_add(max, 1));
        CPPUNIT_ASSERT_EQUAL(max, o3tl::saturating_add(max, max - 1));
        CPPUNIT_ASSERT_EQUAL(max, o3tl::saturating_add(max, max));
    }

    void testUnsignedSaturatingAdd()
    {
        auto const max = std::numeric_limits<unsigned int>::max();

        CPPUNIT_ASSERT_EQUAL(0U, o3tl::saturating_add(0U, 0U));
        CPPUNIT_ASSERT_EQUAL(1U, o3tl::saturating_add(0U, 1U));
        CPPUNIT_ASSERT_EQUAL(max - 1, o3tl::saturating_add(0U, max - 1));
        CPPUNIT_ASSERT_EQUAL(max, o3tl::saturating_add(0U, max));

        CPPUNIT_ASSERT_EQUAL(1U, o3tl::saturating_add(1U, 0U));
        CPPUNIT_ASSERT_EQUAL(2U, o3tl::saturating_add(1U, 1U));
        CPPUNIT_ASSERT_EQUAL(max, o3tl::saturating_add(1U, max - 1));
        CPPUNIT_ASSERT_EQUAL(max, o3tl::saturating_add(1U, max));

        CPPUNIT_ASSERT_EQUAL(max - 1, o3tl::saturating_add(max - 1, 0U));
        CPPUNIT_ASSERT_EQUAL(max, o3tl::saturating_add(max - 1, 1U));
        CPPUNIT_ASSERT_EQUAL(max, o3tl::saturating_add(max - 1, max - 1));
        CPPUNIT_ASSERT_EQUAL(max, o3tl::saturating_add(max - 1, max));

        CPPUNIT_ASSERT_EQUAL(max, o3tl::saturating_add(max, 0U));
        CPPUNIT_ASSERT_EQUAL(max, o3tl::saturating_add(max, 1U));
        CPPUNIT_ASSERT_EQUAL(max, o3tl::saturating_add(max, max - 1));
        CPPUNIT_ASSERT_EQUAL(max, o3tl::saturating_add(max, max));
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
