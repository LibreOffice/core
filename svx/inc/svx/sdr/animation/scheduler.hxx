/*************************************************************************
 *
 *  $RCSfile: scheduler.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-12-13 08:53:39 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

            // flag-array
            // Flag which remembers if this timer is paused. Default
            // is false.
            unsigned                                        mbIsPaused : 1;

        public:
            // constructor/destructor
            Scheduler();
            virtual ~Scheduler();

            // From baseclass Timer, the timeout call
            virtual void Timeout();

            // get time
            sal_uInt32 GetTime();

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
