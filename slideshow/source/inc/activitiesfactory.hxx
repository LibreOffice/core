/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: activitiesfactory.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:03:04 $
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

#ifndef _SLIDESHOW_ACTIVITIESFACTORY_HXX
#define _SLIDESHOW_ACTIVITIESFACTORY_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_XANIMATE_HPP_
#include <com/sun/star/animations/XAnimate.hpp>
#endif
#ifndef _COM_SUN_STAR_ANIMATIONS_XANIMATECOLOR_HPP_
#include <com/sun/star/animations/XAnimateColor.hpp>
#endif

#ifndef _COMPHELPER_OPTIONALVALUE_HXX
#include <comphelper/optionalvalue.hxx>
#endif

#include <animationactivity.hxx>
#include <activitiesqueue.hxx>
#include <event.hxx>
#include <eventqueue.hxx>
#include <shape.hxx>
#include <layermanager.hxx>

#include <numberanimation.hxx>
#include <enumanimation.hxx>
#include <coloranimation.hxx>
#include <hslcoloranimation.hxx>
#include <stringanimation.hxx>
#include <boolanimation.hxx>
#include <pairanimation.hxx>


/* Definition of ActivitiesFactory class */

namespace presentation
{
    namespace internal
    {

        class ActivitiesFactory
        {
        public:
            /// Collection of common factory parameters
            struct CommonParameters
            {
                CommonParameters( const EventSharedPtr&                 rEndEvent,
                                  EventQueue&                           rEventQueue,
                                  ActivitiesQueue&                      rActivitiesQueue,
                                  double                                nMinDuration,
                                  sal_uInt32                            nMinNumberOfFrames,
                                  bool                                  bAutoReverse,
                                  ::comphelper::OptionalValue<double>   aRepeats,
                                  double                                nAcceleration,
                                  double                                nDeceleration,
                                  const ShapeSharedPtr&                 rShape,
                                  const LayerManagerSharedPtr&          rLayerManager ) :
                    mpEndEvent( rEndEvent ),
                    mrEventQueue( rEventQueue ),
                    mrActivitiesQueue( rActivitiesQueue ),
                    mnMinDuration( nMinDuration ),
                    mnMinNumberOfFrames( nMinNumberOfFrames ),
                    maRepeats( aRepeats ),
                    mnAcceleration( nAcceleration ),
                    mnDeceleration( nDeceleration ),
                    mpShape( rShape ),
                    mpLayerManager( rLayerManager ),
                    mbAutoReverse( bAutoReverse )
                {
                }

                /// End event to fire when animation is over
                EventSharedPtr                                  mpEndEvent;

                /// Event queue to insert the end event into.
                EventQueue&                                     mrEventQueue;
                /// Event queue to insert the end event into.
                ActivitiesQueue&                                mrActivitiesQueue;

                /** Simple duration of the activity

                    Specifies the minimal simple duration of the
                    activity (minimal, because mnMinNumberOfFrames
                    might prolongue the activity). According to SMIL,
                    this might also be indefinite, which for our
                    framework does not make much sense, though
                    (wouldn't have a clue, then, how to scale the
                    animation over time).
                 */
                double                                          mnMinDuration;

                /** Minimal number of frames for this activity.

                    This specifies the minimal number of frames this
                    activity will display per simple duration. If less
                    than this number are displayed until mnMinDuration
                    is over, the activity will be prolongued until
                    mnMinNumberOfFrames are rendered.
                 */
                sal_uInt32                                      mnMinNumberOfFrames;

                /** Number of repeats for the simple duration

                    This specified the number of repeats. The
                    mnMinDuration times maRepeats yields the total
                    duration of this activity. If this value is
                    unspecified, the activity will repeat
                    indefinitely.
                 */
                ::comphelper::OptionalValue<double>             maRepeats;

                /// Fraction of simple time to accelerate animation
                double                                          mnAcceleration;

                /// Fraction of simple time to decelerate animation
                double                                          mnDeceleration;

                /// Shape, to get bounds from
                ShapeSharedPtr                                  mpShape;

                /// LayerManager, to get page size from
                LayerManagerSharedPtr                           mpLayerManager;

                /// When true, activity is played reversed after mnDuration.
                bool                                            mbAutoReverse;
            };

            /** Create an activity from an XAnimate node.

                This method creates an animated activity from the
                given XAnimate node, extracting all necessary
                animation parameters from that. Note that due to the
                animator parameter, the animation values must be
                convertible to a double value.

                @param rParms
                Factory parameter structure

                @param rAnimator
                Animator sub-object

                @param xNode
                The SMIL animation node to animate
             */
            static AnimationActivitySharedPtr createAnimateActivity( const CommonParameters&                        rParms,
                                                                     const NumberAnimationSharedPtr&                rAnimator,
                                                                     const ::com::sun::star::uno::Reference<
                                                                         ::com::sun::star::animations::XAnimate >&  xNode );

