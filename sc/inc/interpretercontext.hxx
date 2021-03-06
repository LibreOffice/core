/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

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
class ScInterpreter;
enum class SvNumFormatType : sal_Int16;

// SetNumberFormat() is not thread-safe, so calls to it need to be delayed to the main thread.
struct DelayedSetNumberFormat
{
    SCCOL mCol;
    SCROW mRow;
    sal_uInt32 mnNumberFormat;
};

struct NFIndexAndFmtType
{
    sal_uInt32 nIndex;
    SvNumFormatType eType : 16;
    bool bIsValid : 1;

    NFIndexAndFmtType()
        : nIndex(0)
        , eType(static_cast<SvNumFormatType>(0))
        , bIsValid(false)
    {
    }
};

class ScInterpreterContextPool;

struct ScInterpreterContext
{
    const ScDocument* mpDoc;
    size_t mnTokenCachePos;
    std::vector<formula::FormulaToken*> maTokens;
    std::vector<DelayedSetNumberFormat> maDelayedSetNumberFormat;
    std::unique_ptr<ScLookupCacheMap> mxScLookupCache; // cache for lookups like VLOOKUP and MATCH
    // Allocation cache for "aConditions" array in ScInterpreter::IterateParameterIfs()
    // This is populated/used only when formula-group threading is enabled.
    std::vector<sal_uInt32> maConditions;
    ScInterpreter* pInterpreter;

    ScInterpreterContext(const ScDocument& rDoc, SvNumberFormatter* pFormatter);

    ScInterpreterContext() = delete;

    ~ScInterpreterContext();

    SvNumberFormatter* GetFormatTable() const
    {
        if (mpFormatter == nullptr)
            const_cast<ScInterpreterContext*>(this)->initFormatTable();
        return mpFormatter;
    }

    SvNumFormatType GetNumberFormatType(sal_uInt32 nFIndex) const;

private:
    friend class ScInterpreterContextPool;
    void ResetTokens();
    void SetDocAndFormatter(const ScDocument& rDoc, SvNumberFormatter* pFormatter);
    void Cleanup();
    void ClearLookupCache();
    void initFormatTable();
    SvNumberFormatter* mpFormatter;
    mutable NFIndexAndFmtType maNFTypeCache;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
