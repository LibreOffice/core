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

#include <sal/types.h>
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
