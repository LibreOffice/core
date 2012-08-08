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
#include <osl/mutex.hxx>

namespace fpicker{
namespace win32{
namespace vista{

//-----------------------------------------------------------------------------
void lcl_sleep(::osl::Condition& aCondition   ,
               ::sal_Int32       nMilliSeconds)
{
    sal_uLong nAcquireCount = Application::ReleaseSolarMutex();

    if (nMilliSeconds < 1)
        aCondition.wait(0);
    else
    {
        TimeValue aTime;
        aTime.Seconds = (nMilliSeconds / 1000);
        aTime.Nanosec = (nMilliSeconds % 1000) * 1000000;
        aCondition.wait(&aTime);
    }

    Application::AcquireSolarMutex( nAcquireCount );
}

//-----------------------------------------------------------------------------
void Request::wait(::sal_Int32 nMilliSeconds)
{
    lcl_sleep(m_aJoiner, nMilliSeconds);
}

void Request::waitProcessMessages()
{
    SolarMutexGuard aGuard;
    while (!m_aJoiner.check())
        Application::Yield();
}

//-----------------------------------------------------------------------------
void Request::notify()
{
    m_aJoiner.set();
}

//-----------------------------------------------------------------------------
AsyncRequests::AsyncRequests(const RequestHandlerRef& rHandler)
    : ::cppu::BaseMutex(         )
    , ::osl::Thread    (         )
    , m_bFinish        (sal_False)
    , m_rHandler       (rHandler )
    , m_lRequests      (         )
{
}

//-----------------------------------------------------------------------------
AsyncRequests::~AsyncRequests()
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);
    m_bFinish = sal_True;
    aLock.clear();
    // <- SYNCHRONIZED

    join();
}

void AsyncRequests::triggerRequestProcessMessages (const RequestRef& rRequest)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);
    m_lRequests.push(rRequest);
    aLock.clear();
    // <- SYNCHRONIZED

    if ( ! isRunning())
        create();

    rRequest->waitProcessMessages();
}

//-----------------------------------------------------------------------------
void AsyncRequests::triggerRequestBlocked(const RequestRef& rRequest)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);
    m_lRequests.push(rRequest);
    aLock.clear();
    // <- SYNCHRONIZED

    if ( ! isRunning())
        create();

    rRequest->wait(Request::WAIT_INFINITE);
}

//-----------------------------------------------------------------------------
void AsyncRequests::triggerRequestNonBlocked(const RequestRef& rRequest)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);
    m_lRequests.push(rRequest);
    aLock.clear();
    // <- SYNCHRONIZED

    if ( ! isRunning())
        create();
}

//-----------------------------------------------------------------------------
void AsyncRequests::triggerRequestDirectly(const RequestRef& rRequest)
{
    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);
    RequestHandlerRef rHandler = m_rHandler;
    aLock.clear();
    // <- SYNCHRONIZED

    if (rHandler != NULL)
        rHandler->doRequest(rRequest);
}

//-----------------------------------------------------------------------------
void AsyncRequests::triggerRequestThreadAware(const RequestRef& rRequest,
                                                    ::sal_Int16  nWait   )
{
    oslThreadIdentifier nOurThreadId    = getIdentifier();
    oslThreadIdentifier nCallerThreadId = ::osl::Thread::getCurrentIdentifier();
    if (nOurThreadId == nCallerThreadId)
        triggerRequestDirectly(rRequest);
    else if (nWait == BLOCKED)
        triggerRequestBlocked(rRequest);
    else if (nWait == PROCESS_MESSAGES)
        triggerRequestProcessMessages(rRequest);
    else
        triggerRequestNonBlocked(rRequest);
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void SAL_CALL AsyncRequests::run()
{
    static const ::sal_Int32 TIME_TO_WAIT_FOR_NEW_REQUESTS = 250;

    // SYNCHRONIZED ->
    ::osl::ResettableMutexGuard aLock(m_aMutex);
    RequestHandlerRef rHandler  = m_rHandler;
    ::sal_Bool        bFinished = m_bFinish;
    aLock.clear();
    // <- SYNCHRONIZED

    if (rHandler != NULL)
        rHandler->before();

    ::osl::Condition aWait;

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

        if (rRequest == NULL)
        {
            lcl_sleep(aWait, TIME_TO_WAIT_FOR_NEW_REQUESTS);
            continue;
        }

        if (rHandler != NULL)
        {
            rHandler->doRequest(rRequest);
            rRequest->notify();
        }
    }

    if (rHandler != NULL)
        rHandler->after();
}

} // namespace vista
} // namespace win32
} // namespace fpicker

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
