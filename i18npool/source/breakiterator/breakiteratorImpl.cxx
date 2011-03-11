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

#include <breakiteratorImpl.hxx>
#include <unicode/uchar.h>
#include <rtl/ustrbuf.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

BreakIteratorImpl::BreakIteratorImpl( const Reference < XMultiServiceFactory >& rxMSF ) : xMSF( rxMSF )
{
}

BreakIteratorImpl::BreakIteratorImpl()
{
}

BreakIteratorImpl::~BreakIteratorImpl()
{
        // Clear lookuptable
        for (size_t l = 0; l < lookupTable.size(); l++)
            delete lookupTable[l];
        lookupTable.clear();
}

#define LBI getLocaleSpecificBreakIterator(rLocale)

sal_Int32 SAL_CALL BreakIteratorImpl::nextCharacters( const OUString& Text, sal_Int32 nStartPos,
        const Locale &rLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 nCount, sal_Int32& nDone )
        throw(RuntimeException)
{
        if (nCount < 0) throw RuntimeException();

        return LBI->nextCharacters( Text, nStartPos, rLocale, nCharacterIteratorMode, nCount, nDone);
}

sal_Int32 SAL_CALL BreakIteratorImpl::previousCharacters( const OUString& Text, sal_Int32 nStartPos,
        const Locale& rLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 nCount, sal_Int32& nDone )
        throw(RuntimeException)
{
        if (nCount < 0) throw RuntimeException();

        return LBI->previousCharacters( Text, nStartPos, rLocale, nCharacterIteratorMode, nCount, nDone);
}

#define isZWSP(c) (ch == 0x200B)

static sal_Int32 skipSpace(const OUString& Text, sal_Int32 nPos, sal_Int32 len, sal_Int16 rWordType, sal_Bool bDirection)
{
        sal_uInt32 ch=0;
        sal_Int32 pos=nPos;
        switch (rWordType) {
            case WordType::ANYWORD_IGNOREWHITESPACES:
                if (bDirection)
                    while (nPos < len && (u_isWhitespace(ch = Text.iterateCodePoints(&pos, 1)) || isZWSP(ch))) nPos=pos;
                else
                    while (nPos > 0 && (u_isWhitespace(ch = Text.iterateCodePoints(&pos, -1)) || isZWSP(ch))) nPos=pos;
            break;
            case WordType::DICTIONARY_WORD:
                if (bDirection)
                    while (nPos < len && (u_isWhitespace(ch = Text.iterateCodePoints(&pos, 1)) || isZWSP(ch) ||
                            ! (ch == 0x002E || u_isalnum(ch)))) nPos=pos;
                else
                    while (nPos > 0 && (u_isWhitespace(ch = Text.iterateCodePoints(&pos, -1)) || isZWSP(ch) ||
                            ! (ch == 0x002E || u_isalnum(ch)))) nPos=pos;
            break;
            case WordType::WORD_COUNT:
                if (bDirection)
                    while (nPos < len && (u_isUWhiteSpace(ch = Text.iterateCodePoints(&pos, 1)) || isZWSP(ch))) nPos=pos;
                else
                    while (nPos > 0 && (u_isUWhiteSpace(ch = Text.iterateCodePoints(&pos, -1)) || isZWSP(ch))) nPos=pos;
            break;
        }
        return nPos;
}

Boundary SAL_CALL BreakIteratorImpl::nextWord( const OUString& Text, sal_Int32 nStartPos,
        const Locale& rLocale, sal_Int16 rWordType ) throw(RuntimeException)
{
        sal_Int32 len = Text.getLength();
        if( nStartPos < 0 || len == 0 )
            result.endPos = result.startPos = 0;
        else if (nStartPos >= len)
            result.endPos = result.startPos = len;
        else {
            result = LBI->nextWord(Text, nStartPos, rLocale, rWordType);

            nStartPos = skipSpace(Text, result.startPos, len, rWordType, sal_True);

            if ( nStartPos != result.startPos) {
                if( nStartPos >= len )
                    result.startPos = result.endPos = len;
                else {
                    result = LBI->getWordBoundary(Text, nStartPos, rLocale, rWordType, sal_True);
                    // i88041: avoid startPos goes back to nStartPos when switching between Latin and CJK scripts
                    if (result.startPos < nStartPos) result.startPos = nStartPos;
                }
            }
        }
        return result;
}

static inline sal_Bool SAL_CALL isCJK( const Locale& rLocale ) {
        return rLocale.Language.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("zh")) || rLocale.Language.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("ja")) || rLocale.Language.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("ko"));
}

