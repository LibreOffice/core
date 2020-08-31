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

#include <com/sun/star/i18n/BreakType.hpp>
#include <com/sun/star/i18n/CharacterIteratorMode.hpp>
#include <com/sun/star/i18n/WordType.hpp>

U_CDECL_BEGIN
extern const char OpenOffice_dat[];
U_CDECL_END

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;

namespace i18npool {

// Cache map of breakiterators, stores state information so has to be
// thread_local.
thread_local static BreakIterator_Unicode::BIMap theBIMap;

BreakIterator_Unicode::BreakIterator_Unicode()
    : cBreakIterator( "com.sun.star.i18n.BreakIterator_Unicode" )    // implementation name
    , lineRule( "line" )
    , icuBI( nullptr )
{
}

BreakIterator_Unicode::~BreakIterator_Unicode()
{
}

namespace {

/*
    Wrapper class to provide public access to the icu::RuleBasedBreakIterator's
    setbreakType method.
*/
class OOoRuleBasedBreakIterator : public icu::RuleBasedBreakIterator
{
    public:
#if (U_ICU_VERSION_MAJOR_NUM < 58)
    // icu::RuleBasedBreakIterator::setBreakType() is private as of ICU 58.
    void publicSetBreakType(int32_t type)
        {
            setBreakType(type);
        };
#endif
    OOoRuleBasedBreakIterator(UDataMemory* image,
                              UErrorCode &status)
        : icu::RuleBasedBreakIterator(image, status)
        { };

};

}

// loading ICU breakiterator on demand.
void BreakIterator_Unicode::loadICUBreakIterator(const css::lang::Locale& rLocale,
        sal_Int16 rBreakType, sal_Int16 nWordType, const char *rule, const OUString& rText)
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

    // Using the cache map prevents accessing the file system for each
    // udata_open() where ICU tries first files then data objects. And that for
    // two fallbacks worst case... for each new allocated EditEngine, layout
    // cell, ... *ouch*  Also non-rule locale based iterators can be mapped.
    // This also speeds up loading iterators for alternating or generally more
    // than one language/locale in that iterators are not constructed and
    // destroyed en masse.
    // Four possible keys, locale rule based with break type, locale rule based
    // only, rule based only, locale based with break type. A fifth global key
    // for the initial lookup.
    // Multiple global keys may map to identical value data.
    // All enums used here should be in the range 0..9 so assert that and avoid
    // expensive numeric conversion in append() for faster construction of the
    // always used global key.
    assert( 0 <= breakType && breakType <= 9 && 0 <= rBreakType && rBreakType <= 9 && 0 <= nWordType && nWordType <= 9);
    const OString aLangtagStr( LanguageTag::convertToBcp47( rLocale).toUtf8());
    OStringBuffer aKeyBuf(64);
    aKeyBuf.append( aLangtagStr).append(';');
    if (rule)
        aKeyBuf.append(rule);
    aKeyBuf.append(';').append( static_cast<char>('0'+breakType)).append(';').
        append( static_cast<char>('0'+rBreakType)).append(';').append( static_cast<char>('0'+nWordType));
    // langtag;rule;breakType;rBreakType;nWordType
    const OString aBIMapGlobalKey( aKeyBuf.makeStringAndClear());

