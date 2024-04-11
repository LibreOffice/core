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

#include <interpretercontext.hxx>
#include <svl/numformat.hxx>
#include <svl/zforlist.hxx>

#include <document.hxx>
#include <comphelper/random.hxx>
#include <formula/token.hxx>
#include <lookupcache.hxx>
#include <rangecache.hxx>
#include <algorithm>

ScInterpreterContextPool ScInterpreterContextPool::aThreadedInterpreterPool(true);
ScInterpreterContextPool ScInterpreterContextPool::aNonThreadedInterpreterPool(false);

ScInterpreterContext::ScInterpreterContext(const ScDocument& rDoc, SvNumberFormatter* pFormatter)
    : mpDoc(&rDoc)
    , mnTokenCachePos(0)
    , maTokens(TOKEN_CACHE_SIZE, nullptr)
    // create a per-interpreter Random Number Generator, seeded from the global rng, so we don't have
    // to lock a mutex to generate a random number
    , aRNG(comphelper::rng::uniform_uint_distribution(0, std::numeric_limits<sal_uInt32>::max()))
    , pInterpreter(nullptr)
    , mpFormatter(pFormatter)
{
    if (!pFormatter)
    {
        mpFormatData = nullptr;
        mpNatNum = nullptr;
    }
    else
        prepFormatterForRoMode(pFormatter);
}

ScInterpreterContext::~ScInterpreterContext() { ResetTokens(); }

void ScInterpreterContext::ResetTokens()
{
    for (auto p : maTokens)
        if (p)
            p->DecRef();

    mnTokenCachePos = 0;
    std::fill(maTokens.begin(), maTokens.end(), nullptr);
}

void ScInterpreterContext::SetDocAndFormatter(const ScDocument& rDoc, SvNumberFormatter* pFormatter)
{
    if (mpDoc != &rDoc)
    {
        mxScLookupCache.reset();
        mpDoc = &rDoc;
    }
    if (mpFormatter != pFormatter)
    {
        mpFormatter = pFormatter;

        // formatter has changed
        prepFormatterForRoMode(pFormatter);

        // drop cache
        std::fill(maNFBuiltInCache.begin(), maNFBuiltInCache.end(), NFBuiltIn());
        std::fill(maNFTypeCache.begin(), maNFTypeCache.end(), NFType());
    }
}

void ScInterpreterContext::prepFormatterForRoMode(SvNumberFormatter* pFormatter)
{
    pFormatter->PrepForRoMode();
    mpFormatData = &pFormatter->GetROFormatData();
    mpNatNum = &pFormatter->GetNatNum();
    mxLanguageData.reset(new SvNFLanguageData(pFormatter->GetROLanguageData()));
    mxAuxFormatKeyMap.reset(new SvNFFormatData::DefaultFormatKeysMap);
    maROPolicy = SvNFEngine::GetROPolicy(*mpFormatData, *mxAuxFormatKeyMap);
}

void ScInterpreterContext::initFormatTable()
{
    mpFormatter = mpDoc->GetFormatTable(); // will assert if not main thread
    prepFormatterForRoMode(mpFormatter);
}

void ScInterpreterContext::MergeDefaultFormatKeys(SvNumberFormatter& rFormatter) const
{
    rFormatter.MergeDefaultFormatKeys(*mxAuxFormatKeyMap);
}

void ScInterpreterContext::Cleanup()
{
    // Do not disturb mxScLookupCache.
    maConditions.clear();
    maDelayedSetNumberFormat.clear();
    ResetTokens();
}

void ScInterpreterContext::ClearLookupCache(const ScDocument* pDoc)
{
    if (pDoc == mpDoc)
    {
        mxScLookupCache.reset();
        mxLanguageData.reset();
        mxAuxFormatKeyMap.reset();
        mpFormatter = nullptr;
        mpFormatData = nullptr;
        mpNatNum = nullptr;
    }
}

