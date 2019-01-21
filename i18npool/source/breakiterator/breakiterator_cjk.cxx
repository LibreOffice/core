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

#include <breakiterator_cjk.hxx>
#include <localedata.hxx>
#include <i18nutil/unicode.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;

namespace i18npool {

//      ----------------------------------------------------
//      class BreakIterator_CJK
//      ----------------------------------------------------;

BreakIterator_CJK::BreakIterator_CJK()
{
    cBreakIterator = "com.sun.star.i18n.BreakIterator_CJK";
}

Boundary SAL_CALL
BreakIterator_CJK::previousWord(const OUString& text, sal_Int32 anyPos,
        const css::lang::Locale& nLocale, sal_Int16 wordType)
{
    if (m_xDict) {
        result = m_xDict->previousWord(text, anyPos, wordType);
        // #109813# for non-CJK, single character word, fallback to ICU breakiterator.
        if (result.endPos - result.startPos != 1 ||
                getScriptType(text, result.startPos) == ScriptType::ASIAN)
            return result;
        result = BreakIterator_Unicode::getWordBoundary(text, result.startPos, nLocale, wordType, true);
        if (result.endPos < anyPos)
            return result;
    }
    return BreakIterator_Unicode::previousWord(text, anyPos, nLocale, wordType);
}

Boundary SAL_CALL
BreakIterator_CJK::nextWord(const OUString& text, sal_Int32 anyPos,
        const css::lang::Locale& nLocale, sal_Int16 wordType)
{
    if (m_xDict) {
        result = m_xDict->nextWord(text, anyPos, wordType);
        // #109813# for non-CJK, single character word, fallback to ICU breakiterator.
        if (result.endPos - result.startPos != 1 ||
                getScriptType(text, result.startPos) == ScriptType::ASIAN)
            return result;
        result = BreakIterator_Unicode::getWordBoundary(text, result.startPos, nLocale, wordType, true);
        if (result.startPos > anyPos)
            return result;
    }
    return BreakIterator_Unicode::nextWord(text, anyPos, nLocale, wordType);
}

Boundary SAL_CALL
BreakIterator_CJK::getWordBoundary( const OUString& text, sal_Int32 anyPos,
        const css::lang::Locale& nLocale, sal_Int16 wordType, sal_Bool bDirection )
{
    if (m_xDict) {
        result = m_xDict->getWordBoundary(text, anyPos, wordType, bDirection);
        // #109813# for non-CJK, single character word, fallback to ICU breakiterator.
        if (result.endPos - result.startPos != 1 ||
                getScriptType(text, result.startPos) == ScriptType::ASIAN)
            return result;
    }
    return BreakIterator_Unicode::getWordBoundary(text, anyPos, nLocale, wordType, bDirection);
}

namespace {
bool isHangul( sal_Unicode cCh )
{
    return (cCh >= 0xAC00 && cCh <= 0xD7AF) || (cCh >= 0x1100 && cCh <= 0x11FF);
}
}

LineBreakResults SAL_CALL BreakIterator_CJK::getLineBreak(
        const OUString& Text, sal_Int32 nStartPos,
        const css::lang::Locale& /*rLocale*/, sal_Int32 /*nMinBreakPos*/,
        const LineBreakHyphenationOptions& /*hOptions*/,
        const LineBreakUserOptions& bOptions )
{
    LineBreakResults lbr;

    const sal_Int32 nOldStartPos = nStartPos;

    if (bOptions.allowPunctuationOutsideMargin &&
            hangingCharacters.indexOf(Text[nStartPos]) != -1 &&
            (Text.iterateCodePoints( &nStartPos ), nStartPos == Text.getLength())) {
        ; // do nothing
    } else if (bOptions.applyForbiddenRules && 0 < nStartPos && nStartPos < Text.getLength()) {

        while (nStartPos > 0 &&
                (bOptions.forbiddenBeginCharacters.indexOf(Text[nStartPos]) != -1 ||
                 bOptions.forbiddenEndCharacters.indexOf(Text[nStartPos-1]) != -1))
            Text.iterateCodePoints( &nStartPos, -1);
    }

    // Prevent cutting Korean words in the middle.
    if (nOldStartPos == nStartPos && nStartPos < Text.getLength()
        && isHangul(Text[nStartPos]))
    {
        while ( nStartPos >= 0 && isHangul( Text[nStartPos] ) )
            --nStartPos;

        // beginning of the last Korean word.
        if ( nStartPos < nOldStartPos )
            ++nStartPos;

        if ( nStartPos == 0 )
            nStartPos = nOldStartPos;
    }

    lbr.breakIndex = nStartPos;
    lbr.breakType = BreakType::WORDBOUNDARY;
    return lbr;
}

#define LOCALE(language, country) css::lang::Locale(language, country, OUString())
//      ----------------------------------------------------
//      class BreakIterator_zh
//      ----------------------------------------------------;
BreakIterator_zh::BreakIterator_zh()
{
    m_xDict = std::make_unique<xdictionary>("zh");
    hangingCharacters = LocaleDataImpl::get()->getHangingCharacters(LOCALE("zh", "CN"));
    cBreakIterator = "com.sun.star.i18n.BreakIterator_zh";
}

//      ----------------------------------------------------
//      class BreakIterator_zh_TW
//      ----------------------------------------------------;
BreakIterator_zh_TW::BreakIterator_zh_TW()
{
    m_xDict = std::make_unique<xdictionary>("zh");
    hangingCharacters = LocaleDataImpl::get()->getHangingCharacters(LOCALE("zh", "TW"));
    cBreakIterator = "com.sun.star.i18n.BreakIterator_zh_TW";
}

//      ----------------------------------------------------
//      class BreakIterator_ja
//      ----------------------------------------------------;
BreakIterator_ja::BreakIterator_ja()
{
    m_xDict = std::make_unique<xdictionary>("ja");
    m_xDict->setJapaneseWordBreak();
    hangingCharacters = LocaleDataImpl::get()->getHangingCharacters(LOCALE("ja", "JP"));
    cBreakIterator = "com.sun.star.i18n.BreakIterator_ja";
}

//      ----------------------------------------------------
//      class BreakIterator_ko
//      ----------------------------------------------------;
BreakIterator_ko::BreakIterator_ko()
{
    hangingCharacters = LocaleDataImpl::get()->getHangingCharacters(LOCALE("ko", "KR"));
    cBreakIterator = "com.sun.star.i18n.BreakIterator_ko";
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
