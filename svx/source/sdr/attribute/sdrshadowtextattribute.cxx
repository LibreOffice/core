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

#include <svx/sdr/attribute/sdrshadowtextattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        SdrShadowTextAttribute::SdrShadowTextAttribute(
            const SdrShadowAttribute& rShadow,
            const SdrTextAttribute& rTextAttribute)
        :   maShadow(rShadow),
            maTextAttribute(rTextAttribute)
        {
        }

        SdrShadowTextAttribute::SdrShadowTextAttribute()
        :   maShadow(),
            maTextAttribute()
        {
        }

        SdrShadowTextAttribute::SdrShadowTextAttribute(const SdrShadowTextAttribute& rCandidate)
        :   maShadow(rCandidate.getShadow()),
            maTextAttribute(rCandidate.getText())
        {
        }

        SdrShadowTextAttribute& SdrShadowTextAttribute::operator=(const SdrShadowTextAttribute& rCandidate)
        {
            maShadow = rCandidate.getShadow();
            maTextAttribute = rCandidate.getText();

            return *this;
        }

        bool SdrShadowTextAttribute::isDefault() const
        {
            return (getShadow().isDefault()
                && getText().isDefault());
        }

        bool SdrShadowTextAttribute::operator==(const SdrShadowTextAttribute& rCandidate) const
        {
            return (getShadow() == rCandidate.getShadow()
                && getText() == rCandidate.getText());
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