            /** Create an activity from an XAnimate node.

                This method creates an animated activity from the
                given XAnimate node, extracting all necessary
                animation parameters from that. Note that due to the
                animator parameter, the animation values must be
                convertible to a double value.

                @param rParms
                Factory parameter structure

                @param rAnimator
                Animator sub-object

                @param xNode
                The SMIL animation node to animate
             */
            static AnimationActivitySharedPtr createAnimateActivity( const CommonParameters&                        rParms,
                                                                     const EnumAnimationSharedPtr&                  rAnimator,
                                                                     const ::com::sun::star::uno::Reference<
                                                                         ::com::sun::star::animations::XAnimate >&  xNode );

            /** Create an activity from an XAnimate node.

                This method creates an animated activity from the
                given XAnimate node, extracting all necessary
                animation parameters from that. Note that due to the
                animator parameter, the animation values must be
                convertible to a color value.

                @param rParms
                Factory parameter structure

                @param rAnimator
                Animator sub-object

                @param xNode
                The SMIL animation node to animate
             */
            static AnimationActivitySharedPtr createAnimateActivity( const CommonParameters&                        rParms,
                                                                     const ColorAnimationSharedPtr&                 rAnimator,
                                                                     const ::com::sun::star::uno::Reference<
                                                                         ::com::sun::star::animations::XAnimate >&  xNode );

            /** Create an activity from an XAnimate node.

                This method creates an animated activity from the
                given XAnimate node, extracting all necessary
                animation parameters from that. Note that due to the
                animator parameter, the animation values must be
                convertible to a color value.

                @param rParms
                Factory parameter structure

                @param rAnimator
                Animator sub-object

                @param xNode
                The SMIL animation node to animate
             */
            static AnimationActivitySharedPtr createAnimateActivity( const CommonParameters&                            rParms,
                                                                     const HSLColorAnimationSharedPtr&                  rAnimator,
                                                                     const ::com::sun::star::uno::Reference<
                                                                         ::com::sun::star::animations::XAnimateColor >& xNode );

            /** Create an activity from an XAnimate node.

                This method creates an animated activity from the
                given XAnimate node, extracting all necessary
                animation parameters from that. Note that due to the
                animator parameter, the animation values must be
                convertible to a pair of double values.

                @param rParms
                Factory parameter structure

                @param rAnimator
                Animator sub-object

                @param xNode
                The SMIL animation node to animate
             */
            static AnimationActivitySharedPtr createAnimateActivity( const CommonParameters&                        rParms,
                                                                     const PairAnimationSharedPtr&                  rAnimator,
                                                                     const ::com::sun::star::uno::Reference<
                                                                         ::com::sun::star::animations::XAnimate >&  xNode );

            /** Create an activity from an XAnimate node.

                This method creates an animated activity from the
                given XAnimate node, extracting all necessary
                animation parameters from that. Note that due to the
                animator parameter, the animation values must be
                convertible to a string.

                @param rParms
                Factory parameter structure

                @param rAnimator
                Animator sub-object

                @param xNode
                The SMIL animation node to animate
             */
            static AnimationActivitySharedPtr createAnimateActivity( const CommonParameters&                        rParms,
                                                                     const StringAnimationSharedPtr&                rAnimator,
                                                                     const ::com::sun::star::uno::Reference<
                                                                         ::com::sun::star::animations::XAnimate >&  xNode );

            /** Create an activity from an XAnimate node.

                This method creates an animated activity from the
                given XAnimate node, extracting all necessary
                animation parameters from that. Note that due to the
                animator parameter, the animation values must be
                convertible to a bool value.

                @param rParms
                Factory parameter structure

                @param rAnimator
                Animator sub-object

                @param xNode
                The SMIL animation node to animate
             */
            static AnimationActivitySharedPtr createAnimateActivity( const CommonParameters&                        rParms,
                                                                     const BoolAnimationSharedPtr&                  rAnimator,
                                                                     const ::com::sun::star::uno::Reference<
                                                                         ::com::sun::star::animations::XAnimate >&  xNode );

            /** Create a simple activity for the given animator

                This method is suited to create activities for custom
                animations, which need a simple double value and lasts
                a given timespan. This activity always generates values
                from the [0,1] range.

                @param rParms
                Factory parameter structure

                @param rAnimator
                Animator sub-object

                @param bDirectionForward
                If true, the activity goes 'forward', i.e. from 0 to
                1. With false, the direction is reversed.
            */
            static AnimationActivitySharedPtr createSimpleActivity( const CommonParameters&         rParms,
                                                                    const NumberAnimationSharedPtr& rAnimator,
                                                                    bool                            bDirectionForward );

        private:
            // default: constructor/destructor disabed
            ActivitiesFactory();
            ~ActivitiesFactory();

            // default: disabled copy/assignment
            ActivitiesFactory(const ActivitiesFactory&);
            ActivitiesFactory& operator=( const ActivitiesFactory& );
        };
    }
}

#endif /* _SLIDESHOW_ACTIVITIESFACTORY_HXX */
