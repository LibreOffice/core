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

#include <algorithm>
#include <vector>


// event class

namespace sdr::animation
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

        Scheduler::Scheduler()
        :   mnTime(0),
            mnDeltaTime(0),
            mbIsPaused(false)
        {
            SetDebugName("sdr::animation::Scheduler");
            SetPriority(TaskPriority::POST_PAINT);
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
            if (mvEvents.empty())
                return;

            // copy events which need to be executed to a vector. Remove them from
            // the scheduler
            ::std::vector< Event* > aToBeExecutedList;

            while(!mvEvents.empty() && mvEvents.front()->GetTime() <= mnTime)
            {
                Event* pNextEvent = mvEvents.front();
                mvEvents.erase(mvEvents.begin());
                aToBeExecutedList.push_back(pNextEvent);
            }

            // execute events from the vector
            for(auto& rpCandidate : aToBeExecutedList)
            {
                // trigger event. This may re-insert the event to the scheduler again
                rpCandidate->Trigger(mnTime);
            }
        }

        void Scheduler::checkTimeout()
        {
            // re-start or stop timer according to event list
            if(!IsPaused() && !mvEvents.empty())
            {
                mnDeltaTime = mvEvents.front()->GetTime() - mnTime;

                if(0 != mnDeltaTime)
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

            if (mvEvents.empty())
                return;

            // reset event time points
            for (auto & rEvent : mvEvents)
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

        void Scheduler::InsertEvent(Event& rNew)
        {
            // insert maintaining time ordering
            auto it = std::find_if(mvEvents.begin(), mvEvents.end(),
                [&rNew](const Event* pEvent) { return rNew.GetTime() < pEvent->GetTime(); });
            mvEvents.insert(it, &rNew);
            checkTimeout();
        }

        void Scheduler::RemoveEvent(Event* pOld)
        {
            if(!mvEvents.empty())
            {
                mvEvents.erase(std::remove(mvEvents.begin(), mvEvents.end(), pOld), mvEvents.end());
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

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
