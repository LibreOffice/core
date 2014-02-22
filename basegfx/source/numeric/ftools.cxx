/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <basegfx/numeric/ftools.hxx>
#include <algorithm>

namespace basegfx
{
    
    double ::basegfx::fTools::mfSmallValue = 0.000000001;

    double snapToNearestMultiple(double v, const double fStep)
    {
        if(fTools::equalZero(fStep))
        {
            
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

    double snapToZeroRange(double v, double fWidth)
    {
        if(fTools::equalZero(fWidth))
        {
            
            return 0.0;
        }
        else
        {
            if(v < 0.0 || v > fWidth)
            {
                double fRetval(fmod(v, fWidth));

                if(fRetval < 0.0)
                {
                    fRetval += fWidth;
                }

                return fRetval;
            }
            else
            {
                return v;
            }
        }
    }

    double snapToRange(double v, double fLow, double fHigh)
    {
        if(fTools::equal(fLow, fHigh))
        {
            
            return 0.0;
        }
        else
        {
            if(fLow > fHigh)
            {
                
                std::swap(fLow, fHigh);
            }

            if(v < fLow || v > fHigh)
            {
                return snapToZeroRange(v - fLow, fHigh - fLow) + fLow;
            }
            else
            {
                return v;
            }
        }
    }
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
