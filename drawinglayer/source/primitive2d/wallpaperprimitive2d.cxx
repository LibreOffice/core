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

#include <drawinglayer/primitive2d/wallpaperprimitive2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/fillgraphicprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <vcl/graph.hxx>


namespace drawinglayer
{
    namespace primitive2d
    {
        void WallpaperBitmapPrimitive2D::create2DDecomposition(Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DReference aRetval;

            if(!getLocalObjectRange().isEmpty() && !getBitmapEx().IsEmpty())
            {
                // get bitmap PIXEL size
                const Size& rPixelSize = getBitmapEx().GetSizePixel();

                if(rPixelSize.Width() > 0 && rPixelSize.Height() > 0)
                {
                    if(WallpaperStyle::Scale == getWallpaperStyle())
                    {
                        // shortcut for scale; use simple BitmapPrimitive2D
                        basegfx::B2DHomMatrix aObjectTransform;

                        aObjectTransform.set(0, 0, getLocalObjectRange().getWidth());
                        aObjectTransform.set(1, 1, getLocalObjectRange().getHeight());
                        aObjectTransform.set(0, 2, getLocalObjectRange().getMinX());
                        aObjectTransform.set(1, 2, getLocalObjectRange().getMinY());

                        Primitive2DReference xReference(
                            new BitmapPrimitive2D(
                                getBitmapEx(),
                                aObjectTransform));

                        aRetval = xReference;
                    }
                    else
                    {
                        // transform to logic size
                        basegfx::B2DHomMatrix aInverseViewTransformation(getViewTransformation());
                        aInverseViewTransformation.invert();
                        basegfx::B2DVector aLogicSize(rPixelSize.Width(), rPixelSize.Height());
                        aLogicSize = aInverseViewTransformation * aLogicSize;

                        // apply layout
                        basegfx::B2DPoint aTargetTopLeft(getLocalObjectRange().getMinimum());
                        bool bUseTargetTopLeft(true);
                        bool bNeedsClipping(false);

                        switch(getWallpaperStyle())
                        {
                            default: //case WallpaperStyle::Tile :, also WallpaperStyle::NONE and WallpaperStyle::ApplicationGradient
                            {
                                bUseTargetTopLeft = false;
                                break;
                            }
                            case WallpaperStyle::Scale :
                            {
                                // handled by shortcut above
                                break;
                            }
                            case WallpaperStyle::TopLeft :
                            {
                                // nothing to do
                                break;
                            }
                            case WallpaperStyle::Top :
                            {
                                const basegfx::B2DPoint aCenter(getLocalObjectRange().getCenter());
                                aTargetTopLeft.setX(aCenter.getX() - (aLogicSize.getX() * 0.5));
                                break;
                            }
                            case WallpaperStyle::TopRight :
                            {
                                aTargetTopLeft.setX(getLocalObjectRange().getMaxX() - aLogicSize.getX());
                                break;
                            }
                            case WallpaperStyle::Left :
                            {
                                const basegfx::B2DPoint aCenter(getLocalObjectRange().getCenter());
                                aTargetTopLeft.setY(aCenter.getY() - (aLogicSize.getY() * 0.5));
                                break;
                            }
                            case WallpaperStyle::Center :
                            {
                                const basegfx::B2DPoint aCenter(getLocalObjectRange().getCenter());
                                aTargetTopLeft = aCenter - (aLogicSize * 0.5);
                                break;
                            }
                            case WallpaperStyle::Right :
                            {
                                const basegfx::B2DPoint aCenter(getLocalObjectRange().getCenter());
                                aTargetTopLeft.setX(getLocalObjectRange().getMaxX() - aLogicSize.getX());
                                aTargetTopLeft.setY(aCenter.getY() - (aLogicSize.getY() * 0.5));
                                break;
                            }
                            case WallpaperStyle::BottomLeft :
                            {
                                aTargetTopLeft.setY(getLocalObjectRange().getMaxY() - aLogicSize.getY());
                                break;
                            }
                            case WallpaperStyle::Bottom :
                            {
                                const basegfx::B2DPoint aCenter(getLocalObjectRange().getCenter());
                                aTargetTopLeft.setX(aCenter.getX() - (aLogicSize.getX() * 0.5));
                                aTargetTopLeft.setY(getLocalObjectRange().getMaxY() - aLogicSize.getY());
                                break;
                            }
                            case WallpaperStyle::BottomRight :
                            {
                                aTargetTopLeft = getLocalObjectRange().getMaximum() - aLogicSize;
                                break;
                            }
                        }

                        if(bUseTargetTopLeft)
                        {
                            // fill target range
                            const basegfx::B2DRange aTargetRange(aTargetTopLeft, aTargetTopLeft + aLogicSize);

                            // create aligned, single BitmapPrimitive2D
                            basegfx::B2DHomMatrix aObjectTransform;

                            aObjectTransform.set(0, 0, aTargetRange.getWidth());
                            aObjectTransform.set(1, 1, aTargetRange.getHeight());
                            aObjectTransform.set(0, 2, aTargetRange.getMinX());
                            aObjectTransform.set(1, 2, aTargetRange.getMinY());

                            Primitive2DReference xReference(
                                new BitmapPrimitive2D(
                                    getBitmapEx(),
                                    aObjectTransform));
                            aRetval = xReference;

                            // clip when not completely inside object range
                            bNeedsClipping = !getLocalObjectRange().isInside(aTargetRange);
                        }
                        else
                        {
                            // WallpaperStyle::Tile, WallpaperStyle::NONE, WallpaperStyle::ApplicationGradient
                            // convert to relative positions
                            const basegfx::B2DVector aRelativeSize(
                                aLogicSize.getX() / (getLocalObjectRange().getWidth() ? getLocalObjectRange().getWidth() : 1.0),
                                aLogicSize.getY() / (getLocalObjectRange().getHeight() ? getLocalObjectRange().getHeight() : 1.0));
                            basegfx::B2DPoint aRelativeTopLeft(0.0, 0.0);

                            if(WallpaperStyle::Tile != getWallpaperStyle())
                            {
                                aRelativeTopLeft.setX(0.5 - aRelativeSize.getX());
                                aRelativeTopLeft.setY(0.5 - aRelativeSize.getY());
                            }

                            // prepare FillGraphicAttribute
                            const attribute::FillGraphicAttribute aFillGraphicAttribute(
                                Graphic(getBitmapEx()),
                                basegfx::B2DRange(aRelativeTopLeft, aRelativeTopLeft+ aRelativeSize),
                                true);

                            // create ObjectTransform
                            const basegfx::B2DHomMatrix aObjectTransform(
                                basegfx::utils::createScaleTranslateB2DHomMatrix(
                                    getLocalObjectRange().getRange(),
                                    getLocalObjectRange().getMinimum()));

                            // create FillBitmapPrimitive
                            const drawinglayer::primitive2d::Primitive2DReference xFillBitmap(
                                new drawinglayer::primitive2d::FillGraphicPrimitive2D(
                                    aObjectTransform,
                                    aFillGraphicAttribute));
                            aRetval = xFillBitmap;

                            // always embed tiled fill to clipping
                            bNeedsClipping = true;
                        }

                        if(bNeedsClipping)
                        {
                            // embed to clipping; this is necessary for tiled fills
                            const basegfx::B2DPolyPolygon aPolyPolygon(
                                basegfx::utils::createPolygonFromRect(getLocalObjectRange()));
                            const drawinglayer::primitive2d::Primitive2DReference xClippedFill(
                                new drawinglayer::primitive2d::MaskPrimitive2D(
                                    aPolyPolygon,
                                    { aRetval }));
                            aRetval = xClippedFill;
                        }
                    }
                }
            }

            if (aRetval.is())
                rContainer.push_back(aRetval);
        }

