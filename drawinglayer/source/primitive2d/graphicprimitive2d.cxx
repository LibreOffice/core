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

#include <sal/config.h>

#include <algorithm>

#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/cropprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <drawinglayer/primitive2d/graphicprimitivehelper2d.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>

namespace drawinglayer
{
    namespace primitive2d
    {
        void GraphicPrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& ) const
        {
            if(255 == getGraphicAttr().GetTransparency())
            {
                // content is invisible, done
                return;
            }

            // do not apply mirroring from GraphicAttr to the Metafile by calling
            // GetTransformedGraphic, this will try to mirror the Metafile using Scale()
            // at the Metafile. This again calls Scale at the single MetaFile actions,
            // but this implementation never worked. I reworked that implementations,
            // but for security reasons i will try not to use it.
            basegfx::B2DHomMatrix aTransform(getTransform());

            if(getGraphicAttr().IsMirrored())
            {
                // content needs mirroring
                const bool bHMirr(getGraphicAttr().GetMirrorFlags() & BmpMirrorFlags::Horizontal);
                const bool bVMirr(getGraphicAttr().GetMirrorFlags() & BmpMirrorFlags::Vertical);

                // mirror by applying negative scale to the unit primitive and
                // applying the object transformation on it.
                aTransform = basegfx::utils::createScaleB2DHomMatrix(
                    bHMirr ? -1.0 : 1.0,
                    bVMirr ? -1.0 : 1.0);
                aTransform.translate(
                    bHMirr ? 1.0 : 0.0,
                    bVMirr ? 1.0 : 0.0);
                aTransform = getTransform() * aTransform;
            }

            // Get transformed graphic. Suppress rotation and cropping, only filtering is needed
            // here (and may be replaced later on). Cropping is handled below as mask primitive (if set).
            // Also need to suppress mirroring, it is part of the transformation now (see above).
            // Also move transparency handling to embedding to a UnifiedTransparencePrimitive2D; do
            // that by remembering original transparency and applying that later if needed
            GraphicAttr aSuppressGraphicAttr(getGraphicAttr());

            aSuppressGraphicAttr.SetCrop(0, 0, 0, 0);
            aSuppressGraphicAttr.SetRotation(0);
            aSuppressGraphicAttr.SetMirrorFlags(BmpMirrorFlags::NONE);
            aSuppressGraphicAttr.SetTransparency(0);

            const GraphicObject& rGraphicObject = getGraphicObject();
            Graphic aTransformedGraphic(rGraphicObject.GetGraphic());
            const bool isBitmap(GraphicType::Bitmap == aTransformedGraphic.GetType() && !aTransformedGraphic.getVectorGraphicData().get());
            const bool isAdjusted(getGraphicAttr().IsAdjusted());
            const bool isDrawMode(GraphicDrawMode::Standard != getGraphicAttr().GetDrawMode());

            if(isBitmap && (isAdjusted || isDrawMode))
            {
                // the pure primitive solution with the color modifiers works well, too, but when
                // it is a bitmap graphic the old modification currently is faster; so use it here
                // instead of creating all as in create2DColorModifierEmbeddingsAsNeeded (see below).
                // Still, crop, rotation, mirroring and transparency is handled by primitives already
                // (see above).
                // This could even be done when vector graphic, but we explicitly want to have the
                // pure primitive solution for this; this will allow vector graphics to stay vector
                // graphics, independent from the color filtering stuff. This will enhance e.g.
                // SVG and print quality while reducing data size at the same time.
                // The other way around the old modifications when only used on already bitmap objects
                // will not lose any quality.
                aTransformedGraphic = rGraphicObject.GetTransformedGraphic(&aSuppressGraphicAttr);

                // reset GraphicAttr after use to not apply double
                aSuppressGraphicAttr = GraphicAttr();
            }

            // create sub-content; helper takes care of correct handling of
            // bitmap, svg or metafile content
            Primitive2DContainer aRetval;
            create2DDecompositionOfGraphic(
                aRetval,
                aTransformedGraphic,
                aTransform);

            if(aRetval.empty())
            {
                // content is invisible, done
                return;
            }

            if(isAdjusted || isDrawMode)
            {
                // embed to needed ModifiedColorPrimitive2D's if necessary. Do this for
                // adjustments and draw mode specials
                aRetval = create2DColorModifierEmbeddingsAsNeeded(
                    aRetval,
                    aSuppressGraphicAttr.GetDrawMode(),
                    std::clamp(aSuppressGraphicAttr.GetLuminance() * 0.01, -1.0, 1.0),
                    std::clamp(aSuppressGraphicAttr.GetContrast() * 0.01, -1.0, 1.0),
                    std::clamp(aSuppressGraphicAttr.GetChannelR() * 0.01, -1.0, 1.0),
                    std::clamp(aSuppressGraphicAttr.GetChannelG() * 0.01, -1.0, 1.0),
                    std::clamp(aSuppressGraphicAttr.GetChannelB() * 0.01, -1.0, 1.0),
                    std::clamp(aSuppressGraphicAttr.GetGamma(), 0.0, 10.0),
                    aSuppressGraphicAttr.IsInvert());

                if(aRetval.empty())
                {
                    // content is invisible, done
                    return;
                }
            }

            if(getGraphicAttr().IsTransparent())
            {
                // check for transparency
                const double fTransparency(std::clamp(getGraphicAttr().GetTransparency() * (1.0 / 255.0), 0.0, 1.0));

                if(!basegfx::fTools::equalZero(fTransparency))
                {
                    const Primitive2DReference aUnifiedTransparence(
                        new UnifiedTransparencePrimitive2D(
                            aRetval,
                            fTransparency));

                    aRetval = Primitive2DContainer { aUnifiedTransparence };
                }
            }

            if(getGraphicAttr().IsCropped())
            {
                // check for cropping
                // calculate scalings between real image size and logic object size. This
                // is necessary since the crop values are relative to original bitmap size
                const basegfx::B2DVector aObjectScale(aTransform * basegfx::B2DVector(1.0, 1.0));
                const basegfx::B2DVector aCropScaleFactor(
                    rGraphicObject.calculateCropScaling(
                        aObjectScale.getX(),
                        aObjectScale.getY(),
                        getGraphicAttr().GetLeftCrop(),
                        getGraphicAttr().GetTopCrop(),
                        getGraphicAttr().GetRightCrop(),
                        getGraphicAttr().GetBottomCrop()));

                // embed content in cropPrimitive
                Primitive2DReference xPrimitive(
                    new CropPrimitive2D(
                        aRetval,
                        aTransform,
                        getGraphicAttr().GetLeftCrop() * aCropScaleFactor.getX(),
                        getGraphicAttr().GetTopCrop() * aCropScaleFactor.getY(),
                        getGraphicAttr().GetRightCrop() * aCropScaleFactor.getX(),
                        getGraphicAttr().GetBottomCrop() * aCropScaleFactor.getY()));

                aRetval = Primitive2DContainer { xPrimitive };
            }

            rContainer.insert(rContainer.end(), aRetval.begin(), aRetval.end());
        }

