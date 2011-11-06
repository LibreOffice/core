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

#include <svx/sdr/attribute/sdrlineshadowtextattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        SdrLineShadowTextAttribute::SdrLineShadowTextAttribute(
            const SdrLineAttribute& rLine,
            const SdrLineStartEndAttribute& rLineStartEnd,
            const SdrShadowAttribute& rShadow,
            const SdrTextAttribute& rTextAttribute)
        :   SdrShadowTextAttribute(rShadow, rTextAttribute),
            maLine(rLine),
            maLineStartEnd(rLineStartEnd)
        {
        }

        SdrLineShadowTextAttribute::SdrLineShadowTextAttribute()
        :   SdrShadowTextAttribute(),
            maLine(),
            maLineStartEnd()
        {
        }

        SdrLineShadowTextAttribute::SdrLineShadowTextAttribute(const SdrLineShadowTextAttribute& rCandidate)
        :   SdrShadowTextAttribute(rCandidate),
            maLine(rCandidate.getLine()),
            maLineStartEnd(rCandidate.getLineStartEnd())
        {
        }

        SdrLineShadowTextAttribute& SdrLineShadowTextAttribute::operator=(const SdrLineShadowTextAttribute& rCandidate)
        {
            SdrShadowTextAttribute::operator=(rCandidate);
            maLine = rCandidate.getLine();
            maLineStartEnd = rCandidate.getLineStartEnd();

            return *this;
        }

        bool SdrLineShadowTextAttribute::isDefault() const
        {
            return(SdrShadowTextAttribute::isDefault()
                && getLine().isDefault()
                && getLineStartEnd().isDefault());
        }

        bool SdrLineShadowTextAttribute::operator==(const SdrLineShadowTextAttribute& rCandidate) const
        {
            return(SdrShadowTextAttribute::operator==(rCandidate)
                && getLine() == rCandidate.getLine()
                && getLineStartEnd() == rCandidate.getLineStartEnd());
        }
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
