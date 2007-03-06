/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: markerarrayprimitive2d.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: aw $ $Date: 2007-03-06 12:30:47 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MARKERARRAYPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MARKERARRAYPRIMITIVE2D_HXX

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BASEPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#endif

#ifndef _BGFX_COLOR_BCOLOR_HXX
#include <basegfx/color/bcolor.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// MarkerPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        enum MarkerStyle2D
        {
            /// Point: Uses RGBColor, 1x1 pixel
            MARKERSTYLE2D_POINT,

            /// Cross: Uses RGBColor, 3x3 pixel, centered, form of a plus sign
            MARKERSTYLE2D_CROSS,

            /// Gluepoint: Uses RGBColor as outline and hardcoded COL_LIGHTBLUE as inner
            /// line pen, 7x7 pixel, centered, looks like a x with thee pixel lines
            MARKERSTYLE2D_GLUEPOINT
        };

        class MarkerArrayPrimitive2D : public BasePrimitive2D
        {
        private:
            std::vector< basegfx::B2DPoint >                maPositions;
            basegfx::BColor                                 maRGBColor;
            MarkerStyle2D                                   meStyle;

        public:
            MarkerArrayPrimitive2D(
                const std::vector< basegfx::B2DPoint >& rPositions,
                MarkerStyle2D eStyle,
                const basegfx::BColor& rRGBColor);

            // get data
            const std::vector< basegfx::B2DPoint >& getPositions() const { return maPositions; }
            const basegfx::BColor& getRGBColor() const { return maRGBColor; }
            MarkerStyle2D getStyle() const { return meStyle; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MARKERARRAYPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
