/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: simplecontinuousactivitybase.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:37:43 $
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

#include <simplecontinuousactivitybase.hxx>


namespace presentation
{
    namespace internal
    {
        SimpleContinuousActivityBase::SimpleContinuousActivityBase(
            const ActivityParameters& rParms ) :
            ActivityBase( rParms ),
            maTimer( rParms.mrActivitiesQueue.getTimer() ),
            mnMinSimpleDuration( rParms.mnMinDuration ),
            mnMinNumberOfFrames( rParms.mnMinNumberOfFrames ),
            mnCurrPerformCalls( 0 )
        {
        }

        void SimpleContinuousActivityBase::startAnimation()
        {
            // init timer. We measure animation time only when we're
            // actually started.
            maTimer.reset();
        }

        double SimpleContinuousActivityBase::calcTimeLag() const
        {
            ActivityBase::calcTimeLag();
            if (! isActive())
                return 0.0;

            // retrieve locally elapsed time
            const double nCurrElapsedTime( maTimer.getElapsedTime() );

            // log time
            VERBOSE_TRACE( "SimpleContinuousActivityBase::calcTimeLag(): "
                           "next step is based on time: %f", nCurrElapsedTime );

            // go to great length to ensure a proper animation
            // run. Since we don't know how often we will be called
            // here, try to spread the animator calls uniquely over
            // the [0,1] parameter range. Be aware of the fact that
            // perform will be called at least mnMinNumberOfTurns
            // times.

            // fraction of time elapsed
            const double nFractionElapsedTime(
                nCurrElapsedTime / mnMinSimpleDuration );

            // fraction of minimum calls performed
            const double nFractionRequiredCalls(
                (double)mnCurrPerformCalls / mnMinNumberOfFrames );

            // okay, so now, the decision is easy:
            //
            // If the fraction of time elapsed is smaller than the
            // number of calls required to be performed, then we calc
            // the position on the animation range according to
            // elapsed time. That is, we're so to say ahead of time.
            //
            // In contrary, if the fraction of time elapsed is larger,
            // then we're lagging, and we thus calc the position on
            // the animation time line according to the fraction of
            // calls performed. Thus, the animation is forced to slow
            // down, and take the required minimal number of steps,
            // sufficiently equally distributed across the animation
            // time line.
            if( nFractionElapsedTime < nFractionRequiredCalls )
            {
                VERBOSE_TRACE( "SimpleContinuousActivityBase::calcTimeLag(): "
                               "t=%f is based on time", nFractionElapsedTime );
                return 0.0;
            }
            else
            {
                VERBOSE_TRACE( "SimpleContinuousActivityBase::perform(): "
                               "t=%f is based on number of calls",
                               nFractionRequiredCalls );

                // lag global time, so all other animations lag, too:
                return ((nFractionElapsedTime - nFractionRequiredCalls)
                        * mnMinSimpleDuration);
            }
        }

        bool SimpleContinuousActivityBase::perform()
        {
            // call base class, for start() calls and end handling
            if( !ActivityBase::perform() )
                return false; // done, we're ended


            // get relative animation position
            // ===============================

            const double nCurrElapsedTime( maTimer.getElapsedTime() );
            double nT( nCurrElapsedTime / mnMinSimpleDuration );


            // one of the stop criteria reached?
            // =================================

            // will be set to true below, if one of the termination criteria
            // matched.
            bool bActivityEnding( false );

            if( isRepeatCountValid() )
            {
                // Finite duration
                // ===============

                // When we've autoreverse on, the repeat count
                // doubles
                const double nRepeatCount( getRepeatCount() );
                const double nEffectiveRepeat( isAutoReverse() ?
                                               2.0*nRepeatCount :
                                               nRepeatCount );

                // time (or frame count) elapsed?
                if( nEffectiveRepeat <= nT )
                {
                    // okee. done for now. Will not exit right here,
                    // to give animation the chance to render the last
                    // frame below
                    bActivityEnding = true;

                    // clamp animation to max permissible value
                    nT = nEffectiveRepeat;
                }
            }


            // need to do auto-reverse?
            // ========================

            double nRepeats;
            double nRelativeSimpleTime;

            // TODO(Q3): Refactor this mess
            if( isAutoReverse() )
            {
                // divert active duration into repeat and
                // fractional part.
                const double nFractionalActiveDuration( modf(nT, &nRepeats) );

                // for auto-reverse, map ranges [1,2), [3,4), ...
                // to ranges [0,1), [1,2), etc.
                if( ((int)nRepeats) % 2 )
                {
                    // we're in an odd range, reverse sweep
                    nRelativeSimpleTime = 1.0 - nFractionalActiveDuration;
                }
                else
                {
                    // we're in an even range, pass on as is
                    nRelativeSimpleTime = nFractionalActiveDuration;
                }

                // effective repeat count for autoreverse is half of
                // the input time's value (each run of an autoreverse
                // cycle is half of a repeat)
                nRepeats /= 2;
            }
            else
            {
                // determine repeat
                // ================

                // calc simple time and number of repeats from nT
                // Now, that's easy, since the fractional part of
                // nT gives the relative simple time, and the
                // integer part the number of full repeats:
                nRelativeSimpleTime = modf(nT, &nRepeats);

                // clamp repeats to max permissible value (maRepeats.getValue() - 1.0)
                if( isRepeatCountValid() &&
                    nRepeats >= getRepeatCount() )
                {
                    // Note that this code here only gets
                    // triggered if maRepeats.getValue() is an
                    // _integer_. Otherwise, nRepeats will never
                    // reach nor exceed
                    // maRepeats.getValue(). Thus, the code below
                    // does not need to handle cases of fractional
                    // repeats, and can always assume that a full
                    // animation run has ended (with
                    // nRelativeSimpleTime=1.0 for
                    // non-autoreversed activities).

                    // with modf, nRelativeSimpleTime will never
                    // become 1.0, since nRepeats is incremented and
                    // nRelativeSimpleTime set to 0.0 then.
                    //
                    // For the animation to reach its final value,
                    // nRepeats must although become
                    // maRepeats.getValue()-1.0, and
                    // nRelativeSimpleTime=1.0.
                    nRelativeSimpleTime = 1.0;
                    nRepeats -= 1.0;
                }
            }

            // actually perform something
            // ==========================

            simplePerform( nRelativeSimpleTime,
                           // nRepeats is already integer-valued
                           static_cast<sal_uInt32>( nRepeats ) );


            // delayed endActivity() call from end condition check
            // below. Issued after the simplePerform() call above, to
            // give animations the chance to correctly reach the
            // animation end value, without spurious bail-outs because
            // of isActive() returning false.
            if( bActivityEnding )
                endActivity();

            // one more frame successfully performed
            ++mnCurrPerformCalls;

            return isActive();
        }

        void SimpleContinuousActivityBase::dequeued()
        {
            if (! isActive())
                endAnimation();
        }

    }
}
