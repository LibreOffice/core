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
#include <drawinglayer/texture/texture.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>



using namespace com::sun::star;



namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DContainer FillHatchPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DContainer aRetval;

            if(!getFillHatch().isDefault())
            {
                // create hatch
                const basegfx::BColor aHatchColor(getFillHatch().getColor());
                const double fAngle(getFillHatch().getAngle());
                ::std::vector< basegfx::B2DHomMatrix > aMatrices;
                double fDistance(getFillHatch().getDistance());
                const bool bAdaptDistance(0 != getFillHatch().getMinimalDiscreteDistance());

                // #i120230# evtl. adapt distance
                if(bAdaptDistance)
                {
                    const double fDiscreteDistance(getFillHatch().getDistance() / getDiscreteUnit());

                    if(fDiscreteDistance < (double)getFillHatch().getMinimalDiscreteDistance())
                    {
                        fDistance = (double)getFillHatch().getMinimalDiscreteDistance() * getDiscreteUnit();
                    }
                }

                // get hatch transformations
                switch(getFillHatch().getStyle())
                {
                    case attribute::HATCHSTYLE_TRIPLE:
                    {
                        // rotated 45 degrees
                        texture::GeoTexSvxHatch aHatch(
                            getDefinitionRange(),
                            getOutputRange(),
                            fDistance,
                            fAngle - F_PI4);

                        aHatch.appendTransformations(aMatrices);

                        // fall-through by purpose
                    }
                    case attribute::HATCHSTYLE_DOUBLE:
                    {
                        // rotated 90 degrees
                        texture::GeoTexSvxHatch aHatch(
                            getDefinitionRange(),
                            getOutputRange(),
                            fDistance,
                            fAngle - F_PI2);

                        aHatch.appendTransformations(aMatrices);

                        // fall-through by purpose
                    }
                    case attribute::HATCHSTYLE_SINGLE:
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
                aRetval.resize(bFillBackground ? aMatrices.size() + 1L : aMatrices.size());

                // evtl. create filled background
                if(bFillBackground)
                {
                    // create primitive for background
                    const Primitive2DReference xRef(
                        new PolyPolygonColorPrimitive2D(
                            basegfx::B2DPolyPolygon(
                                basegfx::tools::createPolygonFromRect(getOutputRange())), getBColor()));
                    aRetval[0] = xRef;
                }

                // create primitives
                const basegfx::B2DPoint aStart(0.0, 0.0);
                const basegfx::B2DPoint aEnd(1.0, 0.0);

                for(size_t a(0); a < aMatrices.size(); a++)
                {
                    const basegfx::B2DHomMatrix& rMatrix = aMatrices[a];
                    basegfx::B2DPolygon aNewLine;

                    aNewLine.append(rMatrix * aStart);
                    aNewLine.append(rMatrix * aEnd);

                    // create hairline
                    const Primitive2DReference xRef(new PolygonHairlinePrimitive2D(aNewLine, aHatchColor));
                    aRetval[bFillBackground ? (a + 1) : a] = xRef;
                }
            }

            return aRetval;
        }

        FillHatchPrimitive2D::FillHatchPrimitive2D(
            const basegfx::B2DRange& rOutputRange,
            const basegfx::BColor& rBColor,
            const attribute::FillHatchAttribute& rFillHatch)
        :   DiscreteMetricDependentPrimitive2D(),
            maOutputRange(rOutputRange),
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
        :   DiscreteMetricDependentPrimitive2D(),
            maOutputRange(rOutputRange),
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

        Primitive2DContainer FillHatchPrimitive2D::get2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            bool bAdaptDistance(0 != getFillHatch().getMinimalDiscreteDistance());

            if(bAdaptDistance)
            {
                // behave view-dependent
                return DiscreteMetricDependentPrimitive2D::get2DDecomposition(rViewInformation);
            }
            else
            {
                // behave view-independent
                return BufferedDecompositionPrimitive2D::get2DDecomposition(rViewInformation);
            }
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(FillHatchPrimitive2D, PRIMITIVE2D_ID_FILLHATCHPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
