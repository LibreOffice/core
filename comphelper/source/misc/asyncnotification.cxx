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
#include <comphelper/guarding.hxx>

#include <cassert>
#include <deque>
#include <functional>
#include <algorithm>


namespace comphelper
{



    //= AnyEvent


    AnyEvent::AnyEvent()
    {
    }


    AnyEvent::~AnyEvent()
    {
    }


    //= ProcessableEvent

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
        EqualProcessor( const ::rtl::Reference< IEventProcessor >& _rProcessor ) :rProcessor( _rProcessor ) { }

        bool operator()( const ProcessableEvent& _rEvent )
        {
            return _rEvent.xProcessor.get() == rProcessor.get();
        }
    };


    //= EventNotifierImpl

    struct EventNotifierImpl
    {
        ::osl::Mutex        aMutex;
        ::osl::Condition    aPendingActions;
        EventQueue          aEvents;
        bool                bTerminate;

        EventNotifierImpl()
            :bTerminate( false )
        {
        }
    };


    //= AsyncEventNotifier


    AsyncEventNotifier::AsyncEventNotifier(char const * name):
        Thread(name), m_pImpl(new EventNotifierImpl)
    {
    }


    AsyncEventNotifier::~AsyncEventNotifier()
    {
    }


    void AsyncEventNotifier::removeEventsForProcessor( const ::rtl::Reference< IEventProcessor >& _xProcessor )
    {
        ::osl::MutexGuard aGuard( m_pImpl->aMutex );

        // remove all events for this processor
        m_pImpl->aEvents.erase(::std::remove_if( m_pImpl->aEvents.begin(), m_pImpl->aEvents.end(), EqualProcessor( _xProcessor ) ), m_pImpl->aEvents.end());
    }


    void SAL_CALL AsyncEventNotifier::terminate()
    {
        ::osl::MutexGuard aGuard( m_pImpl->aMutex );

        // remember the termination request
        m_pImpl->bTerminate = true;

        // awake the thread
        m_pImpl->aPendingActions.set();
    }


    void AsyncEventNotifier::addEvent( const AnyEventRef& _rEvent, const ::rtl::Reference< IEventProcessor >& _xProcessor )
    {
        ::osl::MutexGuard aGuard( m_pImpl->aMutex );

        OSL_TRACE( "AsyncEventNotifier(%p): adding %p", this, _rEvent.get() );
        // remember this event
        m_pImpl->aEvents.push_back( ProcessableEvent( _rEvent, _xProcessor ) );

        // awake the thread
        m_pImpl->aPendingActions.set();
    }


    void AsyncEventNotifier::execute()
    {
        for (;;)
        {
            m_pImpl->aPendingActions.wait();
            ProcessableEvent aEvent;
            {
                osl::MutexGuard aGuard(m_pImpl->aMutex);
                if (m_pImpl->bTerminate)
                {
                    break;
                }
                if (!m_pImpl->aEvents.empty())
                {
                    aEvent = m_pImpl->aEvents.front();
                    m_pImpl->aEvents.pop_front();
                    OSL_TRACE(
                        "AsyncEventNotifier(%p): popping %p", this,
                        aEvent.aEvent.get());
                }
                if (m_pImpl->aEvents.empty())
                {
                    m_pImpl->aPendingActions.reset();
                }
            }
            if (aEvent.aEvent.is()) {
                assert(aEvent.xProcessor.is());
                aEvent.xProcessor->processEvent(*aEvent.aEvent);
            }
        }
    }

} // namespace comphelper


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
