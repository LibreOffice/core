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

#include <comphelper/asyncnotification.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <rtl/instance.hxx>
#include <comphelper/guarding.hxx>

#include <cassert>
#include <deque>
#include <vector>
#include <functional>
#include <algorithm>

namespace comphelper
{
    AnyEvent::AnyEvent()
    {
    }

    AnyEvent::~AnyEvent()
    {
    }

    struct ProcessableEvent
    {
        AnyEventRef                         aEvent;
        ::rtl::Reference< IEventProcessor > xProcessor;

        ProcessableEvent()
        {
        }

        ProcessableEvent( const AnyEventRef& _rEvent, const ::rtl::Reference< IEventProcessor >& _xProcessor )
            :aEvent( _rEvent )
            ,xProcessor( _xProcessor )
        {
        }
    };


    typedef ::std::deque< ProcessableEvent >    EventQueue;


    struct EqualProcessor : public ::std::unary_function< ProcessableEvent, bool >
    {
        const ::rtl::Reference< IEventProcessor >&  rProcessor;
        explicit EqualProcessor( const ::rtl::Reference< IEventProcessor >& _rProcessor ) :rProcessor( _rProcessor ) { }

        bool operator()( const ProcessableEvent& _rEvent )
        {
            return _rEvent.xProcessor.get() == rProcessor.get();
        }
    };


    struct EventNotifierImpl
    {
        ::osl::Mutex        aMutex;
        ::osl::Condition    aPendingActions;
        EventQueue          aEvents;
        bool                bTerminate;
        // only used for AsyncEventNotifierAutoJoin
        char const*         name;
        std::shared_ptr<AsyncEventNotifierAutoJoin> pKeepThisAlive;

        EventNotifierImpl()
            :bTerminate( false )
        {
        }
    };

    AsyncEventNotifierBase::AsyncEventNotifierBase()
        : m_xImpl(new EventNotifierImpl)
    {
    }


    AsyncEventNotifierBase::~AsyncEventNotifierBase()
    {
    }


    void AsyncEventNotifierBase::removeEventsForProcessor( const ::rtl::Reference< IEventProcessor >& _xProcessor )
    {
        ::osl::MutexGuard aGuard( m_xImpl->aMutex );

        // remove all events for this processor
        m_xImpl->aEvents.erase(::std::remove_if( m_xImpl->aEvents.begin(), m_xImpl->aEvents.end(), EqualProcessor( _xProcessor ) ), m_xImpl->aEvents.end());
    }


    void SAL_CALL AsyncEventNotifierBase::terminate()
    {
        ::osl::MutexGuard aGuard( m_xImpl->aMutex );

        // remember the termination request
        m_xImpl->bTerminate = true;

        // awake the thread
        m_xImpl->aPendingActions.set();
    }


    void AsyncEventNotifierBase::addEvent( const AnyEventRef& _rEvent, const ::rtl::Reference< IEventProcessor >& _xProcessor )
    {
        ::osl::MutexGuard aGuard( m_xImpl->aMutex );

        OSL_TRACE( "AsyncEventNotifier(%p): adding %p", this, _rEvent.get() );
        // remember this event
        m_xImpl->aEvents.push_back( ProcessableEvent( _rEvent, _xProcessor ) );

        // awake the thread
        m_xImpl->aPendingActions.set();
    }


    void AsyncEventNotifierBase::execute()
    {
        for (;;)
        {
            m_xImpl->aPendingActions.wait();
            ProcessableEvent aEvent;
            {
                osl::MutexGuard aGuard(m_xImpl->aMutex);
                if (m_xImpl->bTerminate)
                {
                    break;
                }
                if (!m_xImpl->aEvents.empty())
                {
                    aEvent = m_xImpl->aEvents.front();
                    m_xImpl->aEvents.pop_front();
                    OSL_TRACE(
                        "AsyncEventNotifier(%p): popping %p", this,
                        aEvent.aEvent.get());
                }
                if (m_xImpl->aEvents.empty())
                {
                    m_xImpl->aPendingActions.reset();
                }
            }
            if (aEvent.aEvent.is()) {
                assert(aEvent.xProcessor.is());
                aEvent.xProcessor->processEvent(*aEvent.aEvent);
            }
        }
    }

