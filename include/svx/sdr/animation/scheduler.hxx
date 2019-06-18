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
#include <vector>


namespace sdr
{
    namespace animation
    {

        class SVX_DLLPUBLIC Event
        {
            // time of event in ms
            sal_uInt32                                      mnTime;

        public:
            // constructor/destructor
            SAL_DLLPRIVATE explicit Event();
            virtual ~Event();

            // get/set time
            SAL_DLLPRIVATE sal_uInt32 GetTime() const {  return mnTime; }
            void SetTime(sal_uInt32 nNew);

            // execute event
            virtual void Trigger(sal_uInt32 nTime) = 0;
        };

        class SVX_DLLPUBLIC Scheduler : public Timer
        {
            // time in ms
            sal_uInt32                                      mnTime;

            // next delta time
            sal_uInt32                                      mnDeltaTime;

            // list of events, sorted by time
            std::vector<Event*>                             mvEvents;

            // Flag which remembers if this timer is paused. Default
            // is false.
            bool                                            mbIsPaused;

        public:
            // constructor/destructor
            SAL_DLLPRIVATE Scheduler();
            virtual ~Scheduler() override;

            // From baseclass Timer, the timeout call
            virtual void Invoke() override;

            // get time
            SAL_DLLPRIVATE sal_uInt32 GetTime() const {  return mnTime; }

            // #i38135#
            SAL_DLLPRIVATE void SetTime(sal_uInt32 nTime);

            // execute all ripe events, removes executed ones from the scheduler
            SAL_DLLPRIVATE void triggerEvents();

            // re-start or stop timer according to event list
            SAL_DLLPRIVATE void checkTimeout();

            // insert/remove events, wrapper to EventList methods
            void InsertEvent(Event& rNew);
            SAL_DLLPRIVATE void RemoveEvent(Event* pOld);

            // get/set pause
            SAL_DLLPRIVATE bool IsPaused() const { return mbIsPaused; }
            SAL_DLLPRIVATE void SetPaused(bool bNew);
        };

    } // end of namespace animation
} // end of namespace sdr


#endif // INCLUDED_SVX_SDR_ANIMATION_SCHEDULER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
