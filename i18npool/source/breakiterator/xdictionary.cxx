/*************************************************************************
 *
 *  $RCSfile: xdictionary.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 14:38:57 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// xdictionary.cpp: implementation of the xdictionary class.
//
//////////////////////////////////////////////////////////////////////


#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include <com/sun/star/i18n/WordType.hpp>
#include <tools/string.hxx>
#include <xdictionary.hxx>
#include <i18nutil/unicode.hxx>
#include <string.h>
#include <breakiteratorImpl.hxx>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

xdictionary::xdictionary(const sal_Char *lang)
{
#ifdef SAL_DLLPREFIX
    OUStringBuffer aBuf( strlen(lang) + 7 + 6 );    // mostly "lib*.so" (with * == dict_zh)
    aBuf.appendAscii( SAL_DLLPREFIX );
#else
    OUStringBuffer aBuf( strlen(lang) + 7 + 4 );    // mostly "*.dll" (with * == dict_zh)
#endif
    aBuf.appendAscii( "dict_" ).appendAscii( lang ).appendAscii( SAL_DLLEXTENSION );
        hModule = osl_loadModule( aBuf.makeStringAndClear().pData, SAL_LOADMODULE_DEFAULT );
        if( hModule ) {
            int (*func)();
            func = (int(*)()) osl_getSymbol( hModule, OUString::createFromAscii("getExistMark").pData );
            existMark = (sal_uInt8*) (*func)();
            func = (int(*)()) osl_getSymbol( hModule, OUString::createFromAscii("getIndex1").pData );
            index1 = (sal_Int16*) (*func)();
            func = (int(*)()) osl_getSymbol( hModule, OUString::createFromAscii("getIndex2").pData );
            index2 = (sal_Int32*) (*func)();
            func = (int(*)()) osl_getSymbol( hModule, OUString::createFromAscii("getLenArray").pData );
            lenArray = (sal_Int32*) (*func)();
            func = (int(*)()) osl_getSymbol( hModule, OUString::createFromAscii("getDataArea").pData );
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
        sal_Bool exist = existMark ? ((existMark[c>>3] & (1<<(c&0x07))) != 0) : sal_False;
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
                (unicode::isWhiteSpace(text[segBoundary.startPos]) || exists(text[segBoundary.startPos]));
            segBoundary.startPos--);
        segBoundary.startPos++;

        for (segBoundary.endPos = pos;
            segBoundary.endPos < len &&
                    (unicode::isWhiteSpace(text[segBoundary.endPos]) || exists(text[segBoundary.endPos]));
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
            while (unicode::isWhiteSpace(text[wordBoundary.startPos + aCache.wordboundary[i] + len]))
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

Boundary SAL_CALL xdictionary::previousWord(const sal_Unicode *text, sal_Int32 anyPos, sal_Int32 len, sal_Int16 wordType)
{
        // looking for the first non-whitespace character from anyPos
        while (unicode::isWhiteSpace(text[anyPos - 1])) anyPos --;
        return getWordBoundary(text, anyPos - 1, len, wordType, true);
}

Boundary SAL_CALL xdictionary::nextWord(const sal_Unicode *text, sal_Int32 anyPos, sal_Int32 len, sal_Int16 wordType)
{
        boundary = getWordBoundary(text, anyPos, len, wordType, true);
        // looknig for the first non-whitespace character from anyPos
        anyPos = boundary.endPos;
        while (unicode::isWhiteSpace(text[anyPos])) anyPos ++;

        return getWordBoundary(text, anyPos, len, wordType, true);
}

Boundary SAL_CALL xdictionary::getWordBoundary(const sal_Unicode *text, sal_Int32 anyPos, sal_Int32 len, sal_Int16 wordType, sal_Bool bDirection)
{
        if (anyPos >= len || anyPos < 0) {
            boundary.startPos = boundary.endPos = anyPos < 0 ? 0 : len;
        } else if (seekSegment(text, anyPos, len, boundary)) {          // character in dict
            WordBreakCache& aCache = getCache(text, boundary);
            sal_Int32 i = 0;

            while (aCache.wordboundary[i] <= (sal_Int32)anyPos - boundary.startPos) i++;

            sal_Int32 startPos = aCache.wordboundary[i - 1];
            // if bDirection is false
            if (!bDirection && startPos > 0 && startPos == (anyPos - boundary.startPos) &&
                                                unicode::isWhiteSpace(text[anyPos - 1]))
                i--;
            boundary.endPos = aCache.wordboundary[i] + boundary.startPos;
            boundary.startPos += aCache.wordboundary[i - 1];
        } else {
            boundary.startPos = anyPos++;
            boundary.endPos = anyPos < len ? anyPos : len;
        }
        if (wordType == WordType::WORD_COUNT) {
            // skip punctuation for word count.
            while (boundary.endPos < len && unicode::isPunctuation(text[boundary.endPos]))
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
