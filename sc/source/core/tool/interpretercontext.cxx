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
    mpFormatter = pFormatter;
}

void ScInterpreterContext::initFormatTable()
{
    mpFormatter = mpDoc->GetFormatTable(); // will assert if not main thread
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
        mxScLookupCache.reset();
}

SvNumFormatType ScInterpreterContext::GetNumberFormatType(sal_uInt32 nFIndex) const
{
    if (!mpDoc->IsThreadedGroupCalcInProgress())
    {
        return mpFormatter->GetType(nFIndex);
    }

    if (maNFTypeCache.bIsValid && maNFTypeCache.nIndex == nFIndex)
    {
        return maNFTypeCache.eType;
    }

    maNFTypeCache.nIndex = nFIndex;
    maNFTypeCache.eType = mpFormatter->GetType(nFIndex);
    maNFTypeCache.bIsValid = true;
    return maNFTypeCache.eType;
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
