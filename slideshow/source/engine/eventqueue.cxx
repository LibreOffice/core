/*************************************************************************
 *
 *  $RCSfile: eventqueue.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 18:53:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// must be first
#include <canvas/debug.hxx>

#ifndef _CANVAS_VERBOSETRACE_HXX
#include <canvas/verbosetrace.hxx>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif
#ifndef BOOST_MEM_FN_HPP_INCLUDED
#include <boost/mem_fn.hpp>
#endif

#include <queue>
#include <algorithm>
#include <limits>

#include <event.hxx>
#include <eventqueue.hxx>
#include <slideshowexceptions.hxx>


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


        EventQueue::EventQueue() :
            maEvents(),
            maElapsedTime()
        {
        }

        EventQueue::~EventQueue()
        {
            // dispose event queue
            while( !maEvents.empty() )
            {
                maEvents.top().pEvent->dispose();
                maEvents.pop();
            }
        }

        bool EventQueue::addEvent( const EventSharedPtr& rEvent )
        {
            OSL_ENSURE( rEvent.get() != NULL, "EventQueue::addEvent: event ptr NULL" );

            if( rEvent.get() == NULL )
                return false;

            // prepare entry
            EventEntry entry;

            const double nCurrTime( maElapsedTime.getElapsedTime() );
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

        void EventQueue::process( double* pTimeoutForNextCall )
        {
            VERBOSE_TRACE( "EventQueue: heartbeat" );

            // perform topmost, ready-to-execute event
            // =======================================

            const double nCurrTime( maElapsedTime.getElapsedTime() );

            // process ready/elapsed events. Note that the 'perceived'
            // current time remains constant for this loop, thus we're
            // processing only those events which where ready when we
            // entered this method.
            while( !maEvents.empty() &&
                   maEvents.top().nTime <= nCurrTime )
            {
                EventEntry event( maEvents.top() );
                maEvents.pop();

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

            // setup optional timeout value
            if( pTimeoutForNextCall )
            {
                if( maEvents.empty() )
                {
                    // no further events available,
                    *pTimeoutForNextCall = ::std::numeric_limits<double>::max();
                }
                else
                {
                    // ensure nothing negative is returned. Fetch
                    // fresh current time, event processing above
                    // might have taken significant time
                    *pTimeoutForNextCall = ::std::max(0.0,
                                                      maEvents.top().nTime - maElapsedTime.getElapsedTime());
                }
            }
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
