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

#ifndef INCLUDED_CANVAS_ELAPSEDTIME_HXX
#define INCLUDED_CANVAS_ELAPSEDTIME_HXX

#include <canvas/canvastoolsdllapi.h>
#include <memory>

namespace canvas
{
    namespace tools
    {
        /** Calculate elapsed time.

            This class provides several time-measurement and
            -management functions. In its simplest use-case, it
            measures the time from its creation.
         */
        class CANVASTOOLS_DLLPUBLIC ElapsedTime
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
            ElapsedTime( std::shared_ptr<ElapsedTime> const & pTimeBase );

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
            */
            void adjustTimer( double fOffset );

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

            const std::shared_ptr<ElapsedTime>  m_pTimeBase;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
