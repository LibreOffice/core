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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_ACTIVITIES_SIMPLECONTINUOUSACTIVITYBASE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_ACTIVITIES_SIMPLECONTINUOUSACTIVITYBASE_HXX

#include "activitybase.hxx"
#include <canvas/elapsedtime.hxx>

namespace slideshow
{
    namespace internal
    {
        /** Simple, continuous animation.

            This class implements a simple, continuous animation
            without considering repeats or acceleration on the
            perform call. Only useful as a base class, you
            probably want to use ContinuousActivityBase.
        */
        class SimpleContinuousActivityBase : public ActivityBase
        {
        public:
            explicit SimpleContinuousActivityBase( const ActivityParameters& rParms );

            virtual double calcTimeLag() const override;
            virtual bool perform() override;

        protected:
            /** Hook for derived classes

                This method will be called from perform().

                @param nSimpleTime
                Simple animation time, without repeat,
                acceleration or deceleration applied. This value
                is always in the [0,1] range, the repeat is
                accounted for with the nRepeatCount parameter.

                @param nRepeatCount
                Number of full repeats already performed
            */
            virtual void simplePerform( double nSimpleTime, sal_uInt32 nRepeatCount ) const = 0;

            virtual void startAnimation() override;

        private:
            /// Time elapsed since activity started
            ::canvas::tools::ElapsedTime    maTimer;

            /// Simple duration of activity
            const double                    mnMinSimpleDuration;

            /// Minimal number of frames to show (see ActivityParameters)
            const sal_uInt32                mnMinNumberOfFrames;

            /// Actual number of frames shown until now.
            sal_uInt32                      mnCurrPerformCalls;
        };
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_ACTIVITIES_SIMPLECONTINUOUSACTIVITYBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
