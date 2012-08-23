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
#include <basegfx/numeric/ftools.hxx>
#include <algorithm>

namespace basegfx
{
    // init static member of class fTools
    double ::basegfx::fTools::mfSmallValue = 0.000000001;

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

    double snapToZeroRange(double v, double fWidth)
    {
        if(fTools::equalZero(fWidth))
        {
            // with no range all snaps to range bound
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
            // with no range all snaps to range bound
            return 0.0;
        }
        else
        {
            if(fLow > fHigh)
            {
                // correct range order. Evtl. assert this (?)
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
} // end of namespace basegfx

// eof
