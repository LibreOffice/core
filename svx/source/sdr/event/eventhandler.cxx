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

#include <svx/sdr/event/eventhandler.hxx>

// for SOLARIS compiler include of algorithm part of _STL is necesary to
// get access to basic algos like ::std::find
#include <algorithm>
#include <tools/debug.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace event
    {
        BaseEvent::BaseEvent(EventHandler& rEventHandler)
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

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace event
    {
        void EventHandler::AddEvent(BaseEvent& rBaseEvent)
        {
            maVector.push_back(&rBaseEvent);
        }

        void EventHandler::RemoveEvent(BaseEvent& rBaseEvent)
        {
            if(maVector.back() == &rBaseEvent)
            {
                // the one to remove is the last, pop
                maVector.pop_back();
            }
            else
            {
                const BaseEventVector::iterator aFindResult = ::std::find(
                    maVector.begin(), maVector.end(), &rBaseEvent);
                DBG_ASSERT(aFindResult != maVector.end(),
                    "EventHandler::RemoveEvent: Event to be removed not found (!)");
                maVector.erase(aFindResult);
            }
        }

        BaseEvent* EventHandler::GetEvent()
        {
            if(!maVector.empty())
            {
                // get the last event, that one is fastest to be removed
                return maVector.back();
            }
            else
            {
                return 0L;
            }
        }

        EventHandler::EventHandler()
        {
        }

        EventHandler::~EventHandler()
        {
            while(!maVector.empty())
            {
                delete GetEvent();
            }
        }

        // Trigger and consume the events
        void EventHandler::ExecuteEvents()
        {
            for(;;)
            {
                BaseEvent* pEvent = GetEvent();
                if(pEvent == NULL)
                    break;
                pEvent->ExecuteEvent();
                delete pEvent;
            }
        }

        // for control
        sal_Bool EventHandler::IsEmpty() const
        {
            return (0L == maVector.size());
        }
    } // end of namespace mixer
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
    namespace event
    {
        TimerEventHandler::TimerEventHandler(sal_uInt32 nTimeout)
        {
            SetTimeout(nTimeout);
            Stop();
        }

        TimerEventHandler::~TimerEventHandler()
        {
            Stop();
        }

        // The timer when it is triggered; from class Timer
        void TimerEventHandler::Timeout()
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
