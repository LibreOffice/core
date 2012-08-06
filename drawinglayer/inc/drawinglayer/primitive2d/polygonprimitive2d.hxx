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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYGONPRIMITIVE2D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYGONPRIMITIVE2D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <drawinglayer/attribute/linestartendattribute.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/color/bcolor.hxx>

//////////////////////////////////////////////////////////////////////////////
// PolygonHairlinePrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** PolygonHairlinePrimitive2D class

            This primitive defines a Hairline. Since hairlines are view-dependent,
            this primitive is view-dependent, too.

            This is one of the non-decomposable primitives, so a renderer
            should proccess it.
         */
        class DRAWINGLAYER_DLLPUBLIC PolygonHairlinePrimitive2D : public BasePrimitive2D
        {
        private:
            /// the hairline geometry
            basegfx::B2DPolygon                     maPolygon;

            /// the hairline color
            basegfx::BColor                         maBColor;

        public:
            /// constructor
            PolygonHairlinePrimitive2D(
                const basegfx::B2DPolygon& rPolygon,
                const basegfx::BColor& rBColor);

            /// data read access
            const basegfx::B2DPolygon& getB2DPolygon() const { return maPolygon; }
            const basegfx::BColor& getBColor() const { return maBColor; }

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
// PolygonMarkerPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** PolygonMarkerPrimitive2D class

            This primitive defines a two-colored marker hairline which is
            dashed with the given dash length. Since hairlines are view-dependent,
            this primitive is view-dependent, too.

            It will be decomposed to the needed PolygonHairlinePrimitive2D if
            not handled directly by a renderer.
         */
        class DRAWINGLAYER_DLLPUBLIC PolygonMarkerPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the marker hairline geometry
            basegfx::B2DPolygon                     maPolygon;

            /// the two colors
            basegfx::BColor                         maRGBColorA;
            basegfx::BColor                         maRGBColorB;

            /// the dash distance in 'pixels'
            double                                  mfDiscreteDashLength;

            /// decomposition is view-dependent, remember last InverseObjectToViewTransformation
            basegfx::B2DHomMatrix                   maLastInverseObjectToViewTransformation;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            PolygonMarkerPrimitive2D(
                const basegfx::B2DPolygon& rPolygon,
                const basegfx::BColor& rRGBColorA,
                const basegfx::BColor& rRGBColorB,
                double fDiscreteDashLength);

            /// data read access
            const basegfx::B2DPolygon& getB2DPolygon() const { return maPolygon; }
            const basegfx::BColor& getRGBColorA() const { return maRGBColorA; }
            const basegfx::BColor& getRGBColorB() const { return maRGBColorB; }
            double getDiscreteDashLength() const { return mfDiscreteDashLength; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const;

            /// get local decomposition. Overloaded since this decomposition is view-dependent
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// PolygonStrokePrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** PolygonStrokePrimitive2D class

            This primitive defines a line with line width, line join, line color
            and stroke attributes. It will be decomposed dependent on the definition
            to the needed primitives, e.g. filled PolyPolygons for fat lines.
         */
        class DRAWINGLAYER_DLLPUBLIC PolygonStrokePrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the line geometry
            basegfx::B2DPolygon                     maPolygon;

            /// the line attributes like width, join and color
            attribute::LineAttribute                maLineAttribute;

            /// the line stroking (if used)
            attribute::StrokeAttribute              maStrokeAttribute;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            PolygonStrokePrimitive2D(
                const basegfx::B2DPolygon& rPolygon,
                const attribute::LineAttribute& rLineAttribute,
                const attribute::StrokeAttribute& rStrokeAttribute);

            /// constructor without stroking
            PolygonStrokePrimitive2D(
                const basegfx::B2DPolygon& rPolygon,
                const attribute::LineAttribute& rLineAttribute);

            /// data read access
            const basegfx::B2DPolygon& getB2DPolygon() const { return maPolygon; }
            const attribute::LineAttribute& getLineAttribute() const { return maLineAttribute; }
            const attribute::StrokeAttribute& getStrokeAttribute() const { return maStrokeAttribute; }

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
// PolygonWavePrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** PolygonWavePrimitive2D class

            This primitive defines a waveline based on a PolygonStrokePrimitive2D
            where the wave is defined by wave width and wave length.
         */
        class DRAWINGLAYER_DLLPUBLIC PolygonWavePrimitive2D : public PolygonStrokePrimitive2D
        {
        private:
            /// wave definition
            double                                  mfWaveWidth;
            double                                  mfWaveHeight;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            PolygonWavePrimitive2D(
                const basegfx::B2DPolygon& rPolygon,
                const attribute::LineAttribute& rLineAttribute,
                const attribute::StrokeAttribute& rStrokeAttribute,
                double fWaveWidth,
                double fWaveHeight);

            /// constructor without stroking
            PolygonWavePrimitive2D(
                const basegfx::B2DPolygon& rPolygon,
                const attribute::LineAttribute& rLineAttribute,
                double fWaveWidth,
                double fWaveHeight);

            /// data read access
            double getWaveWidth() const { return mfWaveWidth; }
            double getWaveHeight() const { return mfWaveHeight; }

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
// PolygonStrokeArrowPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** PolygonStrokeArrowPrimitive2D class

            This primitive defines a PolygonStrokePrimitive2D which is extended
            eventually by start and end definitions which are normally used for
            arrows.
         */
        class DRAWINGLAYER_DLLPUBLIC PolygonStrokeArrowPrimitive2D : public PolygonStrokePrimitive2D
        {
        private:
            /// geometric definitions for line start and end
            attribute::LineStartEndAttribute                maStart;
            attribute::LineStartEndAttribute                maEnd;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const;

        public:
            /// constructor
            PolygonStrokeArrowPrimitive2D(
                const basegfx::B2DPolygon& rPolygon,
                const attribute::LineAttribute& rLineAttribute,
                const attribute::StrokeAttribute& rStrokeAttribute,
                const attribute::LineStartEndAttribute& rStart,
                const attribute::LineStartEndAttribute& rEnd);

            /// data read access
            const attribute::LineStartEndAttribute& getStart() const { return maStart; }
            const attribute::LineStartEndAttribute& getEnd() const { return maEnd; }

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

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYGONPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
