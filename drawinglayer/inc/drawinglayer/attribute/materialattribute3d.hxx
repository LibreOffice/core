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



#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_MATERIALATTRIBUTE3D_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_MATERIALATTRIBUTE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
//////////////////////////////////////////////////////////////////////////////
// predefines

namespace drawinglayer { namespace attribute {
    class ImpMaterialAttribute3D;
}}

namespace basegfx {
    class BColor;
}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC MaterialAttribute3D
        {
        private:
            ImpMaterialAttribute3D*                             mpMaterialAttribute3D;

        public:
            // constructors/destructor
            MaterialAttribute3D(
                const basegfx::BColor& rColor,
                const basegfx::BColor& rSpecular,
                const basegfx::BColor& rEmission,
                sal_uInt16 nSpecularIntensity);
            MaterialAttribute3D(const basegfx::BColor& rColor);
            MaterialAttribute3D();
            MaterialAttribute3D(const MaterialAttribute3D& rCandidate);
            ~MaterialAttribute3D();

            // assignment operator
            MaterialAttribute3D& operator=(const MaterialAttribute3D& rCandidate);

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const MaterialAttribute3D& rCandidate) const;

            // data read access
            const basegfx::BColor& getColor() const;
            const basegfx::BColor& getSpecular() const;
            const basegfx::BColor& getEmission() const;
            sal_uInt16 getSpecularIntensity() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_MATERIALATTRIBUTE3D_HXX

// eof
