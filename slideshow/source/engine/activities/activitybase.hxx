/*************************************************************************
 *
 *  $RCSfile: activitybase.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 16:58:13 $
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

            /** Hook for derived classes

                This method will be called from the first
                perform() invocation, to signal the start of the
                activity.
            */
            virtual void start() = 0;

            /// From Disposable interface
            virtual void dispose();

            /** From Activity interface

                Derived classes should override, call this first
                and then perform their work.
            */
            virtual bool perform();

            virtual bool isActive() const;
            virtual bool needsScreenUpdate() const;
            virtual void end();

            // From AnimationActivity interface
            virtual void setTargets( const AnimatableShapeSharedPtr&        rShape,
                                     const ShapeAttributeLayerSharedPtr&    rAttrLayer );

        protected:
            /** Modify fractional time.

                This method modifies the fractional time (total
                duration mapped to the [0,1] range) to the
                effective simple time, but only according to
                acceleration/deceleration.
            */
            double calcAcceleratedTime( double nT ) const;

            // TODO(Q2): Wrap direct member access with protected
            // accessor methods

            EventSharedPtr                              mpEndEvent;
            EventQueue&                                 mrEventQueue;
            AnimatableShapeSharedPtr                    mpShape;            // only to pass on to animation
            ShapeAttributeLayerSharedPtr                mpAttributeLayer;   // only to pass on to animation

            const ::comphelper::OptionalValue<double>   maRepeats;
            const double                                mnAccelerationFraction;
            const double                                mnDecelerationFraction;

            const bool                                  mbAutoReverse;

            bool                                        mbFirstPerformCall; // true, if perform() has not yet been called
            bool                                        mbIsActive;
        };
    }
}

#endif /* _SLIDESHOW_ACTIVITYBASE_HXX */
