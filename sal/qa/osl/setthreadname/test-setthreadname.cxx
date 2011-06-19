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
#include <sal/cppunit.h>

#include <cstdlib>
#include <iostream>
#include <limits>

#include "boost/noncopyable.hpp"
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
