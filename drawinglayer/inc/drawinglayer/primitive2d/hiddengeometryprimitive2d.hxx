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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_HIDDENGEOMETRYPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_HIDDENGEOMETRYPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        // This primitive is used to represent geometry for non-visible objects,
        // e.g. a PresObj's outline. To still be able to use primitives for HitTest
        // functionality, the 2d decompositions will produce an as much as possible
        // simplified line geometry encapsulated in this primtive when there is no
        // line geometry. In a further enchanced version this may change to 'if neither
        // filled nor lines' creation criteria. The whole primitive decomposes to nothing,
        // so no one not knowing it will be influenced. Only helper processors for hit test
        // (and maybe BoundRect extractors) will use it and it's children subcontent.
        class DRAWINGLAYER_DLLPUBLIC HiddenGeometryPrimitive2D : public GroupPrimitive2D
        {
        public:
            HiddenGeometryPrimitive2D(const Primitive2DSequence& rChildren);

            // despite returning an empty decomposition since it's no visualisation data,
            // range calculation is intended to use hidden geometry, so
            // the local implementation will return the children's range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// local decomposition. Implementation will return empty Primitive2DSequence
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_HIDDENGEOMETRYPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
