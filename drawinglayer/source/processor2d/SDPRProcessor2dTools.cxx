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

#include <drawinglayer/processor2d/SDPRProcessor2dTools.hxx>
#include <drawinglayer/primitive2d/fillgraphicprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/graph.hxx>
#include <basegfx/range/b2drange.hxx>

#ifdef DBG_UTIL
#include <tools/stream.hxx>
#include <vcl/filter/PngImageWriter.hxx>
#endif

namespace drawinglayer::processor2d
{
void setOffsetXYCreatedBitmap(
    drawinglayer::primitive2d::FillGraphicPrimitive2D& rFillGraphicPrimitive2D,
    const BitmapEx& rBitmap)
{
    rFillGraphicPrimitive2D.impSetOffsetXYCreatedBitmap(rBitmap);
}

void takeCareOfOffsetXY(
    const drawinglayer::primitive2d::FillGraphicPrimitive2D& rFillGraphicPrimitive2D,
    BitmapEx& rTarget, basegfx::B2DRange& rFillUnitRange)
{
    const attribute::FillGraphicAttribute& rFillGraphicAttribute(
        rFillGraphicPrimitive2D.getFillGraphic());
    const bool bOffsetXIsUsed(rFillGraphicAttribute.getOffsetX() > 0.0
                              && rFillGraphicAttribute.getOffsetX() < 1.0);
    const bool bOffsetYIsUsed(rFillGraphicAttribute.getOffsetY() > 0.0
                              && rFillGraphicAttribute.getOffsetY() < 1.0);

    if (bOffsetXIsUsed)
    {
        if (rFillGraphicPrimitive2D.getOffsetXYCreatedBitmap().IsEmpty())
        {
            const Size& rSize(rTarget.GetSizePixel());
            const tools::Long w(rSize.Width());
            const tools::Long a(
                basegfx::fround<tools::Long>(w * (1.0 - rFillGraphicAttribute.getOffsetX())));

            if (0 != a && w != a)
            {
                const tools::Long h(rSize.Height());
                const tools::Long b(w - a);
                BitmapEx aTarget(Size(w, h * 2), rTarget.getPixelFormat());

                aTarget.SetPrefSize(
                    Size(rTarget.GetPrefSize().Width(), rTarget.GetPrefSize().Height() * 2));
                const tools::Rectangle aSrcDst(Point(), rSize);
                aTarget.CopyPixel(aSrcDst, // Dst
                                  aSrcDst, // Src
                                  rTarget);
                const Size aSizeA(b, h);
                aTarget.CopyPixel(tools::Rectangle(Point(0, h), aSizeA), // Dst
                                  tools::Rectangle(Point(a, 0), aSizeA), // Src
                                  rTarget);
                const Size aSizeB(a, h);
                aTarget.CopyPixel(tools::Rectangle(Point(b, h), aSizeB), // Dst
                                  tools::Rectangle(Point(), aSizeB), // Src
                                  rTarget);

                setOffsetXYCreatedBitmap(
                    const_cast<drawinglayer::primitive2d::FillGraphicPrimitive2D&>(
                        rFillGraphicPrimitive2D),
                    aTarget);
            }
        }

        if (!rFillGraphicPrimitive2D.getOffsetXYCreatedBitmap().IsEmpty())
        {
            rTarget = rFillGraphicPrimitive2D.getOffsetXYCreatedBitmap();
            rFillUnitRange.expand(basegfx::B2DPoint(
                rFillUnitRange.getMinX(), rFillUnitRange.getMaxY() + rFillUnitRange.getHeight()));
        }
    }
    else if (bOffsetYIsUsed)
    {
        if (rFillGraphicPrimitive2D.getOffsetXYCreatedBitmap().IsEmpty())
        {
            const Size& rSize(rTarget.GetSizePixel());
            const tools::Long h(rSize.Height());
            const tools::Long a(
                basegfx::fround<tools::Long>(h * (1.0 - rFillGraphicAttribute.getOffsetY())));

            if (0 != a && h != a)
            {
                const tools::Long w(rSize.Width());
                const tools::Long b(h - a);
                BitmapEx aTarget(Size(w * 2, h), rTarget.getPixelFormat());

                aTarget.SetPrefSize(
                    Size(rTarget.GetPrefSize().Width() * 2, rTarget.GetPrefSize().Height()));
                const tools::Rectangle aSrcDst(Point(), rSize);
                aTarget.CopyPixel(aSrcDst, // Dst
                                  aSrcDst, // Src
                                  rTarget);
                const Size aSizeA(w, b);
                aTarget.CopyPixel(tools::Rectangle(Point(w, 0), aSizeA), // Dst
                                  tools::Rectangle(Point(0, a), aSizeA), // Src
                                  rTarget);
                const Size aSizeB(w, a);
                aTarget.CopyPixel(tools::Rectangle(Point(w, b), aSizeB), // Dst
                                  tools::Rectangle(Point(), aSizeB), // Src
                                  rTarget);

                setOffsetXYCreatedBitmap(
                    const_cast<drawinglayer::primitive2d::FillGraphicPrimitive2D&>(
                        rFillGraphicPrimitive2D),
                    aTarget);
            }
        }

        if (!rFillGraphicPrimitive2D.getOffsetXYCreatedBitmap().IsEmpty())
        {
            rTarget = rFillGraphicPrimitive2D.getOffsetXYCreatedBitmap();
            rFillUnitRange.expand(basegfx::B2DPoint(
                rFillUnitRange.getMaxX() + rFillUnitRange.getWidth(), rFillUnitRange.getMinY()));
        }
    }
}

bool prepareBitmapForDirectRender(
    const drawinglayer::primitive2d::FillGraphicPrimitive2D& rFillGraphicPrimitive2D,
    const drawinglayer::geometry::ViewInformation2D& rViewInformation2D, BitmapEx& rTarget,
    basegfx::B2DRange& rFillUnitRange, double fBigDiscreteArea)
{
    const attribute::FillGraphicAttribute& rFillGraphicAttribute(
        rFillGraphicPrimitive2D.getFillGraphic());
    const Graphic& rGraphic(rFillGraphicAttribute.getGraphic());

    if (rFillGraphicAttribute.isDefault() || rGraphic.IsNone())
    {
        // default attributes or GraphicType::NONE, so no fill .-> done
        return false;
    }

    if (!rFillGraphicAttribute.getTiling())
    {
        // If no tiling used, the Graphic will need to be painted just once. This
        // is perfectly done using the decomposition, so use it.
        // What we want to do here is to optimize tiled paint, for two reasons:
        // (a) speed: draw one tile, repeat -> obvious
        // (b) correctness: not so obvious, but since in AAed paint the same edge
        //     of touching polygons both AAed do *not* sum up, but get blended by
        //     multiplication (0.5 * 0.5 -> 0.25) the connection will stay visible,
        //     not only with filled polygons, but also with bitmaps
        // Signal that paint is needed
        return true;
    }

    if (rFillUnitRange.isEmpty())
    {
        // no fill range definition, no fill, done
        return false;
    }

    const basegfx::B2DHomMatrix aLocalTransform(rViewInformation2D.getObjectToViewTransformation()
                                                * rFillGraphicPrimitive2D.getTransformation());
    const basegfx::B2DRange& rDiscreteViewPort(rViewInformation2D.getDiscreteViewport());

    if (!rDiscreteViewPort.isEmpty())
    {
        // calculate discrete covered pixel area
        basegfx::B2DRange aDiscreteRange(basegfx::B2DRange::getUnitB2DRange());
        aDiscreteRange.transform(aLocalTransform);

        if (!rDiscreteViewPort.overlaps(rDiscreteViewPort))
        {
            // we have a Viewport and visible range of geometry is outside -> not visible, done
            return false;
        }
    }

    if (GraphicType::Bitmap == rGraphic.GetType() && rGraphic.IsAnimated())
    {
        // Need to prepare specialized AnimatedGraphicPrimitive2D,
        // cannot handle here. Signal that paint is needed
        return true;
    }

    if (GraphicType::Bitmap == rGraphic.GetType() && !rGraphic.getVectorGraphicData())
    {
        // bitmap graphic, always handle locally, so get bitmap data independent
        // if it'sie or it's discrete display size
        rTarget = rGraphic.GetBitmapEx();
    }
    else
    {
        // Vector Graphic Data fill, including metafile:
        // We can know about discrete pixel size here, calculate and use it.
        // To do so, using Vectors is sufficient to get the lengths. It is
        // not necessary to transform the whole target coordinate system.
        const basegfx::B2DVector aDiscreteXAxis(
            aLocalTransform
            * basegfx::B2DVector(rFillUnitRange.getMaxX() - rFillUnitRange.getMinX(), 0.0));
        const basegfx::B2DVector aDiscreteYAxis(
            aLocalTransform
            * basegfx::B2DVector(0.0, rFillUnitRange.getMaxY() - rFillUnitRange.getMinY()));

        // get and ensure minimal size
        const double fDiscreteWidth(std::max(1.0, aDiscreteXAxis.getLength()));
        const double fDiscreteHeight(std::max(1.0, aDiscreteYAxis.getLength()));

        // compare with a big visualization size in discrete pixels
        const double fTargetDiscreteArea(fDiscreteWidth * fDiscreteHeight);

        if (fTargetDiscreteArea > fBigDiscreteArea)
        {
            // When the vector data is visualized big it is better to not handle here
            // but use decomposition fallback which then will visualize the vector data
            // directly -> better quality, acceptable number of tile repeat(s)
            // signal that paint is needed
            return true;
        }
        else
        {
            // If visualized small, the amount of repeated fills gets expensive, so
            // in that case use a Bitmap and the Brush technique below.
            // The Bitmap may be created here exactly for the needed target size
            // (using local D2DBitmapPixelProcessor2D and the vector data),
            // but since we have a HW renderer and re-use of system-dependent data
            // at BitmapEx is possible, just get the default fallback Bitmap from the
            // vector data to continue. Trust the existing converters for now to
            // do something with good quality.
            rTarget = rGraphic.GetBitmapEx();
        }
    }

    if (rTarget.IsEmpty() || rTarget.GetSizePixel().IsEmpty())
    {
        // no pixel data, done
        return false;
    }

    // react if OffsetX/OffsetY of the FillGraphicAttribute is used
    takeCareOfOffsetXY(rFillGraphicPrimitive2D, rTarget, rFillUnitRange);

#ifdef DBG_UTIL
    // allow to check bitmap data, e.g. control OffsetX/OffsetY stuff
    static bool bDoSaveForVisualControl(false); // loplugin:constvars:ignore
    if (bDoSaveForVisualControl)
    {
        static const OUString sDumpPath(
            OUString::createFromAscii(std::getenv("VCL_DUMP_BMP_PATH")));
        if (!sDumpPath.isEmpty())
        {
            SvFileStream aNew(sDumpPath + "test_getreplacement.png",
                              StreamMode::WRITE | StreamMode::TRUNC);
            vcl::PngImageWriter aPNGWriter(aNew);
            aPNGWriter.write(rTarget);
        }
    }
#endif

    // signal to render it
    return true;
}

void calculateDiscreteVisibleRange(
    basegfx::B2DRange& rDiscreteVisibleRange, const basegfx::B2DRange& rContentRange,
    const drawinglayer::geometry::ViewInformation2D& rViewInformation2D)
{
    if (rContentRange.isEmpty())
    {
        // no content, done
        rDiscreteVisibleRange.reset();
        return;
    }

    basegfx::B2DRange aDiscreteRange(rContentRange);
    aDiscreteRange.transform(rViewInformation2D.getObjectToViewTransformation());
    const basegfx::B2DRange& rDiscreteViewPort(rViewInformation2D.getDiscreteViewport());
    rDiscreteVisibleRange = aDiscreteRange;

    if (!rDiscreteViewPort.isEmpty())
    {
        rDiscreteVisibleRange.intersect(rDiscreteViewPort);
    }
}
} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
