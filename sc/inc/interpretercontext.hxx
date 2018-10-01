/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_INTERPRETERCONTEXT_HXX
#define INCLUDED_SC_INC_INTERPRETERCONTEXT_HXX

#include <vector>
#include <formula/token.hxx>
#include "types.hxx"

#define TOKEN_CACHE_SIZE 8

class ScDocument;
class SvNumberFormatter;
struct ScLookupCacheMap;

// SetNumberFormat() is not thread-safe, so calls to it need to be delayed to the main thread.
struct DelayedSetNumberFormat
{
    SCROW mRow; // Used only with formula groups, so column and tab do not need to be stored.
    sal_uInt32 mnNumberFormat;
};

struct ScInterpreterContext
{
    const ScDocument& mrDoc;
    SvNumberFormatter* mpFormatter;
    size_t mnTokenCachePos;
    std::vector<formula::FormulaToken*> maTokens;
    std::vector<DelayedSetNumberFormat> maDelayedSetNumberFormat;
    ScLookupCacheMap* mScLookupCache; // cache for lookups like VLOOKUP and MATCH

    ScInterpreterContext(const ScDocument& rDoc, SvNumberFormatter* pFormatter)
        : mrDoc(rDoc)
        , mpFormatter(pFormatter)
        , mnTokenCachePos(0)
        , maTokens(TOKEN_CACHE_SIZE, nullptr)
        , mInterpretTailZone(nullptr)
        , mScLookupCache(nullptr)
    {
    }

    ~ScInterpreterContext();

    SvNumberFormatter* GetFormatTable() const { return mpFormatter; }
};

#endif // INCLUDED_SC_INC_INTERPRETERCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
