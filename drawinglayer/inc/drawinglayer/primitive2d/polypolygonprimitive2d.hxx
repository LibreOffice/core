/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: polypolygonprimitive2d.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: aw $ $Date: 2007-11-07 14:27:16 $
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
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYPOLYGONPRIMITIVE2D_HXX

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_BASEPRIMITIVE2D_HXX
#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLATTRIBUTE_HXX
#include <drawinglayer/attribute/fillattribute.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_FILLBITMAPATTRIBUTE_HXX
#include <drawinglayer/attribute/fillbitmapattribute.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_LINEATTRIBUTE_HXX
#include <drawinglayer/attribute/lineattribute.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_STROKEATTRIBUTE_HXX
#include <drawinglayer/attribute/strokeattribute.hxx>
#endif

#ifndef INCLUDED_DRAWINGLAYER_ATTRIBUTE_LINESTARTENDATTRIBUTE_HXX
#include <drawinglayer/attribute/linestartendattribute.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// PolyPolygonStrokePrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        class PolyPolygonHairlinePrimitive2D : public BasePrimitive2D
        {
        private:
            basegfx::B2DPolyPolygon                 maPolyPolygon;
            basegfx::BColor                         maBColor;

        protected:
            // local decomposition.
            virtual Primitive2DSequence createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            PolyPolygonHairlinePrimitive2D(const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::BColor& rBColor);

            // get data
            basegfx::B2DPolyPolygon getB2DPolyPolygon() const { return maPolyPolygon; }
            const basegfx::BColor& getBColor() const { return maBColor; }

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
// PolyPolygonStrokePrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        class PolyPolygonStrokePrimitive2D : public BasePrimitive2D
        {
        private:
            basegfx::B2DPolyPolygon                 maPolyPolygon;
            attribute::LineAttribute                maLineAttribute;
            attribute::StrokeAttribute              maStrokeAttribute;

        protected:
            // local decomposition.
            virtual Primitive2DSequence createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            PolyPolygonStrokePrimitive2D(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const attribute::LineAttribute& rLineAttribute,
                const attribute::StrokeAttribute& rStrokeAttribute);

            PolyPolygonStrokePrimitive2D(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const attribute::LineAttribute& rLineAttribute);

            // get data
            basegfx::B2DPolyPolygon getB2DPolyPolygon() const { return maPolyPolygon; }
            const attribute::LineAttribute& getLineAttribute() const { return maLineAttribute; }
            const attribute::StrokeAttribute& getStrokeAttribute() const { return maStrokeAttribute; }

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
// PolyPolygonStrokeArrowPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        class PolyPolygonStrokeArrowPrimitive2D : public PolyPolygonStrokePrimitive2D
        {
        private:
            attribute::LineStartEndAttribute                maStart;
            attribute::LineStartEndAttribute                maEnd;

        protected:
            // local decomposition.
            virtual Primitive2DSequence createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            PolyPolygonStrokeArrowPrimitive2D(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const attribute::LineAttribute& rLineAttribute,
                const attribute::StrokeAttribute& rStrokeAttribute,
                const attribute::LineStartEndAttribute& rStart,
                const attribute::LineStartEndAttribute& rEnd);

            PolyPolygonStrokeArrowPrimitive2D(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const attribute::LineAttribute& rLineAttribute,
                const attribute::LineStartEndAttribute& rStart,
                const attribute::LineStartEndAttribute& rEnd);

            // get data
            const attribute::LineStartEndAttribute& getStart() const { return maStart; }
            const attribute::LineStartEndAttribute& getEnd() const { return maEnd; }

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
// PolyPolygonColorPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        class PolyPolygonColorPrimitive2D : public BasePrimitive2D
        {
        private:
            basegfx::B2DPolyPolygon                 maPolyPolygon;
            basegfx::BColor                         maBColor;

        public:
            PolyPolygonColorPrimitive2D(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const basegfx::BColor& rBColor);

            // get data
            const basegfx::B2DPolyPolygon& getB2DPolyPolygon() const { return maPolyPolygon; }
            const basegfx::BColor& getBColor() const { return maBColor; }

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
// PolyPolygonGradientPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        class PolyPolygonGradientPrimitive2D : public PolyPolygonColorPrimitive2D
        {
        private:
            attribute::FillGradientAttribute            maFillGradient;

        protected:
            // local decomposition.
            virtual Primitive2DSequence createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            PolyPolygonGradientPrimitive2D(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const basegfx::BColor& rBColor,
                const attribute::FillGradientAttribute& rFillGradient);

            // get data
            const attribute::FillGradientAttribute& getFillGradient() const { return maFillGradient; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// PolyPolygonHatchPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        class PolyPolygonHatchPrimitive2D : public PolyPolygonColorPrimitive2D
        {
        private:
            attribute::FillHatchAttribute               maFillHatch;

        protected:
            // local decomposition.
            virtual Primitive2DSequence createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            PolyPolygonHatchPrimitive2D(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const basegfx::BColor& rBColor,
                const attribute::FillHatchAttribute& rFillHatch);

            // get data
            const attribute::FillHatchAttribute& getFillHatch() const { return maFillHatch; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// PolyPolygonBitmapPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        class PolyPolygonBitmapPrimitive2D : public PolyPolygonColorPrimitive2D
        {
        private:
            attribute::FillBitmapAttribute              maFillBitmap;

        protected:
            // local decomposition.
            virtual Primitive2DSequence createLocalDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            PolyPolygonBitmapPrimitive2D(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const basegfx::BColor& rBColor,
                const attribute::FillBitmapAttribute& rFillBitmap);

            // get data
            const attribute::FillBitmapAttribute& getFillBitmap() const { return maFillBitmap; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYPOLYGONPRIMITIVE2D_HXX

//////////////////////////////////////////////////////////////////////////////
// eof
