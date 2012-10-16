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



#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLGRAPHICATTRIBUTE_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLGRAPHICATTRIBUTE_HXX

#include <drawinglayer/drawinglayerdllapi.h>
//////////////////////////////////////////////////////////////////////////////
// predefines

class Graphic;

namespace basegfx {
    class B2DRange;
}

namespace drawinglayer { namespace attribute {
    class ImpFillGraphicAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC FillGraphicAttribute
        {
        private:
            ImpFillGraphicAttribute*        mpFillGraphicAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            FillGraphicAttribute(
                const Graphic& rGraphic,
                const basegfx::B2DRange& rGraphicRange,
                bool bTiling = false,
                double fOffsetX = 0.0,
                double fOffsetY = 0.0);
            FillGraphicAttribute();
            FillGraphicAttribute(const FillGraphicAttribute& rCandidate);
            FillGraphicAttribute& operator=(const FillGraphicAttribute& rCandidate);
            ~FillGraphicAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const FillGraphicAttribute& rCandidate) const;

            // data read access
            const Graphic& getGraphic() const;
            const basegfx::B2DRange& getGraphicRange() const;
            bool getTiling() const;
            double getOffsetX() const;
            double getOffsetY() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLGRAPHICATTRIBUTE_HXX

// eof
