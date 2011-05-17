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

#ifndef _SDR_EVENT_EVENTHANDLER_HXX
#define _SDR_EVENT_EVENTHANDLER_HXX

#include <sal/types.h>

#include <vector>
#include <vcl/timer.hxx>

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
