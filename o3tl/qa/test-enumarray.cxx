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

#include <o3tl/enumarray.hxx>

namespace {

class Test: public CppUnit::TestFixture {
private:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testOperations);
    CPPUNIT_TEST_SUITE_END();


    void testOperations() {
        enum class MyEnum { ONE, TWO, THREE, LAST = THREE };
        o3tl::enumarray<MyEnum, int> aModules;

        aModules[MyEnum::ONE] = 1;
        aModules[MyEnum::TWO] = 1;
        aModules[MyEnum::THREE] = 1;

        o3tl::enumarray<MyEnum, int> const & rModules = aModules;

        for (auto const & i : rModules)
            CPPUNIT_ASSERT_EQUAL( 1, i );
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
