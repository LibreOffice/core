/*************************************************************************
 *
 *  $RCSfile: asynceventnotifier.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: tra $ $Date: 2002-03-21 07:35:14 $
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

#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPICKERNOTIFIER_HPP_
#include <com/sun/star/ui/dialogs/XFilePickerNotifier.hpp>
#endif

#include <process.h>
#include <windows.h>
#include <memory>

//------------------------------------------------
//
//------------------------------------------------

using namespace osl;
using namespace com::sun::star::uno;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::ui::dialogs::XFilePickerListener;
using ::com::sun::star::ui::dialogs::XFilePickerNotifier;
using ::com::sun::star::ui::dialogs::FilePickerEvent;
using ::cppu::OBroadcastHelper;

//------------------------------------------------
//
//------------------------------------------------

CAsyncEventNotifier::CAsyncEventNotifier(cppu::OBroadcastHelper& rBroadcastHelper) :
    m_hThread(0),
    m_bRun(false),
    m_ThreadId(0),
    m_rBroadcastHelper(rBroadcastHelper)
{
}

//------------------------------------------------
//
//------------------------------------------------

bool SAL_CALL CAsyncEventNotifier::start()
{
    MutexGuard aGuard(m_Mutex);

    // m_bRun may already be false because of a
    // call to stop but the thread did not yet
    // terminate so m_hEventNotifierThread is
    // yet a valid thread handle that should
    // not be overwritten
    if ( !m_bRun && 0==m_hThread )
    {
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

void SAL_CALL CAsyncEventNotifier::stop()
{
    OSL_PRECOND(GetCurrentThreadId() != m_ThreadId, "Method called in wrong thread context!");

    ResettableMutexGuard aGuard(m_Mutex);

    OSL_PRECOND(m_bRun,"Event notifier does not run!");

    m_bRun = false;
    m_EventList.clear();
    m_NotifyEvent.set( );

    // releas the mutex here because the event
    // notifier thread may need it to finish
    aGuard.clear();

    // we are waiting infinite, so error will
    // be better detected in form of deadlocks
    sal_uInt32 dwResult = WaitForSingleObject(m_hThread, INFINITE);

    OSL_ENSURE(WAIT_FAILED != dwResult, "Waiting for thread termination failed!");

    // lock mutex again to reset m_hThread
    // and prevent a race with start()
    aGuard.reset();

    CloseHandle(m_hThread);
    m_hThread = 0;
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CAsyncEventNotifier::notifyEvent(CEventNotification* EventNotification)
{
    MutexGuard aGuard(m_Mutex);

    OSL_ENSURE(m_bRun,"Event notifier is not running!");

    if (m_bRun)
    {
        m_EventList.push_back(EventNotification);
        m_NotifyEvent.set();
    }
}

//------------------------------------------------
//
//------------------------------------------------

size_t SAL_CALL CAsyncEventNotifier::getEventListSize()
{
    MutexGuard aGuard(m_Mutex);
    return m_EventList.size();
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CAsyncEventNotifier::resetNotifyEvent()
{
    MutexGuard aGuard(m_Mutex);
    if (0 == m_EventList.size())
        m_NotifyEvent.reset();
}

//------------------------------------------------
//
//------------------------------------------------

CEventNotification* SAL_CALL CAsyncEventNotifier::getNextEventRecord()
{
    MutexGuard aGuard(m_Mutex);
    return m_EventList.front();
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CAsyncEventNotifier::removeNextEventRecord()
{
    MutexGuard aGuard(m_Mutex);
    m_EventList.pop_front();
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CAsyncEventNotifier::run()
{
    while (m_bRun)
    {
        m_NotifyEvent.wait();

        if (m_bRun)
        {
            while (getEventListSize() > 0)
            {
                std::auto_ptr<CEventNotification> EventNotification(getNextEventRecord());
                removeNextEventRecord();

                ::cppu::OInterfaceContainerHelper* pICHelper =
                    m_rBroadcastHelper.getContainer(getCppuType((Reference<XFilePickerListener>*)0));

                if (pICHelper)
                {
                    ::cppu::OInterfaceIteratorHelper iter(*pICHelper);

                    while(iter.hasMoreElements())
                    {
                        try
                        {
                            EventNotification->notifyEventListener(iter.next());
                        }
                        catch(RuntimeException&)
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