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

#include <svx/sdr/animation/scheduler.hxx>

#include <vector>


// event class

namespace sdr
{
    namespace animation
    {
        Event::Event() : mnTime(0)
        {
        }

        Event::~Event()
        {
        }


        void Event::SetTime(sal_uInt32 nNew)
        {
            if(mnTime != nNew)
            {
                mnTime = nNew;
            }
        }

        bool CompareEvent::operator()(Event* const& lhs, Event* const& rhs) const
        {
            return lhs->GetTime() < rhs->GetTime();
        }


        Scheduler::Scheduler()
        :   mnTime(0),
            mnDeltaTime(0),
            mbIsPaused(false)
        {
        }

        Scheduler::~Scheduler()
        {
            Stop();
        }

        void Scheduler::Invoke()
        {
            // stop timer and add time
            Stop();
            mnTime += mnDeltaTime;

            // execute events
            triggerEvents();

            // re-start or stop timer according to event list
            checkTimeout();
        }

        void Scheduler::triggerEvents()
        {
            if (maList.empty())
                return;

            // copy events which need to be executed to a vector. Remove them from
            // the scheduler
            ::std::vector< Event* > aToBeExecutedList;

            while(!maList.empty() && maList[0]->GetTime() <= mnTime)
            {
                Event* pNextEvent = maList.front();
                maList.erase(maList.begin());
                aToBeExecutedList.push_back(pNextEvent);
            }

            // execute events from the vector
            ::std::vector< Event* >::const_iterator aEnd = aToBeExecutedList.end();
            for(::std::vector< Event* >::iterator aCandidate = aToBeExecutedList.begin();
                aCandidate != aEnd; ++aCandidate)
            {
                // trigger event. This may re-insert the event to the scheduler again
                (*aCandidate)->Trigger(mnTime);
            }
        }

        void Scheduler::checkTimeout()
        {
            // re-start or stop timer according to event list
            if(!IsPaused() && !maList.empty())
            {
                mnDeltaTime = maList.front()->GetTime() - mnTime;

                if(0L != mnDeltaTime)
                {
                    SetTimeout(mnDeltaTime);
                    Start();
                }
            }
            else
            {
                Stop();
            }
        }


        // #i38135#
        void Scheduler::SetTime(sal_uInt32 nTime)
        {
            // reset time
            Stop();
            mnTime = nTime;

            if (maList.empty())
                return;

            // reset event time points
            for (auto & rEvent : maList)
            {
                rEvent->SetTime(nTime);
            }

            if(!IsPaused())
            {
                // without delta time, init events by triggering them. This will invalidate
                // painted objects and add them to the scheduler again
                mnDeltaTime = 0;
                triggerEvents();
                checkTimeout();
             }
        }

        void Scheduler::InsertEvent(Event* pNew)
        {
            maList.insert(pNew);
            checkTimeout();
        }

        void Scheduler::RemoveEvent(Event* pOld)
        {
            if(!maList.empty())
            {
                auto it = maList.find(pOld);
                if (it != maList.end())
                    maList.erase(it);
                checkTimeout();
            }
        }

        void Scheduler::SetPaused(bool bNew)
        {
            if(bNew != mbIsPaused)
            {
                mbIsPaused = bNew;
                checkTimeout();
            }
        }
    } // end of namespace animation
} // end of namespace sdr

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
