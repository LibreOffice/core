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

#include <com/sun/star/i18n/BreakType.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;

namespace i18npool
{
//      ----------------------------------------------------
//      class BreakIterator_CJK
//      ----------------------------------------------------;

BreakIterator_CJK::BreakIterator_CJK()
{
    cBreakIterator = u"com.sun.star.i18n.BreakIterator_CJK"_ustr;
}

namespace
{
bool isHangul(sal_Unicode cCh)
{
    return (cCh >= 0xAC00 && cCh <= 0xD7AF) || (cCh >= 0x1100 && cCh <= 0x11FF)
           || (cCh >= 0xA960 && cCh <= 0xA97F) || (cCh >= 0xD7B0 && cCh <= 0xD7FF)
           || (cCh >= 0x3130 && cCh <= 0x318F);
}
}

LineBreakResults SAL_CALL BreakIterator_CJK::getLineBreak(
    const OUString& Text, sal_Int32 nStartPos, const css::lang::Locale& rLocale,
    sal_Int32 nMinBreakPos, const LineBreakHyphenationOptions& hOptions,
    const LineBreakUserOptions& bOptions)
{
    auto fnIsForbiddenBreak = [&](sal_Int32 nBreakPos) {
        return nBreakPos > 0
               && (bOptions.forbiddenBeginCharacters.indexOf(Text[nBreakPos]) != -1
                   || bOptions.forbiddenEndCharacters.indexOf(Text[nBreakPos - 1]) != -1);
    };

    auto fnIsNonBreak = [](sal_Unicode cChB) {
        switch (cChB)
        {
            case 0x00A0: // No-break Space
            case 0x2011: // Non-breaking Hyphen
            case 0x202F: // Narrow No-break Space
            case 0x2060: // Word Joiner
            case 0xFEFF: // NWNBSP
                return true;
            default:
                return false;
        }
    };

    while (nStartPos > 0 && nStartPos < Text.getLength())
    {
        // Apply hanging punctuation
        if (bOptions.allowPunctuationOutsideMargin
            && hangingCharacters.indexOf(Text[nStartPos]) != -1)
        {
            // The current character is allowed to overhang the margin.
            sal_Int32 nNextPos = nStartPos;
            Text.iterateCodePoints(&nNextPos);

            // tdf#130592: The original code always allowed a line break after hanging
            // punctuation, even if it's not a valid ICU break. This refactor preserves the
            // original behavior in order to avoid regressing tdf#58604.
            if (nNextPos >= Text.getLength() || !fnIsForbiddenBreak(nNextPos))
            {
                LineBreakResults stBreak;
                stBreak.breakIndex = nNextPos;
                stBreak.breakType = BreakType::HANGINGPUNCTUATION;
                return stBreak;
            }
        }

        const sal_Int32 nOldStartPos = nStartPos;

        // Apply forbidden rules
        if (bOptions.applyForbiddenRules)
        {
            while (fnIsForbiddenBreak(nStartPos))
            {
                Text.iterateCodePoints(&nStartPos, -1);
            }
        }

        // Prevent cutting Korean words in the middle
        if (nOldStartPos == nStartPos && isHangul(Text[nStartPos]))
        {
            while (nStartPos >= 0 && isHangul(Text[nStartPos]))
                --nStartPos;

            // beginning of the last Korean word.
            if (nStartPos < nOldStartPos)
                ++nStartPos;

            if (nStartPos == 0)
                nStartPos = nOldStartPos;
        }

        // Formatting Marks
        while (nStartPos > 0
               && (fnIsNonBreak(Text[nStartPos]) || fnIsNonBreak(Text[nStartPos - 1])))
        {
            Text.iterateCodePoints(&nStartPos, -1);
        }

        // tdf#130592: Fall back to the ICU breakiterator after applying CJK-specific rules
        auto stBreak = BreakIterator_Unicode::getLineBreak(Text, nStartPos, rLocale, nMinBreakPos,
                                                           hOptions, bOptions);
        if (stBreak.breakIndex == nStartPos)
        {
            // Located break is valid under both iterators
            return stBreak;
        }
        if (nStartPos > stBreak.breakIndex)
        {
            // tdf#169590 Clarify the case where bOptions.applyForbiddenRules is False
            if (!bOptions.applyForbiddenRules)
            {
                stBreak.breakIndex = nStartPos;
                stBreak.breakType = BreakType::WORDBOUNDARY;
                return stBreak;
            }

            // Respect user changes to forbidden character list
            sal_Int32 nTempPos = stBreak.breakIndex;
            Text.iterateCodePoints(&nTempPos);
            if (nTempPos == nStartPos && !fnIsForbiddenBreak(nStartPos)
                && !fnIsNonBreak(Text[nStartPos]))
            {
                stBreak.breakIndex = nStartPos;
                stBreak.breakType = BreakType::WORDBOUNDARY;
                return stBreak;
            }
        }

        // CJK break is not valid; restart search from the next candidate
        sal_Int32 nNextCandidate = stBreak.breakIndex;
        while (bOptions.allowPunctuationOutsideMargin && nStartPos > stBreak.breakIndex)
        {
            if (hangingCharacters.indexOf(Text[nStartPos]) != -1)
            {
                nNextCandidate = nStartPos;
                break;
            }

            Text.iterateCodePoints(&nStartPos, -1);
        }

        nStartPos = nNextCandidate;
    }

    return BreakIterator_Unicode::getLineBreak(Text, nStartPos, rLocale, nMinBreakPos, hOptions,
                                               bOptions);
}

#define LOCALE(language, country) css::lang::Locale(language, country, OUString())
//      ----------------------------------------------------
//      class BreakIterator_zh
//      ----------------------------------------------------;
BreakIterator_zh::BreakIterator_zh()
{
    assert(hangingCharacters.pData);
    hangingCharacters = LocaleDataImpl::get()->getHangingCharacters(LOCALE(u"zh"_ustr, u"CN"_ustr));
    cBreakIterator = u"com.sun.star.i18n.BreakIterator_zh"_ustr;
}

//      ----------------------------------------------------
//      class BreakIterator_zh_TW
//      ----------------------------------------------------;
BreakIterator_zh_TW::BreakIterator_zh_TW()
{
    assert(hangingCharacters.pData);
    hangingCharacters = LocaleDataImpl::get()->getHangingCharacters(LOCALE(u"zh"_ustr, u"TW"_ustr));
    cBreakIterator = u"com.sun.star.i18n.BreakIterator_zh_TW"_ustr;
}

//      ----------------------------------------------------
//      class BreakIterator_ja
//      ----------------------------------------------------;
BreakIterator_ja::BreakIterator_ja()
{
    assert(hangingCharacters.pData);
    hangingCharacters = LocaleDataImpl::get()->getHangingCharacters(LOCALE(u"ja"_ustr, u"JP"_ustr));
    cBreakIterator = u"com.sun.star.i18n.BreakIterator_ja"_ustr;
}

//      ----------------------------------------------------
//      class BreakIterator_ko
//      ----------------------------------------------------;
BreakIterator_ko::BreakIterator_ko()
{
    assert(hangingCharacters.pData);
    hangingCharacters = LocaleDataImpl::get()->getHangingCharacters(LOCALE(u"ko"_ustr, u"KR"_ustr));
    cBreakIterator = u"com.sun.star.i18n.BreakIterator_ko"_ustr;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
