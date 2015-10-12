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

#ifndef INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONPRIMITIVE3D_HXX
#define INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONPRIMITIVE3D_HXX

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive3d/baseprimitive3d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <drawinglayer/attribute/lineattribute.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>



namespace drawinglayer
{
    namespace primitive3d
    {
        /** PolygonHairlinePrimitive3D class

            This primitive defines a Hairline in 3D. Since hairlines are view-dependent,
            this primitive is view-dependent, too.

            This is one of the non-decomposable 3D primitives, so a renderer
            should process it.
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
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const override;

            /// get range
            virtual basegfx::B3DRange getB3DRange(const geometry::ViewInformation3D& rViewInformation) const override;

            /// provide unique ID
            DeclPrimitive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer



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
            virtual Primitive3DSequence create3DDecomposition(const geometry::ViewInformation3D& rViewInformation) const override;

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
            virtual bool operator==(const BasePrimitive3D& rPrimitive) const override;

            /// provide unique ID
            DeclPrimitive3DIDBlock()
        };
    } // end of namespace primitive3d
} // end of namespace drawinglayer



#endif //INCLUDED_DRAWINGLAYER_PRIMITIVE3D_POLYGONPRIMITIVE3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
