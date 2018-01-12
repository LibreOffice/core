/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/threadpool.hxx>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <stdlib.h>
#include <thread>

class ThreadPoolTest : public CppUnit::TestFixture
{
public:
    void testPreferredConcurrency();

    CPPUNIT_TEST_SUITE(ThreadPoolTest);
    CPPUNIT_TEST(testPreferredConcurrency);
    CPPUNIT_TEST_SUITE_END();
};

void ThreadPoolTest::testPreferredConcurrency() {

    // Check default.
    auto nThreads = comphelper::ThreadPool::getPreferredConcurrency();
    sal_Int32 nExpected = 4; // UTs are capped to 4.
    CPPUNIT_ASSERT_MESSAGE("Expected no more than 4 threads", nExpected >= nThreads);

#ifndef _WIN32
    // The result should be cached, so this should change anything.
    nThreads = std::thread::hardware_concurrency() * 2;
    setenv("MAX_CONCURRENCY", std::to_string(nThreads).c_str(), true);
    nThreads = comphelper::ThreadPool::getPreferredConcurrency();
    CPPUNIT_ASSERT_MESSAGE("Expected no more than hardware threads",
                           nThreads <= static_cast<sal_Int32>(std::thread::hardware_concurrency()));

    // Revert and check. Again, nothing should change.
    unsetenv("MAX_CONCURRENCY");
    nThreads = comphelper::ThreadPool::getPreferredConcurrency();
    CPPUNIT_ASSERT_MESSAGE("Expected no more than 4 threads", nExpected >= nThreads);
#endif
}

CPPUNIT_TEST_SUITE_REGISTRATION(ThreadPoolTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
