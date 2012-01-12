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
#include "precompiled_basegfx.hxx"

#include "basegfx/tools/keystoplerp.hxx"
#include <com/sun/star/uno/Sequence.hxx>

#include <algorithm>

static void validateInput(const std::vector<double>& rKeyStops)
{
    (void)rKeyStops;
#ifdef DBG_UTIL
    OSL_ENSURE( rKeyStops.size() > 1,
                "KeyStopLerp::KeyStopLerp(): key stop vector must have two entries or more" );

    // rKeyStops must be sorted in ascending order
    for( ::std::size_t i=1, len=rKeyStops.size(); i<len; ++i )
    {
        if( rKeyStops[i-1] > rKeyStops[i] )
            OSL_ENSURE( false,
                        "KeyStopLerp::KeyStopLerp(): time vector is not sorted in ascending order!" );
    }
#endif
}

namespace basegfx
{
    namespace tools
    {
        KeyStopLerp::KeyStopLerp( const std::vector<double>& rKeyStops ) :
            maKeyStops(rKeyStops),
            mnLastIndex(0)
        {
            validateInput(maKeyStops);
        }

        KeyStopLerp::KeyStopLerp( const ::com::sun::star::uno::Sequence<double>& rKeyStops ) :
            maKeyStops(rKeyStops.getLength()),
            mnLastIndex(0)
        {
            std::copy( rKeyStops.getConstArray(),
                       rKeyStops.getConstArray()+rKeyStops.getLength(),
                       maKeyStops.begin() );
            validateInput(maKeyStops);
        }

        KeyStopLerp::ResultType KeyStopLerp::lerp(double fAlpha) const
        {
            // cached value still okay?
            if( maKeyStops.at(mnLastIndex) < fAlpha ||
                maKeyStops.at(mnLastIndex+1) >= fAlpha )
            {
                // nope, find new index
                mnLastIndex = std::min<std::ptrdiff_t>(
                    maKeyStops.size()-2,
                    // range is ensured by max below
                    std::max<std::ptrdiff_t>(
                        0,
                        std::distance( maKeyStops.begin(),
                                       std::lower_bound( maKeyStops.begin(),
                                                         maKeyStops.end(),
                                                         fAlpha )) - 1 ));
            }

            // lerp between stop and stop+1
            const double fRawLerp=
                (fAlpha-maKeyStops.at(mnLastIndex)) /
                (maKeyStops.at(mnLastIndex+1) - maKeyStops.at(mnLastIndex));

            // clamp to permissible range (input fAlpha might be
            // everything)
            return ResultType(
                mnLastIndex,
                clamp(fRawLerp,0.0,1.0));
        }
    }
}
