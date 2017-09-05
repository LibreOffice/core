/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/threadpool.hxx>

#include <osl/thread.hxx>
#include <sal/config.h>
#include <salhelper/simplereferenceobject.hxx>
#include <rtl/instance.hxx>
#include <rtl/string.hxx>
#include <algorithm>
#include <memory>
#include <thread>
#include <chrono>

namespace sal {

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


class ThreadPool::ThreadWorker : public osl::Thread
{
    ThreadPool *mpPool;
    oslInterlockedCount m_nCount;
public:

    explicit ThreadWorker( ThreadPool *pPool ) :
        mpPool( pPool ), m_nCount(0)
    {
    }

    void acquire()
    { osl_atomic_increment(&m_nCount); }

    void release()
    { if (osl_atomic_decrement(&m_nCount) == 0) delete this; }

    void launch() {
        // Assumption is that osl::Thread::create returns normally with a true
        // return value iff it causes osl::Thread::run to start executing:
        acquire();
        try {
            if (!create()) {
                throw std::runtime_error("osl::Thread::create failed");
            }
        } catch (...) {
            release();
            throw;
        }
    }

    virtual void run() override
    {
        setName("sal-thread-pool");
#if defined DBG_UTIL && (defined LINUX || defined _WIN32)
        gbIsWorkerThread = true;
#endif
        std::unique_lock< std::mutex > aGuard( mpPool->maMutex );

        while( !mpPool->mbTerminate )
        {
            ThreadTask *pTask = mpPool->popWorkLocked( aGuard, true );
            if( pTask )
            {
                aGuard.unlock();

                pTask->execAndDelete();

                aGuard.lock();
            }
        }
    }
};

ThreadPool::ThreadPool(sal_Int32 nWorkers)
    : mbTerminate(true)
    , mnWorkers(nWorkers)
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
}

struct ThreadPoolStatic : public rtl::StaticWithInit< std::shared_ptr< ThreadPool >,
                                                      ThreadPoolStatic >
{
    std::shared_ptr< ThreadPool > operator () () {
        const sal_Int32 nThreads = ThreadPool::getPreferredConcurrency();
        return std::shared_ptr< ThreadPool >( new ThreadPool(nThreads) );
    };
};

ThreadPool& ThreadPool::getSharedOptimalPool()
{
    return *ThreadPoolStatic::get().get();
}

sal_Int32 ThreadPool::getPreferredConcurrency()
{
    static sal_Int32 ThreadCount = 0;
    if (ThreadCount == 0)
    {
        const sal_Int32 nHardThreads = std::max(std::thread::hardware_concurrency(), 1U);
        sal_Int32 nThreads = nHardThreads;
        const char *pEnv = getenv("MAX_CONCURRENCY");
        if (pEnv != nullptr)
        {
            // Override with user/admin preferrence.
            nThreads = rtl_str_toInt32(pEnv, 10);
        }

        nThreads = std::min(nHardThreads, nThreads);
        ThreadCount = std::max<sal_Int32>(nThreads, 1);
    }

    return ThreadCount;
}

