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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#include "maildispatcher.hxx"
#include "imaildsplistener.hxx"

#include <algorithm>

using namespace ::com::sun::star;
using ::rtl::OUString;

typedef std::list< uno::Reference<mail::XMailMessage> > MailMessageContainer_t;
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
            ::rtl::Reference<MailDispatcher> mail_dispatcher) :
            notification_function_(notification_function),
            mail_dispatcher_(mail_dispatcher)
        {}

        void operator() (::rtl::Reference<IMailDispatcherListener> listener) const
        { (listener.get()->*notification_function_)(mail_dispatcher_); }

    private:
        GenericNotificationFunc_t notification_function_;
        ::rtl::Reference<MailDispatcher> mail_dispatcher_;
    };

    class MailDeliveryNotifier
    {
    public:
        MailDeliveryNotifier(::rtl::Reference<MailDispatcher> xMailDispatcher, uno::Reference<mail::XMailMessage> message) :
            mail_dispatcher_(xMailDispatcher),
            message_(message)
        {}

        void operator() (::rtl::Reference<IMailDispatcherListener> listener) const
        { listener->mailDelivered(mail_dispatcher_, message_); }

    private:
        ::rtl::Reference<MailDispatcher> mail_dispatcher_;
        uno::Reference<mail::XMailMessage> message_;
    };

    class MailDeliveryErrorNotifier
    {
    public:
        MailDeliveryErrorNotifier(
            ::rtl::Reference<MailDispatcher> xMailDispatcher,
            uno::Reference<mail::XMailMessage> message,
            const ::rtl::OUString& error_message) :
            mail_dispatcher_(xMailDispatcher),
            message_(message),
            error_message_(error_message)
        {}

        void operator() (::rtl::Reference<IMailDispatcherListener> listener) const
        { listener->mailDeliveryError(mail_dispatcher_, message_, error_message_); }

    private:
        ::rtl::Reference<MailDispatcher> mail_dispatcher_;
        uno::Reference<mail::XMailMessage> message_;
        ::rtl::OUString error_message_;
    };

} // namespace private

MailDispatcher::MailDispatcher(uno::Reference<mail::XSmtpService> mailserver) :
    mailserver_ (mailserver),
    run_(false),
    shutdown_requested_(false)
{
    wakening_call_.reset();
    mail_dispatcher_active_.reset();

    if (!create())
        throw uno::RuntimeException();

    // wait until the mail dispatcher thread is really alive
    // and has aquired a reference to this instance of the
    // class
    mail_dispatcher_active_.wait();
}

MailDispatcher::~MailDispatcher()
{
}

void MailDispatcher::enqueueMailMessage(uno::Reference<mail::XMailMessage> message)
{
    ::osl::MutexGuard thread_status_guard(thread_status_mutex_);
    ::osl::MutexGuard message_container_guard(message_container_mutex_);

    OSL_PRECOND(!shutdown_requested_, "MailDispatcher thread is shuting down already");

    messages_.push_back(message);
    if (run_)
        wakening_call_.set();
}

uno::Reference<mail::XMailMessage> MailDispatcher::dequeueMailMessage()
{
    ::osl::MutexGuard guard(message_container_mutex_);
    uno::Reference<mail::XMailMessage> message;
    if(!messages_.empty())
    {
        message = messages_.front();
        messages_.pop_front();
    }
    return message;
}

void MailDispatcher::start()
{
    OSL_PRECOND(!isStarted(), "MailDispatcher is already started!");

    ::osl::ClearableMutexGuard thread_status_guard(thread_status_mutex_);

    OSL_PRECOND(!shutdown_requested_, "MailDispatcher thread is shuting down already");

    if (!shutdown_requested_)
    {
        run_ = true;
        wakening_call_.set();
        thread_status_guard.clear();

        MailDispatcherListenerContainer_t listeners_cloned(cloneListener());
        std::for_each(listeners_cloned.begin(), listeners_cloned.end(), GenericEventNotifier(&IMailDispatcherListener::started, this));
    }
}