    AsyncEventNotifier::AsyncEventNotifier(char const* name)
        : salhelper::Thread(name)
    {
    }

    AsyncEventNotifier::~AsyncEventNotifier()
    {
    }

    void AsyncEventNotifier::execute()
    {
        return AsyncEventNotifierBase::execute();
    }

    void AsyncEventNotifier::terminate()
    {
        return AsyncEventNotifierBase::terminate();
    }

    struct theNotifiersMutex : public rtl::Static<osl::Mutex, theNotifiersMutex> {};
    static std::vector<std::weak_ptr<AsyncEventNotifierAutoJoin>> g_Notifiers;

    void JoinAsyncEventNotifiers()
    {
        std::vector<std::weak_ptr<AsyncEventNotifierAutoJoin>> notifiers;
        {
            ::osl::MutexGuard g(theNotifiersMutex::get());
            notifiers = g_Notifiers;
        }
        for (std::weak_ptr<AsyncEventNotifierAutoJoin> const& wNotifier : notifiers)
        {
            std::shared_ptr<AsyncEventNotifierAutoJoin> const pNotifier(
                    wNotifier.lock());
            if (pNotifier)
            {
                pNotifier->terminate();
                pNotifier->join();
            }
        }
        // note it's possible that g_Notifiers isn't empty now in case of leaks,
        // particularly since the UNO service manager isn't disposed yet
    }

    AsyncEventNotifierAutoJoin::AsyncEventNotifierAutoJoin(char const* name)
    {
        m_xImpl->name = name;
    }

    AsyncEventNotifierAutoJoin::~AsyncEventNotifierAutoJoin()
    {
        ::osl::MutexGuard g(theNotifiersMutex::get());
        // note: this doesn't happen atomically with the refcount
        // hence it's possible this deletes > 1 or 0 elements
        g_Notifiers.erase(
            std::remove_if(g_Notifiers.begin(), g_Notifiers.end(),
                [](std::weak_ptr<AsyncEventNotifierAutoJoin> const& w) {
                    return w.expired();
                } ),
            g_Notifiers.end());
    }

    std::shared_ptr<AsyncEventNotifierAutoJoin>
    AsyncEventNotifierAutoJoin::newAsyncEventNotifierAutoJoin(char const* name)
    {
        std::shared_ptr<AsyncEventNotifierAutoJoin> const ret(
                new AsyncEventNotifierAutoJoin(name));
        ::osl::MutexGuard g(theNotifiersMutex::get());
        g_Notifiers.push_back(ret);
        return ret;
    }

    void AsyncEventNotifierAutoJoin::terminate()
    {
        return AsyncEventNotifierBase::terminate();
    }

    void AsyncEventNotifierAutoJoin::launch(std::shared_ptr<AsyncEventNotifierAutoJoin> const& xThis)
    {
        // see salhelper::Thread::launch
        xThis->m_xImpl->pKeepThisAlive = xThis;
        try {
            if (!xThis->create()) {
                throw std::runtime_error("osl::Thread::create failed");
            }
        } catch (...) {
            xThis->m_xImpl->pKeepThisAlive.reset();
            throw;
        }
    }

    void AsyncEventNotifierAutoJoin::run()
    {
        // see salhelper::Thread::run
        try {
            setName(m_xImpl->name);
            execute();
        } catch (...) {
            onTerminated();
            throw;
        }
    }

    void AsyncEventNotifierAutoJoin::onTerminated()
    {
        // try to delete "this"
        m_xImpl->pKeepThisAlive.reset();
        return osl::Thread::onTerminated();
    }

} // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
