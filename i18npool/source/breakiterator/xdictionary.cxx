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


namespace com { namespace sun { namespace star { namespace i18n {

#ifndef DISABLE_DYNLOADING

extern "C" { static void SAL_CALL thisModule() {} }

#else

extern "C" {

sal_uInt8* getExistMark_ja();
sal_Int16* getIndex1_ja();
sal_Int32* getIndex2_ja();
sal_Int32* getLenArray_ja();
sal_Unicode* getDataArea_ja();

sal_uInt8* getExistMark_zh();
sal_Int16* getIndex1_zh();
sal_Int32* getIndex2_zh();
sal_Int32* getLenArray_zh();
sal_Unicode* getDataArea_zh();

}

#endif

xdictionary::xdictionary(const sal_Char *lang) :
    existMark( NULL ),
    index1( NULL ),
    index2( NULL ),
    lenArray( NULL ),
    dataArea( NULL ),
#ifndef DISABLE_DYNLOADING
    hModule( NULL ),
#endif
    boundary(),
    japaneseWordBreak( sal_False )
{
    index1 = 0;
#ifndef DISABLE_DYNLOADING
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
        func = (sal_IntPtr(*)()) osl_getFunctionSymbol( hModule, OUString("getExistMark").pData );
        existMark = (sal_uInt8*) (*func)();
        func = (sal_IntPtr(*)()) osl_getFunctionSymbol( hModule, OUString("getIndex1").pData );
        index1 = (sal_Int16*) (*func)();
        func = (sal_IntPtr(*)()) osl_getFunctionSymbol( hModule, OUString("getIndex2").pData );
        index2 = (sal_Int32*) (*func)();
        func = (sal_IntPtr(*)()) osl_getFunctionSymbol( hModule, OUString("getLenArray").pData );
        lenArray = (sal_Int32*) (*func)();
        func = (sal_IntPtr(*)()) osl_getFunctionSymbol( hModule, OUString("getDataArea").pData );
        dataArea = (sal_Unicode*) (*func)();
    }
    else
    {
        existMark = NULL;
        index1 = NULL;
        index2 = NULL;
        lenArray = NULL;
        dataArea = NULL;
    }

#else
    if( strcmp( lang, "ja" ) == 0 ) {
        existMark = getExistMark_ja();
        index1 = getIndex1_ja();
        index2 = getIndex2_ja();
        lenArray = getLenArray_ja();
        dataArea = getDataArea_ja();
    }
    else if( strcmp( lang, "zh" ) == 0 ) {
        existMark = getExistMark_zh();
        index1 = getIndex1_zh();
        index2 = getIndex2_zh();
        lenArray = getLenArray_zh();
        dataArea = getDataArea_zh();
    }
    else
    {
        existMark = NULL;
        index1 = NULL;
        index2 = NULL;
        lenArray = NULL;
        dataArea = NULL;
    }
#endif

    for (sal_Int32 i = 0; i < CACHE_MAX; i++)
        cache[i].size = 0;

