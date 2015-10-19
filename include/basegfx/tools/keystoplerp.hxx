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

#ifndef INCLUDED_BASEGFX_TOOLS_KEYSTOPLERP_HXX
#define INCLUDED_BASEGFX_TOOLS_KEYSTOPLERP_HXX

#include <basegfx/numeric/ftools.hxx>
#include <vector>
#include <basegfx/basegfxdllapi.h>

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
        class BASEGFX_DLLPUBLIC KeyStopLerp
        {
        public:
            typedef std::pair<std::ptrdiff_t,double> ResultType;

            /** Create lerper with given vector of stops

                @param rKeyStops

                Vector of stops, must contain at least two elements
                (though preferably more, otherwise you probably don't
                need key stop lerping in the first place). All
                elements must be of monotonically increasing value.
             */
            explicit KeyStopLerp( const std::vector<double>& rKeyStops );

            /** Create lerper with given sequence of stops

                @param rKeyStops

                Sequence of stops, must contain at least two elements
                (though preferably more, otherwise you probably don't
                need key stop lerping in the first place). All
                elements must be of monotonically increasing value.
             */
            explicit KeyStopLerp( const css::uno::Sequence<double>& rKeyStops );

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
