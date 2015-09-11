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

#include <drawinglayer/primitive2d/textbreakuphelper.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <com/sun/star/i18n/BreakIterator.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/i18n/CharacterIteratorMode.hdl>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/CharType.hpp>



namespace drawinglayer
{
    namespace primitive2d
    {
        TextBreakupHelper::TextBreakupHelper(const TextSimplePortionPrimitive2D& rSource)
        :   mrSource(rSource),
            mxResult(),
            maTextLayouter(),
            maDecTrans(),
            mbNoDXArray(false)
        {
            OSL_ENSURE(dynamic_cast< const TextSimplePortionPrimitive2D* >(&mrSource), "TextBreakupHelper with illegal primitive created (!)");
            maDecTrans = mrSource.getTextTransform();
            mbNoDXArray = mrSource.getDXArray().empty();

            if(mbNoDXArray)
            {
                // init TextLayouter when no dxarray
                maTextLayouter.setFontAttribute(
                    mrSource.getFontAttribute(),
                    maDecTrans.getScale().getX(),
                    maDecTrans.getScale().getY(),
                    mrSource.getLocale());
            }
        }

        TextBreakupHelper::~TextBreakupHelper()
        {
        }

        void TextBreakupHelper::breakupPortion(Primitive2DVector& rTempResult, sal_Int32 nIndex, sal_Int32 nLength, bool bWordLineMode)
        {
            if(nLength && !(nIndex == mrSource.getTextPosition() && nLength == mrSource.getTextLength()))
            {
                // prepare values for new portion
                basegfx::B2DHomMatrix aNewTransform;
                ::std::vector< double > aNewDXArray;
                const bool bNewStartIsNotOldStart(nIndex > mrSource.getTextPosition());

                if(!mbNoDXArray)
                {
                    // prepare new DXArray for the single word
                    aNewDXArray = ::std::vector< double >(
                        mrSource.getDXArray().begin() + (nIndex - mrSource.getTextPosition()),
                        mrSource.getDXArray().begin() + ((nIndex + nLength) - mrSource.getTextPosition()));
                }

                if(bNewStartIsNotOldStart)
                {
                    // needs to be moved to a new start position
                    double fOffset(0.0);

                    if(mbNoDXArray)
                    {
                        // evaluate using TextLayouter
                        fOffset = maTextLayouter.getTextWidth(mrSource.getText(), mrSource.getTextPosition(), nIndex);
                    }
                    else
                    {
                        // get from DXArray
                        const sal_Int32 nIndex2(nIndex - mrSource.getTextPosition());
                        fOffset = mrSource.getDXArray()[nIndex2 - 1];
                    }

                    // need offset without FontScale for building the new transformation. The
                    // new transformation will be multiplied with the current text transformation
                    // so FontScale would be double
                    double fOffsetNoScale(fOffset);
                    const double fFontScaleX(maDecTrans.getScale().getX());

                    if(!basegfx::fTools::equal(fFontScaleX, 1.0)
                        && !basegfx::fTools::equalZero(fFontScaleX))
                    {
                        fOffsetNoScale /= fFontScaleX;
                    }

                    // apply needed offset to transformation
                    aNewTransform.translate(fOffsetNoScale, 0.0);

                    if(!mbNoDXArray)
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
                aNewTransform = maDecTrans.getB2DHomMatrix() * aNewTransform;

                // callback to allow evtl. changes
                const bool bCreate(allowChange(rTempResult.size(), aNewTransform, nIndex, nLength));

                if(bCreate)
                {
                    // check if we have a decorated primitive as source
                    const TextDecoratedPortionPrimitive2D* pTextDecoratedPortionPrimitive2D =
                        dynamic_cast< const TextDecoratedPortionPrimitive2D* >(&mrSource);

                    if(pTextDecoratedPortionPrimitive2D)
                    {
                        // create a TextDecoratedPortionPrimitive2D
                        rTempResult.push_back(
                            new TextDecoratedPortionPrimitive2D(
                                aNewTransform,
                                mrSource.getText(),
                                nIndex,
                                nLength,
                                aNewDXArray,
                                mrSource.getFontAttribute(),
                                mrSource.getLocale(),
                                mrSource.getFontColor(),
                                mrSource.getTextFillColor(),

                                pTextDecoratedPortionPrimitive2D->getOverlineColor(),
                                pTextDecoratedPortionPrimitive2D->getTextlineColor(),
                                pTextDecoratedPortionPrimitive2D->getFontOverline(),
                                pTextDecoratedPortionPrimitive2D->getFontUnderline(),
                                pTextDecoratedPortionPrimitive2D->getUnderlineAbove(),
                                pTextDecoratedPortionPrimitive2D->getTextStrikeout(),

                                // reset WordLineMode when BreakupUnit_word is executed; else copy original
                                !bWordLineMode && pTextDecoratedPortionPrimitive2D->getWordLineMode(),

                                pTextDecoratedPortionPrimitive2D->getTextEmphasisMark(),
                                pTextDecoratedPortionPrimitive2D->getEmphasisMarkAbove(),
                                pTextDecoratedPortionPrimitive2D->getEmphasisMarkBelow(),
                                pTextDecoratedPortionPrimitive2D->getTextRelief(),
                                pTextDecoratedPortionPrimitive2D->getShadow()));
                    }
                    else
                    {
                        // create a SimpleTextPrimitive
                        rTempResult.push_back(
                            new TextSimplePortionPrimitive2D(
                                aNewTransform,
                                mrSource.getText(),
                                nIndex,
                                nLength,
                                aNewDXArray,
                                mrSource.getFontAttribute(),
                                mrSource.getLocale(),
                                mrSource.getFontColor()));
                    }
                }
            }
        }

        bool TextBreakupHelper::allowChange(sal_uInt32 /*nCount*/, basegfx::B2DHomMatrix& /*rNewTransform*/, sal_uInt32 /*nIndex*/, sal_uInt32 /*nLength*/)
        {
            return true;
        }

        void TextBreakupHelper::breakup(BreakupUnit aBreakupUnit)
        {
            if(mrSource.getTextLength())
            {
                Primitive2DVector aTempResult;
                static css::uno::Reference< css::i18n::XBreakIterator > xBreakIterator;

                if(!xBreakIterator.is())
                {
                    css::uno::Reference< css::uno::XComponentContext > xContext( ::comphelper::getProcessComponentContext() );
                    xBreakIterator = css::i18n::BreakIterator::create(xContext);
                }

                const OUString& rTxt = mrSource.getText();
                const sal_Int32 nTextLength(mrSource.getTextLength());
                const css::lang::Locale& rLocale = mrSource.getLocale();
                const sal_Int32 nTextPosition(mrSource.getTextPosition());
                sal_Int32 nCurrent(nTextPosition);

                switch(aBreakupUnit)
                {
                    case BreakupUnit_character:
                    {
                        sal_Int32 nDone;
                        sal_Int32 nNextCellBreak(xBreakIterator->nextCharacters(rTxt, nTextPosition, rLocale, css::i18n::CharacterIteratorMode::SKIPCELL, 0, nDone));
                        sal_Int32 a(nTextPosition);

                        for(; a < nTextPosition + nTextLength; a++)
                        {
                            if(a == nNextCellBreak)
                            {
                                breakupPortion(aTempResult, nCurrent, a - nCurrent, false);
                                nCurrent = a;
                                nNextCellBreak = xBreakIterator->nextCharacters(rTxt, a, rLocale, css::i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
                            }
                        }

                        breakupPortion(aTempResult, nCurrent, a - nCurrent, false);
                        break;
                    }
                    case BreakupUnit_word:
                    {
                        css::i18n::Boundary nNextWordBoundary(xBreakIterator->getWordBoundary(rTxt, nTextPosition, rLocale, css::i18n::WordType::ANY_WORD, sal_True));
                        sal_Int32 a(nTextPosition);

                        for(; a < nTextPosition + nTextLength; a++)
                        {
                            if(a == nNextWordBoundary.endPos)
                            {
                                if(a > nCurrent)
                                {
                                    breakupPortion(aTempResult, nCurrent, a - nCurrent, true);
                                }

                                nCurrent = a;

                                // skip spaces (maybe enhanced with a bool later if needed)
                                {
                                    const sal_Int32 nEndOfSpaces(xBreakIterator->endOfCharBlock(rTxt, a, rLocale, css::i18n::CharType::SPACE_SEPARATOR));

                                    if(nEndOfSpaces > a)
                                    {
                                        nCurrent = nEndOfSpaces;
                                    }
                                }

                                nNextWordBoundary = xBreakIterator->getWordBoundary(rTxt, a + 1, rLocale, css::i18n::WordType::ANY_WORD, sal_True);
                            }
                        }

                        if(a > nCurrent)
                        {
                            breakupPortion(aTempResult, nCurrent, a - nCurrent, true);
                        }
                        break;
                    }
                    case BreakupUnit_sentence:
                    {
                        sal_Int32 nNextSentenceBreak(xBreakIterator->endOfSentence(rTxt, nTextPosition, rLocale));
                        sal_Int32 a(nTextPosition);

                        for(; a < nTextPosition + nTextLength; a++)
                        {
                            if(a == nNextSentenceBreak)
                            {
                                breakupPortion(aTempResult, nCurrent, a - nCurrent, false);
                                nCurrent = a;
                                nNextSentenceBreak = xBreakIterator->endOfSentence(rTxt, a + 1, rLocale);
                            }
                        }

                        breakupPortion(aTempResult, nCurrent, a - nCurrent, false);
                        break;
                    }
                }

                mxResult = Primitive2DVectorToPrimitive2DSequence(aTempResult);
            }
        }

        const Primitive2DSequence& TextBreakupHelper::getResult(BreakupUnit aBreakupUnit) const
        {
            if(!mxResult.hasElements())
            {
                const_cast< TextBreakupHelper* >(this)->breakup(aBreakupUnit);
            }

            return mxResult;
        }

    } // end of namespace primitive2d
} // end of namespace drawinglayer

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
