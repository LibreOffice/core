/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */



#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
#include <canvas/verbosetrace.hxx>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <event.hxx>
#include <eventqueue.hxx>
#include <slideshowexceptions.hxx>

#include <boost/shared_ptr.hpp>
#include <limits>


using namespace ::com::sun::star;

namespace slideshow
{
    namespace internal
    {
        bool EventQueue::EventEntry::operator<( const EventEntry& rEvent ) const
        {
            
            
            return this->nTime > rEvent.nTime;
        }


        EventQueue::EventQueue(
            boost::shared_ptr<canvas::tools::ElapsedTime> const & pPresTimer )
            : maMutex(),
              maEvents(),
              maNextEvents(),
              maNextNextEvents(),
              mpTimer( pPresTimer )
        {
        }

        EventQueue::~EventQueue()
        {
            
            EventEntryVector::const_iterator const iEnd( maNextEvents.end() );
            for ( EventEntryVector::const_iterator iPos( maNextEvents.begin() );
                  iPos != iEnd; ++iPos )
            {
                maEvents.push(*iPos);
            }
            EventEntryVector().swap( maNextEvents );

            
            while( !maEvents.empty() )
            {
                try
                {
                    maEvents.top().pEvent->dispose();
                }
                catch (uno::Exception &)
                {
                    OSL_FAIL( OUStringToOString(
                                    comphelper::anyToString(
                                        cppu::getCaughtException() ),
                                    RTL_TEXTENCODING_UTF8 ).getStr() );
                }
                maEvents.pop();
            }
        }

        bool EventQueue::addEvent( const EventSharedPtr& rEvent )
        {
            ::osl::MutexGuard aGuard( maMutex );

            SAL_INFO("slideshow.eventqueue", "adding event \"" << rEvent->GetDescription()
                << "\" [" << rEvent.get()
                << "] at " << mpTimer->getElapsedTime()
                << " with delay " << rEvent->getActivationTime(0.0)
                );
            ENSURE_OR_RETURN_FALSE( rEvent,
                               "EventQueue::addEvent: event ptr NULL" );

            

            
            
            
            
            

            
            maEvents.push( EventEntry( rEvent, rEvent->getActivationTime(
                                           mpTimer->getElapsedTime()) ) );
            return true;
        }

        bool EventQueue::addEventForNextRound( EventSharedPtr const& rEvent )
        {
            ::osl::MutexGuard aGuard( maMutex );

            SAL_INFO("slideshow.eventqueue", "adding event \"" << rEvent->GetDescription()
                << "\" [" << rEvent.get()
                << "] for the next round at " << mpTimer->getElapsedTime()
                << " with delay " << rEvent->getActivationTime(0.0)
                );

            ENSURE_OR_RETURN_FALSE( rEvent.get() != NULL,
                               "EventQueue::addEvent: event ptr NULL" );
            maNextEvents.push_back(
                EventEntry( rEvent, rEvent->getActivationTime(
                                mpTimer->getElapsedTime()) ) );
            return true;
        }

        bool EventQueue::addEventWhenQueueIsEmpty (const EventSharedPtr& rpEvent)
        {
            ::osl::MutexGuard aGuard( maMutex );

            SAL_INFO("slideshow.eventqueue", "adding event \"" << rpEvent->GetDescription()
                << "\" [" << rpEvent.get()
                << "] for execution when the queue is empty at " << mpTimer->getElapsedTime()
                << " with delay " << rpEvent->getActivationTime(0.0)
                );

            ENSURE_OR_RETURN_FALSE(
                rpEvent.get() != NULL,
                    "EventQueue::addEvent: event ptr NULL");

            maNextNextEvents.push(
                EventEntry(
                    rpEvent,
                    rpEvent->getActivationTime(mpTimer->getElapsedTime())));

            return true;
        }

        void EventQueue::forceEmpty()
        {
            ::osl::MutexGuard aGuard( maMutex );

            process_(true);
        }

