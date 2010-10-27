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

#ifndef INCLUDED_SLIDESHOW_ANIMATIONACTIVITY_HXX
#define INCLUDED_SLIDESHOW_ANIMATIONACTIVITY_HXX

#include "activity.hxx"
#include "animatableshape.hxx"
#include "shapeattributelayer.hxx"


/* Definition of AnimationActivity interface */

namespace slideshow
{
    namespace internal
    {

        /** Extends the Activity interface with animation-specific functions
         */
        class AnimationActivity : public Activity
        {
        public:
            /** Sets targets (shape and attributeLayer)

                Since attribute layers can only be generated when the
                animation starts, the Activity owner must be able to
                pass it into the Activity after initial creation. The
                same applies to the actual shape the animation must
                run for, since e.g. subsetted shapes are generated
                close before the animation starts, too (this is not
                necessary in and out of itself, but for performance
                reasons. Otherwise, character iterations produce tons
                of subset shapes).

                @param rShape
                Shape to play the animation on.

                @param rAttrLayer
                Attribute layer to change the animated values on.
             */
            virtual void setTargets( const AnimatableShapeSharedPtr&        rShape,
                                     const ShapeAttributeLayerSharedPtr&    rAttrLayer ) = 0;
        };

        typedef ::boost::shared_ptr< AnimationActivity > AnimationActivitySharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_ANIMATIONACTIVITY_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
