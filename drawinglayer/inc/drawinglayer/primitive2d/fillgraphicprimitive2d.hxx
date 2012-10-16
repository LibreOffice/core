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



#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_FILLGRAPHICPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_FILLGRAPHICPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <drawinglayer/attribute/fillgraphicattribute.hxx>

//////////////////////////////////////////////////////////////////////////////
// FillbitmapPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** FillGraphicPrimitive2D class

            This class defines a bitmap filling for a rectangular area. The
            Range is defined by the Transformation, the fill by the FillGraphicAttribute.
            There, the fill consists of a Bitmap (not transparent) defining the fill data
            and a Point/Vector pair defining the relative position/size [0.0 .. 1.0]
            inside the area where the bitmap is positioned. A flag defines then if this
            is tiled or not.

            Renderers should handle this primitive; it has a geometrically correct
            decomposition, but on pixel oututs the areas where the tiled pieces are
            aligned tend to show up (one overlapping or empty pixel)
         */
        class DRAWINGLAYER_DLLPUBLIC FillGraphicPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the geometric definition
            basegfx::B2DHomMatrix                       maTransformation;

            /// the fill attributes
            attribute::FillGraphicAttribute             maFillGraphic;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            FillGraphicPrimitive2D(
                const basegfx::B2DHomMatrix& rTransformation,
                const attribute::FillGraphicAttribute& rFillGraphic);

            /// data read access
            const basegfx::B2DHomMatrix& getTransformation() const { return maTransformation; }
            const attribute::FillGraphicAttribute& getFillGraphic() const { return maFillGraphic; }

            /// compare operator
            virtual bool operator==( const BasePrimitive2D& rPrimitive ) const;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_FILLGRAPHICPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
