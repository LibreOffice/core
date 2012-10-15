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

#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlayoutdevice.hxx>
#include <drawinglayer/primitive2d/polygonprimitive2d.hxx>
#include <drawinglayer/attribute/strokeattribute.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/i18n/WordType.hpp>
#include <drawinglayer/primitive2d/texteffectprimitive2d.hxx>
#include <drawinglayer/primitive2d/shadowprimitive2d.hxx>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/primitive2d/textlineprimitive2d.hxx>
#include <drawinglayer/primitive2d/textstrikeoutprimitive2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        void TextDecoratedPortionPrimitive2D::impCreateGeometryContent(
            std::vector< Primitive2DReference >& rTarget,
            basegfx::tools::B2DHomMatrixBufferedOnDemandDecompose& rDecTrans,
            const String& rText,
            xub_StrLen aTextPosition,
            xub_StrLen aTextLength,
            const ::std::vector< double >& rDXArray,
            const attribute::FontAttribute& rFontAttribute) const
        {
            // create the SimpleTextPrimitive needed in any case
            rTarget.push_back(Primitive2DReference(
                new TextSimplePortionPrimitive2D(
                    rDecTrans.getB2DHomMatrix(),
                    rText,
                    aTextPosition,
                    aTextLength,
                    rDXArray,
                    rFontAttribute,
                    getLocale(),
                    getFontColor())));

            // see if something else needs to be done
            const bool bOverlineUsed(TEXT_LINE_NONE != getFontOverline());
            const bool bUnderlineUsed(TEXT_LINE_NONE != getFontUnderline());
            const bool bStrikeoutUsed(TEXT_STRIKEOUT_NONE != getTextStrikeout());

            if(bUnderlineUsed || bStrikeoutUsed || bOverlineUsed)
            {
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
                    fTextWidth = aTextLayouter.getTextWidth(rText, aTextPosition, aTextLength);
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
                    rTarget.push_back(Primitive2DReference(
                        new TextLinePrimitive2D(
                            rDecTrans.getB2DHomMatrix(),
                            fTextWidth,
                            aTextLayouter.getOverlineOffset(),
                            aTextLayouter.getOverlineHeight(),
                            getFontOverline(),
                            getOverlineColor())));
                }

                if(bUnderlineUsed)
                {
                    // create primitive geometry for underline
                    rTarget.push_back(Primitive2DReference(
                        new TextLinePrimitive2D(
                            rDecTrans.getB2DHomMatrix(),
                            fTextWidth,
                            aTextLayouter.getUnderlineOffset(),
                            aTextLayouter.getUnderlineHeight(),
                            getFontUnderline(),
                            getTextlineColor())));
                }

                if(bStrikeoutUsed)
                {
                    // create primitive geometry for strikeout
                    if(TEXT_STRIKEOUT_SLASH == getTextStrikeout() || TEXT_STRIKEOUT_X == getTextStrikeout())
                    {
                        // strikeout with character
                        const sal_Unicode aStrikeoutChar(TEXT_STRIKEOUT_SLASH == getTextStrikeout() ? '/' : 'X');

                        rTarget.push_back(Primitive2DReference(
                            new TextCharacterStrikeoutPrimitive2D(
                                rDecTrans.getB2DHomMatrix(),
                                fTextWidth,
                                getFontColor(),
                                aStrikeoutChar,
                                getFontAttribute(),
                                getLocale())));
                    }
                    else
                    {
                        // strikeout with geometry
                        rTarget.push_back(Primitive2DReference(
                            new TextGeometryStrikeoutPrimitive2D(
                                rDecTrans.getB2DHomMatrix(),
                                fTextWidth,
                                getFontColor(),
                                aTextLayouter.getUnderlineHeight(),
                                aTextLayouter.getStrikeoutOffset(),
                                getTextStrikeout())));
                    }
                }
            }

            // TODO: Handle Font Emphasis Above/Below
        }

        void TextDecoratedPortionPrimitive2D::impCorrectTextBoundary(::com::sun::star::i18n::Boundary& rNextWordBoundary) const
        {
            // truncate aNextWordBoundary to min/max possible values. This is necessary since the word start may be
            // before/after getTextPosition() when a long string is the content and getTextPosition()
            // is right inside a word. Same for end.
            const sal_Int32 aMinPos(static_cast< sal_Int32 >(getTextPosition()));
            const sal_Int32 aMaxPos(aMinPos + static_cast< sal_Int32 >(getTextLength()));

            if(rNextWordBoundary.startPos < aMinPos)
            {
                rNextWordBoundary.startPos = aMinPos;
            }
            else if(rNextWordBoundary.startPos > aMaxPos)
            {
                rNextWordBoundary.startPos = aMaxPos;
            }

            if(rNextWordBoundary.endPos < aMinPos)
            {
                rNextWordBoundary.endPos = aMinPos;
            }
            else if(rNextWordBoundary.endPos > aMaxPos)
            {
                rNextWordBoundary.endPos = aMaxPos;
            }
        }

        void TextDecoratedPortionPrimitive2D::impSplitSingleWords(
            std::vector< Primitive2DReference >& rTarget,
            basegfx::tools::B2DHomMatrixBufferedOnDemandDecompose& rDecTrans) const
        {
            // break iterator support
            // made static so it only needs to be fetched once, even with many single
            // constructed VclMetafileProcessor2D. It's still incarnated on demand,
            // but exists for OOo runtime now by purpose.
            static ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator > xLocalBreakIterator;

            if(!xLocalBreakIterator.is())
            {
                ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext(::comphelper::getProcessComponentContext());
                xLocalBreakIterator = com::sun::star::i18n::BreakIterator::create(xContext);
            }

            if( getTextLength() )
            {
                // init word iterator, get first word and truncate to possibilities
                ::com::sun::star::i18n::Boundary aNextWordBoundary(xLocalBreakIterator->getWordBoundary(
                    getText(), getTextPosition(), getLocale(), ::com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES, sal_True));

                if(aNextWordBoundary.endPos == getTextPosition())
                {
                    // backward hit, force next word
                    aNextWordBoundary = xLocalBreakIterator->getWordBoundary(
                        getText(), getTextPosition() + 1, getLocale(), ::com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES, sal_True);
                }

                impCorrectTextBoundary(aNextWordBoundary);

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

                if(aNextWordBoundary.startPos == getTextPosition() && aNextWordBoundary.endPos == getTextLength())
                {
                    // it IS only a single word, handle as one word
                    impCreateGeometryContent(rTarget, rDecTrans, getText(), getTextPosition(), getTextLength(), getDXArray(), aNewFontAttribute);
                }
                else
                {
                    // prepare TextLayouter
                    const bool bNoDXArray(getDXArray().empty());
                    TextLayouterDevice aTextLayouter;

                    if(bNoDXArray)
                    {
                        // ..but only completely when no DXArray
                        aTextLayouter.setFontAttribute(
                            getFontAttribute(),
                            rDecTrans.getScale().getX(),
                            rDecTrans.getScale().getY(),
                            getLocale());
                    }

                    // do iterate over single words
                    while(aNextWordBoundary.startPos != aNextWordBoundary.endPos)
                    {
                        // prepare values for new portion
                        const xub_StrLen nNewTextStart(static_cast< xub_StrLen >(aNextWordBoundary.startPos));
                        const xub_StrLen nNewTextEnd(static_cast< xub_StrLen >(aNextWordBoundary.endPos));

                        // prepare transform for the single word
                        basegfx::B2DHomMatrix aNewTransform;
                        ::std::vector< double > aNewDXArray;
                        const bool bNewStartIsNotOldStart(nNewTextStart > getTextPosition());

                        if(!bNoDXArray)
                        {
                            // prepare new DXArray for the single word
                            aNewDXArray = ::std::vector< double >(
                                getDXArray().begin() + static_cast< sal_uInt32 >(nNewTextStart - getTextPosition()),
                                getDXArray().begin() + static_cast< sal_uInt32 >(nNewTextEnd - getTextPosition()));
                        }

                        if(bNewStartIsNotOldStart)
                        {
                            // needs to be moved to a new start position
                            double fOffset(0.0);

                            if(bNoDXArray)
                            {
                                // evaluate using TextLayouter
                                fOffset = aTextLayouter.getTextWidth(getText(), getTextPosition(), nNewTextStart);
                            }
                            else
                            {
                                // get from DXArray
                                const sal_uInt32 nIndex(static_cast< sal_uInt32 >(nNewTextStart - getTextPosition()));
                                fOffset = getDXArray()[nIndex - 1];
                            }

                            // need offset without FontScale for building the new transformation. The
                            // new transformation will be multiplied with the current text transformation
                            // so FontScale would be double
                            double fOffsetNoScale(fOffset);
                            const double fFontScaleX(rDecTrans.getScale().getX());

                            if(!basegfx::fTools::equal(fFontScaleX, 1.0)
                                && !basegfx::fTools::equalZero(fFontScaleX))
                            {
                                fOffsetNoScale /= fFontScaleX;
                            }

                            // apply needed offset to transformation
                            aNewTransform.translate(fOffsetNoScale, 0.0);

                            if(!bNoDXArray)
                            {
                                // DXArray values need to be corrected with the offset, too. Here,
                                // take the scaled offset since the DXArray is scaled
                                const sal_uInt32 nArraySize(aNewDXArray.size());

                                for(sal_uInt32 a(0); a < nArraySize; a++)
                                {
                                    aNewDXArray[a] -= fOffset;
                                }
                            }
                        }

                        // add text transformation to new transformation
                        aNewTransform *= rDecTrans.getB2DHomMatrix();

                        // create geometry content for the single word. Do not forget
                        // to use the new transformation
                        basegfx::tools::B2DHomMatrixBufferedOnDemandDecompose aDecTrans(aNewTransform);

                        impCreateGeometryContent(rTarget, aDecTrans, getText(), nNewTextStart,
                            nNewTextEnd - nNewTextStart, aNewDXArray, aNewFontAttribute);

                        if(aNextWordBoundary.endPos >= getTextPosition() + getTextLength())
                        {
                            // end reached
                            aNextWordBoundary.startPos = aNextWordBoundary.endPos;
                        }
                        else
                        {
                            // get new word portion
                            const sal_Int32 nLastEndPos(aNextWordBoundary.endPos);

                            aNextWordBoundary = xLocalBreakIterator->getWordBoundary(
                                getText(), aNextWordBoundary.endPos, getLocale(),
                                ::com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES, sal_True);

                            if(nLastEndPos == aNextWordBoundary.endPos)
                            {
                                // backward hit, force next word
                                aNextWordBoundary = xLocalBreakIterator->getWordBoundary(
                                    getText(), nLastEndPos + 1, getLocale(),
                                    ::com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES, sal_True);
                            }

                            impCorrectTextBoundary(aNextWordBoundary);
                        }
                    }
                }
            }
        }

        Primitive2DSequence TextDecoratedPortionPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            std::vector< Primitive2DReference > aNewPrimitives;
            basegfx::tools::B2DHomMatrixBufferedOnDemandDecompose aDecTrans(getTextTransform());
            Primitive2DSequence aRetval;

            // create basic geometry such as SimpleTextPrimitive, Overline, Underline,
            // Strikeout, etc...
            if(getWordLineMode())
            {
                // support for single word mode
                impSplitSingleWords(aNewPrimitives, aDecTrans);
            }
            else
            {
                // prepare new font attributes WITHOUT outline
                const attribute::FontAttribute aNewFontAttribute(
                    getFontAttribute().getFamilyName(),
                    getFontAttribute().getStyleName(),
                    getFontAttribute().getWeight(),
                    getFontAttribute().getSymbol(),
                    getFontAttribute().getVertical(),
                    getFontAttribute().getItalic(),
                    false,             // no outline anymore, handled locally
                    getFontAttribute().getRTL(),
                    getFontAttribute().getBiDiStrong());

                // handle as one word
                impCreateGeometryContent(aNewPrimitives, aDecTrans, getText(), getTextPosition(), getTextLength(), getDXArray(), aNewFontAttribute);
            }

            // convert to Primitive2DSequence
            const sal_uInt32 nMemberCount(aNewPrimitives.size());

            if(nMemberCount)
            {
                aRetval.realloc(nMemberCount);

                for(sal_uInt32 a(0); a < nMemberCount; a++)
                {
                    aRetval[a] = aNewPrimitives[a];
                }
            }

            // Handle Shadow, Outline and TextRelief
            if(aRetval.hasElements())
            {
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
                        static double fFactor(1.0 / 24.0);
                        const double fTextShadowOffset(aDecTrans.getScale().getY() * fFactor);
                        static basegfx::BColor aShadowColor(0.3, 0.3, 0.3);

                        // preapare shadow transform matrix
                        const basegfx::B2DHomMatrix aShadowTransform(basegfx::tools::createTranslateB2DHomMatrix(
                            fTextShadowOffset, fTextShadowOffset));

                        // create shadow primitive
                        aShadow = Primitive2DReference(new ShadowPrimitive2D(
                            aShadowTransform,
                            aShadowColor,
                            aRetval));
                    }

                    if(bHasTextRelief)
                    {
                        // create emboss using an own helper primitive since this will
                        // be view-dependent
                        const basegfx::BColor aBBlack(0.0, 0.0, 0.0);
                        const bool bDefaultTextColor(aBBlack == getFontColor());
                        TextEffectStyle2D aTextEffectStyle2D(TEXTEFFECTSTYLE2D_RELIEF_EMBOSSED);

                        if(bDefaultTextColor)
                        {
                            if(TEXT_RELIEF_ENGRAVED == getTextRelief())
                            {
                                aTextEffectStyle2D = TEXTEFFECTSTYLE2D_RELIEF_ENGRAVED_DEFAULT;
                            }
                            else
                            {
                                aTextEffectStyle2D = TEXTEFFECTSTYLE2D_RELIEF_EMBOSSED_DEFAULT;
                            }
                        }
                        else
                        {
                            if(TEXT_RELIEF_ENGRAVED == getTextRelief())
                            {
                                aTextEffectStyle2D = TEXTEFFECTSTYLE2D_RELIEF_ENGRAVED;
                            }
                            else
                            {
                                aTextEffectStyle2D = TEXTEFFECTSTYLE2D_RELIEF_EMBOSSED;
                            }
                        }

                        Primitive2DReference aNewTextEffect(new TextEffectPrimitive2D(
                            aRetval,
                            aDecTrans.getTranslate(),
                            aDecTrans.getRotate(),
                            aTextEffectStyle2D));
                        aRetval = Primitive2DSequence(&aNewTextEffect, 1);
                    }
                    else if(bHasOutline)
                    {
                        // create outline using an own helper primitive since this will
                        // be view-dependent
                        Primitive2DReference aNewTextEffect(new TextEffectPrimitive2D(
                            aRetval,
                            aDecTrans.getTranslate(),
                            aDecTrans.getRotate(),
                            TEXTEFFECTSTYLE2D_OUTLINE));
                        aRetval = Primitive2DSequence(&aNewTextEffect, 1);
                    }

                    if(aShadow.is())
                    {
                        // put shadow in front if there is one to paint timely before
                        // but placed behind content
                        const Primitive2DSequence aContent(aRetval);
                        aRetval = Primitive2DSequence(&aShadow, 1);
                        appendPrimitive2DSequenceToPrimitive2DSequence(aRetval, aContent);
                    }
                }
            }

            return aRetval;
        }

        TextDecoratedPortionPrimitive2D::TextDecoratedPortionPrimitive2D(

            // TextSimplePortionPrimitive2D parameters
            const basegfx::B2DHomMatrix& rNewTransform,
            const String& rText,
            xub_StrLen aTextPosition,
            xub_StrLen aTextLength,
            const ::std::vector< double >& rDXArray,
            const attribute::FontAttribute& rFontAttribute,
            const ::com::sun::star::lang::Locale& rLocale,
            const basegfx::BColor& rFontColor,

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
        :   TextSimplePortionPrimitive2D(rNewTransform, rText, aTextPosition, aTextLength, rDXArray, rFontAttribute, rLocale, rFontColor),
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
                const TextDecoratedPortionPrimitive2D& rCompare = (TextDecoratedPortionPrimitive2D&)rPrimitive;

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
            const bool bDecoratedIsNeeded(
                TEXT_LINE_NONE != getFontOverline()
             || TEXT_LINE_NONE != getFontUnderline()
             || TEXT_STRIKEOUT_NONE != getTextStrikeout()
             || TEXT_EMPHASISMARK_NONE != getTextEmphasisMark()
             || TEXT_RELIEF_NONE != getTextRelief()
             || getShadow());

            if(bDecoratedIsNeeded)
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
        ImplPrimitrive2DIDBlock(TextDecoratedPortionPrimitive2D, PRIMITIVE2D_ID_TEXTDECORATEDPORTIONPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
