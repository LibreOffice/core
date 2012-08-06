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

#include <drawinglayer/primitive2d/mediaprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <svtools/grfmgr.hxx>
#include <drawinglayer/primitive2d/graphicprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/hiddengeometryprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence MediaPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& rViewInformation) const
        {
            Primitive2DSequence xRetval(1);

            // create background object
            basegfx::B2DPolygon aBackgroundPolygon(basegfx::tools::createUnitPolygon());
            aBackgroundPolygon.transform(getTransform());
            const Primitive2DReference xRefBackground(
                new PolyPolygonColorPrimitive2D(
                    basegfx::B2DPolyPolygon(aBackgroundPolygon),
                    getBackgroundColor()));
            xRetval[0] = xRefBackground;

            if(GRAPHIC_BITMAP == maSnapshot.GetType() || GRAPHIC_GDIMETAFILE == maSnapshot.GetType())
            {
                const GraphicObject aGraphicObject(maSnapshot);
                const GraphicAttr aGraphicAttr;
                xRetval.realloc(2);
                xRetval[0] = xRefBackground;
                xRetval[1] = Primitive2DReference(new GraphicPrimitive2D(getTransform(), aGraphicObject, aGraphicAttr));
            }

            if(getDiscreteBorder())
            {
                const basegfx::B2DVector aDiscreteInLogic(rViewInformation.getInverseObjectToViewTransformation() *
                    basegfx::B2DVector((double)getDiscreteBorder(), (double)getDiscreteBorder()));
                const double fDiscreteSize(aDiscreteInLogic.getX() + aDiscreteInLogic.getY());

                basegfx::B2DRange aSourceRange(0.0, 0.0, 1.0, 1.0);
                aSourceRange.transform(getTransform());

                basegfx::B2DRange aDestRange(aSourceRange);
                aDestRange.grow(-0.5 * fDiscreteSize);

                if(basegfx::fTools::equalZero(aDestRange.getWidth()) || basegfx::fTools::equalZero(aDestRange.getHeight()))
                {
                    // shrunk primitive has no content (zero size in X or Y), nothing to display. Still create
                    // invisible content for HitTest and BoundRect
                    const Primitive2DReference xHiddenLines(new HiddenGeometryPrimitive2D(xRetval));

                    xRetval = Primitive2DSequence(&xHiddenLines, 1);
                }
                else
                {
                    // create transformation matrix from original range to shrunk range
                    basegfx::B2DHomMatrix aTransform;
                    aTransform.translate(-aSourceRange.getMinX(), -aSourceRange.getMinY());
                    aTransform.scale(aDestRange.getWidth() / aSourceRange.getWidth(), aDestRange.getHeight() / aSourceRange.getHeight());
                    aTransform.translate(aDestRange.getMinX(), aDestRange.getMinY());

                    // add transform primitive
                    const Primitive2DReference aScaled(new TransformPrimitive2D(aTransform, xRetval));
                    xRetval = Primitive2DSequence(&aScaled, 1L);
                }
            }

            return xRetval;
        }

        MediaPrimitive2D::MediaPrimitive2D(
            const basegfx::B2DHomMatrix& rTransform,
            const rtl::OUString& rURL,
            const basegfx::BColor& rBackgroundColor,
            sal_uInt32 nDiscreteBorder,
            const Graphic &rSnapshot)
        :   BufferedDecompositionPrimitive2D(),
            maTransform(rTransform),
            maURL(rURL),
            maBackgroundColor(rBackgroundColor),
            mnDiscreteBorder(nDiscreteBorder),
            maSnapshot(rSnapshot)
        {
        }

        bool MediaPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(BufferedDecompositionPrimitive2D::operator==(rPrimitive))
            {
                const MediaPrimitive2D& rCompare = (MediaPrimitive2D&)rPrimitive;

                return (getTransform() == rCompare.getTransform()
                    && getURL() == rCompare.getURL()
                    && getBackgroundColor() == rCompare.getBackgroundColor()
                    && getDiscreteBorder() == rCompare.getDiscreteBorder());
            }

            return false;
        }

        basegfx::B2DRange MediaPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            basegfx::B2DRange aRetval(0.0, 0.0, 1.0, 1.0);
            aRetval.transform(getTransform());

            if(getDiscreteBorder())
            {
                const basegfx::B2DVector aDiscreteInLogic(rViewInformation.getInverseObjectToViewTransformation() *
                    basegfx::B2DVector((double)getDiscreteBorder(), (double)getDiscreteBorder()));
                const double fDiscreteSize(aDiscreteInLogic.getX() + aDiscreteInLogic.getY());

                aRetval.grow(-0.5 * fDiscreteSize);
            }

            return aRetval;
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(MediaPrimitive2D, PRIMITIVE2D_ID_MEDIAPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
