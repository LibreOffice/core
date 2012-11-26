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

#include <drawinglayer/primitive2d/texthierarchyprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        TextHierarchyLinePrimitive2D::TextHierarchyLinePrimitive2D(const Primitive2DSequence& rChildren)
        :   GroupPrimitive2D(rChildren)
        {
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(TextHierarchyLinePrimitive2D, PRIMITIVE2D_ID_TEXTHIERARCHYLINEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        TextHierarchyParagraphPrimitive2D::TextHierarchyParagraphPrimitive2D(const Primitive2DSequence& rChildren)
        :   GroupPrimitive2D(rChildren)
        {
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(TextHierarchyParagraphPrimitive2D, PRIMITIVE2D_ID_TEXTHIERARCHYPARAGRAPHPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        TextHierarchyBulletPrimitive2D::TextHierarchyBulletPrimitive2D(const Primitive2DSequence& rChildren)
        :   GroupPrimitive2D(rChildren)
        {
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(TextHierarchyBulletPrimitive2D, PRIMITIVE2D_ID_TEXTHIERARCHYBULLETPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        TextHierarchyBlockPrimitive2D::TextHierarchyBlockPrimitive2D(const Primitive2DSequence& rChildren)
        :   GroupPrimitive2D(rChildren)
        {
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(TextHierarchyBlockPrimitive2D, PRIMITIVE2D_ID_TEXTHIERARCHYBLOCKPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        TextHierarchyFieldPrimitive2D::TextHierarchyFieldPrimitive2D(
            const Primitive2DSequence& rChildren,
            const FieldType& rFieldType,
            const rtl::OUString& rStringA,
            const rtl::OUString& rStringB)
        :   GroupPrimitive2D(rChildren),
            meType(rFieldType),
            maStringA(rStringA),
            maStringB(rStringB)
        {
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(TextHierarchyFieldPrimitive2D, PRIMITIVE2D_ID_TEXTHIERARCHYFIELDPRIMITIVE2D)
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        TextHierarchyEditPrimitive2D::TextHierarchyEditPrimitive2D(const Primitive2DSequence& rChildren)
        :   GroupPrimitive2D(rChildren)
        {
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(TextHierarchyEditPrimitive2D, PRIMITIVE2D_ID_TEXTHIERARCHYEDITPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
