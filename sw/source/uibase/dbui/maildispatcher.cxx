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

#include "maildispatcher.hxx"
#include "imaildsplistener.hxx"

#include <algorithm>

#include <com/sun/star/mail/MailException.hpp>
#include <osl/diagnose.h>

using namespace ::com::sun::star;

typedef std::list< ::rtl::Reference<IMailDispatcherListener> > MailDispatcherListenerContainer_t;

namespace /* private */
{
    /* Generic event notifier for started,
       stopped, and idle events which are
       very similary */
    class GenericEventNotifier
    {
    public:
        // pointer to virtual function typedef
        typedef void (IMailDispatcherListener::*GenericNotificationFunc_t)(::rtl::Reference<MailDispatcher>);

        GenericEventNotifier(
            GenericNotificationFunc_t notification_function,
            ::rtl::Reference<MailDispatcher> const & mail_dispatcher) :
            notification_function_(notification_function),
            mail_dispatcher_(mail_dispatcher)
        {}

        void operator() (::rtl::Reference<IMailDispatcherListener> const & listener) const
        { (listener.get()->*notification_function_)(mail_dispatcher_); }

    private:
        GenericNotificationFunc_t notification_function_;
        ::rtl::Reference<MailDispatcher> mail_dispatcher_;
    };

    class MailDeliveryNotifier
    {
    public:
        MailDeliveryNotifier(::rtl::Reference<MailDispatcher> const & xMailDispatcher, uno::Reference<mail::XMailMessage> const & message) :
            mail_dispatcher_(xMailDispatcher),
            message_(message)
        {}

        void operator() (::rtl::Reference<IMailDispatcherListener> const & listener) const
        { listener->mailDelivered(mail_dispatcher_, message_); }

    private:
        ::rtl::Reference<MailDispatcher> mail_dispatcher_;
        uno::Reference<mail::XMailMessage> message_;
    };

    class MailDeliveryErrorNotifier
    {
    public:
        MailDeliveryErrorNotifier(
            ::rtl::Reference<MailDispatcher> const & xMailDispatcher,
            uno::Reference<mail::XMailMessage> const & message,
            const OUString& error_message) :
            mail_dispatcher_(xMailDispatcher),
            message_(message),
            error_message_(error_message)
        {}

        void operator() (::rtl::Reference<IMailDispatcherListener> const & listener) const
        { listener->mailDeliveryError(mail_dispatcher_, message_, error_message_); }

    private:
        ::rtl::Reference<MailDispatcher> mail_dispatcher_;
        uno::Reference<mail::XMailMessage> message_;
        OUString error_message_;
    };

} // namespace private

MailDispatcher::MailDispatcher(uno::Reference<mail::XSmtpService> const & mailserver) :
    m_xMailserver( mailserver ),
    m_bActive( false ),
    m_bShutdownRequested( false )
{
    m_aWakeupCondition.reset();
    m_aRunCondition.reset();

    if (!create())
        throw uno::RuntimeException();

    // wait until the mail dispatcher thread is really alive
    // and has acquired a reference to this instance of the
    // class
    m_aRunCondition.wait();
}

MailDispatcher::~MailDispatcher()
{
}

void MailDispatcher::enqueueMailMessage(uno::Reference<mail::XMailMessage> const & message)
{
    ::osl::MutexGuard thread_status_guard( m_aThreadStatusMutex );
    ::osl::MutexGuard message_container_guard( m_aMessageContainerMutex );

    OSL_PRECOND( !m_bShutdownRequested, "MailDispatcher thread is shuting down already" );

    m_aXMessageList.push_back( message );
    if ( m_bActive )
        m_aWakeupCondition.set();
}

uno::Reference<mail::XMailMessage> MailDispatcher::dequeueMailMessage()
{
    ::osl::MutexGuard guard( m_aMessageContainerMutex );
    uno::Reference<mail::XMailMessage> message;
    if ( !m_aXMessageList.empty() )
    {
        message = m_aXMessageList.front();
        m_aXMessageList.pop_front();
    }
    return message;
}

void MailDispatcher::start()
{
    OSL_PRECOND(!isStarted(), "MailDispatcher is already started!");

    ::osl::ClearableMutexGuard thread_status_guard( m_aThreadStatusMutex );

    OSL_PRECOND(!m_bShutdownRequested, "MailDispatcher thread is shuting down already");

    if ( !m_bShutdownRequested )
    {
        m_bActive = true;
        m_aWakeupCondition.set();
        thread_status_guard.clear();

        MailDispatcherListenerContainer_t aClonedListenerList(cloneListener());
        std::for_each( aClonedListenerList.begin(), aClonedListenerList.end(),
                       GenericEventNotifier(&IMailDispatcherListener::started, this) );
    }
}

