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

#include <config_folders.h>

#include <osl/file.h>
#include <osl/mutex.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <com/sun/star/i18n/WordType.hpp>
#include <xdictionary.hxx>
#include <unicode/uchar.h>
#include <string.h>
#include <breakiteratorImpl.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

#ifdef DICT_JA_ZH_IN_DATAFILE

#elif !defined DISABLE_DYNLOADING

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
    boundary(),
    japaneseWordBreak( false )
{

#ifdef DICT_JA_ZH_IN_DATAFILE

    if( strcmp( lang, "ja" ) == 0 || strcmp( lang, "zh" ) == 0 )
    {
        OUString sUrl( "$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/dict_" );
        rtl::Bootstrap::expandMacros(sUrl);

        if( strcmp( lang, "ja" ) == 0 )
            sUrl += "ja.data";
        else if( strcmp( lang, "zh" ) == 0 )
            sUrl += "zh.data";

        oslFileHandle aFileHandle;
        sal_uInt64 nFileSize;
        char *pMapping;
        if( osl_openFile( sUrl.pData, &aFileHandle, osl_File_OpenFlag_Read ) == osl_File_E_None &&
            osl_getFileSize( aFileHandle, &nFileSize) == osl_File_E_None &&
            osl_mapFile( aFileHandle, (void **) &pMapping, nFileSize, 0, osl_File_MapFlag_RandomAccess ) == osl_File_E_None )
        {
            // We have the offsets to the parts of the file at its end, see gendict.cxx
            sal_Int64 *pEOF = (sal_Int64*)(pMapping + nFileSize);

            data.existMark = (sal_uInt8*) (pMapping + pEOF[-1]);
            data.index2 = (sal_Int32*) (pMapping + pEOF[-2]);
            data.index1 = (sal_Int16*) (pMapping + pEOF[-3]);
            data.lenArray = (sal_Int32*) (pMapping + pEOF[-4]);
            data.dataArea = (sal_Unicode*) (pMapping + pEOF[-5]);
        }
    }

#elif !defined DISABLE_DYNLOADING

    initDictionaryData( lang );

#else

    if( strcmp( lang, "ja" ) == 0 ) {
        data.existMark = getExistMark_ja();
        data.index1 = getIndex1_ja();
        data.index2 = getIndex2_ja();
        data.lenArray = getLenArray_ja();
        data.dataArea = getDataArea_ja();
    }
    else if( strcmp( lang, "zh" ) == 0 ) {
        data.existMark = getExistMark_zh();
        data.index1 = getIndex1_zh();
        data.index2 = getIndex2_zh();
        data.lenArray = getLenArray_zh();
        data.dataArea = getDataArea_zh();
    }

#endif

    for (WordBreakCache & i : cache)
        i.size = 0;

    japaneseWordBreak = false;
}

xdictionary::~xdictionary()
{
    for (WordBreakCache & i : cache) {
        if (i.size > 0) {
            delete [] i.contents;
            delete [] i.wordboundary;
        }
    }
}

namespace {
    struct datacache {
        oslModule       mhModule;
        OString         maLang;
        xdictionarydata maData;
    };
}

#if !defined(DICT_JA_ZH_IN_DATAFILE) && !defined(DISABLE_DYNLOADING)

void xdictionary::initDictionaryData(const sal_Char *pLang)
{
    // Global cache, never released for performance
    static std::vector< datacache > aLoadedCache;

    osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() );
    for(datacache & i : aLoadedCache)
    {
        if( !strcmp( pLang, i.maLang.getStr() ) )
        {
            data = i.maData;
            return;
        }
    }

    // otherwise add to the cache, positive or negative.
    datacache aEntry;
    aEntry.maLang = OString( pLang, strlen( pLang ) );

#ifdef SAL_DLLPREFIX
    OUStringBuffer aBuf( sal::static_int_cast<int>(strlen(pLang) + 7 + 6) );    // mostly "lib*.so" (with * == dict_zh)
    aBuf.append( SAL_DLLPREFIX );
#else
    OUStringBuffer aBuf( sal::static_int_cast<int>(strlen(pLang) + 7 + 4) );    // mostly "*.dll" (with * == dict_zh)
#endif
    aBuf.append( "dict_" ).appendAscii( pLang ).append( SAL_DLLEXTENSION );
    aEntry.mhModule = osl_loadModuleRelative( &thisModule, aBuf.makeStringAndClear().pData, SAL_LOADMODULE_DEFAULT );
    if( aEntry.mhModule ) {
        oslGenericFunction func;
        func = osl_getAsciiFunctionSymbol( aEntry.mhModule, "getExistMark" );
        aEntry.maData.existMark = reinterpret_cast<sal_uInt8 const * (*)()>(func)();
        func = osl_getAsciiFunctionSymbol( aEntry.mhModule, "getIndex1" );
        aEntry.maData.index1 = reinterpret_cast<sal_Int16 const * (*)()>(func)();
        func = osl_getAsciiFunctionSymbol( aEntry.mhModule, "getIndex2" );
        aEntry.maData.index2 = reinterpret_cast<sal_Int32 const * (*)()>(func)();
        func = osl_getAsciiFunctionSymbol( aEntry.mhModule, "getLenArray" );
        aEntry.maData.lenArray = reinterpret_cast<sal_Int32 const * (*)()>(func)();
        func = osl_getAsciiFunctionSymbol( aEntry.mhModule, "getDataArea" );
        aEntry.maData.dataArea = reinterpret_cast<sal_Unicode const * (*)()>(func)();
    }

    data = aEntry.maData;
    aLoadedCache.push_back( aEntry );
}