Boundary SAL_CALL BreakIteratorImpl::previousWord( const OUString& Text, sal_Int32 nStartPos,
        const Locale& rLocale, sal_Int16 rWordType) throw(RuntimeException)
{
        sal_Int32 len = Text.getLength();
        if( nStartPos <= 0 || len == 0 ) {
            result.endPos = result.startPos = 0;
            return result;
        } else if (nStartPos > len) {
            result.endPos = result.startPos = len;
            return result;
        }

        sal_Int32 nPos = skipSpace(Text, nStartPos, len, rWordType, sal_False);

        // if some spaces are skiped, and the script type is Asian with no CJK rLocale, we have to return
        // (nStartPos, -1) for caller to send correct rLocale for loading correct dictionary.
        result.startPos = nPos;
        if (nPos != nStartPos && nPos > 0 && !isCJK(rLocale) && getScriptClass(Text.iterateCodePoints(&nPos, -1)) == ScriptType::ASIAN) {
            result.endPos = -1;
            return result;
        }

        return LBI->previousWord(Text, result.startPos, rLocale, rWordType);
}


Boundary SAL_CALL BreakIteratorImpl::getWordBoundary( const OUString& Text, sal_Int32 nPos, const Locale& rLocale,
        sal_Int16 rWordType, sal_Bool bDirection ) throw(RuntimeException)
{
        sal_Int32 len = Text.getLength();
        if( nPos < 0 || len == 0 )
            result.endPos = result.startPos = 0;
        else if (nPos > len)
            result.endPos = result.startPos = len;
        else {
            sal_Int32 next, prev;
            next = skipSpace(Text, nPos, len, rWordType, sal_True);
            prev = skipSpace(Text, nPos, len, rWordType, sal_False);
            if (prev == 0 && next == len) {
                result.endPos = result.startPos = nPos;
            } else if (prev == 0 && ! bDirection) {
                result.endPos = result.startPos = 0;
            } else if (next == len && bDirection) {
                result.endPos = result.startPos = len;
            } else {
                if (next != prev) {
                    if (next == nPos && next != len)
                        bDirection = sal_True;
                    else if (prev == nPos && prev != 0)
                        bDirection = sal_False;
                    else
                        nPos = bDirection ? next : prev;
                }
                result = LBI->getWordBoundary(Text, nPos, rLocale, rWordType, bDirection);
            }
        }
        return result;
}

sal_Bool SAL_CALL BreakIteratorImpl::isBeginWord( const OUString& Text, sal_Int32 nPos,
        const Locale& rLocale, sal_Int16 rWordType ) throw(RuntimeException)
{
        sal_Int32 len = Text.getLength();

        if (nPos < 0 || nPos >= len) return sal_False;

        sal_Int32 tmp = skipSpace(Text, nPos, len, rWordType, sal_True);

        if (tmp != nPos) return sal_False;

        result = getWordBoundary(Text, nPos, rLocale, rWordType, sal_True);

        return result.startPos == nPos;
}

sal_Bool SAL_CALL BreakIteratorImpl::isEndWord( const OUString& Text, sal_Int32 nPos,
        const Locale& rLocale, sal_Int16 rWordType ) throw(RuntimeException)
{
        sal_Int32 len = Text.getLength();

        if (nPos <= 0 || nPos > len) return sal_False;

        sal_Int32 tmp = skipSpace(Text, nPos, len, rWordType, sal_False);

        if (tmp != nPos) return sal_False;

        result = getWordBoundary(Text, nPos, rLocale, rWordType, sal_False);

        return result.endPos == nPos;
}

sal_Int32 SAL_CALL BreakIteratorImpl::beginOfSentence( const OUString& Text, sal_Int32 nStartPos,
        const Locale &rLocale ) throw(RuntimeException)
{
        if (nStartPos < 0 || nStartPos > Text.getLength())
            return -1;
        if (Text.getLength() == 0) return 0;
        return LBI->beginOfSentence(Text, nStartPos, rLocale);
}

sal_Int32 SAL_CALL BreakIteratorImpl::endOfSentence( const OUString& Text, sal_Int32 nStartPos,
        const Locale &rLocale ) throw(RuntimeException)
{
        if (nStartPos < 0 || nStartPos > Text.getLength())
            return -1;
        if (Text.getLength() == 0) return 0;
        return LBI->endOfSentence(Text, nStartPos, rLocale);
}

