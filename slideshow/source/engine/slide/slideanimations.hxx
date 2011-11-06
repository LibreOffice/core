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



#ifndef INCLUDED_SLIDESHOW_SLIDEANIMATIONS_HXX
#define INCLUDED_SLIDESHOW_SLIDEANIMATIONS_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <basegfx/vector/b2dvector.hxx>

#include "event.hxx"
#include "slideshowcontext.hxx"
#include "subsettableshapemanager.hxx"
#include "animationnode.hxx"

namespace com { namespace sun { namespace star { namespace animations
{
    class XAnimationNode;
} } } }


/* Definition of SlideAnimations class */

namespace slideshow
{
    namespace internal
    {
        /** This class generates and manages all animations of a slide.

            Provided with the root animation node, this class imports
            the effect information and builds the event tree for all
            of the slide's animations.
         */
        class SlideAnimations
        {
        public:
            /** Create an animation generator.

                @param rContext
                Slide show context, passing on common parameters
             */
            SlideAnimations( const SlideShowContext&     rContext,
                             const ::basegfx::B2DVector& rSlideSize );
            ~SlideAnimations();

            /** Import animations from a SMIL root animation node.

                This method might take some time, depending on the
                complexity of the SMIL animation network to be
                imported.

                @param xRootAnimationNode
                Root animation node for the effects to be
                generated. This is typically obtained from the
                XDrawPage's XAnimationNodeSupplier.

             */
            bool importAnimations( const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::animations::XAnimationNode >&   xRootAnimationNode );

            /** Check, whether imported animations actually contain
                any effects.

                @return true, if there are actual animations in the
                imported node tree.
             */
            bool isAnimated() const;

            /** Start the animations.

                This method creates the network of events and
                activities for all animations. The events and
                activities are inserted into the constructor-provided
                queues. These queues are not explicitely cleared, if
                you rely on this object's effects to run without
                interference, you should clear the queues by yourself.

                @return true, if all events have been successfully
                created.
             */
            bool start();

            /** End all animations.

                This method force-ends all animations. If a slide end
                event has been registered, that is fired, too.
             */
            void end();

            /// Release all references. Does not notify anything.
            void dispose();

        private:
            SlideShowContext         maContext;
            const basegfx::B2DVector maSlideSize;
            AnimationNodeSharedPtr   mpRootNode;
        };
    }
}

#endif /* INCLUDED_SLIDESHOW_SLIDEANIMATIONS_HXX */
