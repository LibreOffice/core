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



#ifndef _SDR_ATTRIBUTE_SDRLINESHADOWTEXTATTRIBUTE_HXX
#define _SDR_ATTRIBUTE_SDRLINESHADOWTEXTATTRIBUTE_HXX

#include <sal/types.h>
#include <svx/sdr/attribute/sdrshadowtextattribute.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>
#include <drawinglayer/attribute/sdrlinestartendattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class SdrLineShadowTextAttribute : public SdrShadowTextAttribute
        {
            // line, shadow, lineStartEnd and text attributes
            SdrLineAttribute                maLine;                     // line attributes (if used)
            SdrLineStartEndAttribute        maLineStartEnd;             // line start end (if used)

        public:
            SdrLineShadowTextAttribute(
                const SdrLineAttribute& rLine,
                const SdrLineStartEndAttribute& rLineStartEnd,
                const SdrShadowAttribute& rShadow,
                const SdrTextAttribute& rTextAttribute);
            SdrLineShadowTextAttribute();
            SdrLineShadowTextAttribute(const SdrLineShadowTextAttribute& rCandidate);
            SdrLineShadowTextAttribute& operator=(const SdrLineShadowTextAttribute& rCandidate);

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrLineShadowTextAttribute& rCandidate) const;

            // data access
            const SdrLineAttribute& getLine() const { return maLine; }
            const SdrLineStartEndAttribute& getLineStartEnd() const { return maLineStartEnd; }
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // _SDR_ATTRIBUTE_SDRLINESHADOWTEXTATTRIBUTE_HXX

// eof
