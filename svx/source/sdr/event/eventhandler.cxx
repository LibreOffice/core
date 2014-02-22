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

#include <svx/sdr/event/eventhandler.hxx>



#include <algorithm>
#include <tools/debug.hxx>



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
    } 
} 



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

        
        bool EventHandler::IsEmpty() const
        {
            return (0L == maVector.size());
        }
    } 
} 



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

        
        void TimerEventHandler::Timeout()
        {
            ExecuteEvents();
        }

        
        void TimerEventHandler::Restart()
        {
            Start();
        }
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
