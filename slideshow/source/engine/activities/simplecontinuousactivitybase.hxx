/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef INCLUDED_SLIDESHOW_SIMPLECONTINUOUSACTIVITYBASE_HXX
#define INCLUDED_SLIDESHOW_SIMPLECONTINUOUSACTIVITYBASE_HXX

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
            SimpleContinuousActivityBase( const ActivityParameters& rParms );

            virtual double calcTimeLag() const;
            virtual bool perform();

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

            virtual void startAnimation();

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

#endif /* INCLUDED_SLIDESHOW_SIMPLECONTINUOUSACTIVITYBASE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
