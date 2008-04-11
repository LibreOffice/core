/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: scheduler.cxx,v $
 * $Revision: 1.8 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/sdr/animation/scheduler.hxx>

#include <vector>

//////////////////////////////////////////////////////////////////////////////
// event class

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
    } // end of namespace animation
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eventlist class

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
    } // end of namespace animation
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
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

        void Scheduler::Timeout()
        {
            mnTime += mnDeltaTime;
            Stop();
            Event* pNextEvent = maList.GetFirst();

            // copy events which need to be executed to a vector
            ::std::vector< Event* > EventPointerVector;

            while(pNextEvent && pNextEvent->GetTime() <= mnTime)
            {
                maList.Remove(pNextEvent);
                EventPointerVector.push_back(pNextEvent);
                pNextEvent = maList.GetFirst();
            }

            // execute events from the vector
            ::std::vector< Event* >::iterator aCandidate = EventPointerVector.begin();

            for(;aCandidate != EventPointerVector.end(); aCandidate++)
            {
                // trigger event. This may re-insert the event to the queue (maList)
                (*aCandidate)->Trigger(mnTime);
            }

            // re-start timer for next event to be scheduled (if any)
            if(!IsPaused() && maList.GetFirst())
            {
                mnDeltaTime = maList.GetFirst()->GetTime() - mnTime;

                if(0L != mnDeltaTime)
                {
                    SetTimeout(mnDeltaTime);
                    Start();
                }
            }
        }

        sal_uInt32 Scheduler::GetTime()
        {
            return mnTime;
        }

        // #i38135#
        void Scheduler::SetTime(sal_uInt32 nTime)
        {
            Stop();
            mnTime = nTime;

            Event* pEvent = maList.GetFirst();
            while(pEvent)
            {
                pEvent->SetTime(nTime);
                pEvent = pEvent->GetNext();
            }

            Execute();
        }

        void Scheduler::Reset(sal_uInt32 nTime)
        {
            mnTime = nTime;
            mnDeltaTime = 0L;
            maList.Clear();
        }

        void Scheduler::Execute()
        {
            if(!IsPaused() && !IsActive() && maList.GetFirst())
            {
                mnDeltaTime = 0L;
                Timeout();
            }
        }

        void Scheduler::InsertEvent(Event* pNew)
        {
            if(pNew)
            {
                maList.Insert(pNew);
            }
        }

        void Scheduler::RemoveEvent(Event* pOld)
        {
            if(pOld && maList.GetFirst())
            {
                maList.Remove(pOld);
            }
        }

        void Scheduler::SetPaused(bool bNew)
        {
            if(bNew != mbIsPaused)
            {
                mbIsPaused = bNew;

                if(mbIsPaused)
                {
                    Timeout();
                }
                else
                {
                    Execute();
                }
            }
        }
    } // end of namespace animation
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
