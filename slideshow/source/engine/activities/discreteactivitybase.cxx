/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: discreteactivitybase.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:36:35 $
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

// must be first
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include <discreteactivitybase.hxx>


namespace presentation
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
            ENSURE_AND_THROW( mpWakeupEvent.get(),
                              "DiscreteActivityBase::DiscreteActivityBase(): Invalid wakeup event" );

            ENSURE_AND_THROW( !maDiscreteTimes.empty(),
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
                    ENSURE_AND_THROW( false, "DiscreteActivityBase::DiscreteActivityBase(): time values not within [0,1] range!" );
                }

                if( maDiscreteTimes[i-1] > maDiscreteTimes[i] )
                    ENSURE_AND_THROW( false, "DiscreteActivityBase::DiscreteActivityBase(): time vector is not sorted in ascending order!" );
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
            double nCurrRepeat( (double)mnCurrPerformCalls / nVectorSize );

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
                //
                // Somewhat condensed, the argument for setNextTimeout below could
                // be written as
                //
                // mnSimpleDuration*(nFullRepeats + calcAcceleratedTime( currentRepeatTime )),
                //
                // with currentRepeatTime = maDiscreteTimes[ currentRepeatIndex ]
                //
                // Note that calcAcceleratedTime() is only applied to the current repeat's value,
                // not to the total resulting time. This is in accordance with the SMIL spec.
                //
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
            if( mpWakeupEvent.get() )
                mpWakeupEvent->dispose();

            // release references
            mpWakeupEvent.reset();

            ActivityBase::dispose();
        }

        void DiscreteActivityBase::dequeued()
        {
            // ignored here, if we're still active. Discrete
            // activities are dequeued after every perform() call,
            // thus, the call is only significant when isActive() ==
            // false.
            if( !isActive() )
                endAnimation();
        }

    }
}
