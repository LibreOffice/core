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

#include <svx/sdr/animation/scheduler.hxx>

#include <vector>




namespace sdr
{
    namespace animation
    {
        Event::Event(sal_uInt32 nTime)
        :   mnTime(nTime),
            mpNext(0L)
        {
        }

        Event::~Event()
        {
        }

        Event* Event::GetNext() const
        {
            return mpNext;
        }

        void Event::SetNext(Event* pNew)
        {
            if(pNew != mpNext)
            {
                mpNext = pNew;
            }
        }

        sal_uInt32 Event::GetTime() const
        {
            return mnTime;
        }

        void Event::SetTime(sal_uInt32 nNew)
        {
            if(mnTime != nNew)
            {
                mnTime = nNew;
            }
        }
    } 
} 




namespace sdr
{
    namespace animation
    {
        EventList::EventList()
        :   mpHead(0L)
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
                Event* pPrev = 0L;

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
                Event* pPrev = 0L;

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

                pOld->SetNext(0L);
            }
        }

        void EventList::Clear()
        {
            while(mpHead)
            {
                Event* pNext = mpHead->GetNext();
                mpHead->SetNext(0L);
                mpHead = pNext;
            }
        }

        Event* EventList::GetFirst()
        {
            return mpHead;
        }
    } 
} 




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

        void Scheduler::Timeout()
        {
            
            Stop();
            mnTime += mnDeltaTime;

            
            triggerEvents();

            
            checkTimeout();
        }

        void Scheduler::triggerEvents()
        {
            Event* pNextEvent = maList.GetFirst();

            if(pNextEvent)
            {
                
                
                ::std::vector< Event* > EventPointerVector;

                while(pNextEvent && pNextEvent->GetTime() <= mnTime)
                {
                    maList.Remove(pNextEvent);
                    EventPointerVector.push_back(pNextEvent);
                    pNextEvent = maList.GetFirst();
                }

                
                for(::std::vector< Event* >::iterator aCandidate = EventPointerVector.begin();
                    aCandidate != EventPointerVector.end(); ++aCandidate)
                {
                    
                    (*aCandidate)->Trigger(mnTime);
                }
            }
        }

        void Scheduler::checkTimeout()
        {
            
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

        sal_uInt32 Scheduler::GetTime()
        {
            return mnTime;
        }

        
        void Scheduler::SetTime(sal_uInt32 nTime)
        {
            
            Stop();
            mnTime = nTime;

            
            Event* pEvent = maList.GetFirst();

            if(pEvent)
            {
                
                while(pEvent)
                {
                    pEvent->SetTime(nTime);
                    pEvent = pEvent->GetNext();
                }

                if(!IsPaused())
                {
                    
                    
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
    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