    if (icuBI->maBIMapKey != aBIMapGlobalKey || !icuBI->mpValue || !icuBI->mpValue->mpBreakIterator)
    {

        auto aMapIt( theBIMap.find( aBIMapGlobalKey));
        bool bInMap = (aMapIt != theBIMap.end());
        if (bInMap)
            icuBI->mpValue = aMapIt->second;
        else
            icuBI->mpValue.reset();

        if (!bInMap && rule) do {
            const uno::Sequence< OUString > breakRules = LocaleDataImpl::get()->getBreakIteratorRules(rLocale);

            status = U_ZERO_ERROR;
            udata_setAppData("OpenOffice", OpenOffice_dat, &status);
            if ( !U_SUCCESS(status) ) throw uno::RuntimeException();

            std::shared_ptr<OOoRuleBasedBreakIterator> rbi;

            if (breakRules.getLength() > breakType && !breakRules[breakType].isEmpty())
            {
                // langtag;rule;breakType
                const OString aBIMapRuleTypeKey( aLangtagStr + ";" + rule + ";" + OString::number(breakType));
                aMapIt = theBIMap.find( aBIMapRuleTypeKey);
                bInMap = (aMapIt != theBIMap.end());
                if (bInMap)
                {
                    icuBI->mpValue = aMapIt->second;
                    icuBI->maBIMapKey = aBIMapGlobalKey;
                    theBIMap.insert( std::make_pair( aBIMapGlobalKey, icuBI->mpValue));
                    break;  // do
                }

                rbi = std::make_shared<OOoRuleBasedBreakIterator>(udata_open("OpenOffice", "brk",
                    OUStringToOString(breakRules[breakType], RTL_TEXTENCODING_ASCII_US).getStr(), &status), status);

                if (U_SUCCESS(status))
                {
                    icuBI->mpValue = std::make_shared<BI_ValueData>();
                    icuBI->mpValue->mpBreakIterator = rbi;
                    theBIMap.insert( std::make_pair( aBIMapRuleTypeKey, icuBI->mpValue));
                }
                else
                {
                    rbi.reset();
                }
            }
            //use icu's breakiterator for Thai, Tibetan and Dzongkha
            else if (rLocale.Language != "th" && rLocale.Language != "lo" && rLocale.Language != "bo" && rLocale.Language != "dz" && rLocale.Language != "km")
            {
                // language;rule (not langtag, unless we'd actually load such)
                OString aLanguage( LanguageTag( rLocale).getLanguage().toUtf8());
                const OString aBIMapRuleKey( aLanguage + ";" + rule);
                aMapIt = theBIMap.find( aBIMapRuleKey);
                bInMap = (aMapIt != theBIMap.end());
                if (bInMap)
                {
                    icuBI->mpValue = aMapIt->second;
                    icuBI->maBIMapKey = aBIMapGlobalKey;
                    theBIMap.insert( std::make_pair( aBIMapGlobalKey, icuBI->mpValue));
                    break;  // do
                }

                status = U_ZERO_ERROR;
                OString aUDName = OString::Concat(rule) + "_" + aLanguage;
                UDataMemory* pUData = udata_open("OpenOffice", "brk", aUDName.getStr(), &status);
                if( U_SUCCESS(status) )
                    rbi = std::make_shared<OOoRuleBasedBreakIterator>( pUData, status);
                if ( U_SUCCESS(status) )
                {
                    icuBI->mpValue = std::make_shared<BI_ValueData>();
                    icuBI->mpValue->mpBreakIterator = rbi;
                    theBIMap.insert( std::make_pair( aBIMapRuleKey, icuBI->mpValue));
                }
                else
                {
                    rbi.reset();

                    // ;rule (only)
                    const OString aBIMapRuleOnlyKey( OString::Concat(";") + rule);
                    aMapIt = theBIMap.find( aBIMapRuleOnlyKey);
                    bInMap = (aMapIt != theBIMap.end());
                    if (bInMap)
                    {
                        icuBI->mpValue = aMapIt->second;
                        icuBI->maBIMapKey = aBIMapGlobalKey;
                        theBIMap.insert( std::make_pair( aBIMapGlobalKey, icuBI->mpValue));
                        break;  // do
                    }

                    status = U_ZERO_ERROR;
                    pUData = udata_open("OpenOffice", "brk", rule, &status);
                    if( U_SUCCESS(status) )
                        rbi = std::make_shared<OOoRuleBasedBreakIterator>( pUData, status);
                    if ( U_SUCCESS(status) )
                    {
                        icuBI->mpValue = std::make_shared<BI_ValueData>();
                        icuBI->mpValue->mpBreakIterator = rbi;
                        theBIMap.insert( std::make_pair( aBIMapRuleOnlyKey, icuBI->mpValue));
                    }
                    else
                    {
                        rbi.reset();
                    }
                }
            }
            if (rbi) {
#if (U_ICU_VERSION_MAJOR_NUM < 58)
                // ICU 58 made RuleBasedBreakIterator::setBreakType() private
                // instead of protected, so the old workaround of
                // https://ssl.icu-project.org/trac/ticket/5498
                // doesn't work anymore. However, they also claim to have fixed
                // the cause that an initial fBreakType==-1 would lead to an
                // endless loop under some circumstances.
                // Let's see ...
                switch (rBreakType) {
                    case LOAD_CHARACTER_BREAKITERATOR: rbi->publicSetBreakType(UBRK_CHARACTER); break;
                    case LOAD_WORD_BREAKITERATOR: rbi->publicSetBreakType(UBRK_WORD); break;
                    case LOAD_SENTENCE_BREAKITERATOR: rbi->publicSetBreakType(UBRK_SENTENCE); break;
                    case LOAD_LINE_BREAKITERATOR: rbi->publicSetBreakType(UBRK_LINE); break;
                }
#endif
            }
        } while (false);

        if (!icuBI->mpValue || !icuBI->mpValue->mpBreakIterator) do {
            // langtag;;;rBreakType (empty rule; empty breakType)
            const OString aBIMapLocaleTypeKey( aLangtagStr + ";;;" + OString::number(rBreakType));
            aMapIt = theBIMap.find( aBIMapLocaleTypeKey);
            bInMap = (aMapIt != theBIMap.end());
            if (bInMap)
            {
                icuBI->mpValue = aMapIt->second;
                icuBI->maBIMapKey = aBIMapGlobalKey;
                theBIMap.insert( std::make_pair( aBIMapGlobalKey, icuBI->mpValue));
                break;  // do
            }

            icu::Locale icuLocale( LanguageTagIcu::getIcuLocale( LanguageTag( rLocale)));
            std::shared_ptr< icu::BreakIterator > pBI;

            status = U_ZERO_ERROR;
            switch (rBreakType) {
                case LOAD_CHARACTER_BREAKITERATOR:
                    pBI.reset( icu::BreakIterator::createCharacterInstance(icuLocale, status) );
                    break;
                case LOAD_WORD_BREAKITERATOR:
                    pBI.reset( icu::BreakIterator::createWordInstance(icuLocale, status) );
                    break;
                case LOAD_SENTENCE_BREAKITERATOR:
                    pBI.reset( icu::BreakIterator::createSentenceInstance(icuLocale, status) );
                    break;
                case LOAD_LINE_BREAKITERATOR:
                    pBI.reset( icu::BreakIterator::createLineInstance(icuLocale, status) );
                    break;
            }
            if ( !U_SUCCESS(status) || !pBI ) {
                throw uno::RuntimeException();
            }
            icuBI->mpValue = std::make_shared<BI_ValueData>();
            icuBI->mpValue->mpBreakIterator = pBI;
            theBIMap.insert( std::make_pair( aBIMapLocaleTypeKey, icuBI->mpValue));
        } while (false);
        if (!icuBI->mpValue || !icuBI->mpValue->mpBreakIterator) {
            throw uno::RuntimeException();
        }
        icuBI->maBIMapKey = aBIMapGlobalKey;
        if (!bInMap)
            theBIMap.insert( std::make_pair( aBIMapGlobalKey, icuBI->mpValue));
        bNewBreak=true;
    }

