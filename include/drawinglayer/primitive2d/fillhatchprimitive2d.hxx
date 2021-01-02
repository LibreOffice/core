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

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <drawinglayer/primitive2d/primitivetools2d.hxx>
#include <drawinglayer/attribute/fillhatchattribute.hxx>
#include <basegfx/color/bcolor.hxx>

namespace drawinglayer::primitive2d
{
        /** FillHatchPrimitive2D class

            This class defines a hatch filling for a rectangular area. The
            Range is defined by the Transformation, the hatch by the FillHatchAttribute.
            If the background is to be filled, a flag in FillHatchAttribute is set and
            the BColor defines the background color.

            #i120230# This primitive is now evtl. metric dependent due to the value
            MinimalDiscreteDistance in the FillHatchAttribute if the value is not zero.
            This is used for a more appealing, VCL-like visualisation by not letting the
            distances get too small between lines.

            The decomposition will deliver the hatch lines.
         */
        class DRAWINGLAYER_DLLPUBLIC FillHatchPrimitive2D final : public DiscreteMetricDependentPrimitive2D
        {
        private:
            /// the geometrically visible area
            basegfx::B2DRange                       maOutputRange;

            /// the area the gradient definition is based on
            /// in the simplest case identical to OutputRange
            basegfx::B2DRange                       maDefinitionRange;

            /// the hatch definition
            attribute::FillHatchAttribute           maFillHatch;

            /// hatch background color (if used)
            basegfx::BColor                         maBColor;

            /// local decomposition.
            virtual void create2DDecomposition(Primitive2DContainer& rContainer, VisitingParameters const & rParameters) const override;

        public:
            /// constructors. The one without definition range will use output range as definition range
            FillHatchPrimitive2D(
                const basegfx::B2DRange& rOutputRange,
                const basegfx::BColor& rBColor,
                const attribute::FillHatchAttribute& rFillHatch);
            FillHatchPrimitive2D(
                const basegfx::B2DRange& rOutputRange,
                const basegfx::B2DRange& rDefinitionRange,
                const basegfx::BColor& rBColor,
                const attribute::FillHatchAttribute& rFillHatch);

            /// data read access
            const basegfx::B2DRange& getOutputRange() const { return maOutputRange; }
            const basegfx::B2DRange& getDefinitionRange() const { return maDefinitionRange; }
            const attribute::FillHatchAttribute& getFillHatch() const { return maFillHatch; }
            const basegfx::BColor& getBColor() const { return maBColor; }

            /// compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const override;

            /// get range
            virtual basegfx::B2DRange getB2DRange(VisitingParameters const & rParameters) const override;

            /// Override standard getDecomposition to be view-dependent here
            virtual void get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, VisitingParameters const & rParameters) const override;

            /// provide unique ID
            virtual sal_uInt32 getPrimitive2DID() const override;
        };
} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