SvNumFormatType ScInterpreterContext::NFGetType(sal_uInt32 nFIndex) const
{
    if (!mpDoc->IsThreadedGroupCalcInProgress())
        return GetFormatTable()->GetType(nFIndex);

    auto aFind = std::find_if(maNFTypeCache.begin(), maNFTypeCache.end(),
                              [nFIndex](const NFType& e) { return e.nKey == nFIndex; });
    if (aFind != maNFTypeCache.end())
        return aFind->eType;

    SvNumFormatType eType = mpFormatData->GetType(nFIndex);

    std::move_backward(maNFTypeCache.begin(),
                       std::next(maNFTypeCache.begin(), maNFTypeCache.size() - 1),
                       maNFTypeCache.end());
    maNFTypeCache[0].nKey = nFIndex;
    maNFTypeCache[0].eType = eType;

    return eType;
}

const SvNumberformat* ScInterpreterContext::NFGetFormatEntry(sal_uInt32 nKey) const
{
    if (!mpDoc->IsThreadedGroupCalcInProgress())
        return GetFormatTable()->GetEntry(nKey);
    return mpFormatData->GetFormatEntry(nKey);
}

bool ScInterpreterContext::NFIsTextFormat(sal_uInt32 nFIndex) const
{
    if (!mpDoc->IsThreadedGroupCalcInProgress())
        return GetFormatTable()->IsTextFormat(nFIndex);
    return mpFormatData->IsTextFormat(nFIndex);
}

const Date& ScInterpreterContext::NFGetNullDate() const
{
    if (!mpDoc->IsThreadedGroupCalcInProgress())
        return GetFormatTable()->GetNullDate();
    return mxLanguageData->GetNullDate();
}

sal_uInt32 ScInterpreterContext::NFGetTimeFormat(double fNumber, LanguageType eLnge,
                                                 bool bForceDuration) const
{
    if (!mpDoc->IsThreadedGroupCalcInProgress())
        return GetFormatTable()->GetTimeFormat(fNumber, eLnge, bForceDuration);
    return SvNFEngine::GetTimeFormat(*mxLanguageData, *mpFormatData, *mpNatNum, maROPolicy, fNumber,
                                     eLnge, bForceDuration);
}

sal_uInt32 ScInterpreterContext::NFGetFormatIndex(NfIndexTableOffset nTabOff,
                                                  LanguageType eLnge) const
{
    if (!mpDoc->IsThreadedGroupCalcInProgress())
        return GetFormatTable()->GetFormatIndex(nTabOff, eLnge);
    return SvNFEngine::GetFormatIndex(*mxLanguageData, maROPolicy, *mpNatNum, nTabOff, eLnge);
}
OUString ScInterpreterContext::NFGetFormatDecimalSep(sal_uInt32 nFormat) const
{
    if (!mpDoc->IsThreadedGroupCalcInProgress())
        return GetFormatTable()->GetFormatDecimalSep(nFormat);
    return SvNFEngine::GetFormatDecimalSep(*mxLanguageData, *mpFormatData, nFormat);
}

sal_uInt16 ScInterpreterContext::NFGetFormatPrecision(sal_uInt32 nFormat) const
{
    if (!mpDoc->IsThreadedGroupCalcInProgress())
        return GetFormatTable()->GetFormatPrecision(nFormat);
    return SvNFEngine::GetFormatPrecision(*mxLanguageData, *mpFormatData, nFormat);
}

sal_uInt32 ScInterpreterContext::NFGetFormatForLanguageIfBuiltIn(sal_uInt32 nFormat,
                                                                 LanguageType eLnge) const
{
    if (!mpDoc->IsThreadedGroupCalcInProgress())
        return GetFormatTable()->GetFormatForLanguageIfBuiltIn(nFormat, eLnge);

    sal_uInt64 nKey = (static_cast<sal_uInt64>(nFormat) << 32) | eLnge.get();

    auto aFind = std::find_if(maNFBuiltInCache.begin(), maNFBuiltInCache.end(),
                              [nKey](const NFBuiltIn& e) { return e.nKey == nKey; });
    if (aFind != maNFBuiltInCache.end())
        return aFind->nFormat;

    nFormat = SvNFEngine::GetFormatForLanguageIfBuiltIn(*mxLanguageData, *mpNatNum, maROPolicy,
                                                        nFormat, eLnge);

    std::move_backward(maNFBuiltInCache.begin(),
                       std::next(maNFBuiltInCache.begin(), maNFBuiltInCache.size() - 1),
                       maNFBuiltInCache.end());
    maNFBuiltInCache[0].nKey = nKey;
    maNFBuiltInCache[0].nFormat = nFormat;

    return nFormat;
}

