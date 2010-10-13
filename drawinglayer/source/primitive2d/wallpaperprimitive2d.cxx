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

#include <drawinglayer/primitive2d/wallpaperprimitive2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/fillbitmapprimitive2d.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <drawinglayer/primitive2d/maskprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence WallpaperBitmapPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence aRetval;

            if(!getLocalObjectRange().isEmpty() && !getBitmapEx().IsEmpty())
            {
                // get bitmap PIXEL size
                const Size& rPixelSize = getBitmapEx().GetSizePixel();

                if(rPixelSize.Width() > 0 && rPixelSize.Height() > 0)
                {
                    if(WALLPAPER_SCALE == getWallpaperStyle())
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

                        aRetval = Primitive2DSequence(&xReference, 1);
                    }
                    else
                    {
                        // transform to logic size
                        basegfx::B2DHomMatrix aInverseViewTransformation(getViewTransformation());
                        aInverseViewTransformation.invert();
                        basegfx::B2DVector aLogicSize(rPixelSize.Width(), rPixelSize.Height());
                        aLogicSize = aInverseViewTransformation * aLogicSize;

                        // apply laout
                        basegfx::B2DPoint aTargetTopLeft(getLocalObjectRange().getMinimum());
                        bool bUseTargetTopLeft(true);
                        bool bNeedsClipping(false);

                        switch(getWallpaperStyle())
                        {
                            default: //case WALLPAPER_TILE :, also WALLPAPER_NULL and WALLPAPER_APPLICATIONGRADIENT
                            {
                                bUseTargetTopLeft = false;
                                break;
                            }
                            case WALLPAPER_SCALE :
                            {
                                // handled by shortcut above
                                break;
                            }
                            case WALLPAPER_TOPLEFT :
                            {
                                // nothing to do
                                break;
                            }
                            case WALLPAPER_TOP :
                            {
                                const basegfx::B2DPoint aCenter(getLocalObjectRange().getCenter());
                                aTargetTopLeft.setX(aCenter.getX() - (aLogicSize.getX() * 0.5));
                                break;
                            }
                            case WALLPAPER_TOPRIGHT :
                            {
                                aTargetTopLeft.setX(getLocalObjectRange().getMaxX() - aLogicSize.getX());
                                break;
                            }
                            case WALLPAPER_LEFT :
                            {
                                const basegfx::B2DPoint aCenter(getLocalObjectRange().getCenter());
                                aTargetTopLeft.setY(aCenter.getY() - (aLogicSize.getY() * 0.5));
                                break;
                            }
                            case WALLPAPER_CENTER :
                            {
                                const basegfx::B2DPoint aCenter(getLocalObjectRange().getCenter());
                                aTargetTopLeft = aCenter - (aLogicSize * 0.5);
                                break;
                            }
                            case WALLPAPER_RIGHT :
                            {
                                const basegfx::B2DPoint aCenter(getLocalObjectRange().getCenter());
                                aTargetTopLeft.setX(getLocalObjectRange().getMaxX() - aLogicSize.getX());
                                aTargetTopLeft.setY(aCenter.getY() - (aLogicSize.getY() * 0.5));
                                break;
                            }
                            case WALLPAPER_BOTTOMLEFT :
                            {
                                aTargetTopLeft.setY(getLocalObjectRange().getMaxY() - aLogicSize.getY());
                                break;
                            }
                            case WALLPAPER_BOTTOM :
                            {
                                const basegfx::B2DPoint aCenter(getLocalObjectRange().getCenter());
                                aTargetTopLeft.setX(aCenter.getX() - (aLogicSize.getX() * 0.5));
                                aTargetTopLeft.setY(getLocalObjectRange().getMaxY() - aLogicSize.getY());
                                break;
                            }
                            case WALLPAPER_BOTTOMRIGHT :
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
                            aRetval = Primitive2DSequence(&xReference, 1);

                            // clip when not completely inside object range
                            bNeedsClipping = !getLocalObjectRange().isInside(aTargetRange);
                        }
                        else
                        {
                            // WALLPAPER_TILE, WALLPAPER_NULL, WALLPAPER_APPLICATIONGRADIENT
                            // convert to relative positions
                            const basegfx::B2DVector aRelativeSize(
                                aLogicSize.getX() / (getLocalObjectRange().getWidth() ? getLocalObjectRange().getWidth() : 1.0),
                                aLogicSize.getY() / (getLocalObjectRange().getHeight() ? getLocalObjectRange().getHeight() : 1.0));
                            basegfx::B2DPoint aRelativeTopLeft(0.0, 0.0);

                            if(WALLPAPER_TILE != getWallpaperStyle())
                            {
                                aRelativeTopLeft.setX(0.5 - aRelativeSize.getX());
                                aRelativeTopLeft.setY(0.5 - aRelativeSize.getY());
                            }

                            // prepare FillBitmapAttribute
                            const attribute::FillBitmapAttribute aFillBitmapAttribute(
                                getBitmapEx(),
                                aRelativeTopLeft,
                                aRelativeSize,
                                true);

                            // create ObjectTransform
                            basegfx::B2DHomMatrix aObjectTransform;

                            aObjectTransform.set(0, 0, getLocalObjectRange().getWidth());
                            aObjectTransform.set(1, 1, getLocalObjectRange().getHeight());
                            aObjectTransform.set(0, 2, getLocalObjectRange().getMinX());
                            aObjectTransform.set(1, 2, getLocalObjectRange().getMinY());

                            // create FillBitmapPrimitive
                            const drawinglayer::primitive2d::Primitive2DReference xFillBitmap(
                                new drawinglayer::primitive2d::FillBitmapPrimitive2D(
                                    aObjectTransform,
                                    aFillBitmapAttribute));
                            aRetval = Primitive2DSequence(&xFillBitmap, 1);

                            // always embed tiled fill to clipping
                            bNeedsClipping = true;
                        }

                        if(bNeedsClipping)
                        {
                            // embed to clipping; this is necessary for tiled fills
                            const basegfx::B2DPolyPolygon aPolyPolygon(
                                basegfx::tools::createPolygonFromRect(getLocalObjectRange()));
                            const drawinglayer::primitive2d::Primitive2DReference xClippedFill(
                                new drawinglayer::primitive2d::MaskPrimitive2D(
                                    aPolyPolygon,
                                    aRetval));
                            aRetval = Primitive2DSequence(&xClippedFill, 1);
                        }
                    }
                }
            }

            return aRetval;
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
                const WallpaperBitmapPrimitive2D& rCompare = (WallpaperBitmapPrimitive2D&)rPrimitive;

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
        ImplPrimitrive2DIDBlock(WallpaperBitmapPrimitive2D, PRIMITIVE2D_ID_WALLPAPERBITMAPPRIMITIVE2D)
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
