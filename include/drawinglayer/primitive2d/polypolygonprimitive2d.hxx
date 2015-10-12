/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYPOLYGONPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYPOLYGONPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/attribute/fillgraphicattribute.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <drawinglayer/attribute/linestartendattribute.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>
#include <drawinglayer/attribute/fillhatchattribute.hxx>
#include <drawinglayer/primitive2d/primitivetools2d.hxx>
#include <basegfx/color/bcolor.hxx>


// PolyPolygonHairlinePrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** PolyPolygonHairlinePrimitive2D class

            This primitive defines a multi-PolygonHairlinePrimitive2D and is
            just for convenience. The definition is not different from the single
                defined PolygonHairlinePrimitive2Ds.
         */
        class DRAWINGLAYER_DLLPUBLIC PolyPolygonHairlinePrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the hairline geometry
            basegfx::B2DPolyPolygon                 maPolyPolygon;

            /// the hairline color
            basegfx::BColor                         maBColor;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructor
            PolyPolygonHairlinePrimitive2D(const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::BColor& rBColor);

            /// data read access
            const basegfx::B2DPolyPolygon& getB2DPolyPolygon() const { return maPolyPolygon; }
            const basegfx::BColor& getBColor() const { return maBColor; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


// PolyPolygonMarkerPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** PolyPolygonMarkerPrimitive2D class

            This primitive defines a multi-PolygonMarkerPrimitive2D and is
            just for convenience. The definition is not different from the single
            defined PolygonMarkerPrimitive2Ds.
         */
        class DRAWINGLAYER_DLLPUBLIC PolyPolygonMarkerPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the marker hairline geometry
            basegfx::B2DPolyPolygon                 maPolyPolygon;

            /// the two colors
            basegfx::BColor                         maRGBColorA;
            basegfx::BColor                         maRGBColorB;

            /// the dash distance in 'pixels'
            double                                  mfDiscreteDashLength;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructor
            PolyPolygonMarkerPrimitive2D(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const basegfx::BColor& rRGBColorA,
                const basegfx::BColor& rRGBColorB,
                double fDiscreteDashLength);

            // data read access
            const basegfx::B2DPolyPolygon& getB2DPolyPolygon() const { return maPolyPolygon; }
            const basegfx::BColor& getRGBColorA() const { return maRGBColorA; }
            const basegfx::BColor& getRGBColorB() const { return maRGBColorB; }
            double getDiscreteDashLength() const { return mfDiscreteDashLength; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


// PolyPolygonStrokePrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** PolyPolygonStrokePrimitive2D class

            This primitive defines a multi-PolygonStrokePrimitive2D and is
            just for convenience. The definition is not different from the single
            defined PolygonStrokePrimitive2Ds.
         */
        class DRAWINGLAYER_DLLPUBLIC PolyPolygonStrokePrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the line geometry
            basegfx::B2DPolyPolygon                 maPolyPolygon;

            /// the line attributes like width, join and color
            attribute::LineAttribute                maLineAttribute;

            /// the line stroking (if used)
            attribute::StrokeAttribute              maStrokeAttribute;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructor
            PolyPolygonStrokePrimitive2D(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const attribute::LineAttribute& rLineAttribute,
                const attribute::StrokeAttribute& rStrokeAttribute);

            PolyPolygonStrokePrimitive2D(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const attribute::LineAttribute& rLineAttribute);

            /// data read access
            const basegfx::B2DPolyPolygon& getB2DPolyPolygon() const { return maPolyPolygon; }
            const attribute::LineAttribute& getLineAttribute() const { return maLineAttribute; }
            const attribute::StrokeAttribute& getStrokeAttribute() const { return maStrokeAttribute; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


// PolyPolygonColorPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** PolyPolygonColorPrimitive2D class

            This primitive defines a tools::PolyPolygon filled with a single color.
            This is one of the non-decomposable primitives, so a renderer
            should process it.
         */
        class DRAWINGLAYER_DLLPUBLIC PolyPolygonColorPrimitive2D : public BasePrimitive2D
        {
        private:
            /// the tools::PolyPolygon geometry
            basegfx::B2DPolyPolygon                 maPolyPolygon;

            /// the polygon fill color
            basegfx::BColor                         maBColor;

        public:
            /// constructor
            PolyPolygonColorPrimitive2D(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const basegfx::BColor& rBColor);

            /// data read access
            const basegfx::B2DPolyPolygon& getB2DPolyPolygon() const { return maPolyPolygon; }
            const basegfx::BColor& getBColor() const { return maBColor; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


// PolyPolygonGradientPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** PolyPolygonColorPrimitive2D class

            This primitive defines a tools::PolyPolygon filled with a gradient. The
            decomosition will create a MaskPrimitive2D containing a
            FillGradientPrimitive2D.
         */
        class DRAWINGLAYER_DLLPUBLIC PolyPolygonGradientPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the tools::PolyPolygon geometry
            basegfx::B2DPolyPolygon                     maPolyPolygon;

            /// the definition range
            basegfx::B2DRange                           maDefinitionRange;

            /// the gradient definition
            attribute::FillGradientAttribute            maFillGradient;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructors. The one without definition range will use output range as definition range
            PolyPolygonGradientPrimitive2D(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const attribute::FillGradientAttribute& rFillGradient);
            PolyPolygonGradientPrimitive2D(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const basegfx::B2DRange& rDefinitionRange,
                const attribute::FillGradientAttribute& rFillGradient);

            /// data read access
            const basegfx::B2DPolyPolygon& getB2DPolyPolygon() const { return maPolyPolygon; }
            const basegfx::B2DRange& getDefinitionRange() const { return maDefinitionRange; }
            const attribute::FillGradientAttribute& getFillGradient() const { return maFillGradient; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


// PolyPolygonHatchPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** PolyPolygonHatchPrimitive2D class

            This primitive defines a tools::PolyPolygon filled with a hatch. The
            decomosition will create a MaskPrimitive2D containing a
            FillHatchPrimitive2D.
         */
        class DRAWINGLAYER_DLLPUBLIC PolyPolygonHatchPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the tools::PolyPolygon geometry
            basegfx::B2DPolyPolygon                     maPolyPolygon;

            /// the definition range
            basegfx::B2DRange                           maDefinitionRange;

            /// the hatch background color (if used)
            basegfx::BColor                             maBackgroundColor;

            /// the hatch definition
            attribute::FillHatchAttribute               maFillHatch;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructors. The one without definition range will use output range as definition range
            PolyPolygonHatchPrimitive2D(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const basegfx::BColor& rBackgroundColor,
                const attribute::FillHatchAttribute& rFillHatch);
            PolyPolygonHatchPrimitive2D(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const basegfx::B2DRange& rDefinitionRange,
                const basegfx::BColor& rBackgroundColor,
                const attribute::FillHatchAttribute& rFillHatch);

            /// data read access
            const basegfx::B2DPolyPolygon& getB2DPolyPolygon() const { return maPolyPolygon; }
            const basegfx::B2DRange& getDefinitionRange() const { return maDefinitionRange; }
            const basegfx::BColor& getBackgroundColor() const { return maBackgroundColor; }
            const attribute::FillHatchAttribute& getFillHatch() const { return maFillHatch; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


// PolyPolygonGraphicPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** PolyPolygonGraphicPrimitive2D class

            This primitive defines a tools::PolyPolygon filled with bitmap data
            (including transparence). The decomosition will create a MaskPrimitive2D
            containing a FillGraphicPrimitive2D.
         */
        class DRAWINGLAYER_DLLPUBLIC PolyPolygonGraphicPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the tools::PolyPolygon geometry
            basegfx::B2DPolyPolygon                     maPolyPolygon;

            /// the definition range
            basegfx::B2DRange                           maDefinitionRange;

            /// the bitmap fill definition (may include tiling)
            attribute::FillGraphicAttribute             maFillGraphic;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructors. The one without definition range will use output range as definition range
            PolyPolygonGraphicPrimitive2D(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const attribute::FillGraphicAttribute& rFillGraphic);
            PolyPolygonGraphicPrimitive2D(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const basegfx::B2DRange& rDefinitionRange,
                const attribute::FillGraphicAttribute& rFillGraphic);

            /// data read access
            const basegfx::B2DPolyPolygon& getB2DPolyPolygon() const { return maPolyPolygon; }
            const basegfx::B2DRange& getDefinitionRange() const { return maDefinitionRange; }
            const attribute::FillGraphicAttribute& getFillGraphic() const { return maFillGraphic; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


// PolyPolygonSelectionPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** PolyPolygonSelectionPrimitive2D class

            This primitive defines a tools::PolyPolygon which gets filled with a defined color
            and a defined transparence, but also gets extended ('grown') by the given
            discrete size (thus being a view-dependent primitive)
         */
        class DRAWINGLAYER_DLLPUBLIC PolyPolygonSelectionPrimitive2D : public DiscreteMetricDependentPrimitive2D
        {
        private:
            /// the tools::PolyPolygon geometry
            basegfx::B2DPolyPolygon                 maPolyPolygon;

            /// the color
            basegfx::BColor                         maColor;

            /// the transparence [0.0 .. 1.0]
            double                                  mfTransparence;

            /// the discrete grow size ('pixels'), only positive values allowed
            double                                  mfDiscreteGrow;

            /// bitfield
            /// draw polygons filled when fill is set
            bool                                    mbFill : 1;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructor
            PolyPolygonSelectionPrimitive2D(
                const basegfx::B2DPolyPolygon& rPolyPolygon,
                const basegfx::BColor& rColor,
                double fTransparence,
                double fDiscreteGrow,
                bool bFill);

            /// data read access
            const basegfx::B2DPolyPolygon& getB2DPolyPolygon() const { return maPolyPolygon; }
            const basegfx::BColor& getColor() const { return maColor; }
            double getTransparence() const { return mfTransparence; }
            double getDiscreteGrow() const { return mfDiscreteGrow; }
            bool getFill() const { return mbFill; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYPOLYGONPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