sal_uInt32 ScInterpreterContext::NFGetStandardFormat(SvNumFormatType eType, LanguageType eLnge)
{
    if (!mpDoc->IsThreadedGroupCalcInProgress())
        return GetFormatTable()->GetStandardFormat(eType, eLnge);
    return SvNFEngine::GetStandardFormat(*mxLanguageData, *mpFormatData, *mpNatNum, maROPolicy,
                                         eType, eLnge);
}

sal_uInt32 ScInterpreterContext::NFGetStandardFormat(sal_uInt32 nFIndex, SvNumFormatType eType,
                                                     LanguageType eLnge)
{
    if (!mpDoc->IsThreadedGroupCalcInProgress())
        return GetFormatTable()->GetStandardFormat(nFIndex, eType, eLnge);
    return SvNFEngine::GetStandardFormat(*mxLanguageData, *mpFormatData, *mpNatNum, maROPolicy,
                                         nFIndex, eType, eLnge);
}

void ScInterpreterContext::NFGetInputLineString(const double& fOutNumber, sal_uInt32 nFIndex,
                                                OUString& rOutString, bool bFiltering,
                                                bool bForceSystemLocale) const
{
    if (!mpDoc->IsThreadedGroupCalcInProgress())
        return GetFormatTable()->GetInputLineString(fOutNumber, nFIndex, rOutString, bFiltering,
                                                    bForceSystemLocale);
    return SvNFEngine::GetInputLineString(*mxLanguageData, *mpFormatData, *mpNatNum, maROPolicy,
                                          fOutNumber, nFIndex, rOutString, bFiltering,
                                          bForceSystemLocale);
}
void ScInterpreterContext::NFGetOutputString(const double& fOutNumber, sal_uInt32 nFIndex,
                                             OUString& sOutString, const Color** ppColor,
                                             bool bUseStarFormat) const
{
    if (!mpDoc->IsThreadedGroupCalcInProgress())
        return GetFormatTable()->GetOutputString(fOutNumber, nFIndex, sOutString, ppColor,
                                                 bUseStarFormat);
    return SvNFEngine::GetOutputString(*mxLanguageData, *mpFormatData, *mpNatNum, maROPolicy,
                                       fOutNumber, nFIndex, sOutString, ppColor, bUseStarFormat);
}

void ScInterpreterContext::NFGetOutputString(const OUString& sString, sal_uInt32 nFIndex,
                                             OUString& sOutString, const Color** ppColor,
                                             bool bUseStarFormat) const
{
    if (!mpDoc->IsThreadedGroupCalcInProgress())
        return GetFormatTable()->GetOutputString(sString, nFIndex, sOutString, ppColor,
                                                 bUseStarFormat);
    return SvNFEngine::GetOutputString(*mxLanguageData, *mpFormatData, sString, nFIndex, sOutString,
                                       ppColor, bUseStarFormat);
}

sal_uInt32 ScInterpreterContext::NFGetStandardIndex(LanguageType eLnge) const
{
    if (!mpDoc->IsThreadedGroupCalcInProgress())
        return GetFormatTable()->GetStandardIndex(eLnge);
    return SvNFEngine::GetStandardIndex(*mxLanguageData, *mpFormatData, *mpNatNum, maROPolicy,
                                        eLnge);
}

