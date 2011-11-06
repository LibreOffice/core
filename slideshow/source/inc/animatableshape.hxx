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



#ifndef INCLUDED_SLIDESHOW_ANIMATABLESHAPE_HXX
#define INCLUDED_SLIDESHOW_ANIMATABLESHAPE_HXX

#include <boost/shared_ptr.hpp>

#include "shape.hxx"


namespace slideshow
{
    namespace internal
    {
        /** Represents an animatable shape.

            This interface adds animation handling methods to a
            shape. It allows transparent switching between
            sprite-based viewing and static painting, depending on
            whether animations are currently running.
         */
        class AnimatableShape : public Shape
        {
        public:
            // Animation methods
            //------------------------------------------------------------------

            /** Notify the Shape that an animation starts now

                This method enters animation mode on all registered
                views.

                @attention This method is supposed to be called only
                from the LayerManager, since it might involve shifting
                shapes between different layers (and removing this
                shape from the background layer in the first place)
             */
            virtual void enterAnimationMode() = 0;

            /** Notify the Shape that it is no longer animated

                This methods requests the Shape to end animation mode
                on all registered views, if called more or equal the
                times enterAnimationMode() was called. That is, the
                Shape only leaves animation mode, if all requested
                enterAnimationMode() call sites have issued their
                matching leaveAnimationMode().

                @attention This method is supposed to be called only
                from the LayerManager, since it might involve shifting
                shapes between different layers (and adding this
                shape to the background layer again)
             */
            virtual void leaveAnimationMode() = 0;

        };

        typedef ::boost::shared_ptr< AnimatableShape > AnimatableShapeSharedPtr;

    }
}

#endif /* INCLUDED_SLIDESHOW_ANIMATABLESHAPE_HXX */
