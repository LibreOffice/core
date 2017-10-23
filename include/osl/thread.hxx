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

#ifndef INCLUDED_OSL_THREAD_HXX
#define INCLUDED_OSL_THREAD_HXX

#include "sal/config.h"

#include <cassert>
#include <cstddef>

#include "osl/time.h"
#include "osl/thread.h"
#include "rtl/alloc.h"

namespace osl
{
/** threadFunc is the function which is executed by the threads
    created by the osl::Thread class. The function's signature
    matches the one of oslWorkerFunction which is declared in
    osl/thread.h
*/
extern "C" inline void SAL_CALL threadFunc( void* param);

/**
   A thread abstraction.

   @deprecated use ::salhelper::Thread instead.  Only the static member
   functions ::osl::Thread::getCurrentIdentifier, ::osl::Thread::wait, and
   ::osl::Thread::yield are not deprecated.
 */
class Thread
{
    Thread( const Thread& ) SAL_DELETED_FUNCTION;
    Thread& operator= ( const Thread& ) SAL_DELETED_FUNCTION;
public:
    // these are here to force memory de/allocation to sal lib.
    static void * SAL_CALL operator new( size_t nSize )
        { return ::rtl_allocateMemory( nSize ); }
    static void SAL_CALL operator delete( void * pMem )
        { ::rtl_freeMemory( pMem ); }
    static void * SAL_CALL operator new( size_t, void * pMem )
        { return pMem; }
    static void SAL_CALL operator delete( void *, void * )
        {}

    Thread(): m_hThread(NULL){}

    virtual  ~Thread() COVERITY_NOEXCEPT_FALSE
    {
        osl_destroyThread( m_hThread);
    }

    bool SAL_CALL create()
    {
        assert(m_hThread == NULL); // only one running thread per instance
        m_hThread = osl_createSuspendedThread( threadFunc, static_cast<void*>(this));
        if (m_hThread == NULL)
        {
            return false;
        }
        osl_resumeThread(m_hThread);
        return true;
    }

    bool SAL_CALL createSuspended()
    {
        assert(m_hThread == NULL); // only one running thread per instance
        if( m_hThread)
            return false;
        m_hThread= osl_createSuspendedThread( threadFunc,
                                             static_cast<void*>(this));
        return m_hThread != NULL;
    }

    virtual void SAL_CALL suspend()
    {
        if( m_hThread )
            osl_suspendThread(m_hThread);
    }

    virtual void SAL_CALL resume()
    {
        if( m_hThread )
            osl_resumeThread(m_hThread);
    }

    virtual void SAL_CALL terminate()
    {
        if( m_hThread )
            osl_terminateThread(m_hThread);
    }

    virtual void SAL_CALL join()
    {
        osl_joinWithThread(m_hThread);
    }

    bool SAL_CALL isRunning() const
    {
        return osl_isThreadRunning(m_hThread);
    }

    void SAL_CALL setPriority( oslThreadPriority Priority)
    {
        if( m_hThread )
            osl_setThreadPriority(m_hThread, Priority);
    }

    oslThreadPriority SAL_CALL getPriority() const
    {
        return m_hThread ? osl_getThreadPriority(m_hThread) : osl_Thread_PriorityUnknown;
    }

    oslThreadIdentifier SAL_CALL getIdentifier() const
    {
        return osl_getThreadIdentifier(m_hThread);
    }

    static oslThreadIdentifier SAL_CALL getCurrentIdentifier()
    {
        return osl_getThreadIdentifier(NULL);
    }

    static void SAL_CALL wait(const TimeValue& Delay)
    {
        osl_waitThread(&Delay);
    }

    static void SAL_CALL yield()
    {
        osl_yieldThread();
    }

    static void setName(char const * name) throw () {
        osl_setThreadName(name);
    }

    virtual bool SAL_CALL schedule()
    {
        return m_hThread && osl_scheduleThread(m_hThread);
    }

    SAL_CALL operator oslThread() const
    {
        return m_hThread;
    }

protected:

    /** The thread functions calls the protected functions
        run and onTerminated.
    */
    friend void SAL_CALL threadFunc( void* param);

    virtual void SAL_CALL run() = 0;

    virtual void SAL_CALL onTerminated()
    {
    }

private:
    oslThread m_hThread;
};

extern "C" inline void SAL_CALL threadFunc( void* param)
{
        Thread* pObj= static_cast<Thread*>(param);
        pObj->run();
        pObj->onTerminated();
}

class ThreadData
{
    ThreadData( const ThreadData& ) SAL_DELETED_FUNCTION;
    ThreadData& operator= (const ThreadData& ) SAL_DELETED_FUNCTION;
public:
     /// Create a thread specific local data key
    ThreadData( oslThreadKeyCallbackFunction pCallback= NULL )
    {
        m_hKey = osl_createThreadKey( pCallback );
    }

    /// Destroy a thread specific local data key
    ~ThreadData()
    {
           osl_destroyThreadKey(m_hKey);
    }

    /** Set the data associated with the data key.
        @returns True if operation was successful
    */
    bool SAL_CALL setData(void *pData)
    {
           return osl_setThreadKeyData(m_hKey, pData);
    }

    /** Get the data associated with the data key.
        @returns The data associated with the data key or
        NULL if no data was set
    */
    void* SAL_CALL getData()
    {
           return osl_getThreadKeyData(m_hKey);
    }

    operator oslThreadKey() const
    {
        return m_hKey;
    }

private:
    oslThreadKey m_hKey;
};

} // end namespace osl

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