bool ScInterpreterContext::NFGetPreviewString(const OUString& sFormatString, double fPreviewNumber,
                                              OUString& sOutString, const Color** ppColor,
                                              LanguageType eLnge)
{
    if (!mpDoc->IsThreadedGroupCalcInProgress())
        return GetFormatTable()->GetPreviewString(sFormatString, fPreviewNumber, sOutString,
                                                  ppColor, eLnge);
    return SvNFEngine::GetPreviewString(*mxLanguageData, *mpFormatData, *mpNatNum, maROPolicy,
                                        sFormatString, fPreviewNumber, sOutString, ppColor, eLnge,
                                        false);
}
bool ScInterpreterContext::NFGetPreviewString(const OUString& sFormatString,
                                              const OUString& sPreviewString, OUString& sOutString,
                                              const Color** ppColor, LanguageType eLnge)
{
    if (!mpDoc->IsThreadedGroupCalcInProgress())
        return GetFormatTable()->GetPreviewString(sFormatString, sPreviewString, sOutString,
                                                  ppColor, eLnge);
    return SvNFEngine::GetPreviewString(*mxLanguageData, *mpFormatData, *mpNatNum, maROPolicy,
                                        sFormatString, sPreviewString, sOutString, ppColor, eLnge);
}

bool ScInterpreterContext::NFGetPreviewStringGuess(const OUString& sFormatString,
                                                   double fPreviewNumber, OUString& sOutString,
                                                   const Color** ppColor, LanguageType eLnge)
{
    if (!mpDoc->IsThreadedGroupCalcInProgress())
        return GetFormatTable()->GetPreviewStringGuess(sFormatString, fPreviewNumber, sOutString,
                                                       ppColor, eLnge);
    return SvNFEngine::GetPreviewStringGuess(*mxLanguageData, *mpFormatData, *mpNatNum, maROPolicy,
                                             sFormatString, fPreviewNumber, sOutString, ppColor,
                                             eLnge);
}

OUString ScInterpreterContext::NFGenerateFormat(sal_uInt32 nIndex, LanguageType eLnge,
                                                bool bThousand, bool bIsRed, sal_uInt16 nPrecision,
                                                sal_uInt16 nLeadingCnt)
{
    if (!mpDoc->IsThreadedGroupCalcInProgress())
        return GetFormatTable()->GenerateFormat(nIndex, eLnge, bThousand, bIsRed, nPrecision,
                                                nLeadingCnt);
    return SvNFEngine::GenerateFormat(*mxLanguageData, *mpFormatData, *mpNatNum, maROPolicy, nIndex,
                                      eLnge, bThousand, bIsRed, nPrecision, nLeadingCnt);
}
OUString ScInterpreterContext::NFGetCalcCellReturn(sal_uInt32 nFormat) const
{
    if (!mpDoc->IsThreadedGroupCalcInProgress())
        return GetFormatTable()->GetCalcCellReturn(nFormat);
    return mpFormatData->GetCalcCellReturn(nFormat);
}

sal_uInt16 ScInterpreterContext::NFExpandTwoDigitYear(sal_uInt16 nYear) const
{
    if (!mpDoc->IsThreadedGroupCalcInProgress())
        return GetFormatTable()->ExpandTwoDigitYear(nYear);
    return mxLanguageData->ExpandTwoDigitYear(nYear);
}

bool ScInterpreterContext::NFIsNumberFormat(const OUString& sString, sal_uInt32& F_Index,
                                            double& fOutNumber, SvNumInputOptions eInputOptions)
{
    if (!mpDoc->IsThreadedGroupCalcInProgress())
        return GetFormatTable()->IsNumberFormat(sString, F_Index, fOutNumber, eInputOptions);
    return SvNFEngine::IsNumberFormat(*mxLanguageData, *mpFormatData, *mpNatNum, maROPolicy,
                                      sString, F_Index, fOutNumber, eInputOptions);
}

/* ScInterpreterContextPool */

// Threaded version
void ScInterpreterContextPool::Init(size_t nNumThreads, const ScDocument& rDoc,
                                    SvNumberFormatter* pFormatter)
{
    assert(mbThreaded);
    size_t nOldSize = maPool.size();
    maPool.resize(nNumThreads);
    for (size_t nIdx = 0; nIdx < nNumThreads; ++nIdx)
    {
        if (nIdx >= nOldSize)
            maPool[nIdx].reset(new ScInterpreterContext(rDoc, pFormatter));
        else
            maPool[nIdx]->SetDocAndFormatter(rDoc, pFormatter);
    }
}

