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

#include "eventhandler.hxx"

// for SOLARIS compiler include of algorithm part of _STL is necessary to
// get access to basic algos like ::std::find
#include <algorithm>
#include <tools/debug.hxx>

namespace sdr
{
    namespace event
    {
        BaseEvent::BaseEvent(TimerEventHandler& rEventHandler)
        :   mrEventHandler(rEventHandler)
        {
            mrEventHandler.AddEvent(*this);
        }

        BaseEvent::~BaseEvent()
        {
            mrEventHandler.RemoveEvent(*this);
        }
    } // end of namespace mixer
} // end of namespace sdr


namespace sdr
{
    namespace event
    {
        void TimerEventHandler::AddEvent(BaseEvent& rBaseEvent)
        {
            maVector.push_back(&rBaseEvent);
        }

        void TimerEventHandler::RemoveEvent(BaseEvent& rBaseEvent)
        {
            if(maVector.back() == &rBaseEvent)
            {
                // the one to remove is the last, pop
                maVector.pop_back();
            }
            else
            {
                const auto aFindResult = ::std::find(
                    maVector.begin(), maVector.end(), &rBaseEvent);
                DBG_ASSERT(aFindResult != maVector.end(),
                    "EventHandler::RemoveEvent: Event to be removed not found (!)");
                maVector.erase(aFindResult);
            }
        }

        BaseEvent* TimerEventHandler::GetEvent()
        {
            if(!maVector.empty())
            {
                // get the last event, that one is fastest to be removed
                return maVector.back();
            }
            else
            {
                return nullptr;
            }
        }

        TimerEventHandler::TimerEventHandler()
        {
            SetPriority(SchedulerPriority::HIGH_IDLE);
            Stop();
        }

        TimerEventHandler::~TimerEventHandler()
        {
            Stop();
            while(!maVector.empty())
            {
                delete GetEvent();
            }
        }

        // Trigger and consume the events
        void TimerEventHandler::ExecuteEvents()
        {
            for(;;)
            {
                BaseEvent* pEvent = GetEvent();
                if(pEvent == nullptr)
                    break;
                pEvent->ExecuteEvent();
                delete pEvent;
            }
        }

        // for control
        bool TimerEventHandler::IsEmpty() const
        {
            return (0L == maVector.size());
        }

        // The timer when it is triggered; from class Timer
        void TimerEventHandler::Invoke()
        {
            ExecuteEvents();
        }

        // reset the timer
        void TimerEventHandler::Restart()
        {
            Start();
        }
    } // end of namespace mixer
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
