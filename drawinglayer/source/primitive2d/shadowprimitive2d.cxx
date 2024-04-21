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

#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>
#include <basegfx/color/bcolormodifier.hxx>
#include <drawinglayer/primitive2d/modifiedcolorprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <drawinglayer/converters.hxx>
#include "GlowSoftEgdeShadowTools.hxx"

#ifdef DBG_UTIL
#include <tools/stream.hxx>
#include <vcl/filter/PngImageWriter.hxx>
#endif

#include <memory>
#include <utility>

using namespace com::sun::star;

namespace drawinglayer::primitive2d
{
ShadowPrimitive2D::ShadowPrimitive2D(basegfx::B2DHomMatrix aShadowTransform,
                                     const basegfx::BColor& rShadowColor, double fShadowBlur,
                                     Primitive2DContainer&& aChildren)
    : BufferedDecompositionGroupPrimitive2D(std::move(aChildren))
    , maShadowTransform(std::move(aShadowTransform))
    , maShadowColor(rShadowColor)
    , mfShadowBlur(fShadowBlur)
    , mfLastDiscreteBlurRadius(0.0)
    , maLastClippedRange()
{
    // activate callback to flush buffered decomposition content
    setCallbackSeconds(15);
}

bool ShadowPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BufferedDecompositionGroupPrimitive2D::operator==(rPrimitive))
    {
        const ShadowPrimitive2D& rCompare = static_cast<const ShadowPrimitive2D&>(rPrimitive);

        return (getShadowTransform() == rCompare.getShadowTransform()
                && getShadowColor() == rCompare.getShadowColor()
                && getShadowBlur() == rCompare.getShadowBlur());
    }

    return false;
}

// Helper to get the to-be-shadowed geometry completely embedded to
// a ModifiedColorPrimitive2D (change to ShadowColor) and TransformPrimitive2D
// (direction/offset/transformation of shadow). Since this is used pretty
// often, pack into a helper
void ShadowPrimitive2D::getFullyEmbeddedShadowPrimitives(Primitive2DContainer& rContainer) const
{
    if (getChildren().empty())
        return;

    // create a modifiedColorPrimitive containing the shadow color and the content
    const basegfx::BColorModifierSharedPtr aBColorModifier
        = std::make_shared<basegfx::BColorModifier_replace>(getShadowColor());
    const Primitive2DReference xRefA(
        new ModifiedColorPrimitive2D(Primitive2DContainer(getChildren()), aBColorModifier));
    Primitive2DContainer aSequenceB{ xRefA };

    // build transformed primitiveVector with shadow offset and add to target
    rContainer.visit(new TransformPrimitive2D(getShadowTransform(), std::move(aSequenceB)));
}

bool ShadowPrimitive2D::prepareValuesAndcheckValidity(
    basegfx::B2DRange& rBlurRange, basegfx::B2DRange& rClippedRange,
    basegfx::B2DVector& rDiscreteBlurSize, double& rfDiscreteBlurRadius,
    const geometry::ViewInformation2D& rViewInformation) const
{
    // no BlurRadius defined, done
    if (getShadowBlur() <= 0.0)
        return false;

    // no geometry, done
    if (getChildren().empty())
        return false;

    // no pixel target, done
    if (rViewInformation.getObjectToViewTransformation().isIdentity())
        return false;

    // get fully embedded ShadowPrimitive
    Primitive2DContainer aEmbedded;
    getFullyEmbeddedShadowPrimitives(aEmbedded);

    // get geometry range that defines area that needs to be pixelated
    rBlurRange = aEmbedded.getB2DRange(rViewInformation);

    // no range of geometry, done
    if (rBlurRange.isEmpty())
        return false;

    // extend range by BlurRadius in all directions
    rBlurRange.grow(getShadowBlur());

    // initialize ClippedRange to full BlurRange -> all is visible
    rClippedRange = rBlurRange;

    // get Viewport and check if used. If empty, all is visible (see
    // ViewInformation2D definition in viewinformation2d.hxx)
    if (!rViewInformation.getViewport().isEmpty())
    {
        // if used, extend by BlurRadius to ensure needed parts are included
        basegfx::B2DRange aVisibleArea(rViewInformation.getViewport());
        aVisibleArea.grow(getShadowBlur());

        // calculate ClippedRange
        rClippedRange.intersect(aVisibleArea);

        // if BlurRange is completely outside of VisibleArea, ClippedRange
        // will be empty and we are done
        if (rClippedRange.isEmpty())
            return false;
    }

    // calculate discrete pixel size of BlurRange. If it's too small to visualize, we are done
    rDiscreteBlurSize = rViewInformation.getObjectToViewTransformation() * rBlurRange.getRange();
    if (ceil(rDiscreteBlurSize.getX()) < 2.0 || ceil(rDiscreteBlurSize.getY()) < 2.0)
        return false;

    // calculate discrete pixel size of BlurRadius. If it's too small to visualize, we are done
    rfDiscreteBlurRadius = ceil(
        (rViewInformation.getObjectToViewTransformation() * basegfx::B2DVector(getShadowBlur(), 0))
            .getLength());
    if (rfDiscreteBlurRadius < 1.0)
        return false;

    return true;
}

