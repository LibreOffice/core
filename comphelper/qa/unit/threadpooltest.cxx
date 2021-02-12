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
#include <tools/time.hxx>
#include <osl/thread.hxx>

#include <stdlib.h>
#include <thread>
#include <mutex>

class ThreadPoolTest : public CppUnit::TestFixture
{
public:
    void testPreferredConcurrency();
    void testWorkerUsage();
    void testTasksInThreads();
    void testNoThreads();
    void testDedicatedPool();

    CPPUNIT_TEST_SUITE(ThreadPoolTest);
    CPPUNIT_TEST(testPreferredConcurrency);
    CPPUNIT_TEST(testWorkerUsage);
    CPPUNIT_TEST(testTasksInThreads);
    CPPUNIT_TEST(testNoThreads);
    CPPUNIT_TEST(testDedicatedPool);
    CPPUNIT_TEST_SUITE_END();
};

void ThreadPoolTest::testPreferredConcurrency()
{
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

namespace
{
class UsageTask : public comphelper::ThreadTask
{
public:
    UsageTask(const std::shared_ptr<comphelper::ThreadTaskTag>& pTag)
        : ThreadTask(pTag)
    {
    }
    virtual void doWork()
    {
        ++count;
        mutex.lock();
        mutex.unlock();
    }
    static inline int count = 0;
    static inline std::mutex mutex;
};
} // namespace

void ThreadPoolTest::testWorkerUsage()
{
    // Create tasks for each available worker. Lock a shared mutex before that to make all
    // tasks block on it. And check that all workers have started, i.e. that the full
    // thread pool capacity is used.
    comphelper::ThreadPool& rSharedPool = comphelper::ThreadPool::getSharedOptimalPool();
    std::shared_ptr<comphelper::ThreadTaskTag> pTag = comphelper::ThreadPool::createThreadTaskTag();
    UsageTask::mutex.lock();
    for (int i = 0; i < rSharedPool.getWorkerCount(); ++i)
    {
        rSharedPool.pushTask(std::make_unique<UsageTask>(pTag));
        osl::Thread::wait(std::chrono::milliseconds(10)); // give it a time to start
    }
    sal_uInt64 startTicks = tools::Time::GetSystemTicks();
    while (UsageTask::count != rSharedPool.getWorkerCount())
    {
        // Wait at most 5 seconds, that should do even on slow systems.
        CPPUNIT_ASSERT_MESSAGE("Thread pool does not use all worker threads.",
                               startTicks + 5000 > tools::Time::GetSystemTicks());
        osl::Thread::wait(std::chrono::milliseconds(10));
    }
    UsageTask::mutex.unlock();
    rSharedPool.waitUntilDone(pTag);
}

namespace
{
class CheckThreadTask : public comphelper::ThreadTask
{
    oslThreadIdentifier mThreadId;
    bool mCheckEqual;

public:
    CheckThreadTask(oslThreadIdentifier threadId, bool checkEqual,
                    const std::shared_ptr<comphelper::ThreadTaskTag>& pTag)
        : ThreadTask(pTag)
        , mThreadId(threadId)
        , mCheckEqual(checkEqual)
    {
    }
    virtual void doWork()
    {
        CPPUNIT_ASSERT(mCheckEqual ? osl::Thread::getCurrentIdentifier() == mThreadId
                                   : osl::Thread::getCurrentIdentifier() != mThreadId);
    }
};
} // namespace

void ThreadPoolTest::testTasksInThreads()
{
    // Check that all tasks are run in worker threads, not this thread.
    comphelper::ThreadPool& pool = comphelper::ThreadPool::getSharedOptimalPool();
    std::shared_ptr<comphelper::ThreadTaskTag> pTag = comphelper::ThreadPool::createThreadTaskTag();
    for (int i = 0; i < 8; ++i)
        pool.pushTask(
            std::make_unique<CheckThreadTask>(osl::Thread::getCurrentIdentifier(), false, pTag));
    pool.waitUntilDone(pTag);
}

void ThreadPoolTest::testNoThreads()
{
    // No worker threads, tasks will be run in this thread.
    comphelper::ThreadPool pool(0);
    std::shared_ptr<comphelper::ThreadTaskTag> pTag = comphelper::ThreadPool::createThreadTaskTag();
    for (int i = 0; i < 8; ++i)
        pool.pushTask(
            std::make_unique<CheckThreadTask>(osl::Thread::getCurrentIdentifier(), true, pTag));
    pool.waitUntilDone(pTag);
}

void ThreadPoolTest::testDedicatedPool()
{
    // Test that a separate thread pool works. The tasks themselves do not matter.
    comphelper::ThreadPool pool(4);
    std::shared_ptr<comphelper::ThreadTaskTag> pTag = comphelper::ThreadPool::createThreadTaskTag();
    for (int i = 0; i < 8; ++i)
        pool.pushTask(
            std::make_unique<CheckThreadTask>(osl::Thread::getCurrentIdentifier(), false, pTag));
    pool.waitUntilDone(pTag);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ThreadPoolTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
