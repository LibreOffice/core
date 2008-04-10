/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xdictionary.cxx,v $
 * $Revision: 1.18 $
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

// xdictionary.cpp: implementation of the xdictionary class.
//
//////////////////////////////////////////////////////////////////////


#include <rtl/ustrbuf.hxx>

#include <com/sun/star/i18n/WordType.hpp>
#include <xdictionary.hxx>
#include <unicode/uchar.h>
#include <string.h>
#include <breakiteratorImpl.hxx>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

extern "C" { static void SAL_CALL thisModule() {} }

xdictionary::xdictionary(const sal_Char *lang)
{
#ifdef SAL_DLLPREFIX
    OUStringBuffer aBuf( strlen(lang) + 7 + 6 );    // mostly "lib*.so" (with * == dict_zh)
    aBuf.appendAscii( SAL_DLLPREFIX );
#else
    OUStringBuffer aBuf( strlen(lang) + 7 + 4 );    // mostly "*.dll" (with * == dict_zh)
#endif
    aBuf.appendAscii( "dict_" ).appendAscii( lang ).appendAscii( SAL_DLLEXTENSION );
        hModule = osl_loadModuleRelative( &thisModule, aBuf.makeStringAndClear().pData, SAL_LOADMODULE_DEFAULT );
        if( hModule ) {
            sal_IntPtr (*func)();
            func = (sal_IntPtr(*)()) osl_getFunctionSymbol( hModule, OUString::createFromAscii("getExistMark").pData );
            existMark = (sal_uInt8*) (*func)();
            func = (sal_IntPtr(*)()) osl_getFunctionSymbol( hModule, OUString::createFromAscii("getIndex1").pData );
            index1 = (sal_Int16*) (*func)();
            func = (sal_IntPtr(*)()) osl_getFunctionSymbol( hModule, OUString::createFromAscii("getIndex2").pData );
            index2 = (sal_Int32*) (*func)();
            func = (sal_IntPtr(*)()) osl_getFunctionSymbol( hModule, OUString::createFromAscii("getLenArray").pData );
            lenArray = (sal_Int32*) (*func)();
            func = (sal_IntPtr(*)()) osl_getFunctionSymbol( hModule, OUString::createFromAscii("getDataArea").pData );
            dataArea = (sal_Unicode*) (*func)();
        }
        else
            existMark = NULL;
        for (sal_Int32 i = 0; i < CACHE_MAX; i++)
            cache[i].size = 0;

        // for CTL breakiterator, which the word boundary should not inside cell.
        useCellBoundary = sal_False;
        japaneseWordBreak = sal_False;
}

xdictionary::~xdictionary() {
        osl_unloadModule(hModule);
        for (sal_Int32 i = 0; i < CACHE_MAX; i++) {
            if (cache[i].size > 0) {
                delete cache[i].contents;
                delete cache[i].wordboundary;
            }
        }
}

void SAL_CALL xdictionary::setJapaneseWordBreak()
{
        japaneseWordBreak = sal_True;
}

sal_Bool xdictionary::exists(const sal_Unicode c) {
        sal_Bool exist = existMark ? sal::static_int_cast<sal_Bool>((existMark[c>>3] & (1<<(c&0x07))) != 0) : sal_False;
        if (!exist && japaneseWordBreak)
            return BreakIteratorImpl::getScriptClass(c) == ScriptType::ASIAN;
        else
            return exist;
}

sal_Int32 SAL_CALL
xdictionary::getLongestMatch(const sal_Unicode* str, sal_Int32 sLen) {

        sal_Int16 idx = index1[str[0] >> 8];

        if (idx == 0xFF) return 0;

        idx = (idx<<8) | (str[0]&0xff);

        sal_uInt32 begin = index2[idx], end = index2[idx+1];

        if (begin == 0) return 0;

        str++; sLen--; // first character is not stored in the dictionary
        for (sal_uInt32 i = end; i > begin; i--) {
            sal_Int32 len = lenArray[i] - lenArray[i - 1];
            if (sLen >= len) {
                const sal_Unicode *dstr = dataArea + lenArray[i-1];
                sal_Int32 pos = 0;

                while (pos < len && dstr[pos] == str[pos]) { pos++; }

                if (pos == len)
                    return len + 1;
            }
        }
        return 0;
}