LineBreakResults SAL_CALL BreakIteratorImpl::getLineBreak( const OUString& Text, sal_Int32 nStartPos,
        const Locale& rLocale, sal_Int32 nMinBreakPos, const LineBreakHyphenationOptions& hOptions,
        const LineBreakUserOptions& bOptions ) throw(RuntimeException)
{
        return LBI->getLineBreak(Text, nStartPos, rLocale, nMinBreakPos, hOptions, bOptions);
}

sal_Int16 SAL_CALL BreakIteratorImpl::getScriptType( const OUString& Text, sal_Int32 nPos )
        throw(RuntimeException)
{
        return (nPos < 0 || nPos >= Text.getLength()) ? ScriptType::WEAK :
                            getScriptClass(Text.iterateCodePoints(&nPos, 0));
}


/** Increments/decrements position first, then obtains character.
    @return current position, may be -1 or text length if string was consumed.
 */
static sal_Int32 SAL_CALL iterateCodePoints(const OUString& Text, sal_Int32 &nStartPos, sal_Int32 inc, sal_uInt32& ch) {
        sal_Int32 nLen = Text.getLength();
        if (nStartPos + inc < 0 || nStartPos + inc >= nLen) {
            ch = 0;
            nStartPos = nStartPos + inc < 0 ? -1 : nLen;
        } else {
            ch = Text.iterateCodePoints(&nStartPos, inc);
            // Fix for #i80436#.
            // erAck: 2009-06-30T21:52+0200  This logic looks somewhat
            // suspicious as if it cures a symptom.. anyway, had to add
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
        sal_Int32 nStartPos, sal_Int16 ScriptType ) throw(RuntimeException)
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
        sal_Int32 nStartPos, sal_Int16 ScriptType ) throw(RuntimeException)
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
        sal_Int32 nStartPos, sal_Int16 ScriptType ) throw(RuntimeException)
{
        if (nStartPos < 0)
            return -1;
        if (nStartPos > Text.getLength())
            nStartPos = Text.getLength();

        sal_Int16 numberOfChange = (ScriptType == getScriptClass(Text.iterateCodePoints(&nStartPos, 0))) ? 3 : 2;

        sal_uInt32 ch=0;
        while (numberOfChange > 0 && iterateCodePoints(Text, nStartPos, -1, ch) >= 0) {
            if ((((numberOfChange % 2) == 0) ^ (ScriptType != getScriptClass(ch))))
                numberOfChange--;
            else if (nStartPos == 0) {
                if (numberOfChange > 0)
                    numberOfChange--;
                if (nStartPos > 0)
                    Text.iterateCodePoints(&nStartPos, -1);
                else
                    return -1;
            }
        }
        return numberOfChange == 0 ? iterateCodePoints(Text, nStartPos, 1, ch) : -1;
}

sal_Int32 SAL_CALL BreakIteratorImpl::nextScript( const OUString& Text, sal_Int32 nStartPos,
        sal_Int16 ScriptType ) throw(RuntimeException)

{
        if (nStartPos < 0)
            nStartPos = 0;
        sal_Int32 strLen = Text.getLength();
        if (nStartPos > strLen)
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
        const Locale& /*rLocale*/, sal_Int16 CharType ) throw(RuntimeException)
{
        if (CharType == CharType::ANY_CHAR) return 0;
        if (nStartPos < 0 || nStartPos >= Text.getLength()) return -1;
        if (CharType != (sal_Int16)u_charType( Text.iterateCodePoints(&nStartPos, 0))) return -1;

        sal_Int32 nPos=nStartPos;
        while(nStartPos > 0 && CharType == (sal_Int16)u_charType(Text.iterateCodePoints(&nPos, -1))) { nStartPos=nPos; }
        return nStartPos; // begin of char block is inclusive
}

sal_Int32 SAL_CALL BreakIteratorImpl::endOfCharBlock( const OUString& Text, sal_Int32 nStartPos,
        const Locale& /*rLocale*/, sal_Int16 CharType ) throw(RuntimeException)
{
        sal_Int32 strLen = Text.getLength();

        if (CharType == CharType::ANY_CHAR) return strLen; // end of char block is exclusive
        if (nStartPos < 0 || nStartPos >= strLen) return -1;
        if (CharType != (sal_Int16)u_charType(Text.iterateCodePoints(&nStartPos, 0))) return -1;

        sal_uInt32 ch=0;
        while(iterateCodePoints(Text, nStartPos, 1, ch) < strLen && CharType == (sal_Int16)u_charType(ch)) {}
        return nStartPos; // end of char block is exclusive
}