#endif

void xdictionary::setJapaneseWordBreak()
{
    japaneseWordBreak = true;
}

bool xdictionary::exists(const sal_uInt32 c)
{
    // 0x1FFF is the hardcoded limit in gendict for data.existMarks
    bool exist = data.existMark && (c>>3) < 0x1FFF && (data.existMark[c>>3] & (1<<(c&0x07))) != 0;
    if (!exist && japaneseWordBreak)
        return BreakIteratorImpl::getScriptClass(c) == ScriptType::ASIAN;
    else
        return exist;
}

sal_Int32 xdictionary::getLongestMatch(const sal_Unicode* str, sal_Int32 sLen)
{
    if ( !data.index1 ) return 0;

    sal_Int16 idx = data.index1[str[0] >> 8];

    if (idx == 0xFF) return 0;

    idx = (idx<<8) | (str[0]&0xff);

    sal_uInt32 begin = data.index2[idx], end = data.index2[idx+1];

    if (begin == 0) return 0;

    str++; sLen--; // first character is not stored in the dictionary
    for (sal_uInt32 i = end; i > begin; i--) {
        sal_Int32 len = data.lenArray[i] - data.lenArray[i - 1];
        if (sLen >= len) {
            const sal_Unicode *dstr = data.dataArea + data.lenArray[i-1];
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
    contents( nullptr ),
    wordboundary( nullptr ),
    size( 0 )
{
}

/*
 * Compare two unicode string,
 */

bool WordBreakCache::equals(const sal_Unicode* str, Boundary& boundary)
{
    // Different length, different string.
    if (length != boundary.endPos - boundary.startPos) return false;

    for (sal_Int32 i = 0; i < length; i++)
        if (contents[i] != str[i + boundary.startPos]) return false;

    return true;
}


/*
 * Retrieve the segment containing the character at pos.
 * @param pos : Position of the given character.
 * @return true if CJK.
 */
bool xdictionary::seekSegment(const OUString &rText, sal_Int32 pos,
    Boundary& segBoundary)
{
    sal_Int32 indexUtf16;

    if (segmentCachedString.pData != rText.pData) {
        // Cache the passed text so we can avoid regenerating the segment if it's the same
        // (pData is refcounted and assigning the OUString references it, which ensures that
        // the object is the same if we get the same pointer back later)
        segmentCachedString = rText;
    } else {
        // If pos is within the cached boundary, use that boundary
        if (pos >= segmentCachedBoundary.startPos && pos <= segmentCachedBoundary.endPos) {
            segBoundary.startPos = segmentCachedBoundary.startPos;
            segBoundary.endPos = segmentCachedBoundary.endPos;
            indexUtf16 = segmentCachedBoundary.startPos;
            rText.iterateCodePoints(&indexUtf16);
            return segmentCachedBoundary.endPos > indexUtf16;
        }
    }

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
        sal_uInt32 ch = rText.iterateCodePoints(&indexUtf16);
        if (u_isWhitespace(ch) || exists(ch))
            segBoundary.endPos = indexUtf16;
        else
            break;
    }

    // Cache the calculated boundary
    segmentCachedBoundary.startPos = segBoundary.startPos;
    segmentCachedBoundary.endPos = segBoundary.endPos;

    indexUtf16 = segBoundary.startPos;
    rText.iterateCodePoints(&indexUtf16);
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
        // look the continuous white space as one word and cache it
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
        sal_uInt32 ch = 0;
        if (anyPos > 0)
            rText.iterateCodePoints(&anyPos, -1);

        while (anyPos > 0 && u_isWhitespace(ch)) ch = rText.iterateCodePoints(&anyPos, -1);

        return getWordBoundary(rText, anyPos, wordType, true);
}

Boundary xdictionary::nextWord(const OUString& rText, sal_Int32 anyPos, sal_Int16 wordType)
{
        boundary = getWordBoundary(rText, anyPos, wordType, true);
        anyPos = boundary.endPos;
        const sal_Int32 nLen = rText.getLength();
        if (anyPos < nLen) {
            // looking for the first non-whitespace character from anyPos
            sal_uInt32 ch = rText.iterateCodePoints(&anyPos);
            while (u_isWhitespace(ch) && (anyPos < nLen)) ch=rText.iterateCodePoints(&anyPos);
            if (anyPos > 0)
                rText.iterateCodePoints(&anyPos, -1);
        }

        return getWordBoundary(rText, anyPos, wordType, true);
}

Boundary const & xdictionary::getWordBoundary(const OUString& rText, sal_Int32 anyPos, sal_Int16 wordType, bool bDirection)
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
                sal_uInt32 ch = rText.iterateCodePoints(&indexUtf16);
                if (u_isWhitespace(ch))
                    i--;
            }

            boundary.endPos = boundary.startPos;
            boundary.endPos += aCache.wordboundary[i];
            boundary.startPos += aCache.wordboundary[i-1];

        } else {
            boundary.startPos = anyPos;
            if (anyPos < len) rText.iterateCodePoints(&anyPos);
            boundary.endPos = anyPos < len ? anyPos : len;
        }
        if (wordType == WordType::WORD_COUNT) {
            // skip punctuation for word count.
            while (boundary.endPos < len)
            {
                sal_Int32 indexUtf16 = boundary.endPos;
                if (u_ispunct(rText.iterateCodePoints(&indexUtf16)))
                    boundary.endPos = indexUtf16;
                else
                    break;
            }
        }

        return boundary;
}

} } } }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
