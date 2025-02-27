/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <o3tl/hash_combine.hxx>

namespace
{
struct TestStruct
{
};

class HashCombineTest : public CppUnit::TestFixture
{
public:
    void testBaseUsage()
    {
        TestStruct objectA;
        TestStruct objectB;

        {
            sal_uInt64 seed(0);
            o3tl::hash_combine(seed, OUString("Hmm-.."));
            o3tl::hash_combine(seed, 24);
            o3tl::hash_combine(seed, 42u);
            o3tl::hash_combine(seed, 5.1);
            o3tl::hash_combine(seed, &objectA);
            o3tl::hash_combine(seed, &objectB);
            CPPUNIT_ASSERT(seed != sal_uInt64(0));

            static_assert(sizeof(seed) == 8);
        }

        {
            sal_uInt32 seed(0);
            o3tl::hash_combine(seed, OUString("Hmm-.."));
            o3tl::hash_combine(seed, 24);
            o3tl::hash_combine(seed, 42u);
            o3tl::hash_combine(seed, 5.1);
            o3tl::hash_combine(seed, &objectA);
            o3tl::hash_combine(seed, &objectB);
            CPPUNIT_ASSERT(seed != sal_uInt32(0));

            static_assert(sizeof(seed) == 4);
        }

        // No assert of the actual value - we just check this compiles and runs. Hash algorithm is probably
        // also implementation specific.

        // Using sal_uInt16 for seed doesn't compile as there is not compatible hash_combine override for seed that is 16 bit.
    }

    CPPUNIT_TEST_SUITE(HashCombineTest);
    CPPUNIT_TEST(testBaseUsage);
    CPPUNIT_TEST_SUITE_END();
};
}

CPPUNIT_TEST_SUITE_REGISTRATION(HashCombineTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
