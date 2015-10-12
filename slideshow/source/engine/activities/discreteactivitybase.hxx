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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_ACTIVITIES_DISCRETEACTIVITYBASE_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_ACTIVITIES_DISCRETEACTIVITYBASE_HXX

#include "activitybase.hxx"
#include "wakeupevent.hxx"

#include <vector>


namespace slideshow
{
    namespace internal
    {
        /** Specialization of ActivityBase for discrete time activities.

            A discrete time activity is one where time proceeds in
            discrete steps, i.e. at given time instants.
        */
        class DiscreteActivityBase : public ActivityBase
        {
        public:
            explicit DiscreteActivityBase( const ActivityParameters& rParms );

            /** Hook for derived classes.

                This method is called for each discrete time
                instant, with nFrame denoting the frame number
                (starting with 0)

                @param nFrame
                Current frame number.

                @param nRepeatCount
                Number of full repeats already performed
            */
            virtual void perform( sal_uInt32 nFrame, sal_uInt32 nRepeatCount ) const = 0;
            virtual void dispose() override;
            virtual bool perform() override;

        protected:
            virtual void startAnimation() override;

            sal_uInt32 calcFrameIndex( sal_uInt32       nCurrCalls,
                                       ::std::size_t    nVectorSize ) const;

            sal_uInt32 calcRepeatCount( sal_uInt32      nCurrCalls,
                                        ::std::size_t   nVectorSize ) const;

            ::std::size_t getNumberOfKeyTimes() const { return maDiscreteTimes.size(); }

        private:
            WakeupEventSharedPtr            mpWakeupEvent;
            const ::std::vector< double >   maDiscreteTimes;
            const double                    mnSimpleDuration;
            sal_uInt32                      mnCurrPerformCalls;
        };
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_ACTIVITIES_DISCRETEACTIVITYBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
