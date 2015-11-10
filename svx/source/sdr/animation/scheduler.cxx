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
        Event::Event(sal_uInt32 nTime)
        :   mnTime(nTime),
            mpNext(nullptr)
        {
        }

        Event::~Event()
        {
        }


        void Event::SetNext(Event* pNew)
        {
            if(pNew != mpNext)
            {
                mpNext = pNew;
            }
        }


        void Event::SetTime(sal_uInt32 nNew)
        {
            if(mnTime != nNew)
            {
                mnTime = nNew;
            }
        }
    } // end of namespace animation
} // end of namespace sdr


// eventlist class

namespace sdr
{
    namespace animation
    {
        EventList::EventList()
        :   mpHead(nullptr)
        {
        }

        EventList::~EventList()
        {
            Clear();
        }

        void EventList::Insert(Event* pNew)
        {
            if(pNew)
            {
                Event* pCurrent = mpHead;
                Event* pPrev = nullptr;

                while(pCurrent && pCurrent->GetTime() < pNew->GetTime())
                {
                    pPrev = pCurrent;
                    pCurrent = pCurrent->GetNext();
                }

                if(pPrev)
                {
                    pNew->SetNext(pPrev->GetNext());
                    pPrev->SetNext(pNew);
                }
                else
                {
                    pNew->SetNext(mpHead);
                    mpHead = pNew;
                }
            }
        }

        void EventList::Remove(Event* pOld)
        {
            if(pOld && mpHead)
            {
                Event* pCurrent = mpHead;
                Event* pPrev = nullptr;

                while(pCurrent && pCurrent != pOld)
                {
                    pPrev = pCurrent;
                    pCurrent = pCurrent->GetNext();
                }

                if(pPrev)
                {
                    pPrev->SetNext(pOld->GetNext());
                }
                else
                {
                    mpHead = pOld->GetNext();
                }

                pOld->SetNext(nullptr);
            }
        }

        void EventList::Clear()
        {
            while(mpHead)
            {
                Event* pNext = mpHead->GetNext();
                mpHead->SetNext(nullptr);
                mpHead = pNext;
            }
        }

    } // end of namespace animation
} // end of namespace sdr


// scheduler class

namespace sdr
{
    namespace animation
    {
        Scheduler::Scheduler()
        :   mnTime(0L),
            mnDeltaTime(0L),
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
            Event* pNextEvent = maList.GetFirst();

            if(pNextEvent)
            {
                // copy events which need to be executed to a vector. Remove them from
                // the scheduler
                ::std::vector< Event* > EventPointerVector;

                while(pNextEvent && pNextEvent->GetTime() <= mnTime)
                {
                    maList.Remove(pNextEvent);
                    EventPointerVector.push_back(pNextEvent);
                    pNextEvent = maList.GetFirst();
                }

                // execute events from the vector
                for(::std::vector< Event* >::iterator aCandidate = EventPointerVector.begin();
                    aCandidate != EventPointerVector.end(); ++aCandidate)
                {
                    // trigger event. This may re-insert the event to the scheduler again
                    (*aCandidate)->Trigger(mnTime);
                }
            }
        }

        void Scheduler::checkTimeout()
        {
            // re-start or stop timer according to event list
            if(!IsPaused() && maList.GetFirst())
            {
                mnDeltaTime = maList.GetFirst()->GetTime() - mnTime;

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

            // get event pointer
            Event* pEvent = maList.GetFirst();

            if(pEvent)
            {
                // retet event time points
                while(pEvent)
                {
                    pEvent->SetTime(nTime);
                    pEvent = pEvent->GetNext();
                }

                if(!IsPaused())
                {
                    // without delta time, init events by triggering them. This will invalidate
                    // painted objects and add them to the scheduler again
                    mnDeltaTime = 0L;
                    triggerEvents();
                    checkTimeout();
                }
            }
        }

        void Scheduler::InsertEvent(Event* pNew)
        {
            if(pNew)
            {
                maList.Insert(pNew);
                checkTimeout();
            }
        }

        void Scheduler::RemoveEvent(Event* pOld)
        {
            if(pOld && maList.GetFirst())
            {
                maList.Remove(pOld);
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
