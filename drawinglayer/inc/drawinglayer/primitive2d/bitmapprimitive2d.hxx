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



#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BITMAPPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BITMAPPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <vcl/bitmapex.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////
// BitmapPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** BitmapPrimitive2D class

            This class is the central primitive for Bitmap-based primitives.
            It provides RGBA-based bitmaps, currently using a BitmapEx from VCL.
            This may change in the future to any other, maybe more general base
            class providing 24bit RGBA.
         */
        class DRAWINGLAYER_DLLPUBLIC BitmapPrimitive2D : public BasePrimitive2D
        {
        private:
            /// the RGBA Bitmap-data
            BitmapEx                                    maBitmapEx;

            /** the object transformation from unit coordinates, defining
                size, shear, rotate and position
             */
            basegfx::B2DHomMatrix                       maTransform;

        public:
            /// constructor
            BitmapPrimitive2D(
                const BitmapEx& rBitmapEx,
                const basegfx::B2DHomMatrix& rTransform);

            /// data read access
            const BitmapEx& getBitmapEx() const { return maBitmapEx; }
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BITMAPPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
