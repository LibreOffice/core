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

#ifndef INCLUDED_SLIDESHOW_CONTINUOUSKEYTIMEACTIVITYBASE_HXX
#define INCLUDED_SLIDESHOW_CONTINUOUSKEYTIMEACTIVITYBASE_HXX

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
            ContinuousKeyTimeActivityBase( const ActivityParameters& rParms );

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
                                        sal_uInt32  nRepeatCount ) const;

        private:
            const ::basegfx::tools::KeyStopLerp maLerper;
        };
    }
}

#endif /* INCLUDED_SLIDESHOW_CONTINUOUSKEYTIMEACTIVITYBASE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
