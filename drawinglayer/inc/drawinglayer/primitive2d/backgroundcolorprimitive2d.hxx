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



#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BACKGROUNDCOLORPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BACKGROUNDCOLORPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////
// BackgroundColorPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** BackgroundColorPrimitive2D class

            This primitive is defined to fill the whole visible Viewport with
            the given color (and thus decomposes to a filled polygon). This
            makes it a view-depnendent primitive by definition. It only has
            a valid decomposition if a valid Viewport is given in the
            ViewInformation2D at decomposition time.

            It will try to buffer it's last decomposition using maLastViewport
            to detect changes in the get2DDecomposition call.
         */
        class DRAWINGLAYER_DLLPUBLIC BackgroundColorPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the fill color to use
            basegfx::BColor                             maBColor;

            /// the last used viewInformation, used from getDecomposition for buffering
            basegfx::B2DRange                           maLastViewport;

        protected:
            /// create local decomposition
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            BackgroundColorPrimitive2D(
                const basegfx::BColor& rBColor);

            /// data read access
            const basegfx::BColor& getBColor() const { return maBColor; }

            /// get B2Drange
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()

            /// Overload standard getDecomposition call to be view-dependent here
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BACKGROUNDCOLORPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
