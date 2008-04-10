/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xdictionary.hxx,v $
 * $Revision: 1.7 $
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
#ifndef _XDICTIONARY_H_
#define _XDICTIONARY_H_

#include <sal/types.h>
#include <osl/module.h>

#include <com/sun/star/i18n/Boundary.hpp>

namespace com { namespace sun { namespace star { namespace i18n {

#define CACHE_MAX 32        // max cache structure number
#define DEFAULT_SIZE 256    // for boundary size, to avoid alloc and release memory

// cache structure.
typedef struct _WrodBreakCache {
    sal_Bool SAL_CALL equals(const sal_Unicode *str, Boundary& boundary);   // checking cached string
    sal_Int32 length;       // contents length saved here.
    sal_Unicode *contents;      // seperated segment contents.
    sal_Int32* wordboundary;        // word boundaries in segments.
    sal_Int32 size;         // size of wordboundary
} WordBreakCache;

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
    sal_Bool useCellBoundary;
    sal_Int32* cellBoundary;
    sal_Bool japaneseWordBreak;

public:
    xdictionary(const sal_Char *lang);
    ~xdictionary();
    Boundary SAL_CALL nextWord( const rtl::OUString& rText, sal_Int32 nPos, sal_Int16 wordType);
    Boundary SAL_CALL previousWord( const rtl::OUString& rText, sal_Int32 nPos, sal_Int16 wordType);
    Boundary SAL_CALL getWordBoundary( const rtl::OUString& rText, sal_Int32 nPos, sal_Int16 wordType, sal_Bool bDirection );
    void SAL_CALL setCellBoundary(sal_Int32* cellBondary);
    void SAL_CALL setJapaneseWordBreak();

private:
    WordBreakCache cache[CACHE_MAX];

    sal_Bool SAL_CALL seekSegment(const sal_Unicode *text, sal_Int32 pos, sal_Int32 len, Boundary& boundary);
    WordBreakCache& SAL_CALL getCache(const sal_Unicode *text, Boundary& boundary);
        sal_Bool  SAL_CALL exists(const sal_Unicode u);
    sal_Int32 SAL_CALL getLongestMatch(const sal_Unicode *text, sal_Int32 len);
};

} } } }

#endif