/*
 * Compare two unicode string,
 */

sal_Bool SAL_CALL WordBreakCache::equals(const sal_Unicode* str, Boundary& boundary) {
        // Different length, different string.
        if (length != boundary.endPos - boundary.startPos) return sal_False;

        for (sal_Int32 i = 0; i < length; i++)
            if (contents[i] != str[i + boundary.startPos]) return sal_False;

        return sal_True;
}


/*
 * Retrieve the segment containing the character at pos.
 * @param pos : Position of the given character.
 * @return true if CJK.
 */
sal_Bool SAL_CALL xdictionary::seekSegment(const sal_Unicode *text, sal_Int32 pos,
                sal_Int32 len, Boundary& segBoundary) {
        for (segBoundary.startPos = pos - 1;
            segBoundary.startPos >= 0 &&
                (u_isWhitespace((sal_uInt32)text[segBoundary.startPos]) || exists(text[segBoundary.startPos]));
            segBoundary.startPos--);
        segBoundary.startPos++;

        for (segBoundary.endPos = pos;
            segBoundary.endPos < len &&
                    (u_isWhitespace((sal_uInt32)text[segBoundary.endPos]) || exists(text[segBoundary.endPos]));
            segBoundary.endPos++);

        return segBoundary.endPos > segBoundary.startPos + 1;
}

#define KANJA       1
#define KATAKANA    2
#define HIRAKANA    3

static sal_Int16 SAL_CALL JapaneseCharType(sal_Unicode c)
{
    if (0x3041 <= c && c <= 0x309e)
        return HIRAKANA;
    if (0x30a1 <= c && c <= 0x30fe || 0xff65 <= c && c <= 0xff9f)
        return KATAKANA;
    return KANJA;
}

WordBreakCache& SAL_CALL xdictionary::getCache(const sal_Unicode *text, Boundary& wordBoundary)
{

        WordBreakCache& aCache = cache[text[0] & 0x1f];

        if (aCache.size != 0 && aCache.equals(text, wordBoundary))
            return aCache;

        sal_Int32 len = wordBoundary.endPos - wordBoundary.startPos;

        if (aCache.size == 0 || len > aCache.size) {
            if (aCache.size != 0) {
                delete aCache.contents;
                delete aCache.wordboundary;
                aCache.size = len;
            }
            else
                aCache.size = len > DEFAULT_SIZE ? len : DEFAULT_SIZE;
            aCache.contents = new sal_Unicode[aCache.size + 1];
            aCache.wordboundary = new sal_Int32[aCache.size + 2];
        }
        aCache.length  = len;
        memcpy(aCache.contents, text + wordBoundary.startPos, len * sizeof(sal_Unicode));
        *(aCache.contents + len) = 0x0000;
        // reset the wordboundary in cache
        memset(aCache.wordboundary, '\0', sizeof(sal_Int32)*(len + 2));

        sal_Int32 i = 0;        // loop variable
        while (aCache.wordboundary[i] < aCache.length) {
            len = 0;
            // look the continuous white space as one word and cashe it
            while (u_isWhitespace((sal_uInt32)text[wordBoundary.startPos + aCache.wordboundary[i] + len]))
                len ++;

            if (len == 0) {
                const sal_Unicode *str = text + wordBoundary.startPos + aCache.wordboundary[i];
                sal_Int32 slen = aCache.length - aCache.wordboundary[i];
                sal_Int16 type = 0, count = 0;
                for (;len == 0 && slen > 0; str++, slen--) {
                    len = getLongestMatch(str, slen);
                    if (len == 0) {
                        if (!japaneseWordBreak) {
                            len = 1;
                        } else {
                            if (count == 0)
                                type = JapaneseCharType(*str);
                            else if (type != JapaneseCharType(*str))
                                break;
                            count++;
                        }
                    }
                }
                if (count) {
                    aCache.wordboundary[i+1] = aCache.wordboundary[i] + count;
                    i++;
                    if (useCellBoundary) {
                        sal_Int32 cBoundary = cellBoundary[aCache.wordboundary[i] + wordBoundary.startPos - 1];
                        if (cBoundary > 0)
                            aCache.wordboundary[i] = cBoundary - wordBoundary.startPos;
                    }
                }
            }

            if (len) {
                aCache.wordboundary[i+1] = aCache.wordboundary[i] + len;
                i++;

                if (useCellBoundary) {
                    sal_Int32 cBoundary = cellBoundary[aCache.wordboundary[i] + wordBoundary.startPos - 1];
                    if (cBoundary > 0)
                        aCache.wordboundary[i] = cBoundary - wordBoundary.startPos;
                }
            }
        }
        aCache.wordboundary[i + 1] = aCache.length + 1;

        return aCache;
}

