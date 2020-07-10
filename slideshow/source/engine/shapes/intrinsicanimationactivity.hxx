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

#ifndef INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_INTRINSICANIMATIONACTIVITY_HXX
#define INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_INTRINSICANIMATIONACTIVITY_HXX

#include <wakeupevent.hxx>
#include <activity.hxx>
#include <slideshowcontext.hxx>
#include "drawshape.hxx"

/* Definition of IntrinsicAnimationActivity class */

namespace slideshow::internal
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
            come from the outside, since wakeup event and this
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
            sal_uInt32                      nNumLoops);

}

#endif // INCLUDED_SLIDESHOW_SOURCE_ENGINE_SHAPES_INTRINSICANIMATIONACTIVITY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
