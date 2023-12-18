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

#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/softedgeprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <drawinglayer/converters.hxx>
#include "GlowSoftEgdeShadowTools.hxx"

#ifdef DBG_UTIL
#include <tools/stream.hxx>
#include <vcl/filter/PngImageWriter.hxx>
#endif

namespace drawinglayer::primitive2d
{
SoftEdgePrimitive2D::SoftEdgePrimitive2D(double fRadius, Primitive2DContainer&& aChildren)
    : BufferedDecompositionGroupPrimitive2D(std::move(aChildren))
    , mfRadius(fRadius)
    , mfLastDiscreteSoftRadius(0.0)
    , maLastClippedRange()
{
    // activate callback to flush buffered decomposition content
    setCallbackSeconds(15);
}

bool SoftEdgePrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BufferedDecompositionGroupPrimitive2D::operator==(rPrimitive))
    {
        auto& rCompare = static_cast<const SoftEdgePrimitive2D&>(rPrimitive);
        return getRadius() == rCompare.getRadius();
    }

    return false;
}

bool SoftEdgePrimitive2D::prepareValuesAndcheckValidity(
    basegfx::B2DRange& rSoftRange, basegfx::B2DRange& rClippedRange,
    basegfx::B2DVector& rDiscreteSoftSize, double& rfDiscreteSoftRadius,
    const geometry::ViewInformation2D& rViewInformation) const
{
    // no SoftRadius defined, done
    if (getRadius() <= 0.0)
        return false;

    // no geometry, done
    if (getChildren().empty())
        return false;

    // no pixel target, done
    if (rViewInformation.getObjectToViewTransformation().isIdentity())
        return false;

    // get geometry range that defines area that needs to be pixelated
    rSoftRange = getChildren().getB2DRange(rViewInformation);

    // no range of geometry, done
    if (rSoftRange.isEmpty())
        return false;

    // initialize ClippedRange to full SoftRange -> all is visible
    rClippedRange = rSoftRange;

    // get Viewport and check if used. If empty, all is visible (see
    // ViewInformation2D definition in viewinformation2d.hxx)
    if (!rViewInformation.getViewport().isEmpty())
    {
        // if used, extend by SoftRadius to ensure needed parts are included
        // that are not visible, but influence the visible parts
        basegfx::B2DRange aVisibleArea(rViewInformation.getViewport());
        aVisibleArea.grow(getRadius() * 2);

        // To do this correctly, it needs to be done in discrete coordinates.
        // The object may be transformed relative to the original#
        // ObjectTransformation, e.g. when re-used in shadow
        aVisibleArea.transform(rViewInformation.getViewTransformation());
        rClippedRange.transform(rViewInformation.getObjectToViewTransformation());

        // calculate ClippedRange
        rClippedRange.intersect(aVisibleArea);

        // if SoftRange is completely outside of VisibleArea, ClippedRange
        // will be empty and we are done
        if (rClippedRange.isEmpty())
            return false;

        // convert result back to object coordinates
        rClippedRange.transform(rViewInformation.getInverseObjectToViewTransformation());
    }

    // calculate discrete pixel size of SoftRange. If it's too small to visualize, we are done
    rDiscreteSoftSize = rViewInformation.getObjectToViewTransformation() * rSoftRange.getRange();
    if (ceil(rDiscreteSoftSize.getX()) < 2.0 || ceil(rDiscreteSoftSize.getY()) < 2.0)
        return false;

    // calculate discrete pixel size of SoftRadius. If it's too small to visualize, we are done
    rfDiscreteSoftRadius = ceil(
        (rViewInformation.getObjectToViewTransformation() * basegfx::B2DVector(getRadius(), 0))
            .getLength());
    if (rfDiscreteSoftRadius < 1.0)
        return false;

    return true;
}

