/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: continuouskeytimeactivitybase.cxx,v $
 * $Revision: 1.8 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include <continuouskeytimeactivitybase.hxx>

#include <algorithm>
#include <iterator>


namespace slideshow
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
                    maKeyTimes.size()-2,
                    // range is ensured by max below
                    ::std::max< ::std::ptrdiff_t >(
                        0,
                        ::std::distance( maKeyTimes.begin(),
                                         ::std::lower_bound( maKeyTimes.begin(),
                                                             maKeyTimes.end(),
                                                             nT ) ) - 1 ) );
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
