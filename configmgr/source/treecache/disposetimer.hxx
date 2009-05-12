/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: disposetimer.hxx,v $
 * $Revision: 1.15 $
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

#ifndef CONFIGMGR_DISPOSETIMER_HXX
#define CONFIGMGR_DISPOSETIMER_HXX

#include "timestamp.hxx"
#include "requestoptions.hxx"
#include <osl/mutex.hxx>
#include <vos/timer.hxx>
#include <vos/ref.hxx>
#include <com/sun/star/lang/WrappedTargetException.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

#ifndef INCLUDED_UTILITY
#include <utility>
#define INCLUDED_UTILITY
#endif
#ifndef INCLUDED_MAP
#include <map>
#define INCLUDED_MAP
#endif

namespace uno   = ::com::sun::star::uno;
namespace lang  = ::com::sun::star::lang;

namespace configmgr
{
    namespace backend { class CacheController; }
////////////////////////////////////////////////////////////////////////////////
/* OTreeDisposeScheduler:
   does something special????
*/

    class OTreeDisposeScheduler
    {
        typedef std::multimap< TimeStamp, RequestOptions, ltTimeStamp > Agenda;

        class Timer : public vos::OTimer
        {
        public:
            OTreeDisposeScheduler* pParent;

            Timer(OTreeDisposeScheduler& _rParent) : pParent(&_rParent) {};

            // vos::OTimer
            virtual void SAL_CALL onShot();

            // stop the scheduling
            void dispose() {stop(); pParent = NULL;}
        };
        friend void Timer::onShot();

    private:
        Agenda              m_aAgenda;
        vos::ORef<Timer>    m_xTimer;
        backend::CacheController&       m_rTreeManager;

        TimeInterval m_aCleanupDelay;
        TimeInterval m_aCleanupInterval;
    public:
    //-------- Construction and destruction -----------------------------------
        explicit
        OTreeDisposeScheduler(backend::CacheController& _rTreeManager, TimeInterval const& _aCleanupDelay)
            : m_rTreeManager(_rTreeManager)
            , m_aCleanupDelay(_aCleanupDelay)
            , m_aCleanupInterval(_aCleanupDelay)
        {
            m_xTimer = new Timer(*this);
        }

        explicit
        OTreeDisposeScheduler(backend::CacheController& _rTreeManager, TimeInterval const& _aCleanupDelay, TimeInterval const& _aCleanupInterval)
            : m_rTreeManager(_rTreeManager)
            , m_aCleanupDelay(_aCleanupDelay)
            , m_aCleanupInterval(_aCleanupInterval)
        {
            m_xTimer = new Timer(*this);
        }

        ~OTreeDisposeScheduler() { stopAndClearTasks(); }

        //-------- Delay and Interval ---------------------------------------------
        /// sets the initial delay to be used for cleanup in the future, does not affect an already started process
        void setCleanupDelay(TimeInterval const& _aCleanupDelay)
        {
            m_aCleanupDelay = _aCleanupDelay;
        }

        /// sets the initial delay and recurrance interval to be used for cleanup in the future, does not affect an already started process
        void setCleanupDelay(TimeInterval const& _aCleanupDelay, TimeInterval const& _aCleanupInterval)
        {
            m_aCleanupDelay = _aCleanupDelay;
            m_aCleanupInterval = _aCleanupInterval;
        }

        /// sets the recurrance interval to be used for cleanup in the future, does not affect an already started process
        void setCleanupInterval(TimeInterval const& _aCleanupInterval)
        {
            m_aCleanupInterval = _aCleanupInterval;
        }

        /// retrieves the initial delay used for cleanup
        TimeInterval const& getCleanupDelay() const
        {
            return m_aCleanupDelay;
        }

        /// retrieves the recurrance interval used for cleanup
        TimeInterval const& getCleanupInterval() const
        {
            return m_aCleanupInterval;
        }

    //-------- Control of execution  ------------------------------------------
        /// ensure this will execute cleanup duties for _xOptions (no later than after getCleanupDelay() has elapsed)
        void scheduleCleanup(RequestOptions const & _aOptions);

        /// stop and discard pending activities
        void stopAndClearTasks();

    private:
        // vos::OTimer
        void onTimerShot();

        std::pair<bool,RequestOptions> getTask(TimeStamp const& _aActualTime, TimeStamp& _aNextTime);

        /// ensure this will execute cleanup duties for _xOptions (no later than after getCleanupDelay() has elapsed)
      //  TimeStamp fillDisposeList(CacheLoadingAccess & _aCache, DisposeList& _rList, TimeStamp const& aLimitTime)

        TimeStamp runDisposer(TimeStamp const& _aActualTime);
    private:
        TimeStamp implAddTask(RequestOptions const& _xOptions, TimeStamp const& _aTime);
        void implStartBefore(TimeStamp const& _aTime);

        static TimeStamp implGetCleanupTime(TimeStamp const& aPostingTime, TimeInterval const& aDelay)
        { return aPostingTime + aDelay; }
    };


////////////////////////////////////////////////////////////////////////////////
} // namespace configmgr

#endif // CONFIGMGR_DISPOSETIMER_HXX

