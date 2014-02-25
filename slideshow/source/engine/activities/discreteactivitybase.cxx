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


// must be first
#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
#include <canvas/verbosetrace.hxx>

#include <discreteactivitybase.hxx>


namespace slideshow
{
    namespace internal
    {
        DiscreteActivityBase::DiscreteActivityBase( const ActivityParameters& rParms ) :
            ActivityBase( rParms ),
            mpWakeupEvent( rParms.mpWakeupEvent ),
            maDiscreteTimes( rParms.maDiscreteTimes ),
            mnSimpleDuration( rParms.mnMinDuration ),
            mnCurrPerformCalls( 0 )
        {
            ENSURE_OR_THROW( mpWakeupEvent,
                              "DiscreteActivityBase::DiscreteActivityBase(): Invalid wakeup event" );

            ENSURE_OR_THROW( !maDiscreteTimes.empty(),
                              "DiscreteActivityBase::DiscreteActivityBase(): time vector is empty, why do you create me?" );

#ifdef DBG_UTIL
            // check parameters: rDiscreteTimes must be sorted in
            // ascending order, and contain values only from the range
            // [0,1]
            for( ::std::size_t i=1, len=maDiscreteTimes.size(); i<len; ++i )
            {
                if( maDiscreteTimes[i] < 0.0 ||
                    maDiscreteTimes[i] > 1.0 ||
                    maDiscreteTimes[i-1] < 0.0 ||
                    maDiscreteTimes[i-1] > 1.0 )
                {
                    ENSURE_OR_THROW( false, "DiscreteActivityBase::DiscreteActivityBase(): time values not within [0,1] range!" );
                }

                if( maDiscreteTimes[i-1] > maDiscreteTimes[i] )
                    ENSURE_OR_THROW( false, "DiscreteActivityBase::DiscreteActivityBase(): time vector is not sorted in ascending order!" );
            }

            // TODO(E2): check this also in production code?
#endif
        }

        void DiscreteActivityBase::startAnimation()
        {
            // start timer on wakeup event
            mpWakeupEvent->start();
        }

        sal_uInt32 DiscreteActivityBase::calcFrameIndex( sal_uInt32     nCurrCalls,
                                                         ::std::size_t  nVectorSize ) const
        {
            if( isAutoReverse() )
            {
                // every full repeat run consists of one
                // forward and one backward traversal.
                sal_uInt32 nFrameIndex( nCurrCalls % (2*nVectorSize) );

                // nFrameIndex values >= nVectorSize belong to
                // the backward traversal
                if( nFrameIndex >= nVectorSize )
                    nFrameIndex = 2*nVectorSize - nFrameIndex; // invert sweep

                return nFrameIndex;
            }
            else
            {
                return nCurrCalls % nVectorSize ;
            }
        }

        sal_uInt32 DiscreteActivityBase::calcRepeatCount( sal_uInt32    nCurrCalls,
                                                          ::std::size_t nVectorSize ) const
        {
            if( isAutoReverse() )
                return nCurrCalls / (2*nVectorSize); // we've got 2 cycles per repeat
            else
                return nCurrCalls / nVectorSize;
        }

        bool DiscreteActivityBase::perform()
        {
            // call base class, for start() calls and end handling
            if( !ActivityBase::perform() )
                return false; // done, we're ended

            const ::std::size_t nVectorSize( maDiscreteTimes.size() );

            // actually perform something
            // ==========================

            // TODO(Q3): Refactor this mess

            // call derived class with current frame index (modulo
            // vector size, to cope with repeats)
            perform( calcFrameIndex( mnCurrPerformCalls, nVectorSize ),
                     calcRepeatCount( mnCurrPerformCalls, nVectorSize ) );

            // calc next index
            ++mnCurrPerformCalls;

            // calc currently reached repeat count
            double nCurrRepeat( double(mnCurrPerformCalls) / nVectorSize );

            // if auto-reverse is specified, halve the
            // effective repeat count, since we pass every
            // repeat run twice: once forward, once backward.
            if( isAutoReverse() )
                nCurrRepeat /= 2.0;

            // schedule next frame, if either repeat is indefinite
            // (repeat forever), or we've not yet reached the requested
            // repeat count
            if( !isRepeatCountValid() ||
                nCurrRepeat < getRepeatCount() )
            {
                // add wake-up event to queue (modulo
                // vector size, to cope with repeats).

                // repeat is handled locally, only apply acceleration/deceleration.
                // Scale time vector with simple duration, offset with full repeat
                // times.

                // Somewhat condensed, the argument for setNextTimeout below could
                // be written as

                // mnSimpleDuration*(nFullRepeats + calcAcceleratedTime( currentRepeatTime )),

                // with currentRepeatTime = maDiscreteTimes[ currentRepeatIndex ]

                // Note that calcAcceleratedTime() is only applied to the current repeat's value,
                // not to the total resulting time. This is in accordance with the SMIL spec.

                mpWakeupEvent->setNextTimeout(
                    mnSimpleDuration*(
                        calcRepeatCount(
                            mnCurrPerformCalls,
                            nVectorSize ) +
                        calcAcceleratedTime(
                            maDiscreteTimes[
                                calcFrameIndex(
                                    mnCurrPerformCalls,
                                    nVectorSize ) ] ) ) );

                getEventQueue().addEvent( mpWakeupEvent );
            }
            else
            {
                // release event reference (relation to wakeup event
                // is circular!)
                mpWakeupEvent.reset();

                // done with this activity
                endActivity();
            }

            return false; // remove from queue, will be added back by the wakeup event.
        }

        void DiscreteActivityBase::dispose()
        {
            // dispose event
            if( mpWakeupEvent )
                mpWakeupEvent->dispose();

            // release references
            mpWakeupEvent.reset();

            ActivityBase::dispose();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
