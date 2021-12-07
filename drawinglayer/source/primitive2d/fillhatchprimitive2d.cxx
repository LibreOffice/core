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

#include <drawinglayer/primitive2d/fillhatchprimitive2d.hxx>
#include <texture/texture.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>


using namespace com::sun::star;


namespace drawinglayer::primitive2d
{
        void FillHatchPrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            if(getFillHatch().isDefault())
                return;

            // create hatch
            const basegfx::BColor aHatchColor(getFillHatch().getColor());
            const double fAngle(getFillHatch().getAngle());
            std::vector< basegfx::B2DHomMatrix > aMatrices;
            double fDistance(getFillHatch().getDistance());
            const bool bAdaptDistance(0 != getFillHatch().getMinimalDiscreteDistance());

            // #i120230# evtl. adapt distance
            if(bAdaptDistance)
            {
                const double fDiscreteDistance(getFillHatch().getDistance() / getDiscreteUnit());

                if(fDiscreteDistance < static_cast<double>(getFillHatch().getMinimalDiscreteDistance()))
                {
                    fDistance = static_cast<double>(getFillHatch().getMinimalDiscreteDistance()) * getDiscreteUnit();
                }
            }

            // get hatch transformations
            switch(getFillHatch().getStyle())
            {
                case attribute::HatchStyle::Triple:
                {
                    // rotated 45 degrees
                    texture::GeoTexSvxHatch aHatch(
                        getDefinitionRange(),
                        getOutputRange(),
                        fDistance,
                        fAngle - M_PI_4);

                    aHatch.appendTransformations(aMatrices);

                    [[fallthrough]];
                }
                case attribute::HatchStyle::Double:
                {
                    // rotated 90 degrees
                    texture::GeoTexSvxHatch aHatch(
                        getDefinitionRange(),
                        getOutputRange(),
                        fDistance,
                        fAngle - M_PI_2);

                    aHatch.appendTransformations(aMatrices);

                    [[fallthrough]];
                }
                case attribute::HatchStyle::Single:
                {
                    // angle as given
                    texture::GeoTexSvxHatch aHatch(
                        getDefinitionRange(),
                        getOutputRange(),
                        fDistance,
                        fAngle);

                    aHatch.appendTransformations(aMatrices);
                }
            }

            // prepare return value
            const bool bFillBackground(getFillHatch().isFillBackground());

            // evtl. create filled background
            if(bFillBackground)
            {
                // create primitive for background
                rContainer.push_back(
                    new PolyPolygonColorPrimitive2D(
                        basegfx::B2DPolyPolygon(
                            basegfx::utils::createPolygonFromRect(getOutputRange())), getBColor()));
            }

            // create primitives
            const basegfx::B2DPoint aStart(0.0, 0.0);
            const basegfx::B2DPoint aEnd(1.0, 0.0);

            for (const auto &a : aMatrices)
            {
                const basegfx::B2DHomMatrix& rMatrix = a;
                basegfx::B2DPolygon aNewLine;

                aNewLine.append(rMatrix * aStart);
                aNewLine.append(rMatrix * aEnd);

                // create hairline
                rContainer.push_back(new PolygonHairlinePrimitive2D(aNewLine, aHatchColor));
            }
        }

        FillHatchPrimitive2D::FillHatchPrimitive2D(
            const basegfx::B2DRange& rOutputRange,
            const basegfx::BColor& rBColor,
            const attribute::FillHatchAttribute& rFillHatch)
        :   maOutputRange(rOutputRange),
            maDefinitionRange(rOutputRange),
            maFillHatch(rFillHatch),
            maBColor(rBColor)
        {
        }

        FillHatchPrimitive2D::FillHatchPrimitive2D(
            const basegfx::B2DRange& rOutputRange,
            const basegfx::B2DRange& rDefinitionRange,
            const basegfx::BColor& rBColor,
            const attribute::FillHatchAttribute& rFillHatch)
        :   maOutputRange(rOutputRange),
            maDefinitionRange(rDefinitionRange),
            maFillHatch(rFillHatch),
            maBColor(rBColor)
        {
        }

        bool FillHatchPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(DiscreteMetricDependentPrimitive2D::operator==(rPrimitive))
            {
                const FillHatchPrimitive2D& rCompare = static_cast<const FillHatchPrimitive2D&>(rPrimitive);

                return (getOutputRange() == rCompare.getOutputRange()
                    && getDefinitionRange() == rCompare.getDefinitionRange()
                    && getFillHatch() == rCompare.getFillHatch()
                    && getBColor() == rCompare.getBColor());
            }

            return false;
        }

        basegfx::B2DRange FillHatchPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // return the geometrically visible area
            return getOutputRange();
        }

        void FillHatchPrimitive2D::get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor, const geometry::ViewInformation2D& rViewInformation) const
        {
            bool bAdaptDistance(0 != getFillHatch().getMinimalDiscreteDistance());

            if(bAdaptDistance)
            {
                // behave view-dependent
                DiscreteMetricDependentPrimitive2D::get2DDecomposition(rVisitor, rViewInformation);
            }
            else
            {
                // behave view-independent
                BufferedDecompositionPrimitive2D::get2DDecomposition(rVisitor, rViewInformation);
            }
        }

        // provide unique ID
        sal_uInt32 FillHatchPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_FILLHATCHPRIMITIVE2D;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
