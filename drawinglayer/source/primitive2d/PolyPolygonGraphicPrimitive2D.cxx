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

#include <drawinglayer/primitive2d/PolyPolygonGraphicPrimitive2D.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/fillgraphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <vcl/graph.hxx>

using namespace com::sun::star;

namespace drawinglayer::primitive2d
{
void PolyPolygonGraphicPrimitive2D::create2DDecomposition(
    Primitive2DContainer& rContainer, VisitingParameters const& /*rParameters*/) const
{
    if (getFillGraphic().isDefault())
        return;

    const Graphic& rGraphic = getFillGraphic().getGraphic();
    const GraphicType aType(rGraphic.GetType());

    // is there a bitmap or a metafile (do we have content)?
    if (GraphicType::Bitmap != aType && GraphicType::GdiMetafile != aType)
        return;

    const Size aPrefSize(rGraphic.GetPrefSize());

    // does content have a size?
    if (!(aPrefSize.Width() && aPrefSize.Height()))
        return;

    // create SubSequence with FillGraphicPrimitive2D based on polygon range
    const basegfx::B2DRange aOutRange(getB2DPolyPolygon().getB2DRange());
    const basegfx::B2DHomMatrix aNewObjectTransform(
        basegfx::utils::createScaleTranslateB2DHomMatrix(aOutRange.getRange(),
                                                         aOutRange.getMinimum()));
    Primitive2DReference xSubRef;

    if (aOutRange != getDefinitionRange())
    {
        // we want to paint (tiled) content which is defined relative to DefinitionRange
        // with the same tiling and offset(s) in the target range of the geometry (the
        // polygon). The range given in the local FillGraphicAttribute defines the position
        // of the graphic in unit coordinates relative to the DefinitionRange. Transform
        // this using DefinitionRange to get to the global definition and then with the
        // inverse transformation from the target range to go to unit coordinates relative
        // to that target coordinate system.
        basegfx::B2DRange aAdaptedRange(getFillGraphic().getGraphicRange());

        const basegfx::B2DHomMatrix aFromDefinitionRangeToGlobal(
            basegfx::utils::createScaleTranslateB2DHomMatrix(getDefinitionRange().getRange(),
                                                             getDefinitionRange().getMinimum()));

        aAdaptedRange.transform(aFromDefinitionRangeToGlobal);

        basegfx::B2DHomMatrix aFromGlobalToOutRange(
            basegfx::utils::createScaleTranslateB2DHomMatrix(aOutRange.getRange(),
                                                             aOutRange.getMinimum()));
        aFromGlobalToOutRange.invert();

        aAdaptedRange.transform(aFromGlobalToOutRange);

        const drawinglayer::attribute::FillGraphicAttribute aAdaptedFillGraphicAttribute(
            getFillGraphic().getGraphic(), aAdaptedRange, getFillGraphic().getTiling(),
            getFillGraphic().getOffsetX(), getFillGraphic().getOffsetY());

        xSubRef = new FillGraphicPrimitive2D(aNewObjectTransform, aAdaptedFillGraphicAttribute);
    }
    else
    {
        xSubRef = new FillGraphicPrimitive2D(aNewObjectTransform, getFillGraphic());
    }

    // embed to mask primitive
    rContainer.push_back(new MaskPrimitive2D(getB2DPolyPolygon(), Primitive2DContainer{ xSubRef }));
}

PolyPolygonGraphicPrimitive2D::PolyPolygonGraphicPrimitive2D(
    const basegfx::B2DPolyPolygon& rPolyPolygon, const basegfx::B2DRange& rDefinitionRange,
    const attribute::FillGraphicAttribute& rFillGraphic)
    : BufferedDecompositionPrimitive2D()
    , maPolyPolygon(rPolyPolygon)
    , maDefinitionRange(rDefinitionRange)
    , maFillGraphic(rFillGraphic)
{
}

bool PolyPolygonGraphicPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BufferedDecompositionPrimitive2D::operator==(rPrimitive))
    {
        const PolyPolygonGraphicPrimitive2D& rCompare
            = static_cast<const PolyPolygonGraphicPrimitive2D&>(rPrimitive);

        return (getB2DPolyPolygon() == rCompare.getB2DPolyPolygon()
                && getDefinitionRange() == rCompare.getDefinitionRange()
                && getFillGraphic() == rCompare.getFillGraphic());
    }

    return false;
}

// provide unique ID
sal_uInt32 PolyPolygonGraphicPrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_POLYPOLYGONGRAPHICPRIMITIVE2D;
}

} // end drawinglayer::primitive2d namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
