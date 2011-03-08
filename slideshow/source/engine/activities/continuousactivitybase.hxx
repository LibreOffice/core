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

#ifndef INCLUDED_SLIDESHOW_CONTINUOUSACTIVITYBASE_HXX
#define INCLUDED_SLIDESHOW_CONTINUOUSACTIVITYBASE_HXX

#include "simplecontinuousactivitybase.hxx"

namespace slideshow
{
    namespace internal
    {
        /** Simple, continuous animation.

            This class implements a simple, continuous
            animation. Only addition to ActivityBase class is an
            explicit animation duration and a minimal number of
            frames to display.
        */
        class ContinuousActivityBase : public SimpleContinuousActivityBase
        {
        public:
            ContinuousActivityBase( const ActivityParameters& rParms );

            using SimpleContinuousActivityBase::perform;

            /** Hook for derived classes

                This method will be called from perform(), already
                equipped with the modified time (nMinNumberOfFrames, repeat,
                acceleration and deceleration taken into account).

                @param nModifiedTime
                Already accelerated/decelerated and repeated time, always
                in the [0,1] range.

                @param nRepeatCount
                Number of full repeats already performed
            */
            virtual void perform( double nModifiedTime, sal_uInt32 nRepeatCount ) const = 0;

            /// From SimpleContinuousActivityBase class
            virtual void simplePerform( double      nSimpleTime,
                                        sal_uInt32  nRepeatCount ) const;
        };
    }
}

#endif /* INCLUDED_SLIDESHOW_CONTINUOUSACTIVITYBASE_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
