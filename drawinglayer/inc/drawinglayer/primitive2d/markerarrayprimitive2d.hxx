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



#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MARKERARRAYPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MARKERARRAYPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <vcl/bitmapex.hxx>

//////////////////////////////////////////////////////////////////////////////
// MarkerArrayPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** MarkerArrayPrimitive2D class

            This primtive defines an array of markers. Their size is defined
            in pixels and independent from the view transformation which makes
            this primitive highly view-dependent. It is also transformation
            invariant, so that the bitmap is always visualized unscaled and
            unrotated.
            It is used e.g. for grid position visualisation. The given Bitmap
            (with transparence) is defined to be visible centered at each of the given
            positions.
            It decomposes to the needed number of BitmapPrimitive2D's, so it would
            be efficient to handle it directly in a renderer.
         */
        class DRAWINGLAYER_DLLPUBLIC MarkerArrayPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the positions for the marker
            std::vector< basegfx::B2DPoint >                maPositions;

            /// the marker definintion to visualize
            BitmapEx                                        maMarker;

        protected:
            /// create local decomposition
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            MarkerArrayPrimitive2D(
                const std::vector< basegfx::B2DPoint >& rPositions,
                const BitmapEx& rMarker);

            /// data read access
            const std::vector< basegfx::B2DPoint >& getPositions() const { return maPositions; }
            const BitmapEx& getMarker() const { return maMarker; }

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MARKERARRAYPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
