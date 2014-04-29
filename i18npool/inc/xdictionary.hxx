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


#ifndef _XDICTIONARY_H_
#define _XDICTIONARY_H_

#include <sal/types.h>
#include <osl/module.h>

#include <com/sun/star/i18n/Boundary.hpp>

namespace com { namespace sun { namespace star { namespace i18n {

// Whether to use cell boundary code, currently unused but prepared.
#define USE_CELL_BOUNDARY_CODE 0

#define CACHE_MAX 32        // max cache structure number
#define DEFAULT_SIZE 256    // for boundary size, to avoid alloc and release memory

// cache structure.
struct WordBreakCache {
    sal_Int32 length;       // contents length saved here.
    sal_Unicode *contents;      // separated segment contents.
    sal_Int32* wordboundary;        // word boundaries in segments.
    sal_Int32 size;         // size of wordboundary

    WordBreakCache();
    sal_Bool equals(const sal_Unicode *str, Boundary& boundary);    // checking cached string
};

class xdictionary
{
private:
    const sal_uInt8  * existMark;
    const sal_Int16  * index1;
    const sal_Int32 * index2;
    const sal_Int32 * lenArray;
    const sal_Unicode* dataArea;
    oslModule hModule;
    Boundary boundary;
    sal_Bool japaneseWordBreak;

#if USE_CELL_BOUNDARY_CODE
    // For CTL breakiterator, where the word boundary should not be inside cell.
    sal_Bool useCellBoundary;
    sal_Int32* cellBoundary;
#endif

public:
    xdictionary(const sal_Char *lang);
    ~xdictionary();
    Boundary nextWord( const rtl::OUString& rText, sal_Int32 nPos, sal_Int16 wordType);
    Boundary previousWord( const rtl::OUString& rText, sal_Int32 nPos, sal_Int16 wordType);
    Boundary getWordBoundary( const rtl::OUString& rText, sal_Int32 nPos, sal_Int16 wordType, sal_Bool bDirection );
    void setJapaneseWordBreak();

#if USE_CELL_BOUNDARY_CODE
    void setCellBoundary(sal_Int32* cellArray);
#endif

private:
    WordBreakCache cache[CACHE_MAX];

    sal_Bool        seekSegment(const rtl::OUString& rText, sal_Int32 pos, Boundary& boundary);
    WordBreakCache& getCache(const sal_Unicode *text, Boundary& boundary);
    sal_Bool        exists(const sal_uInt32 u);
    sal_Int32       getLongestMatch(const sal_Unicode *text, sal_Int32 len);
};

} } } }

#endif

