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



#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRFILLGRAPHICATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRFILLGRAPHICATTRIBUTE_HXX

#include <drawinglayer/drawinglayerdllapi.h>

//////////////////////////////////////////////////////////////////////////////
// predefines

class Graphic;

namespace basegfx {
    class B2DRange;
    class B2DVector;
}

namespace drawinglayer { namespace attribute {
    class FillGraphicAttribute;
    class ImpSdrFillGraphicAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC SdrFillGraphicAttribute
        {
        private:
            ImpSdrFillGraphicAttribute*     mpSdrFillGraphicAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            SdrFillGraphicAttribute(
                const Graphic& rFillGraphic,
                const basegfx::B2DVector& rGraphicLogicSize,
                const basegfx::B2DVector& rSize,
                const basegfx::B2DVector& rOffset,
                const basegfx::B2DVector& rOffsetPosition,
                const basegfx::B2DVector& rRectPoint,
                bool bTiling,
                bool bStretch,
                bool bLogSize);
            SdrFillGraphicAttribute();
            SdrFillGraphicAttribute(const SdrFillGraphicAttribute& rCandidate);
            SdrFillGraphicAttribute& operator=(const SdrFillGraphicAttribute& rCandidate);
            ~SdrFillGraphicAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrFillGraphicAttribute& rCandidate) const;

            // data read access
            const Graphic& getFillGraphic() const;
            const basegfx::B2DVector& getGraphicLogicSize() const;
            const basegfx::B2DVector& getSize() const;
            const basegfx::B2DVector& getOffset() const;
            const basegfx::B2DVector& getOffsetPosition() const;
            const basegfx::B2DVector& getRectPoint() const;
            bool getTiling() const;
            bool getStretch() const;
            bool getLogSize() const;

            // FillGraphicAttribute generator
            FillGraphicAttribute createFillGraphicAttribute(const basegfx::B2DRange& rRange) const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRFILLGRAPHICATTRIBUTE_HXX

// eof
