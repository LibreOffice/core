/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
            OSL_FAIL( "KeyStopLerp::KeyStopLerp(): time vector is not sorted in ascending order!" );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
