/*************************************************************************
 *
 *  $RCSfile: thread.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:17 $
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


#include <vos/diagnose.hxx>
#include <vos/object.hxx>
#include <vos/thread.hxx>



#if defined ( _USE_NAMESPACE ) && !defined ( WNT )

void _OThread_WorkerFunction(void* pthis)
{
    NAMESPACE_VOS(_cpp_OThread_WorkerFunction)(pthis);
}

void NAMESPACE_VOS(_cpp_OThread_WorkerFunction)(void* pthis)

#else

void SAL_CALL _OThread_WorkerFunction(void* pthis)

#endif
{
    NAMESPACE_VOS(OThread)* pThis= (NAMESPACE_VOS(OThread)*)pthis;

    // call Handler-Function of OThread-derived class
    pThis->run();

    // if not already terminating, by a kill do normal shutdown
    if (! pThis->m_bTerminating)
    {
        pThis->m_bTerminating = sal_True;

        pThis->onTerminated();      // could e.g. delete this
    }
}

#ifdef _USE_NAMESPACE
using namespace vos;
#endif


/////////////////////////////////////////////////////////////////////////////
//
// Thread class
//

VOS_IMPLEMENT_CLASSINFO(VOS_CLASSNAME(OThread, vos),
                        VOS_NAMESPACE(OThread, vos),
                        VOS_NAMESPACE(OObject, vos), 0);

OThread::OThread()
{
    m_hThread      = 0;
    m_bTerminating = sal_False;
}

OThread::~OThread()
{
    if (m_hThread != 0)
    {
        if (! m_bTerminating)
            kill();

        osl_freeThreadHandle(m_hThread);
    }
}

sal_Bool OThread::create()
{
    VOS_ASSERT(m_hThread == 0); // only one running thread per instance

    if (m_hThread= osl_createSuspendedThread(_OThread_WorkerFunction,
                                             (void*)this))
        osl_resumeThread(m_hThread);

    return m_hThread != 0;
}

sal_Bool OThread::createSuspended()
{
    VOS_ASSERT(m_hThread == 0); // only one running thread per instance

    m_hThread= osl_createSuspendedThread(_OThread_WorkerFunction,
                                         (void*)this);
    return m_hThread != 0;
}

void OThread::suspend()
{
    VOS_ASSERT(m_hThread != 0); // use only on running thread

    osl_suspendThread(m_hThread);
}

void OThread::resume()
{
    VOS_ASSERT(m_hThread != 0); // use only on running thread

    osl_resumeThread(m_hThread);
}

sal_Bool OThread::isRunning()
{
    return m_hThread != 0 && osl_isThreadRunning(m_hThread);
}

OThread::TThreadIdentifier OThread::getIdentifier() const
{
    return (TThreadIdentifier)osl_getThreadIdentifier(m_hThread);
}

OThread::TThreadIdentifier OThread::getCurrentIdentifier()
{
    return (TThreadIdentifier)osl_getThreadIdentifier(0);
}

void OThread::join()
{
    if (m_hThread) {
        VOS_ASSERT(getCurrentIdentifier() != getIdentifier());
        osl_joinWithThread(m_hThread);
    }
}

OThread::TThreadSleep OThread::sleep(const TimeValue& Delay)
{
    if(m_hThread)
    {
        return (TThreadSleep)osl_sleepThread(m_hThread, &Delay);
    }

    return TSleep_Error;
}

void OThread::wait(const TimeValue& Delay) {
    osl_waitThread(&Delay);
}

sal_Bool OThread::awake()
{
    if(m_hThread) {
        return osl_awakeThread(m_hThread);
    }

    return sal_False;
}

void OThread::terminate()
{
    osl_terminateThread(m_hThread);
}

sal_Bool OThread::schedule() {
    return osl_scheduleThread(m_hThread);
}

void OThread::kill()
{
    if (osl_isThreadRunning(m_hThread))
    {
        // flag we are shutting down
        m_bTerminating = sal_True;

        osl_destroyThread(m_hThread);
    }
}

void OThread::setPriority(OThread::TThreadPriority Priority)
{
    osl_setThreadPriority(m_hThread, (oslThreadPriority)Priority);
}

OThread::TThreadPriority OThread::getPriority()
{
    return  (TThreadPriority)osl_getThreadPriority(m_hThread);
}


void OThread::yield()
{
    osl_yieldThread();
}

void OThread::onTerminated()
{
}

/////////////////////////////////////////////////////////////////////////////
//
// ThreadData class
//

VOS_IMPLEMENT_CLASSINFO(VOS_CLASSNAME(OThreadData, vos),
                        VOS_NAMESPACE(OThreadData, vos),
                        VOS_NAMESPACE(OObject, vos), 0);

OThreadData::OThreadData()
{
    VOS_VERIFY(m_hKey = osl_createThreadKey());
}

OThreadData::~OThreadData()
{
    osl_destroyThreadKey(m_hKey);
}

sal_Bool OThreadData::setData(void *pData)
{
    VOS_ASSERT(m_hKey != 0);

    return (osl_setThreadKeyData(m_hKey, pData));
}

void *OThreadData::getData()
{
    VOS_ASSERT(m_hKey != 0);

    return (osl_getThreadKeyData(m_hKey));
}

