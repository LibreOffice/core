/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2011 Oracle and/or its affiliates.
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

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"

#include "../source/cache.hxx"

namespace {

class Test: public CppUnit::TestFixture {
private:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testNothingLostFromLruList);
    CPPUNIT_TEST_SUITE_END();

    void testNothingLostFromLruList();
};

// cf. jurt/test/com/sun/star/lib/uno/protocols/urp/Cache_Test.java:
void Test::testNothingLostFromLruList() {
    int a[8];
    for (int i = 0; i != sizeof a / sizeof a[0]; ++i) {
        for (int j = 0; j != i; ++j) {
            a[j] = 0;
        }
        for (;;) {
            binaryurp::Cache< int > c(4);
            for (int k = 0; k != i; ++k) {
                bool f;
                c.add(a[k], &f);
            }
            bool f;
            CPPUNIT_ASSERT_EQUAL(
                6,
                c.add(-1, &f) + c.add(-2, &f) + c.add(-3, &f) + c.add(-4, &f));
            int j = i - 1;
            while (j >= 0 && a[j] == 3) {
                --j;
            }
            if (j < 0) {
                break;
            }
            ++a[j];
            for (int k = j + 1; k != i; ++k) {
                a[k] = 0;
            }
        }
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();
