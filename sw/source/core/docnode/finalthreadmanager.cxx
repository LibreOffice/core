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

#include <finalthreadmanager.hxx>

#include <osl/diagnose.h>
#include <osl/thread.hxx>
#include <pausethreadstarting.hxx>
#include <swthreadjoiner.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/TerminationVetoException.hpp>
#include <rtl/ustring.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <mutex>
#include <thread>
#include <utility>

/** thread to cancel a give list of cancellable jobs

    helper class for FinalThreadManager
*/
class CancelJobsThread : public osl::Thread
{
    public:
        explicit CancelJobsThread( std::list< css::uno::Reference< css::util::XCancellable > >&& rJobs )
            : maJobs( std::move(rJobs) ),
              mbAllJobsCancelled( false ),
              mbStopped( false )
        {
        }

        void addJobs( std::list< css::uno::Reference< css::util::XCancellable > >& rJobs );
        bool allJobsCancelled() const;
        void stopWhenAllJobsCancelled();

    private:
        bool existJobs() const;

        css::uno::Reference< css::util::XCancellable > getNextJob();

        bool stopped() const;
        virtual void SAL_CALL run() override;
        mutable std::mutex maMutex;

        std::list< css::uno::Reference< css::util::XCancellable > > maJobs;

        bool mbAllJobsCancelled;
        bool mbStopped;
};

void CancelJobsThread::addJobs( std::list< css::uno::Reference< css::util::XCancellable > >& rJobs )
{
    std::scoped_lock aGuard(maMutex);

    maJobs.insert( maJobs.end(), rJobs.begin(), rJobs.end() );
    mbAllJobsCancelled = !maJobs.empty();
}

bool CancelJobsThread::existJobs() const
{
    std::scoped_lock aGuard(maMutex);

    return !maJobs.empty();
}

bool CancelJobsThread::allJobsCancelled() const
{
    std::scoped_lock aGuard(maMutex);

    return maJobs.empty() && mbAllJobsCancelled;
}

void CancelJobsThread::stopWhenAllJobsCancelled()
{
    std::scoped_lock aGuard(maMutex);

    mbStopped = true;
}

css::uno::Reference< css::util::XCancellable > CancelJobsThread::getNextJob()
{
    css::uno::Reference< css::util::XCancellable > xRet;

    {
        std::scoped_lock aGuard(maMutex);

        if ( !maJobs.empty() )
        {
            xRet = maJobs.front();
            maJobs.pop_front();
        }
    }

    return xRet;
}

bool CancelJobsThread::stopped() const
{
    std::scoped_lock aGuard(maMutex);

    return mbStopped;
}

