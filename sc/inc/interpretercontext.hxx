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

class ScInterpreterContextPool;

struct ScInterpreterContext
{
    const ScDocument* mpDoc;
    size_t mnTokenCachePos;
    std::vector<formula::FormulaToken*> maTokens;
    std::vector<DelayedSetNumberFormat> maDelayedSetNumberFormat;
    ScLookupCacheMap* mScLookupCache; // cache for lookups like VLOOKUP and MATCH
    // Allocation cache for "aConditions" array in ScInterpreter::IterateParameterIfs()
    // This is populated/used only when formula-group threading is enabled.
    std::vector<sal_uInt32> maConditions;

    ScInterpreterContext(const ScDocument& rDoc, SvNumberFormatter* pFormatter)
        : mpDoc(&rDoc)
        , mnTokenCachePos(0)
        , maTokens(TOKEN_CACHE_SIZE, nullptr)
        , mScLookupCache(nullptr)
        , mpFormatter(pFormatter)
    {
    }

    ScInterpreterContext() = delete;

    ~ScInterpreterContext();

    SvNumberFormatter* GetFormatTable() const
    {
        if (mpFormatter == nullptr)
            const_cast<ScInterpreterContext*>(this)->initFormatTable();
        return mpFormatter;
    }

private:
    friend class ScInterpreterContextPool;
    void ResetTokens();
    void SetDocAndFormatter(const ScDocument& rDoc, SvNumberFormatter* pFormatter);
    void Cleanup();
    void ClearLookupCache();
    void initFormatTable();
    SvNumberFormatter* mpFormatter;
};

class ScThreadedInterpreterContextGetterGuard;
class ScInterpreterContextGetterGuard;

class ScInterpreterContextPool
{
    friend class ScThreadedInterpreterContextGetterGuard;
    friend class ScInterpreterContextGetterGuard;

    std::vector<std::unique_ptr<ScInterpreterContext>> maPool;
    size_t mnNextFree;
    bool mbThreaded;

    ScInterpreterContextPool(bool bThreaded)
        : mnNextFree(0)
        , mbThreaded(bThreaded)
    {
    }

    ~ScInterpreterContextPool() {}

    static ScInterpreterContextPool aThreadedInterpreterPool;
    static ScInterpreterContextPool aNonThreadedInterpreterPool;

    // API for threaded case

    // Ensures nNumThreads elements in pool.
    void Init(size_t nNumThreads, const ScDocument& rDoc, SvNumberFormatter* pFormatter);

    // Returns ScInterpreterContext* for thread index nThreadIdx
    ScInterpreterContext* GetInterpreterContextForThreadIdx(size_t nThreadIdx) const;

    // API for non-threaded

    // Ensures there is one unused element in the pool.
    void Init(const ScDocument& rDoc, SvNumberFormatter* pFormatter);

    // Returns ScInterpreterContext* for non-threaded use.
    ScInterpreterContext* GetInterpreterContext() const;

    // Common API for threaded/non-threaded

    // Cleans up the contexts prepared by call to immediately previous Init() and
    // marks them all as unused.
    void ReturnToPool();

public:
    // Only to be used to clear lookup cache in all pool elements
    static void ClearLookupCaches();
};

class ScThreadedInterpreterContextGetterGuard
{
    ScInterpreterContextPool& rPool;

public:
    ScThreadedInterpreterContextGetterGuard(size_t nNumThreads, const ScDocument& rDoc,
                                            SvNumberFormatter* pFormatter);
    ~ScThreadedInterpreterContextGetterGuard();

    // Returns ScInterpreterContext* for thread index nThreadIdx
    ScInterpreterContext* GetInterpreterContextForThreadIdx(size_t nThreadIdx) const;
};

class ScInterpreterContextGetterGuard
{
    ScInterpreterContextPool& rPool;
#if !defined NDEBUG
    size_t nContextIdx;
#endif

public:
    ScInterpreterContextGetterGuard(const ScDocument& rDoc, SvNumberFormatter* pFormatter);
    ~ScInterpreterContextGetterGuard();

    // Returns ScInterpreterContext* for non-threaded use.
    ScInterpreterContext* GetInterpreterContext() const;
};

#endif // INCLUDED_SC_INC_INTERPRETERCONTEXT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
