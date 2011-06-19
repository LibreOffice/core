/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _ASYNCEVENTNOTIFIER_HXX_
#define _ASYNCEVENTNOTIFIER_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <cppuhelper/interfacecontainer.h>

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <list>
#include <utility>
#include "eventnotification.hxx"

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
