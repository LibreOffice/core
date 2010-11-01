/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: canvastools.hxx,v $
 * $Revision: 1.10 $
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

#ifndef _BGFX_TOOLS_KEYSTOPLERP_HXX
#define _BGFX_TOOLS_KEYSTOPLERP_HXX

#include <basegfx/numeric/ftools.hxx>
#include <vector>

namespace com{ namespace sun{ namespace star{ namespace uno {
    template<typename T> class Sequence;
}}}}

namespace basegfx
{
    namespace tools
    {
        /** Lerp in a vector of key stops

            This class holds a key stop vector and provides the
            functionality to lerp inside it. Useful e.g. for
            multi-stop gradients, or the SMIL key time activity.

            For those, given a global [0,1] lerp alpha, one need to
            find the suitable bucket index from key stop vector, and
            then calculate the relative alpha between the two buckets
            found.
         */
        class KeyStopLerp
        {
        public:
            typedef std::pair<std::ptrdiff_t,double> ResultType;

            /** Create lerper with given vector of stops

                @param rKeyStops

                Vector of stops, must contain at least two elements
                (though preferrably more, otherwise you probably don't
                need key stop lerping in the first place). All
                elements must be of monotonically increasing value.
             */
            explicit KeyStopLerp( const std::vector<double>& rKeyStops );

            /** Create lerper with given sequence of stops

                @param rKeyStops

                Sequence of stops, must contain at least two elements
                (though preferrably more, otherwise you probably don't
                need key stop lerping in the first place). All
                elements must be of monotonically increasing value.
             */
            explicit KeyStopLerp( const ::com::sun::star::uno::Sequence<double>& rKeyStops );

            /** Find two nearest bucket index & interpolate

                @param fAlpha
                Find bucket index i, with keyStops[i] < fAlpha <=
                keyStops[i+1]. Return new alpha value in [0,1),
                proportional to fAlpha's position between keyStops[i]
                and keyStops[i+1]
             */
            ResultType lerp(double fAlpha) const;

        private:
            std::vector<double>    maKeyStops;
            mutable std::ptrdiff_t mnLastIndex;
        };
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