sal_Int32 SAL_CALL BreakIteratorImpl::nextCharBlock( const OUString& Text, sal_Int32 nStartPos,
        const Locale& /*rLocale*/, sal_Int16 CharType ) throw(RuntimeException)
{
        if (CharType == CharType::ANY_CHAR) return -1;
        if (nStartPos < 0 || nStartPos >= Text.getLength()) return -1;

        sal_Int16 numberOfChange = (CharType == (sal_Int16)u_charType(Text.iterateCodePoints(&nStartPos, 0))) ? 2 : 1;
        sal_Int32 strLen = Text.getLength();

    sal_uInt32 ch=0;
    while (numberOfChange > 0 && iterateCodePoints(Text, nStartPos, 1, ch) < strLen) {
        if ((CharType != (sal_Int16)u_charType(ch)) ^ (numberOfChange == 1))
            numberOfChange--;
    }
    return numberOfChange == 0 ? nStartPos : -1;
}

sal_Int32 SAL_CALL BreakIteratorImpl::previousCharBlock( const OUString& Text, sal_Int32 nStartPos,
        const Locale& /*rLocale*/, sal_Int16 CharType ) throw(RuntimeException)
{
        if(CharType == CharType::ANY_CHAR) return -1;
        if (nStartPos < 0 || nStartPos >= Text.getLength()) return -1;

        sal_Int16 numberOfChange = (CharType == (sal_Int16)u_charType(Text.iterateCodePoints(&nStartPos, 0))) ? 3 : 2;

        sal_uInt32 ch=0;
        while (numberOfChange > 0 && iterateCodePoints(Text, nStartPos, -1, ch) >= 0) {
            if (((numberOfChange % 2) == 0) ^ (CharType != (sal_Int16)u_charType(ch)))
                numberOfChange--;
            if (nStartPos == 0 && numberOfChange > 0) {
                numberOfChange--;
                if (numberOfChange == 0) return nStartPos;
            }
        }
        return numberOfChange == 0 ? iterateCodePoints(Text, nStartPos, 1, ch) : -1;
}



sal_Int16 SAL_CALL BreakIteratorImpl::getWordType( const OUString& /*Text*/,
        sal_Int32 /*nPos*/, const Locale& /*rLocale*/ ) throw(RuntimeException)
{
        return 0;
}

static sal_Int16 scriptTypes[] = {
    ScriptType::WEAK, ScriptType::WEAK, ScriptType::COMPLEX, ScriptType::LATIN, ScriptType::COMPLEX,
    ScriptType::ASIAN, ScriptType::LATIN, ScriptType::LATIN, ScriptType::LATIN, ScriptType::COMPLEX,
    ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::LATIN, ScriptType::LATIN, ScriptType::LATIN,
// 15
    ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::ASIAN, ScriptType::ASIAN, ScriptType::COMPLEX,
    ScriptType::ASIAN, ScriptType::COMPLEX, ScriptType::ASIAN, ScriptType::COMPLEX, ScriptType::COMPLEX,
    ScriptType::LATIN, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::LATIN,
// 30
    ScriptType::LATIN, ScriptType::COMPLEX, ScriptType::LATIN, ScriptType::COMPLEX, ScriptType::COMPLEX,
    ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
    ScriptType::LATIN, ScriptType::ASIAN, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
// 45
    ScriptType::COMPLEX, ScriptType::LATIN, ScriptType::LATIN, ScriptType::COMPLEX, ScriptType::COMPLEX,
    ScriptType::LATIN, ScriptType::LATIN, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::LATIN,
    ScriptType::COMPLEX, ScriptType::LATIN, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
// 60
    ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
    ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::LATIN, ScriptType::LATIN, ScriptType::COMPLEX,
    ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::ASIAN, ScriptType::ASIAN,
// 75
    ScriptType::COMPLEX, ScriptType::LATIN, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
    ScriptType::LATIN, ScriptType::LATIN, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
    ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
// 90
    ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
    ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
    ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::WEAK, ScriptType::WEAK, ScriptType::COMPLEX,
// 105
    ScriptType::ASIAN, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
    ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
    ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::ASIAN,
// 120
    ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
    ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::WEAK, ScriptType::WEAK,
    ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
// 135
    ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
    ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX, ScriptType::COMPLEX,
    ScriptType::COMPLEX,
    ScriptType::WEAK};

