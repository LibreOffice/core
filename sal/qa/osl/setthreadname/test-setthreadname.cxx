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



#include "precompiled_sal.hxx"
#include "sal/config.h"

#include <cstdlib>
#include <iostream>
#include <limits>

#include "boost/noncopyable.hpp"
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include "osl/thread.hxx"

namespace {

class TestThread: public osl::Thread, private boost::noncopyable {
private:
    virtual void SAL_CALL run();
};

void TestThread::run() {
#if defined WNT
    if (std::getenv("URE_TEST_SETTHREADNAME") != 0) {
        // On Windows, setting thread names appears to only take effect when the
        // process is being debugged, so attach a debugger now:
        std::cout << "set: ";
        std::cin.ignore(std::numeric_limits< int >::max(), '\n');
    }
#endif
    setName("TestThread");
    if (std::getenv("URE_TEST_SETTHREADNAME") != 0) {
        // On Linux, the thread name can now be observed with "ps -L"; on
        // Windows with the Microsoft compiler, the thread name can now be
        // observed in a debugger.
        std::cout << "stop: ";
        std::cin.ignore(std::numeric_limits< int >::max(), '\n');
    }
}

class Test: public CppUnit::TestFixture {
private:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();

    void test();
};

void Test::test() {
    TestThread t;
    t.create();
    t.join();
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();
