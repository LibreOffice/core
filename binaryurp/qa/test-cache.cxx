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
