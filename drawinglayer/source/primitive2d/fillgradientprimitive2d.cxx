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

#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <texture/texture.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <utility>
#include <algorithm>


using namespace com::sun::star;


namespace drawinglayer::primitive2d
{
        // Get the OuterColor. Take into account that for attribute::GradientStyle::Axial
        // this is the last one due to inverted gradient usage (see constructor there)
        basegfx::BColor FillGradientPrimitive2D::getOuterColor() const
        {
            if (getFillGradient().getColorStops().empty())
                return basegfx::BColor();

            if (attribute::GradientStyle::Axial == getFillGradient().getStyle())
                return getFillGradient().getColorStops().back().getStopColor();

            return getFillGradient().getColorStops().front().getStopColor();
        }

        // Get the needed UnitPolygon dependent on the GradientStyle
        basegfx::B2DPolygon FillGradientPrimitive2D::getUnitPolygon() const
        {
            if (attribute::GradientStyle::Radial == getFillGradient().getStyle()
                || attribute::GradientStyle::Elliptical == getFillGradient().getStyle())
            {
                return basegfx::utils::createPolygonFromCircle(basegfx::B2DPoint(0.0, 0.0), 1.0);
            }

            return basegfx::utils::createPolygonFromRect(basegfx::B2DRange(-1.0, -1.0, 1.0, 1.0));
        }

        void FillGradientPrimitive2D::generateMatricesAndColors(
            std::function<void(const basegfx::B2DHomMatrix& rMatrix, const basegfx::BColor& rColor)> aCallback) const
        {
            switch(getFillGradient().getStyle())
            {
                case attribute::GradientStyle::Linear:
                {
                    texture::GeoTexSvxGradientLinear aGradient(
                        getDefinitionRange(),
                        getOutputRange(),
                        getFillGradient().getSteps(),
                        getFillGradient().getColorStops(),
                        getFillGradient().getBorder(),
                        getFillGradient().getAngle());
                    aGradient.appendTransformationsAndColors(aCallback);
                    break;
                }
                case attribute::GradientStyle::Axial:
                {
                    texture::GeoTexSvxGradientAxial aGradient(
                        getDefinitionRange(),
                        getOutputRange(),
                        getFillGradient().getSteps(),
                        getFillGradient().getColorStops(),
                        getFillGradient().getBorder(),
                        getFillGradient().getAngle());
                    aGradient.appendTransformationsAndColors(aCallback);
                    break;
                }
                case attribute::GradientStyle::Radial:
                {
                    texture::GeoTexSvxGradientRadial aGradient(
                        getDefinitionRange(),
                        getFillGradient().getSteps(),
                        getFillGradient().getColorStops(),
                        getFillGradient().getBorder(),
                        getFillGradient().getOffsetX(),
                        getFillGradient().getOffsetY());
                    aGradient.appendTransformationsAndColors(aCallback);
                    break;
                }
                case attribute::GradientStyle::Elliptical:
                {
                    texture::GeoTexSvxGradientElliptical aGradient(
                        getDefinitionRange(),
                        getFillGradient().getSteps(),
                        getFillGradient().getColorStops(),
                        getFillGradient().getBorder(),
                        getFillGradient().getOffsetX(),
                        getFillGradient().getOffsetY(),
                        getFillGradient().getAngle());
                    aGradient.appendTransformationsAndColors(aCallback);
                    break;
                }
                case attribute::GradientStyle::Square:
                {
                    texture::GeoTexSvxGradientSquare aGradient(
                        getDefinitionRange(),
                        getFillGradient().getSteps(),
                        getFillGradient().getColorStops(),
                        getFillGradient().getBorder(),
                        getFillGradient().getOffsetX(),
                        getFillGradient().getOffsetY(),
                        getFillGradient().getAngle());
                    aGradient.appendTransformationsAndColors(aCallback);
                    break;
                }
                case attribute::GradientStyle::Rect:
                {
                    texture::GeoTexSvxGradientRect aGradient(
                        getDefinitionRange(),
                        getFillGradient().getSteps(),
                        getFillGradient().getColorStops(),
                        getFillGradient().getBorder(),
                        getFillGradient().getOffsetX(),
                        getFillGradient().getOffsetY(),
                        getFillGradient().getAngle());
                    aGradient.appendTransformationsAndColors(aCallback);
                    break;
                }
            }
        }

