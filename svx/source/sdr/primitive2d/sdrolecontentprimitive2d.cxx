/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svx/sdr/primitive2d/sdrolecontentprimitive2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <svx/svdoole2.hxx>
#include <vcl/svapp.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <svtools/colorcfg.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>



namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence SdrOleContentPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            Primitive2DSequence aRetval;
            const SdrOle2Obj* pSource = (mpSdrOle2Obj.is() ? static_cast< SdrOle2Obj* >(mpSdrOle2Obj.get()) : 0);
            bool bScaleContent(false);
            Graphic aGraphic;

            if(pSource)
            {
                const Graphic* pOLEGraphic = pSource->GetGraphic();

                if(pOLEGraphic)
                {
                    aGraphic = *pOLEGraphic;
                    bScaleContent = pSource->IsEmptyPresObj();
                }
            }
#ifdef WNT 
            if(GRAPHIC_NONE == aGraphic.GetType())
            {
                
                aGraphic = SdrOle2Obj::GetEmptyOLEReplacementGraphic();
                bScaleContent = true;
            }
#endif
            if(GRAPHIC_NONE != aGraphic.GetType())
            {
                const GraphicObject aGraphicObject(aGraphic);
                const GraphicAttr aGraphicAttr;

                if(bScaleContent)
                {
                    
                    basegfx::B2DVector aScale, aTranslate;
                    double fRotate, fShearX;
                    getObjectTransform().decompose(aScale, aTranslate, fRotate, fShearX);

                    
                    Size aPrefSize(aGraphic.GetPrefSize());

                    if(MAP_PIXEL == aGraphic.GetPrefMapMode().GetMapUnit())
                    {
                        aPrefSize = Application::GetDefaultDevice()->PixelToLogic(aPrefSize, MAP_100TH_MM);
                    }
                    else
                    {
                        aPrefSize = Application::GetDefaultDevice()->LogicToLogic(aPrefSize, aGraphic.GetPrefMapMode(), MAP_100TH_MM);
                    }

                    const double fOffsetX((aScale.getX() - aPrefSize.getWidth()) / 2.0);
                    const double fOffsetY((aScale.getY() - aPrefSize.getHeight()) / 2.0);

                    if(basegfx::fTools::moreOrEqual(fOffsetX, 0.0) && basegfx::fTools::moreOrEqual(fOffsetY, 0.0))
                    {
                        
                        basegfx::B2DHomMatrix aInnerObjectMatrix(basegfx::tools::createScaleTranslateB2DHomMatrix(
                            aPrefSize.getWidth(), aPrefSize.getHeight(), fOffsetX, fOffsetY));
                        aInnerObjectMatrix = basegfx::tools::createShearXRotateTranslateB2DHomMatrix(fShearX, fRotate, aTranslate)
                            * aInnerObjectMatrix;

                        const drawinglayer::primitive2d::Primitive2DReference aGraphicPrimitive(
                            new drawinglayer::primitive2d::GraphicPrimitive2D(
                                aInnerObjectMatrix,
                                aGraphicObject,
                                aGraphicAttr));
                        drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, aGraphicPrimitive);
                    }
                }
                else
                {
                    
                    const drawinglayer::primitive2d::Primitive2DReference aGraphicPrimitive(
                        new drawinglayer::primitive2d::GraphicPrimitive2D(
                            getObjectTransform(),
                            aGraphicObject,
                            aGraphicAttr));
                    drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, aGraphicPrimitive);
                }

                
                if(bScaleContent)
                {
                    const svtools::ColorConfig aColorConfig;
                    const svtools::ColorConfigValue aColor(aColorConfig.GetColorValue(svtools::OBJECTBOUNDARIES));

                    if(aColor.bIsVisible)
                    {
                        basegfx::B2DPolygon aOutline(basegfx::tools::createUnitPolygon());
                        const Color aVclColor(aColor.nColor);
                        aOutline.transform(getObjectTransform());
                        const drawinglayer::primitive2d::Primitive2DReference xOutline(
                            new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(aOutline, aVclColor.getBColor()));
                        drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, xOutline);
                    }
                }
            }

            
            return aRetval;
        }

        SdrOleContentPrimitive2D::SdrOleContentPrimitive2D(
            const SdrOle2Obj& rSdrOle2Obj,
            const basegfx::B2DHomMatrix& rObjectTransform,
            sal_uInt32 nGraphicVersion
        )
        :   BufferedDecompositionPrimitive2D(),
            mpSdrOle2Obj(const_cast< SdrOle2Obj* >(&rSdrOle2Obj)),
            maObjectTransform(rObjectTransform),
            mnGraphicVersion(nGraphicVersion)
        {
        }

        bool SdrOleContentPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if( BufferedDecompositionPrimitive2D::operator==(rPrimitive) )
            {
                const SdrOleContentPrimitive2D& rCompare = (SdrOleContentPrimitive2D&)rPrimitive;
                const bool bBothNot(!mpSdrOle2Obj.is() && !rCompare.mpSdrOle2Obj.is());
                const bool bBothAndEqual(mpSdrOle2Obj.is() && rCompare.mpSdrOle2Obj.is()
                    && mpSdrOle2Obj.get() == rCompare.mpSdrOle2Obj.get());

                return ((bBothNot || bBothAndEqual)
                    && getObjectTransform() == rCompare.getObjectTransform()

                    
                    
                    && getGraphicVersion() == rCompare.getGraphicVersion()
                );
            }

            return false;
        }

        basegfx::B2DRange SdrOleContentPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            basegfx::B2DRange aRange(0.0, 0.0, 1.0, 1.0);
            aRange.transform(getObjectTransform());

            return aRange;
        }

        
        ImplPrimitive2DIDBlock(SdrOleContentPrimitive2D, PRIMITIVE2D_ID_SDROLECONTENTPRIMITIVE2D)

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