void SoftEdgePrimitive2D::create2DDecomposition(
    Primitive2DContainer& rContainer, const geometry::ViewInformation2D& rViewInformation) const
{
    // Use endless while-loop-and-break mechanism due to having multiple
    // exit scenarios that all have to do the same thing when exiting
    while (true)
    {
        basegfx::B2DRange aSoftRange;
        basegfx::B2DRange aClippedRange;
        basegfx::B2DVector aDiscreteSoftSize;
        double fDiscreteSoftRadius(0.0);

        // Check various validity details and calculate/prepare values. If false, we are done
        if (!prepareValuesAndcheckValidity(aSoftRange, aClippedRange, aDiscreteSoftSize,
                                           fDiscreteSoftRadius, rViewInformation))
            break;

        // Create embedding transformation from object to top-left zero-aligned
        // target pixel geometry (discrete form of ClippedRange)
        // First, move to top-left of SoftRange
        const sal_uInt32 nDiscreteSoftWidth(ceil(aDiscreteSoftSize.getX()));
        const sal_uInt32 nDiscreteSoftHeight(ceil(aDiscreteSoftSize.getY()));
        basegfx::B2DHomMatrix aEmbedding(basegfx::utils::createTranslateB2DHomMatrix(
            -aClippedRange.getMinX(), -aClippedRange.getMinY()));
        // Second, scale to discrete bitmap size
        // Even when using the offset from ClippedRange, we need to use the
        // scaling from the full representation, thus from SoftRange
        aEmbedding.scale(nDiscreteSoftWidth / aSoftRange.getWidth(),
                         nDiscreteSoftHeight / aSoftRange.getHeight());

        // Embed content graphics to TransformPrimitive2D
        const primitive2d::Primitive2DReference xEmbedRef(
            new primitive2d::TransformPrimitive2D(aEmbedding, Primitive2DContainer(getChildren())));
        primitive2d::Primitive2DContainer xEmbedSeq{ xEmbedRef };

        // Create BitmapEx using drawinglayer tooling, including a MaximumQuadraticPixel
        // limitation to be safe and not go runtime/memory havoc. Use a pretty small
        // limit due to this is softEdge functionality and will look good with bitmap scaling
        // anyways. The value of 250.000 square pixels below maybe adapted as needed.
        const basegfx::B2DVector aDiscreteClippedSize(
            rViewInformation.getObjectToViewTransformation() * aClippedRange.getRange());
        const sal_uInt32 nDiscreteClippedWidth(ceil(aDiscreteClippedSize.getX()));
        const sal_uInt32 nDiscreteClippedHeight(ceil(aDiscreteClippedSize.getY()));
        const geometry::ViewInformation2D aViewInformation2D;
        const sal_uInt32 nMaximumQuadraticPixels(250000);
        // tdf#156808 force an alpha mask to be created even if it has no alpha
        // We need an alpha mask, even if it is totally opaque, so that
        // drawinglayer::primitive2d::ProcessAndBlurAlphaMask() can be called.
        // Otherwise, blurring of edges will fail in cases like running in a
        // slideshow or exporting to PDF.
        const BitmapEx aBitmapEx(::drawinglayer::convertToBitmapEx(
            std::move(xEmbedSeq), aViewInformation2D, nDiscreteClippedWidth, nDiscreteClippedHeight,
            nMaximumQuadraticPixels, true));

        if (aBitmapEx.IsEmpty())
            break;

        // Get BitmapEx and check size. If no content, we are done
        const Size& rBitmapExSizePixel(aBitmapEx.GetSizePixel());
        if (!(rBitmapExSizePixel.Width() > 0 && rBitmapExSizePixel.Height() > 0))
            break;

        // We may have to take a corrective scaling into account when the
        // MaximumQuadraticPixel limit was used/triggered
        double fScale(1.0);

        if (static_cast<sal_uInt32>(rBitmapExSizePixel.Width()) != nDiscreteClippedWidth
            || static_cast<sal_uInt32>(rBitmapExSizePixel.Height()) != nDiscreteClippedHeight)
        {
            // scale in X and Y should be the same (see fReduceFactor in convertToBitmapEx),
            // so adapt numerically to a single scale value, they are integer rounded values
            const double fScaleX(static_cast<double>(rBitmapExSizePixel.Width())
                                 / static_cast<double>(nDiscreteClippedWidth));
            const double fScaleY(static_cast<double>(rBitmapExSizePixel.Height())
                                 / static_cast<double>(nDiscreteClippedHeight));

            fScale = (fScaleX + fScaleY) * 0.5;
        }

        // Get the Alpha and use as base to blur and apply the effect
        AlphaMask aMask(aBitmapEx.GetAlphaMask());
        if (aMask.IsEmpty()) // There is no mask, fully opaque
            break;
        AlphaMask blurMask(drawinglayer::primitive2d::ProcessAndBlurAlphaMask(
            aMask, -fDiscreteSoftRadius * fScale, fDiscreteSoftRadius * fScale, 0));
        aMask.BlendWith(blurMask);

        // The end result is the original bitmap with blurred 8-bit alpha mask
        BitmapEx result(aBitmapEx.GetBitmap(), aMask);

#ifdef DBG_UTIL
        static bool bDoSaveForVisualControl(false); // loplugin:constvars:ignore
        if (bDoSaveForVisualControl)
        {
            // VCL_DUMP_BMP_PATH should be like C:/path/ or ~/path/
            static const OUString sDumpPath(
                OUString::createFromAscii(std::getenv("VCL_DUMP_BMP_PATH")));
            if (!sDumpPath.isEmpty())
            {
                SvFileStream aNew(sDumpPath + "test_softedge.png",
                                  StreamMode::WRITE | StreamMode::TRUNC);
                vcl::PngImageWriter aPNGWriter(aNew);
                aPNGWriter.write(result);
            }
        }
#endif

        // Independent from discrete sizes of soft alpha creation, always
        // map and project soft result to geometry range extended by soft
        // radius, but to the eventually clipped instance (ClippedRange)
        const primitive2d::Primitive2DReference xEmbedRefBitmap(
            new BitmapPrimitive2D(result, basegfx::utils::createScaleTranslateB2DHomMatrix(
                                              aClippedRange.getWidth(), aClippedRange.getHeight(),
                                              aClippedRange.getMinX(), aClippedRange.getMinY())));

        rContainer = primitive2d::Primitive2DContainer{ xEmbedRefBitmap };

        // we made it, return
        return;
    }

    // creation failed for some of many possible reasons, use original
    // content, so the unmodified original geometry will be the result,
    // just without any softEdge effect
    rContainer = getChildren();
}

