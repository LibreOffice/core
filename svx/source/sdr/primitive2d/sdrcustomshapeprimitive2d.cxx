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

#include <sdr/primitive2d/sdrcustomshapeprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <sdr/primitive2d/sdrdecompositiontools.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <drawinglayer/attribute/sdrlineattribute.hxx>


using namespace com::sun::star;


namespace drawinglayer::primitive2d
{
        void SdrCustomShapePrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            Primitive2DContainer aRetval(getSubPrimitives());

            // Soft edges should be before text, since text is not affected by soft edges
            if (!aRetval.empty() && getSdrSTAttribute().getSoftEdgeRadius())
            {
                aRetval = createEmbeddedSoftEdgePrimitive(aRetval,
                                                          getSdrSTAttribute().getSoftEdgeRadius());
            }

            // add text
            if(!getSdrSTAttribute().getText().isDefault())
            {
                const basegfx::B2DPolygon& aUnitOutline(basegfx::utils::createUnitPolygon());

                aRetval.push_back(
                    createTextPrimitive(
                        basegfx::B2DPolyPolygon(aUnitOutline),
                        getTextBox(),
                        getSdrSTAttribute().getText(),
                        attribute::SdrLineAttribute(),
                        false,
                        getWordWrap()));
            }

            // tdf#132199: put glow before shadow, to have shadow of the glow, not the opposite
            if (!aRetval.empty() && !getSdrSTAttribute().getGlow().isDefault())
            {
                // glow
                aRetval = createEmbeddedGlowPrimitive(aRetval, getSdrSTAttribute().getGlow());
            }

            // add shadow
            if(!aRetval.empty() && !getSdrSTAttribute().getShadow().isDefault())
            {
                // #i105323# add generic shadow only for 2D shapes. For
                // 3D shapes shadow will be set at the individual created
                // visualisation objects and be visualized by the 3d renderer
                // as a single shadow.

                // The shadow for AutoShapes could be handled uniformly by not setting any
                // shadow items at the helper model objects and only adding shadow here for
                // 2D and 3D (and it works, too), but this would lead to two 3D scenes for
                // the 3D object; one for the shadow and one for the content. The one for the
                // shadow will be correct (using ColorModifierStack), but expensive.
                if(!get3DShape())
                {
                    aRetval = createEmbeddedShadowPrimitive(aRetval, getSdrSTAttribute().getShadow(),
                                                            maTransform);
                }
            }

            rContainer.insert(rContainer.end(), aRetval.begin(), aRetval.end());
        }

        SdrCustomShapePrimitive2D::SdrCustomShapePrimitive2D(
            const attribute::SdrEffectsTextAttribute& rSdrSTAttribute,
            const Primitive2DContainer& rSubPrimitives,
            const basegfx::B2DHomMatrix& rTextBox,
            bool bWordWrap,
            bool b3DShape,
            const basegfx::B2DHomMatrix& rTransform)
        :   BufferedDecompositionPrimitive2D(),
            maSdrSTAttribute(rSdrSTAttribute),
            maSubPrimitives(rSubPrimitives),
            maTextBox(rTextBox),
            mbWordWrap(bWordWrap),
            mb3DShape(b3DShape),
            maTransform(rTransform)
        {
        }

        bool SdrCustomShapePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const SdrCustomShapePrimitive2D& rCompare = static_cast<const SdrCustomShapePrimitive2D&>(rPrimitive);

                return (getSdrSTAttribute() == rCompare.getSdrSTAttribute()
                    && getSubPrimitives() == rCompare.getSubPrimitives()
                    && getTextBox() == rCompare.getTextBox()
                    && getWordWrap() == rCompare.getWordWrap()
                    && get3DShape() == rCompare.get3DShape());
            }

            return false;
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(SdrCustomShapePrimitive2D, PRIMITIVE2D_ID_SDRCUSTOMSHAPEPRIMITIVE2D)

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
