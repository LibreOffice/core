/*************************************************************************
 *
 *  $RCSfile: thread.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mhu $ $Date: 2001-03-13 11:58:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _THREAD_HXX_
#define _THREAD_HXX_

#ifdef __cplusplus

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif

namespace osl
{
/** threadFunc is the function which is executed by the threads
    created by the osl::Thread class. The function's signature
    matches the one of oslWorkerFunction which is declared in
    osl/thread.h .
*/
extern "C" inline void SAL_CALL threadFunc( void* param);

class Thread
{
    Thread( const Thread& );
    Thread& operator= ( const Thread& );
public:
    // these are here to force memory de/allocation to sal lib.
    inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW (())
        { return ::rtl_allocateMemory( nSize ); }
    inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW (())
        { ::rtl_freeMemory( pMem ); }
    inline static void * SAL_CALL operator new( size_t, void * pMem ) SAL_THROW (())
        { return pMem; }
    inline static void SAL_CALL operator delete( void *, void * ) SAL_THROW (())
        {}

    Thread(): m_hThread(0){}

    virtual  ~Thread()
    {
        osl_destroyThread( m_hThread);
    }

    sal_Bool SAL_CALL create()
    {
        OSL_ASSERT(m_hThread == 0); // only one running thread per instance
           if (m_hThread)
            return sal_False;

        if (m_hThread= osl_createSuspendedThread( threadFunc,
                                             (void*)this))
            osl_resumeThread(m_hThread);

        return m_hThread != 0;
    }

    sal_Bool SAL_CALL createSuspended()
    {
        OSL_ASSERT(m_hThread == 0); // only one running thread per instance
        if( m_hThread)
            return sal_False;
        m_hThread= osl_createSuspendedThread( threadFunc,
                                             (void*)this);
        return m_hThread != 0;
    }

    virtual void SAL_CALL suspend()
    {
        osl_suspendThread(m_hThread);
    }

    virtual void SAL_CALL resume()
    {
        osl_resumeThread(m_hThread);
    }

    virtual void SAL_CALL terminate()
    {
        osl_terminateThread(m_hThread);
    }

    virtual void SAL_CALL join()
    {
        osl_joinWithThread(m_hThread);
    }

    sal_Bool SAL_CALL isRunning()
    {
        return m_hThread != 0 && osl_isThreadRunning(m_hThread);
    }

    void SAL_CALL setPriority( oslThreadPriority Priority)
    {
        osl_setThreadPriority(m_hThread, Priority);
    }

    oslThreadPriority SAL_CALL getPriority()
    {
        return osl_getThreadPriority(m_hThread);
    }

    oslThreadIdentifier SAL_CALL getIdentifier() const
    {
        return osl_getThreadIdentifier(m_hThread);
    }

    static oslThreadIdentifier SAL_CALL getCurrentIdentifier()
    {
        return osl_getThreadIdentifier(0);
    }

    virtual oslThreadSleep SAL_CALL sleep(const TimeValue& Delay)
    {
        return osl_sleepThread(m_hThread, &Delay);
    }

    virtual sal_Bool SAL_CALL awake()
    {
        return osl_awakeThread(m_hThread);
    }

    static void SAL_CALL wait(const TimeValue& Delay)
    {
        osl_waitThread(&Delay);
    }

    static void SAL_CALL yield()
    {
        osl_yieldThread();
    }


    virtual sal_Bool SAL_CALL schedule()
    {
        return osl_scheduleThread(m_hThread);
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
        Thread* pObj= (Thread*)param;
        pObj->run();
        pObj->onTerminated();
}

class ThreadData
{
    ThreadData( const ThreadData& );
    ThreadData& operator= (const ThreadData& );
public:
     /// Create a thread specific local data key
    ThreadData( oslThreadKeyCallbackFunction pCallback= 0 )
    {
        m_hKey = osl_createThreadKey( pCallback );
    }

    /// Destroy a thread specific local data key
    ~ThreadData()
    {
           osl_destroyThreadKey(m_hKey);
    }

    /** Set the data associated with the data key.
        @returns True if operation was successfull
    */
    sal_Bool SAL_CALL setData(void *pData)
    {
           return (osl_setThreadKeyData(m_hKey, pData));
    }

    /** Get the data associated with the data key.
        @returns The data asscoitaed with the data key or
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
#endif