void MailDispatcher::stop()
{
    OSL_PRECOND(isStarted(), "MailDispatcher not started!");

    ::osl::ClearableMutexGuard thread_status_guard( m_aThreadStatusMutex );

    OSL_PRECOND(!m_bShutdownRequested, "MailDispatcher thread is shuting down already");

    if (!m_bShutdownRequested)
    {
        m_bActive = false;
        m_aWakeupCondition.reset();
        thread_status_guard.clear();

        MailDispatcherListenerContainer_t aClonedListenerList(cloneListener());
        std::for_each( aClonedListenerList.begin(), aClonedListenerList.end(),
                       GenericEventNotifier(&IMailDispatcherListener::stopped, this) );
    }
}

void MailDispatcher::shutdown()
{
    ::osl::MutexGuard thread_status_guard( m_aThreadStatusMutex );

    OSL_PRECOND(!m_bShutdownRequested, "MailDispatcher thread is shuting down already");

    m_bShutdownRequested = true;
    m_aWakeupCondition.set();
}


void MailDispatcher::addListener(::rtl::Reference<IMailDispatcherListener> const & listener)
{
    OSL_PRECOND(!m_bShutdownRequested, "MailDispatcher thread is shuting down already");

    ::osl::MutexGuard guard( m_aListenerContainerMutex );
    m_aListenerList.push_back( listener );
}

std::list< ::rtl::Reference<IMailDispatcherListener> > MailDispatcher::cloneListener()
{
    ::osl::MutexGuard guard( m_aListenerContainerMutex );
    return m_aListenerList;
}

void MailDispatcher::sendMailMessageNotifyListener(uno::Reference<mail::XMailMessage> const & message)
{
    try
    {
        m_xMailserver->sendMailMessage( message );
        MailDispatcherListenerContainer_t aClonedListenerList(cloneListener());
        std::for_each( aClonedListenerList.begin(), aClonedListenerList.end(),
                       MailDeliveryNotifier(this, message) );
    }
    catch (const mail::MailException& ex)
    {
        MailDispatcherListenerContainer_t aClonedListenerList(cloneListener());
        std::for_each( aClonedListenerList.begin(), aClonedListenerList.end(),
                       MailDeliveryErrorNotifier(this, message, ex.Message) );
    }
    catch (const uno::RuntimeException& ex)
    {
        MailDispatcherListenerContainer_t aClonedListenerList(cloneListener());
        std::for_each( aClonedListenerList.begin(), aClonedListenerList.end(),
                       MailDeliveryErrorNotifier(this, message, ex.Message) );
    }
}

void MailDispatcher::run()
{
    osl_setThreadName("MailDispatcher");

    // acquire a self reference in order to avoid race
    // conditions. The last client of this class must
    // call shutdown before releasing his last reference
    // to this class in order to shutdown this thread
    // which will release his (the very last reference
    // to the class and so force their destruction
    m_xSelfReference = this;

    // signal that the mail dispatcher thread is now alive
    m_aRunCondition.set();

    for(;;)
    {
        m_aWakeupCondition.wait();

        ::osl::ClearableMutexGuard thread_status_guard( m_aThreadStatusMutex );
        if ( m_bShutdownRequested )
            break;

        ::osl::ClearableMutexGuard message_container_guard( m_aMessageContainerMutex );

        if ( m_aXMessageList.size() )
        {
            thread_status_guard.clear();
            uno::Reference<mail::XMailMessage> message = m_aXMessageList.front();
            m_aXMessageList.pop_front();
            message_container_guard.clear();
            sendMailMessageNotifyListener( message );
        }
        else // idle - put ourself to sleep
        {
            m_aWakeupCondition.reset();
            message_container_guard.clear();
            thread_status_guard.clear();
            MailDispatcherListenerContainer_t aListenerListcloned( cloneListener() );
            std::for_each( aListenerListcloned.begin(), aListenerListcloned.end(),
                           GenericEventNotifier(&IMailDispatcherListener::idle, this) );
        }
    }
}

void MailDispatcher::onTerminated()
{
    //keep the reference until the end of onTerminated() because of the call order in the
    //_threadFunc() from osl/thread.hxx
    m_xSelfReference = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
