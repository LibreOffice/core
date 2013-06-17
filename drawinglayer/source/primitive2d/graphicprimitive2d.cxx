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

            if(255L != getGraphicAttr().GetTransparency())
            {
                // do not apply mirroring from GraphicAttr to the Metafile by calling
                // GetTransformedGraphic, this will try to mirror the Metafile using Scale()
                // at the Metafile. This again calls Scale at the single MetaFile actions,
                // but this implementation never worked. I reworked that implementations,
                // but for security reasons i will try not to use it.
                basegfx::B2DHomMatrix aTransform(getTransform());

                if(getGraphicAttr().IsMirrored())
                {
                    // content needs mirroring
                    const bool bHMirr(getGraphicAttr().GetMirrorFlags() & BMP_MIRROR_HORZ);
                    const bool bVMirr(getGraphicAttr().GetMirrorFlags() & BMP_MIRROR_VERT);

                    // mirror by applying negative scale to the unit primitive and
                    // applying the object transformation on it.
                    aTransform = basegfx::tools::createScaleB2DHomMatrix(
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
                GraphicAttr aSuppressGraphicAttr(getGraphicAttr());
                aSuppressGraphicAttr.SetCrop(0, 0, 0, 0);
                aSuppressGraphicAttr.SetRotation(0);
                aSuppressGraphicAttr.SetMirrorFlags(0);

                const GraphicObject& rGraphicObject = getGraphicObject();
                const Graphic aTransformedGraphic(rGraphicObject.GetTransformedGraphic(&aSuppressGraphicAttr));

                aRetval = create2DDecompositionOfGraphic(
                    aTransformedGraphic,
                    aTransform);

                if(aRetval.getLength())
                {
                    // check for cropping
                    if(getGraphicAttr().IsCropped())
                    {
                        // calculate scalings between real image size and logic object size. This
                        // is necessary since the crop values are relative to original bitmap size
                        double fFactorX(1.0);
                        double fFactorY(1.0);

                        {
                            const MapMode aMapMode100thmm(MAP_100TH_MM);
                            Size aBitmapSize(rGraphicObject.GetPrefSize());

                            // #i95968# better support PrefMapMode; special for MAP_PIXEL was missing
                            if(MAP_PIXEL == rGraphicObject.GetPrefMapMode().GetMapUnit())
                            {
                                aBitmapSize = Application::GetDefaultDevice()->PixelToLogic(aBitmapSize, aMapMode100thmm);
                            }
                            else
                            {
                                aBitmapSize = Application::GetDefaultDevice()->LogicToLogic(aBitmapSize, rGraphicObject.GetPrefMapMode(), aMapMode100thmm);
                            }

                            const double fDivX(aBitmapSize.Width() - getGraphicAttr().GetLeftCrop() - getGraphicAttr().GetRightCrop());
                            const double fDivY(aBitmapSize.Height() - getGraphicAttr().GetTopCrop() - getGraphicAttr().GetBottomCrop());
                            const basegfx::B2DVector aScale(aTransform * basegfx::B2DVector(1.0, 1.0));

                            if(!basegfx::fTools::equalZero(fDivX))
                            {
                                fFactorX = fabs(aScale.getX()) / fDivX;
                            }

                            if(!basegfx::fTools::equalZero(fDivY))
                            {
                                fFactorY = fabs(aScale.getY()) / fDivY;
                            }
                        }

                        // embed content in cropPrimitive
                        Primitive2DReference xPrimitive(
                            new CropPrimitive2D(
                                aRetval,
                                aTransform,
                                getGraphicAttr().GetLeftCrop() * fFactorX,
                                getGraphicAttr().GetTopCrop() * fFactorY,
                                getGraphicAttr().GetRightCrop() * fFactorX,
                                getGraphicAttr().GetBottomCrop() * fFactorY));

                        aRetval = Primitive2DSequence(&xPrimitive, 1);
                    }
                }
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

        // provide unique ID
        ImplPrimitive2DIDBlock(GraphicPrimitive2D, PRIMITIVE2D_ID_GRAPHICPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
