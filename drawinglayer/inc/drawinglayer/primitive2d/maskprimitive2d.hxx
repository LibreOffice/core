/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: maskprimitive2d.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: aw $ $Date: 2008-05-27 14:11:17 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MASKPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MASKPRIMITIVE2D_HXX

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
        class MaskPrimitive2D : public GroupPrimitive2D
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

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MASKPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
