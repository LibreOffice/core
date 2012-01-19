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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLIGHTATTRIBUTE3D_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLIGHTATTRIBUTE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx {
    class BColor;
    class B3DVector;
}

namespace drawinglayer { namespace attribute {
    class ImpSdr3DLightAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC Sdr3DLightAttribute
        {
        private:
            ImpSdr3DLightAttribute*             mpSdr3DLightAttribute;

        public:
            // constructors/destructor
            Sdr3DLightAttribute(
                const basegfx::BColor& rColor,
                const basegfx::B3DVector& rDirection,
                bool bSpecular);
            Sdr3DLightAttribute(const basegfx::BColor& rColor);
            Sdr3DLightAttribute();
            Sdr3DLightAttribute(const Sdr3DLightAttribute& rCandidate);
            ~Sdr3DLightAttribute();

            // assignment operator
            Sdr3DLightAttribute& operator=(const Sdr3DLightAttribute& rCandidate);

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const Sdr3DLightAttribute& rCandidate) const;

            // data read access
            const basegfx::BColor& getColor() const;
            const basegfx::B3DVector& getDirection() const;
            bool getSpecular() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLIGHTATTRIBUTE3D_HXX

// eof
