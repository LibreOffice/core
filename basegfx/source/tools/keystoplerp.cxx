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

#include <basegfx/utils/keystoplerp.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/diagnose.h>

#include <algorithm>

static void validateInput(const std::vector<double>& rKeyStops)
{
#ifdef DBG_UTIL
    OSL_ENSURE( rKeyStops.size() > 1,
                "KeyStopLerp::KeyStopLerp(): key stop vector must have two entries or more" );

    // rKeyStops must be sorted in ascending order
    for( std::size_t i=1, len=rKeyStops.size(); i<len; ++i )
    {
        if( rKeyStops[i-1] > rKeyStops[i] )
            OSL_FAIL( "KeyStopLerp::KeyStopLerp(): time vector is not sorted in ascending order!" );
    }
#else
    (void)rKeyStops;
#endif
}

namespace basegfx
{
    namespace utils
    {
        KeyStopLerp::KeyStopLerp( const std::vector<double>& rKeyStops ) :
            maKeyStops(rKeyStops),
            mnLastIndex(0)
        {
            validateInput(maKeyStops);
        }

        KeyStopLerp::KeyStopLerp( const ::css::uno::Sequence<double>& rKeyStops ) :
            maKeyStops(rKeyStops.getLength()),
            mnLastIndex(0)
        {
            std::copy( rKeyStops.begin(), rKeyStops.end(), maKeyStops.begin() );
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
                std::clamp(fRawLerp,0.0,1.0));
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
