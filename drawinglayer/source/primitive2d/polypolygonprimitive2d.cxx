/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: polypolygonprimitive2d.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: aw $ $Date: 2006-10-19 10:35:04 $
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

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

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
            return basegfx::tools::getRange(basegfx::tools::adaptiveSubdivideByAngle(getB2DPolyPolygon()));
        }

        sal_uInt32 PolyPolygonColorPrimitive2D::getPrimitiveID() const
        {
            return Create2DPrimitiveID('2','P','P','C');
        }
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

        sal_uInt32 PolyPolygonGradientPrimitive2D::getPrimitiveID() const
        {
            return Create2DPrimitiveID('2','P','P','G');
        }
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

        sal_uInt32 PolyPolygonHatchPrimitive2D::getPrimitiveID() const
        {
            return Create2DPrimitiveID('2','P','P','H');
        }
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

        sal_uInt32 PolyPolygonBitmapPrimitive2D::getPrimitiveID() const
        {
            return Create2DPrimitiveID('2','P','P','B');
        }
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
