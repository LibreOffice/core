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

#pragma once

#include <drawinglayer/drawinglayerdllapi.h>

#include <drawinglayer/primitive2d/BufferedDecompositionPrimitive2D.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>


// predefines

namespace basegfx { class B2DPolygon; }


// FillGradientPrimitive2D class

namespace drawinglayer::primitive2d
{
        /** FillGradientPrimitive2D class

            This class defines a gradient filling for a rectangular area. The
            Range is defined by the Transformation, the gradient by the FillGradientAttribute.

            The decomposition will deliver the decomposed gradient, e.g. for an ellipse
            gradient the various ellipses in various color steps will be created.

            I have added functionality to create both versions of filled decompositions:
            Those who overlap and non-overlapping ones. The overlapping version is the
            default one since it works with and without AntiAliasing. The non-overlapping
            version is used in the MetafilePrimitive2D decomposition when the old XOR
            paint was recorded.
         */
        class DRAWINGLAYER_DLLPUBLIC FillGradientPrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            /// the geometrically visible area
            basegfx::B2DRange                       maOutputRange;

            /// the area the gradient definition is based on
            /// in the simplest case identical to OutputRange
            basegfx::B2DRange                       maDefinitionRange;

            /// the gradient definition
            attribute::FillGradientAttribute        maFillGradient;


        protected:
            /// local helper
            Primitive2DReference createFill(bool bOverlapping) const;

            /// local decomposition.
            virtual Primitive2DReference create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const override;

        public:
            /// helpers that support e.g. direct paint/geometry creation
            basegfx::B2DPolygon getUnitPolygon() const;
            basegfx::BColor getOuterColor() const;
            void generateMatricesAndColors(
                std::function<void(const basegfx::B2DHomMatrix& rMatrix, const basegfx::BColor& rColor)> aCallback) const;

            /// constructors. The one without definition range will use output range as definition range
            FillGradientPrimitive2D(
                const basegfx::B2DRange& rOutputRange,
                attribute::FillGradientAttribute aFillGradient);
            FillGradientPrimitive2D(
                const basegfx::B2DRange& rOutputRange,
                const basegfx::B2DRange& rDefinitionRange,
                attribute::FillGradientAttribute aFillGradient);

            /// data read access
            const basegfx::B2DRange& getOutputRange() const { return maOutputRange; }
            const basegfx::B2DRange& getDefinitionRange() const { return maDefinitionRange; }
            const attribute::FillGradientAttribute& getFillGradient() const { return maFillGradient; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override final;

            /// get range
            virtual basegfx::B2DRange getB2DRange(const geometry::ViewInformation2D& rViewInformation) const override final;

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override final;
        };
} // end of namespace drawinglayer::primitive2d


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
