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


#include <tools/diagnose_ex.h>

#include <continuouskeytimeactivitybase.hxx>

#include <algorithm>
#include <iterator>
#include <tuple>

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

            std::tie(nIndex,fAlpha) = maLerper.lerp(fAlpha);

            perform(
                nIndex,
                fAlpha,
                nRepeatCount );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