        GraphicPrimitive2D::GraphicPrimitive2D(
            const basegfx::B2DHomMatrix& rTransform,
            const GraphicObject& rGraphicObject,
            const GraphicAttr& rGraphicAttr)
        :   BufferedDecompositionPrimitive2D(),
            maTransform(rTransform),
            maGraphicObject(rGraphicObject),
            maGraphicAttr(rGraphicAttr)
        {
        }

        GraphicPrimitive2D::GraphicPrimitive2D(
            const basegfx::B2DHomMatrix& rTransform,
            const GraphicObject& rGraphicObject)
        :   BufferedDecompositionPrimitive2D(),
            maTransform(rTransform),
            maGraphicObject(rGraphicObject),
            maGraphicAttr()
        {
        }

        bool GraphicPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const GraphicPrimitive2D& rCompare = static_cast<const GraphicPrimitive2D&>(rPrimitive);

                return (getTransform() == rCompare.getTransform()
                    && getGraphicObject() == rCompare.getGraphicObject()
                    && getGraphicAttr() == rCompare.getGraphicAttr());
            }

            return false;
        }

        basegfx::B2DRange GraphicPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            basegfx::B2DRange aRetval(0.0, 0.0, 1.0, 1.0);
            aRetval.transform(getTransform());
            return aRetval;
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(GraphicPrimitive2D, PRIMITIVE2D_ID_GRAPHICPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
