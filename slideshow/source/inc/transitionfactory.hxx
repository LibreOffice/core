/*************************************************************************
 *
 *  $RCSfile: transitionfactory.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:22:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SLIDESHOW_TRANSITIONFACTORY_HXX
#define _SLIDESHOW_TRANSITIONFACTORY_HXX

#ifndef _COM_SUN_STAR_ANIMATIONS_XTRANSITIONFILTER_HPP_
#include <com/sun/star/animations/XTransitionFilter.hpp>
#endif
#ifndef _COMPHELPER_OPTIONALVALUE_HXX
#include <comphelper/optionalvalue.hxx>
#endif

#include <unoviewcontainer.hxx>
#include <animatableshape.hxx>
#include <rgbcolor.hxx>
#include <slide.hxx>
#include <layermanager.hxx>
#include <animationactivity.hxx>
#include <activitiesfactory.hxx>
#include <slidechangeanimation.hxx>
#include <transitioninfo.hxx>
#include <soundplayer.hxx>


namespace presentation
{
    namespace internal
    {
        /* Definition of Transitionfactory class */
        class TransitionFactory
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

                @param rLayerManager
                LayerManager, to manage shape animation

                @param xTransition
                The transition effect

                @return the created activity, or NULL for no
                transition effect
             */
            static AnimationActivitySharedPtr createShapeTransition(
                const ActivitiesFactory::CommonParameters&              rParms,
                const AnimatableShapeSharedPtr&                         rShape,
                const LayerManagerSharedPtr&                            rLayerManager,
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::animations::XTransitionFilter >&  xTransition );


            /** Create a transition effect for slides.

                This method creates a SlideChangeAnimation, which,
                when run, performs the requested transition effect
                with the slide bitmaps.

                @param rEnteringBitmap
                Bitmap of the slide which 'enters' the screen.

                @param rLeavingBitmap
                Bitmap of the slide which 'leaves' the screen.

                @param rSlideSize
                Size of the entering slide in user coordinate space

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
            static SlideChangeAnimationSharedPtr createSlideTransition(
                const SlideSharedPtr&                           rLeavingSlide,
                const SlideSharedPtr&                           rEnteringSlide,
                const UnoViewContainer&                         rViews,
                const ::basegfx::B2DSize&                       rSlideSize,
                sal_Int16                                       nTransitionType,
                sal_Int16                                       nTransitionSubType,
                bool                                            bTransitionDirection,
                const ::comphelper::OptionalValue< RGBColor >&  rTransitionFadeColor,
                const SoundPlayerSharedPtr&                     rSoundPlayer );

        private:
            static const TransitionInfo* getTransitionInfo(
                sal_Int16 nTransitionType, sal_Int16 nTransitionSubType );
            static const TransitionInfo* getRandomTransitionInfo();

            static AnimationActivitySharedPtr createShapeTransition(
                const ActivitiesFactory::CommonParameters&              rParms,
                const AnimatableShapeSharedPtr&                         rShape,
                const LayerManagerSharedPtr&                            rLayerManager,
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::animations::XTransitionFilter >&  xTransition,
                sal_Int16                                               nTransitionType,
                sal_Int16                                               nTransitionSubType );

            // default: disabled copy/assignment
            TransitionFactory(const TransitionFactory&);
            TransitionFactory& operator=( const TransitionFactory& );

            // static factory
            TransitionFactory();
            ~TransitionFactory();
        };
    }
}

#endif /* _SLIDESHOW_TRANSITIONFACTORY_HXX */
