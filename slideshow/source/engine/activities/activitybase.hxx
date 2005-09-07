/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: activitybase.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:34:45 $
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

#ifndef _SLIDESHOW_ACTIVITYBASE_HXX
#define _SLIDESHOW_ACTIVITYBASE_HXX

#include <animationactivity.hxx>
#include <activityparameters.hxx>
#include <animatableshape.hxx>
#include <shapeattributelayer.hxx>


namespace presentation
{
    namespace internal
    {
        /** Base class for animation activities.

            This whole class hierarchy is only for code sharing
            between the various specializations (with or without
            key times, fully discrete, etc.).
        */
        class ActivityBase : public AnimationActivity
        {
        public:
            ActivityBase( const ActivityParameters& rParms );

            /// From Disposable interface
            virtual void dispose();

            /** From Activity interface

                Derived classes should override, call this first
                and then perform their work.
            */
            virtual bool perform();
            virtual double calcTimeLag() const;

            virtual bool isActive() const;
            virtual bool needsScreenUpdate() const;
            // virtual void dequeued() = 0; must be overridden by derived classes

            // From AnimationActivity interface
            virtual void setTargets( const AnimatableShapeSharedPtr&        rShape,
                                     const ShapeAttributeLayerSharedPtr&    rAttrLayer );

        protected:
            /** Hook for derived classes

                This method will be called from the first
                perform() invocation, to signal the start of the
                activity.
            */
            virtual void startAnimation() = 0;

            /** Hook for derived classes

                This method will be called after the last perform()
                invocation, and after the potential changes of that
                perform() call are committed to screen. That is, in
                endAnimation(), the animation objects (sprites,
                animation) can safely be destroyed, without causing
                visible artifacts on screen.
            */
            virtual void endAnimation() = 0;

            /** End this activity, in a regular way.

                This method is for derived classes needing to signal a
                regular activity end (i.e. because the regular
                duration is over)
             */
            void endActivity();

            /** Modify fractional time.

                This method modifies the fractional time (total
                duration mapped to the [0,1] range) to the
                effective simple time, but only according to
                acceleration/deceleration.
            */
            double calcAcceleratedTime( double nT ) const;

            EventQueue&                     getEventQueue() const { return mrEventQueue; }
            AnimatableShapeSharedPtr        getShape() const { return mpShape; }
            ShapeAttributeLayerSharedPtr    getShapeAttributeLayer() const { return mpAttributeLayer; }
            bool                            isRepeatCountValid() const { return maRepeats.isValid(); }
            double                          getRepeatCount() const { return maRepeats.getValue(); }
            bool                            isAutoReverse() const { return mbAutoReverse; }

        private:
            EventSharedPtr                              mpEndEvent;
            EventQueue&                                 mrEventQueue;
            AnimatableShapeSharedPtr                    mpShape;            // only to pass on to animation
            ShapeAttributeLayerSharedPtr                mpAttributeLayer;   // only to pass on to animation

            const ::comphelper::OptionalValue<double>   maRepeats;
            const double                                mnAccelerationFraction;
            const double                                mnDecelerationFraction;

            const bool                                  mbAutoReverse;

            mutable bool                                mbFirstPerformCall; // true, if perform() has not yet been called
            bool                                        mbIsActive;
        };
    }
}

#endif /* _SLIDESHOW_ACTIVITYBASE_HXX */
