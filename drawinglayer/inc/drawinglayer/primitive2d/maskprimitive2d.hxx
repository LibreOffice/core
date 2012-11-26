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



#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MASKPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MASKPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** MaskPrimitive2D class

            This is the central masking primitive. It's a grouping
            primitive and contains a PolyPolygon which defines the visible
            area. Only visualisation parts of the Child primitive sequence
            inside of the mask PolyPolygon is defined to be visible.

            This primitive should be handled by a renderer. If it is not handled,
            it decomposes to it's Child content, and thus the visualisation would
            contaiun no clips.

            The geometrc range of this primitive is completely defined by the Mask
            PolyPolygon since by definition nothing outside of the mask is visible.
         */
        class DRAWINGLAYER_DLLPUBLIC MaskPrimitive2D : public GroupPrimitive2D
        {
        private:
            /// the mask PolyPolygon
            basegfx::B2DPolyPolygon                 maMask;

        public:
            /// constructor
            MaskPrimitive2D(
                const basegfx::B2DPolyPolygon& rMask,
                const Primitive2DSequence& rChildren);

            /// data read access
            const basegfx::B2DPolyPolygon& getMask() const { return maMask; }

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MASKPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