void MailDispatcher::stop()
{
    OSL_PRECOND(isStarted(), "MailDispatcher not started!");

    ::osl::ClearableMutexGuard thread_status_guard(thread_status_mutex_);

    OSL_PRECOND(!shutdown_requested_, "MailDispatcher thread is shuting down already");

    if (!shutdown_requested_)
    {
        run_ = false;
        wakening_call_.reset();
        thread_status_guard.clear();

        MailDispatcherListenerContainer_t listeners_cloned(cloneListener());
        std::for_each(listeners_cloned.begin(), listeners_cloned.end(), GenericEventNotifier(&IMailDispatcherListener::stopped, this));
    }
}

void MailDispatcher::shutdown()
{
    ::osl::MutexGuard thread_status_guard(thread_status_mutex_);

    OSL_PRECOND(!shutdown_requested_, "MailDispatcher thread is shuting down already");

    shutdown_requested_ = true;
    wakening_call_.set();
}

bool MailDispatcher::isStarted() const
{
    return run_;
}

void MailDispatcher::addListener(::rtl::Reference<IMailDispatcherListener> listener)
{
    OSL_PRECOND(!shutdown_requested_, "MailDispatcher thread is shuting down already");

    ::osl::MutexGuard guard(listener_container_mutex_);
    listeners_.push_back(listener);
}

void MailDispatcher::removeListener(::rtl::Reference<IMailDispatcherListener> listener)
{
    OSL_PRECOND(!shutdown_requested_, "MailDispatcher thread is shuting down already");

    ::osl::MutexGuard guard(listener_container_mutex_);
    listeners_.remove(listener);
}

std::list< ::rtl::Reference<IMailDispatcherListener> > MailDispatcher::cloneListener()
{
    ::osl::MutexGuard guard(listener_container_mutex_);
    return listeners_;
}

void MailDispatcher::sendMailMessageNotifyListener(uno::Reference<mail::XMailMessage> message)
{
    try
    {
        mailserver_->sendMailMessage(message);
        MailDispatcherListenerContainer_t listeners_cloned(cloneListener());
        std::for_each(listeners_cloned.begin(), listeners_cloned.end(), MailDeliveryNotifier(this, message));
    }
    catch (mail::MailException& ex)
    {
        MailDispatcherListenerContainer_t listeners_cloned(cloneListener());
        std::for_each(listeners_cloned.begin(), listeners_cloned.end(), MailDeliveryErrorNotifier(this, message, ex.Message));
    }
    catch (uno::RuntimeException& ex)
    {
        MailDispatcherListenerContainer_t listeners_cloned(cloneListener());
        std::for_each(listeners_cloned.begin(), listeners_cloned.end(), MailDeliveryErrorNotifier(this, message, ex.Message));
    }
}

void MailDispatcher::run()
{
    // aquire a self reference in order to avoid race
    // conditions. The last client of this class must
    // call shutdown before releasing his last reference
    // to this class in order to shutdown this thread
    // which will release his (the very last reference
    // to the class and so force their destruction
    m_xSelfReference = this;

    // signal that the mail dispatcher thread is now alive
    mail_dispatcher_active_.set();

    for(;;)
    {
        wakening_call_.wait();

        ::osl::ClearableMutexGuard thread_status_guard(thread_status_mutex_);
        if (shutdown_requested_)
           break;

        ::osl::ClearableMutexGuard message_container_guard(message_container_mutex_);

        if (messages_.size())
        {
            thread_status_guard.clear();
            uno::Reference<mail::XMailMessage> message = messages_.front();
            messages_.pop_front();
            message_container_guard.clear();
            sendMailMessageNotifyListener(message);
        }
        else // idle - put ourself to sleep
        {
            wakening_call_.reset();
            message_container_guard.clear();
            thread_status_guard.clear();
            MailDispatcherListenerContainer_t listeners_cloned(cloneListener());
            std::for_each(listeners_cloned.begin(), listeners_cloned.end(), GenericEventNotifier(&IMailDispatcherListener::idle, this));
        }
    } // end for        SSH ALI
}

void MailDispatcher::onTerminated()
{
    //keep the reference until the end of onTerminated() because of the call order in the
    //_threadFunc() from osl/thread.hxx
    m_xSelfReference = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
