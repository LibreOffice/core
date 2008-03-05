/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fillhatchprimitive2d.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: aw $ $Date: 2008-03-05 09:15:42 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_FILLHATCHPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/fillhatchprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_TEXTURE_TEXTURE_HXX
#include <drawinglayer/texture/texture.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYPOLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolygontools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
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
        Primitive2DSequence FillHatchPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // create hatch
            const basegfx::BColor aHatchColor(maFillHatch.getColor());
            const double fAngle(-maFillHatch.getAngle());
            ::std::vector< basegfx::B2DHomMatrix > aMatrices;

            // get hatch transformations
            switch(maFillHatch.getStyle())
            {
                case attribute::HATCHSTYLE_TRIPLE:
                {
                    // rotated 45 degrees
                    texture::GeoTexSvxHatch aHatch(getObjectRange(), maFillHatch.getDistance(), fAngle + F_PI4);
                    aHatch.appendTransformations(aMatrices);

                    // fall-through by purpose
                }
                case attribute::HATCHSTYLE_DOUBLE:
                {
                    // rotated 90 degrees
                    texture::GeoTexSvxHatch aHatch(getObjectRange(), maFillHatch.getDistance(), fAngle + F_PI2);
                    aHatch.appendTransformations(aMatrices);

                    // fall-through by purpose
                }
                case attribute::HATCHSTYLE_SINGLE:
                {
                    // angle as given
                    texture::GeoTexSvxHatch aHatch(getObjectRange(), maFillHatch.getDistance(), fAngle);
                    aHatch.appendTransformations(aMatrices);
                }
            }

            // prepare return value
            const bool bFillBackground(maFillHatch.isFillBackground());
            Primitive2DSequence aRetval(bFillBackground ? aMatrices.size() + 1L : aMatrices.size());

            // evtl. create filled background
            if(bFillBackground)
            {
                // create primitive for background
                const Primitive2DReference xRef(new PolyPolygonColorPrimitive2D(basegfx::B2DPolyPolygon(basegfx::tools::createPolygonFromRect(getObjectRange())), maBColor));
                aRetval[0L] = xRef;
            }

            // create primitives
            const basegfx::B2DPoint aStart(0.0, 0.0);
            const basegfx::B2DPoint aEnd(1.0, 0.0);

            for(sal_uInt32 a(0L); a < aMatrices.size(); a++)
            {
                const basegfx::B2DHomMatrix& rMatrix = aMatrices[a];
                basegfx::B2DPolygon aNewLine;

                aNewLine.append(rMatrix * aStart);
                aNewLine.append(rMatrix * aEnd);

                // create hairline
                const Primitive2DReference xRef(new PolygonHairlinePrimitive2D(aNewLine, aHatchColor));
                aRetval[bFillBackground ? (a + 1L) : a] = xRef;
            }

            return aRetval;
        }

        FillHatchPrimitive2D::FillHatchPrimitive2D(
            const basegfx::B2DRange& rObjectRange,
            const basegfx::BColor& rBColor,
            const attribute::FillHatchAttribute& rFillHatch)
        :   BasePrimitive2D(),
            maObjectRange(rObjectRange),
            maFillHatch(rFillHatch),
            maBColor(rBColor)
        {
        }

        bool FillHatchPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const FillHatchPrimitive2D& rCompare = (FillHatchPrimitive2D&)rPrimitive;

                return (getObjectRange() == rCompare.getObjectRange()
                    && maFillHatch == rCompare.maFillHatch
                    && maBColor == rCompare.maBColor);
            }

            return false;
        }

        basegfx::B2DRange FillHatchPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // return ObjectRange
            return getObjectRange();
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(FillHatchPrimitive2D, PRIMITIVE2D_ID_FILLHATCHPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
