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



#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_DISCRETEBITMAPPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_DISCRETEBITMAPPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/primitivetools2d.hxx>
#include <vcl/bitmapex.hxx>

//////////////////////////////////////////////////////////////////////////////
// DiscreteBitmapPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** DiscreteBitmapPrimitive2D class

            This class defines a view-dependent BitmapPrimitive which has a
            logic position for the top-left position and is always to be
            painted in 1:1 pixel resolution. It will never be sheared, rotated
            or scaled with the view.
         */
        class DRAWINGLAYER_DLLPUBLIC DiscreteBitmapPrimitive2D : public ObjectAndViewTransformationDependentPrimitive2D
        {
        private:
            /// the RGBA Bitmap-data
            BitmapEx                                    maBitmapEx;

            /** the top-left object position */
            basegfx::B2DPoint                           maTopLeft;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            DiscreteBitmapPrimitive2D(
                const BitmapEx& rBitmapEx,
                const basegfx::B2DPoint& rTopLeft);

            /// data read access
            const BitmapEx& getBitmapEx() const { return maBitmapEx; }
            const basegfx::B2DPoint& getTopLeft() const { return maTopLeft; }

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PRIMITIVE2D_DISCRETEBITMAPPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
