/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <drawinglayer/primitive2d/ReflectionPrimitive2D.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/fillgradientprimitive2d.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/transparenceprimitive2d.hxx>
#include <drawinglayer/attribute/fillgradientattribute.hxx>
#include <drawinglayer/converters.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/utils/bgradient.hxx>
#include "GlowSoftEgdeShadowTools.hxx"

#include <cmath>

namespace drawinglayer::primitive2d
{
namespace
{
// The reflection has the size of the content and starts fDistance below its bottom edge.
basegfx::B2DRange getReflectionRange(const basegfx::B2DRange& rContentRange, double fDistance)
{
    const double fTop = rContentRange.getMaxY() + fDistance;
    return basegfx::B2DRange(rContentRange.getMinX(), fTop, rContentRange.getMaxX(),
                             fTop + rContentRange.getHeight());
}

// The blur radius in pixels. 0.0 when there is no pixel target.
double getDiscreteBlurRadius(double fBlurRadius,
                             const geometry::ViewInformation2D& rViewInformation)
{
    if (fBlurRadius <= 0.0 || rViewInformation.getObjectToViewTransformation().isIdentity())
        return 0.0;

    return (rViewInformation.getObjectToViewTransformation() * basegfx::B2DVector(fBlurRadius, 0))
        .getLength();
}
}

ReflectionPrimitive2D::ReflectionPrimitive2D(const attribute::ReflectionAttribute& rReflection,
                                             Primitive2DContainer&& rChildren)
    : BufferedDecompositionGroupPrimitive2D(std::move(rChildren))
    , maReflection(rReflection)
{
    // activate callback to flush buffered decomposition content
    activateFlushOnTimer();
}

bool ReflectionPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (!BufferedDecompositionGroupPrimitive2D::operator==(rPrimitive))
        return false;

    const ReflectionPrimitive2D& rCompare = static_cast<const ReflectionPrimitive2D&>(rPrimitive);

    return getReflection() == rCompare.getReflection();
}

void ReflectionPrimitive2D::create2DDecomposition(
    Primitive2DContainer& rContainer, const geometry::ViewInformation2D& rViewInformation) const
{
    if (getChildren().empty())
        return;

    const attribute::ReflectionAttribute& rReflection = getReflection();

    // A reflection that is invisible along its whole height is not painted.
    if (rReflection.isDefault())
        return;

    const basegfx::B2DRange aContentRange(getChildren().getB2DRange(rViewInformation));
    if (aContentRange.isEmpty())
        return;

    const basegfx::B2DRange aReflectionRange(
        getReflectionRange(aContentRange, rReflection.getDistance()));

    // Mirror the content at its bottom edge, then move it down by the distance.
    basegfx::B2DHomMatrix aMirror(
        basegfx::utils::createTranslateB2DHomMatrix(0.0, -aContentRange.getMaxY()));
    aMirror.scale(1.0, -1.0);
    aMirror.translate(0.0, aContentRange.getMaxY() + rReflection.getDistance());

    // The mask is a linear gradient from top to bottom, and its luminance is the transparency.
    const basegfx::BColor aStart(rReflection.getStartTransparency(),
                                 rReflection.getStartTransparency(),
                                 rReflection.getStartTransparency());
    const basegfx::BColor aEnd(rReflection.getEndTransparency(), rReflection.getEndTransparency(),
                               rReflection.getEndTransparency());
    const basegfx::BColorStops aStops{ basegfx::BColorStop(0.0, aStart),
                                       basegfx::BColorStop(rReflection.getStartPosition(), aStart),
                                       basegfx::BColorStop(rReflection.getEndPosition(), aEnd),
                                       basegfx::BColorStop(1.0, aEnd) };
    const attribute::FillGradientAttribute aGradient(css::awt::GradientStyle_LINEAR, 0.0, 0.0, 0.0,
                                                     0.0, aStops);

    Primitive2DContainer aReflection{ new TransparencePrimitive2D(
        Primitive2DContainer{
            new TransformPrimitive2D(aMirror, Primitive2DContainer(getChildren())) },
        Primitive2DContainer{ new FillGradientPrimitive2D(aReflectionRange, aGradient) }) };

    const double fDiscreteBlurRadius(
        getDiscreteBlurRadius(rReflection.getBlurRadius(), rViewInformation));

    // A blur of less than a pixel does not show, so the reflection stays vector geometry.
    if (fDiscreteBlurRadius < 1.0)
    {
        rContainer = std::move(aReflection);
        return;
    }

    basegfx::B2DRange aBlurRange(aReflectionRange);
    aBlurRange.grow(rReflection.getBlurRadius());

    // get Viewport and check if used. If empty, all is visible (see
    // ViewInformation2D definition in viewinformation2d.hxx)
    if (!rViewInformation.getViewport().isEmpty())
    {
        basegfx::B2DRange aVisibleArea(rViewInformation.getViewport());
        aVisibleArea.grow(rReflection.getBlurRadius());

        if (!aVisibleArea.overlaps(aBlurRange))
            return;
    }

    const basegfx::B2DVector aDiscreteBlurSize(rViewInformation.getObjectToViewTransformation()
                                               * aBlurRange.getRange());
    const sal_uInt32 nDiscreteBlurWidth(ceil(aDiscreteBlurSize.getX()));
    const sal_uInt32 nDiscreteBlurHeight(ceil(aDiscreteBlurSize.getY()));
    if (nDiscreteBlurWidth < 2 || nDiscreteBlurHeight < 2)
        return;

    // Move the top-left of the blur range to zero and scale it to the discrete bitmap size.
    basegfx::B2DHomMatrix aEmbedding(
        basegfx::utils::createTranslateB2DHomMatrix(-aBlurRange.getMinX(), -aBlurRange.getMinY()));
    aEmbedding.scale(nDiscreteBlurWidth / aBlurRange.getWidth(),
                     nDiscreteBlurHeight / aBlurRange.getHeight());

    // The limit on the pixel count keeps the time and the memory for a large reflection bounded.
    // A blurred reflection looks good when its bitmap is scaled up, so the limit is a small one.
    const geometry::ViewInformation2D aViewInformation2D;
    constexpr sal_uInt32 constMaximumQuadraticPixels = 250000;
    const Bitmap aBitmap(::drawinglayer::convertToBitmap(
        Primitive2DContainer{ new TransformPrimitive2D(aEmbedding, std::move(aReflection)) },
        aViewInformation2D, nDiscreteBlurWidth, nDiscreteBlurHeight, constMaximumQuadraticPixels,
        true));

    if (aBitmap.IsEmpty())
        return;

    const Size aBitmapSizePixel(aBitmap.GetSizePixel());
    if (aBitmapSizePixel.Width() <= 0 || aBitmapSizePixel.Height() <= 0)
        return;

    // The limit on the pixel count scales the bitmap down by the same factor on both axes, and
    // the blur radius shrinks with it.
    const double fScale = (double(aBitmapSizePixel.Width()) / double(nDiscreteBlurWidth)
                           + double(aBitmapSizePixel.Height()) / double(nDiscreteBlurHeight))
                          * 0.5;

    const Bitmap aBlurred(BlurBitmapWithAlpha(aBitmap, fDiscreteBlurRadius * fScale));

    rContainer = Primitive2DContainer{ new BitmapPrimitive2D(
        aBlurred, basegfx::utils::createScaleTranslateB2DHomMatrix(
                      aBlurRange.getWidth(), aBlurRange.getHeight(), aBlurRange.getMinX(),
                      aBlurRange.getMinY())) };
}