// FIXME: there should be no need for this as/when our baseline
// is >VS2015 and drop WinXP; the sorry details are here:
// https://connect.microsoft.com/VisualStudio/feedback/details/1282596
void ThreadPool::shutdown()
{
    if (mbTerminate)
        return;

    std::unique_lock< std::mutex > aGuard( maMutex );

    if( maWorkers.empty() )
    { // no threads at all -> execute the work in-line
        ThreadTask *pTask;
        while ( ( pTask = popWorkLocked(aGuard, false) ) )
            pTask->execAndDelete();
    }
    else
    {
        while( !maTasks.empty() )
            maTasksChanged.wait( aGuard );
    }
    assert( maTasks.empty() );

    // coverity[missing_lock]
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

void ThreadPool::pushTask( ThreadTask *pTask )
{
    std::unique_lock< std::mutex > aGuard( maMutex );

    mbTerminate = false;

    if (maWorkers.size() < mnWorkers && maWorkers.size() <= maTasks.size())
    {
        maWorkers.push_back( new ThreadWorker( this ) );
        maWorkers.back()->launch();
    }

    pTask->mpTag->onTaskPushed();
    maTasks.insert( maTasks.begin(), pTask );

    maTasksChanged.notify_one();
}

ThreadTask *ThreadPool::popWorkLocked( std::unique_lock< std::mutex > & rGuard, bool bWait )
{
    do
    {
        if( !maTasks.empty() )
        {
            ThreadTask *pTask = maTasks.back();
            maTasks.pop_back();
            return pTask;
        }
        else if (!bWait || mbTerminate)
            return nullptr;

        maTasksChanged.wait( rGuard );

    } while (!mbTerminate);

    return nullptr;
}

void ThreadPool::waitUntilDone(const std::shared_ptr<ThreadTaskTag>& rTag)
{
#if defined DBG_UTIL && (defined LINUX || defined _WIN32)
    assert(!gbIsWorkerThread && "cannot wait for tasks from inside a task");
#endif
    {
        std::unique_lock< std::mutex > aGuard( maMutex );

        if( maWorkers.empty() )
        { // no threads at all -> execute the work in-line
            ThreadTask *pTask;
            while (!rTag->isDone() &&
                   ( pTask = popWorkLocked(aGuard, false) ) )
                pTask->execAndDelete();
        }
    }

    rTag->waitUntilDone();
}

std::shared_ptr<ThreadTaskTag> ThreadPool::createThreadTaskTag()
{
    return std::make_shared<ThreadTaskTag>();
}

bool ThreadPool::isTaskTagDone(const std::shared_ptr<ThreadTaskTag>& pTag)
{
    return pTag->isDone();
}

ThreadTask::ThreadTask(const std::shared_ptr<ThreadTaskTag>& pTag)
    : mpTag(pTag)
{
}

void ThreadTask::execAndDelete()
{
    std::shared_ptr<ThreadTaskTag> pTag(mpTag);
    try {
        doWork();
    }
    catch (const std::exception &e)
    {
        SAL_WARN("sal.threadpool", "exception in thread worker while calling doWork(): " << e.what());
    }
    catch (...)
    {
        SAL_WARN("sal.threadpool", "exception in thread worker while calling doWork()");
    }

    delete this;
    pTag->onTaskWorkerDone();
}

ThreadTaskTag::ThreadTaskTag() : mnTasksWorking(0)
{
}

void ThreadTaskTag::onTaskPushed()
{
    std::unique_lock< std::mutex > aGuard( maMutex );
    mnTasksWorking++;
    assert( mnTasksWorking < 65536 ); // sanity checking
}

void ThreadTaskTag::onTaskWorkerDone()
{
    std::unique_lock< std::mutex > aGuard( maMutex );
    mnTasksWorking--;
    assert(mnTasksWorking >= 0);
    if (mnTasksWorking == 0)
        maTasksComplete.notify_all();
}

bool ThreadTaskTag::isDone()
{
    std::unique_lock< std::mutex > aGuard( maMutex );
    return mnTasksWorking == 0;
}

void ThreadTaskTag::waitUntilDone()
{
    std::unique_lock< std::mutex > aGuard( maMutex );
    while( mnTasksWorking > 0 )
    {
#ifdef DBG_UTIL
        // 3 minute timeout in debug mode so our tests fail sooner rather than later
        std::cv_status result = maTasksComplete.wait_for(
            aGuard, std::chrono::seconds( 3 * 60 ));
        assert(result != std::cv_status::timeout);
#else
        // 10 minute timeout in production so the app eventually throws some kind of error
        if (maTasksComplete.wait_for(
                aGuard, std::chrono::seconds( 10 * 60 )) == std::cv_status::timeout)
            throw std::runtime_error("timeout waiting for threadpool tasks");
#endif
    }
}

} // namespace sal

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
