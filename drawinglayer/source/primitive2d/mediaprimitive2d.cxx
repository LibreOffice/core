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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive2d/mediaprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/polypolygonprimitive2d.hxx>
#include <avmedia/mediawindow.hxx>
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

            // try to get graphic snapshot
            const Graphic aGraphic(avmedia::MediaWindow::grabFrame(getURL(), true));

            if(GRAPHIC_BITMAP == aGraphic.GetType() || GRAPHIC_GDIMETAFILE == aGraphic.GetType())
            {
                const GraphicObject aGraphicObject(aGraphic);
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
            sal_uInt32 nDiscreteBorder)
        :   BufferedDecompositionPrimitive2D(),
            maTransform(rTransform),
            maURL(rURL),
            maBackgroundColor(rBackgroundColor),
            mnDiscreteBorder(nDiscreteBorder)
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

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