void ReflectionPrimitive2D::get2DDecomposition(
    Primitive2DDecompositionVisitor& rVisitor,
    const geometry::ViewInformation2D& rViewInformation) const
{
    const basegfx::B2DRange aContentRange(getChildren().getB2DRange(rViewInformation));
    const double fDiscreteBlurRadius
        = getDiscreteBlurRadius(getReflection().getBlurRadius(), rViewInformation);

    if (hasBuffered2DDecomposition())
    {
        // The content range changes by a fraction of a pixel with the zoom, for example through a
        // hairline. The buffered reflection stays in use for a change below half a pixel.
        double fHalfPixel = 0.0;
        if (!rViewInformation.getObjectToViewTransformation().isIdentity())
        {
            fHalfPixel = (rViewInformation.getInverseObjectToViewTransformation()
                          * basegfx::B2DVector(0.5, 0))
                             .getLength();
        }

        bool bFree = !maLastContentRange.equal(aContentRange, fHalfPixel);

        const bool bLastBlurred = mfLastDiscreteBlurRadius >= 1.0;
        const bool bBlurred = fDiscreteBlurRadius >= 1.0;
        if (bLastBlurred != bBlurred)
        {
            bFree = true;
        }
        else if (bBlurred)
        {
            // A blur looks nearly the same for a small change of its radius, so only a change
            // of 15% or more makes a new one.
            const double fDiff(std::abs(mfLastDiscreteBlurRadius - fDiscreteBlurRadius));
            const double fLen(mfLastDiscreteBlurRadius + fDiscreteBlurRadius);
            if (fDiff / fLen >= 0.15)
                bFree = true;
        }

        if (bFree)
        {
            const_cast<ReflectionPrimitive2D*>(this)->setBuffered2DDecomposition(
                Primitive2DContainer());
        }
    }

    if (!hasBuffered2DDecomposition())
    {
        const_cast<ReflectionPrimitive2D*>(this)->maLastContentRange = aContentRange;
        const_cast<ReflectionPrimitive2D*>(this)->mfLastDiscreteBlurRadius = fDiscreteBlurRadius;
    }

    // call parent, that will check for empty, call create2DDecomposition and
    // set as decomposition
    BufferedDecompositionGroupPrimitive2D::get2DDecomposition(rVisitor, rViewInformation);
}

basegfx::B2DRange
ReflectionPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
{
    // The range follows from the content range, so the decomposition is not needed here.
    const basegfx::B2DRange aContentRange(getChildren().getB2DRange(rViewInformation));
    if (aContentRange.isEmpty())
        return aContentRange;

    basegfx::B2DRange aRetval(getReflectionRange(aContentRange, getReflection().getDistance()));
    aRetval.grow(getReflection().getBlurRadius());
    return aRetval;
}

sal_uInt32 ReflectionPrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_REFLECTIONPRIMITIVE2D;
}

} // end of namespace drawinglayer::primitive2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
