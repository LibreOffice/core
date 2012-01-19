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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLGRADIENTATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLGRADIENTATTRIBUTE_HXX

#include <drawinglayer/drawinglayerdllapi.h>
//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx {
    class BColor;
}

namespace drawinglayer { namespace attribute {
    class ImpFillGradientAttribute;
}}

//////////////////////////////////////////////////////////////////////////////
// declarations

namespace drawinglayer
{
    namespace attribute
    {
        enum GradientStyle
        {
            GRADIENTSTYLE_LINEAR,
            GRADIENTSTYLE_AXIAL,
            GRADIENTSTYLE_RADIAL,
            GRADIENTSTYLE_ELLIPTICAL,
            GRADIENTSTYLE_SQUARE,
            GRADIENTSTYLE_RECT
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC FillGradientAttribute
        {
        private:
            ImpFillGradientAttribute*           mpFillGradientAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            FillGradientAttribute(
                GradientStyle eStyle,
                double fBorder,
                double fOffsetX,
                double fOffsetY,
                double fAngle,
                const basegfx::BColor& rStartColor,
                const basegfx::BColor& rEndColor,
                sal_uInt16 nSteps);
            FillGradientAttribute();
            FillGradientAttribute(const FillGradientAttribute& rCandidate);
            FillGradientAttribute& operator=(const FillGradientAttribute& rCandidate);
            ~FillGradientAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const FillGradientAttribute& rCandidate) const;

            // data read access
            GradientStyle getStyle() const;
            double getBorder() const;
            double getOffsetX() const;
            double getOffsetY() const;
            double getAngle() const;
            const basegfx::BColor& getStartColor() const;
            const basegfx::BColor& getEndColor() const;
            sal_uInt16 getSteps() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLGRADIENTATTRIBUTE_HXX

// eof
