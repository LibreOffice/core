/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: asynceventnotifier.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 07:33:58 $
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

#ifndef _ASYNCEVENTNOTIFIER_HXX_
#define _ASYNCEVENTNOTIFIER_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _OSL_CONDITN_HXX_
#include <osl/conditn.hxx>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <list>
#include <utility>

#ifndef _EVENTNOTIFICATION_HXX_
#include "eventnotification.hxx"
#endif

//---------------------------------------------
//
//---------------------------------------------

class CAsyncEventNotifier
{
public:
    CAsyncEventNotifier(cppu::OBroadcastHelper& rBroadcastHelper);
    ~CAsyncEventNotifier();

    bool SAL_CALL startup(bool bCreateSuspended = true);
    void SAL_CALL shutdown();

    // notifications may be added the
    // the event queue but will only
    // be notified to the clients after
    // resume was called
    void suspend();

    // resume notifying events
    void resume();

    // this class is responsible for the memory management of
    // the CEventNotification instance
    void SAL_CALL notifyEvent(CEventNotification* EventNotification);

    void SAL_CALL addListener   (const ::com::sun::star::uno::Type&                                           aType    ,
                                 const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xListener);
    void SAL_CALL removeListener(const ::com::sun::star::uno::Type&                                           aType    ,
                                 const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xListener);

private:
    size_t              SAL_CALL getEventListSize();
    void                SAL_CALL resetNotifyEvent();
    CEventNotification* SAL_CALL getNextEventRecord();
    void                SAL_CALL removeNextEventRecord();

    void SAL_CALL run( );

    static unsigned int WINAPI ThreadProc(LPVOID pParam);

private:
    std::list<CEventNotification*>  m_EventList;
    HANDLE                          m_hThread;
    bool                            m_bRun;
    unsigned                        m_ThreadId;
    ::cppu::OBroadcastHelper&       m_rBroadcastHelper;
    HANDLE                          m_hEvents[2];
    HANDLE&                         m_NotifyEvent;
    HANDLE&                         m_ResumeNotifying;
    osl::Mutex                      m_Mutex;

// prevent copy and assignment
private:
    CAsyncEventNotifier( const CAsyncEventNotifier& );
    CAsyncEventNotifier& operator=( const CAsyncEventNotifier& );
};

#endif
