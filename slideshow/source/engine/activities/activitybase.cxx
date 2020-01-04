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

#include <sal/config.h>

#include <algorithm>

#include <tools/diagnose_ex.h>
#include <canvas/canvastools.hxx>

#include "activitybase.hxx"


namespace slideshow
{
    namespace internal
    {
        // TODO(P1): Elide some virtual function calls, by templifying this
        // static hierarchy

        ActivityBase::ActivityBase( const ActivityParameters& rParms ) :
            mpEndEvent( rParms.mrEndEvent ),
            mrEventQueue( rParms.mrEventQueue ),
            mpShape(),
            mpAttributeLayer(),
            maRepeats( rParms.mrRepeats ),
            mnAccelerationFraction( rParms.mnAccelerationFraction ),
            mnDecelerationFraction( rParms.mnDecelerationFraction ),
            mbAutoReverse( rParms.mbAutoReverse ),
            mbFirstPerformCall( true ),
            mbIsActive( true ) {}

        void ActivityBase::dispose()
        {
            // deactivate
            mbIsActive = false;

            // dispose event
            if( mpEndEvent )
                mpEndEvent->dispose();

            // release references
            mpEndEvent.reset();
            mpShape.reset();
            mpAttributeLayer.reset();
        }

        double ActivityBase::calcTimeLag() const
        {
            // TODO(Q1): implement different init process!
            if (isActive() && mbFirstPerformCall)
            {
                mbFirstPerformCall = false;

                // notify derived classes that we're
                // starting now
                const_cast<ActivityBase *>(this)->startAnimation();
            }
            return 0.0;
        }

        bool ActivityBase::perform()
        {
            // still active?
            if( !isActive() )
                return false; // no, early exit.

            OSL_ASSERT( ! mbFirstPerformCall );

            return true;
        }

        bool ActivityBase::isActive() const
        {
            return mbIsActive;
        }

        void ActivityBase::setTargets( const AnimatableShapeSharedPtr&      rShape,
                                       const ShapeAttributeLayerSharedPtr&  rAttrLayer )
        {
            ENSURE_OR_THROW( rShape,
                              "ActivityBase::setTargets(): Invalid shape" );
            ENSURE_OR_THROW( rAttrLayer,
                              "ActivityBase::setTargets(): Invalid attribute layer" );

            mpShape = rShape;
            mpAttributeLayer = rAttrLayer;
        }

        void ActivityBase::endActivity()
        {
            // this is a regular activity end
            mbIsActive = false;

            // Activity is ending, queue event, then
            if( mpEndEvent )
                mrEventQueue.addEvent( mpEndEvent );

            // release references
            mpEndEvent.reset();
        }

        void ActivityBase::dequeued()
        {
            // xxx todo:
//             // ignored here, if we're still active. Discrete
//             // activities are dequeued after every perform() call,
//             // thus, the call is only significant when isActive() ==
//             // false.
            if( !isActive() )
                endAnimation();
        }

        void ActivityBase::end()
        {
            if (!isActive() || isDisposed())
                return;
            // assure animation is started:
            if (mbFirstPerformCall) {
                mbFirstPerformCall = false;
                // notify derived classes that we're starting now
                startAnimation();
            }

            performEnd(); // calling private virtual
            endAnimation();
            endActivity();
        }

        double ActivityBase::calcAcceleratedTime( double nT ) const
        {
            // Handle acceleration/deceleration
            // ================================

            // clamp nT to permissible [0,1] range
            nT = std::clamp( nT, 0.0, 1.0 );

            // take acceleration/deceleration into account. if the sum
            // of mnAccelerationFraction and mnDecelerationFraction
            // exceeds 1.0, ignore both (that's according to SMIL spec)
            if( (mnAccelerationFraction > 0.0 ||
                 mnDecelerationFraction > 0.0) &&
                mnAccelerationFraction + mnDecelerationFraction <= 1.0 )
            {
                /*
                // calc accelerated/decelerated time.

                // We have three intervals:
                // 1 [0,a]
                // 2 [a,d]
                // 3 [d,1] (with a and d being acceleration/deceleration
                // fraction, resp.)

                // The change rate during interval 1 is constantly
                // increasing, reaching 1 at a. It then stays at 1,
                // starting a linear decrease at d, ending with 0 at
                // time 1. The integral of this function is the
                // required new time nT'.

                // As we arbitrarily assumed 1 as the upper value of
                // the change rate, the integral must be normalized to
                // reach nT'=1 at the end of the interval. This
                // normalization constant is:

                // c = 1 - 0.5a - 0.5d

                // The integral itself then amounts to:

                // 0.5 nT^2 / a + (nT-a) + (nT - 0.5 nT^2 / d)

                // (where each of the three summands correspond to the
                // three intervals above, and are applied only if nT
                // has reached the corresponding interval)

                // The graph of the change rate is a trapezoid:

                //   |
                //  1|      /--------------\
                //   |     /                \
                //   |    /                  \
                //   |   /                    \
                //   -----------------------------
                //      0   a              d  1

                //*/
                const double nC( 1.0 - 0.5*mnAccelerationFraction - 0.5*mnDecelerationFraction );

                // this variable accumulates the new time value
                double nTPrime(0.0);

                if( nT < mnAccelerationFraction )
                {
                    nTPrime += 0.5*nT*nT/mnAccelerationFraction; // partial first interval
                }
                else
                {
                    nTPrime += 0.5*mnAccelerationFraction; // full first interval

                    if( nT <= 1.0-mnDecelerationFraction )
                    {
                        nTPrime += nT-mnAccelerationFraction; // partial second interval
                    }
                    else
                    {
                        nTPrime += 1.0 - mnAccelerationFraction - mnDecelerationFraction; // full second interval

                        const double nTRelative( nT - 1.0 + mnDecelerationFraction );

                        nTPrime += nTRelative - 0.5*nTRelative*nTRelative / mnDecelerationFraction;
                    }
                }

                // normalize, and assign to work variable
                nT = nTPrime / nC;
            }

            return nT;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
