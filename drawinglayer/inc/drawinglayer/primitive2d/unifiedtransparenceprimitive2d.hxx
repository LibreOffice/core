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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_UNIFIEDTRANSPARENCEPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_UNIFIEDTRANSPARENCEPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** UnifiedTransparencePrimitive2D class

            This primitive encapsualtes a child hierarchy and defines
            that it shall be visualized with the given transparency. That
            transparency is unique for all contained geometry, so that
            e.g. overlapping polygons in the child geometry will not show
            regions of combined transparency, but be all rendered with the
            defined, single transparency.
         */
        class DRAWINGLAYER_DLLPUBLIC UnifiedTransparencePrimitive2D : public GroupPrimitive2D
        {
        private:
            /// the unified transparence
            double                              mfTransparence;

        public:
            /// constructor
            UnifiedTransparencePrimitive2D(
                const Primitive2DSequence& rChildren,
                double fTransparence);

            /// data read access
            double getTransparence() const { return mfTransparence; }

            /// own getB2DRange implementation to include transparent geometries to BoundRect calculations
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// create decomposition
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_UNIFIEDTRANSPARENCEPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
