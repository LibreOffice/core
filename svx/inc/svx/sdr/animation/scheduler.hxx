/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scheduler.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:00:15 $
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

#ifndef _SDR_ANIMATION_SCHEDULER_HXX
#define _SDR_ANIMATION_SCHEDULER_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// event class

namespace sdr
{
    namespace animation
    {
        class Event
        {
            // time of event in ms
            sal_uInt32                                      mnTime;

            // pointer for simply linked list
            Event*                                          mpNext;

        public:
            // constructor/destructor
            Event(sal_uInt32 nTime);
            virtual ~Event();

            // access to mpNext
            Event* GetNext() const;
            void SetNext(Event* pNew);

            // get/set time
            sal_uInt32 GetTime() const;
            void SetTime(sal_uInt32 nNew);

            // execute event
            virtual void Trigger(sal_uInt32 nTime) = 0;
        };
    } // end of namespace animation
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eventlist class

namespace sdr
{
    namespace animation
    {
        class EventList
        {
            // pointer to first entry
            Event*                                          mpHead;

        public:
            // constructor/destructor
            EventList();
            virtual ~EventList();

            // insert/remove time dependent
            void Insert(Event* pNew);
            void Remove(Event* pOld);

            // clear list
            void Clear();

            // get first
            Event* GetFirst();
        };
    } // end of namespace animation
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// scheduler class

namespace sdr
{
    namespace animation
    {
        class Scheduler : public Timer
        {
            // time in ms
            sal_uInt32                                      mnTime;

            // next delta time
            sal_uInt32                                      mnDeltaTime;

            // list of events
            EventList                                       maList;

            // Flag which remembers if this timer is paused. Default
            // is false.
            bool                                            mbIsPaused;

        public:
            // constructor/destructor
            Scheduler();
            virtual ~Scheduler();

            // From baseclass Timer, the timeout call
            virtual void Timeout();

            // get time
            sal_uInt32 GetTime();

            // #i38135#
            void SetTime(sal_uInt32 nTime);

            // reset
            void Reset(sal_uInt32 nTime);

            // execute
            void Execute();

            // insert/remove events, wrapper to EventList methods
            void InsertEvent(Event* pNew);
            void RemoveEvent(Event* pOld);

            // get/set pause
            bool IsPaused() const { return mbIsPaused; }
            void SetPaused(bool bNew);
        };
    } // end of namespace animation
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_ANIMATION_SCHEDULER_HXX

// eof
