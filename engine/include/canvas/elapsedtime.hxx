/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <canvas/canvastoolsdllapi.h>
#include <memory>

namespace canvastools
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
