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

#ifndef INCLUDED_SLIDESHOW_DISCRETEACTIVITYBASE_HXX
#define INCLUDED_SLIDESHOW_DISCRETEACTIVITYBASE_HXX

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
            DiscreteActivityBase( const ActivityParameters& rParms );

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
            virtual void dispose();
            virtual bool perform();

        protected:
            virtual void startAnimation();

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

#endif /* INCLUDED_SLIDESHOW_DISCRETEACTIVITYBASE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
