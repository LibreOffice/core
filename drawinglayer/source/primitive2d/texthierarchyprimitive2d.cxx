/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
            const rtl::OUString& rString)
        :   GroupPrimitive2D(rChildren),
            meType(rFieldType),
            maString(rString)
        {
        }

        bool TextHierarchyFieldPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(GroupPrimitive2D::operator==(rPrimitive))
            {
                const TextHierarchyFieldPrimitive2D& rCompare = (TextHierarchyFieldPrimitive2D&)rPrimitive;

                return (getType() == rCompare.getType()
                    &&  getString() == rCompare.getString());
            }

            return false;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
