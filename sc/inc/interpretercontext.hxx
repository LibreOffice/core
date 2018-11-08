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
#include <memory>
#include "types.hxx"

namespace formula
{
class FormulaToken;
}

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
    typedef std::vector<std::unique_ptr<ScInterpreterContext>> ContextPoolType;

    const ScDocument* mpDoc;
    SvNumberFormatter* mpFormatter;
    size_t mnTokenCachePos;
    std::vector<formula::FormulaToken*> maTokens;
    std::vector<DelayedSetNumberFormat> maDelayedSetNumberFormat;
    ScLookupCacheMap* mScLookupCache; // cache for lookups like VLOOKUP and MATCH
    sal_Int16 mnThreadIndex;
    // Allocation cache for "aConditions" array in ScInterpreter::IterateParameterIfs()
    // This is populated/used only when formula-group threading is enabled.
    std::vector<sal_uInt32> maConditions;

    ScInterpreterContext(const ScDocument& rDoc, SvNumberFormatter* pFormatter,
                         sal_Int16 nThreadIdx = -1)
        : mpDoc(&rDoc)
        , mpFormatter(pFormatter)
        , mnTokenCachePos(0)
        , maTokens(TOKEN_CACHE_SIZE, nullptr)
        , mScLookupCache(nullptr)
        , mnThreadIndex(nThreadIdx)
    {
    }

    ScInterpreterContext() = delete;

    ~ScInterpreterContext();

    SvNumberFormatter* GetFormatTable() const { return mpFormatter; }

    static ContextPoolType& GetContexts(size_t nSize, const ScDocument& rDoc,
                                        SvNumberFormatter* pFormatter);

private:
    void ResetTokens();
    void Reset(const ScDocument& rDoc, SvNumberFormatter* pFormatter);
    static ContextPoolType aContextPool;
};

#endif // INCLUDED_SC_INC_INTERPRETERCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
