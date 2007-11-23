/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cachewritescheduler.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 14:37:40 $
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

#ifndef CONFIGMGR_CACHEWRITESCHEDULER_HXX
#define CONFIGMGR_CACHEWRITESCHEDULER_HXX

#ifndef CONFIGMGR_MISC_REQUESTOPTIONS_HXX_
#include "requestoptions.hxx"
#endif
#ifndef CONFIGMGR_TIMESTAMP_HXX
#include "timestamp.hxx"
#endif
#ifndef CONFIGMGR_UTILITY_HXX_
#include "utility.hxx"
#endif

#ifndef INCLUDED_SET
#include <set>
#define INCLUDED_SET
#endif

#ifndef _VOS_TIMER_HXX_
#include <vos/timer.hxx>
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>

#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

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
        void scheduleWrite(backend::ComponentRequest _aComponent) CFG_UNO_THROW_ALL(  );

        /// stop pending activities for one set of options (do not discard them)
        bool clearTasks(RequestOptions const& _xOptions);

        /// stop and discard pending activities
        void stopAndWriteCache();
    private:
        // vos::OTimer
        void onTimerShot();

        void runWriter();
        void implStartBefore(TimeStamp const& _aTime);
        void writeOneTreeFoundByOption(RequestOptions const& _aOption) CFG_UNO_THROW_ALL(  );
    };
} // namespace configmgr

#endif // CONFIGMGR_DISPOSETIMER_HXX

