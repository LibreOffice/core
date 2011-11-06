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
 *   http://www.apache.org/licenses/LICENSE-2.0
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
