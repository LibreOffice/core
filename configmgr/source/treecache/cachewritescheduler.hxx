/*************************************************************************
 *
 *  $RCSfile: cachewritescheduler.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jb $ $Date: 2002-03-15 11:48:53 $
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

#ifndef CONFIGMGR_CACHEWRITESCHEDULER_HXX
#define CONFIGMGR_CACHEWRITESCHEDULER_HXX

#ifndef CONFIGMGR_MISC_OPTIONS_HXX_
#include "options.hxx"
#endif

#ifndef CONFIGMGR_TIMESTAMP_HXX
#include "timestamp.hxx"
#endif

#ifndef INCLUDED_SET
#include <set>
#define INCLUDED_SET
#endif

#ifndef _VOS_TIMER_HXX_
#include <vos/timer.hxx>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

// -----------------------------------------------------------------------------
namespace configmgr
{
    class RequestOptions;
    namespace backend { class ComponentRequest; }

    // Write down the Cache, much less complex than caching Nodes
    // (better control)
    class OCacheWriteScheduler
    {
        typedef std::set< vos::ORef< OOptions >, ltOptions > CacheWriteList; // fire and forget!

        class Timer : public vos::OTimer
        {
            osl::Mutex  m_aMutex;
        public:
            OCacheWriteScheduler* pParent;

            Timer(OCacheWriteScheduler& _rParent) : pParent(&_rParent) {};

            // vos::OTimer
            virtual void SAL_CALL onShot();

            //
            osl::Mutex&     getShotMutex() {return m_aMutex;}

            // stop the scheduling
            void dispose() {
                stop();
                pParent = NULL;
            }

        };
        friend void Timer::onShot();
    private:
        mutable osl::Mutex  m_aMutex;
        vos::ORef<Timer>    m_xTimer;
        TreeManager&        m_rTreeManager;

        CacheWriteList      m_aWriteList;
        // TimeInterval m_aCleanupDelay;
        TimeInterval m_aCleanupInterval;

    public:
    //-------- Construction and destruction -----------------------------------
        explicit
        OCacheWriteScheduler(TreeManager& _rTreeManager, TimeInterval const& _aCleanupDelay)
            : m_rTreeManager(_rTreeManager)
            , m_aCleanupInterval(_aCleanupDelay)
        {
            m_xTimer = new Timer(*this);
        }
        ~OCacheWriteScheduler();

    //-------- Delay and Interval ---------------------------------------------
        /// retrieves the recurrance interval used for cleanup
        TimeInterval const& getCleanupInterval() const
        {
            osl::MutexGuard aGuard(m_aMutex);
            return m_aCleanupInterval;
        }
        /// calculate the time when to cleanup an pbject that became eligible at <var>aBaseTime</var>.
        TimeStamp getCleanupTime(TimeStamp const& aBaseTime = TimeStamp::getCurrentTime())
        {
            return implGetCleanupTime(aBaseTime, getCleanupInterval());
        }
        static TimeStamp implGetCleanupTime(TimeStamp const& aBaseTime, TimeInterval const& aDelay)
        {
            return aBaseTime + aDelay;
        }
    //-------- Control of execution  ------------------------------------------
        void scheduleWrite(vos::ORef< OOptions > const& _xOptions, bool _bASync = false) CFG_UNO_THROW_ALL(  );
        void scheduleWrite(backend::ComponentRequest _aComponent) CFG_UNO_THROW_ALL(  );

        /// stop pending activities for one set of options (do not discard them)
        bool clearTasks(vos::ORef< OOptions > const& _xOptions);
        bool clearTasks(RequestOptions const& _xOptions);

        /// stop and discard pending activities
        void stopAndWriteCache();

        /// mutex for synchronisation
        osl::Mutex&     getShotMutex() {return m_xTimer->getShotMutex();}

    private:
        // vos::OTimer
        void onTimerShot();

        void runWriter();
        void implStartBefore(TimeStamp const& _aTime);
        void writeOneTreeFoundByOption(vos::ORef< OOptions > const& _xOption) CFG_UNO_THROW_ALL(  );
    };
} // namespace configmgr

#endif // CONFIGMGR_DISPOSETIMER_HXX

