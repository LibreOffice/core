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

#ifndef INCLUDED_SVX_SOURCE_INC_EVENTHANDLER_HXX
#define INCLUDED_SVX_SOURCE_INC_EVENTHANDLER_HXX

#include <sal/types.h>

#include <vector>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>

namespace sdr
{
    namespace event
    {
        class BaseEvent;
        class EventHandler;

        typedef ::std::vector< BaseEvent* > BaseEventVector;
    } // end of namespace event
} // end of namespace sdr

namespace sdr
{
    namespace event
    {
        class BaseEvent
        {
            // the EventHandler this event is registered at
            EventHandler& mrEventHandler;

        public:
            BaseEvent(EventHandler& rEventHandler);

            virtual ~BaseEvent();

            // the called method if the event is triggered
            virtual void ExecuteEvent() = 0;
        };
    } // end of namespace event
} // end of namespace sdr

namespace sdr
{
    namespace event
    {
        class EventHandler
        {
            BaseEventVector maVector;

            // to allow BaseEvents to use the add/remove functionality
            friend class BaseEvent;

            // methods to add/remove events. These are private since
            // they are used from BaseEvent only.
            void AddEvent(BaseEvent& rBaseEvent);
            void RemoveEvent(BaseEvent& rBaseEvent);

            // access to a event, 0L when no more events
            BaseEvent* GetEvent();

        public:
            EventHandler();

            virtual ~EventHandler();

            // Trigger and consume the events
            void ExecuteEvents();

            bool IsEmpty() const;
        };
    } // end of namespace event
} // end of namespace sdr

namespace sdr
{
    namespace event
    {
        class TimerEventHandler : public EventHandler, public Idle
        {
        public:
            TimerEventHandler();

            virtual ~TimerEventHandler();

            // The timer when it is triggered; from class Timer
            virtual void Invoke() override;

            // reset the timer
            void Restart();
        };
    } // end of namespace event
} // end of namespace sdr

#endif // INCLUDED_SVX_SOURCE_INC_EVENTHANDLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
