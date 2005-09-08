/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: eventhandler.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:03:11 $
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

#ifndef _SDR_EVENT_EVENTHANDLER_HXX
#define _SDR_EVENT_EVENTHANDLER_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#include <vector>

#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// predeclarations

namespace sdr
{
    namespace event
    {
        class BaseEvent;
        class EventHandler;

        // typedefs for a list of BaseEvents
        typedef ::std::vector< BaseEvent* > BaseEventVector;
    } // end of namespace event
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace event
    {
        class BaseEvent
        {
            // the EventHandler this event is registered at
            EventHandler&                                   mrEventHandler;

        public:
            // basic constructor.
            BaseEvent(EventHandler& rEventHandler);

            // destructor
            virtual ~BaseEvent();

            // the called method if the event is triggered
            virtual void ExecuteEvent() = 0;
        };
    } // end of namespace event
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace event
    {
        class EventHandler
        {
            BaseEventVector                                 maVector;

            // to allow BaseEvents to use the add/remove functionality
            friend class BaseEvent;

            // methods to add/remove events. These are private since
            // they are used from BaseEvent only.
            void AddEvent(BaseEvent& rBaseEvent);
            void RemoveEvent(BaseEvent& rBaseEvent);

            // access to a event, 0L when no more events
            BaseEvent* GetEvent();

        public:
            // basic constructor.
            EventHandler();

            // destructor
            virtual ~EventHandler();

            // Trigger and consume the events
            virtual void ExecuteEvents();

            // for control
            sal_Bool IsEmpty() const;
        };
    } // end of namespace event
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace event
    {
        class TimerEventHandler : public EventHandler, public Timer
        {
        public:
            // basic constructor.
            TimerEventHandler(sal_uInt32 nTimeout = 1L);

            // destructor
            virtual ~TimerEventHandler();

            // The timer when it is triggered; from class Timer
            virtual void Timeout();

            // reset the timer
            void Restart();
        };
    } // end of namespace event
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_EVENT_EVENTHANDLER_HXX

// eof
