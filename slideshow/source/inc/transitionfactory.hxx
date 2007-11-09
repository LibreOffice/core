/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: transitionfactory.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2007-11-09 10:19:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