Boundary SAL_CALL xdictionary::previousWord(const OUString& rText, sal_Int32 anyPos, sal_Int16 wordType)
{
        // looking for the first non-whitespace character from anyPos
        sal_uInt32 ch = rText.iterateCodePoints(&anyPos, -1);

        while (anyPos > 0 && u_isWhitespace(ch)) ch = rText.iterateCodePoints(&anyPos, -1);

        return getWordBoundary(rText, anyPos, wordType, true);
}

Boundary SAL_CALL xdictionary::nextWord(const OUString& rText, sal_Int32 anyPos, sal_Int16 wordType)
{
        boundary = getWordBoundary(rText, anyPos, wordType, true);
        anyPos = boundary.endPos;
        if (anyPos < rText.getLength()) {
            // looknig for the first non-whitespace character from anyPos
            sal_uInt32 ch = rText.iterateCodePoints(&anyPos, 1);
            while (u_isWhitespace(ch)) ch=rText.iterateCodePoints(&anyPos, 1);
            rText.iterateCodePoints(&anyPos, -1);
        }

        return getWordBoundary(rText, anyPos, wordType, true);
}

Boundary SAL_CALL xdictionary::getWordBoundary(const OUString& rText, sal_Int32 anyPos, sal_Int16 wordType, sal_Bool bDirection)
{
        const sal_Unicode *text=rText.getStr();
        sal_Int32 len=rText.getLength();
        if (anyPos >= len || anyPos < 0) {
            boundary.startPos = boundary.endPos = anyPos < 0 ? 0 : len;
        } else if (seekSegment(text, anyPos, len, boundary)) {          // character in dict
            WordBreakCache& aCache = getCache(text, boundary);
            sal_Int32 i = 0;

            while (aCache.wordboundary[i] <= (sal_Int32)anyPos - boundary.startPos) i++;

            sal_Int32 startPos = aCache.wordboundary[i - 1];
            // if bDirection is false
            if (!bDirection && startPos > 0 && startPos == (anyPos - boundary.startPos) &&
                                                u_isWhitespace((sal_uInt32) text[anyPos - 1]))
                i--;
            boundary.endPos = aCache.wordboundary[i] + boundary.startPos;
            boundary.startPos += aCache.wordboundary[i - 1];
        } else {
            boundary.startPos = anyPos;
            if (anyPos < len) rText.iterateCodePoints(&anyPos, 1);
            boundary.endPos = anyPos < len ? anyPos : len;
        }
        if (wordType == WordType::WORD_COUNT) {
            // skip punctuation for word count.
            while (boundary.endPos < len && u_ispunct((sal_uInt32)text[boundary.endPos]))
                boundary.endPos++;
        }

        return boundary;
}


void SAL_CALL xdictionary::setCellBoundary(sal_Int32* cellArray)
{
        useCellBoundary = sal_True;
        cellBoundary = cellArray;
}

} } } }
