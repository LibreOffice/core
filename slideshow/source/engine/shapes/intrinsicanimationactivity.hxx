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

#ifndef INCLUDED_SLIDESHOW_INTRINSICANIMATIONACTIVITY_HXX
#define INCLUDED_SLIDESHOW_INTRINSICANIMATIONACTIVITY_HXX

#include "wakeupevent.hxx"
#include "activity.hxx"
#include "slideshowcontext.hxx"
#include "drawshape.hxx"
#include "tools.hxx"

/* Definition of IntrinsicAnimationActivity class */

namespace slideshow
{
    namespace internal
    {
        /** Create an IntrinsicAnimationActivity.

            This is an Activity interface implementation for intrinsic
            shape animations. Intrinsic shape animations are
            animations directly within a shape, e.g. drawing layer
            animations, or GIF animations.

            @param rContext
            Common slideshow objects

            @param rDrawShape
            Shape to control the intrinsic animation for

            @param rWakeupEvent
            Externally generated wakeup event, to set this
            activity to sleep during inter-frame intervals. Must
            come frome the outside, since wakeup event and this
            object have mutual references to each other.

            @param rTimeouts
            Vector of timeout values, to wait before the next
            frame is shown.
        */
        ActivitySharedPtr createIntrinsicAnimationActivity(
            const SlideShowContext&         rContext,
            const DrawShapeSharedPtr&       rDrawShape,
            const WakeupEventSharedPtr&     rWakeupEvent,
            const ::std::vector<double>&    rTimeouts,
            ::std::size_t                   nNumLoops,
            CycleMode                       eCycleMode );
    }
}

#endif /* INCLUDED_SLIDESHOW_INTRINSICANIMATIONACTIVITY_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
