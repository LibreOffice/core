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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONPRIMITIVE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        /** PolygonHairlinePrimitive3D class

            This primitive defines a Hairline in 3D. Since hairlines are view-dependent,
            this primitive is view-dependent, too.

            This is one of the non-decomposable 3D primitives, so a renderer
            should proccess it.
         */
        class DRAWINGLAYER_DLLPUBLIC PolygonHairlinePrimitive3D : public BasePrimitive3D
        {
        private:
            /// the hairline geometry
            basegfx::B3DPolygon                     maPolygon;

            /// the hairline color
            basegfx::BColor                         maBColor;

        public:
            /// constructor
            PolygonHairlinePrimitive3D(
                const basegfx::B3DPolygon& rPolygon,
                const basegfx::BColor& rBColor);

            /// data read access
            const basegfx::B3DPolygon& getB3DPolygon() const { return maPolygon; }
            const basegfx::BColor& getBColor() const { return maBColor; }

            /// compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const;

            /// get range
            virtual basegfx::B3DRange getB3DRange(const geometry::ViewInformation3D& rViewInformation) const;

            /// provide unique ID
            DeclPrimitrive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive3d
    {
        /** PolygonStrokePrimitive3D class

            This primitive defines a 3D line with line width, line join, line color
            and stroke attributes. It will be decomposed dependent on the definition
            to the needed primitives, e.g. filled Tubes for fat lines.
         */
        class DRAWINGLAYER_DLLPUBLIC PolygonStrokePrimitive3D : public BufferedDecompositionPrimitive3D
        {
        private:
            /// the line geometry
            basegfx::B3DPolygon                     maPolygon;

            /// the line attributes like width, join and color
            attribute::LineAttribute                maLineAttribute;

            /// the line stroking (if used)
            attribute::StrokeAttribute              maStrokeAttribute;

        protected:
            /// local decomposition.
            virtual Primitive3DSequence create3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const;

        public:
            /// constructor
            PolygonStrokePrimitive3D(
                const basegfx::B3DPolygon& rPolygon,
                const attribute::LineAttribute& rLineAttribute,
                const attribute::StrokeAttribute& rStrokeAttribute);

            /// data read access
            basegfx::B3DPolygon getB3DPolygon() const { return maPolygon; }
            const attribute::LineAttribute& getLineAttribute() const { return maLineAttribute; }
            const attribute::StrokeAttribute& getStrokeAttribute() const { return maStrokeAttribute; }

            /// compare operator
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const;

            /// provide unique ID
            DeclPrimitrive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONPRIMITIVE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
