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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_PAIRANIMATION_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_PAIRANIMATION_HXX

#include "animation.hxx"
#include <basegfx/tuple/b2dtuple.hxx>


/* Definition of PairAnimation interface */

namespace slideshow
{
    namespace internal
    {
        /** Interface defining a pair of numbers animation.

            This interface is a specialization of the Animation
            interface, and is used to animate attributes representable
            by a pair of floating point values (e.g. a position or a
            size).
         */
        class PairAnimation : public Animation
        {
        public:
            typedef ::basegfx::B2DTuple ValueType;

            /** Set the animation to the given value

                @param rValue
                Current animation value.
             */
            virtual bool operator()( const ::basegfx::B2DTuple& rValue ) = 0;

            /** Request the underlying value for this animation.

                This is necessary for pure To or By animations, as the
                Activity cannot determine a sensible start value
                otherwise.

                @attention Note that you are only permitted to query
                for the underlying value, if the animation has actually
                been started (via start() call).
             */
            virtual ::basegfx::B2DTuple getUnderlyingValue() const = 0;
        };

        typedef ::std::shared_ptr< PairAnimation > PairAnimationSharedPtr;

    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_PAIRANIMATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
