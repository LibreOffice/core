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

#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <primitive2d/texteffectprimitive2d.hxx>
#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>
#include <primitive2d/textlineprimitive2d.hxx>
#include <primitive2d/textstrikeoutprimitive2d.hxx>
#include <drawinglayer/primitive2d/textbreakuphelper.hxx>


namespace drawinglayer::primitive2d
{
        void TextDecoratedPortionPrimitive2D::impCreateGeometryContent(
            Primitive2DContainer& rTarget,
            basegfx::utils::B2DHomMatrixBufferedOnDemandDecompose const & rDecTrans,
            const OUString& rText,
            sal_Int32 nTextPosition,
            sal_Int32 nTextLength,
            const std::vector< double >& rDXArray,
            const std::vector< sal_Bool >& rKashidaArray,
            const attribute::FontAttribute& rFontAttribute) const
        {
            // create the SimpleTextPrimitive needed in any case
            rTarget.push_back(
                new TextSimplePortionPrimitive2D(
                    rDecTrans.getB2DHomMatrix(),
                    rText,
                    nTextPosition,
                    nTextLength,
                    std::vector(rDXArray),
                    std::vector(rKashidaArray),
                    rFontAttribute,
                    getLocale(),
                    getFontColor()));

            CreateDecorationGeometryContent(rTarget, rDecTrans, rText,
                                            nTextPosition, nTextLength,
                                            rDXArray);
        }

        void TextDecoratedPortionPrimitive2D::CreateDecorationGeometryContent(
            Primitive2DContainer& rTarget,
            basegfx::utils::B2DHomMatrixBufferedOnDemandDecompose const & rDecTrans,
            const OUString& rText,
            sal_Int32 nTextPosition,
            sal_Int32 nTextLength,
            const std::vector< double >& rDXArray) const
        {
            // see if something else needs to be done
            const bool bOverlineUsed(TEXT_LINE_NONE != getFontOverline());
            const bool bUnderlineUsed(TEXT_LINE_NONE != getFontUnderline());
            const bool bStrikeoutUsed(TEXT_STRIKEOUT_NONE != getTextStrikeout());

            if(!(bUnderlineUsed || bStrikeoutUsed || bOverlineUsed))
                return;

            // common preparations
            TextLayouterDevice aTextLayouter;

            // TextLayouterDevice is needed to get metrics for text decorations like
            // underline/strikeout/emphasis marks from it. For setup, the font size is needed
            aTextLayouter.setFontAttribute(
                getFontAttribute(),
                rDecTrans.getScale().getX(),
                rDecTrans.getScale().getY(),
                getLocale());

            // get text width
            double fTextWidth(0.0);

            if(rDXArray.empty())
            {
                fTextWidth = aTextLayouter.getTextWidth(rText, nTextPosition, nTextLength);
            }
            else
            {
                fTextWidth = rDXArray.back() * rDecTrans.getScale().getX();
                const double fFontScaleX(rDecTrans.getScale().getX());

                if(!basegfx::fTools::equal(fFontScaleX, 1.0)
                    && !basegfx::fTools::equalZero(fFontScaleX))
                {
                    // need to take FontScaling out of the DXArray
                    fTextWidth /= fFontScaleX;
                }
            }

            if(bOverlineUsed)
            {
                // create primitive geometry for overline
                rTarget.push_back(
                    new TextLinePrimitive2D(
                        rDecTrans.getB2DHomMatrix(),
                        fTextWidth,
                        aTextLayouter.getOverlineOffset(),
                        aTextLayouter.getOverlineHeight(),
                        getFontOverline(),
                        getOverlineColor()));
            }

            if(bUnderlineUsed)
            {
                // create primitive geometry for underline
                rTarget.push_back(
                    new TextLinePrimitive2D(
                        rDecTrans.getB2DHomMatrix(),
                        fTextWidth,
                        aTextLayouter.getUnderlineOffset(),
                        aTextLayouter.getUnderlineHeight(),
                        getFontUnderline(),
                        getTextlineColor()));
            }

            if(!bStrikeoutUsed)
                return;

            // create primitive geometry for strikeout
            if(TEXT_STRIKEOUT_SLASH == getTextStrikeout() || TEXT_STRIKEOUT_X == getTextStrikeout())
            {
                // strikeout with character
                const sal_Unicode aStrikeoutChar(TEXT_STRIKEOUT_SLASH == getTextStrikeout() ? '/' : 'X');

                rTarget.push_back(
                    new TextCharacterStrikeoutPrimitive2D(
                        rDecTrans.getB2DHomMatrix(),
                        fTextWidth,
                        getFontColor(),
                        aStrikeoutChar,
                        getFontAttribute(),
                        getLocale()));
            }
            else
            {
                // strikeout with geometry
                rTarget.push_back(
                    new TextGeometryStrikeoutPrimitive2D(
                        rDecTrans.getB2DHomMatrix(),
                        fTextWidth,
                        getFontColor(),
                        aTextLayouter.getUnderlineHeight(),
                        aTextLayouter.getStrikeoutOffset(),
                        getTextStrikeout()));
            }

            // TODO: Handle Font Emphasis Above/Below
        }

