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

#include <basegfx/numeric/ftools.hxx>
#include <algorithm>

namespace basegfx
{
    double snapToNearestMultiple(double v, const double fStep)
    {
        if(fTools::equalZero(fStep))
        {
            // with a zero step, all snaps to 0.0
            return 0.0;
        }
        else
        {
            const double fHalfStep(fStep * 0.5);
            const double fChange(fHalfStep - fmod(v + fHalfStep, fStep));

            if(basegfx::fTools::equal(fabs(v), fabs(fChange)))
            {
                return 0.0;
            }
            else
            {
                return v + fChange;
            }
        }
    }

    double normalizeToRange(double v, const double fRange)
    {
        if(fTools::lessOrEqual(fRange, 0.0))
        {
            // with a zero (or less) range, all normalizes to 0.0
            return 0.0;
        }

        const bool bNegative(fTools::less(v, 0.0));

        if(bNegative)
        {
            if(fTools::moreOrEqual(v, -fRange))
            {
                // in range [-fRange, 0.0[, shift one step
                return v + fRange;
            }

            // re-calculate
            return v - (floor(v/fRange)*fRange);
        }
        else
        {
            if(fTools::less(v, fRange))
            {
                // already in range [0.0, fRange[, nothing to do
                return v;
            }

            // re-calculate
            return v - (floor(v/fRange)*fRange);
        }
    }
} // end of namespace basegfx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
