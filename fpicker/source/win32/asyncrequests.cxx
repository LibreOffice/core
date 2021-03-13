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

#include "asyncrequests.hxx"
#include <vcl/svapp.hxx>
#include <vcl/winscheduler.hxx>
#include <osl/mutex.hxx>

namespace fpicker{
namespace win32{
namespace vista{

static void lcl_sleep( ::osl::Condition& aCondition,
                       ::sal_Int32       nMilliSeconds )
{
    if (nMilliSeconds < 1)
        aCondition.wait();
    else
    {
        TimeValue aTime;
        aTime.Seconds = (nMilliSeconds / 1000);
        aTime.Nanosec = (nMilliSeconds % 1000) * 1000000;
        aCondition.wait(&aTime);
    }
}

void Request::wait( ::sal_Int32 nMilliSeconds )
{
    SolarMutexReleaser aReleaser;

    lcl_sleep( m_aJoiner, nMilliSeconds );
}

void Request::waitProcessMessages()
{
    SolarMutexGuard aGuard;
    while ( !m_aJoiner.check() && !Application::IsQuit())
        Application::Yield();
}

void Request::notify()
{
    m_aJoiner.set();
    // Make sure that main loop receives at least this message to return from GetMessage and recheck
    // the condition, even in case when there's no visible application windows present, and thus no
    // other messages might arrive to the main loop.
    WinScheduler::PostDummyMessage();
}

AsyncRequests::AsyncRequests(const RequestHandlerRef& rHandler)
    : ::cppu::BaseMutex(         )
    , ::osl::Thread    (         )
    , m_bFinish        (false)
    , m_rHandler       (rHandler )
    , m_lRequests      (         )
{
}

AsyncRequests::~AsyncRequests()
{
    // SYNCHRONIZED ->
    {
        osl::MutexGuard aLock(m_aMutex);
        m_bFinish = true;
    }
    // <- SYNCHRONIZED

    // The static AsyncRequests aNotify in VistaFilePickerEventHandler::impl_sendEvent
    // is destructed at DLL atexit. But it won't run, so needs no join and release of
    // the already destructed SolarMutex, which would crash LO on exit.
    if (isRunning())
    {
        // tdf#123502: make sure we actually hold the mutex before releasing it
        // UNO directly destroys the VistaFilePicker object, so we need GUI protection in there.
        // But since we redirect GUI stuff to the async thread we also have to release it, so we
        // can join it, if the thread currently blocks on the SolarMutex.
        SolarMutexGuard aGuard;
        SolarMutexReleaser aReleaser;
        join();
    }
}

void AsyncRequests::triggerJobExecution()
{
    if ( ! isRunning())
        create();
    else
        maWait.set();
}

void AsyncRequests::triggerRequestProcessMessages (const RequestRef& rRequest)
{
    // SYNCHRONIZED ->
    {
        osl::MutexGuard aLock(m_aMutex);
        m_lRequests.push(rRequest);
    }
    // <- SYNCHRONIZED

    rRequest->waitProcessMessages();
}

void AsyncRequests::triggerRequestBlocked(const RequestRef& rRequest)
{
    // SYNCHRONIZED ->
    {
        osl::MutexGuard aLock(m_aMutex);
        m_lRequests.push(rRequest);
    }
    // <- SYNCHRONIZED

    triggerJobExecution();

    rRequest->wait();
}

void AsyncRequests::triggerRequestNonBlocked(const RequestRef& rRequest)
{
    // SYNCHRONIZED ->
    {
        osl::MutexGuard aLock(m_aMutex);
        m_lRequests.push(rRequest);
    }
    // <- SYNCHRONIZED

    triggerJobExecution();
}

void AsyncRequests::triggerRequestDirectly(const RequestRef& rRequest)
{
    // SYNCHRONIZED ->
    osl::ClearableMutexGuard aLock(m_aMutex);
    RequestHandlerRef rHandler = m_rHandler;
    aLock.clear();
    // <- SYNCHRONIZED

    if (rHandler != nullptr)
        rHandler->doRequest(rRequest);
}

void AsyncRequests::triggerRequestThreadAware(const RequestRef& rRequest,
                                                    ::sal_Int16  nWait   )
{
    oslThreadIdentifier nOurThreadId    = getIdentifier();
    oslThreadIdentifier nCallerThreadId = ::osl::Thread::getCurrentIdentifier();
    SolarMutexGuard aGuard;
    if (nOurThreadId == nCallerThreadId)
        triggerRequestDirectly(rRequest);
    else if (nWait == BLOCKED)
        triggerRequestBlocked(rRequest);
    else if (nWait == PROCESS_MESSAGES)
        triggerRequestProcessMessages(rRequest);
    else
        triggerRequestNonBlocked(rRequest);
}

void SAL_CALL AsyncRequests::run()
{
    osl_setThreadName("fpicker::win32::vista::AsyncRequests");

    static const ::sal_Int32 TIME_TO_WAIT_FOR_NEW_REQUESTS = 250;

    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);
    RequestHandlerRef rHandler  = m_rHandler;
    bool bFinished = m_bFinish;
    aLock.clear();
    // <- SYNCHRONIZED

    if (rHandler != nullptr)
        rHandler->before();

    while ( ! bFinished)
    {
        // SYNCHRONIZED ->
        aLock.reset();

        RequestRef rRequest;
        if ( ! m_lRequests.empty())
        {
            rRequest = m_lRequests.front();
            m_lRequests.pop();
        }
        bFinished = m_bFinish;

        aLock.clear();
        // <- SYNCHRONIZED

        if (rRequest == nullptr)
        {
            lcl_sleep(maWait, TIME_TO_WAIT_FOR_NEW_REQUESTS);
            maWait.reset();
            continue;
        }

        if (rHandler != nullptr)
        {
            rHandler->doRequest(rRequest);
            rRequest->notify();
        }
    }

    if (rHandler != nullptr)
        rHandler->after();
}

} // namespace vista
} // namespace win32
} // namespace fpicker

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