        Primitive2DReference TextDecoratedPortionPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            if(getWordLineMode())
            {
                // support for single word mode; split to single word primitives
                // using TextBreakupHelper
                TextBreakupHelper aTextBreakupHelper(*this);
                Primitive2DContainer aBroken(aTextBreakupHelper.extractResult(BreakupUnit::Word));

                if(!aBroken.empty())
                {
                    // was indeed split to several words, use as result
                    return new GroupPrimitive2D(std::move(aBroken));
                }
                else
                {
                    // no split, was already a single word. Continue to
                    // decompose local entity
                }
            }
            basegfx::utils::B2DHomMatrixBufferedOnDemandDecompose aDecTrans(getTextTransform());
            Primitive2DContainer aRetval;

            // create basic geometry such as SimpleTextPrimitive, Overline, Underline,
            // Strikeout, etc...
            // prepare new font attributes WITHOUT outline
            const attribute::FontAttribute aNewFontAttribute(
                getFontAttribute().getFamilyName(),
                getFontAttribute().getStyleName(),
                getFontAttribute().getWeight(),
                getFontAttribute().getSymbol(),
                getFontAttribute().getVertical(),
                getFontAttribute().getItalic(),
                getFontAttribute().getMonospaced(),
                false,             // no outline anymore, handled locally
                getFontAttribute().getRTL(),
                getFontAttribute().getBiDiStrong());

            // handle as one word
            impCreateGeometryContent(aRetval, aDecTrans, getText(), getTextPosition(), getTextLength(), getDXArray(), getKashidaArray(), aNewFontAttribute);

            // Handle Shadow, Outline and TextRelief
            if(aRetval.empty())
                return nullptr;

            // outline AND shadow depend on NO TextRelief (see dialog)
            const bool bHasTextRelief(TEXT_RELIEF_NONE != getTextRelief());
            const bool bHasShadow(!bHasTextRelief && getShadow());
            const bool bHasOutline(!bHasTextRelief && getFontAttribute().getOutline());

            if(bHasShadow || bHasTextRelief || bHasOutline)
            {
                Primitive2DReference aShadow;

                if(bHasShadow)
                {
                    // create shadow with current content (in aRetval). Text shadow
                    // is constant, relative to font size, rotated with the text and has a
                    // constant color.
                    // shadow parameter values
                    static const double fFactor(1.0 / 24.0);
                    const double fTextShadowOffset(aDecTrans.getScale().getY() * fFactor);
                    static basegfx::BColor aShadowColor(0.3, 0.3, 0.3);

                    // prepare shadow transform matrix
                    const basegfx::B2DHomMatrix aShadowTransform(basegfx::utils::createTranslateB2DHomMatrix(
                        fTextShadowOffset, fTextShadowOffset));

                    // create shadow primitive
                    aShadow = new ShadowPrimitive2D(
                        aShadowTransform,
                        aShadowColor,
                        0,          // fShadowBlur = 0, there's no blur for text shadow yet.
                        Primitive2DContainer(aRetval));
                }

                if(bHasTextRelief)
                {
                    // create emboss using an own helper primitive since this will
                    // be view-dependent
                    const basegfx::BColor aBBlack(0.0, 0.0, 0.0);
                    const bool bDefaultTextColor(aBBlack == getFontColor());
                    TextEffectStyle2D aTextEffectStyle2D(TextEffectStyle2D::ReliefEmbossed);

                    if(bDefaultTextColor)
                    {
                        if(TEXT_RELIEF_ENGRAVED == getTextRelief())
                        {
                            aTextEffectStyle2D = TextEffectStyle2D::ReliefEngravedDefault;
                        }
                        else
                        {
                            aTextEffectStyle2D = TextEffectStyle2D::ReliefEmbossedDefault;
                        }

                        aRetval = Primitive2DContainer {
                            new TextEffectPrimitive2D(
                                std::move(aRetval),
                                aDecTrans.getTranslate(),
                                aDecTrans.getRotate(),
                                aTextEffectStyle2D)
                         };
                    }
                    else
                    {
                        // create outline using an own helper primitive since this will
                        // be view-dependent
                        aRetval = Primitive2DContainer {
                            new TextEffectPrimitive2D(
                                std::move(aRetval),
                                aDecTrans.getTranslate(),
                                aDecTrans.getRotate(),
                                TextEffectStyle2D::Outline)
                         };
                    }

                    aRetval = Primitive2DContainer {
                        Primitive2DReference(new TextEffectPrimitive2D(
                            std::move(aRetval),
                            aDecTrans.getTranslate(),
                            aDecTrans.getRotate(),
                            aTextEffectStyle2D))
                     };
                }
                else if(bHasOutline)
                {
                    // create outline using an own helper primitive since this will
                    // be view-dependent
                    aRetval = Primitive2DContainer {
                        Primitive2DReference(new TextEffectPrimitive2D(
                            std::move(aRetval),
                            aDecTrans.getTranslate(),
                            aDecTrans.getRotate(),
                            TextEffectStyle2D::Outline))
                     };
                }

                if(aShadow.is())
                {
                    // put shadow in front if there is one to paint timely before
                    // but placed behind content
                    aRetval.insert(aRetval.begin(), aShadow);
                }
            }

