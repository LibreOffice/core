/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
