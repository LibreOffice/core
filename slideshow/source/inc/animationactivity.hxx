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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_ANIMATIONACTIVITY_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_ANIMATIONACTIVITY_HXX

#include "activity.hxx"
#include "animatableshape.hxx"
#include "shapeattributelayer.hxx"


/* Definition of AnimationActivity interface */

namespace slideshow::internal
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

        typedef ::std::shared_ptr< AnimationActivity > AnimationActivitySharedPtr;

}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_ANIMATIONACTIVITY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
