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



#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_QUADRATICSHADOWPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_QUADRATICSHADOWPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/primitivetools2d.hxx>
#include <vcl/bitmapex.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>

//////////////////////////////////////////////////////////////////////////////
// DiscreteShadowPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** DiscreteShadow data class

         */
        class DRAWINGLAYER_DLLPUBLIC DiscreteShadow
        {
        private:
            /// the original shadow BitmapEx in a special form
            BitmapEx                    maBitmapEx;

            /// buffered extracted parts of CombinedShadow for easier usage
            BitmapEx                    maTopLeft;
            BitmapEx                    maTop;
            BitmapEx                    maTopRight;
            BitmapEx                    maRight;
            BitmapEx                    maBottomRight;
            BitmapEx                    maBottom;
            BitmapEx                    maBottomLeft;
            BitmapEx                    maLeft;

        public:
            /// constructor
            DiscreteShadow(const BitmapEx& rBitmapEx);

            /// data read access
            const BitmapEx& getBitmapEx() const { return maBitmapEx; }

            /// helper accesses which create on-demand needed segments
            const BitmapEx& getTopLeft() const;
            const BitmapEx& getTop() const;
            const BitmapEx& getTopRight() const;
            const BitmapEx& getRight() const;
            const BitmapEx& getBottomRight() const;
            const BitmapEx& getBottom() const;
            const BitmapEx& getBottomLeft() const;
            const BitmapEx& getLeft() const;
        };

        /** DiscreteShadowPrimitive2D class

         */
        class DRAWINGLAYER_DLLPUBLIC DiscreteShadowPrimitive2D : public DiscreteMetricDependentPrimitive2D
        {
        private:
            // the object transformation of the rectangular object
            basegfx::B2DHomMatrix       maTransform;

            // the bitmap shadow data
            DiscreteShadow              maDiscreteShadow;

        protected:
            /// create local decomposition
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            DiscreteShadowPrimitive2D(
                const basegfx::B2DHomMatrix& rTransform,
                const DiscreteShadow& rDiscreteShadow);

            /// data read access
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
            const DiscreteShadow& getDiscreteShadow() const { return maDiscreteShadow; }

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PRIMITIVE2D_QUADRATICSHADOWPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
