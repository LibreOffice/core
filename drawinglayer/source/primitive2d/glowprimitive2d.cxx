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

#include <drawinglayer/primitive2d/glowprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <drawinglayer/converters.hxx>
#include "GlowSoftEgdeShadowTools.hxx"

#ifdef DBG_UTIL
#include <tools/stream.hxx>
#include <vcl/filter/PngImageWriter.hxx>
#endif

using namespace com::sun::star;

namespace drawinglayer::primitive2d
{
GlowPrimitive2D::GlowPrimitive2D(const Color& rGlowColor, double fRadius,
                                 Primitive2DContainer&& rChildren)
    : BufferedDecompositionGroupPrimitive2D(std::move(rChildren))
    , maGlowColor(rGlowColor)
    , mfGlowRadius(fRadius)
    , mfLastDiscreteGlowRadius(0.0)
    , maLastClippedRange()
{
    // activate callback to flush buffered decomposition content
    setCallbackSeconds(15);
}

bool GlowPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BufferedDecompositionGroupPrimitive2D::operator==(rPrimitive))
    {
        const GlowPrimitive2D& rCompare = static_cast<const GlowPrimitive2D&>(rPrimitive);

        return (getGlowRadius() == rCompare.getGlowRadius()
                && getGlowColor() == rCompare.getGlowColor());
    }

    return false;
}

bool GlowPrimitive2D::prepareValuesAndcheckValidity(
    basegfx::B2DRange& rGlowRange, basegfx::B2DRange& rClippedRange,
    basegfx::B2DVector& rDiscreteGlowSize, double& rfDiscreteGlowRadius,
    const geometry::ViewInformation2D& rViewInformation) const
{
    // no GlowRadius defined, done
    if (getGlowRadius() <= 0.0)
        return false;

    // no geometry, done
    if (getChildren().empty())
        return false;

    // no pixel target, done
    if (rViewInformation.getObjectToViewTransformation().isIdentity())
        return false;

    // get geometry range that defines area that needs to be pixelated
    rGlowRange = getChildren().getB2DRange(rViewInformation);

    // no range of geometry, done
    if (rGlowRange.isEmpty())
        return false;

    // extend range by GlowRadius in all directions
    rGlowRange.grow(getGlowRadius());

    // initialize ClippedRange to full GlowRange -> all is visible
    rClippedRange = rGlowRange;

    // get Viewport and check if used. If empty, all is visible (see
    // ViewInformation2D definition in viewinformation2d.hxx)
    if (!rViewInformation.getViewport().isEmpty())
    {
        // if used, extend by GlowRadius to ensure needed parts are included
        basegfx::B2DRange aVisibleArea(rViewInformation.getViewport());
        aVisibleArea.grow(getGlowRadius());

        // To do this correctly, it needs to be done in discrete coordinates.
        // The object may be transformed relative to the original#
        // ObjectTransformation, e.g. when re-used in shadow
        aVisibleArea.transform(rViewInformation.getViewTransformation());
        rClippedRange.transform(rViewInformation.getObjectToViewTransformation());

        // calculate ClippedRange
        rClippedRange.intersect(aVisibleArea);

        // if GlowRange is completely outside of VisibleArea, ClippedRange
        // will be empty and we are done
        if (rClippedRange.isEmpty())
            return false;

        // convert result back to object coordinates
        rClippedRange.transform(rViewInformation.getInverseObjectToViewTransformation());
    }

    // calculate discrete pixel size of GlowRange. If it's too small to visualize, we are done
    rDiscreteGlowSize = rViewInformation.getObjectToViewTransformation() * rGlowRange.getRange();
    if (ceil(rDiscreteGlowSize.getX()) < 2.0 || ceil(rDiscreteGlowSize.getY()) < 2.0)
        return false;

    // calculate discrete pixel size of GlowRadius. If it's too small to visualize, we are done
    rfDiscreteGlowRadius = ceil(
        (rViewInformation.getObjectToViewTransformation() * basegfx::B2DVector(getGlowRadius(), 0))
            .getLength());
    if (rfDiscreteGlowRadius < 1.0)
        return false;

    return true;
}

