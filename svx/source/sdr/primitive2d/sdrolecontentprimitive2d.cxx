/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

//////////////////////////////////////////////////////////////////////////////

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
                Graphic* pOLEGraphic = pSource->GetGraphic();

                if(pOLEGraphic)
                {
                    aGraphic = *pOLEGraphic;
                    bScaleContent = pSource->IsEmptyPresObj();
                }
            }

            if(GRAPHIC_NONE == aGraphic.GetType())
            {
                // no source, use fallback resource emty OLE graphic
                const Bitmap aEmptyOLEBitmap(SdrOle2Obj::GetEmtyOLEReplacementBitmap());
                aGraphic = Graphic(aEmptyOLEBitmap);
                bScaleContent = true;
            }

            if(GRAPHIC_NONE != aGraphic.GetType())
            {
                const GraphicObject aGraphicObject(aGraphic);
                const GraphicAttr aGraphicAttr;
                drawinglayer::primitive2d::Primitive2DSequence xOLEContent;

                if(bScaleContent)
                {
                    // get transformation atoms
                    basegfx::B2DVector aScale, aTranslate;
                    double fRotate, fShearX;
                    getObjectTransform().decompose(aScale, aTranslate, fRotate, fShearX);

                    // get PrefSize from the graphic in 100th mm
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
                        // if content fits into frame, create it
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
                    // create graphic primitive for content
                    const drawinglayer::primitive2d::Primitive2DReference aGraphicPrimitive(
                        new drawinglayer::primitive2d::GraphicPrimitive2D(
                            getObjectTransform(),
                            aGraphicObject,
                            aGraphicAttr));
                    drawinglayer::primitive2d::appendPrimitive2DReferenceToPrimitive2DSequence(aRetval, aGraphicPrimitive);
                }

                // a standard gray outline is created for scaled content
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

            // get graphic and check scale content state
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

                    // #i104867# to find out if the Graphic content of the
                    // OLE has changed, use GraphicVersion number
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

        // provide unique ID
        ImplPrimitrive2DIDBlock(SdrOleContentPrimitive2D, PRIMITIVE2D_ID_SDROLECONTENTPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