    if (!(bNewBreak || icuBI->mpValue->maICUText.pData != rText.pData))
        return;

    const UChar *pText = reinterpret_cast<const UChar *>(rText.getStr());

    status = U_ZERO_ERROR;
    icuBI->mpValue->mpUt = utext_openUChars(icuBI->mpValue->mpUt, pText, rText.getLength(), &status);

    if (!U_SUCCESS(status))
        throw uno::RuntimeException();

    icuBI->mpValue->mpBreakIterator->setText(icuBI->mpValue->mpUt, status);

    if (!U_SUCCESS(status))
        throw uno::RuntimeException();

    icuBI->mpValue->maICUText = rText;
}

sal_Int32 SAL_CALL BreakIterator_Unicode::nextCharacters( const OUString& Text,
        sal_Int32 nStartPos, const lang::Locale &rLocale,
        sal_Int16 nCharacterIteratorMode, sal_Int32 nCount, sal_Int32& nDone )
{
    if (nCharacterIteratorMode == CharacterIteratorMode::SKIPCELL ) { // for CELL mode
        loadICUBreakIterator(rLocale, LOAD_CHARACTER_BREAKITERATOR, 0, "char", Text);
        icu::BreakIterator* pBI = character.mpValue->mpBreakIterator.get();
        for (nDone = 0; nDone < nCount; nDone++) {
            nStartPos = pBI->following(nStartPos);
            if (nStartPos == icu::BreakIterator::DONE)
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
{
    if (nCharacterIteratorMode == CharacterIteratorMode::SKIPCELL ) { // for CELL mode
        loadICUBreakIterator(rLocale, LOAD_CHARACTER_BREAKITERATOR, 0, "char", Text);
        icu::BreakIterator* pBI = character.mpValue->mpBreakIterator.get();
        for (nDone = 0; nDone < nCount; nDone++) {
            nStartPos = pBI->preceding(nStartPos);
            if (nStartPos == icu::BreakIterator::DONE)
                return 0;
        }
    } else { // for BS to delete one char and CHARACTER mode.
        for (nDone = 0; nDone < nCount && nStartPos > 0; nDone++)
            Text.iterateCodePoints(&nStartPos, -1);
    }
    return nStartPos;
}


Boundary SAL_CALL BreakIterator_Unicode::nextWord( const OUString& Text, sal_Int32 nStartPos,
    const lang::Locale& rLocale, sal_Int16 rWordType )
{
    loadICUBreakIterator(rLocale, LOAD_WORD_BREAKITERATOR, rWordType, nullptr, Text);

    Boundary rv;
    rv.startPos = icuBI->mpValue->mpBreakIterator->following(nStartPos);
    if( rv.startPos >= Text.getLength() || rv.startPos == icu::BreakIterator::DONE )
        rv.endPos = result.startPos;
    else {
        if ( (rWordType == WordType::ANYWORD_IGNOREWHITESPACES ||
                    rWordType == WordType::DICTIONARY_WORD ) &&
                u_isWhitespace(Text.iterateCodePoints(&rv.startPos, 0)) )
            rv.startPos = icuBI->mpValue->mpBreakIterator->following(rv.startPos);

        rv.endPos = icuBI->mpValue->mpBreakIterator->following(rv.startPos);
        if(rv.endPos == icu::BreakIterator::DONE)
            rv.endPos = rv.startPos;
    }
    return rv;
}


Boundary SAL_CALL BreakIterator_Unicode::previousWord(const OUString& Text, sal_Int32 nStartPos,
        const lang::Locale& rLocale, sal_Int16 rWordType)
{
    loadICUBreakIterator(rLocale, LOAD_WORD_BREAKITERATOR, rWordType, nullptr, Text);

    Boundary rv;
    rv.startPos = icuBI->mpValue->mpBreakIterator->preceding(nStartPos);
    if( rv.startPos < 0)
        rv.endPos = rv.startPos;
    else {
        if ( (rWordType == WordType::ANYWORD_IGNOREWHITESPACES ||
                    rWordType == WordType::DICTIONARY_WORD) &&
                u_isWhitespace(Text.iterateCodePoints(&rv.startPos, 0)) )
            rv.startPos = icuBI->mpValue->mpBreakIterator->preceding(rv.startPos);

        rv.endPos = icuBI->mpValue->mpBreakIterator->following(rv.startPos);
        if(rv.endPos == icu::BreakIterator::DONE)
            rv.endPos = rv.startPos;
    }
    return rv;
}


Boundary SAL_CALL BreakIterator_Unicode::getWordBoundary( const OUString& Text, sal_Int32 nPos, const lang::Locale& rLocale,
        sal_Int16 rWordType, sal_Bool bDirection )
{
    loadICUBreakIterator(rLocale, LOAD_WORD_BREAKITERATOR, rWordType, nullptr, Text);
    sal_Int32 len = Text.getLength();

    Boundary rv;
    if(icuBI->mpValue->mpBreakIterator->isBoundary(nPos)) {
        rv.startPos = rv.endPos = nPos;
        if((bDirection || nPos == 0) && nPos < len) //forward
            rv.endPos = icuBI->mpValue->mpBreakIterator->following(nPos);
        else
            rv.startPos = icuBI->mpValue->mpBreakIterator->preceding(nPos);
    } else {
        if(nPos <= 0) {
            rv.startPos = 0;
            rv.endPos = len ? icuBI->mpValue->mpBreakIterator->following(sal_Int32(0)) : 0;
        } else if(nPos >= len) {
            rv.startPos = icuBI->mpValue->mpBreakIterator->preceding(len);
            rv.endPos = len;
        } else {
            rv.startPos = icuBI->mpValue->mpBreakIterator->preceding(nPos);
            rv.endPos = icuBI->mpValue->mpBreakIterator->following(nPos);
        }
    }
    if (rv.startPos == icu::BreakIterator::DONE)
        rv.startPos = rv.endPos;
    else if (rv.endPos == icu::BreakIterator::DONE)
        rv.endPos = rv.startPos;

    return rv;
}


sal_Int32 SAL_CALL BreakIterator_Unicode::beginOfSentence( const OUString& Text, sal_Int32 nStartPos,
        const lang::Locale &rLocale )
{
    loadICUBreakIterator(rLocale, LOAD_SENTENCE_BREAKITERATOR, 0, "sent", Text);

    sal_Int32 len = Text.getLength();
    if (len > 0 && nStartPos == len)
        Text.iterateCodePoints(&nStartPos, -1); // issue #i27703# treat end position as part of last sentence
    if (!sentence.mpValue->mpBreakIterator->isBoundary(nStartPos))
        nStartPos = sentence.mpValue->mpBreakIterator->preceding(nStartPos);

    // skip preceding space.
    sal_uInt32 ch = Text.iterateCodePoints(&nStartPos);
    while (nStartPos < len && u_isWhitespace(ch)) ch = Text.iterateCodePoints(&nStartPos);
    Text.iterateCodePoints(&nStartPos, -1);

    return nStartPos;
}

sal_Int32 SAL_CALL BreakIterator_Unicode::endOfSentence( const OUString& Text, sal_Int32 nStartPos,
        const lang::Locale &rLocale )
{
    loadICUBreakIterator(rLocale, LOAD_SENTENCE_BREAKITERATOR, 0, "sent", Text);

    sal_Int32 len = Text.getLength();
    if (len > 0 && nStartPos == len)
        Text.iterateCodePoints(&nStartPos, -1); // issue #i27703# treat end position as part of last sentence
    nStartPos = sentence.mpValue->mpBreakIterator->following(nStartPos);

    sal_Int32 nPos=nStartPos;
    while (nPos > 0 && u_isWhitespace(Text.iterateCodePoints(&nPos, -1))) nStartPos=nPos;

    return nStartPos;
}

LineBreakResults SAL_CALL BreakIterator_Unicode::getLineBreak(
        const OUString& Text, sal_Int32 nStartPos,
        const lang::Locale& rLocale, sal_Int32 nMinBreakPos,
        const LineBreakHyphenationOptions& hOptions,
        const LineBreakUserOptions& /*rOptions*/ )
{
    LineBreakResults lbr;

    if (nStartPos >= Text.getLength()) {
        lbr.breakIndex = Text.getLength();
        lbr.breakType = BreakType::WORDBOUNDARY;
        return lbr;
    }

    loadICUBreakIterator(rLocale, LOAD_LINE_BREAKITERATOR, 0, lineRule, Text);

    icu::BreakIterator* pLineBI = line.mpValue->mpBreakIterator.get();
    bool GlueSpace=true;
    while (GlueSpace) {
        // don't break with Slash U+002F SOLIDUS at end of line; see "else" below!
        if (pLineBI->preceding(nStartPos + 1) == nStartPos
                && (nStartPos == 0 || Text[nStartPos - 1] != '/'))
        { //Line boundary break
            lbr.breakIndex = nStartPos;
            lbr.breakType = BreakType::WORDBOUNDARY;
        } else if (hOptions.rHyphenator.is()) { //Hyphenation break
            sal_Int32 boundary_with_punctuation = (pLineBI->next() != icu::BreakIterator::DONE) ? pLineBI->current() : 0;
            pLineBI->preceding(nStartPos + 1); // reset to check correct hyphenation of "word-word"

            sal_Int32 nStartPosWordEnd = nStartPos;
            while (pLineBI->current() < nStartPosWordEnd && u_ispunct(static_cast<sal_uInt32>(Text[nStartPosWordEnd]))) // starting punctuation
                nStartPosWordEnd --;

            Boundary wBoundary = getWordBoundary( Text, nStartPosWordEnd, rLocale,
                WordType::DICTIONARY_WORD, false);

            nStartPosWordEnd = wBoundary.endPos;
            while (nStartPosWordEnd < Text.getLength() && (u_ispunct(static_cast<sal_uInt32>(Text[nStartPosWordEnd])))) // ending punctuation
                nStartPosWordEnd ++;
            nStartPosWordEnd = nStartPosWordEnd - wBoundary.endPos;
            if (hOptions.hyphenIndex - wBoundary.startPos < nStartPosWordEnd) nStartPosWordEnd = hOptions.hyphenIndex - wBoundary.startPos;
#define SPACE 0x0020
            while (boundary_with_punctuation > wBoundary.endPos && Text[--boundary_with_punctuation] == SPACE);
            uno::Reference< linguistic2::XHyphenatedWord > aHyphenatedWord = hOptions.rHyphenator->hyphenate(Text.copy(wBoundary.startPos,
                        wBoundary.endPos - wBoundary.startPos), rLocale,
                    static_cast<sal_Int16>(hOptions.hyphenIndex - wBoundary.startPos - ((hOptions.hyphenIndex == wBoundary.endPos)? nStartPosWordEnd : 0)), hOptions.aHyphenationOptions);
            if (aHyphenatedWord.is()) {
                lbr.rHyphenatedWord = aHyphenatedWord;
                if(wBoundary.startPos + aHyphenatedWord->getHyphenationPos() + 1 < nMinBreakPos )
                    lbr.breakIndex = -1;
                else
                    lbr.breakIndex = wBoundary.startPos; //aHyphenatedWord->getHyphenationPos();
                lbr.breakType = BreakType::HYPHENATION;

                // check not optimal hyphenation of "word-word" (word with hyphens)
                if (lbr.breakIndex > -1 && wBoundary.startPos + aHyphenatedWord->getHyphenationPos() < pLineBI->current()) {
                    lbr.breakIndex = pLineBI->current();
                    lbr.breakType = BreakType::WORDBOUNDARY;
                }

            } else {
                lbr.breakIndex = pLineBI->preceding(nStartPos);
                lbr.breakType = BreakType::WORDBOUNDARY;
            }
        } else { //word boundary break
            lbr.breakIndex = pLineBI->preceding(nStartPos);
            lbr.breakType = BreakType::WORDBOUNDARY;

            // Special case for Slash U+002F SOLIDUS in URI and path names.
            // TR14 defines that as SY: Symbols Allowing Break After (A).
            // This is unwanted in paths, see also i#17155
            if (lbr.breakIndex > 0 && Text[lbr.breakIndex-1] == '/')
            {
                // Look backward and take any whitespace before as a break
                // opportunity. This also glues something like "w/o".
                // Avoid an overly long path and break it as was indicated.
                // Overly long here is arbitrarily defined.
                const sal_Int32 nOverlyLong = 66;
                sal_Int32 nPos = lbr.breakIndex - 1;
                while (nPos > 0 && lbr.breakIndex - nPos < nOverlyLong)
                {
                    if (u_isWhitespace(Text.iterateCodePoints( &nPos, -1)))
                    {
                        lbr.breakIndex = nPos + 1;
                        break;
                    }
                }
            }
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
BreakIterator_Unicode::getImplementationName()
{
    return OUString::createFromAscii(cBreakIterator);
}

sal_Bool SAL_CALL
BreakIterator_Unicode::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SAL_CALL
BreakIterator_Unicode::getSupportedServiceNames()
{
    uno::Sequence< OUString > aRet { OUString::createFromAscii(cBreakIterator) };
    return aRet;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_i18n_BreakIterator_Unicode_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new i18npool::BreakIterator_Unicode());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
