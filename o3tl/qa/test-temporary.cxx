/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <o3tl/temporary.hxx>

namespace
{
void modify(int& n) { n = 1; }

class Test : public CppUnit::TestFixture
{
private:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testLvalue);
    CPPUNIT_TEST_SUITE_END();

    void testLvalue()
    {
        {
            int n = 0;
            modify(n);
            CPPUNIT_ASSERT_EQUAL(1, n);
        }
        {
            int n = 0;
            modify(o3tl::temporary(int(n)));
            CPPUNIT_ASSERT_EQUAL(0, n);
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
