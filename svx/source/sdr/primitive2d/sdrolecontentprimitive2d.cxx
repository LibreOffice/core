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

#include <sdr/primitive2d/sdrolecontentprimitive2d.hxx>
#include <svx/sdr/primitive2d/svx_primitivetypes2d.hxx>
#include <svx/svdoole2.hxx>
#include <utility>
#include <vcl/svapp.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#include <svtools/colorcfg.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>


namespace drawinglayer::primitive2d
{
        Primitive2DReference SdrOleContentPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*aViewInformation*/) const
        {
            rtl::Reference<SdrOle2Obj> pSource = mpSdrOle2Obj.get();
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
#ifdef _WIN32 // Little point in displaying the "broken OLE" graphic on OSes that don't have real OLE, maybe?
            if(GraphicType::NONE == aGraphic.GetType())
            {
                // no source, use fallback resource empty OLE graphic
                aGraphic = SdrOle2Obj::GetEmptyOLEReplacementGraphic();
                bScaleContent = true;
            }
#endif
            if(GraphicType::NONE == aGraphic.GetType())
                return nullptr;

            const GraphicObject aGraphicObject(aGraphic);
            const GraphicAttr aGraphicAttr;
            Primitive2DContainer aContainer;
            if(bScaleContent)
            {
                // get transformation atoms
                basegfx::B2DVector aScale, aTranslate;
                double fRotate, fShearX;
                getObjectTransform().decompose(aScale, aTranslate, fRotate, fShearX);

                // get PrefSize from the graphic in 100th mm
                Size aPrefSize(aGraphic.GetPrefSize());

                if(MapUnit::MapPixel == aGraphic.GetPrefMapMode().GetMapUnit())
                {
                    aPrefSize = Application::GetDefaultDevice()->PixelToLogic(aPrefSize, MapMode(MapUnit::Map100thMM));
                }
                else
                {
                    aPrefSize = OutputDevice::LogicToLogic(aPrefSize, aGraphic.GetPrefMapMode(), MapMode(MapUnit::Map100thMM));
                }

                const double fOffsetX((aScale.getX() - aPrefSize.getWidth()) / 2.0);
                const double fOffsetY((aScale.getY() - aPrefSize.getHeight()) / 2.0);

                if(fOffsetX >= 0.0 && fOffsetY >= 0.0)
                {
                    // if content fits into frame, create it
                    basegfx::B2DHomMatrix aInnerObjectMatrix(basegfx::utils::createScaleTranslateB2DHomMatrix(
                        aPrefSize.getWidth(), aPrefSize.getHeight(), fOffsetX, fOffsetY));
                    aInnerObjectMatrix = basegfx::utils::createShearXRotateTranslateB2DHomMatrix(fShearX, fRotate, aTranslate)
                        * aInnerObjectMatrix;

                    const drawinglayer::primitive2d::Primitive2DReference aGraphicPrimitive(
                        new drawinglayer::primitive2d::GraphicPrimitive2D(
                            aInnerObjectMatrix,
                            aGraphicObject,
                            aGraphicAttr));
                    aContainer.push_back(aGraphicPrimitive);
                }
            }
            else
            {
                // create graphic primitive for content
                const drawinglayer::primitive2d::Primitive2DReference aGraphicPrimitive(
                    new drawinglayer::primitive2d::GraphicPrimitive2D(
                        getObjectTransform(),
                        aGraphicObject,
                        aGraphicAttr));
                aContainer.push_back(aGraphicPrimitive);
            }

            // a standard gray outline is created for scaled content
            if(!bScaleContent)
                return new GroupPrimitive2D(std::move(aContainer));

            const svtools::ColorConfig aColorConfig;
            const svtools::ColorConfigValue aColor(aColorConfig.GetColorValue(svtools::OBJECTBOUNDARIES));

            if(aColor.bIsVisible)
            {
                basegfx::B2DPolygon aOutline(basegfx::utils::createUnitPolygon());
                const Color aVclColor(aColor.nColor);
                aOutline.transform(getObjectTransform());
                const drawinglayer::primitive2d::Primitive2DReference xOutline(
                    new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(std::move(aOutline), aVclColor.getBColor()));
                aContainer.push_back(xOutline);
            }
            return new GroupPrimitive2D(std::move(aContainer));
        }

        SdrOleContentPrimitive2D::SdrOleContentPrimitive2D(
            const SdrOle2Obj& rSdrOle2Obj,
            basegfx::B2DHomMatrix aObjectTransform,
            sal_uInt32 nGraphicVersion
        )
        :   mpSdrOle2Obj(const_cast< SdrOle2Obj* >(&rSdrOle2Obj)),
            maObjectTransform(std::move(aObjectTransform)),
            mnGraphicVersion(nGraphicVersion)
        {
        }

        bool SdrOleContentPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if( BufferedDecompositionPrimitive2D::operator==(rPrimitive) )
            {
                const SdrOleContentPrimitive2D& rCompare = static_cast<const SdrOleContentPrimitive2D&>(rPrimitive);
                auto xSdrThis = mpSdrOle2Obj.get();
                auto xSdrThat = rCompare.mpSdrOle2Obj.get();
                const bool bBothNot(!xSdrThis && !xSdrThat);
                const bool bBothAndEqual(xSdrThis && xSdrThat
                    && xSdrThis.get() == xSdrThat.get());

                return ((bBothNot || bBothAndEqual)
                    && getObjectTransform() == rCompare.getObjectTransform()

                    // #i104867# to find out if the Graphic content of the
                    // OLE has changed, use GraphicVersion number
                    && mnGraphicVersion == rCompare.mnGraphicVersion
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

        // provide unique ID
        sal_uInt32 SdrOleContentPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_SDROLECONTENTPRIMITIVE2D;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