        void FillGradientPrimitive2D::createFill(Primitive2DContainer& rContainer, bool bOverlapping) const
        {
            if (bOverlapping)
            {
                // OverlappingFill: create solid fill with outmost color
                rContainer.push_back(
                    new PolyPolygonColorPrimitive2D(
                        basegfx::B2DPolyPolygon(
                            basegfx::utils::createPolygonFromRect(getOutputRange())),
                        getOuterColor()));

                // create solid fill steps by providing callback as lambda
                auto aCallback([&rContainer,this](
                    const basegfx::B2DHomMatrix& rMatrix,
                    const basegfx::BColor& rColor)
                {
                    // create part polygon
                    basegfx::B2DPolygon aNewPoly(getUnitPolygon());
                    aNewPoly.transform(rMatrix);

                    // create solid fill
                    rContainer.push_back(
                        new PolyPolygonColorPrimitive2D(
                            basegfx::B2DPolyPolygon(aNewPoly),
                            rColor));
                });

                // call value generator to trigger callbacks
                generateMatricesAndColors(aCallback);
            }
            else
            {
                // NonOverlappingFill
                if (getFillGradient().getColorStops().size() < 2)
                {
                    // not really a gradient, we need to create a start primitive
                    // entry using the single color and the covered area
                    const basegfx::B2DRange aOutmostRange(getOutputRange());
                    rContainer.push_back(
                        new PolyPolygonColorPrimitive2D(
                            basegfx::B2DPolyPolygon(basegfx::utils::createPolygonFromRect(aOutmostRange)),
                            getOuterColor()));
                }
                else
                {
                    // gradient with stops, prepare CombinedPolyPoly, use callback
                    basegfx::B2DPolyPolygon aCombinedPolyPoly;
                    basegfx::BColor aLastColor;

                    auto aCallback([&rContainer,&aCombinedPolyPoly,&aLastColor,this](
                        const basegfx::B2DHomMatrix& rMatrix,
                        const basegfx::BColor& rColor)
                    {
                        if (rContainer.empty())
                        {
                            // 1st callback, init CombinedPolyPoly & create 1st entry
                            basegfx::B2DRange aOutmostRange(getOutputRange());

                            // expand aOutmostRange with transformed first polygon
                            // to ensure confinement
                            basegfx::B2DPolygon aFirstPoly(getUnitPolygon());
                            aFirstPoly.transform(rMatrix);
                            aOutmostRange.expand(aFirstPoly.getB2DRange());

                            // build 1st combined polygon; outmost range 1st, then
                            // the shaped, transformed polygon
                            aCombinedPolyPoly.append(basegfx::utils::createPolygonFromRect(aOutmostRange));
                            aCombinedPolyPoly.append(aFirstPoly);

                            // create first primitive
                            rContainer.push_back(
                                new PolyPolygonColorPrimitive2D(
                                    aCombinedPolyPoly,
                                    getOuterColor()));

                            // save first polygon for re-use in next call, it's the second
                            // one, so remove 1st
                            aCombinedPolyPoly.remove(0);

                            // remember color for next primitive creation
                            aLastColor = rColor;
                        }
                        else
                        {
                            // regular n-th callback, create combined entry by re-using
                            // CombinedPolyPoly and aLastColor
                            basegfx::B2DPolygon aNextPoly(getUnitPolygon());
                            aNextPoly.transform(rMatrix);
                            aCombinedPolyPoly.append(aNextPoly);

                            // create primitive with correct color
                            rContainer.push_back(
                                new PolyPolygonColorPrimitive2D(
                                    aCombinedPolyPoly,
                                    aLastColor));

                            // prepare re-use of inner polygon, save color
                            aCombinedPolyPoly.remove(0);
                            aLastColor = rColor;
                        }
                    });

                    // call value generator to trigger callbacks
                    generateMatricesAndColors(aCallback);

                    // add last inner polygon with last color
                    rContainer.push_back(
                        new PolyPolygonColorPrimitive2D(
                            aCombinedPolyPoly,
                            aLastColor));
                }
            }
        }

        void FillGradientPrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // default creates overlapping fill which works with AntiAliasing and without.
            // The non-overlapping version does not create single filled polygons, but
            // PolyPolygons where each one describes a 'ring' for the gradient such
            // that the rings will not overlap. This is useful for the old XOR-paint
            // 'trick' of VCL which is recorded in Metafiles; so this version may be
            // used from the MetafilePrimitive2D in its decomposition.

            if(!getFillGradient().isDefault())
            {
                createFill(rContainer, /*bOverlapping*/true);
            }
        }

        FillGradientPrimitive2D::FillGradientPrimitive2D(
            const basegfx::B2DRange& rOutputRange,
            attribute::FillGradientAttribute aFillGradient)
        :   maOutputRange(rOutputRange),
            maDefinitionRange(rOutputRange),
            maFillGradient(std::move(aFillGradient))
        {
        }

        FillGradientPrimitive2D::FillGradientPrimitive2D(
            const basegfx::B2DRange& rOutputRange,
            const basegfx::B2DRange& rDefinitionRange,
            attribute::FillGradientAttribute aFillGradient)
        :   maOutputRange(rOutputRange),
            maDefinitionRange(rDefinitionRange),
            maFillGradient(std::move(aFillGradient))
        {
        }

        bool FillGradientPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const FillGradientPrimitive2D& rCompare = static_cast<const FillGradientPrimitive2D&>(rPrimitive);

                return (getOutputRange() == rCompare.getOutputRange()
                    && getDefinitionRange() == rCompare.getDefinitionRange()
                    && getFillGradient() == rCompare.getFillGradient());
            }

            return false;
        }

        basegfx::B2DRange FillGradientPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            // return the geometrically visible area
            return getOutputRange();
        }

        // provide unique ID
        sal_uInt32 FillGradientPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_FILLGRADIENTPRIMITIVE2D;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
