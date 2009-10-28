/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pointarrayprimitive2d.hxx,v $
 *
 *  $Revision: 1.2 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POINTARRAYPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POINTARRAYPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////
// PointArrayPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** PointArrayPrimitive2D class

            This primitive defines single,discrete 'pixels' for the given
            positions in the given color. This makes it view-dependent since
            the logic size of a 'pixel' depends on the view transformation.

            This is one of the non-decomposable primitives, so a renderer
            should proccess it (Currently it is only used for grid visualisation,
            but this may change).
         */
        class PointArrayPrimitive2D : public BasePrimitive2D
        {
        private:
            /// the array of positions
            std::vector< basegfx::B2DPoint >                maPositions;

            /// the color to use
            basegfx::BColor                                 maRGBColor;

            /// #i96669# add simple range buffering for this primitive
            basegfx::B2DRange                               maB2DRange;

        public:
            /// constructor
            PointArrayPrimitive2D(
                const std::vector< basegfx::B2DPoint >& rPositions,
                const basegfx::BColor& rRGBColor);

            /// data read access
            const std::vector< basegfx::B2DPoint >& getPositions() const { return maPositions; }
            const basegfx::BColor& getRGBColor() const { return maRGBColor; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POINTARRAYPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
