/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: continuouskeytimeactivitybase.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:35:59 $
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

#include <continuouskeytimeactivitybase.hxx>

#include <algorithm>
#include <iterator>


namespace presentation
{
    namespace internal
    {
        ContinuousKeyTimeActivityBase::ContinuousKeyTimeActivityBase( const ActivityParameters& rParms ) :
            SimpleContinuousActivityBase( rParms ),
            maKeyTimes( rParms.maDiscreteTimes ),
            mnLastIndex( 0 )
        {
            ENSURE_AND_THROW( maKeyTimes.size() > 1,
                              "ContinuousKeyTimeActivityBase::ContinuousKeyTimeActivityBase(): key times vector must have two entries or more" );

#ifdef DBG_UTIL
            // check parameters: rKeyTimes must be sorted in
            // ascending order, and contain values only from the range
            // [0,1]
            for( ::std::size_t i=1, len=maKeyTimes.size(); i<len; ++i )
            {
                if( maKeyTimes[i] < 0.0 ||
                    maKeyTimes[i] > 1.0 ||
                    maKeyTimes[i-1] < 0.0 ||
                    maKeyTimes[i-1] > 1.0 )
                {
                    ENSURE_AND_THROW( false, "ContinuousKeyTimeActivityBase::ContinuousKeyTimeActivityBase(): time values not within [0,1] range!" );
                }

                if( maKeyTimes[i-1] > maKeyTimes[i] )
                {
                    ENSURE_AND_THROW( false, "ContinuousKeyTimeActivityBase::ContinuousKeyTimeActivityBase(): time vector is not sorted in ascending order!" );
                }
            }

            // TODO(E2): check this also in production code?
#endif
        }

        void ContinuousKeyTimeActivityBase::simplePerform( double       nSimpleTime,
                                                           sal_uInt32   nRepeatCount ) const
        {
            // calc simple time from global time - sweep through the
            // array multiple times for repeated animations (according to
            // SMIL spec).
            const double nT( calcAcceleratedTime( nSimpleTime ) );

            // determine position within key times vector from
            // current simple time

            // shortcut: cached value still okay?
            if( maKeyTimes[ mnLastIndex   ] < nT ||
                maKeyTimes[ mnLastIndex+1 ] >= nT  )
            {
                // nope, find new index
                mnLastIndex = ::std::min< ::std::ptrdiff_t >(
                    maKeyTimes.size()-2L,
                    // range is ensured by max below
                    ::std::max( 0L,
                                ::std::distance( maKeyTimes.begin(),
                                                 ::std::lower_bound( maKeyTimes.begin(),
                                                                     maKeyTimes.end(),
                                                                     nT ) ) - 1L ) );
            }

            OSL_ENSURE( mnLastIndex+1 < maKeyTimes.size(),
                        "ContinuousKeyTimeActivityBase::simplePerform(): index out of range" );

            // mnLastIndex is now valid and up-to-date

            // calc current simple time, as a fractional value ([0,1] range).
            // I.e. the relative position between the two index times.
            const double nCurrFractionalSimplTime( (nT - maKeyTimes[ mnLastIndex ]) /
                                                   (maKeyTimes[ mnLastIndex+1 ] - maKeyTimes[ mnLastIndex ]) );

            perform(
                mnLastIndex,
                nCurrFractionalSimplTime,
                nRepeatCount );
        }
    }
}
