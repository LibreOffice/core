/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: asyncrequests.cxx,v $
 * $Revision: 1.3 $
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

#include "asyncrequests.hxx"

//-----------------------------------------------------------------------------
// namespace
//-----------------------------------------------------------------------------

namespace fpicker{
namespace win32{
namespace vista{

namespace css = ::com::sun::star;

//-----------------------------------------------------------------------------
void lcl_sleep(::osl::Condition& aCondition   ,
               ::sal_Int32       nMilliSeconds)
{
    if (nMilliSeconds < 1)
        aCondition.wait(0);
    else
    {
        TimeValue aTime;
        aTime.Seconds = (nMilliSeconds / 1000);
        aTime.Nanosec = (nMilliSeconds % 1000);
        aCondition.wait(&aTime);
    }
}

//-----------------------------------------------------------------------------
void Request::wait(::sal_Int32 nMilliSeconds)
{
    lcl_sleep(m_aJoiner, nMilliSeconds);
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
                                                    ::sal_Bool  bWait   )
{
    oslThreadIdentifier nOurThreadId    = getIdentifier();
    oslThreadIdentifier nCallerThreadId = ::osl::Thread::getCurrentIdentifier();

    if (nOurThreadId == nCallerThreadId)
        triggerRequestDirectly(rRequest);
    else
    if (bWait)
        triggerRequestBlocked(rRequest);
    else
        triggerRequestNonBlocked(rRequest);
}

//-----------------------------------------------------------------------------
class SmallAsyncThread : private ::cppu::BaseMutex
                       , public  ::osl::Thread
{
    public:

        SmallAsyncThread(const RequestHandlerRef& rHandler,
                         const RequestRef&        rRequest)
            : m_rHandler(rHandler)
            , m_rRequest(rRequest)
        {};

        virtual ~SmallAsyncThread() {};

    private:

        virtual void SAL_CALL run()
        {
            m_rHandler->doRequest(m_rRequest);
            m_rRequest->notify();
        }

    private:

        RequestHandlerRef m_rHandler;
        RequestRef m_rRequest;
};

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
            /*
            SmallAsyncThread* pThread = new SmallAsyncThread(rHandler, rRequest);
            pThread->create();
            */
        }
    }

    if (rHandler != NULL)
        rHandler->after();
}

} // namespace vista
} // namespace win32
} // namespace fpicker
