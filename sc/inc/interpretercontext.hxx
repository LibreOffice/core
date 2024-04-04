/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <array>
#include <memory>
#include <random>
#include <vector>
#include <i18nlangtag/mslangid.hxx>
#include <svl/numformat.hxx>
#include "types.hxx"

namespace formula
{
class FormulaToken;
}

#define TOKEN_CACHE_SIZE 8

class Color;
class ScDocument;
struct ScLookupCacheMap;
class ScInterpreter;

// SetNumberFormat() is not thread-safe, so calls to it need to be delayed to the main thread.
struct DelayedSetNumberFormat
{
    SCCOL mCol;
    SCROW mRow;
    sal_uInt32 mnNumberFormat;
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
    std::vector<sal_uInt8> maConditions;
    std::mt19937 aRNG;
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

    SvNumFormatType NFGetType(sal_uInt32 nFIndex) const;
    const SvNumberformat* NFGetFormatEntry(sal_uInt32 nKey) const;
    sal_uInt32 NFGetFormatIndex(NfIndexTableOffset, LanguageType eLnge = LANGUAGE_DONTKNOW) const;
    bool NFIsTextFormat(sal_uInt32 nFIndex) const;
    sal_uInt32 NFGetTimeFormat(double fNumber, LanguageType eLnge, bool bForceDuration) const;
    const Date& NFGetNullDate() const;
    OUString NFGetFormatDecimalSep(sal_uInt32 nFormat) const;
    sal_uInt16 NFGetFormatPrecision(sal_uInt32 nFormat) const;

    sal_uInt32 NFGetFormatForLanguageIfBuiltIn(sal_uInt32 nFormat, LanguageType eLnge) const;

    bool NFIsNumberFormat(const OUString& sString, sal_uInt32& F_Index, double& fOutNumber,
                          SvNumInputOptions eInputOptions = SvNumInputOptions::NONE);

    void NFGetInputLineString(const double& fOutNumber, sal_uInt32 nFIndex, OUString& rOutString,
                              bool bFiltering = false, bool bForceSystemLocale = false) const;

    void NFGetOutputString(const double& fOutNumber, sal_uInt32 nFIndex, OUString& sOutString,
                           const Color** ppColor, bool bUseStarFormat = false) const;

    void NFGetOutputString(const OUString& sString, sal_uInt32 nFIndex, OUString& sOutString,
                           const Color** ppColor, bool bUseStarFormat = false) const;

    sal_uInt32 NFGetStandardFormat(SvNumFormatType eType, LanguageType eLnge = LANGUAGE_DONTKNOW);
    sal_uInt32 NFGetStandardFormat(sal_uInt32 nFIndex, SvNumFormatType eType, LanguageType eLnge);

    bool NFGetPreviewString(const OUString& sFormatString, double fPreviewNumber,
                            OUString& sOutString, const Color** ppColor, LanguageType eLnge);
    bool NFGetPreviewString(const OUString& sFormatString, const OUString& sPreviewString,
                            OUString& sOutString, const Color** ppColor,
                            LanguageType eLnge = LANGUAGE_DONTKNOW);
    bool NFGetPreviewStringGuess(const OUString& sFormatString, double fPreviewNumber,
                                 OUString& sOutString, const Color** ppColor,
                                 LanguageType eLnge = LANGUAGE_DONTKNOW);

    sal_uInt32 NFGetStandardIndex(LanguageType eLnge = LANGUAGE_DONTKNOW) const;

    OUString NFGenerateFormat(sal_uInt32 nIndex, LanguageType eLnge = LANGUAGE_DONTKNOW,
                              bool bThousand = false, bool IsRed = false, sal_uInt16 nPrecision = 0,
                              sal_uInt16 nLeadingCnt = 1);

    sal_uInt16 NFExpandTwoDigitYear(sal_uInt16 nYear) const;

    OUString NFGetCalcCellReturn(sal_uInt32 nFormat) const;

    void MergeDefaultFormatKeys(SvNumberFormatter& rFormatter) const;

private:
    friend class ScInterpreterContextPool;
    void ResetTokens();
    void SetDocAndFormatter(const ScDocument& rDoc, SvNumberFormatter* pFormatter);
    void Cleanup();
    void ClearLookupCache(const ScDocument* pDoc);
    void initFormatTable();
    void prepFormatterForRoMode(SvNumberFormatter* pFormatter);

    // During threaded calculation, where we don't need to add to the number
    // format data, we can access the numbering data with a RO unlocked view of
    // the NumberFormat's data and a throw-away object for currently used language
    // This is essentially an exploded view of mpFormatter
    std::unique_ptr<SvNFLanguageData> mxLanguageData;
    // FormatData can be driven read-only, but may want to cache some data,
    // in RO Mode we can cache per thread to mxAuxFormatKeyMap, and
    // discard or merge after threaded calculation is over
    std::unique_ptr<SvNFFormatData::DefaultFormatKeysMap> mxAuxFormatKeyMap;

    const SvNFFormatData* mpFormatData;
    SvNFEngine::Accessor maROPolicy;

    // Some temp caches of the 4 most recent results from NumberFormatting
    // lookups.
    struct NFBuiltIn
    {
        sal_uInt64 nKey;
        sal_uInt32 nFormat;
        NFBuiltIn()
            : nKey(SAL_MAX_UINT64)
            , nFormat(SAL_MAX_UINT32)
        {
        }
    };
    // from format+lang to builtin format
    mutable std::array<NFBuiltIn, 4> maNFBuiltInCache;
    struct NFType
    {
        sal_uInt32 nKey;
        SvNumFormatType eType;
        NFType()
            : nKey(SAL_MAX_UINT32)
            , eType(SvNumFormatType::ALL)
        {
        }
    };
    // from format index to type
    mutable std::array<NFType, 4> maNFTypeCache;

    // Formatter used when non-nthreaded calculation
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
    static void ClearLookupCaches(const ScDocument* pDoc);
    // Called from ScModule dtor, drop all resources
    static void ModuleExiting();
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