        void EventQueue::process()
        {
            ::osl::MutexGuard aGuard( maMutex );

            process_(false);
        }

        void EventQueue::process_( bool bFireAllEvents )
        {
            VERBOSE_TRACE( "EventQueue: heartbeat" );

            
            EventEntryVector::const_iterator const iEnd( maNextEvents.end() );
            for ( EventEntryVector::const_iterator iPos( maNextEvents.begin() );
                  iPos != iEnd; ++iPos ) {
                maEvents.push(*iPos);
            }
            EventEntryVector().swap( maNextEvents );

            
            

            const double nCurrTime( mpTimer->getElapsedTime() );

            
            
            if (!maNextNextEvents.empty()
                && !bFireAllEvents
                && (maEvents.empty() || maEvents.top().nTime > nCurrTime))
            {
                const EventEntry aEvent (maNextNextEvents.top());
                maNextNextEvents.pop();
                maEvents.push(aEvent);
            }

            
            
            
            
            while( !maEvents.empty() &&
                   (bFireAllEvents || maEvents.top().nTime <= nCurrTime) )
            {
                EventEntry event( maEvents.top() );
                maEvents.pop();

                
                
                
                
                
                
                
                if( event.pEvent->isCharged() )
                {
                    try
                    {
                        SAL_INFO("slideshow.eventqueue", "firing event \""
                                << event.pEvent->GetDescription()
                                << "\" [" << event.pEvent.get()
                                << "] at " << mpTimer->getElapsedTime()
                                << " with delay " << event.pEvent->getActivationTime(0.0)
                                );
                        event.pEvent->fire();
                        SAL_INFO("slideshow.eventqueue", "event \""
                                << event.pEvent->GetDescription()
                                << "\" [" << event.pEvent.get() << "] fired"
                                );
                    }
                    catch( uno::RuntimeException& )
                    {
                        throw;
                    }
                    catch( uno::Exception& )
                    {
                        
                        
                        
                        
                        

                        
                        
                        
                        
                        OSL_FAIL( OUStringToOString(
                                        comphelper::anyToString( cppu::getCaughtException() ),
                                        RTL_TEXTENCODING_UTF8 ).getStr() );
                    }
                    catch( SlideShowException& )
                    {
                        
                        
                        
                        
                        

                        
                        
                        
                        
                        OSL_TRACE( "::presentation::internal::EventQueue: Event threw a SlideShowException, action might not have been fully performed" );
                    }
                }
                else
                {
#if OSL_DEBUG_LEVEL > 0
                    VERBOSE_TRACE( "Ignoring discharged event: unknown (0x%X), timeout was: %f",
                                   event.pEvent.get(),
                                   event.pEvent->getActivationTime(0.0) );
#endif
                }
            }
        }

        bool EventQueue::isEmpty() const
        {
            ::osl::MutexGuard aGuard( maMutex );

            return maEvents.empty() && maNextEvents.empty() && maNextNextEvents.empty();
        }

        double EventQueue::nextTimeout() const
        {
            ::osl::MutexGuard aGuard( maMutex );

            
            double nTimeout (::std::numeric_limits<double>::max());
            const double nCurrentTime (mpTimer->getElapsedTime());
            if ( ! maEvents.empty())
                nTimeout = maEvents.top().nTime - nCurrentTime;
            if ( ! maNextEvents.empty())
                nTimeout = ::std::min(nTimeout, maNextEvents.front().nTime - nCurrentTime);
            if ( ! maNextNextEvents.empty())
                nTimeout = ::std::min(nTimeout, maNextNextEvents.top().nTime - nCurrentTime);

            return nTimeout;
        }

        void EventQueue::clear()
        {
            ::osl::MutexGuard aGuard( maMutex );

            
            
            maEvents = ImplQueueType();

            maNextEvents.clear();
            maNextNextEvents = ImplQueueType();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
