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



#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MEDIAPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MEDIAPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** MediaPrimitive2D class

            This is a unified primitive for Media visualisation, e.g. animations
            or sounds. It's geometry is defined by Transform. For conveinience,
            it also contains a discrete border size (aka Pixels) which will be added
            if used. This makes it a view-dependent primitive. It also gets a filled
            background and the decomposition will try to create a graphic representation
            if te content (defined by the URL), e.g. a still frome for animated stuff.
         */
        class DRAWINGLAYER_DLLPUBLIC MediaPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the geometry definition
            basegfx::B2DHomMatrix                       maTransform;

            /// the content definition
            rtl::OUString                               maURL;

            /// style: background color
            basegfx::BColor                             maBackgroundColor;

            /// discrete border (in 'pixels')
            sal_uInt32                                  mnDiscreteBorder;

        protected:
            /// local decomposition
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            MediaPrimitive2D(
                const basegfx::B2DHomMatrix& rTransform,
                const rtl::OUString& rURL,
                const basegfx::BColor& rBackgroundColor,
                sal_uInt32 nDiscreteBorder);

            /// data read access
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
            const rtl::OUString& getURL() const { return maURL; }
            const basegfx::BColor& getBackgroundColor() const { return maBackgroundColor; }
            sal_uInt32 getDiscreteBorder() const { return mnDiscreteBorder; }

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MEDIAPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
