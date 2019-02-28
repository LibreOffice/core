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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_ANIMATION_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_ANIMATION_HXX

#include "animatableshape.hxx"
#include "shapeattributelayer.hxx"
#include "disposable.hxx"


/* Definition of Animation interface */

namespace slideshow
{
    namespace internal
    {
        /** Interface defining a generic animation.

            This interface is used by objects implementing the
            Activity interface to drive the animation effect. Objects
            implementing this interface will receive time-varying
            animation values, which they can forward to the
            appropriate attributes. The type of these animation values
            is given in derived interfaces.

            @see NumberAnimation
            @see ColorAnimation
            @see PairAnimation
         */
        class Animation : public virtual SharedPtrAble
        {
        public:
            /** Notify that the animation going active soon.

                Implementers should preload any buffers, and create
                any expensive objects at this time.

                @param rShape
                Shape to apply this animation to.

                @param rAttrLayer
                Attribute layer to play the animation on.
             */
            virtual void prefetch( const AnimatableShapeSharedPtr&     rShape,
                                   const ShapeAttributeLayerSharedPtr& rAttrLayer ) = 0;

            /** Notify that the animation is about to begin.

                Implementers are free to start accompanying effects,
                such as sounds, and the animation timer now.

                @param rShape
                Shape to apply this animation to.

                @param rAttrLayer
                Attribute layer to play the animation on.
             */
            virtual void start( const AnimatableShapeSharedPtr&     rShape,
                                const ShapeAttributeLayerSharedPtr& rAttrLayer ) = 0;

            /** Notify that the animation is about to end.
             */
            virtual void end() = 0;
        };

    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_ANIMATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
