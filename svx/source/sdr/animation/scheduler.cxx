/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scheduler.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:33:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _SDR_ANIMATION_SCHEDULER_HXX
#include <svx/sdr/animation/scheduler.hxx>
#endif

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
