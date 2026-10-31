/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
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

#include <drawinglayer/primitive2d/fonttextprimitive.hxx>
#include <primitive2d/fontattributeandtransform.hxx>

#include <drawinglayer/attribute/fontattribute.hxx>
#include <drawinglayer/primitive2d/PolyPolygonColorPrimitive2D.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/textprimitive2d.hxx>

#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/range/b2drange.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/font.hxx>

namespace drawinglayer::primitive2d
{
namespace
{
/// Whether the underline goes above the text, which is right for vertical Japanese only.
bool isUnderlineAbove(const vcl::Font& rFont)
{
    if (!rFont.IsVertical())
    {
        return false;
    }

    return (LANGUAGE_JAPANESE == rFont.GetLanguage())
           || (LANGUAGE_JAPANESE == rFont.GetCJKContextLanguage());
}
}

void createFontAttributeAndTransform(const vcl::Font& rFont,
                                     vcl::text::ComplexTextLayoutFlags eLayoutMode,
                                     attribute::FontAttribute& rFontAttribute,
                                     basegfx::B2DHomMatrix& rTextTransform,
                                     basegfx::B2DVector& rAlignmentOffset)
{
    basegfx::B2DVector aFontScaling;

    rFontAttribute = getFontAttributeFromVclFont(
        aFontScaling, rFont, bool(eLayoutMode & vcl::text::ComplexTextLayoutFlags::BiDiRtl),
        bool(eLayoutMode & vcl::text::ComplexTextLayoutFlags::BiDiStrong));

    // add FontScaling
    rTextTransform.scale(aFontScaling.getX(), aFontScaling.getY());

    // take text align into account
    if (ALIGN_BASELINE != rFont.GetAlignment())
    {
        TextLayouterDevice aTextLayouterDevice;
        aTextLayouterDevice.setFont(rFont);

        if (ALIGN_TOP == rFont.GetAlignment())
        {
            rAlignmentOffset.setY(aTextLayouterDevice.getFontAscent());
        }
        else // ALIGN_BOTTOM
        {
            rAlignmentOffset.setY(-aTextLayouterDevice.getFontDescent());
        }

        rTextTransform.translate(rAlignmentOffset.getX(), rAlignmentOffset.getY());
    }

    // add FontRotation (if used)
    if (rFont.GetOrientation())
    {
        rTextTransform.rotate(-toRadians(rFont.GetOrientation()));
    }
}

Primitive2DReference createTextPrimitiveFromFont(const basegfx::B2DPoint& rStart,
                                                 const OUString& rText, sal_Int32 nTextStart,
                                                 sal_Int32 nTextLength,
                                                 std::vector<double>&& rDXArray,
                                                 boost::container::vector<bool>&& rKashidaArray,
                                                 const vcl::Font& rFont,
                                                 const TextDrawState& rState)
{
    rtl::Reference<BasePrimitive2D> pResult;
    basegfx::B2DVector aAlignmentOffset(0.0, 0.0);

    if(nTextLength)
    {
        drawinglayer::attribute::FontAttribute aFontAttribute;
        basegfx::B2DHomMatrix aTextTransform;

        // fill parameters derived from current font
        createFontAttributeAndTransform(rFont, rState.meLayoutMode, aFontAttribute,
                                        aTextTransform, aAlignmentOffset);

        // add TextStartPosition
        aTextTransform.translate(rStart.getX(), rStart.getY());

        // prepare FontColor and Locale
        const basegfx::BColor aFontColor(rState.maTextColor);
        const Color aFillColor(rFont.GetFillColor());
        css::lang::Locale aLocale(LanguageTag(rState.meLanguage).getLocale());
        const bool bWordLineMode(rFont.IsWordLineMode());

        const bool bDecoratedIsNeeded(
               LINESTYLE_NONE != rFont.GetOverline()
            || LINESTYLE_NONE != rFont.GetUnderline()
            || STRIKEOUT_NONE != rFont.GetStrikeout()
            || FontEmphasisMark::NONE != (rFont.GetEmphasisMark() & FontEmphasisMark::Style)
            || FontRelief::NONE != rFont.GetRelief()
            || rFont.IsShadow()
            || bWordLineMode);

        if(bDecoratedIsNeeded)
        {
            // prepare overline, underline and strikeout data
            const TextLine eFontOverline(mapFontLineStyleToTextLine(rFont.GetOverline()));
            const TextLine eFontLineStyle(mapFontLineStyleToTextLine(rFont.GetUnderline()));
            const TextStrikeout eTextStrikeout(
                mapFontStrikeoutToTextStrikeout(rFont.GetStrikeout()));

            // check UndelineAbove
            const bool bUnderlineAbove(TEXT_LINE_NONE != eFontLineStyle && isUnderlineAbove(rFont));

            // prepare emphasis mark data
            TextEmphasisMark eTextEmphasisMark(TEXT_FONT_EMPHASIS_MARK_NONE);

            switch(rFont.GetEmphasisMark() & FontEmphasisMark::Style)
            {
                case FontEmphasisMark::Dot:
                    eTextEmphasisMark = TEXT_FONT_EMPHASIS_MARK_DOT;
                    break;
                case FontEmphasisMark::Circle:
                    eTextEmphasisMark = TEXT_FONT_EMPHASIS_MARK_CIRCLE;
                    break;
                case FontEmphasisMark::Disc:
                    eTextEmphasisMark = TEXT_FONT_EMPHASIS_MARK_DISC;
                    break;
                case FontEmphasisMark::Accent:
                    eTextEmphasisMark = TEXT_FONT_EMPHASIS_MARK_ACCENT;
                    break;
                default:
                    break;
            }

            const bool bEmphasisMarkAbove(rFont.GetEmphasisMark() & FontEmphasisMark::PosAbove);
            const bool bEmphasisMarkBelow(rFont.GetEmphasisMark() & FontEmphasisMark::PosBelow);

            // prepare font relief data
            TextRelief eTextRelief(TEXT_RELIEF_NONE);

            switch(rFont.GetRelief())
            {
                case FontRelief::Embossed : eTextRelief = TEXT_RELIEF_EMBOSSED; break;
                case FontRelief::Engraved : eTextRelief = TEXT_RELIEF_ENGRAVED; break;
                default : break; // RELIEF_NONE, FontRelief_FORCE_EQUAL_SIZE
            }

            // prepare shadow/outline data
            const bool bShadow(rFont.IsShadow());

            // TextDecoratedPortionPrimitive2D is needed, create one
            pResult = new TextDecoratedPortionPrimitive2D(

                // attributes for TextSimplePortionPrimitive2D
                aTextTransform,
                rText,
                nTextStart,
                nTextLength,
                std::vector(rDXArray),
                std::move(rKashidaArray),
                aFontAttribute,
                aLocale,
                aFontColor,
                aFillColor,
                rFont.GetFixKerning(),
                100, 0,
                rFont.GetOpticalSizing(),

                // attributes for TextDecoratedPortionPrimitive2D
                rState.moOverlineColor ? *rState.moOverlineColor : aFontColor,
                rState.moTextLineColor ? *rState.moTextLineColor : aFontColor,
                eFontOverline,
                eFontLineStyle,
                bUnderlineAbove,
                eTextStrikeout,
                bWordLineMode,
                eTextEmphasisMark,
                bEmphasisMarkAbove,
                bEmphasisMarkBelow,
                eTextRelief,
                bShadow);
        }
        else
        {
            // TextSimplePortionPrimitive2D is enough
            pResult = new TextSimplePortionPrimitive2D(
                aTextTransform,
                rText,
                nTextStart,
                nTextLength,
                std::vector(rDXArray),
                std::move(rKashidaArray),
                std::move(aFontAttribute),
                std::move(aLocale),
                aFontColor,
                COL_TRANSPARENT,
                rFont.GetFixKerning(),
                100, 0,
                rFont.GetOpticalSizing());
        }
    }

    if(pResult && rState.moTextFillColor)
    {
        // text background is requested, add and encapsulate both to new primitive
        TextLayouterDevice aTextLayouterDevice;
        aTextLayouterDevice.setFont(rFont);

        // get text width
        double fTextWidth(0.0);

        if (rDXArray.empty())
        {
            fTextWidth = aTextLayouterDevice.getTextWidth(rText, nTextStart, nTextLength);
        }
        else
        {
            fTextWidth = rDXArray.back();
        }

        if (fTextWidth > 0.0 && !basegfx::fTools::equalZero(fTextWidth))
        {
            // build text range
            const basegfx::B2DRange aTextRange(
                0.0, -aTextLayouterDevice.getFontAscent(),
                fTextWidth, aTextLayouterDevice.getFontDescent());

            // create Transform
            basegfx::B2DHomMatrix aTextTransform;

            aTextTransform.translate(aAlignmentOffset.getX(), aAlignmentOffset.getY());

            if(rFont.GetOrientation())
            {
                aTextTransform.rotate(-toRadians(rFont.GetOrientation()));
            }

            aTextTransform.translate(rStart.getX(), rStart.getY());

            // prepare Primitive2DSequence, put text in foreground
            Primitive2DContainer aSequence(2);
            aSequence[1] = pResult;

            // prepare filled polygon
            basegfx::B2DPolygon aOutline(basegfx::utils::createPolygonFromRect(aTextRange));
            aOutline.transform(aTextTransform);

            aSequence[0] = Primitive2DReference(
                new PolyPolygonColorPrimitive2D(
                    basegfx::B2DPolyPolygon(aOutline),
                    *rState.moTextFillColor));

            // set as group at pResult
            pResult = new GroupPrimitive2D(std::move(aSequence));
        }
    }

    return pResult;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
