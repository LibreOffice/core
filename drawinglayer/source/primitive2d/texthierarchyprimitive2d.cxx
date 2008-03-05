/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: texthierarchyprimitive2d.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_TEXTHIERARCHYPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/texthierarchyprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_PRIMITIVETYPES2D_HXX
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#endif

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
// eof
