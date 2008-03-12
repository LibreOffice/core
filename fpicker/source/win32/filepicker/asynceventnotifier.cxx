/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: asynceventnotifier.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 07:33:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_fpicker.hxx"

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _ASYNCEVENTNOTIFIER_HXX_
#include "asynceventnotifier.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERLISTENER_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerListener.hpp>
#endif

#include <process.h>
#include <memory>

//------------------------------------------------
//
//------------------------------------------------

using namespace com::sun::star;
using ::com::sun::star::ui::dialogs::XFilePickerListener;

//------------------------------------------------
//
//------------------------------------------------

CAsyncEventNotifier::CAsyncEventNotifier(cppu::OBroadcastHelper& rBroadcastHelper) :
    m_hThread(0),
    m_bRun(false),
    m_ThreadId(0),
    m_rBroadcastHelper(rBroadcastHelper),
    m_NotifyEvent(m_hEvents[0]),
    m_ResumeNotifying(m_hEvents[1])
{
    // m_NotifyEvent
    m_hEvents[0] = CreateEvent(0,       /* no security */
                               true,    /* manual reset */
                               false,   /* initial state not signaled */
                               0);      /* automatic name */

    // m_ResumeNotifying
    m_hEvents[1] = CreateEvent(0,       /* no security */
                               true,    /* manual reset */
                               false,   /* initial state not signaled */
                               0);      /* automatic name */
}

//------------------------------------------------
//
//------------------------------------------------

CAsyncEventNotifier::~CAsyncEventNotifier()
{
    OSL_ENSURE(0 == m_hThread,"Thread not stopped, destroying this instance leads to desaster");

    CloseHandle(m_hEvents[0]);
    CloseHandle(m_hEvents[1]);
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CAsyncEventNotifier::addListener(const uno::Type&                         aType    ,
                                               const uno::Reference< uno::XInterface >& xListener)
{
    if ( m_rBroadcastHelper.bDisposed )
        throw lang::DisposedException(
            ::rtl::OUString::createFromAscii( "FilePicker is already disposed" ),
            uno::Reference< uno::XInterface >() );

    if ( m_rBroadcastHelper.bInDispose )
        throw lang::DisposedException(
            ::rtl::OUString::createFromAscii( "FilePicker will be disposed now." ),
            uno::Reference< uno::XInterface >() );

    m_rBroadcastHelper.aLC.addInterface( aType, xListener );
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CAsyncEventNotifier::removeListener(const uno::Type&                         aType    ,
                                                  const uno::Reference< uno::XInterface >& xListener)
{
    if ( m_rBroadcastHelper.bDisposed )
        throw lang::DisposedException(
            ::rtl::OUString::createFromAscii( "FilePicker is already disposed." ),
            uno::Reference< uno::XInterface >() );

    m_rBroadcastHelper.aLC.removeInterface( aType, xListener );
}

//------------------------------------------------
//
//------------------------------------------------

bool SAL_CALL CAsyncEventNotifier::startup(bool bCreateSuspended)
{
    osl::MutexGuard aGuard(m_Mutex);

    // m_bRun may already be false because of a
    // call to stop but the thread did not yet
    // terminate so m_hEventNotifierThread is
    // yet a valid thread handle that should
    // not be overwritten
    if (!m_bRun)
    {
        if (!bCreateSuspended)
            SetEvent(m_ResumeNotifying);

        m_hThread = (HANDLE)_beginthreadex(
            NULL, 0, CAsyncEventNotifier::ThreadProc, this, 0, &m_ThreadId);

        OSL_ASSERT(0 != m_hThread);

        if (m_hThread)
            m_bRun = true;
    }

    OSL_POSTCOND(m_bRun,"Could not start event notifier!");

    return m_bRun;
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CAsyncEventNotifier::shutdown()
{
    OSL_PRECOND(GetCurrentThreadId() != m_ThreadId, "Method called in wrong thread context!");

    osl::ResettableMutexGuard aGuard(m_Mutex);

    OSL_PRECOND(m_bRun,"Event notifier does not run!");

    m_bRun = false;
    m_EventList.clear();

    // awake the the notifier thread
    SetEvent(m_ResumeNotifying);
    SetEvent(m_NotifyEvent);

    // releas the mutex here because the event
    // notifier thread may need it to finish
    aGuard.clear();

    // we are waiting infinite, so error will
    // be better detected in form of deadlocks
    if (WaitForSingleObject(m_hThread, INFINITE) == WAIT_FAILED) {
        OSL_ENSURE(false, "Waiting for thread termination failed!");
    }

    // lock mutex again to reset m_hThread
    // and prevent a race with start()
    aGuard.reset();

    CloseHandle(m_hThread);
    m_hThread = 0;
}

//------------------------------------------------
//
//------------------------------------------------

void CAsyncEventNotifier::suspend()
{
    ResetEvent(m_ResumeNotifying);
}

//------------------------------------------------
//
//------------------------------------------------

void CAsyncEventNotifier::resume()
{
    SetEvent(m_ResumeNotifying);
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CAsyncEventNotifier::notifyEvent(CEventNotification* EventNotification)
{
    osl::MutexGuard aGuard(m_Mutex);

    OSL_ENSURE(m_bRun,"Event notifier is not running!");

    if (m_bRun)
    {
        m_EventList.push_back(EventNotification);
        SetEvent(m_NotifyEvent);
    }
}

//------------------------------------------------
//
//------------------------------------------------

size_t SAL_CALL CAsyncEventNotifier::getEventListSize()
{
    osl::MutexGuard aGuard(m_Mutex);
    return m_EventList.size();
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CAsyncEventNotifier::resetNotifyEvent()
{
    osl::MutexGuard aGuard(m_Mutex);
    if (0 == m_EventList.size())
        ResetEvent(m_NotifyEvent);
}

//------------------------------------------------
//
//------------------------------------------------

CEventNotification* SAL_CALL CAsyncEventNotifier::getNextEventRecord()
{
    osl::MutexGuard aGuard(m_Mutex);
    return m_EventList.front();
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CAsyncEventNotifier::removeNextEventRecord()
{
    osl::MutexGuard aGuard(m_Mutex);
    m_EventList.pop_front();
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CAsyncEventNotifier::run()
{
    while (m_bRun)
    {
        WaitForMultipleObjects(2, m_hEvents, true, INFINITE);

        if (m_bRun)
        {
            while (getEventListSize() > 0)
            {
                std::auto_ptr<CEventNotification> EventNotification(getNextEventRecord());
                removeNextEventRecord();

                ::cppu::OInterfaceContainerHelper* pICHelper =
                    m_rBroadcastHelper.getContainer(getCppuType((uno::Reference<XFilePickerListener>*)0));

                if (pICHelper)
                {
                    ::cppu::OInterfaceIteratorHelper iter(*pICHelper);

                    while(iter.hasMoreElements())
                    {
                        try
                        {
                            EventNotification->notifyEventListener(iter.next());
                        }
                        catch(uno::RuntimeException&)
                        {
                            OSL_ENSURE(sal_False,"RuntimeException during event dispatching");
                        }
                    }
                }

            } // while(getEventListSize() > 0)

            resetNotifyEvent();

        } // if (m_bRun)

    } // while(m_bRun)
}

//------------------------------------------------
//
//------------------------------------------------

unsigned int WINAPI CAsyncEventNotifier::ThreadProc(LPVOID pParam)
{
    CAsyncEventNotifier* pInst = reinterpret_cast< CAsyncEventNotifier* >(pParam);
    OSL_ASSERT(pInst);

    pInst->run();

    return 0;
}
