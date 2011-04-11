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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
