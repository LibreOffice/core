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



#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SHADOWPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SHADOWPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        /** ShadowPrimitive2D class

            This primitive defines a generic shadow geometry construction
            for 2D objects. It decomposes to a TransformPrimitive2D embedded
            into a ModifiedColorPrimitive2D.

            It's for primtive usage convenience, so that not everyone has
            to implement the generic shadow construction by himself.

            The same geometry as sequence of primitives is used as geometry and
            as shadow. Since these are RefCounted Uno-Api objects, no extra objects
            are needed for the shadow itself; all the local decompositions of the
            original geometry can be reused from the renderer for shadow visualisation.
        */
        class DRAWINGLAYER_DLLPUBLIC ShadowPrimitive2D : public GroupPrimitive2D
        {
        private:
            /// the shadow transformation, normally just an offset
            basegfx::B2DHomMatrix                   maShadowTransform;

            /// the shadow color to which all geometry is to be forced
            basegfx::BColor                         maShadowColor;

        public:
            /// constructor
            ShadowPrimitive2D(
                const basegfx::B2DHomMatrix& rShadowTransform,
                const basegfx::BColor& rShadowColor,
                const Primitive2DSequence& rChildren);

            /// data read access
            const basegfx::B2DHomMatrix& getShadowTransform() const { return maShadowTransform; }
            const basegfx::BColor& getShadowColor() const { return maShadowColor; }

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            ///  create decomposition
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_SHADOWPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
