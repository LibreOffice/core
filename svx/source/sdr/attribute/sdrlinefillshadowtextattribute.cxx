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



#include "precompiled_svx.hxx"

#include <svx/sdr/attribute/sdrlinefillshadowtextattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        SdrLineFillShadowTextAttribute::SdrLineFillShadowTextAttribute(
            const SdrLineAttribute& rLine,
            const SdrFillAttribute& rFill,
            const SdrLineStartEndAttribute& rLineStartEnd,
            const SdrShadowAttribute& rShadow,
            const FillGradientAttribute& rFillFloatTransGradient,
            const SdrTextAttribute& rTextAttribute)
        :   SdrLineShadowTextAttribute(rLine, rLineStartEnd, rShadow, rTextAttribute),
            maFill(rFill),
            maFillFloatTransGradient(rFillFloatTransGradient)
        {
        }

        SdrLineFillShadowTextAttribute::SdrLineFillShadowTextAttribute()
        :   SdrLineShadowTextAttribute(),
            maFill(),
            maFillFloatTransGradient()
        {
        }

        SdrLineFillShadowTextAttribute::SdrLineFillShadowTextAttribute(const SdrLineFillShadowTextAttribute& rCandidate)
        :   SdrLineShadowTextAttribute(rCandidate),
            maFill(rCandidate.getFill()),
            maFillFloatTransGradient(rCandidate.getFillFloatTransGradient())
        {
        }

        SdrLineFillShadowTextAttribute& SdrLineFillShadowTextAttribute::operator=(const SdrLineFillShadowTextAttribute& rCandidate)
        {
            SdrLineShadowTextAttribute::operator=(rCandidate);
            maFill = rCandidate.getFill();
            maFillFloatTransGradient = rCandidate.getFillFloatTransGradient();

            return *this;
        }

        bool SdrLineFillShadowTextAttribute::isDefault() const
        {
            return (SdrLineShadowTextAttribute::isDefault()
                && getFill().isDefault()
                && getFillFloatTransGradient().isDefault());
        }

        bool SdrLineFillShadowTextAttribute::operator==(const SdrLineFillShadowTextAttribute& rCandidate) const
        {
            return(SdrLineShadowTextAttribute::operator==(rCandidate)
                && getFill() == rCandidate.getFill()
                && getFillFloatTransGradient() == rCandidate.getFillFloatTransGradient());
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