void ShadowPrimitive2D::create2DDecomposition(
    Primitive2DContainer& rContainer, const geometry::ViewInformation2D& rViewInformation) const
{
    if (getShadowBlur() <= 0.0)
    {
        // Normal (non-blurred) shadow is already completely
        // handled by get2DDecomposition and not buffered. It
        // does not need to be since it's a simple embedding
        // to a ModifiedColorPrimitive2D and TransformPrimitive2D
        return;
    }

    // from here on we process a blurred shadow
    basegfx::B2DRange aBlurRange;
    basegfx::B2DRange aClippedRange;
    basegfx::B2DVector aDiscreteBlurSize;
    double fDiscreteBlurRadius(0.0);

    // Check various validity details and calculate/prepare values. If false, we are done
    if (!prepareValuesAndcheckValidity(aBlurRange, aClippedRange, aDiscreteBlurSize,
                                       fDiscreteBlurRadius, rViewInformation))
        return;

    // Create embedding transformation from object to top-left zero-aligned
    // target pixel geometry (discrete form of ClippedRange)
    // First, move to top-left of BlurRange
    const sal_uInt32 nDiscreteBlurWidth(ceil(aDiscreteBlurSize.getX()));
    const sal_uInt32 nDiscreteBlurHeight(ceil(aDiscreteBlurSize.getY()));
    basegfx::B2DHomMatrix aEmbedding(basegfx::utils::createTranslateB2DHomMatrix(
        -aClippedRange.getMinX(), -aClippedRange.getMinY()));
    // Second, scale to discrete bitmap size
    // Even when using the offset from ClippedRange, we need to use the
    // scaling from the full representation, thus from BlurRange
    aEmbedding.scale(nDiscreteBlurWidth / aBlurRange.getWidth(),
                     nDiscreteBlurHeight / aBlurRange.getHeight());

    // Get fully embedded ShadowPrimitives. This will also embed to
    // ModifiedColorPrimitive2D (what is not urgently needed) to create
    // the alpha channel, but a paint with all colors set to a single
    // one (like shadowColor here) is often less expensive due to possible
    // simplifications painting the primitives (e.g. gradient)
    Primitive2DContainer aEmbedded;
    getFullyEmbeddedShadowPrimitives(aEmbedded);

    // Embed content graphics to TransformPrimitive2D
    const primitive2d::Primitive2DReference xEmbedRef(
        new primitive2d::TransformPrimitive2D(aEmbedding, std::move(aEmbedded)));
    primitive2d::Primitive2DContainer xEmbedSeq{ xEmbedRef };

    // Create BitmapEx using drawinglayer tooling, including a MaximumQuadraticPixel
    // limitation to be safe and not go runtime/memory havoc. Use a pretty small
    // limit due to this is Blurred Shadow functionality and will look good with bitmap
    // scaling anyways. The value of 250.000 square pixels below maybe adapted as needed.
    const basegfx::B2DVector aDiscreteClippedSize(rViewInformation.getObjectToViewTransformation()
                                                  * aClippedRange.getRange());
    const sal_uInt32 nDiscreteClippedWidth(ceil(aDiscreteClippedSize.getX()));
    const sal_uInt32 nDiscreteClippedHeight(ceil(aDiscreteClippedSize.getY()));
    const geometry::ViewInformation2D aViewInformation2D;
    const sal_uInt32 nMaximumQuadraticPixels(250000);

    // I have now added a helper that just creates the mask without having
    // to render the content, use it, it's faster
    const AlphaMask aAlpha(::drawinglayer::createAlphaMask(
        std::move(xEmbedSeq), aViewInformation2D,
        basegfx::B2DRange(0, 0, nDiscreteClippedWidth, nDiscreteClippedHeight),
        nMaximumQuadraticPixels));

    // if we have no shadow, we are done
    if (aAlpha.IsEmpty())
        return;

    const Size& rBitmapExSizePixel(aAlpha.GetSizePixel());
    if (!(rBitmapExSizePixel.Width() > 0 && rBitmapExSizePixel.Height() > 0))
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

    // Use the Alpha as base to blur and apply the effect
    const AlphaMask mask(drawinglayer::primitive2d::ProcessAndBlurAlphaMask(
        aAlpha, 0, fDiscreteBlurRadius * fScale, 0, false));

    // The end result is the bitmap filled with blur color and blurred 8-bit alpha mask
    Bitmap bmp(aAlpha.GetSizePixel(), vcl::PixelFormat::N24_BPP);
    bmp.Erase(Color(getShadowColor()));
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
            SvFileStream aNew(sDumpPath + "test_shadowblur.png",
                              StreamMode::WRITE | StreamMode::TRUNC);
            vcl::PngImageWriter aPNGWriter(aNew);
            aPNGWriter.write(result);
        }
    }
