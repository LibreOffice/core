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



#ifndef _SDR_ATTRIBUTE_SDRFORMTEXTATTRIBUTE_HXX
#define _SDR_ATTRIBUTE_SDRFORMTEXTATTRIBUTE_HXX

#include <sal/types.h>
#include <svx/xenum.hxx>
#include <tools/color.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

class SfxItemSet;

namespace drawinglayer { namespace attribute {
    class ImpSdrFormTextAttribute;
    class SdrFormTextOutlineAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class SdrFormTextAttribute
        {
        private:
            ImpSdrFormTextAttribute*            mpSdrFormTextAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            SdrFormTextAttribute(const SfxItemSet& rSet);
            SdrFormTextAttribute();
            SdrFormTextAttribute(const SdrFormTextAttribute& rCandidate);
            SdrFormTextAttribute& operator=(const SdrFormTextAttribute& rCandidate);
            ~SdrFormTextAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrFormTextAttribute& rCandidate) const;

            // data read access
            sal_Int32 getFormTextDistance() const;
            sal_Int32 getFormTextStart() const;
            sal_Int32 getFormTextShdwXVal() const;
            sal_Int32 getFormTextShdwYVal() const;
            sal_uInt16 getFormTextShdwTransp() const;
            XFormTextStyle getFormTextStyle() const;
            XFormTextAdjust getFormTextAdjust() const;
            XFormTextShadow getFormTextShadow() const;
            Color getFormTextShdwColor() const;
            const SdrFormTextOutlineAttribute& getOutline() const;
            const SdrFormTextOutlineAttribute& getShadowOutline() const;
            bool getFormTextMirror() const;
            bool getFormTextOutline() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // _SDR_ATTRIBUTE_SDRFORMTEXTATTRIBUTE_HXX

// eof
