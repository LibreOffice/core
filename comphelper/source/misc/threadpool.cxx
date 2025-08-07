/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/threadpool.hxx>

#include <com/sun/star/uno/Exception.hpp>
#include <config_options.h>
#include <o3tl/safeint.hxx>
#include <sal/config.h>
#include <sal/log.hxx>
#include <salhelper/thread.hxx>
#include <algorithm>
#include <memory>
#include <thread>
#include <chrono>
#include <cstddef>
#include <comphelper/debuggerinfo.hxx>
#include <utility>

#if defined HAVE_VALGRIND_HEADERS
#include <valgrind/memcheck.h>
#endif

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace comphelper {

/** prevent waiting for a task from inside a task */
#if defined DBG_UTIL && (defined LINUX || defined _WIN32)
static thread_local bool gbIsWorkerThread;
#endif

// used to group thread-tasks for waiting in waitTillDone()
class ThreadTaskTag
{
    std::mutex maMutex;
    sal_Int32 mnTasksWorking;
    std::condition_variable maTasksComplete;

public:
    ThreadTaskTag();
    bool isDone();
    void waitUntilDone();
    void onTaskWorkerDone();
    void onTaskPushed();
};


class ThreadPool::ThreadWorker : public salhelper::Thread
{
    ThreadPool *mpPool;
public:

    explicit ThreadWorker( ThreadPool *pPool ) :
        salhelper::Thread("thread-pool"),
        mpPool( pPool )
    {
    }

    virtual void execute() override
    {
#if defined DBG_UTIL && (defined LINUX || defined _WIN32)
        gbIsWorkerThread = true;
#endif
        std::unique_lock< std::mutex > aGuard( mpPool->maMutex );

        while( !mpPool->mbTerminate )
        {
            std::unique_ptr<ThreadTask> pTask = mpPool->popWorkLocked( aGuard, true );
            if( pTask )
            {
                std::shared_ptr<ThreadTaskTag> pTag(pTask->mpTag);
                mpPool->incBusyWorker();
                aGuard.unlock();

                pTask->exec();
                pTask.reset();

                aGuard.lock();
                mpPool->decBusyWorker();
                pTag->onTaskWorkerDone();
            }
        }
    }
};

ThreadPool::ThreadPool(std::size_t nWorkers)
    : mbTerminate(true)
    , mnMaxWorkers(nWorkers)
    , mnBusyWorkers(0)
{
}

ThreadPool::~ThreadPool()
{
    // note: calling shutdown from global variable dtor blocks forever on Win7
    // note2: there isn't enough MSVCRT left on exit to call assert() properly
    // so these asserts just print something to stderr but exit status is
    // still 0, but hopefully they will be more helpful on non-WNT platforms
    assert(mbTerminate);
    assert(maTasks.empty());
    assert(mnBusyWorkers == 0);
}

namespace {

std::shared_ptr< ThreadPool >& GetStaticThreadPool()
{
    static std::shared_ptr< ThreadPool > POOL =
    []()
    {
        const std::size_t nThreads = ThreadPool::getPreferredConcurrency();
        return std::make_shared< ThreadPool >( nThreads );
    }();
    return POOL;
}

}

ThreadPool& ThreadPool::getSharedOptimalPool()
{
    return *GetStaticThreadPool();
}

std::size_t ThreadPool::getPreferredConcurrency()
{
    static std::size_t ThreadCount = []()
    {
        const std::size_t nHardThreads = o3tl::clamp_to_unsigned<std::size_t>(
            std::max(std::thread::hardware_concurrency(), 1U));
        std::size_t nThreads = nHardThreads;
        const char *pEnv = getenv("MAX_CONCURRENCY");
        if (pEnv != nullptr)
        {
            // Override with user/admin preference.
            nThreads = o3tl::clamp_to_unsigned<std::size_t>(rtl_str_toInt32(pEnv, 10));
        }

        nThreads = std::min(nHardThreads, nThreads);
        return std::max<std::size_t>(nThreads, 1);
    }();

    return ThreadCount;
}

// Used to order shutdown, and to ensure there are no lingering
// threads after LibreOfficeKit pre-init.
void ThreadPool::shutdown()
{
//    if (mbTerminate)
//        return;

    std::unique_lock< std::mutex > aGuard( maMutex );
    shutdownLocked(aGuard);
}

void ThreadPool::shutdownLocked(std::unique_lock<std::mutex>& aGuard)
{
    if( maWorkers.empty() )
    { // no threads at all -> execute the work in-line
        std::unique_ptr<ThreadTask> pTask;
        while ( ( pTask = popWorkLocked(aGuard, false) ) )
        {
            std::shared_ptr<ThreadTaskTag> pTag(pTask->mpTag);
            pTask->exec();
            pTag->onTaskWorkerDone();
        }
    }
    else
    {
        while( !maTasks.empty() )
        {
            maTasksChanged.wait( aGuard );
            // In the (unlikely but possible?) case pushTask() gets called meanwhile,
            // its notify_one() call is meant to wake a up a thread and process the task.
            // But if this code gets woken up instead, it could lead to a deadlock.
            // Pass on the notification.
            maTasksChanged.notify_one();
        }
    }
    assert( maTasks.empty() );

    mbTerminate = true;

    maTasksChanged.notify_all();

    decltype(maWorkers) aWorkers;
    std::swap(maWorkers, aWorkers);
    aGuard.unlock();

    while (!aWorkers.empty())
    {
        rtl::Reference<ThreadWorker> xWorker = aWorkers.back();
        aWorkers.pop_back();
        assert(std::find(aWorkers.begin(), aWorkers.end(), xWorker)
                == aWorkers.end());
        {
            xWorker->join();
            xWorker.clear();
        }
    }
}

void ThreadPool::pushTask( std::unique_ptr<ThreadTask> pTask )
{
    std::scoped_lock< std::mutex > aGuard( maMutex );

    mbTerminate = false;

    // Worked on tasks are already removed from maTasks, so include the count of busy workers.
    if (maWorkers.size() < mnMaxWorkers && maWorkers.size() <= maTasks.size() + mnBusyWorkers)
    {
        maWorkers.push_back( new ThreadWorker( this ) );
        maWorkers.back()->launch();
    }

    pTask->mpTag->onTaskPushed();
    maTasks.insert( maTasks.begin(), std::move(pTask) );

    maTasksChanged.notify_one();
}

std::unique_ptr<ThreadTask> ThreadPool::popWorkLocked( std::unique_lock< std::mutex > & rGuard, bool bWait )
{
    do
    {
        if( !maTasks.empty() )
        {
            std::unique_ptr<ThreadTask> pTask = std::move(maTasks.back());
            maTasks.pop_back();
            return pTask;
        }
        else if (!bWait || mbTerminate)
            return nullptr;

        maTasksChanged.wait( rGuard );

    } while (!mbTerminate);

    return nullptr;
}

void ThreadPool::incBusyWorker()
{
    ++mnBusyWorkers;
}

void ThreadPool::decBusyWorker()
{
    assert(mnBusyWorkers >= 1);
    --mnBusyWorkers;
}

void ThreadPool::waitUntilDone(const std::shared_ptr<ThreadTaskTag>& rTag, bool bJoin)
{
#if defined DBG_UTIL && (defined LINUX || defined _WIN32)
    assert(!gbIsWorkerThread && "cannot wait for tasks from inside a task");
#endif
    {
        std::unique_lock< std::mutex > aGuard( maMutex );

        if( maWorkers.empty() )
        { // no threads at all -> execute the work in-line
            while (!rTag->isDone())
            {
                std::unique_ptr<ThreadTask> pTask = popWorkLocked(aGuard, false);
                if (!pTask)
                    break;
                std::shared_ptr<ThreadTaskTag> pTag(pTask->mpTag);
                pTask->exec();
                pTag->onTaskWorkerDone();
            }
        }
    }

    rTag->waitUntilDone();

    if (bJoin)
        joinThreadsIfIdle();
}

bool ThreadPool::joinThreadsIfIdle()
{
    std::unique_lock< std::mutex > aGuard( maMutex );
    if (isIdle()) // check if there are still tasks from another tag
    {
        shutdownLocked(aGuard);
        return true;
    }
    return false;
}

std::shared_ptr<ThreadTaskTag> ThreadPool::createThreadTaskTag()
{
    return std::make_shared<ThreadTaskTag>();
}

bool ThreadPool::isTaskTagDone(const std::shared_ptr<ThreadTaskTag>& pTag)
{
    return pTag->isDone();
}

ThreadTask::ThreadTask(std::shared_ptr<ThreadTaskTag> xTag)
    : mpTag(std::move(xTag))
{
}

void ThreadTask::exec()
{
    try {
        doWork();
    }
    catch (const std::exception &e)
    {
        SAL_WARN("comphelper", "exception in thread worker while calling doWork(): " << e.what());
    }
    catch (const css::uno::Exception &e)
    {
        SAL_WARN("comphelper", "exception in thread worker while calling doWork(): " << e);
    }
    catch (...)
    {
        SAL_WARN("comphelper", "unknown exception in thread worker while calling doWork()");
    }
}

ThreadTaskTag::ThreadTaskTag() : mnTasksWorking(0)
{
}

void ThreadTaskTag::onTaskPushed()
{
    std::scoped_lock< std::mutex > aGuard( maMutex );
    mnTasksWorking++;
    assert( mnTasksWorking < 65536 ); // sanity checking
}

void ThreadTaskTag::onTaskWorkerDone()
{
    std::scoped_lock< std::mutex > aGuard( maMutex );
    mnTasksWorking--;
    assert(mnTasksWorking >= 0);
    if (mnTasksWorking == 0)
        maTasksComplete.notify_all();
}

bool ThreadTaskTag::isDone()
{
    std::scoped_lock< std::mutex > aGuard( maMutex );
    return mnTasksWorking == 0;
}

void ThreadTaskTag::waitUntilDone()
{
    std::unique_lock< std::mutex > aGuard( maMutex );
    while( mnTasksWorking > 0 )
    {
#if defined DBG_UTIL && !defined NDEBUG
        // 10 minute timeout in debug mode, unless the code is built with
        // sanitizers or debugged in valgrind or gdb, in which case the threads
        // should not time out in the middle of a debugging session
        int maxTimeout = 10 * 60;
#if !ENABLE_RUNTIME_OPTIMIZATIONS
        maxTimeout = 30 * 60;
#endif
#if defined HAVE_VALGRIND_HEADERS
        if( RUNNING_ON_VALGRIND )
            maxTimeout = 30 * 60;
#endif
        if( isDebuggerAttached())
            maxTimeout = 300 * 60;
        std::cv_status result = maTasksComplete.wait_for(
            aGuard, std::chrono::seconds( maxTimeout ));
        assert(result != std::cv_status::timeout);
#else
        // 10 minute timeout in production so the app eventually throws some kind of error
        if (maTasksComplete.wait_for(
                aGuard, std::chrono::seconds( 10 * 60 )) == std::cv_status::timeout)
            throw std::runtime_error("timeout waiting for threadpool tasks");
#endif
    }
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
