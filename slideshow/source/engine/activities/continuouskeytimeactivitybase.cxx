/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

// must be first
#include <canvas/debug.hxx>
#include <tools/diagnose_ex.h>
#include <canvas/verbosetrace.hxx>

#include <continuouskeytimeactivitybase.hxx>

#include <boost/tuple/tuple.hpp>
#include <algorithm>
#include <iterator>


namespace slideshow
{
    namespace internal
    {
        ContinuousKeyTimeActivityBase::ContinuousKeyTimeActivityBase( const ActivityParameters& rParms ) :
            SimpleContinuousActivityBase( rParms ),
            maLerper( rParms.maDiscreteTimes )
        {
            ENSURE_OR_THROW( rParms.maDiscreteTimes.size() > 1,
                              "ContinuousKeyTimeActivityBase::ContinuousKeyTimeActivityBase(): key times vector must have two entries or more" );
            ENSURE_OR_THROW( rParms.maDiscreteTimes.front() == 0.0,
                              "ContinuousKeyTimeActivityBase::ContinuousKeyTimeActivityBase(): key times vector first entry must be zero" );
            ENSURE_OR_THROW( rParms.maDiscreteTimes.back() <= 1.0,
                              "ContinuousKeyTimeActivityBase::ContinuousKeyTimeActivityBase(): key times vector last entry must be less or equal 1" );
        }

        void ContinuousKeyTimeActivityBase::simplePerform( double       nSimpleTime,
                                                           sal_uInt32   nRepeatCount ) const
        {
            // calc simple time from global time - sweep through the
            // array multiple times for repeated animations (according to
            // SMIL spec).
            double fAlpha( calcAcceleratedTime( nSimpleTime ) );
            std::ptrdiff_t nIndex;

            boost::tuples::tie(nIndex,fAlpha) = maLerper.lerp(fAlpha);

            perform(
                nIndex,
                fAlpha,
                nRepeatCount );
        }
    }
}
