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

#include <drawinglayer/processor2d/textaspolygonextractor2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonHairlinePrimitive2D.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>


namespace drawinglayer::processor2d
{
        void TextAsPolygonExtractor2D::processBasePrimitive2D(const primitive2d::BasePrimitive2D& rCandidate)
        {
            switch(rCandidate.getPrimitive2DID())
            {
                case PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D :
                {
                    // TextDecoratedPortionPrimitive2D can produce the following primitives
                    // when being decomposed:
                    //
                    // - TextSimplePortionPrimitive2D
                    // - PolygonWavePrimitive2D
                    //      - PolygonStrokePrimitive2D
                    // - PolygonStrokePrimitive2D
                    //      - PolyPolygonColorPrimitive2D
                    //      - PolyPolygonHairlinePrimitive2D
                    //          - PolygonHairlinePrimitive2D
                    // - ShadowPrimitive2D
                    //      - ModifiedColorPrimitive2D
                    //      - TransformPrimitive2D
                    // - TextEffectPrimitive2D
                    //      - ModifiedColorPrimitive2D
                    //      - TransformPrimitive2D
                    //      - GroupPrimitive2D

                    // encapsulate with flag and use decomposition
                    mnInText++;
                    process(rCandidate);
                    mnInText--;

                    break;
                }
                case PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D :
                {
                    // TextSimplePortionPrimitive2D can produce the following primitives
                    // when being decomposed:
                    //
                    // - PolyPolygonColorPrimitive2D
                    // - TextEffectPrimitive2D
                    //      - ModifiedColorPrimitive2D
                    //      - TransformPrimitive2D
                    //      - GroupPrimitive2D

                    // encapsulate with flag and use decomposition
                    mnInText++;
                    process(rCandidate);
                    mnInText--;

                    break;
                }

                // as can be seen from the TextSimplePortionPrimitive2D and the
                // TextDecoratedPortionPrimitive2D, inside of the mnInText marks
                // the following primitives can occur containing geometry data
                // from text decomposition:
                //
                // - PolyPolygonColorPrimitive2D
                // - PolygonHairlinePrimitive2D
                // - PolyPolygonHairlinePrimitive2D (for convenience)
                //
                case PRIMITIVE2D_ID_POLYPOLYGONCOLORPRIMITIVE2D :
                {
                    if(mnInText)
                    {
                        const primitive2d::PolyPolygonColorPrimitive2D& rPoPoCoCandidate(static_cast< const primitive2d::PolyPolygonColorPrimitive2D& >(rCandidate));
                        basegfx::B2DPolyPolygon aPolyPolygon(rPoPoCoCandidate.getB2DPolyPolygon());

                        if(aPolyPolygon.count())
                        {
                            // transform the PolyPolygon
                            aPolyPolygon.transform(getViewInformation2D().getObjectToViewTransformation());

                            // get evtl. corrected color
                            const basegfx::BColor aColor(maBColorModifierStack.getModifiedColor(rPoPoCoCandidate.getBColor()));

                            // add to result vector
                            maTarget.emplace_back(aPolyPolygon, aColor, true);
                        }
                    }

                    break;
                }
                case PRIMITIVE2D_ID_POLYGONHAIRLINEPRIMITIVE2D :
                {
                    if(mnInText)
                    {
                        const primitive2d::PolygonHairlinePrimitive2D& rPoHaCandidate(static_cast< const primitive2d::PolygonHairlinePrimitive2D& >(rCandidate));
                        basegfx::B2DPolygon aPolygon(rPoHaCandidate.getB2DPolygon());

                        if(aPolygon.count())
                        {
                            // transform the Polygon
                            aPolygon.transform(getViewInformation2D().getObjectToViewTransformation());

                            // get evtl. corrected color
                            const basegfx::BColor aColor(maBColorModifierStack.getModifiedColor(rPoHaCandidate.getBColor()));

                            // add to result vector
                            maTarget.emplace_back(basegfx::B2DPolyPolygon(aPolygon), aColor, false);
                        }
                    }

                    break;
                }
                case PRIMITIVE2D_ID_POLYPOLYGONHAIRLINEPRIMITIVE2D :
                {
                    if(mnInText)
                    {
                        const primitive2d::PolyPolygonHairlinePrimitive2D& rPoPoHaCandidate(static_cast< const primitive2d::PolyPolygonHairlinePrimitive2D& >(rCandidate));
                        basegfx::B2DPolyPolygon aPolyPolygon(rPoPoHaCandidate.getB2DPolyPolygon());

                        if(aPolyPolygon.count())
                        {
                            // transform the Polygon
                            aPolyPolygon.transform(getViewInformation2D().getObjectToViewTransformation());

                            // get evtl. corrected color
                            const basegfx::BColor aColor(maBColorModifierStack.getModifiedColor(rPoPoHaCandidate.getBColor()));

                            // add to result vector
                            maTarget.emplace_back(aPolyPolygon, aColor, false);
                        }
                    }

                    break;
                }

                // usage of color modification stack is needed
                case PRIMITIVE2D_ID_MODIFIEDCOLORPRIMITIVE2D :
                {
                    const primitive2d::ModifiedColorPrimitive2D& rModifiedColorCandidate(static_cast< const primitive2d::ModifiedColorPrimitive2D& >(rCandidate));

                    if(!rModifiedColorCandidate.getChildren().empty())
                    {
                        maBColorModifierStack.push(rModifiedColorCandidate.getColorModifier());
                        process(rModifiedColorCandidate.getChildren());
                        maBColorModifierStack.pop();
                    }

                    break;
                }

                // usage of transformation stack is needed
                case PRIMITIVE2D_ID_TRANSFORMPRIMITIVE2D :
                {
                    // remember current transformation and ViewInformation
                    const primitive2d::TransformPrimitive2D& rTransformCandidate(static_cast< const primitive2d::TransformPrimitive2D& >(rCandidate));
                    const geometry::ViewInformation2D aLastViewInformation2D(getViewInformation2D());

                    // create new transformations for CurrentTransformation and for local ViewInformation2D
                    const geometry::ViewInformation2D aViewInformation2D(
                        getViewInformation2D().getObjectTransformation() * rTransformCandidate.getTransformation(),
                        getViewInformation2D().getViewTransformation(),
                        getViewInformation2D().getViewport(),
                        getViewInformation2D().getVisualizedPage(),
                        getViewInformation2D().getViewTime());
                    updateViewInformation(aViewInformation2D);

                    // process content
                    process(rTransformCandidate.getChildren());

                    // restore transformations
                    updateViewInformation(aLastViewInformation2D);

                    break;
                }

                // ignorable primitives
                case PRIMITIVE2D_ID_SCENEPRIMITIVE2D :
                case PRIMITIVE2D_ID_WRONGSPELLPRIMITIVE2D :
                case PRIMITIVE2D_ID_MARKERARRAYPRIMITIVE2D :
                case PRIMITIVE2D_ID_POINTARRAYPRIMITIVE2D :
                case PRIMITIVE2D_ID_BITMAPPRIMITIVE2D :
                case PRIMITIVE2D_ID_METAFILEPRIMITIVE2D :
                case PRIMITIVE2D_ID_MASKPRIMITIVE2D :
                {
                    break;
                }

                default :
                {
                    // process recursively
                    process(rCandidate);
                    break;
                }
            }
        }

        TextAsPolygonExtractor2D::TextAsPolygonExtractor2D(const geometry::ViewInformation2D& rViewInformation)
        :   BaseProcessor2D(rViewInformation),
            maBColorModifierStack(),
            mnInText(0)
        {
        }

        TextAsPolygonExtractor2D::~TextAsPolygonExtractor2D()
        {
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
