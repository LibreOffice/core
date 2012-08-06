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

#ifndef _SDR_ANIMATION_SCHEDULER_HXX
#define _SDR_ANIMATION_SCHEDULER_HXX

#include <sal/types.h>
#include <vcl/timer.hxx>
#include <svx/svxdllapi.h>

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
            explicit Event(sal_uInt32 nTime);
            SVX_DLLPUBLIC virtual ~Event();

            // access to mpNext
            Event* GetNext() const;
            void SetNext(Event* pNew);

            // get/set time
            sal_uInt32 GetTime() const;
            void SVX_DLLPUBLIC SetTime(sal_uInt32 nNew);

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
            SVX_DLLPUBLIC virtual ~EventList();

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
            SVX_DLLPUBLIC virtual ~Scheduler();

            // From baseclass Timer, the timeout call
            SVX_DLLPUBLIC virtual void Timeout();

            // get time
            sal_uInt32 GetTime();

            // #i38135#
            void SetTime(sal_uInt32 nTime);

            // execute all ripe events, removes executed ones from the scheduler
            void triggerEvents();

            // re-start or stop timer according to event list
            void checkTimeout();

            // insert/remove events, wrapper to EventList methods
            void SVX_DLLPUBLIC InsertEvent(Event* pNew);
            void RemoveEvent(Event* pOld);

            // get/set pause
            bool IsPaused() const { return mbIsPaused; }
            void SetPaused(bool bNew);
        };
    } // end of namespace animation
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////

#endif //_SDR_ANIMATION_SCHEDULER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
