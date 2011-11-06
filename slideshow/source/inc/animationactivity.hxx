/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