        WallpaperBitmapPrimitive2D::WallpaperBitmapPrimitive2D(
            const basegfx::B2DRange& rObjectRange,
            const BitmapEx& rBitmapEx,
            WallpaperStyle eWallpaperStyle)
        :   ViewTransformationDependentPrimitive2D(),
            maObjectRange(rObjectRange),
            maBitmapEx(rBitmapEx),
            meWallpaperStyle(eWallpaperStyle)
        {
        }

        bool WallpaperBitmapPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(ViewTransformationDependentPrimitive2D::operator==(rPrimitive))
            {
                const WallpaperBitmapPrimitive2D& rCompare = static_cast<const WallpaperBitmapPrimitive2D&>(rPrimitive);

                return (getLocalObjectRange() == rCompare.getLocalObjectRange()
                    && getBitmapEx() == rCompare.getBitmapEx()
                    && getWallpaperStyle() == rCompare.getWallpaperStyle());
            }

            return false;
        }

        basegfx::B2DRange WallpaperBitmapPrimitive2D::getB2DRange(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            return getLocalObjectRange();
        }

        // provide unique ID
        ImplPrimitive2DIDBlock(WallpaperBitmapPrimitive2D, PRIMITIVE2D_ID_WALLPAPERBITMAPPRIMITIVE2D)
    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