void SAL_CALL CancelJobsThread::run()
{
    osl_setThreadName("sw CancelJobsThread");

    while ( !stopped() )
    {
        while ( existJobs() )
        {
            css::uno::Reference< css::util::XCancellable > aJob( getNextJob() );
            if ( aJob.is() )
                aJob->cancel();
        }

        mbAllJobsCancelled = true;

        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

/** thread to terminate office, when all jobs are cancelled.

    helper class for FinalThreadManager
*/
class TerminateOfficeThread : public osl::Thread
{
    public:
        TerminateOfficeThread( CancelJobsThread const & rCancelJobsThread,
                               css::uno::Reference< css::uno::XComponentContext >  xContext )
            : mrCancelJobsThread( rCancelJobsThread ),
              mbStopOfficeTermination( false ),
              mxContext(std::move( xContext ))
        {
        }

        void StopOfficeTermination();

    private:
        virtual void SAL_CALL run() override;
        virtual void SAL_CALL onTerminated() override;
        bool OfficeTerminationStopped();
        void PerformOfficeTermination();

        osl::Mutex maMutex;

        const CancelJobsThread& mrCancelJobsThread;
        bool mbStopOfficeTermination;

        css::uno::Reference< css::uno::XComponentContext > mxContext;
};

void TerminateOfficeThread::StopOfficeTermination()
{
    osl::MutexGuard aGuard(maMutex);

    mbStopOfficeTermination = true;
}

bool TerminateOfficeThread::OfficeTerminationStopped()
{
    osl::MutexGuard aGuard(maMutex);

    return mbStopOfficeTermination;
}

void SAL_CALL TerminateOfficeThread::run()
{
    osl_setThreadName("sw TerminateOfficeThread");

    while ( !OfficeTerminationStopped() )
    {
        osl::MutexGuard aGuard(maMutex);

        if ( mrCancelJobsThread.allJobsCancelled() )
            break;
    }

    if ( !OfficeTerminationStopped() )
        PerformOfficeTermination();
}

void TerminateOfficeThread::PerformOfficeTermination()
{
    css::uno::Reference< css::frame::XDesktop2 > xDesktop = css::frame::Desktop::create(mxContext);

    css::uno::Reference< css::container::XElementAccess > xList = xDesktop->getFrames();
    if ( !xList.is() )
    {
        OSL_FAIL( "<TerminateOfficeThread::PerformOfficeTermination()> - no XElementAccess!" );
        return;
    }

    if ( !xList->hasElements() )
    {
        if ( !OfficeTerminationStopped() )
            xDesktop->terminate();
    }
}

void SAL_CALL TerminateOfficeThread::onTerminated()
{
    if ( OfficeTerminationStopped() )
        delete this;
}

FinalThreadManager::FinalThreadManager(css::uno::Reference< css::uno::XComponentContext > context)
    : m_xContext(std::move(context)),
      mpTerminateOfficeThread( nullptr ),
      mbRegisteredAtDesktop( false )
{

}

void FinalThreadManager::registerAsListenerAtDesktop()
{
    css::uno::Reference< css::frame::XDesktop2 > xDesktop = css::frame::Desktop::create(m_xContext);
    xDesktop->addTerminateListener( css::uno::Reference< css::frame::XTerminateListener >( this ) );
}

FinalThreadManager::~FinalThreadManager()
{
    if ( mpPauseThreadStarting )
    {
        mpPauseThreadStarting.reset();
    }

    if ( mpTerminateOfficeThread != nullptr )
    {
        mpTerminateOfficeThread->StopOfficeTermination(); // thread kills itself.
        mpTerminateOfficeThread = nullptr;
    }

    if ( !maThreads.empty() )
    {
        OSL_FAIL( "<FinalThreadManager::~FinalThreadManager()> - still registered jobs are existing -> perform cancellation" );
        cancelAllJobs();
    }

    if ( mpCancelJobsThread != nullptr )
    {
        if ( !mpCancelJobsThread->allJobsCancelled() )
            OSL_FAIL( "<FinalThreadManager::~FinalThreadManager()> - cancellation of registered jobs not yet finished -> wait for its finish" );

        mpCancelJobsThread->stopWhenAllJobsCancelled();
        mpCancelJobsThread->join();
        mpCancelJobsThread.reset();
    }
}

// com.sun.star.uno.XServiceInfo:
OUString SAL_CALL FinalThreadManager::getImplementationName()
{
    return u"com.sun.star.util.comp.FinalThreadManager"_ustr;
}

sal_Bool SAL_CALL FinalThreadManager::supportsService(OUString const & serviceName)
{
    return cppu::supportsService(this, serviceName);
}

css::uno::Sequence< OUString > SAL_CALL FinalThreadManager::getSupportedServiceNames()
{
    return { u"com.sun.star.util.JobManager"_ustr };
}

// css::util::XJobManager:
void SAL_CALL FinalThreadManager::registerJob(const css::uno::Reference< css::util::XCancellable > & Job)
{
    osl::MutexGuard aGuard(maMutex);

    maThreads.push_back( Job );

    if ( !mbRegisteredAtDesktop )
    {
        registerAsListenerAtDesktop();
        mbRegisteredAtDesktop = true;
    }
}

void SAL_CALL FinalThreadManager::releaseJob(const css::uno::Reference< css::util::XCancellable > & Job)
{
    osl::MutexGuard aGuard(maMutex);

    maThreads.remove( Job );
}

void SAL_CALL FinalThreadManager::cancelAllJobs()
{
    std::list< css::uno::Reference< css::util::XCancellable > > aThreads;
    {
        osl::MutexGuard aGuard(maMutex);

        aThreads.insert( aThreads.end(), maThreads.begin(), maThreads.end() );
        maThreads.clear();
    }

    if ( aThreads.empty() )
        return;

    osl::MutexGuard aGuard(maMutex);

    if ( mpCancelJobsThread == nullptr )
    {
        mpCancelJobsThread.reset(new CancelJobsThread( std::list(aThreads) ));
        if ( !mpCancelJobsThread->create() )
        {
            mpCancelJobsThread.reset();
            for (auto const& elem : aThreads)
            {
                elem->cancel();
            }
            aThreads.clear();
        }
    }
    else
        mpCancelJobsThread->addJobs( aThreads );
}

// css::frame::XTerminateListener
void SAL_CALL FinalThreadManager::queryTermination( const css::lang::EventObject& )
{
    osl::MutexGuard aGuard(maMutex);

    cancelAllJobs();
    // Sleep 1 second to give the thread for job cancellation some time.
    // Probably, all started threads have already finished its work.
    if ( mpCancelJobsThread != nullptr &&
         !mpCancelJobsThread->allJobsCancelled() )
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if ( mpCancelJobsThread != nullptr &&
         !mpCancelJobsThread->allJobsCancelled() )
    {
        if ( mpTerminateOfficeThread != nullptr )
        {
            if ( mpTerminateOfficeThread->isRunning() )
                mpTerminateOfficeThread->StopOfficeTermination(); // thread kills itself.
            else
                delete mpTerminateOfficeThread;

            mpTerminateOfficeThread = nullptr;
        }
        mpTerminateOfficeThread = new TerminateOfficeThread( *mpCancelJobsThread,
                                                 m_xContext );
        if ( !mpTerminateOfficeThread->create() )
        {
            delete mpTerminateOfficeThread;
            mpTerminateOfficeThread = nullptr;
        }

        throw css::frame::TerminationVetoException();
    }

    mpPauseThreadStarting.reset(new SwPauseThreadStarting());
}

void SAL_CALL FinalThreadManager::cancelTermination( const css::lang::EventObject& )
{
    mpPauseThreadStarting.reset();
}

void SAL_CALL FinalThreadManager::notifyTermination( const css::lang::EventObject& )
{
    if ( mpTerminateOfficeThread != nullptr )
    {
        if ( mpTerminateOfficeThread->isRunning() )
            mpTerminateOfficeThread->StopOfficeTermination(); // thread kills itself.
        else
            delete mpTerminateOfficeThread;

        mpTerminateOfficeThread = nullptr;
    }

    if ( !maThreads.empty() )
        cancelAllJobs();

    if ( mpCancelJobsThread != nullptr )
    {
        mpCancelJobsThread->stopWhenAllJobsCancelled();
        mpCancelJobsThread->join();
        mpCancelJobsThread.reset();
    }

    // get reference of this
    css::uno::Reference< css::uno::XInterface > aOwnRef( getXWeak());
    // notify <SwThreadJoiner> to release its reference
    SwThreadJoiner::ReleaseThreadJoiner();
}

// ::com::sun:star::lang::XEventListener (inherited via css::frame::XTerminateListener)
void SAL_CALL FinalThreadManager::disposing( const css::lang::EventObject& )
{
    // nothing to do, because instance doesn't hold any references of observed objects
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_util_comp_FinalThreadManager_get_implementation(css::uno::XComponentContext* context,
                                css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new FinalThreadManager(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
