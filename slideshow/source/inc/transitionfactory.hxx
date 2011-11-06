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



#ifndef INCLUDED_SLIDESHOW_TRANSITIONFACTORY_HXX
#define INCLUDED_SLIDESHOW_TRANSITIONFACTORY_HXX

#include <com/sun/star/animations/XTransitionFilter.hpp>
#include <com/sun/star/presentation/XTransitionFactory.hpp>

#include "animatableshape.hxx"
#include "rgbcolor.hxx"
#include "slide.hxx"
#include "screenupdater.hxx"
#include "animationactivity.hxx"
#include "activitiesfactory.hxx"
#include "numberanimation.hxx"
#include "transitioninfo.hxx"
#include "soundplayer.hxx"

#include <boost/utility.hpp>

namespace slideshow
{
    namespace internal
    {
        /* Definition of Transitionfactory class */
        class TransitionFactory : private boost::noncopyable
        {
        public:
            /** Create a transition effect for shapes.

                This method creates an AnimationActivity, which, when
                run, performs the requested transition effect on the
                given shape.

                @param rParms
                Collection of activity parameters, see ActivitiesFactory

                @param rShape
                Shape to animate

                @param rShapeManager
                ShapeManager, to manage shape animation

                @param xTransition
                The transition effect

                @return the created activity, or NULL for no
                transition effect
             */
            static AnimationActivitySharedPtr createShapeTransition(
                const ActivitiesFactory::CommonParameters&              rParms,
                const AnimatableShapeSharedPtr&                         rShape,
                const ShapeManagerSharedPtr&                            rShapeManager,
                const ::basegfx::B2DVector&                             rSlideSize,
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::animations::XTransitionFilter > const&  xTransition );


            /** Create a transition effect for slides.

                This method creates a NumberAnimation, which,
                when run, performs the requested transition effect
                with the slide bitmaps.

                @param rEnteringBitmap
                Bitmap of the slide which 'enters' the screen.

                @param rLeavingBitmap
                Bitmap of the slide which 'leaves' the screen.

                @param nTransitionType
                Type of the transition (see XTransitionFilter)

                @param nTransitionSubType
                Subtype of the transition (see XTransitionFilter)

                @param bTransitionDirection
                Direction of the transition (see XTransitionFilter)

                @param rTransitionFadeColor
                Optional fade color for the transition

                @return the created animation, or NULL for no
                transition effect
             */
            static NumberAnimationSharedPtr createSlideTransition(
                const SlideSharedPtr&                          rLeavingSlide,
                const SlideSharedPtr&                          rEnteringSlide,
                const UnoViewContainer&                        rViewContainer,
                ScreenUpdater&                                 rScreenUpdater,
                EventMultiplexer&                              rEventMultiplexer,
                const com::sun::star::uno::Reference<
                      com::sun::star::presentation::XTransitionFactory>&
                                                               xOptionalFactory,
                sal_Int16                                      nTransitionType,
                sal_Int16                                      nTransitionSubType,
                bool                                           bTransitionDirection,
                const RGBColor&                                rTransitionFadeColor,
                const SoundPlayerSharedPtr&                    rSoundPlayer );

        private:
            static const TransitionInfo* getTransitionInfo(
                sal_Int16 nTransitionType, sal_Int16 nTransitionSubType );
            static const TransitionInfo* getRandomTransitionInfo();

            static AnimationActivitySharedPtr createShapeTransition(
                const ActivitiesFactory::CommonParameters&              rParms,
                const AnimatableShapeSharedPtr&                         rShape,
                const ShapeManagerSharedPtr&                            rShapeManager,
                const ::basegfx::B2DVector&                             rSlideSize,
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::animations::XTransitionFilter > const& xTransition,
                sal_Int16                                               nTransitionType,
                sal_Int16                                               nTransitionSubType );

            // static factory
            TransitionFactory();
            ~TransitionFactory();
        };
    }
}

#endif /* INCLUDED_SLIDESHOW_TRANSITIONFACTORY_HXX */
