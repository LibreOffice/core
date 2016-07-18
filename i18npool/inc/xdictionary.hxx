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
#ifndef INCLUDED_I18NPOOL_INC_XDICTIONARY_HXX
#define INCLUDED_I18NPOOL_INC_XDICTIONARY_HXX

#include <sal/types.h>
#include <osl/module.h>

#include <com/sun/star/i18n/Boundary.hpp>

namespace com { namespace sun { namespace star { namespace i18n {

#define CACHE_MAX 32        // max cache structure number
#define DEFAULT_SIZE 256    // for boundary size, to avoid alloc and release memory

// cache structure.
struct WordBreakCache {
    sal_Int32 length;       // contents length saved here.
    sal_Unicode *contents;      // separated segment contents.
    sal_Int32* wordboundary;        // word boundaries in segments.
    sal_Int32 size;         // size of wordboundary

    WordBreakCache();
    bool equals(const sal_Unicode *str, Boundary& boundary);    // checking cached string
};

struct xdictionarydata
{
    const sal_uInt8  * existMark;
    const sal_Int16  * index1;
    const sal_Int32  * index2;
    const sal_Int32  * lenArray;
    const sal_Unicode* dataArea;
    xdictionarydata() :
        existMark( nullptr ),
        index1( nullptr ),
        index2( nullptr ),
        lenArray( nullptr ),
        dataArea( nullptr )
    {
    }
};

class xdictionary
{
private:
    xdictionarydata data;
    void initDictionaryData(const sal_Char *lang);

    Boundary boundary;
    bool japaneseWordBreak;

public:
    xdictionary(const sal_Char *lang);
    ~xdictionary();
    Boundary nextWord( const OUString& rText, sal_Int32 nPos, sal_Int16 wordType);
    Boundary previousWord( const OUString& rText, sal_Int32 nPos, sal_Int16 wordType);
    Boundary const & getWordBoundary( const OUString& rText, sal_Int32 nPos, sal_Int16 wordType, bool bDirection );
    void setJapaneseWordBreak();

private:
    WordBreakCache cache[CACHE_MAX];
    OUString segmentCachedString;
    Boundary segmentCachedBoundary;

    bool        seekSegment(const OUString& rText, sal_Int32 pos, Boundary& boundary);
    WordBreakCache& getCache(const sal_Unicode *text, Boundary& boundary);
    bool        exists(const sal_uInt32 u);
    sal_Int32       getLongestMatch(const sal_Unicode *text, sal_Int32 len);
};

} } } }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