void SoftEdgePrimitive2D::get2DDecomposition(
    Primitive2DDecompositionVisitor& rVisitor,
    const geometry::ViewInformation2D& rViewInformation) const
{
    // Use endless while-loop-and-break mechanism due to having multiple
    // exit scenarios that all have to do the same thing when exiting
    while (true)
    {
        basegfx::B2DRange aSoftRange;
        basegfx::B2DRange aClippedRange;
        basegfx::B2DVector aDiscreteSoftSize;
        double fDiscreteSoftRadius(0.0);

        // Check various validity details and calculate/prepare values. If false, we are done
        if (!prepareValuesAndcheckValidity(aSoftRange, aClippedRange, aDiscreteSoftSize,
                                           fDiscreteSoftRadius, rViewInformation))
            break;

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
                    const_cast<SoftEdgePrimitive2D*>(this)->setBuffered2DDecomposition(
                        Primitive2DContainer());
                }
            }
        }

        if (!getBuffered2DDecomposition().empty())
        {
            // Second check is to react on changes of the DiscreteSoftRadius when
            // zooming in/out (see similar implementation at GlowPrimitive2D).
            bool bFree(mfLastDiscreteSoftRadius <= 0.0 || fDiscreteSoftRadius <= 0.0);

            if (!bFree)
            {
                const double fDiff(fabs(mfLastDiscreteSoftRadius - fDiscreteSoftRadius));
                const double fLen(fabs(mfLastDiscreteSoftRadius) + fabs(fDiscreteSoftRadius));
                const double fRelativeChange(fDiff / fLen);

                // Use a lower value here, soft edge keeps it's content so avoid that it gets too
                // unsharp in the pixel visualization
                // Value is in the range of ]0.0 .. 1.0]
                bFree = fRelativeChange >= 0.075;
            }

            if (bFree)
            {
                // Conditions of last local decomposition have changed, delete
                const_cast<SoftEdgePrimitive2D*>(this)->setBuffered2DDecomposition(
                    Primitive2DContainer());
            }
        }

        if (getBuffered2DDecomposition().empty())
        {
            // refresh last used DiscreteSoftRadius and ClippedRange to new remembered values
            const_cast<SoftEdgePrimitive2D*>(this)->mfLastDiscreteSoftRadius = fDiscreteSoftRadius;
            const_cast<SoftEdgePrimitive2D*>(this)->maLastClippedRange = aClippedRange;
        }

        // call parent, that will check for empty, call create2DDecomposition and
        // set as decomposition
        BufferedDecompositionGroupPrimitive2D::get2DDecomposition(rVisitor, rViewInformation);

        // we made it, return
        return;
    }

    // No soft edge needed for some of many possible reasons, use original content
    rVisitor.visit(getChildren());
}

basegfx::B2DRange
SoftEdgePrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
{
    // Hint: Do *not* use GroupPrimitive2D::getB2DRange, that will (unnecessarily)
    // use the decompose - what works, but is not needed here.
    // We know the to-be-visualized geometry and the radius it needs to be extended,
    // so simply calculate the exact needed range.
    return getChildren().getB2DRange(rViewInformation);
}

sal_uInt32 SoftEdgePrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_SOFTEDGEPRIMITIVE2D;
}

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
