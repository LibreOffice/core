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

#include <cstdlib>
#include <iostream>
#include <limits>

#include <sal/types.h>
#include "boost/noncopyable.hpp"
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"
#include "osl/thread.hxx"

namespace {

class TestThread: public osl::Thread, private boost::noncopyable {
private:
    virtual void SAL_CALL run() override;
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
    if (std::getenv("URE_TEST_SETTHREADNAME") != nullptr) {
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
