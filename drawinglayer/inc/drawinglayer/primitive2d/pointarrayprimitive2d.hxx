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



#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POINTARRAYPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POINTARRAYPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////
// PointArrayPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** PointArrayPrimitive2D class

            This primitive defines single,discrete 'pixels' for the given
            positions in the given color. This makes it view-dependent since
            the logic size of a 'pixel' depends on the view transformation.

            This is one of the non-decomposable primitives, so a renderer
            should process it (Currently it is only used for grid visualisation,
            but this may change).
         */
        class DRAWINGLAYER_DLLPUBLIC PointArrayPrimitive2D : public BasePrimitive2D
        {
        private:
            /// the array of positions
            std::vector< basegfx::B2DPoint >                maPositions;

            /// the color to use
            basegfx::BColor                                 maRGBColor;

            /// #i96669# add simple range buffering for this primitive
            basegfx::B2DRange                               maB2DRange;

        public:
            /// constructor
            PointArrayPrimitive2D(
                const std::vector< basegfx::B2DPoint >& rPositions,
                const basegfx::BColor& rRGBColor);

            /// data read access
            const std::vector< basegfx::B2DPoint >& getPositions() const { return maPositions; }
            const basegfx::BColor& getRGBColor() const { return maRGBColor; }

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POINTARRAYPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