            return new GroupPrimitive2D(std::move(aRetval));
        }

        TextDecoratedPortionPrimitive2D::TextDecoratedPortionPrimitive2D(
            // TextSimplePortionPrimitive2D parameters
            const basegfx::B2DHomMatrix& rNewTransform,
            const OUString& rText,
            sal_Int32 nTextPosition,
            sal_Int32 nTextLength,
            std::vector< double >&& rDXArray,
            std::vector< sal_Bool >&& rKashidaArray,
            const attribute::FontAttribute& rFontAttribute,
            const css::lang::Locale& rLocale,
            const basegfx::BColor& rFontColor,
            const Color& rFillColor,

            // local parameters
            const basegfx::BColor& rOverlineColor,
            const basegfx::BColor& rTextlineColor,
            TextLine eFontOverline,
            TextLine eFontUnderline,
            bool bUnderlineAbove,
            TextStrikeout eTextStrikeout,
            bool bWordLineMode,
            TextEmphasisMark eTextEmphasisMark,
            bool bEmphasisMarkAbove,
            bool bEmphasisMarkBelow,
            TextRelief eTextRelief,
            bool bShadow)
        :   TextSimplePortionPrimitive2D(
                rNewTransform,
                rText,
                nTextPosition,
                nTextLength,
                std::move(rDXArray),
                std::move(rKashidaArray),
                rFontAttribute,
                rLocale,
                rFontColor,
                rFillColor),
            maOverlineColor(rOverlineColor),
            maTextlineColor(rTextlineColor),
            meFontOverline(eFontOverline),
            meFontUnderline(eFontUnderline),
            meTextStrikeout(eTextStrikeout),
            meTextEmphasisMark(eTextEmphasisMark),
            meTextRelief(eTextRelief),
            mbUnderlineAbove(bUnderlineAbove),
            mbWordLineMode(bWordLineMode),
            mbEmphasisMarkAbove(bEmphasisMarkAbove),
            mbEmphasisMarkBelow(bEmphasisMarkBelow),
            mbShadow(bShadow)
        {
        }

        bool TextDecoratedPortionPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(TextSimplePortionPrimitive2D::operator==(rPrimitive))
            {
                const TextDecoratedPortionPrimitive2D& rCompare = static_cast<const TextDecoratedPortionPrimitive2D&>(rPrimitive);

                return (getOverlineColor() == rCompare.getOverlineColor()
                    && getTextlineColor() == rCompare.getTextlineColor()
                    && getFontOverline() == rCompare.getFontOverline()
                    && getFontUnderline() == rCompare.getFontUnderline()
                    && getTextStrikeout() == rCompare.getTextStrikeout()
                    && getTextEmphasisMark() == rCompare.getTextEmphasisMark()
                    && getTextRelief() == rCompare.getTextRelief()
                    && getUnderlineAbove() == rCompare.getUnderlineAbove()
                    && getWordLineMode() == rCompare.getWordLineMode()
                    && getEmphasisMarkAbove() == rCompare.getEmphasisMarkAbove()
                    && getEmphasisMarkBelow() == rCompare.getEmphasisMarkBelow()
                    && getShadow() == rCompare.getShadow());
            }

            return false;
        }

        // #i96475#
        // Added missing implementation. Decorations may (will) stick out of the text's
        // inking area, so add them if needed
        basegfx::B2DRange TextDecoratedPortionPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            // check if this needs to be a TextDecoratedPortionPrimitive2D or
            // if a TextSimplePortionPrimitive2D would be sufficient
            if (TEXT_LINE_NONE != getFontOverline()
                 || TEXT_LINE_NONE != getFontUnderline()
                 || TEXT_STRIKEOUT_NONE != getTextStrikeout()
                 || TEXT_FONT_EMPHASIS_MARK_NONE != getTextEmphasisMark()
                 || TEXT_RELIEF_NONE != getTextRelief()
                 || getShadow())
            {
                // decoration is used, fallback to BufferedDecompositionPrimitive2D::getB2DRange which uses
                // the own local decomposition for computation and thus creates all necessary
                // geometric objects
                return BufferedDecompositionPrimitive2D::getB2DRange(rViewInformation);
            }
            else
            {
                // no relevant decoration used, fallback to TextSimplePortionPrimitive2D::getB2DRange
                return TextSimplePortionPrimitive2D::getB2DRange(rViewInformation);
            }
        }

        // provide unique ID
        sal_uInt32 TextDecoratedPortionPrimitive2D::getPrimitive2DID() const
        {
            return PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D;
        }

} // end of namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
