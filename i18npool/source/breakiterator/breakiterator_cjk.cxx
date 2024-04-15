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

namespace i18npool {

//      ----------------------------------------------------
//      class BreakIterator_CJK
//      ----------------------------------------------------;

BreakIterator_CJK::BreakIterator_CJK()
{
    cBreakIterator = u"com.sun.star.i18n.BreakIterator_CJK"_ustr;
}

namespace {
bool isHangul( sal_Unicode cCh )
{
    return (cCh >= 0xAC00 && cCh <= 0xD7AF) || (cCh >= 0x1100 && cCh <= 0x11FF) ||
           (cCh >= 0xA960 && cCh <= 0xA97F) || (cCh >= 0xD7B0 && cCh <= 0xD7FF) ||
           (cCh >= 0x3130 && cCh <= 0x318F);
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
            nStartPos != Text.getLength() &&
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
    assert(hangingCharacters.pData);
    hangingCharacters = LocaleDataImpl::get()->getHangingCharacters(LOCALE("zh", "CN"));
    cBreakIterator = u"com.sun.star.i18n.BreakIterator_zh"_ustr;
}

//      ----------------------------------------------------
//      class BreakIterator_zh_TW
//      ----------------------------------------------------;
BreakIterator_zh_TW::BreakIterator_zh_TW()
{
    assert(hangingCharacters.pData);
    hangingCharacters = LocaleDataImpl::get()->getHangingCharacters(LOCALE("zh", "TW"));
    cBreakIterator = u"com.sun.star.i18n.BreakIterator_zh_TW"_ustr;
}

//      ----------------------------------------------------
//      class BreakIterator_ja
//      ----------------------------------------------------;
BreakIterator_ja::BreakIterator_ja()
{
    assert(hangingCharacters.pData);
    hangingCharacters = LocaleDataImpl::get()->getHangingCharacters(LOCALE("ja", "JP"));
    cBreakIterator = u"com.sun.star.i18n.BreakIterator_ja"_ustr;
}

//      ----------------------------------------------------
//      class BreakIterator_ko
//      ----------------------------------------------------;
BreakIterator_ko::BreakIterator_ko()
{
    assert(hangingCharacters.pData);
    hangingCharacters = LocaleDataImpl::get()->getHangingCharacters(LOCALE("ko", "KR"));
    cBreakIterator = u"com.sun.star.i18n.BreakIterator_ko"_ustr;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
