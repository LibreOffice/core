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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_HIDDENGEOMETRYPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_HIDDENGEOMETRYPRIMITIVE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>
#include <drawinglayer/primitive3d/groupprimitive3d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        // This primitive is used to represent geometry for non-visible objects,
        // e.g. a 3D cube without fill attributes. To still be able to use
        // primitives for HitTest functionality, the 3d decompositions produce
        // an as much as possible simplified fill geometry encapsulated in this
        // primtive when there is no fill geometry. Currently, the 3d hit test
        // uses only areas, so maybe in a further enchanced version this will change
        // to 'if neither filled nor lines' creation criteria. The whole primitive
        // decomposes to nothing, so no one not knowing it will be influenced. Only
        // helper processors for hit test (and maybe BoundRect extractors) will
        // use it and it's children subcontent.
        class DRAWINGLAYER_DLLPUBLIC HiddenGeometryPrimitive3D : public GroupPrimitive3D
        {
        public:
            HiddenGeometryPrimitive3D(const Primitive3DSequence& rChildren);

            // despite returning an empty decomposition since it's no visualisation data,
            // range calculation is intended to use hidden geometry, so
            // the local implementation will return the children's range
            virtual basegfx::B3DRange getB3DRange(const geometry::ViewInformation3D& rViewInformation) const;

            /// The default implementation returns an empty sequence
            virtual Primitive3DSequence get3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const;

            // provide unique ID
            DeclPrimitrive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_HIDDENGEOMETRYPRIMITIVE3D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
