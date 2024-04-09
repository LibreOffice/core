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
#include <comphelper/scopeguard.hxx>
#include <mutex>
#include <condition_variable>

#include <cassert>
#include <stdexcept>
#include <vector>
#include <algorithm>

namespace comphelper
{
    AnyEvent::AnyEvent()
    {
    }

    AnyEvent::~AnyEvent()
    {
    }

    namespace {

    struct ProcessableEvent
    {
        AnyEventRef                         aEvent;
        ::rtl::Reference< IEventProcessor > xProcessor;
    };

    struct EqualProcessor
    {
        const ::rtl::Reference< IEventProcessor >&  rProcessor;
        explicit EqualProcessor( const ::rtl::Reference< IEventProcessor >& _rProcessor ) :rProcessor( _rProcessor ) { }

        bool operator()( const ProcessableEvent& _rEvent )
        {
            return _rEvent.xProcessor.get() == rProcessor.get();
        }
    };

    }

    struct EventNotifierImpl
    {
        std::mutex        aMutex;
        std::condition_variable aPendingActions;
        std::vector< ProcessableEvent > aEvents;
        bool                bTerminate;
        // only used for AsyncEventNotifierAutoJoin
        char const*         name;
        std::shared_ptr<AsyncEventNotifierAutoJoin> pKeepThisAlive;

        EventNotifierImpl()
            : bTerminate(false)
            , name(nullptr)
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
        std::scoped_lock aGuard( m_xImpl->aMutex );

        // remove all events for this processor
        std::erase_if( m_xImpl->aEvents, EqualProcessor( _xProcessor ) );
    }


    void SAL_CALL AsyncEventNotifierBase::terminate()
    {
        std::scoped_lock aGuard( m_xImpl->aMutex );

        // remember the termination request
        m_xImpl->bTerminate = true;

        // awake the thread
        m_xImpl->aPendingActions.notify_all();
    }


    void AsyncEventNotifierBase::addEvent( const AnyEventRef& _rEvent, const ::rtl::Reference< IEventProcessor >& _xProcessor )
    {
        std::scoped_lock aGuard( m_xImpl->aMutex );

        // remember this event
        m_xImpl->aEvents.emplace_back( ProcessableEvent {_rEvent, _xProcessor} );

        // awake the thread
        m_xImpl->aPendingActions.notify_all();
    }


    void AsyncEventNotifierBase::execute()
    {
        for (;;)
        {
            std::vector< ProcessableEvent > aEvents;
            {
                std::unique_lock aGuard(m_xImpl->aMutex);
                m_xImpl->aPendingActions.wait(aGuard,
                    [this] { return m_xImpl->bTerminate || !m_xImpl->aEvents.empty(); } );
                if (m_xImpl->bTerminate)
                    return;
                else
                    std::swap(aEvents, m_xImpl->aEvents);
            }
            for (ProcessableEvent& rEvent : aEvents)
            {
                assert(rEvent.xProcessor.is());
                rEvent.xProcessor->processEvent(*rEvent.aEvent);
            }
            aEvents.clear();
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

    namespace {

    std::mutex& GetTheNotifiersMutex()
    {
        static std::mutex MUTEX;
        return MUTEX;
    }

    }

    static std::vector<std::weak_ptr<AsyncEventNotifierAutoJoin>> g_Notifiers;

    void JoinAsyncEventNotifiers()
    {
        std::vector<std::weak_ptr<AsyncEventNotifierAutoJoin>> notifiers;
        {
            std::scoped_lock g(GetTheNotifiersMutex());
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
        std::scoped_lock g(GetTheNotifiersMutex());
        // note: this doesn't happen atomically with the refcount
        // hence it's possible this deletes > 1 or 0 elements
        std::erase_if(g_Notifiers,
                [](std::weak_ptr<AsyncEventNotifierAutoJoin> const& w) {
                    return w.expired();
                });
    }

    std::shared_ptr<AsyncEventNotifierAutoJoin>
    AsyncEventNotifierAutoJoin::newAsyncEventNotifierAutoJoin(char const* name)
    {
        std::shared_ptr<AsyncEventNotifierAutoJoin> const ret(
                new AsyncEventNotifierAutoJoin(name));
        std::scoped_lock g(GetTheNotifiersMutex());
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
        comphelper::ScopeGuard g([&xThis] { xThis->m_xImpl->pKeepThisAlive.reset(); });
        if (!xThis->create()) {
            throw std::runtime_error("osl::Thread::create failed");
        }
        g.dismiss();
    }

    void AsyncEventNotifierAutoJoin::run()
    {
        // see salhelper::Thread::run
        comphelper::ScopeGuard g([this] { onTerminated(); });
        setName(m_xImpl->name);
        execute();
        g.dismiss();
    }

    void AsyncEventNotifierAutoJoin::onTerminated()
    {
        // try to delete "this"
        m_xImpl->pKeepThisAlive.reset();
    }

} // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
