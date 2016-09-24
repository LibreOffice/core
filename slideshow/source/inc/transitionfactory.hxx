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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_TRANSITIONFACTORY_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_TRANSITIONFACTORY_HXX

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

namespace slideshow
{
    namespace internal
    {
        /* Definition of Transitionfactory class */
        namespace TransitionFactory
        {
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
            AnimationActivitySharedPtr createShapeTransition(
                const ActivitiesFactory::CommonParameters&              rParms,
                const AnimatableShapeSharedPtr&                         rShape,
                const ShapeManagerSharedPtr&                            rShapeManager,
                const ::basegfx::B2DVector&                             rSlideSize,
                css::uno::Reference< css::animations::XTransitionFilter > const&  xTransition );


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
            NumberAnimationSharedPtr createSlideTransition(
                const SlideSharedPtr&                          rLeavingSlide,
                const SlideSharedPtr&                          rEnteringSlide,
                const UnoViewContainer&                        rViewContainer,
                ScreenUpdater&                                 rScreenUpdater,
                EventMultiplexer&                              rEventMultiplexer,
                const css::uno::Reference< css::presentation::XTransitionFactory>&
                                                               xOptionalFactory,
                sal_Int16                                      nTransitionType,
                sal_Int16                                      nTransitionSubType,
                bool                                           bTransitionDirection,
                const RGBColor&                                rTransitionFadeColor,
                const SoundPlayerSharedPtr&                    rSoundPlayer );
        }
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_TRANSITIONFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
