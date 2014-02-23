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

#ifndef INCLUDED_SVX_SDR_ANIMATION_SCHEDULER_HXX
#define INCLUDED_SVX_SDR_ANIMATION_SCHEDULER_HXX

#include <sal/types.h>
#include <vcl/timer.hxx>
#include <svx/svxdllapi.h>


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



#endif // INCLUDED_SVX_SDR_ANIMATION_SCHEDULER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
