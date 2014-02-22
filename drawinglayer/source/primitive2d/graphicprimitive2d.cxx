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

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence GraphicPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D&
            ) const
        {
            Primitive2DSequence aRetval;

            if(255L == getGraphicAttr().GetTransparency())
            {
                
                return aRetval;
            }

            
            
            
            
            
            basegfx::B2DHomMatrix aTransform(getTransform());

            if(getGraphicAttr().IsMirrored())
            {
                
                const bool bHMirr(getGraphicAttr().GetMirrorFlags() & BMP_MIRROR_HORZ);
                const bool bVMirr(getGraphicAttr().GetMirrorFlags() & BMP_MIRROR_VERT);

                
                
                aTransform = basegfx::tools::createScaleB2DHomMatrix(
                    bHMirr ? -1.0 : 1.0,
                    bVMirr ? -1.0 : 1.0);
                aTransform.translate(
                    bHMirr ? 1.0 : 0.0,
                    bVMirr ? 1.0 : 0.0);
                aTransform = getTransform() * aTransform;
            }

            
            
            
            
            
            GraphicAttr aSuppressGraphicAttr(getGraphicAttr());

            aSuppressGraphicAttr.SetCrop(0, 0, 0, 0);
            aSuppressGraphicAttr.SetRotation(0);
            aSuppressGraphicAttr.SetMirrorFlags(0);
            aSuppressGraphicAttr.SetTransparency(0);

            const GraphicObject& rGraphicObject = getGraphicObject();
            Graphic aTransformedGraphic(rGraphicObject.GetGraphic());
            const bool isBitmap(GRAPHIC_BITMAP == aTransformedGraphic.GetType() && !aTransformedGraphic.getSvgData().get());
            const bool isAdjusted(getGraphicAttr().IsAdjusted());
            const bool isDrawMode(GRAPHICDRAWMODE_STANDARD != getGraphicAttr().GetDrawMode());

            if(isBitmap && (isAdjusted || isDrawMode))
            {
                
                
                
                
                
                
                
                
                
                
                
                aTransformedGraphic = rGraphicObject.GetTransformedGraphic(&aSuppressGraphicAttr);

                
                aSuppressGraphicAttr = GraphicAttr();
            }

            
            
            aRetval = create2DDecompositionOfGraphic(
                aTransformedGraphic,
                aTransform);

            if(!aRetval.getLength())
            {
                
                return aRetval;
            }

            if(isAdjusted || isDrawMode)
            {
                
                
                aRetval = create2DColorModifierEmbeddingsAsNeeded(
                    aRetval,
                    aSuppressGraphicAttr.GetDrawMode(),
                    basegfx::clamp(aSuppressGraphicAttr.GetLuminance() * 0.01, -1.0, 1.0),
                    basegfx::clamp(aSuppressGraphicAttr.GetContrast() * 0.01, -1.0, 1.0),
                    basegfx::clamp(aSuppressGraphicAttr.GetChannelR() * 0.01, -1.0, 1.0),
                    basegfx::clamp(aSuppressGraphicAttr.GetChannelG() * 0.01, -1.0, 1.0),
                    basegfx::clamp(aSuppressGraphicAttr.GetChannelB() * 0.01, -1.0, 1.0),
                    basegfx::clamp(aSuppressGraphicAttr.GetGamma(), 0.0, 10.0),
                    aSuppressGraphicAttr.IsInvert());

                if(!aRetval.getLength())
                {
                    
                    return aRetval;
                }
            }

            if(getGraphicAttr().IsTransparent())
            {
                
                const double fTransparency(basegfx::clamp(getGraphicAttr().GetTransparency() * (1.0 / 255.0), 0.0, 1.0));

                if(!basegfx::fTools::equalZero(fTransparency))
                {
                    const Primitive2DReference aUnifiedTransparence(
                        new UnifiedTransparencePrimitive2D(
                            aRetval,
                            fTransparency));

                    aRetval = Primitive2DSequence(&aUnifiedTransparence, 1);
                }
            }

            if(getGraphicAttr().IsCropped())
            {
                
                
                
                const basegfx::B2DVector aObjectScale(aTransform * basegfx::B2DVector(1.0, 1.0));
                const basegfx::B2DVector aCropScaleFactor(
                    rGraphicObject.calculateCropScaling(
                        aObjectScale.getX(),
                        aObjectScale.getY(),
                        getGraphicAttr().GetLeftCrop(),
                        getGraphicAttr().GetTopCrop(),
                        getGraphicAttr().GetRightCrop(),
                        getGraphicAttr().GetBottomCrop()));

                
                Primitive2DReference xPrimitive(
                    new CropPrimitive2D(
                        aRetval,
                        aTransform,
                        getGraphicAttr().GetLeftCrop() * aCropScaleFactor.getX(),
                        getGraphicAttr().GetTopCrop() * aCropScaleFactor.getY(),
                        getGraphicAttr().GetRightCrop() * aCropScaleFactor.getX(),
                        getGraphicAttr().GetBottomCrop() * aCropScaleFactor.getY()));

                aRetval = Primitive2DSequence(&xPrimitive, 1);
            }

            return aRetval;
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
                const GraphicPrimitive2D& rCompare = (GraphicPrimitive2D&)rPrimitive;

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

        
        ImplPrimitive2DIDBlock(GraphicPrimitive2D, PRIMITIVE2D_ID_GRAPHICPRIMITIVE2D)

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
