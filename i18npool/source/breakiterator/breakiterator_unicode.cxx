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

#include <breakiterator_unicode.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <localedata.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/languagetagicu.hxx>
#include <unicode/uchar.h>
#include <unicode/locid.h>
#include <unicode/rbbi.h>
#include <unicode/udata.h>
#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>
#include <string.h>

U_CDECL_BEGIN
extern const char OpenOffice_dat[];
U_CDECL_END

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;

namespace com { namespace sun { namespace star { namespace i18n {


BreakIterator_Unicode::BreakIterator_Unicode()
    : cBreakIterator( "com.sun.star.i18n.BreakIterator_Unicode" )    // implementation name
    , lineRule( "line" )
    , icuBI( nullptr )
{
}

BreakIterator_Unicode::~BreakIterator_Unicode()
{
    delete character.aBreakIterator;
    delete sentence.aBreakIterator;
    delete line.aBreakIterator;
    for (BI_Data & word : words)
        delete word.aBreakIterator;
}

/*
    Wrapper class to provide public access to the RuleBasedBreakIterator's
    setbreakType method.
*/
class OOoRuleBasedBreakIterator : public RuleBasedBreakIterator
{
    public:
    inline void publicSetBreakType(int32_t type)
        {
            setBreakType(type);
        };
    OOoRuleBasedBreakIterator(UDataMemory* image,
                              UErrorCode &status)
        : RuleBasedBreakIterator(image, status)
        { };

};

// loading ICU breakiterator on demand.
void SAL_CALL BreakIterator_Unicode::loadICUBreakIterator(const css::lang::Locale& rLocale,
        sal_Int16 rBreakType, sal_Int16 nWordType, const sal_Char *rule, const OUString& rText) throw(uno::RuntimeException)
{
    bool bNewBreak = false;
    UErrorCode status = U_ZERO_ERROR;
    sal_Int16 breakType = 0;
    switch (rBreakType) {
        case LOAD_CHARACTER_BREAKITERATOR: icuBI=&character; breakType = 3; break;
        case LOAD_WORD_BREAKITERATOR:
            assert (nWordType >= 0 && nWordType<= WordType::WORD_COUNT);
            icuBI=&words[nWordType];
            switch (nWordType) {
                case WordType::ANY_WORD: break; // odd but previous behavior
                case WordType::ANYWORD_IGNOREWHITESPACES:
                    breakType = 0; rule = "edit_word"; break;
                case WordType::DICTIONARY_WORD:
                    breakType = 1; rule = "dict_word"; break;
                default:
                case WordType::WORD_COUNT:
                    breakType = 2; rule = "count_word"; break;
            }
            break;
        case LOAD_SENTENCE_BREAKITERATOR: icuBI=&sentence; breakType = 5; break;
        case LOAD_LINE_BREAKITERATOR: icuBI=&line; breakType = 4; break;
    }
    if (!icuBI->aBreakIterator ||
        rLocale.Language != icuBI->maLocale.Language ||
        rLocale.Country  != icuBI->maLocale.Country  ||
        rLocale.Variant  != icuBI->maLocale.Variant) {
        if (icuBI->aBreakIterator) {
            delete icuBI->aBreakIterator;
            icuBI->aBreakIterator=nullptr;
        }
        if (rule) {
            uno::Sequence< OUString > breakRules = LocaleDataImpl().getBreakIteratorRules(rLocale);

            status = U_ZERO_ERROR;
            udata_setAppData("OpenOffice", OpenOffice_dat, &status);
            if ( !U_SUCCESS(status) ) throw uno::RuntimeException();

            OOoRuleBasedBreakIterator *rbi = nullptr;

            if (breakRules.getLength() > breakType && !breakRules[breakType].isEmpty())
            {
                rbi = new OOoRuleBasedBreakIterator(udata_open("OpenOffice", "brk",
                    OUStringToOString(breakRules[breakType], RTL_TEXTENCODING_ASCII_US).getStr(), &status), status);
            }
            //use icu's breakiterator for Thai, Tibetan and Dzongkha
            else if (rLocale.Language != "th" && rLocale.Language != "lo" && rLocale.Language != "bo" && rLocale.Language != "dz" && rLocale.Language != "km")
            {
                status = U_ZERO_ERROR;
                OStringBuffer aUDName(64);
                aUDName.append(rule);
                aUDName.append('_');
                aUDName.append( OUStringToOString(rLocale.Language, RTL_TEXTENCODING_ASCII_US));
                UDataMemory* pUData = udata_open("OpenOffice", "brk", aUDName.getStr(), &status);
                if( U_SUCCESS(status) )
                    rbi = new OOoRuleBasedBreakIterator( pUData, status);
                if (!U_SUCCESS(status) ) {
                    status = U_ZERO_ERROR;
                    pUData = udata_open("OpenOffice", "brk", rule, &status);
                    if( U_SUCCESS(status) )
                        rbi = new OOoRuleBasedBreakIterator( pUData, status);
                    if (!U_SUCCESS(status) ) icuBI->aBreakIterator=nullptr;
                }
            }
            if (rbi) {
                switch (rBreakType) {
                    case LOAD_CHARACTER_BREAKITERATOR: rbi->publicSetBreakType(UBRK_CHARACTER); break;
                    case LOAD_WORD_BREAKITERATOR: rbi->publicSetBreakType(UBRK_WORD); break;
                    case LOAD_SENTENCE_BREAKITERATOR: rbi->publicSetBreakType(UBRK_SENTENCE); break;
                    case LOAD_LINE_BREAKITERATOR: rbi->publicSetBreakType(UBRK_LINE); break;
                }
                icuBI->aBreakIterator = rbi;
            }
        }

        if (!icuBI->aBreakIterator) {
            icu::Locale icuLocale( LanguageTagIcu::getIcuLocale( LanguageTag( rLocale)));

            status = U_ZERO_ERROR;
            switch (rBreakType) {
                case LOAD_CHARACTER_BREAKITERATOR:
                    icuBI->aBreakIterator =  icu::BreakIterator::createCharacterInstance(icuLocale, status);
                    break;
                case LOAD_WORD_BREAKITERATOR:
                    icuBI->aBreakIterator =  icu::BreakIterator::createWordInstance(icuLocale, status);
                    break;
                case LOAD_SENTENCE_BREAKITERATOR:
                    icuBI->aBreakIterator = icu::BreakIterator::createSentenceInstance(icuLocale, status);
                    break;
                case LOAD_LINE_BREAKITERATOR:
                    icuBI->aBreakIterator = icu::BreakIterator::createLineInstance(icuLocale, status);
                    break;
            }
            if ( !U_SUCCESS(status) ) {
                icuBI->aBreakIterator=nullptr;
                throw uno::RuntimeException();
            }
        }
        if (icuBI->aBreakIterator) {
            icuBI->maLocale=rLocale;
            bNewBreak=true;
        } else {
            throw uno::RuntimeException();
        }
    }

    if (bNewBreak || icuBI->aICUText.pData != rText.pData)
    {
        // UChar != sal_Unicode in MinGW
        const UChar *pText = reinterpret_cast<const UChar *>(rText.getStr());

        icuBI->ut = utext_openUChars(icuBI->ut, pText, rText.getLength(), &status);

        if (!U_SUCCESS(status))
            throw uno::RuntimeException();

        icuBI->aBreakIterator->setText(icuBI->ut, status);

        if (!U_SUCCESS(status))
            throw uno::RuntimeException();

        icuBI->aICUText = rText;
    }
}

sal_Int32 SAL_CALL BreakIterator_Unicode::nextCharacters( const OUString& Text,
        sal_Int32 nStartPos, const lang::Locale &rLocale,
        sal_Int16 nCharacterIteratorMode, sal_Int32 nCount, sal_Int32& nDone )
        throw(uno::RuntimeException, std::exception)
{
    if (nCharacterIteratorMode == CharacterIteratorMode::SKIPCELL ) { // for CELL mode
        loadICUBreakIterator(rLocale, LOAD_CHARACTER_BREAKITERATOR, 0, "char", Text);
        for (nDone = 0; nDone < nCount; nDone++) {
            nStartPos = character.aBreakIterator->following(nStartPos);
            if (nStartPos == BreakIterator::DONE)
                return Text.getLength();
        }
    } else { // for CHARACTER mode
        for (nDone = 0; nDone < nCount && nStartPos < Text.getLength(); nDone++)
            Text.iterateCodePoints(&nStartPos);
    }
    return nStartPos;
}

sal_Int32 SAL_CALL BreakIterator_Unicode::previousCharacters( const OUString& Text,
        sal_Int32 nStartPos, const lang::Locale& rLocale,
        sal_Int16 nCharacterIteratorMode, sal_Int32 nCount, sal_Int32& nDone )
        throw(uno::RuntimeException, std::exception)
{
    if (nCharacterIteratorMode == CharacterIteratorMode::SKIPCELL ) { // for CELL mode
        loadICUBreakIterator(rLocale, LOAD_CHARACTER_BREAKITERATOR, 0, "char", Text);
        for (nDone = 0; nDone < nCount; nDone++) {
            nStartPos = character.aBreakIterator->preceding(nStartPos);
            if (nStartPos == BreakIterator::DONE)
                return 0;
        }
    } else { // for BS to delete one char and CHARACTER mode.
        for (nDone = 0; nDone < nCount && nStartPos > 0; nDone++)
            Text.iterateCodePoints(&nStartPos, -1);
    }
    return nStartPos;
}


Boundary SAL_CALL BreakIterator_Unicode::nextWord( const OUString& Text, sal_Int32 nStartPos,
    const lang::Locale& rLocale, sal_Int16 rWordType ) throw(uno::RuntimeException, std::exception)
{
    loadICUBreakIterator(rLocale, LOAD_WORD_BREAKITERATOR, rWordType, nullptr, Text);

    result.startPos = icuBI->aBreakIterator->following(nStartPos);
    if( result.startPos >= Text.getLength() || result.startPos == BreakIterator::DONE )
        result.endPos = result.startPos;
    else {
        if ( (rWordType == WordType::ANYWORD_IGNOREWHITESPACES ||
                    rWordType == WordType::DICTIONARY_WORD ) &&
                u_isWhitespace(Text.iterateCodePoints(&result.startPos, 0)) )
            result.startPos = icuBI->aBreakIterator->following(result.startPos);

        result.endPos = icuBI->aBreakIterator->following(result.startPos);
        if(result.endPos == BreakIterator::DONE)
            result.endPos = result.startPos;
    }
    return result;
}


Boundary SAL_CALL BreakIterator_Unicode::previousWord(const OUString& Text, sal_Int32 nStartPos,
        const lang::Locale& rLocale, sal_Int16 rWordType) throw(uno::RuntimeException, std::exception)
{
    loadICUBreakIterator(rLocale, LOAD_WORD_BREAKITERATOR, rWordType, nullptr, Text);

    result.startPos = icuBI->aBreakIterator->preceding(nStartPos);
    if( result.startPos < 0 || result.startPos == BreakIterator::DONE)
        result.endPos = result.startPos;
    else {
        if ( (rWordType == WordType::ANYWORD_IGNOREWHITESPACES ||
                    rWordType == WordType::DICTIONARY_WORD) &&
                u_isWhitespace(Text.iterateCodePoints(&result.startPos, 0)) )
            result.startPos = icuBI->aBreakIterator->preceding(result.startPos);

        result.endPos = icuBI->aBreakIterator->following(result.startPos);
        if(result.endPos == BreakIterator::DONE)
            result.endPos = result.startPos;
    }
    return result;
}


Boundary SAL_CALL BreakIterator_Unicode::getWordBoundary( const OUString& Text, sal_Int32 nPos, const lang::Locale& rLocale,
        sal_Int16 rWordType, sal_Bool bDirection ) throw(uno::RuntimeException, std::exception)
{
    loadICUBreakIterator(rLocale, LOAD_WORD_BREAKITERATOR, rWordType, nullptr, Text);
    sal_Int32 len = Text.getLength();

    if(icuBI->aBreakIterator->isBoundary(nPos)) {
        result.startPos = result.endPos = nPos;
        if((bDirection || nPos == 0) && nPos < len) //forward
            result.endPos = icuBI->aBreakIterator->following(nPos);
        else
            result.startPos = icuBI->aBreakIterator->preceding(nPos);
    } else {
        if(nPos <= 0) {
            result.startPos = 0;
            result.endPos = len ? icuBI->aBreakIterator->following((sal_Int32)0) : 0;
        } else if(nPos >= len) {
            result.startPos = icuBI->aBreakIterator->preceding(len);
            result.endPos = len;
        } else {
            result.startPos = icuBI->aBreakIterator->preceding(nPos);
            result.endPos = icuBI->aBreakIterator->following(nPos);
        }
    }
    if (result.startPos == BreakIterator::DONE)
        result.startPos = result.endPos;
    else if (result.endPos == BreakIterator::DONE)
        result.endPos = result.startPos;

    return result;
}


sal_Int32 SAL_CALL BreakIterator_Unicode::beginOfSentence( const OUString& Text, sal_Int32 nStartPos,
        const lang::Locale &rLocale ) throw(uno::RuntimeException, std::exception)
{
    loadICUBreakIterator(rLocale, LOAD_SENTENCE_BREAKITERATOR, 0, "sent", Text);

    sal_Int32 len = Text.getLength();
    if (len > 0 && nStartPos == len)
        Text.iterateCodePoints(&nStartPos, -1); // issue #i27703# treat end position as part of last sentence
    if (!sentence.aBreakIterator->isBoundary(nStartPos))
        nStartPos = sentence.aBreakIterator->preceding(nStartPos);

    // skip preceding space.
    sal_uInt32 ch = Text.iterateCodePoints(&nStartPos);
    while (nStartPos < len && u_isWhitespace(ch)) ch = Text.iterateCodePoints(&nStartPos);
    Text.iterateCodePoints(&nStartPos, -1);

    return nStartPos;
}

sal_Int32 SAL_CALL BreakIterator_Unicode::endOfSentence( const OUString& Text, sal_Int32 nStartPos,
        const lang::Locale &rLocale ) throw(uno::RuntimeException, std::exception)
{
    loadICUBreakIterator(rLocale, LOAD_SENTENCE_BREAKITERATOR, 0, "sent", Text);

    sal_Int32 len = Text.getLength();
    if (len > 0 && nStartPos == len)
        Text.iterateCodePoints(&nStartPos, -1); // issue #i27703# treat end position as part of last sentence
    nStartPos = sentence.aBreakIterator->following(nStartPos);

    sal_Int32 nPos=nStartPos;
    while (nPos > 0 && u_isWhitespace(Text.iterateCodePoints(&nPos, -1))) nStartPos=nPos;

    return nStartPos;
}

LineBreakResults SAL_CALL BreakIterator_Unicode::getLineBreak(
        const OUString& Text, sal_Int32 nStartPos,
        const lang::Locale& rLocale, sal_Int32 nMinBreakPos,
        const LineBreakHyphenationOptions& hOptions,
        const LineBreakUserOptions& /*rOptions*/ ) throw(uno::RuntimeException, std::exception)
{
    LineBreakResults lbr;

    if (nStartPos >= Text.getLength()) {
        lbr.breakIndex = Text.getLength();
        lbr.breakType = BreakType::WORDBOUNDARY;
        return lbr;
    }

    loadICUBreakIterator(rLocale, LOAD_LINE_BREAKITERATOR, 0, lineRule, Text);

    bool GlueSpace=true;
    while (GlueSpace) {
        if (line.aBreakIterator->preceding(nStartPos + 1) == nStartPos) { //Line boundary break
            lbr.breakIndex = nStartPos;
            lbr.breakType = BreakType::WORDBOUNDARY;
        } else if (hOptions.rHyphenator.is()) { //Hyphenation break
            sal_Int32 boundary_with_punctuation = (line.aBreakIterator->next() != BreakIterator::DONE) ? line.aBreakIterator->current() : 0;
            line.aBreakIterator->preceding(nStartPos + 1); // reset to check correct hyphenation of "word-word"

            sal_Int32 nStartPosWordEnd = nStartPos;
            while (line.aBreakIterator->current() < nStartPosWordEnd && u_ispunct((sal_uInt32)Text[nStartPosWordEnd])) // starting punctuation
                nStartPosWordEnd --;

            Boundary wBoundary = getWordBoundary( Text, nStartPosWordEnd, rLocale,
                WordType::DICTIONARY_WORD, false);

            nStartPosWordEnd = wBoundary.endPos;
            while (nStartPosWordEnd < Text.getLength() && (u_ispunct((sal_uInt32)Text[nStartPosWordEnd]))) // ending punctuation
                nStartPosWordEnd ++;
            nStartPosWordEnd = nStartPosWordEnd - wBoundary.endPos;
            if (hOptions.hyphenIndex - wBoundary.startPos < nStartPosWordEnd) nStartPosWordEnd = hOptions.hyphenIndex - wBoundary.startPos;
#define SPACE 0x0020
            while (boundary_with_punctuation > wBoundary.endPos && Text[--boundary_with_punctuation] == SPACE);
            uno::Reference< linguistic2::XHyphenatedWord > aHyphenatedWord;
            aHyphenatedWord = hOptions.rHyphenator->hyphenate(Text.copy(wBoundary.startPos,
                        wBoundary.endPos - wBoundary.startPos), rLocale,
                    (sal_Int16) (hOptions.hyphenIndex - wBoundary.startPos - ((hOptions.hyphenIndex == wBoundary.endPos)? nStartPosWordEnd : 0)), hOptions.aHyphenationOptions);
            if (aHyphenatedWord.is()) {
                lbr.rHyphenatedWord = aHyphenatedWord;
                if(wBoundary.startPos + aHyphenatedWord->getHyphenationPos() + 1 < nMinBreakPos )
                    lbr.breakIndex = -1;
                else
                    lbr.breakIndex = wBoundary.startPos; //aHyphenatedWord->getHyphenationPos();
                lbr.breakType = BreakType::HYPHENATION;

                // check not optimal hyphenation of "word-word" (word with hyphens)
                if (lbr.breakIndex > -1 && wBoundary.startPos + aHyphenatedWord->getHyphenationPos() < line.aBreakIterator->current()) {
                    lbr.breakIndex = line.aBreakIterator->current();
                    lbr.breakType = BreakType::WORDBOUNDARY;
                }

            } else {
                lbr.breakIndex = line.aBreakIterator->preceding(nStartPos);
                lbr.breakType = BreakType::WORDBOUNDARY;
            }
        } else { //word boundary break
            lbr.breakIndex = line.aBreakIterator->preceding(nStartPos);
            lbr.breakType = BreakType::WORDBOUNDARY;
        }

#define WJ 0x2060   // Word Joiner
        GlueSpace=false;
        if (lbr.breakType == BreakType::WORDBOUNDARY) {
            nStartPos = lbr.breakIndex;
            if (nStartPos >= 0 && Text[nStartPos--] == WJ)
                GlueSpace=true;
            while (nStartPos >= 0 &&
                    (u_isWhitespace(Text.iterateCodePoints(&nStartPos, 0)) || Text[nStartPos] == WJ)) {
                if (Text[nStartPos--] == WJ)
                    GlueSpace=true;
            }
            if (GlueSpace && nStartPos < 0)  {
                lbr.breakIndex = 0;
                break;
            }
        }
    }

    return lbr;
}

OUString SAL_CALL
BreakIterator_Unicode::getImplementationName() throw( uno::RuntimeException, std::exception )
{
    return OUString::createFromAscii(cBreakIterator);
}

sal_Bool SAL_CALL
BreakIterator_Unicode::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
BreakIterator_Unicode::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
{
    uno::Sequence< OUString > aRet { OUString::createFromAscii(cBreakIterator) };
    return aRet;
}

} } } }

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_i18n_BreakIterator_Unicode_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new css::i18n::BreakIterator_Unicode());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