void GlowPrimitive2D::create2DDecomposition(
    Primitive2DContainer& rContainer, const geometry::ViewInformation2D& rViewInformation) const
{
    basegfx::B2DRange aGlowRange;
    basegfx::B2DRange aClippedRange;
    basegfx::B2DVector aDiscreteGlowSize;
    double fDiscreteGlowRadius(0.0);

    // Check various validity details and calculate/prepare values. If false, we are done
    if (!prepareValuesAndcheckValidity(aGlowRange, aClippedRange, aDiscreteGlowSize,
                                       fDiscreteGlowRadius, rViewInformation))
        return;

    // Create embedding transformation from object to top-left zero-aligned
    // target pixel geometry (discrete form of ClippedRange)
    // First, move to top-left of GlowRange
    const sal_uInt32 nDiscreteGlowWidth(ceil(aDiscreteGlowSize.getX()));
    const sal_uInt32 nDiscreteGlowHeight(ceil(aDiscreteGlowSize.getY()));
    basegfx::B2DHomMatrix aEmbedding(basegfx::utils::createTranslateB2DHomMatrix(
        -aClippedRange.getMinX(), -aClippedRange.getMinY()));
    // Second, scale to discrete bitmap size
    // Even when using the offset from ClippedRange, we need to use the
    // scaling from the full representation, thus from GlowRange
    aEmbedding.scale(nDiscreteGlowWidth / aGlowRange.getWidth(),
                     nDiscreteGlowHeight / aGlowRange.getHeight());

    // Embed content graphics to TransformPrimitive2D
    const primitive2d::Primitive2DReference xEmbedRef(
        new primitive2d::TransformPrimitive2D(aEmbedding, Primitive2DContainer(getChildren())));
    primitive2d::Primitive2DContainer xEmbedSeq{ xEmbedRef };

    // Create BitmapEx using drawinglayer tooling, including a MaximumQuadraticPixel
    // limitation to be safe and not go runtime/memory havoc. Use a pretty small
    // limit due to this is glow functionality and will look good with bitmap scaling
    // anyways. The value of 250.000 square pixels below maybe adapted as needed.
    const basegfx::B2DVector aDiscreteClippedSize(rViewInformation.getObjectToViewTransformation()
                                                  * aClippedRange.getRange());
    const sal_uInt32 nDiscreteClippedWidth(ceil(aDiscreteClippedSize.getX()));
    const sal_uInt32 nDiscreteClippedHeight(ceil(aDiscreteClippedSize.getY()));
    const geometry::ViewInformation2D aViewInformation2D;
    const sal_uInt32 nMaximumQuadraticPixels(250000);

    // I have now added a helper that just creates the mask without having
    // to render the content, use it, it's faster
    const AlphaMask aAlpha(::drawinglayer::createAlphaMask(
        std::move(xEmbedSeq), aViewInformation2D, nDiscreteClippedWidth, nDiscreteClippedHeight,
        nMaximumQuadraticPixels));

    if (aAlpha.IsEmpty())
        return;

    const Size& rBitmapExSizePixel(aAlpha.GetSizePixel());

    if (rBitmapExSizePixel.Width() <= 0 || rBitmapExSizePixel.Height() <= 0)
        return;

    // We may have to take a corrective scaling into account when the
    // MaximumQuadraticPixel limit was used/triggered
    double fScale(1.0);

    if (static_cast<sal_uInt32>(rBitmapExSizePixel.Width()) != nDiscreteClippedWidth
        || static_cast<sal_uInt32>(rBitmapExSizePixel.Height()) != nDiscreteClippedHeight)
    {
        // scale in X and Y should be the same (see fReduceFactor in createAlphaMask),
        // so adapt numerically to a single scale value, they are integer rounded values
        const double fScaleX(static_cast<double>(rBitmapExSizePixel.Width())
                             / static_cast<double>(nDiscreteClippedWidth));
        const double fScaleY(static_cast<double>(rBitmapExSizePixel.Height())
                             / static_cast<double>(nDiscreteClippedHeight));

        fScale = (fScaleX + fScaleY) * 0.5;
    }

    // fDiscreteGlowRadius is the size of the halo from each side of the object. The halo is the
    // border of glow color that fades from glow transparency level to fully transparent
    // When blurring a sharp boundary (our case), it gets 50% of original intensity, and
    // fades to both sides by the blur radius; thus blur radius is half of glow radius.
    // Consider glow transparency (initial transparency near the object edge)
    AlphaMask mask(ProcessAndBlurAlphaMask(aAlpha, fDiscreteGlowRadius * fScale / 2.0,
                                           fDiscreteGlowRadius * fScale / 2.0,
                                           255 - getGlowColor().GetAlpha()));

    // The end result is the bitmap filled with glow color and blurred 8-bit alpha mask
    Bitmap bmp(aAlpha.GetSizePixel(), vcl::PixelFormat::N24_BPP);
    bmp.Erase(getGlowColor());
    BitmapEx result(bmp, mask);

#ifdef DBG_UTIL
    static bool bDoSaveForVisualControl(false); // loplugin:constvars:ignore
    if (bDoSaveForVisualControl)
    {
        // VCL_DUMP_BMP_PATH should be like C:/path/ or ~/path/
        static const OUString sDumpPath(
            OUString::createFromAscii(std::getenv("VCL_DUMP_BMP_PATH")));
        if (!sDumpPath.isEmpty())
        {
            SvFileStream aNew(sDumpPath + "test_glow.png", StreamMode::WRITE | StreamMode::TRUNC);
            vcl::PngImageWriter aPNGWriter(aNew);
            aPNGWriter.write(result);
        }
    }
#endif

    // Independent from discrete sizes of glow alpha creation, always
    // map and project glow result to geometry range extended by glow
    // radius, but to the eventually clipped instance (ClippedRange)
    const primitive2d::Primitive2DReference xEmbedRefBitmap(
        new BitmapPrimitive2D(result, basegfx::utils::createScaleTranslateB2DHomMatrix(
                                          aClippedRange.getWidth(), aClippedRange.getHeight(),
                                          aClippedRange.getMinX(), aClippedRange.getMinY())));

    rContainer = primitive2d::Primitive2DContainer{ xEmbedRefBitmap };
}

// Using tooling class BufferedDecompositionGroupPrimitive2D now, so
// no more need to locally do the buffered get2DDecomposition here,
// see BufferedDecompositionGroupPrimitive2D::get2DDecomposition
void GlowPrimitive2D::get2DDecomposition(Primitive2DDecompositionVisitor& rVisitor,
                                         const geometry::ViewInformation2D& rViewInformation) const
{
    basegfx::B2DRange aGlowRange;
    basegfx::B2DRange aClippedRange;
    basegfx::B2DVector aDiscreteGlowSize;
    double fDiscreteGlowRadius(0.0);

    // Check various validity details and calculate/prepare values. If false, we are done
    if (!prepareValuesAndcheckValidity(aGlowRange, aClippedRange, aDiscreteGlowSize,
                                       fDiscreteGlowRadius, rViewInformation))
        return;

    if (!getBuffered2DDecomposition().empty())
    {
        // First check is to detect if the last created decompose is capable
        // to represent the now requested visualization.
        // ClippedRange is the needed visualizationArea for the current glow
        // effect, LastClippedRange is the one from the existing/last rendering.
        // Check if last created area is sufficient and can be re-used
        if (!maLastClippedRange.isEmpty() && !maLastClippedRange.isInside(aClippedRange))
        {
            // To avoid unnecessary invalidations due to being *very* correct
            // with HairLines (which are view-dependent and thus change the
            // result(s) here slightly when changing zoom), add a slight unsharp
            // component if we have a ViewTransform. The derivation is inside
            // the range of half a pixel (due to one pixel hairline)
            basegfx::B2DRange aLastClippedRangeAndHairline(maLastClippedRange);

            if (!rViewInformation.getObjectToViewTransformation().isIdentity())
            {
                // Grow by view-dependent size of 1/2 pixel
                const double fHalfPixel((rViewInformation.getInverseObjectToViewTransformation()
                                         * basegfx::B2DVector(0.5, 0))
                                            .getLength());
                aLastClippedRangeAndHairline.grow(fHalfPixel);
            }

            if (!aLastClippedRangeAndHairline.isInside(aClippedRange))
            {
                // Conditions of last local decomposition have changed, delete
                const_cast<GlowPrimitive2D*>(this)->setBuffered2DDecomposition(
                    Primitive2DContainer());
            }
        }
    }

    if (!getBuffered2DDecomposition().empty())
    {
        // Second check is to react on changes of the DiscreteGlowRadius when
        // zooming in/out.
        // Use the known last and current DiscreteGlowRadius to decide
        // if the visualization can be re-used. Be a little 'creative' here
        // and make it dependent on a *relative* change - it is not necessary
        // to re-create everytime if the exact value is missed since zooming
        // pixel-based glow effect is pretty good due to it's smooth nature
        bool bFree(mfLastDiscreteGlowRadius <= 0.0 || fDiscreteGlowRadius <= 0.0);

        if (!bFree)
        {
            const double fDiff(fabs(mfLastDiscreteGlowRadius - fDiscreteGlowRadius));
            const double fLen(fabs(mfLastDiscreteGlowRadius) + fabs(fDiscreteGlowRadius));
            const double fRelativeChange(fDiff / fLen);

            // Use lower fixed values here to change more often, higher to change less often.
            // Value is in the range of ]0.0 .. 1.0]
            bFree = fRelativeChange >= 0.15;
        }

        if (bFree)
        {
            // Conditions of last local decomposition have changed, delete
            const_cast<GlowPrimitive2D*>(this)->setBuffered2DDecomposition(Primitive2DContainer());
        }
    }

    if (getBuffered2DDecomposition().empty())
    {
        // refresh last used DiscreteGlowRadius and ClippedRange to new remembered values
        const_cast<GlowPrimitive2D*>(this)->mfLastDiscreteGlowRadius = fDiscreteGlowRadius;
        const_cast<GlowPrimitive2D*>(this)->maLastClippedRange = aClippedRange;
    }

    // call parent, that will check for empty, call create2DDecomposition and
    // set as decomposition
    BufferedDecompositionGroupPrimitive2D::get2DDecomposition(rVisitor, rViewInformation);
}

basegfx::B2DRange
GlowPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
{
    // Hint: Do *not* use GroupPrimitive2D::getB2DRange, that will (unnecessarily)
    // use the decompose - what works, but is not needed here.
    // We know the to-be-visualized geometry and the radius it needs to be extended,
    // so simply calculate the exact needed range.
    basegfx::B2DRange aRetval(getChildren().getB2DRange(rViewInformation));

    // We need additional space for the glow from all sides
    aRetval.grow(getGlowRadius());

    return aRetval;
}

// provide unique ID
sal_uInt32 GlowPrimitive2D::getPrimitive2DID() const { return PRIMITIVE2D_ID_GLOWPRIMITIVE2D; }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