#endif

    // Independent from discrete sizes of blur alpha creation, always
    // map and project blur result to geometry range extended by blur
    // radius, but to the eventually clipped instance (ClippedRange)
    const primitive2d::Primitive2DReference xEmbedRefBitmap(
        new BitmapPrimitive2D(result, basegfx::utils::createScaleTranslateB2DHomMatrix(
                                          aClippedRange.getWidth(), aClippedRange.getHeight(),
                                          aClippedRange.getMinX(), aClippedRange.getMinY())));

    rContainer = primitive2d::Primitive2DContainer{ xEmbedRefBitmap };
}

void ShadowPrimitive2D::get2DDecomposition(
    Primitive2DDecompositionVisitor& rVisitor,
    const geometry::ViewInformation2D& rViewInformation) const
{
    if (getShadowBlur() <= 0.0)
    {
        // normal (non-blurred) shadow
        if (getChildren().empty())
            return;

        // get fully embedded ShadowPrimitives
        Primitive2DContainer aEmbedded;
        getFullyEmbeddedShadowPrimitives(aEmbedded);

        rVisitor.visit(aEmbedded);
        return;
    }

    // here we have a blurred shadow, check conditions of last
    // buffered decompose and decide re-use or re-create by using
    // setBuffered2DDecomposition to reset local buffered version
    basegfx::B2DRange aBlurRange;
    basegfx::B2DRange aClippedRange;
    basegfx::B2DVector aDiscreteBlurSize;
    double fDiscreteBlurRadius(0.0);

    // Check various validity details and calculate/prepare values. If false, we are done
    if (!prepareValuesAndcheckValidity(aBlurRange, aClippedRange, aDiscreteBlurSize,
                                       fDiscreteBlurRadius, rViewInformation))
        return;

    if (!getBuffered2DDecomposition().empty())
    {
        // First check is to detect if the last created decompose is capable
        // to represent the now requested visualization (see similar
        // implementation at GlowPrimitive2D).
        if (!maLastClippedRange.isEmpty() && !maLastClippedRange.isInside(aClippedRange))
        {
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
                const_cast<ShadowPrimitive2D*>(this)->setBuffered2DDecomposition(
                    Primitive2DContainer());
            }
        }
    }

    if (!getBuffered2DDecomposition().empty())
    {
        // Second check is to react on changes of the DiscreteSoftRadius when
        // zooming in/out (see similar implementation at ShadowPrimitive2D).
        bool bFree(mfLastDiscreteBlurRadius <= 0.0 || fDiscreteBlurRadius <= 0.0);

        if (!bFree)
        {
            const double fDiff(fabs(mfLastDiscreteBlurRadius - fDiscreteBlurRadius));
            const double fLen(fabs(mfLastDiscreteBlurRadius) + fabs(fDiscreteBlurRadius));
            const double fRelativeChange(fDiff / fLen);

            // Use lower fixed values here to change more often, higher to change less often.
            // Value is in the range of ]0.0 .. 1.0]
            bFree = fRelativeChange >= 0.15;
        }

        if (bFree)
        {
            // Conditions of last local decomposition have changed, delete
            const_cast<ShadowPrimitive2D*>(this)->setBuffered2DDecomposition(
                Primitive2DContainer());
        }
    }

    if (getBuffered2DDecomposition().empty())
    {
        // refresh last used DiscreteBlurRadius and ClippedRange to new remembered values
        const_cast<ShadowPrimitive2D*>(this)->mfLastDiscreteBlurRadius = fDiscreteBlurRadius;
        const_cast<ShadowPrimitive2D*>(this)->maLastClippedRange = aClippedRange;
    }

    // call parent, that will check for empty, call create2DDecomposition and
    // set as decomposition
    BufferedDecompositionGroupPrimitive2D::get2DDecomposition(rVisitor, rViewInformation);
}

basegfx::B2DRange
ShadowPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
{
    // Hint: Do *not* use GroupPrimitive2D::getB2DRange, that will (unnecessarily)
    // use the decompose - what works, but is not needed here.
    // We know the to-be-visualized geometry and the radius it needs to be extended,
    // so simply calculate the exact needed range.
    basegfx::B2DRange aRetval(getChildren().getB2DRange(rViewInformation));

    if (getShadowBlur() > 0.0)
    {
        // blurred shadow, that extends the geometry
        aRetval.grow(getShadowBlur());
    }

    aRetval.transform(getShadowTransform());
    return aRetval;
}

// provide unique ID
sal_uInt32 ShadowPrimitive2D::getPrimitive2DID() const { return PRIMITIVE2D_ID_SHADOWPRIMITIVE2D; }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
