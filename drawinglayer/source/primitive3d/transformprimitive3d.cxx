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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive3d/transformprimitive3d.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <drawinglayer/primitive3d/drawinglayer_primitivetypes3d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        TransformPrimitive3D::TransformPrimitive3D(
            const basegfx::B3DHomMatrix& rTransformation,
            const Primitive3DSequence& rChildren)
        :   GroupPrimitive3D(rChildren),
            maTransformation(rTransformation)
        {
        }

        basegfx::B3DRange TransformPrimitive3D::getB3DRange(const geometry::ViewInformation3D& rViewInformation) const
        {
            basegfx::B3DRange aRetval(getB3DRangeFromPrimitive3DSequence(getChildren(), rViewInformation));
            aRetval.transform(getTransformation());
            return aRetval;
        }

        // provide unique ID
        ImplPrimitrive3DIDBlock(TransformPrimitive3D, PRIMITIVE3D_ID_TRANSFORMPRIMITIVE3D)

    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
