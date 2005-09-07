/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xdictionary.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:59:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    Boundary SAL_CALL nextWord( const sal_Unicode *text, sal_Int32 nPos, sal_Int32 len, sal_Int16 wordType);
    Boundary SAL_CALL previousWord( const sal_Unicode *text, sal_Int32 nPos, sal_Int32 len, sal_Int16 wordType);
    Boundary SAL_CALL getWordBoundary( const sal_Unicode *text, sal_Int32 nPos, sal_Int32 len, sal_Int16 wordType,
        sal_Bool bDirection );
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

