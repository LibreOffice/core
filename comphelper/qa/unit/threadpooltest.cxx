/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/threadpool.hxx>
#include <sal/log.hxx>

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"

#include <chrono>
#include <cstdlib>
#include <ctime>
#include <string>
#include <thread>

class ThreadPoolTest : public CppUnit::TestFixture
{
public:
    void testPreferredConcurrency();
    void testSeparateTasks();

    CPPUNIT_TEST_SUITE(ThreadPoolTest);
    CPPUNIT_TEST(testPreferredConcurrency);
    CPPUNIT_TEST(testSeparateTasks);
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
                           nThreads <= (sal_Int32)std::thread::hardware_concurrency());

    // Revert and check. Again, nothing should change.
    unsetenv("MAX_CONCURRENCY");
    nThreads = comphelper::ThreadPool::getPreferredConcurrency();
    CPPUNIT_ASSERT_MESSAGE("Expected no more than 4 threads", nExpected >= nThreads);
#endif
}

class WaitForFlagOrTime : public comphelper::ThreadTask
{
private:
    std::string msId;
    std::time_t mnWaitTime;
    bool& mrStop;

public:
    WaitForFlagOrTime(std::shared_ptr<comphelper::ThreadTaskTag>& rTag,
                      std::string sId,
                      std::time_t nWaitTime,
                      bool& rStop) :
        comphelper::ThreadTask(rTag),
        msId(sId),
        mnWaitTime(nWaitTime),
        mrStop(rStop)
    {
    }

    virtual void doWork() override
    {
        std::time_t nStartTime = std::time(nullptr);
        SAL_WARN("comphelper", "Starting task " << msId);
        while (!mrStop &&
               std::time(nullptr) < nStartTime + mnWaitTime)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        SAL_WARN("comphelper", "Finishing task " << msId << ", took " << (std::time(nullptr) - nStartTime) << " s");
    }
};

void ThreadPoolTest::testSeparateTasks()
{
    comphelper::ThreadPool& rThreadPool(comphelper::ThreadPool::getSharedOptimalPool());

    // Start a task A that will run for a specific "long" time

    bool bStopA = false;
    std::shared_ptr<comphelper::ThreadTaskTag> aTagA = comphelper::ThreadPool::createThreadTaskTag();
    rThreadPool.pushTask(new WaitForFlagOrTime(aTagA, "A", 10, bStopA));

    // Get the current wallclock time, and start a task B that will run for a specific clearly shorter time

    std::time_t nStartTimeOfB = std::time(nullptr);
    bool bStopB = false;
    std::shared_ptr<comphelper::ThreadTaskTag> aTagB = comphelper::ThreadPool::createThreadTaskTag();
    rThreadPool.pushTask(new WaitForFlagOrTime(aTagB, "B", 1, bStopB));

    // Wait for task B

    rThreadPool.waitUntilDone(aTagB);

    // Get the current time, and verify that we are only at the time task B should have taken

    std::time_t nEndTimeOfB = std::time(nullptr);

    SAL_WARN("comphelper", "Starting and waiting for task B took " << (nEndTimeOfB - nStartTimeOfB) << " s");

    CPPUNIT_ASSERT_MESSAGE("Waiting for task B apparently waited for task A, too", (nEndTimeOfB - nStartTimeOfB) < 5);

    // Tell task A to stop and wait for it
    bStopA = true;
    rThreadPool.waitUntilDone(aTagA);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ThreadPoolTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