    japaneseWordBreak = sal_False;
}

xdictionary::~xdictionary()
{
#ifndef DISABLE_DYNLOADING
        osl_unloadModule(hModule);
#endif
        for (sal_Int32 i = 0; i < CACHE_MAX; i++) {
            if (cache[i].size > 0) {
                delete [] cache[i].contents;
                delete [] cache[i].wordboundary;
            }
        }
}

void xdictionary::setJapaneseWordBreak()
{
    japaneseWordBreak = sal_True;
}

sal_Bool xdictionary::exists(const sal_uInt32 c)
{
    // 0x1FFF is the hardcoded limit in gendict for existMarks
    sal_Bool exist = (existMark && ((c>>3) < 0x1FFF)) ? sal::static_int_cast<sal_Bool>((existMark[c>>3] & (1<<(c&0x07))) != 0) : sal_False;
    if (!exist && japaneseWordBreak)
        return BreakIteratorImpl::getScriptClass(c) == ScriptType::ASIAN;
    else
        return exist;
}

sal_Int32 xdictionary::getLongestMatch(const sal_Unicode* str, sal_Int32 sLen)
{

    if ( !index1 ) return 0;

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
 * c-tor
 */

WordBreakCache::WordBreakCache() :
    length( 0 ),
    contents( NULL ),
    wordboundary( NULL ),
    size( 0 )
{
}

/*
 * Compare two unicode string,
 */

sal_Bool WordBreakCache::equals(const sal_Unicode* str, Boundary& boundary)
{
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
sal_Bool xdictionary::seekSegment(const OUString &rText, sal_Int32 pos,
    Boundary& segBoundary)
{
    sal_Int32 indexUtf16;
    segBoundary.endPos = segBoundary.startPos = pos;

    indexUtf16 = pos;
    while (indexUtf16 > 0)
    {
        sal_uInt32 ch = rText.iterateCodePoints(&indexUtf16, -1);
        if (u_isWhitespace(ch) || exists(ch))
            segBoundary.startPos = indexUtf16;
        else
            break;
    }

    indexUtf16 = pos;
    while (indexUtf16 < rText.getLength())
    {
        sal_uInt32 ch = rText.iterateCodePoints(&indexUtf16, 1);
        if (u_isWhitespace(ch) || exists(ch))
            segBoundary.endPos = indexUtf16;
        else
            break;
    }

    indexUtf16 = segBoundary.startPos;
    rText.iterateCodePoints(&indexUtf16, 1);
    return segBoundary.endPos > indexUtf16;
}

#define KANJA       1
#define KATAKANA    2
#define HIRAKANA    3

static sal_Int16 JapaneseCharType(sal_Unicode c)
{
    if (0x3041 <= c && c <= 0x309e)
        return HIRAKANA;
    if ((0x30a1 <= c && c <= 0x30fe) || (0xff65 <= c && c <= 0xff9f))
        return KATAKANA;
    return KANJA;
}

WordBreakCache& xdictionary::getCache(const sal_Unicode *text, Boundary& wordBoundary)
{
    WordBreakCache& rCache = cache[text[0] & 0x1f];

    if (rCache.size != 0 && rCache.equals(text, wordBoundary))
        return rCache;

    sal_Int32 len = wordBoundary.endPos - wordBoundary.startPos;

    if (rCache.size == 0 || len > rCache.size) {
        if (rCache.size != 0) {
            delete [] rCache.contents;
            delete [] rCache.wordboundary;
            rCache.size = len;
        }
        else
            rCache.size = len > DEFAULT_SIZE ? len : DEFAULT_SIZE;
        rCache.contents = new sal_Unicode[rCache.size + 1];
        rCache.wordboundary = new sal_Int32[rCache.size + 2];
    }
    rCache.length  = len;
    memcpy(rCache.contents, text + wordBoundary.startPos, len * sizeof(sal_Unicode));
    *(rCache.contents + len) = 0x0000;
    // reset the wordboundary in cache
    memset(rCache.wordboundary, '\0', sizeof(sal_Int32)*(len + 2));

    sal_Int32 i = 0;        // loop variable
    while (rCache.wordboundary[i] < rCache.length) {
        len = 0;
        // look the continuous white space as one word and cashe it
        while (u_isWhitespace((sal_uInt32)text[wordBoundary.startPos + rCache.wordboundary[i] + len]))
            len ++;

        if (len == 0) {
            const sal_Unicode *str = text + wordBoundary.startPos + rCache.wordboundary[i];
            sal_Int32 slen = rCache.length - rCache.wordboundary[i];
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
            if (count)
            {
                rCache.wordboundary[i+1] = rCache.wordboundary[i] + count;
                i++;
            }
        }

        if (len) {
            rCache.wordboundary[i+1] = rCache.wordboundary[i] + len;
            i++;
        }
    }
    rCache.wordboundary[i + 1] = rCache.length + 1;

    return rCache;
}

Boundary xdictionary::previousWord(const OUString& rText, sal_Int32 anyPos, sal_Int16 wordType)
{
        // looking for the first non-whitespace character from anyPos
        sal_uInt32 ch = rText.iterateCodePoints(&anyPos, -1);

        while (anyPos > 0 && u_isWhitespace(ch)) ch = rText.iterateCodePoints(&anyPos, -1);

        return getWordBoundary(rText, anyPos, wordType, true);
}

Boundary xdictionary::nextWord(const OUString& rText, sal_Int32 anyPos, sal_Int16 wordType)
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

Boundary xdictionary::getWordBoundary(const OUString& rText, sal_Int32 anyPos, sal_Int16 wordType, sal_Bool bDirection)
{
        const sal_Unicode *text=rText.getStr();
        sal_Int32 len=rText.getLength();
        if (anyPos >= len || anyPos < 0) {
            boundary.startPos = boundary.endPos = anyPos < 0 ? 0 : len;
        } else if (seekSegment(rText, anyPos, boundary)) {          // character in dict
            WordBreakCache& aCache = getCache(text, boundary);
            sal_Int32 i = 0;

            while (aCache.wordboundary[i] <= anyPos - boundary.startPos) i++;

            sal_Int32 startPos = aCache.wordboundary[i - 1];
            // if bDirection is false
            if (!bDirection && startPos > 0 && startPos == (anyPos - boundary.startPos))
            {
                sal_Int32 indexUtf16 = anyPos-1;
                sal_uInt32 ch = rText.iterateCodePoints(&indexUtf16, 1);
                if (u_isWhitespace(ch))
                    i--;
            }
            boundary.endPos = boundary.startPos;
            rText.iterateCodePoints(&boundary.endPos, aCache.wordboundary[i]);
            rText.iterateCodePoints(&boundary.startPos, aCache.wordboundary[i-1]);
        } else {
            boundary.startPos = anyPos;
            if (anyPos < len) rText.iterateCodePoints(&anyPos, 1);
            boundary.endPos = anyPos < len ? anyPos : len;
        }
        if (wordType == WordType::WORD_COUNT) {
            // skip punctuation for word count.
            while (boundary.endPos < len)
            {
                sal_Int32 indexUtf16 = boundary.endPos;
                if (u_ispunct(rText.iterateCodePoints(&indexUtf16, 1)))
                    boundary.endPos = indexUtf16;
                else
                    break;
            }
        }

        return boundary;
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
