/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cachewritescheduler.hxx,v $
 * $Revision: 1.7 $
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

#ifndef CONFIGMGR_CACHEWRITESCHEDULER_HXX
#define CONFIGMGR_CACHEWRITESCHEDULER_HXX

#include "datalock.hxx"
#include "requestoptions.hxx"
#include "timestamp.hxx"
#include "utility.hxx"

#ifndef INCLUDED_SET
#include <set>
#define INCLUDED_SET
#endif
#include <vos/timer.hxx>
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>

#endif
#include <osl/mutex.hxx>

// -----------------------------------------------------------------------------
namespace configmgr
{
    class RequestOptions;
    namespace backend { class ComponentRequest; class CacheController; }

    // Write down the Cache, much less complex than caching Nodes
    // (better control)
    class OCacheWriteScheduler
    {
        typedef std::set< RequestOptions, lessRequestOptions > CacheWriteList; // fire and forget!

        class Timer : public vos::OTimer
        {
        public:
            OCacheWriteScheduler* pParent;

            Timer(OCacheWriteScheduler& _rParent) : pParent(&_rParent) {};

            // vos::OTimer
            virtual void SAL_CALL onShot();

            // stop the scheduling
            void dispose() {
                stop();
                pParent = NULL;
            }

        };
        friend void Timer::onShot();
    private:
        vos::ORef<Timer>          m_xTimer;
        backend::CacheController &m_rTreeManager;
        CacheWriteList        m_aWriteList;
        TimeInterval          m_aWriteInterval;

    public:
    //-------- Construction and destruction -----------------------------------
        explicit
        OCacheWriteScheduler(backend::CacheController& _rTreeManager, TimeInterval const& _aWriteInterval)
            : m_rTreeManager(_rTreeManager)
            , m_aWriteInterval(_aWriteInterval)
        {
            m_xTimer = new Timer(*this);
        }
        ~OCacheWriteScheduler();

    //-------- Delay and Interval ---------------------------------------------
        /// retrieves the recurrance interval used for cleanup
        TimeInterval const& getWriteInterval() const
        {
            UnoApiLock aLock;
            return m_aWriteInterval;
        }

        static TimeStamp implGetScheduleTime(TimeStamp const& aBaseTime, TimeInterval const& aDelay)
        {
            return aBaseTime + aDelay;
        }
    //-------- Control of execution  ------------------------------------------
        void scheduleWrite(backend::ComponentRequest _aComponent) SAL_THROW((com::sun::star::uno::Exception));

        /// stop pending activities for one set of options (do not discard them)
        bool clearTasks(RequestOptions const& _xOptions);

        /// stop and discard pending activities
        void stopAndWriteCache();
    private:
        // vos::OTimer
        void onTimerShot();

        void runWriter();
        void implStartBefore(TimeStamp const& _aTime);
        void writeOneTreeFoundByOption(RequestOptions const& _aOption) SAL_THROW((com::sun::star::uno::Exception));
    };
} // namespace configmgr

#endif // CONFIGMGR_DISPOSETIMER_HXX