ScInterpreterContext*
ScInterpreterContextPool::GetInterpreterContextForThreadIdx(size_t nThreadIdx) const
{
    assert(mbThreaded);
    assert(nThreadIdx < maPool.size());
    return maPool[nThreadIdx].get();
}

// Non-Threaded version
void ScInterpreterContextPool::Init(const ScDocument& rDoc, SvNumberFormatter* pFormatter)
{
    assert(!mbThreaded);
    assert(mnNextFree <= maPool.size());
    bool bCreateNew = (maPool.size() == mnNextFree);
    size_t nCurrIdx = mnNextFree;
    if (bCreateNew)
    {
        maPool.resize(maPool.size() + 1);
        maPool[nCurrIdx].reset(new ScInterpreterContext(rDoc, pFormatter));
    }
    else
        maPool[nCurrIdx]->SetDocAndFormatter(rDoc, pFormatter);

    ++mnNextFree;
}

ScInterpreterContext* ScInterpreterContextPool::GetInterpreterContext() const
{
    assert(!mbThreaded);
    assert(mnNextFree && (mnNextFree <= maPool.size()));
    return maPool[mnNextFree - 1].get();
}

void ScInterpreterContextPool::ReturnToPool()
{
    if (mbThreaded)
    {
        for (size_t nIdx = 0; nIdx < maPool.size(); ++nIdx)
            maPool[nIdx]->Cleanup();
    }
    else
    {
        assert(mnNextFree && (mnNextFree <= maPool.size()));
        --mnNextFree;
        maPool[mnNextFree]->Cleanup();
    }
}

// static
void ScInterpreterContextPool::ClearLookupCaches(const ScDocument* pDoc)
{
    for (auto& rPtr : aThreadedInterpreterPool.maPool)
        rPtr->ClearLookupCache(pDoc);
    for (auto& rPtr : aNonThreadedInterpreterPool.maPool)
        rPtr->ClearLookupCache(pDoc);
}

// static
void ScInterpreterContextPool::ModuleExiting()
{
    for (auto& rPtr : aThreadedInterpreterPool.maPool)
        rPtr->mxLanguageData.reset();
    for (auto& rPtr : aNonThreadedInterpreterPool.maPool)
        rPtr->mxLanguageData.reset();
}

/* ScThreadedInterpreterContextGetterGuard */

ScThreadedInterpreterContextGetterGuard::ScThreadedInterpreterContextGetterGuard(
    size_t nNumThreads, const ScDocument& rDoc, SvNumberFormatter* pFormatter)
    : rPool(ScInterpreterContextPool::aThreadedInterpreterPool)
{
    rPool.Init(nNumThreads, rDoc, pFormatter);
}

ScThreadedInterpreterContextGetterGuard::~ScThreadedInterpreterContextGetterGuard()
{
    rPool.ReturnToPool();
}

ScInterpreterContext*
ScThreadedInterpreterContextGetterGuard::GetInterpreterContextForThreadIdx(size_t nThreadIdx) const
{
    return rPool.GetInterpreterContextForThreadIdx(nThreadIdx);
}

/* ScInterpreterContextGetterGuard */

ScInterpreterContextGetterGuard::ScInterpreterContextGetterGuard(const ScDocument& rDoc,
                                                                 SvNumberFormatter* pFormatter)
    : rPool(ScInterpreterContextPool::aNonThreadedInterpreterPool)
#if !defined NDEBUG
    , nContextIdx(rPool.mnNextFree)
#endif
{
    rPool.Init(rDoc, pFormatter);
}

ScInterpreterContextGetterGuard::~ScInterpreterContextGetterGuard()
{
    assert(nContextIdx == (rPool.mnNextFree - 1));
    rPool.ReturnToPool();
}

ScInterpreterContext* ScInterpreterContextGetterGuard::GetInterpreterContext() const
{
    return rPool.GetInterpreterContext();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
