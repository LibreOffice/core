/*************************************************************************
 *
 *  $RCSfile: eventqueue.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: thb $ $Date: 2004-03-18 10:44:31 $
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

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CANVAS_VERBOSETRACE_HXX
#include <canvas/verbosetrace.hxx>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <external/boost/shared_ptr.hpp>
#endif

#include <queue>

#include "event.hxx"
#include "eventqueue.hxx"


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

        bool EventQueue::addEvent( const EventSharedPtr& rEvent )
        {
            OSL_ENSURE( rEvent.get() != NULL, "EventQueue::addEvent: event ptr NULL" );

            if( rEvent.get() == NULL )
                return false;

            // prepare entry
            EventEntry entry;

            entry.pEvent = rEvent;
            entry.nTime  = rEvent->getActivationTime( maElapsedTime.getElapsedTime() );

            // add entry
            maEvents.push( entry );

            return true;
        }

        void EventQueue::process()
        {
            VERBOSE_TRACE( "EventQueue: heartbeat" );

            // perform topmost, ready-to-execute event
            // =======================================

            // process ready events
            if( !maEvents.empty() &&
                maEvents.top().nTime <= maElapsedTime.getElapsedTime() )
            {
                EventEntry event( maEvents.top() );
                maEvents.pop();

                try
                {
                    event.pEvent->fire();
                }
                catch(...)
                {
                    // catch anything here, we don't want
                    // to leave this frame under _any_
                    // circumstance.
                    OSL_TRACE( "::presentation::internal::EventQueue: Event threw, action might not have been fully performed" );
                }
            }
        }

        bool EventQueue::isEmpty()
        {
            return maEvents.empty();
        }

    }
}
