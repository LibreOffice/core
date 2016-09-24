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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_ACTIVITIES_CONTINUOUSKEYTIMEACTIVITYBASE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_ACTIVITIES_CONTINUOUSKEYTIMEACTIVITYBASE_HXX

#include "simplecontinuousactivitybase.hxx"

#include <basegfx/tools/keystoplerp.hxx>
#include <vector>


namespace slideshow
{
    namespace internal
    {
        /** Interpolated, key-times animation.

            This class implements an interpolated key-times
            animation, with continuous time.
        */
        class ContinuousKeyTimeActivityBase : public SimpleContinuousActivityBase
        {
        public:
            explicit ContinuousKeyTimeActivityBase( const ActivityParameters& rParms );

            using SimpleContinuousActivityBase::perform;

            /** Hook for derived classes

                This method will be called from perform(), already
                equipped with the modified time (nMinNumberOfFrames, repeat,
                acceleration and deceleration taken into account).

                @param nIndex
                Current index of the key times/key values.

                @param nFractionalIndex
                Fractional value from the [0,1] range, specifying
                the position between nIndex and nIndex+1.

                @param nRepeatCount
                Number of full repeats already performed
            */
            virtual void perform( sal_uInt32    nIndex,
                                  double        nFractionalIndex,
                                  sal_uInt32    nRepeatCount ) const = 0;

            /// From SimpleContinuousActivityBase class
            virtual void simplePerform( double      nSimpleTime,
                                        sal_uInt32  nRepeatCount ) const override;

        private:
            const ::basegfx::tools::KeyStopLerp maLerper;
        };
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_ACTIVITIES_CONTINUOUSKEYTIMEACTIVITYBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
