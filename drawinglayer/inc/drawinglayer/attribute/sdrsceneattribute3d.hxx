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

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRSCENEATTRIBUTE3D_HXX
#define INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRSCENEATTRIBUTE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <com/sun/star/drawing/ProjectionMode.hpp>
#include <com/sun/star/drawing/ShadeMode.hpp>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace drawinglayer { namespace attribute {
    class ImpSdrSceneAttribute;
}}

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace attribute
    {
        class DRAWINGLAYER_DLLPUBLIC SdrSceneAttribute
        {
        private:
            ImpSdrSceneAttribute*               mpSdrSceneAttribute;

        public:
            /// constructors/assignmentoperator/destructor
            SdrSceneAttribute(
                double fDistance,
                double fShadowSlant,
                ::com::sun::star::drawing::ProjectionMode aProjectionMode,
                ::com::sun::star::drawing::ShadeMode aShadeMode,
                bool bTwoSidedLighting);
            SdrSceneAttribute();
            SdrSceneAttribute(const SdrSceneAttribute& rCandidate);
            SdrSceneAttribute& operator=(const SdrSceneAttribute& rCandidate);
            ~SdrSceneAttribute();

            // checks if the incarnation is default constructed
            bool isDefault() const;

            // compare operator
            bool operator==(const SdrSceneAttribute& rCandidate) const;

            // data read access
            double getDistance() const;
            double getShadowSlant() const;
            ::com::sun::star::drawing::ProjectionMode getProjectionMode() const;
            ::com::sun::star::drawing::ShadeMode getShadeMode() const;
            bool getTwoSidedLighting() const;
        };
    } // end of namespace attribute
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_ATTRIBUTE_SDRSCENEATTRIBUTE3D_HXX

// eof
