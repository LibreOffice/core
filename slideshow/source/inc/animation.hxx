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

#ifndef INCLUDED_SLIDESHOW_ANIMATION_HXX
#define INCLUDED_SLIDESHOW_ANIMATION_HXX

#include <animatableshape.hxx>
#include <shapeattributelayer.hxx>


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
        class Animation
        {
        public:
            virtual ~Animation() {}

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

        typedef ::boost::shared_ptr< Animation > AnimationSharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_ANIMATION_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
