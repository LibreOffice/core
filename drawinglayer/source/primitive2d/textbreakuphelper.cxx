/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http:\\www.apache.org\licenses\LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_drawinglayer.hxx"

#include <drawinglayer/primitive2d/textbreakuphelper.hxx>
#include <drawinglayer/primitive2d/textdecoratedprimitive2d.hxx>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/i18n/CharacterIteratorMode.hdl>
#include <com/sun/star/i18n/WordType.hpp>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        TextBreakupHelper::TextBreakupHelper(const Primitive2DReference& rxSource)
        :   mxSource(rxSource),
            mxResult(),
            mpSource(dynamic_cast< const TextSimplePortionPrimitive2D* >(rxSource.get())),
            maTextLayouter(),
            maDecTrans(),
            mbNoDXArray()
        {
            if(mpSource)
            {
                maDecTrans = mpSource->getTextTransform();
                mbNoDXArray = mpSource->getDXArray().empty();

                if(mbNoDXArray)
                {
                    // init TextLayouter when no dxarray
                    maTextLayouter.setFontAttribute(
                        mpSource->getFontAttribute(),
                        maDecTrans.getScale().getX(),
                        maDecTrans.getScale().getY(),
                        mpSource->getLocale());
                }
            }
        }

        TextBreakupHelper::~TextBreakupHelper()
        {
        }

        void TextBreakupHelper::breakupPortion(Primitive2DVector& rTempResult, sal_uInt32 nIndex, sal_uInt32 nLength)
        {
            if(mpSource && nLength && !(nIndex == mpSource->getTextPosition() && nLength == mpSource->getTextLength()))
            {
                // prepare values for new portion
                basegfx::B2DHomMatrix aNewTransform;
                ::std::vector< double > aNewDXArray;
                const bool bNewStartIsNotOldStart(nIndex > mpSource->getTextPosition());

                if(!mbNoDXArray)
                {
                    // prepare new DXArray for the single word
                    aNewDXArray = ::std::vector< double >(
                        mpSource->getDXArray().begin() + (nIndex - mpSource->getTextPosition()),
                        mpSource->getDXArray().begin() + ((nIndex + nLength) - mpSource->getTextPosition()));
                }

                if(bNewStartIsNotOldStart)
                {
                    // needs to be moved to a new start position
                    double fOffset(0.0);

                    if(mbNoDXArray)
                    {
                        // evaluate using TextLayouter
                        fOffset = maTextLayouter.getTextWidth(mpSource->getText(), mpSource->getTextPosition(), nIndex);
                    }
                    else
                    {
                        // get from DXArray
                        const sal_uInt32 nIndex2(static_cast< sal_uInt32 >(nIndex - mpSource->getTextPosition()));
                        fOffset = mpSource->getDXArray()[nIndex2 - 1];
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
                        dynamic_cast< const TextDecoratedPortionPrimitive2D* >(mpSource);

                    if(pTextDecoratedPortionPrimitive2D)
                    {
                        // create a TextDecoratedPortionPrimitive2D
                        rTempResult.push_back(
                            new TextDecoratedPortionPrimitive2D(
                                aNewTransform,
                                mpSource->getText(),
                                nIndex,
                                nLength,
                                aNewDXArray,
                                mpSource->getFontAttribute(),
                                mpSource->getLocale(),
                                mpSource->getFontColor(),

                                pTextDecoratedPortionPrimitive2D->getOverlineColor(),
                                pTextDecoratedPortionPrimitive2D->getTextlineColor(),
                                pTextDecoratedPortionPrimitive2D->getFontOverline(),
                                pTextDecoratedPortionPrimitive2D->getFontUnderline(),
                                pTextDecoratedPortionPrimitive2D->getUnderlineAbove(),
                                pTextDecoratedPortionPrimitive2D->getTextStrikeout(),
                                pTextDecoratedPortionPrimitive2D->getWordLineMode(),
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
                                mpSource->getText(),
                                nIndex,
                                nLength,
                                aNewDXArray,
                                mpSource->getFontAttribute(),
                                mpSource->getLocale(),
                                mpSource->getFontColor()));
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
            if(mpSource && mpSource->getTextLength())
            {
                Primitive2DVector aTempResult;
                static ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator > xBreakIterator;

                if(!xBreakIterator.is())
                {
                    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xMSF(::comphelper::getProcessServiceFactory());
                    xBreakIterator.set(xMSF->createInstance(rtl::OUString::createFromAscii("com.sun.star.i18n.BreakIterator")), ::com::sun::star::uno::UNO_QUERY);
                }

                if(xBreakIterator.is())
                {
                    const rtl::OUString& rTxt = mpSource->getText();
                    const sal_Int32 nTextLength(mpSource->getTextLength());
                    const ::com::sun::star::lang::Locale& rLocale = mpSource->getLocale();
                    const sal_Int32 nTextPosition(mpSource->getTextPosition());
                    sal_Int32 nCurrent(nTextPosition);

                    switch(aBreakupUnit)
                    {
                        case BreakupUnit_character:
                        {
                            sal_Int32 nDone;
                            sal_Int32 nNextCellBreak(xBreakIterator->nextCharacters(rTxt, nTextPosition, rLocale, ::com::sun::star::i18n::CharacterIteratorMode::SKIPCELL, 0, nDone));
                            sal_Int32 a(nTextPosition);

                            for(; a < nTextPosition + nTextLength; a++)
                            {
                                if(a == nNextCellBreak)
                                {
                                    breakupPortion(aTempResult, nCurrent, a - nCurrent);
                                    nCurrent = a;
                                    nNextCellBreak = xBreakIterator->nextCharacters(rTxt, a, rLocale, ::com::sun::star::i18n::CharacterIteratorMode::SKIPCELL, 1, nDone);
                                }
                            }

                            breakupPortion(aTempResult, nCurrent, a - nCurrent);
                            break;
                        }
                        case BreakupUnit_word:
                        {
                            ::com::sun::star::i18n::Boundary nNextWordBoundary(xBreakIterator->getWordBoundary(rTxt, nTextPosition, rLocale, ::com::sun::star::i18n::WordType::ANY_WORD, sal_True));
                            sal_Int32 a(nTextPosition);

                            for(; a < nTextPosition + nTextLength; a++)
                            {
                                if(a == nNextWordBoundary.endPos)
                                {
                                    breakupPortion(aTempResult, nCurrent, a - nCurrent);
                                    nCurrent = a;
                                    nNextWordBoundary = xBreakIterator->getWordBoundary(rTxt, a + 1, rLocale, ::com::sun::star::i18n::WordType::ANY_WORD, sal_True);
                                }
                            }

                            breakupPortion(aTempResult, nCurrent, a - nCurrent);
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
                                    breakupPortion(aTempResult, nCurrent, a - nCurrent);
                                    nCurrent = a;
                                    nNextSentenceBreak = xBreakIterator->endOfSentence(rTxt, a + 1, rLocale);
                                }
                            }

                            breakupPortion(aTempResult, nCurrent, a - nCurrent);
                            break;
                        }
                    }
                }

                mxResult = Primitive2DVectorToPrimitive2DSequence(aTempResult);
            }
        }

        const Primitive2DSequence& TextBreakupHelper::getResult(BreakupUnit aBreakupUnit) const
        {
            if(mxResult.hasElements())
            {
                return mxResult;
            }
            else if(mpSource)
            {
                const_cast< TextBreakupHelper* >(this)->breakup(aBreakupUnit);
            }

            return mxResult;
        }

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof
