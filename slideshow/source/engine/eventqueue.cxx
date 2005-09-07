/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eventqueue.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:25:42 $
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

// must be first
#include <canvas/debug.hxx>
#ifndef _CANVAS_VERBOSETRACE_HXX
#include <canvas/verbosetrace.hxx>
#endif
#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <event.hxx>
#include <eventqueue.hxx>
#include <slideshowexceptions.hxx>

#include <boost/shared_ptr.hpp>
#include <boost/mem_fn.hpp>
#include <queue>
#include <algorithm>
#include <limits>


using namespace ::com::sun::star;

namespace presentation
{
    namespace internal
    {
        bool EventQueue::EventEntry::operator<( const EventEntry& rEvent ) const
        {
            // negate comparison, we want priority queue to be sorted
            // in increasing order of activation times
            return this->nTime > rEvent.nTime;
        }


        EventQueue::EventQueue(
            boost::shared_ptr<canvas::tools::ElapsedTime> const & pPresTimer )
            : maEvents(),
              mpTimer( pPresTimer )
        {
        }

        EventQueue::~EventQueue()
        {
            // dispose event queue
            while( !maEvents.empty() )
            {
                try {
                    maEvents.top().pEvent->dispose();
                }
                catch (uno::Exception &) {
                    OSL_ENSURE( false, rtl::OUStringToOString(
                                    comphelper::anyToString(
                                        cppu::getCaughtException() ),
                                    RTL_TEXTENCODING_UTF8 ).getStr() );
                }
                maEvents.pop();
            }
        }

        bool EventQueue::addEvent( const EventSharedPtr& rEvent )
        {
            ENSURE_AND_RETURN( rEvent.get() != NULL,
                               "EventQueue::addEvent: event ptr NULL" );

            // prepare entry
            EventEntry entry;

            const double nCurrTime( mpTimer->getElapsedTime() );
            entry.pEvent = rEvent;
            entry.nTime  = rEvent->getActivationTime( nCurrTime );

            // A seemingly obvious optimization cannot be used here,
            // because it breaks assumed order of notification: zero
            // timeout events could be fired() immediately, but that
            // would not unwind the stack and furthermore changes
            // order of notification

            // add entry
            maEvents.push( entry );

            return true;
        }

        double EventQueue::process()
        {
            VERBOSE_TRACE( "EventQueue: heartbeat" );

            // perform topmost, ready-to-execute event
            // =======================================

            const double nCurrTime( mpTimer->getElapsedTime() );

            // process ready/elapsed events. Note that the 'perceived'
            // current time remains constant for this loop, thus we're
            // processing only those events which where ready when we
            // entered this method.
            while( !maEvents.empty() &&
                   maEvents.top().nTime <= nCurrTime )
            {
                EventEntry event( maEvents.top() );
                maEvents.pop();

                // only process event, if it is still 'charged',
                // i.e. the fire() call effects something. This is
                // used when e.g. having events registered at multiple
                // places, which should fire only once: after the
                // initial fire() call, those events become inactive
                // and return false on isCharged. This frees us from
                // the need to prune queues of those inactive shells.
                if( event.pEvent->isCharged() )
                {
                    try
                    {
#if OSL_DEBUG_LEVEL > 0
                        VERBOSE_TRACE( "Firing event: unknown (0x%X), timeout was: %f",
                                       event.pEvent.get(),
                                       event.pEvent->getActivationTime(0.0) );
#endif

                        event.pEvent->fire();
                    }
                    catch( uno::Exception& )
                    {
                        // catch anything here, we don't want
                        // to leave this scope under _any_
                        // circumstance. Although, do _not_
                        // reinsert an activity that threw
                        // once.

                        // NOTE: we explicitely don't catch(...) here,
                        // since this will also capture segmentation
                        // violations and the like. In such a case, we
                        // still better let our clients now...
                        OSL_TRACE( "::presentation::internal::EventQueue: Event threw a uno::Exception, action might not have been fully performed" );
                    }
                    catch( SlideShowException& )
                    {
                        // catch anything here, we don't want
                        // to leave this scope under _any_
                        // circumstance. Although, do _not_
                        // reinsert an activity that threw
                        // once.

                        // NOTE: we explicitely don't catch(...) here,
                        // since this will also capture segmentation
                        // violations and the like. In such a case, we
                        // still better let our clients now...
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

            // return time for next entry (if any)
            return maEvents.empty() ?
                ::std::numeric_limits<double>::max() :
                maEvents.top().nTime;
        }

        bool EventQueue::isEmpty()
        {
            return maEvents.empty();
        }

        void EventQueue::clear()
        {
            // TODO(P1): Maybe a plain vector and vector.swap will
            // be faster here. Profile.
            maEvents = ImplQueueType();
        }
    }
}
