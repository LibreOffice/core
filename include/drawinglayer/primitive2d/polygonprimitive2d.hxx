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


// PolygonHairlinePrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** PolygonHairlinePrimitive2D class

            This primitive defines a Hairline. Since hairlines are view-dependent,
            this primitive is view-dependent, too.

            This is one of the non-decomposable primitives, so a renderer
            should process it.
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
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


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
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

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
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// Override standard getDecomposition to be view-dependent here
            virtual Primitive2DSequence get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


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
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

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
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


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
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

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
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer


// PolygonStrokeArrowPrimitive2D class

namespace drawinglayer
{
    namespace primitive2d
    {
        /** PolygonStrokeArrowPrimitive2D class

            This primitive defines a PolygonStrokePrimitive2D,
            possibly extended by start and end definitions, which are
            normally used for arrows.
         */
        class DRAWINGLAYER_DLLPUBLIC PolygonStrokeArrowPrimitive2D : public PolygonStrokePrimitive2D
        {
        private:
            /// geometric definitions for line start and end
            attribute::LineStartEndAttribute                maStart;
            attribute::LineStartEndAttribute                maEnd;

        protected:
            /// local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// constructor
            PolygonStrokeArrowPrimitive2D(
                const basegfx::B2DPolygon& rPolygon,
                const attribute::LineAttribute& rLineAttribute,
                const attribute::StrokeAttribute& rStrokeAttribute,
                const attribute::LineStartEndAttribute& rStart,
                const attribute::LineStartEndAttribute& rEnd);

            /// constructor without stroking
            PolygonStrokeArrowPrimitive2D(
                const basegfx::B2DPolygon& rPolygon,
                const attribute::LineAttribute& rLineAttribute,
                const attribute::LineStartEndAttribute& rStart,
                const attribute::LineStartEndAttribute& rEnd);

            /// data read access
            const attribute::LineStartEndAttribute& getStart() const { return maStart; }
            const attribute::LineStartEndAttribute& getEnd() const { return maEnd; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE2D_POLYGONPRIMITIVE2D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
