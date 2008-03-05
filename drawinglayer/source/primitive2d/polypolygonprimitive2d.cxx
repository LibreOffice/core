/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: polypolygonprimitive2d.cxx,v $
 *
 *  $Revision: 1.7 $
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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYPOLYGONPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#ifndef _BGFX_TOOLS_CANVASTOOLS_HXX
#include <basegfx/tools/canvastools.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_FILLGRADIENTPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_MASKPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_FILLHATCHPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/fillhatchprimitive2d.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_FILLBITMAPPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/fillbitmapprimitive2d.hxx>
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
        Primitive2DSequence PolyPolygonHairlinePrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            const basegfx::B2DPolyPolygon aPolyPolygon(getB2DPolyPolygon());
            const sal_uInt32 nCount(aPolyPolygon.count());

            if(nCount)
            {
                Primitive2DSequence aRetval(nCount);

                for(sal_uInt32 a(0L); a < nCount; a++)
                {
                    aRetval[a] = Primitive2DReference(new PolygonHairlinePrimitive2D(aPolyPolygon.getB2DPolygon(a), getBColor()));
                }

                return aRetval;
            }
            else
            {
                return Primitive2DSequence();
            }
        }

        PolyPolygonHairlinePrimitive2D::PolyPolygonHairlinePrimitive2D(const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::BColor& rBColor)
        :   BasePrimitive2D(),
            maPolyPolygon(rPolyPolygon),
            maBColor(rBColor)
        {
        }

        bool PolyPolygonHairlinePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const PolyPolygonHairlinePrimitive2D& rCompare = (PolyPolygonHairlinePrimitive2D&)rPrimitive;

                return (getB2DPolyPolygon() == rCompare.getB2DPolyPolygon()
                    && getBColor() == rCompare.getBColor());
            }

            return false;
        }

        basegfx::B2DRange PolyPolygonHairlinePrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // return range
            return basegfx::tools::getRange(getB2DPolyPolygon());
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(PolyPolygonHairlinePrimitive2D, PRIMITIVE2D_ID_POLYPOLYGONHAIRLINEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence PolyPolygonStrokePrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            const basegfx::B2DPolyPolygon aPolyPolygon(getB2DPolyPolygon());
            const sal_uInt32 nCount(aPolyPolygon.count());

            if(nCount)
            {
                Primitive2DSequence aRetval(nCount);

                for(sal_uInt32 a(0L); a < nCount; a++)
                {
                    aRetval[a] = Primitive2DReference(new PolygonStrokePrimitive2D(aPolyPolygon.getB2DPolygon(a), getLineAttribute(), getStrokeAttribute()));
                }

                return aRetval;
            }
            else
            {
                return Primitive2DSequence();
            }
        }

        PolyPolygonStrokePrimitive2D::PolyPolygonStrokePrimitive2D(
            const basegfx::B2DPolyPolygon& rPolyPolygon,
              const attribute::LineAttribute& rLineAttribute,
            const attribute::StrokeAttribute& rStrokeAttribute)
        :   BasePrimitive2D(),
            maPolyPolygon(rPolyPolygon),
            maLineAttribute(rLineAttribute),
            maStrokeAttribute(rStrokeAttribute)
        {
        }

        PolyPolygonStrokePrimitive2D::PolyPolygonStrokePrimitive2D(
            const basegfx::B2DPolyPolygon& rPolyPolygon,
              const attribute::LineAttribute& rLineAttribute)
        :   BasePrimitive2D(),
            maPolyPolygon(rPolyPolygon),
            maLineAttribute(rLineAttribute),
            maStrokeAttribute()
        {
        }

        bool PolyPolygonStrokePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const PolyPolygonStrokePrimitive2D& rCompare = (PolyPolygonStrokePrimitive2D&)rPrimitive;

                return (getB2DPolyPolygon() == rCompare.getB2DPolyPolygon()
                    && getLineAttribute() == rCompare.getLineAttribute()
                    && getStrokeAttribute() == rCompare.getStrokeAttribute());
            }

            return false;
        }

        basegfx::B2DRange PolyPolygonStrokePrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // get range of it (subdivided)
            basegfx::B2DRange aRetval(basegfx::tools::getRange(getB2DPolyPolygon()));

            // if width, grow by line width
            if(getLineAttribute().getWidth())
            {
                aRetval.grow(getLineAttribute().getWidth() / 2.0);
            }

            return aRetval;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(PolyPolygonStrokePrimitive2D, PRIMITIVE2D_ID_POLYPOLYGONSTROKEPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence PolyPolygonStrokeArrowPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            const basegfx::B2DPolyPolygon aPolyPolygon(getB2DPolyPolygon());
            const sal_uInt32 nCount(aPolyPolygon.count());

            if(nCount)
            {
                Primitive2DSequence aRetval(nCount);

                for(sal_uInt32 a(0L); a < nCount; a++)
                {
                    const basegfx::B2DPolygon aPolygon(aPolyPolygon.getB2DPolygon(a));

                    if(aPolygon.isClosed())
                    {
                        // no need for PolygonStrokeArrowPrimitive2D when polygon is closed
                        aRetval[a] = Primitive2DReference(new PolygonStrokePrimitive2D(aPolygon, getLineAttribute(), getStrokeAttribute()));
                    }
                    else
                    {
                        aRetval[a] = Primitive2DReference(new PolygonStrokeArrowPrimitive2D(aPolygon, getLineAttribute(), getStrokeAttribute(), getStart(), getEnd()));
                    }
                }

                return aRetval;
            }
            else
            {
                return Primitive2DSequence();
            }
        }

        PolyPolygonStrokeArrowPrimitive2D::PolyPolygonStrokeArrowPrimitive2D(
            const basegfx::B2DPolyPolygon& rPolyPolygon,
               const attribute::LineAttribute& rLineAttribute,
            const attribute::StrokeAttribute& rStrokeAttribute,
            const attribute::LineStartEndAttribute& rStart,
            const attribute::LineStartEndAttribute& rEnd)
        :   PolyPolygonStrokePrimitive2D(rPolyPolygon, rLineAttribute, rStrokeAttribute),
            maStart(rStart),
            maEnd(rEnd)
        {
        }

        PolyPolygonStrokeArrowPrimitive2D::PolyPolygonStrokeArrowPrimitive2D(
            const basegfx::B2DPolyPolygon& rPolyPolygon,
               const attribute::LineAttribute& rLineAttribute,
            const attribute::LineStartEndAttribute& rStart,
            const attribute::LineStartEndAttribute& rEnd)
        :   PolyPolygonStrokePrimitive2D(rPolyPolygon, rLineAttribute),
            maStart(rStart),
            maEnd(rEnd)
        {
        }

        bool PolyPolygonStrokeArrowPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(PolyPolygonStrokePrimitive2D::operator==(rPrimitive))
            {
                const PolyPolygonStrokeArrowPrimitive2D& rCompare = (PolyPolygonStrokeArrowPrimitive2D&)rPrimitive;

                return (getStart() == rCompare.getStart()
                    && getEnd() == rCompare.getEnd());
            }

            return false;
        }

        basegfx::B2DRange PolyPolygonStrokeArrowPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            basegfx::B2DRange aRetval;

            if(getStart().isActive() || getEnd().isActive())
            {
                // use decomposition when line start/end is used
                return BasePrimitive2D::getB2DRange(rViewInformation);
            }
            else
            {
                // get range from parent
                return PolyPolygonStrokePrimitive2D::getB2DRange(rViewInformation);
            }
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(PolyPolygonStrokeArrowPrimitive2D, PRIMITIVE2D_ID_POLYPOLYGONSTROKEARROWPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        PolyPolygonColorPrimitive2D::PolyPolygonColorPrimitive2D(
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            const basegfx::BColor& rBColor)
        :   BasePrimitive2D(),
            maPolyPolygon(rPolyPolygon),
            maBColor(rBColor)
        {
        }

        bool PolyPolygonColorPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BasePrimitive2D::operator==(rPrimitive))
            {
                const PolyPolygonColorPrimitive2D& rCompare = (PolyPolygonColorPrimitive2D&)rPrimitive;

                return (getB2DPolyPolygon() == rCompare.getB2DPolyPolygon()
                    && getBColor() == rCompare.getBColor());
            }

            return false;
        }

        basegfx::B2DRange PolyPolygonColorPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // return range
            return basegfx::tools::getRange(getB2DPolyPolygon());
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(PolyPolygonColorPrimitive2D, PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence PolyPolygonGradientPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            // create SubSequence with FillGradientPrimitive2D
            FillGradientPrimitive2D* pNewGradient = new FillGradientPrimitive2D(getB2DRange(rViewInformation), getFillGradient());
            const Primitive2DReference xSubRef(pNewGradient);
            const Primitive2DSequence aSubSequence(&xSubRef, 1L);

            // create mask primitive
            MaskPrimitive2D* pNewMask = new MaskPrimitive2D(getB2DPolyPolygon(), aSubSequence);
            const Primitive2DReference xRef(pNewMask);
            return Primitive2DSequence(&xRef, 1L);
        }

        PolyPolygonGradientPrimitive2D::PolyPolygonGradientPrimitive2D(
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            const basegfx::BColor& rBColor,
            const attribute::FillGradientAttribute& rFillGradient)
        :   PolyPolygonColorPrimitive2D(rPolyPolygon, rBColor),
            maFillGradient(rFillGradient)
        {
        }

        bool PolyPolygonGradientPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(PolyPolygonColorPrimitive2D::operator==(rPrimitive))
            {
                const PolyPolygonGradientPrimitive2D& rCompare = (PolyPolygonGradientPrimitive2D&)rPrimitive;

                return (getFillGradient() == rCompare.getFillGradient());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(PolyPolygonGradientPrimitive2D, PRIMITIVE2D_ID_POLYPOLYGONGRADIENTPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence PolyPolygonHatchPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            // create SubSequence with FillHatchPrimitive2D
            FillHatchPrimitive2D* pNewHatch = new FillHatchPrimitive2D(getB2DRange(rViewInformation), getBColor(), getFillHatch());
            const Primitive2DReference xSubRef(pNewHatch);
            const Primitive2DSequence aSubSequence(&xSubRef, 1L);

            // create mask primitive
            MaskPrimitive2D* pNewMask = new MaskPrimitive2D(getB2DPolyPolygon(), aSubSequence);
            const Primitive2DReference xRef(pNewMask);
            return Primitive2DSequence(&xRef, 1L);
        }

        PolyPolygonHatchPrimitive2D::PolyPolygonHatchPrimitive2D(
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            const basegfx::BColor& rBColor,
            const attribute::FillHatchAttribute& rFillHatch)
        :   PolyPolygonColorPrimitive2D(rPolyPolygon, rBColor),
            maFillHatch(rFillHatch)
        {
        }

        bool PolyPolygonHatchPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(PolyPolygonColorPrimitive2D::operator==(rPrimitive))
            {
                const PolyPolygonHatchPrimitive2D& rCompare = (PolyPolygonHatchPrimitive2D&)rPrimitive;

                return (getFillHatch() == rCompare.getFillHatch());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(PolyPolygonHatchPrimitive2D, PRIMITIVE2D_ID_POLYPOLYGONHATCHPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence PolyPolygonBitmapPrimitive2D::createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            // create SubSequence with FillBitmapPrimitive2D
            const basegfx::B2DRange aOwnRange(getB2DRange(rViewInformation));
            basegfx::B2DHomMatrix aNewObjectTransform;
            aNewObjectTransform.set(0, 0, aOwnRange.getWidth());
            aNewObjectTransform.set(1, 1, aOwnRange.getHeight());
            aNewObjectTransform.set(0, 2, aOwnRange.getMinX());
            aNewObjectTransform.set(1, 2, aOwnRange.getMinY());
            FillBitmapPrimitive2D* pNewBitmap = new FillBitmapPrimitive2D(aNewObjectTransform, getFillBitmap());
            const Primitive2DReference xSubRef(pNewBitmap);
            const Primitive2DSequence aSubSequence(&xSubRef, 1L);

            // create mask primitive
            MaskPrimitive2D* pNewMask = new MaskPrimitive2D(getB2DPolyPolygon(), aSubSequence);
            const Primitive2DReference xRef(pNewMask);
            return Primitive2DSequence(&xRef, 1L);
        }

        PolyPolygonBitmapPrimitive2D::PolyPolygonBitmapPrimitive2D(
            const basegfx::B2DPolyPolygon& rPolyPolygon,
            const basegfx::BColor& rBColor,
            const attribute::FillBitmapAttribute& rFillBitmap)
        :   PolyPolygonColorPrimitive2D(rPolyPolygon, rBColor),
            maFillBitmap(rFillBitmap)
        {
        }

        bool PolyPolygonBitmapPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(PolyPolygonColorPrimitive2D::operator==(rPrimitive))
            {
                const PolyPolygonBitmapPrimitive2D& rCompare = (PolyPolygonBitmapPrimitive2D&)rPrimitive;

                return (getFillBitmap() == rCompare.getFillBitmap());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(PolyPolygonBitmapPrimitive2D, PRIMITIVE2D_ID_POLYPOLYGONBITMAPPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
