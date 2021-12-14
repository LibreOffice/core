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

#include <drawinglayer/primitive2d/textprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <primitive2d/texteffectprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <osl/diagnose.h>

using namespace com::sun::star;

namespace drawinglayer::primitive2d
{
namespace
{
// adapts fontScale for usage with TextLayouter. Input is rScale which is the extracted
// scale from a text transformation. A copy is modified so that it contains only positive
// scalings and XY-equal scalings to allow to get a non-X-scaled Vcl-Font for TextLayouter.
// rScale is adapted accordingly to contain the corrected scale which would need to be
// applied to e.g. outlines received from TextLayouter under usage of fontScale. This
// includes Y-Scale, X-Scale-correction and mirrorings.
basegfx::B2DVector getCorrectedScaleAndFontScale(basegfx::B2DVector& rScale)
{
    // copy input value
    basegfx::B2DVector aFontScale(rScale);

    // correct FontHeight settings
    if (basegfx::fTools::equalZero(aFontScale.getY()))
    {
        // no font height; choose one and adapt scale to get back to original scaling
        static const double fDefaultFontScale(100.0);
        rScale.setY(1.0 / fDefaultFontScale);
        aFontScale.setY(fDefaultFontScale);
    }
    else if (basegfx::fTools::less(aFontScale.getY(), 0.0))
    {
        // negative font height; invert and adapt scale to get back to original scaling
        aFontScale.setY(-aFontScale.getY());
        rScale.setY(-1.0);
    }
    else
    {
        // positive font height; adapt scale; scaling will be part of the polygons
        rScale.setY(1.0);
    }

    // correct FontWidth settings
    if (basegfx::fTools::equal(aFontScale.getX(), aFontScale.getY()))
    {
        // no FontScale, adapt scale
        rScale.setX(1.0);
    }
    else
    {
        // If FontScale is used, force to no FontScale to get a non-scaled VCL font.
        // Adapt scaling in X accordingly.
        rScale.setX(aFontScale.getX() / aFontScale.getY());
        aFontScale.setX(aFontScale.getY());
    }

    return aFontScale;
}
} // end of anonymous namespace

void TextSimplePortionPrimitive2D::getTextOutlinesAndTransformation(
    basegfx::B2DPolyPolygonVector& rTarget, basegfx::B2DHomMatrix& rTransformation) const
{
    if (!getTextLength())
        return;

    // decompose object transformation to single values
    basegfx::B2DVector aScale, aTranslate;
    double fRotate, fShearX;

    // if decomposition returns false, create no geometry since e.g. scaling may
    // be zero
    if (!(getTextTransform().decompose(aScale, aTranslate, fRotate, fShearX)
          && aScale.getX() != 0.0))
        return;

    // handle special case: If scale is negative in (x,y) (3rd quadrant), it can
    // be expressed as rotation by PI
    if (basegfx::fTools::less(aScale.getX(), 0.0) && basegfx::fTools::less(aScale.getY(), 0.0))
    {
        aScale = basegfx::absolute(aScale);
        fRotate += M_PI;
    }

    // for the TextLayouterDevice, it is necessary to have a scaling representing
    // the font size. Since we want to extract polygons here, it is okay to
    // work just with scaling and to ignore shear, rotation and translation,
    // all that can be applied to the polygons later
    const basegfx::B2DVector aFontScale(getCorrectedScaleAndFontScale(aScale));

    // prepare textlayoutdevice
    TextLayouterDevice aTextLayouter;
    aTextLayouter.setFontAttribute(getFontAttribute(), aFontScale.getX(), aFontScale.getY(),
                                   getLocale());

    // When getting outlines from stretched text (aScale.getX() != 1.0) it
    // is necessary to inverse-scale the DXArray (if used) to not get the
    // outlines already aligned to given, but wrong DXArray
    if (!getDXArray().empty() && !basegfx::fTools::equal(aScale.getX(), 1.0))
    {
        std::vector<double> aScaledDXArray = getDXArray();
        const double fDXArrayScale(1.0 / aScale.getX());

        for (double& a : aScaledDXArray)
        {
            a *= fDXArrayScale;
        }

        // get the text outlines
        aTextLayouter.getTextOutlines(rTarget, getText(), getTextPosition(), getTextLength(),
                                      aScaledDXArray);
    }
    else
    {
        // get the text outlines
        aTextLayouter.getTextOutlines(rTarget, getText(), getTextPosition(), getTextLength(),
                                      getDXArray());
    }

    // create primitives for the outlines
    const sal_uInt32 nCount(rTarget.size());

    if (nCount)
    {
        // prepare object transformation for polygons
        rTransformation = basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix(
            aScale, fShearX, fRotate, aTranslate);
    }
}

void TextSimplePortionPrimitive2D::create2DDecomposition(
    Primitive2DContainer& rContainer, const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    if (!getTextLength())
        return;

    Primitive2DContainer aRetval;
    basegfx::B2DPolyPolygonVector aB2DPolyPolyVector;
    basegfx::B2DHomMatrix aPolygonTransform;

    // get text outlines and their object transformation
    getTextOutlinesAndTransformation(aB2DPolyPolyVector, aPolygonTransform);

    // create primitives for the outlines
    const sal_uInt32 nCount(aB2DPolyPolyVector.size());

    if (!nCount)
        return;

    // alloc space for the primitives
    aRetval.resize(nCount);

    // color-filled polypolygons
    for (sal_uInt32 a(0); a < nCount; a++)
    {
        // prepare polypolygon
        basegfx::B2DPolyPolygon& rPolyPolygon = aB2DPolyPolyVector[a];
        rPolyPolygon.transform(aPolygonTransform);
        aRetval[a] = new PolyPolygonColorPrimitive2D(rPolyPolygon, getFontColor());
    }

    if (getFontAttribute().getOutline())
    {
        // decompose polygon transformation to single values
        basegfx::B2DVector aScale, aTranslate;
        double fRotate, fShearX;
        aPolygonTransform.decompose(aScale, aTranslate, fRotate, fShearX);

        // create outline text effect with current content and replace
        Primitive2DReference aNewTextEffect(new TextEffectPrimitive2D(
            std::move(aRetval), aTranslate, fRotate, TextEffectStyle2D::Outline));

        aRetval = Primitive2DContainer{ aNewTextEffect };
    }

    rContainer.insert(rContainer.end(), aRetval.begin(), aRetval.end());
}

TextSimplePortionPrimitive2D::TextSimplePortionPrimitive2D(
    const basegfx::B2DHomMatrix& rNewTransform, const OUString& rText, sal_Int32 nTextPosition,
    sal_Int32 nTextLength, std::vector<double>&& rDXArray,
    const attribute::FontAttribute& rFontAttribute, const css::lang::Locale& rLocale,
    const basegfx::BColor& rFontColor, bool bFilled, tools::Long nWidthToFill,
    const Color& rTextFillColor)
    : maTextTransform(rNewTransform)
    , maText(rText)
    , mnTextPosition(nTextPosition)
    , mnTextLength(nTextLength)
    , maDXArray(std::move(rDXArray))
    , maFontAttribute(rFontAttribute)
    , maLocale(rLocale)
    , maFontColor(rFontColor)
    , mbFilled(bFilled)
    , mnWidthToFill(nWidthToFill)
    , maTextFillColor(rTextFillColor)
{
#if OSL_DEBUG_LEVEL > 0
    const sal_Int32 aStringLength(getText().getLength());
    OSL_ENSURE(aStringLength >= getTextPosition()
                   && aStringLength >= getTextPosition() + getTextLength(),
               "TextSimplePortionPrimitive2D with text out of range (!)");
#endif
}

bool LocalesAreEqual(const css::lang::Locale& rA, const css::lang::Locale& rB)
{
    return (rA.Language == rB.Language && rA.Country == rB.Country && rA.Variant == rB.Variant);
}

bool TextSimplePortionPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
{
    if (BufferedDecompositionPrimitive2D::operator==(rPrimitive))
    {
        const TextSimplePortionPrimitive2D& rCompare
            = static_cast<const TextSimplePortionPrimitive2D&>(rPrimitive);

        return (getTextTransform() == rCompare.getTextTransform() && getText() == rCompare.getText()
                && getTextPosition() == rCompare.getTextPosition()
                && getTextLength() == rCompare.getTextLength()
                && getDXArray() == rCompare.getDXArray()
                && getFontAttribute() == rCompare.getFontAttribute()
                && LocalesAreEqual(getLocale(), rCompare.getLocale())
                && getFontColor() == rCompare.getFontColor() && mbFilled == rCompare.mbFilled
                && mnWidthToFill == rCompare.mnWidthToFill
                && maTextFillColor == rCompare.maTextFillColor);
    }

    return false;
}

basegfx::B2DRange TextSimplePortionPrimitive2D::getB2DRange(
    const geometry::ViewInformation2D& /*rViewInformation*/) const
{
    if (maB2DRange.isEmpty() && getTextLength())
    {
        // get TextBoundRect as base size
        // decompose object transformation to single values
        basegfx::B2DVector aScale, aTranslate;
        double fRotate, fShearX;

        if (getTextTransform().decompose(aScale, aTranslate, fRotate, fShearX))
        {
            // for the TextLayouterDevice, it is necessary to have a scaling representing
            // the font size. Since we want to extract polygons here, it is okay to
            // work just with scaling and to ignore shear, rotation and translation,
            // all that can be applied to the polygons later
            const basegfx::B2DVector aFontScale(getCorrectedScaleAndFontScale(aScale));

            // prepare textlayoutdevice
            TextLayouterDevice aTextLayouter;
            aTextLayouter.setFontAttribute(getFontAttribute(), aFontScale.getX(), aFontScale.getY(),
                                           getLocale());

            // get basic text range
            basegfx::B2DRange aNewRange(
                aTextLayouter.getTextBoundRect(getText(), getTextPosition(), getTextLength()));

            // #i104432#, #i102556# take empty results into account
            if (!aNewRange.isEmpty())
            {
                // prepare object transformation for range
                const basegfx::B2DHomMatrix aRangeTransformation(
                    basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix(
                        aScale, fShearX, fRotate, aTranslate));

                // apply range transformation to it
                aNewRange.transform(aRangeTransformation);

                // assign to buffered value
                const_cast<TextSimplePortionPrimitive2D*>(this)->maB2DRange = aNewRange;
            }
        }
    }

    return maB2DRange;
}

// provide unique ID
sal_uInt32 TextSimplePortionPrimitive2D::getPrimitive2DID() const
{
    return PRIMITIVE2D_ID_TEXTSIMPLEPORTIONPRIMITIVE2D;
}

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
