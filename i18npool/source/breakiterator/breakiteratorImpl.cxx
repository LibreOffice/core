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

#include <config_locales.h>

#include <breakiteratorImpl.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unicode/uchar.h>
#include <i18nutil/scriptclass.hxx>
#include <i18nutil/unicode.hxx>
#include <o3tl/string_view.hxx>

#include <com/sun/star/i18n/CharType.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;

namespace i18npool {

BreakIteratorImpl::BreakIteratorImpl( const Reference < XComponentContext >& rxContext ) : m_xContext( rxContext )
{
}

BreakIteratorImpl::BreakIteratorImpl()
{
}

BreakIteratorImpl::~BreakIteratorImpl()
{
}

#define LBI getLocaleSpecificBreakIterator(rLocale)

sal_Int32 SAL_CALL BreakIteratorImpl::nextCharacters( const OUString& Text, sal_Int32 nStartPos,
        const Locale &rLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 nCount, sal_Int32& nDone )
{
    if (nCount < 0)
        throw RuntimeException("BreakIteratorImpl::nextCharacters: expected nCount >=0, got "
                               + OUString::number(nCount));

    return LBI->nextCharacters( Text, nStartPos, rLocale, nCharacterIteratorMode, nCount, nDone);
}

sal_Int32 SAL_CALL BreakIteratorImpl::previousCharacters( const OUString& Text, sal_Int32 nStartPos,
        const Locale& rLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 nCount, sal_Int32& nDone )
{
    if (nCount < 0)
        throw RuntimeException("BreakIteratorImpl::previousCharacters: expected nCount >=0, got "
                               + OUString::number(nCount));

    return LBI->previousCharacters( Text, nStartPos, rLocale, nCharacterIteratorMode, nCount, nDone);
}

#define isZWSP(c) (ch == 0x200B)

static sal_Int32 skipSpace(std::u16string_view Text, sal_Int32 nPos, sal_Int32 len, sal_Int16 rWordType, bool bDirection)
{
    sal_uInt32 ch=0;
    sal_Int32 pos=nPos;
    switch (rWordType) {
        case WordType::ANYWORD_IGNOREWHITESPACES:
        case WordType::WORD_COUNT:
            if (bDirection)
                while (nPos < len)
                {
                    ch = o3tl::iterateCodePoints(Text, &pos);
                    if (!u_isUWhiteSpace(ch) && !isZWSP(ch))
                        break;
                    nPos = pos;
                }
            else
                while (nPos > 0)
                {
                    ch = o3tl::iterateCodePoints(Text, &pos, -1);
                    if (!u_isUWhiteSpace(ch) && !isZWSP(ch))
                        break;
                    nPos = pos;
                }
            break;
        case WordType::DICTIONARY_WORD:
            if (bDirection)
                while (nPos < len)
                {
                    ch = o3tl::iterateCodePoints(Text, &pos);
                    if (!u_isWhitespace(ch) && !isZWSP(ch) && (ch == 0x002E || u_isalnum(ch)))
                        break;
                    nPos = pos;
                }
            else
                while (nPos > 0)
                {
                    ch = o3tl::iterateCodePoints(Text, &pos, -1);
                    if (!u_isWhitespace(ch) && !isZWSP(ch) && (ch == 0x002E || u_isalnum(ch)))
                        break;
                    nPos = pos;
                }
            break;
    }
    return nPos;
}

Boundary SAL_CALL BreakIteratorImpl::nextWord( const OUString& Text, sal_Int32 nStartPos,
        const Locale& rLocale, sal_Int16 rWordType )
{
    sal_Int32 len = Text.getLength();
    if( nStartPos < 0 || len == 0 )
        result.endPos = result.startPos = 0;
    else if (nStartPos >= len)
        result.endPos = result.startPos = len;
    else {
        result = LBI->nextWord(Text, nStartPos, rLocale, rWordType);

        nStartPos = skipSpace(Text, result.startPos, len, rWordType, true);

        if ( nStartPos != result.startPos) {
            if( nStartPos >= len )
                result.startPos = result.endPos = len;
            else {
                result = LBI->getWordBoundary(Text, nStartPos, rLocale, rWordType, true);
                // i88041: avoid startPos goes back to nStartPos when switching between Latin and CJK scripts
                if (result.startPos < nStartPos) result.startPos = nStartPos;
            }
        }
    }
    return result;
}

static bool isCJK( const Locale& rLocale ) {
        return rLocale.Language == "zh" || rLocale.Language == "ja" || rLocale.Language == "ko";
}

Boundary SAL_CALL BreakIteratorImpl::previousWord( const OUString& Text, sal_Int32 nStartPos,
        const Locale& rLocale, sal_Int16 rWordType)
{
    sal_Int32 len = Text.getLength();
    if( nStartPos <= 0 || len == 0 ) {
        result.endPos = result.startPos = 0;
        return result;
    } else if (nStartPos > len) {
        result.endPos = result.startPos = len;
        return result;
    }

    sal_Int32 nPos = skipSpace(Text, nStartPos, len, rWordType, false);

    // if some spaces are skipped, and the script type is Asian with no CJK rLocale, we have to return
    // (nStartPos, -1) for caller to send correct rLocale for loading correct dictionary.
    result.startPos = nPos;
    if (nPos != nStartPos && nPos > 0 && !isCJK(rLocale) && getScriptClass(Text.iterateCodePoints(&nPos, -1)) == ScriptType::ASIAN) {
        result.endPos = -1;
        return result;
    }

    return LBI->previousWord(Text, result.startPos, rLocale, rWordType);
}


Boundary SAL_CALL BreakIteratorImpl::getWordBoundary( const OUString& Text, sal_Int32 nPos, const Locale& rLocale,
        sal_Int16 rWordType, sal_Bool bDirection )
{
    sal_Int32 len = Text.getLength();
    if( nPos < 0 || len == 0 )
        result.endPos = result.startPos = 0;
    else if (nPos > len)
        result.endPos = result.startPos = len;
    else {
        sal_Int32 next, prev;
        next = skipSpace(Text, nPos, len, rWordType, true);
        prev = skipSpace(Text, nPos, len, rWordType, false);
        if (prev == 0 && next == len) {
            result.endPos = result.startPos = nPos;
        } else if (prev == 0 && ! bDirection) {
            result.endPos = result.startPos = 0;
        } else if (next == len && bDirection) {
            result.endPos = result.startPos = len;
        } else {
            if (next != prev) {
                if (next == nPos && next != len)
                    bDirection = true;
                else if (prev == nPos && prev != 0)
                    bDirection = false;
                else
                    nPos = bDirection ? next : prev;
            }
            result = LBI->getWordBoundary(Text, nPos, rLocale, rWordType, bDirection);
        }
    }
    return result;
}

sal_Bool SAL_CALL BreakIteratorImpl::isBeginWord( const OUString& Text, sal_Int32 nPos,
        const Locale& rLocale, sal_Int16 rWordType )
{
    sal_Int32 len = Text.getLength();

    if (nPos < 0 || nPos >= len) return false;

    sal_Int32 tmp = skipSpace(Text, nPos, len, rWordType, true);

    if (tmp != nPos) return false;

    result = getWordBoundary(Text, nPos, rLocale, rWordType, true);

    return result.startPos == nPos;
}

sal_Bool SAL_CALL BreakIteratorImpl::isEndWord( const OUString& Text, sal_Int32 nPos,
        const Locale& rLocale, sal_Int16 rWordType )
{
    sal_Int32 len = Text.getLength();

    if (nPos <= 0 || nPos > len) return false;

    sal_Int32 tmp = skipSpace(Text, nPos, len, rWordType, false);

    if (tmp != nPos) return false;

    result = getWordBoundary(Text, nPos, rLocale, rWordType, false);

    return result.endPos == nPos;
}

sal_Int32 SAL_CALL BreakIteratorImpl::beginOfSentence( const OUString& Text, sal_Int32 nStartPos,
        const Locale &rLocale )
{
    if (nStartPos < 0 || nStartPos > Text.getLength())
        return -1;
    if (Text.isEmpty()) return 0;
    return LBI->beginOfSentence(Text, nStartPos, rLocale);
}

sal_Int32 SAL_CALL BreakIteratorImpl::endOfSentence( const OUString& Text, sal_Int32 nStartPos,
        const Locale &rLocale )
{
    if (nStartPos < 0 || nStartPos > Text.getLength())
        return -1;
    if (Text.isEmpty()) return 0;
    return LBI->endOfSentence(Text, nStartPos, rLocale);
}

LineBreakResults SAL_CALL BreakIteratorImpl::getLineBreak( const OUString& Text, sal_Int32 nStartPos,
        const Locale& rLocale, sal_Int32 nMinBreakPos, const LineBreakHyphenationOptions& hOptions,
        const LineBreakUserOptions& bOptions )
{
    return LBI->getLineBreak(Text, nStartPos, rLocale, nMinBreakPos, hOptions, bOptions);
}

sal_Int16 SAL_CALL BreakIteratorImpl::getScriptType( const OUString& Text, sal_Int32 nPos )
{
    return (nPos < 0 || nPos >= Text.getLength()) ? ScriptType::WEAK :
        getScriptClass(Text.iterateCodePoints(&nPos, 0));
}


/** Increments/decrements position first, then obtains character.
    @return current position, may be -1 or text length if string was consumed.
 */
static sal_Int32 iterateCodePoints(const OUString& Text, sal_Int32 &nStartPos, sal_Int32 inc, sal_uInt32& ch) {
        sal_Int32 nLen = Text.getLength();
        if (nStartPos + inc < 0 || nStartPos + inc >= nLen) {
            ch = 0;
            nStartPos = nStartPos + inc < 0 ? -1 : nLen;
        } else {
            ch = Text.iterateCodePoints(&nStartPos, inc);
            // Fix for #i80436#.
            // erAck: 2009-06-30T21:52+0200  This logic looks somewhat
            // suspicious as if it cures a symptom... anyway, had to add
            // nStartPos < Text.getLength() to silence the (correct) assertion
            // in rtl_uString_iterateCodePoints() if Text was one character
            // (codepoint) only, made up of a surrogate pair.
            //if (inc > 0 && nStartPos < Text.getLength())
            //    ch = Text.iterateCodePoints(&nStartPos, 0);
            // With surrogates, nStartPos may actually point behind string
            // now, even if inc is only +1
            if (inc > 0)
                ch = (nStartPos < nLen ? Text.iterateCodePoints(&nStartPos, 0) : 0);
        }
        return nStartPos;
}


sal_Int32 SAL_CALL BreakIteratorImpl::beginOfScript( const OUString& Text,
        sal_Int32 nStartPos, sal_Int16 ScriptType )
{
    if (nStartPos < 0 || nStartPos >= Text.getLength())
        return -1;

    if(ScriptType != getScriptClass(Text.iterateCodePoints(&nStartPos, 0)))
        return -1;

    if (nStartPos == 0) return 0;
    sal_uInt32 ch=0;
    while (iterateCodePoints(Text, nStartPos, -1, ch) >= 0 && ScriptType == getScriptClass(ch)) {
        if (nStartPos == 0) return 0;
    }

    return  iterateCodePoints(Text, nStartPos, 1, ch);
}

sal_Int32 SAL_CALL BreakIteratorImpl::endOfScript( const OUString& Text,
        sal_Int32 nStartPos, sal_Int16 ScriptType )
{
    if (nStartPos < 0 || nStartPos >= Text.getLength())
        return -1;

    if(ScriptType != getScriptClass(Text.iterateCodePoints(&nStartPos, 0)))
        return -1;

    sal_Int32 strLen = Text.getLength();
    sal_uInt32 ch=0;
    while(iterateCodePoints(Text, nStartPos, 1, ch) < strLen ) {
        sal_Int16 currentCharScriptType = getScriptClass(ch);
        if(ScriptType != currentCharScriptType && currentCharScriptType != ScriptType::WEAK)
            break;
    }
    return  nStartPos;
}

sal_Int32  SAL_CALL BreakIteratorImpl::previousScript( const OUString& Text,
        sal_Int32 nStartPos, sal_Int16 ScriptType )
{
    if (nStartPos < 0)
        return -1;
    if (nStartPos > Text.getLength())
        nStartPos = Text.getLength();

    sal_Int16 numberOfChange = (ScriptType == getScriptClass(Text.iterateCodePoints(&nStartPos, 0))) ? 3 : 2;

    sal_uInt32 ch=0;
    while (numberOfChange > 0 && iterateCodePoints(Text, nStartPos, -1, ch) >= 0) {
        if (((numberOfChange % 2) == 0) != (ScriptType != getScriptClass(ch)))
            numberOfChange--;
        else if (nStartPos == 0) {
            return -1;
        }
    }
    return numberOfChange == 0 ? iterateCodePoints(Text, nStartPos, 1, ch) : -1;
}

sal_Int32 SAL_CALL BreakIteratorImpl::nextScript( const OUString& Text, sal_Int32 nStartPos,
        sal_Int16 ScriptType )

{
    if (nStartPos < 0)
        nStartPos = 0;
    sal_Int32 strLen = Text.getLength();
    if (nStartPos >= strLen)
        return -1;

    sal_Int16 numberOfChange = (ScriptType == getScriptClass(Text.iterateCodePoints(&nStartPos, 0))) ? 2 : 1;

    sal_uInt32 ch=0;
    while (numberOfChange > 0 && iterateCodePoints(Text, nStartPos, 1, ch) < strLen) {
        sal_Int16 currentCharScriptType = getScriptClass(ch);
        if ((numberOfChange == 1) ? (ScriptType == currentCharScriptType) :
                (ScriptType != currentCharScriptType && currentCharScriptType != ScriptType::WEAK))
            numberOfChange--;
    }
    return numberOfChange == 0 ? nStartPos : -1;
}

sal_Int32 SAL_CALL BreakIteratorImpl::beginOfCharBlock( const OUString& Text, sal_Int32 nStartPos,
        const Locale& /*rLocale*/, sal_Int16 CharType )
{
    if (CharType == CharType::ANY_CHAR) return 0;
    if (nStartPos < 0 || nStartPos >= Text.getLength()) return -1;
    if (CharType != static_cast<sal_Int16>(u_charType( Text.iterateCodePoints(&nStartPos, 0)))) return -1;

    sal_Int32 nPos=nStartPos;
    while(nStartPos > 0 && CharType == static_cast<sal_Int16>(u_charType(Text.iterateCodePoints(&nPos, -1)))) { nStartPos=nPos; }
    return nStartPos; // begin of char block is inclusive
}

sal_Int32 SAL_CALL BreakIteratorImpl::endOfCharBlock( const OUString& Text, sal_Int32 nStartPos,
        const Locale& /*rLocale*/, sal_Int16 CharType )
{
    sal_Int32 strLen = Text.getLength();

    if (CharType == CharType::ANY_CHAR) return strLen; // end of char block is exclusive
    if (nStartPos < 0 || nStartPos >= strLen) return -1;
    if (CharType != static_cast<sal_Int16>(u_charType(Text.iterateCodePoints(&nStartPos, 0)))) return -1;

    sal_uInt32 ch=0;
    while(iterateCodePoints(Text, nStartPos, 1, ch) < strLen && CharType == static_cast<sal_Int16>(u_charType(ch))) {}
    return nStartPos; // end of char block is exclusive
}

sal_Int32 SAL_CALL BreakIteratorImpl::nextCharBlock( const OUString& Text, sal_Int32 nStartPos,
        const Locale& /*rLocale*/, sal_Int16 CharType )
{
    if (CharType == CharType::ANY_CHAR) return -1;
    if (nStartPos < 0 || nStartPos >= Text.getLength()) return -1;

    sal_Int16 numberOfChange = (CharType == static_cast<sal_Int16>(u_charType(Text.iterateCodePoints(&nStartPos, 0)))) ? 2 : 1;
    sal_Int32 strLen = Text.getLength();

    sal_uInt32 ch=0;
    while (numberOfChange > 0 && iterateCodePoints(Text, nStartPos, 1, ch) < strLen) {
        if ((CharType != static_cast<sal_Int16>(u_charType(ch))) != (numberOfChange == 1))
            numberOfChange--;
    }
    return numberOfChange == 0 ? nStartPos : -1;
}

sal_Int32 SAL_CALL BreakIteratorImpl::previousCharBlock( const OUString& Text, sal_Int32 nStartPos,
        const Locale& /*rLocale*/, sal_Int16 CharType )
{
    if(CharType == CharType::ANY_CHAR) return -1;
    if (nStartPos < 0 || nStartPos >= Text.getLength()) return -1;

    sal_Int16 numberOfChange = (CharType == static_cast<sal_Int16>(u_charType(Text.iterateCodePoints(&nStartPos, 0)))) ? 3 : 2;

    sal_uInt32 ch=0;
    while (numberOfChange > 0 && iterateCodePoints(Text, nStartPos, -1, ch) >= 0) {
        if (((numberOfChange % 2) == 0) != (CharType != static_cast<sal_Int16>(u_charType(ch))))
            numberOfChange--;
        if (nStartPos == 0 && numberOfChange > 0) {
            numberOfChange--;
            if (numberOfChange == 0) return nStartPos;
        }
    }
    return numberOfChange == 0 ? iterateCodePoints(Text, nStartPos, 1, ch) : -1;
}


sal_Int16 SAL_CALL BreakIteratorImpl::getWordType( const OUString& /*Text*/,
        sal_Int32 /*nPos*/, const Locale& /*rLocale*/ )
{
    return 0;
}

sal_Int16  BreakIteratorImpl::getScriptClass(sal_uInt32 currentChar)
{
    static sal_uInt32 lastChar = 0;
    static sal_Int16 nRet = ScriptType::WEAK;

    if (currentChar != lastChar)
    {
        lastChar = currentChar;
        nRet = i18nutil::GetScriptClass(currentChar);
    }

    return nRet;
}

bool BreakIteratorImpl::createLocaleSpecificBreakIterator(const OUString& aLocaleName)
{
    // to share service between same Language but different Country code, like zh_CN and zh_TW
    for (const lookupTableItem& listItem : lookupTable) {
        if (aLocaleName == listItem.aLocale.Language) {
            xBI = listItem.xBI;
            return true;
        }
    }

#if !WITH_LOCALE_ALL && !WITH_LOCALE_ja
    if (aLocaleName == "ja")
        return false;
#endif
#if !WITH_LOCALE_ALL && !WITH_LOCALE_zh
    if (aLocaleName == "zh" || aLocaleName == "zh_TW")
        return false;
#endif
#if !WITH_LOCALE_ALL && !WITH_LOCALE_ko
    if (aLocaleName == "ko")
        return false;
#endif

    Reference < uno::XInterface > xI = m_xContext->getServiceManager()->createInstanceWithContext(
            "com.sun.star.i18n.BreakIterator_" + aLocaleName, m_xContext);

    if ( xI.is() ) {
        xBI.set(xI, UNO_QUERY);
        if (xBI.is()) {
            lookupTable.emplace_back(Locale(aLocaleName, aLocaleName, aLocaleName), xBI);
            return true;
        }
    }
    return false;
}

const Reference < XBreakIterator > &
BreakIteratorImpl::getLocaleSpecificBreakIterator(const Locale& rLocale)
{
    if (xBI.is() && rLocale == aLocale)
        return xBI;
    else if (m_xContext.is()) {
        aLocale = rLocale;

        for (const lookupTableItem& listItem : lookupTable) {
            if (rLocale == listItem.aLocale)
            {
                xBI = listItem.xBI;
                return xBI;
            }
        }

        static constexpr OUString under(u"_"_ustr);

        sal_Int32 l = rLocale.Language.getLength();
        sal_Int32 c = rLocale.Country.getLength();
        sal_Int32 v = rLocale.Variant.getLength();

        if ((l > 0 && c > 0 && v > 0 &&
                    // load service with name <base>_<lang>_<country>_<variant>
                    createLocaleSpecificBreakIterator(rLocale.Language + under +
                            rLocale.Country + under + rLocale.Variant)) ||
                (l > 0 && c > 0 &&
                 // load service with name <base>_<lang>_<country>
                 createLocaleSpecificBreakIterator(rLocale.Language + under +
                         rLocale.Country)) ||
                (l > 0 && c > 0 && rLocale.Language == "zh" &&
                 (rLocale.Country == "HK" ||
                  rLocale.Country == "MO" ) &&
                 // if the country code is HK or MO, one more step to try TW.
                 createLocaleSpecificBreakIterator(rLocale.Language + under +
                         "TW")) ||
                (l > 0 &&
                 // load service with name <base>_<lang>
                 createLocaleSpecificBreakIterator(rLocale.Language)) ||
                // load default service with name <base>_Unicode
                createLocaleSpecificBreakIterator(u"Unicode"_ustr)) {
            lookupTable.emplace_back( aLocale, xBI );
            return xBI;
        }
    }
    throw RuntimeException(u"getLocaleSpecificBreakIterator: iterator not found"_ustr);
}

OUString SAL_CALL
BreakIteratorImpl::getImplementationName()
{
    return u"com.sun.star.i18n.BreakIterator"_ustr;
}

sal_Bool SAL_CALL
BreakIteratorImpl::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > SAL_CALL
BreakIteratorImpl::getSupportedServiceNames()
{
    return { u"com.sun.star.i18n.BreakIterator"_ustr };
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_i18n_BreakIterator_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new i18npool::BreakIteratorImpl(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
