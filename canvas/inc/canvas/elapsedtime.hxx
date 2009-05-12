/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: elapsedtime.hxx,v $
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

#ifndef INCLUDED_CANVAS_ELAPSEDTIME_HXX
#define INCLUDED_CANVAS_ELAPSEDTIME_HXX

#include <sal/types.h>

#include "boost/shared_ptr.hpp"

namespace canvas
{
    namespace tools
    {
        /** Calculate elapsed time.

            This class provides several time-measurement and
            -management functions. In its simplest use-case, it
            measures the time from its creation.
         */
        class ElapsedTime
        {
        public:
            /** Create a new ElapsedTime object

                The moment of construction starts the time
                measurement. That means, a subsequent getElapsedTime()
                call will return the time difference between object
                creation and getElapsedTime() call.
             */
            ElapsedTime();

            /** Creates a new ElapsedTime object based on another
                timer.

                The moment of construction starts the time
                measurement. That means, a subsequent getElapsedTime()
                call will return the time difference between object
                creation and getElapsedTime() call. All time values
                are not taken from the system's time base, but from
                the provided timer.
             */
            ElapsedTime( ::boost::shared_ptr<ElapsedTime> const & pTimeBase );

            /** Gets this timer's base timer.
             */
            ::boost::shared_ptr<ElapsedTime> const & getTimeBase() const;

            /** Reset the time

                The instance of the reset() call starts the time
                measurement from scratch. That means, a subsequent
                getElapsedTime() call will return the time difference
                between reset() and getElapsedTime() call.
             */
            void reset();

            /** Query the elapsed time

                This method returns the elapsed time in seconds
                between either the construction of this object, or the
                last reset() call, if any (but see the time modulation
                methods below, for means to modify the otherwise
                continuous flow of time).

                @return the elapsed time in seconds.
             */
            double getElapsedTime() const;

            /** Pauses the running timer.

                This method stops the time, as returned by this
                object, until continueTimer() is called. During this
                period, getElapsedTime() will always return the same
                time value (i.e. the instant when pauseTimer() was
                called).
             */
            void pauseTimer();

            /** Continues the paused timer.

                This method re-enables the time flow, that is, time
                starts running again for clients calling
                getElapsedTime(). The (subtle) difference to the
                holdTimer/releaseTimer() methods below is, that there
                is no perceived time 'jump' between the pauseTimer()
                call and the continueTimer() call, i.e. the time
                starts over with the same value it has stopped on
                pauseTimer().
             */
            void continueTimer();

            /** Adjusts the timer, hold and pause times.

                This method modifies the time as returned by this
                object by the specified amount. This affects the time
                as returned by getElapsedTime(), regardless of the
                mode (e.g. paused, or on hold).

                @param fOffset
                This value will be added to the current time, i.e. the
                next call to getElapsedTime() (when performed
                immediately) will be adjusted by fOffset.

                @param bLimitToLastQueriedTime
                Limits the given offset to the time that has been
                taken via getElapsedTime()
            */
            void adjustTimer( double fOffset,
                              bool bLimitToLastQueriedTime = true );

            /** Holds the current time.

                This call makes the timer hold the current time
                (e.g. getElapsedTime() will return the time when
                holdTimer() was called), while the underlying time is
                running on. When releaseTimer() is called, the time
                will 'jump' to the then-current, underlying time. This
                is equivalent to pressing the "interim time" button on
                a stop watch, which shows this stopped time, while the
                clock keeps running internally.
            */
            void holdTimer();

            /** Releases a held timer.

                After this call, the timer again returns the running
                time on getElapsedTime().
             */
            void releaseTimer();

        private:
            static double getSystemTime();
            double getCurrentTime() const;
            double getElapsedTimeImpl() const; // does not set m_fLastQueriedTime

            const ::boost::shared_ptr<ElapsedTime>  m_pTimeBase;

            /// To validate adjustTimer() calls with bLimitToLastQueriedTime=true
            mutable double                          m_fLastQueriedTime;

            /// Start time, from which the difference to the time base is returned
            double                                  m_fStartTime;

            /// Instant, when last pause or hold started, relative to m_fStartTime
            double                                  m_fFrozenTime;

            /// True, when in pause mode
            bool                                    m_bInPauseMode;

            /// True, when in hold mode
            bool                                    m_bInHoldMode;
        };

    }
}

#endif /* INCLUDED_CANVAS_ELAPSEDTIME_HXX */