sal_Int16  BreakIteratorImpl::getScriptClass(sal_uInt32 currentChar)
{
        static sal_uInt32 lastChar = 0;
        static sal_Int16 nRet = 0;

        if (currentChar != lastChar) {
            lastChar = currentChar;

            int32_t script = u_getIntPropertyValue(currentChar, UCHAR_SCRIPT);
            if (script < 0)
                nRet = ScriptType::WEAK;
            else if (static_cast<size_t>(script) >= SAL_N_ELEMENTS(scriptTypes))
                nRet = ScriptType::COMPLEX;         // anything new is going to be pretty wild
            else
                nRet = scriptTypes[script];
        }
        return nRet;
}

static inline sal_Bool operator == (const Locale& l1, const Locale& l2) {
        return l1.Language == l2.Language && l1.Country == l2.Country && l1.Variant == l2.Variant;
}

sal_Bool SAL_CALL BreakIteratorImpl::createLocaleSpecificBreakIterator(const OUString& aLocaleName) throw( RuntimeException )
{
        // to share service between same Language but different Country code, like zh_CN and zh_TW
        for (size_t l = 0; l < lookupTable.size(); l++) {
            lookupTableItem *listItem = lookupTable[l];
            if (aLocaleName == listItem->aLocale.Language) {
                xBI = listItem->xBI;
                return sal_True;
            }
        }

        Reference < uno::XInterface > xI = xMSF->createInstance(
            OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.i18n.BreakIterator_")) + aLocaleName);

        if ( xI.is() ) {
            xI->queryInterface( getCppuType((const Reference< XBreakIterator>*)0) ) >>= xBI;
            if (xBI.is()) {
                lookupTable.push_back(new lookupTableItem(Locale(aLocaleName, aLocaleName, aLocaleName), xBI));
                return sal_True;
            }
        }
        return sal_False;
}

Reference < XBreakIterator > SAL_CALL
BreakIteratorImpl::getLocaleSpecificBreakIterator(const Locale& rLocale) throw (RuntimeException)
{
        if (xBI.is() && rLocale == aLocale)
            return xBI;
        else if (xMSF.is()) {
            aLocale = rLocale;

            for (size_t i = 0; i < lookupTable.size(); i++) {
                lookupTableItem *listItem = lookupTable[i];
                if (rLocale == listItem->aLocale)
                    return xBI = listItem->xBI;
            }

            sal_Unicode under = (sal_Unicode)'_';

            sal_Int32 l = rLocale.Language.getLength();
            sal_Int32 c = rLocale.Country.getLength();
            sal_Int32 v = rLocale.Variant.getLength();
            OUStringBuffer aBuf(l+c+v+3);

            if ((l > 0 && c > 0 && v > 0 &&
                    // load service with name <base>_<lang>_<country>_<varian>
                    createLocaleSpecificBreakIterator(aBuf.append(rLocale.Language).append(under).append(
                                    rLocale.Country).append(under).append(rLocale.Variant).makeStringAndClear())) ||
                (l > 0 && c > 0 &&
                    // load service with name <base>_<lang>_<country>
                    createLocaleSpecificBreakIterator(aBuf.append(rLocale.Language).append(under).append(
                                    rLocale.Country).makeStringAndClear())) ||
                (l > 0 && c > 0 && rLocale.Language.compareToAscii("zh") == 0 &&
                                    (rLocale.Country.compareToAscii("HK") == 0 ||
                                    rLocale.Country.compareToAscii("MO") == 0) &&
                    // if the country code is HK or MO, one more step to try TW.
                    createLocaleSpecificBreakIterator(aBuf.append(rLocale.Language).append(under).appendAscii(
                                    "TW").makeStringAndClear())) ||
                (l > 0 &&
                    // load service with name <base>_<lang>
                    createLocaleSpecificBreakIterator(rLocale.Language)) ||
                    // load default service with name <base>_Unicode
                    createLocaleSpecificBreakIterator(OUString(RTL_CONSTASCII_USTRINGPARAM("Unicode")))) {
                lookupTable.push_back( new lookupTableItem(aLocale, xBI) );
                return xBI;
            }
        }
        throw RuntimeException();
}

const sal_Char cBreakIterator[] = "com.sun.star.i18n.BreakIterator";

OUString SAL_CALL
BreakIteratorImpl::getImplementationName(void) throw( RuntimeException )
{
        return OUString::createFromAscii(cBreakIterator);
}

sal_Bool SAL_CALL
BreakIteratorImpl::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
        return !rServiceName.compareToAscii(cBreakIterator);
}

Sequence< OUString > SAL_CALL
BreakIteratorImpl::getSupportedServiceNames(void) throw( RuntimeException )
{
        Sequence< OUString > aRet(1);
        aRet[0] = OUString::createFromAscii(cBreakIterator);
        return aRet;
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
