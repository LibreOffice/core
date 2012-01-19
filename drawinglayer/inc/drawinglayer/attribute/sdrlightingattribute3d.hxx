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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLIGHTINGATTRIBUTE3D_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLIGHTINGATTRIBUTE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <vector>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace basegfx {
    class BColor;
    class B3DVector;
}

namespace drawinglayer { namespace attribute {
    class ImpSdrLightingAttribute;
    class Sdr3DLightAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC SdrLightingAttribute
        {
        private:
            ImpSdrLightingAttribute*            mpSdrLightingAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            SdrLightingAttribute(
                const basegfx::BColor& rAmbientLight,
                const ::std::vector< Sdr3DLightAttribute >& rLightVector);
            SdrLightingAttribute();
            SdrLightingAttribute(const SdrLightingAttribute& rCandidate);
            SdrLightingAttribute& operator=(const SdrLightingAttribute& rCandidate);
            ~SdrLightingAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrLightingAttribute& rCandidate) const;

            // data read access
            const basegfx::BColor& getAmbientLight() const;
            const ::std::vector< Sdr3DLightAttribute >& getLightVector() const;

            // color model solver
            basegfx::BColor solveColorModel(
                const basegfx::B3DVector& rNormalInEyeCoordinates,
                const basegfx::BColor& rColor, const basegfx::BColor& rSpecular,
                const basegfx::BColor& rEmission, sal_uInt16 nSpecularIntensity) const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRLIGHTINGATTRIBUTE3D_HXX

// eof
