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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"

#define BREAKITERATOR_ALL
#include <breakiterator_cjk.hxx>
#include <localedata.hxx>
#include <i18nutil/unicode.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

//      ----------------------------------------------------
//      class BreakIterator_CJK
//      ----------------------------------------------------;

BreakIterator_CJK::BreakIterator_CJK() :
    dict( NULL ),
    hangingCharacters()
{
        cBreakIterator = "com.sun.star.i18n.BreakIterator_CJK";
}

Boundary SAL_CALL
BreakIterator_CJK::previousWord(const OUString& text, sal_Int32 anyPos,
        const lang::Locale& nLocale, sal_Int16 wordType) throw(RuntimeException)
{
        if (dict) {
            result = dict->previousWord(text, anyPos, wordType);
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
        const lang::Locale& nLocale, sal_Int16 wordType) throw(RuntimeException)
{
        if (dict) {
            result = dict->nextWord(text, anyPos, wordType);
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
        const lang::Locale& nLocale, sal_Int16 wordType, sal_Bool bDirection )
        throw(RuntimeException)
{
        if (dict) {
            result = dict->getWordBoundary(text, anyPos, wordType, bDirection);
            // #109813# for non-CJK, single character word, fallback to ICU breakiterator.
            if (result.endPos - result.startPos != 1 ||
                    getScriptType(text, result.startPos) == ScriptType::ASIAN)
                return result;
        }
        return BreakIterator_Unicode::getWordBoundary(text, anyPos, nLocale, wordType, bDirection);
}

LineBreakResults SAL_CALL BreakIterator_CJK::getLineBreak(
        const OUString& Text, sal_Int32 nStartPos,
        const lang::Locale& /*rLocale*/, sal_Int32 /*nMinBreakPos*/,
        const LineBreakHyphenationOptions& /*hOptions*/,
        const LineBreakUserOptions& bOptions ) throw(RuntimeException)
{
        LineBreakResults lbr;

        if (bOptions.allowPunctuationOutsideMargin &&
                hangingCharacters.indexOf(Text[nStartPos]) != -1 &&
                (Text.iterateCodePoints( &nStartPos, 1), nStartPos == Text.getLength())) {
            ; // do nothing
        } else if (bOptions.applyForbiddenRules && 0 < nStartPos && nStartPos < Text.getLength()) {
            while (nStartPos > 0 &&
                    (bOptions.forbiddenBeginCharacters.indexOf(Text[nStartPos]) != -1 ||
                    bOptions.forbiddenEndCharacters.indexOf(Text[nStartPos-1]) != -1))
                Text.iterateCodePoints( &nStartPos, -1);
        }

        lbr.breakIndex = nStartPos;
        lbr.breakType = BreakType::WORDBOUNDARY;
        return lbr;
}

#define LOCALE(language, country) lang::Locale(OUString::createFromAscii(language), OUString::createFromAscii(country), OUString())
//      ----------------------------------------------------
//      class BreakIterator_zh
//      ----------------------------------------------------;
BreakIterator_zh::BreakIterator_zh()
{
        dict = new xdictionary("zh");
        hangingCharacters = LocaleData().getHangingCharacters(LOCALE("zh", "CN"));
        cBreakIterator = "com.sun.star.i18n.BreakIterator_zh";
}

BreakIterator_zh::~BreakIterator_zh()
{
        delete dict;
}

//      ----------------------------------------------------
//      class BreakIterator_zh_TW
//      ----------------------------------------------------;
BreakIterator_zh_TW::BreakIterator_zh_TW()
{
        dict = new xdictionary("zh");
        hangingCharacters = LocaleData().getHangingCharacters(LOCALE("zh", "TW"));
        cBreakIterator = "com.sun.star.i18n.BreakIterator_zh_TW";
}

BreakIterator_zh_TW::~BreakIterator_zh_TW()
{
        delete dict;
}

//      ----------------------------------------------------
//      class BreakIterator_ja
//      ----------------------------------------------------;
BreakIterator_ja::BreakIterator_ja()
{
        dict = new xdictionary("ja");
        dict->setJapaneseWordBreak();
        hangingCharacters = LocaleData().getHangingCharacters(LOCALE("ja", "JP"));
        cBreakIterator = "com.sun.star.i18n.BreakIterator_ja";
}

BreakIterator_ja::~BreakIterator_ja()
{
        delete dict;
}

//      ----------------------------------------------------
//      class BreakIterator_ko
//      ----------------------------------------------------;
BreakIterator_ko::BreakIterator_ko()
{
        hangingCharacters = LocaleData().getHangingCharacters(LOCALE("ko", "KR"));
        cBreakIterator = "com.sun.star.i18n.BreakIterator_ko";
}

BreakIterator_ko::~BreakIterator_ko()
{
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
