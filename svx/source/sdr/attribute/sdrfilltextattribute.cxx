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

#include <svx/sdr/attribute/sdrfilltextattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        SdrFillTextAttribute::SdrFillTextAttribute(
            const SdrFillAttribute& rFill,
            const FillGradientAttribute& rFillFloatTransGradient,
            const SdrTextAttribute& rTextAttribute)
        :   maFill(rFill),
            maFillFloatTransGradient(rFillFloatTransGradient),
            maTextAttribute(rTextAttribute)
        {
        }

        SdrFillTextAttribute::SdrFillTextAttribute()
        :   maFill(),
            maFillFloatTransGradient(),
            maTextAttribute()
        {
        }

        SdrFillTextAttribute::SdrFillTextAttribute(const SdrFillTextAttribute& rCandidate)
        :   maFill(rCandidate.getFill()),
            maFillFloatTransGradient(rCandidate.getFillFloatTransGradient()),
            maTextAttribute(rCandidate.getText())
        {
        }

        SdrFillTextAttribute& SdrFillTextAttribute::operator=(const SdrFillTextAttribute& rCandidate)
        {
            maFill = rCandidate.getFill();
            maFillFloatTransGradient = rCandidate.getFillFloatTransGradient();
            maTextAttribute = rCandidate.getText();

            return *this;
        }

        bool SdrFillTextAttribute::isDefault() const
        {
            return(getFill().isDefault()
                && getFillFloatTransGradient().isDefault()
                && getText().isDefault());
        }

        bool SdrFillTextAttribute::operator==(const SdrFillTextAttribute& rCandidate) const
        {
            return(getFill() == rCandidate.getFill()
                && getFillFloatTransGradient() == rCandidate.getFillFloatTransGradient()
                && getText() == rCandidate.getText());
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
