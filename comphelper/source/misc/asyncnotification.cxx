/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"
#include <comphelper/asyncnotification.hxx>
#include <osl/diagnose.h>
#include <osl/mutex.hxx>
#include <osl/conditn.hxx>
#include <comphelper/guarding.hxx>

#include <deque>
#include <set>
#include <functional>
#include <algorithm>

//........................................................................
namespace comphelper
{
//........................................................................

    //====================================================================
    //= AnyEvent
    //====================================================================
    //--------------------------------------------------------------------
    AnyEvent::AnyEvent()
        :m_refCount( 0 )
    {
    }

    //--------------------------------------------------------------------
    AnyEvent::~AnyEvent()
    {
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL AnyEvent::acquire()
    {
        return osl_incrementInterlockedCount( &m_refCount );
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL AnyEvent::release()
    {
        if ( 0 == osl_decrementInterlockedCount( &m_refCount ) )
        {
            delete this;
            return 0;
        }
        return m_refCount;
    }

    //====================================================================
    //= ProcessableEvent
    //====================================================================
    struct ProcessableEvent
    {
        AnyEventRef                         aEvent;
        ::rtl::Reference< IEventProcessor > xProcessor;

        ProcessableEvent( const AnyEventRef& _rEvent, const ::rtl::Reference< IEventProcessor >& _xProcessor )
            :aEvent( _rEvent )
            ,xProcessor( _xProcessor )
        {
        }

        ProcessableEvent( const ProcessableEvent& _rRHS )
            :aEvent( _rRHS.aEvent )
            ,xProcessor( _rRHS.xProcessor )
        {
        }

        ProcessableEvent& operator=( const ProcessableEvent& _rRHS )
        {
            aEvent = _rRHS.aEvent;
            xProcessor = _rRHS.xProcessor;
            return *this;
        }
    };

    //====================================================================
    typedef ::std::deque< ProcessableEvent >    EventQueue;

    //====================================================================
    struct EqualProcessor : public ::std::unary_function< ProcessableEvent, bool >
    {
        const ::rtl::Reference< IEventProcessor >&  rProcessor;
        EqualProcessor( const ::rtl::Reference< IEventProcessor >& _rProcessor ) :rProcessor( _rProcessor ) { }

        bool operator()( const ProcessableEvent& _rEvent )
        {
            return _rEvent.xProcessor.get() == rProcessor.get();
        }
    };

    //====================================================================
    //= EventNotifierImpl
    //====================================================================
    struct EventNotifierImpl
    {
        ::osl::Mutex        aMutex;
        oslInterlockedCount m_refCount;
        ::osl::Condition    aPendingActions;
        EventQueue          aEvents;
        ::std::set< ::rtl::Reference< IEventProcessor > >
                            m_aDeadProcessors;

        EventNotifierImpl()
            :m_refCount( 0 )
        {
        }

    private:
        EventNotifierImpl( const EventNotifierImpl& );              // never implemented
        EventNotifierImpl& operator=( const EventNotifierImpl& );   // never implemented
    };

    //====================================================================
    //= AsyncEventNotifier
    //====================================================================
    //--------------------------------------------------------------------
    AsyncEventNotifier::AsyncEventNotifier()
        :m_pImpl( new EventNotifierImpl )
    {
    }

    //--------------------------------------------------------------------
    AsyncEventNotifier::~AsyncEventNotifier()
    {
    }

    //--------------------------------------------------------------------
    void AsyncEventNotifier::removeEventsForProcessor( const ::rtl::Reference< IEventProcessor >& _xProcessor )
    {
        ::osl::MutexGuard aGuard( m_pImpl->aMutex );

        // remove all events for this processor
        ::std::remove_if( m_pImpl->aEvents.begin(), m_pImpl->aEvents.end(), EqualProcessor( _xProcessor ) );

        // and just in case that an event for exactly this processor has just been
        // popped from the queue, but not yet processed: remember it:
        m_pImpl->m_aDeadProcessors.insert( _xProcessor );
    }

    //--------------------------------------------------------------------
    void SAL_CALL AsyncEventNotifier::terminate()
    {
        ::osl::MutexGuard aGuard( m_pImpl->aMutex );

        // remember the termination request
        AsyncEventNotifier_TBASE::terminate();

        // awake the thread
        m_pImpl->aPendingActions.set();
    }

    //--------------------------------------------------------------------
    void AsyncEventNotifier::addEvent( const AnyEventRef& _rEvent, const ::rtl::Reference< IEventProcessor >& _xProcessor )
    {
        ::osl::MutexGuard aGuard( m_pImpl->aMutex );

        OSL_TRACE( "AsyncEventNotifier(%p): adding %p\n", this, _rEvent.get() );
        // remember this event
        m_pImpl->aEvents.push_back( ProcessableEvent( _rEvent, _xProcessor ) );

        // awake the thread
        m_pImpl->aPendingActions.set();
    }

    //--------------------------------------------------------------------
    void AsyncEventNotifier::run()
    {
        acquire();

        // keep us alive, in case we're terminated in the mid of the following
        ::rtl::Reference< AsyncEventNotifier > xKeepAlive( this );

        do
        {
            AnyEventRef aNextEvent;
            ::rtl::Reference< IEventProcessor > xNextProcessor;

            ::osl::ClearableMutexGuard aGuard( m_pImpl->aMutex );
            while ( m_pImpl->aEvents.size() > 0 )
            {
                ProcessableEvent aEvent( m_pImpl->aEvents.front() );
                aNextEvent = aEvent.aEvent;
                xNextProcessor = aEvent.xProcessor;
                m_pImpl->aEvents.pop_front();

                OSL_TRACE( "AsyncEventNotifier(%p): popping %p\n", this, aNextEvent.get() );

                if ( !aNextEvent.get() )
                    continue;

                // process the event, but only if it's processor did not die inbetween
                ::std::set< ::rtl::Reference< IEventProcessor > >::iterator deadPos = m_pImpl->m_aDeadProcessors.find( xNextProcessor );
                if ( deadPos != m_pImpl->m_aDeadProcessors.end() )
                {
                    m_pImpl->m_aDeadProcessors.erase( xNextProcessor );
                    xNextProcessor.clear();
                    OSL_TRACE( "AsyncEventNotifier(%p): removing %p\n", this, aNextEvent.get() );
                }

                // if there was a termination request (->terminate), respect it
                if ( !schedule() )
                    return;

                {
                    ::comphelper::MutexRelease aReleaseOnce( m_pImpl->aMutex );
                    if ( xNextProcessor.get() )
                        xNextProcessor->processEvent( *aNextEvent.get() );
                }
            }

            // if there was a termination request (->terminate), respect it
            if ( !schedule() )
                return;

            // wait for new events to process
            aGuard.clear();
            m_pImpl->aPendingActions.reset();
            m_pImpl->aPendingActions.wait();
        }
        while ( sal_True );
    }

    //--------------------------------------------------------------------
    void SAL_CALL AsyncEventNotifier::onTerminated()
    {
        AsyncEventNotifier_TBASE::onTerminated();
        // when we were started (->run), we acquired ourself. Release this now
        // that we were finally terminated
        release();
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL AsyncEventNotifier::acquire()
    {
        return osl_incrementInterlockedCount( &m_pImpl->m_refCount );
    }

    //--------------------------------------------------------------------
    oslInterlockedCount SAL_CALL AsyncEventNotifier::release()
    {
        if ( 0 == osl_decrementInterlockedCount( &m_pImpl->m_refCount ) )
        {
            delete this;
            return 0;
        }
        return m_pImpl->m_refCount;
    }

//........................................................................
} // namespace comphelper
//........................................................................

