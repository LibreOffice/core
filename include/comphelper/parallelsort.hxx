/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPHELPER_PARALLELSORT_HXX
#define INCLUDED_COMPHELPER_PARALLELSORT_HXX

#include <comphelper/comphelperdllapi.h>
#include <comphelper/threadpool.hxx>
#include <tools/cpuid.hxx>

#include <memory>
#include <iterator>
#include <thread>
#include <algorithm>
#include <cmath>
#include <random>
#include <functional>
#include <iostream>
#include <chrono>

namespace comphelper
{
const size_t nThreadCountGlobal = std::thread::hardware_concurrency();
const bool bHyperThreadingActive = cpuid::hasHyperThreading();
static comphelper::ThreadPool& rTPool(comphelper::ThreadPool::getSharedOptimalPool());

static thread_local std::mt19937 aGenerator{ std::random_device{}() };

#define PARALLELSORT_ENABLEPZ 0

namespace
{
class ProfileZone
{
public:
#if PARALLELSORT_ENABLEPZ
    ProfileZone(const char* pTag)
        : maTag(pTag)
        , maStart(std::chrono::steady_clock::now())
        , mbFinished(false)
    {
    }

    ~ProfileZone()
    {
        if (!mbFinished)
            showTimeElapsed();
    }

    void stop()
    {
        showTimeElapsed();
        mbFinished = true;
    }
#else
    ProfileZone(const char* /*pTag*/)
        : mbDummy(true)
    {
    }

    void stop()
    {
        // Avoid loplugin:staticmethods, loplugin:staticaccess errors
        (void)mbDummy;
    }
#endif

private:
#if PARALLELSORT_ENABLEPZ

    void showTimeElapsed()
    {
        auto end = std::chrono::steady_clock::now();
        size_t elapsed
            = std::chrono::duration_cast<std::chrono::milliseconds>(end - maStart).count();
        std::cout << maTag << " : " << elapsed << " ms" << std::endl << std::flush;
    }

    std::string maTag;
    std::chrono::steady_clock::time_point maStart;
    bool mbFinished;
#else
    bool mbDummy;

#endif
};

class ParallelRunner
{
    class Executor final : public comphelper::ThreadTask
    {
    public:
        Executor(const std::shared_ptr<comphelper::ThreadTaskTag>& rTag,
                 std::function<void()> aFunc)
            : comphelper::ThreadTask(rTag)
            , maFunc(std::move(aFunc))
        {
        }

        virtual void doWork() override { maFunc(); }

    private:
        const std::function<void()> maFunc;
    };

public:
    ParallelRunner() { maTag = comphelper::ThreadPool::createThreadTaskTag(); }

    void enqueue(std::function<void()> aFunc)
    {
        rTPool.pushTask(std::make_unique<Executor>(maTag, aFunc));
    }

    void wait() { rTPool.waitUntilDone(maTag, false); }

private:
    std::shared_ptr<comphelper::ThreadTaskTag> maTag;
};

constexpr size_t nMaxTreeArraySize = 64;

size_t lcl_round_down_pow2(size_t nNum)
{
    size_t nPow2;
    for (nPow2 = 1; nPow2 <= nNum; nPow2 <<= 1)
        ;
    return std::min((nPow2 >> 1), nMaxTreeArraySize);
}

template <class RandItr> struct Sampler
{
    using ValueType = typename std::iterator_traits<RandItr>::value_type;

    static void sample(RandItr aBegin, RandItr aEnd, ValueType* pSamples, size_t nSamples,
                       size_t /*nParallelism*/)
    {
        ProfileZone aZone("\tsample()");
        assert(aBegin <= aEnd);
        size_t nLen = static_cast<std::size_t>(aEnd - aBegin);
        assert(std::mt19937::max() >= nLen);

        for (size_t nIdx = 0; nIdx < nSamples; ++nIdx)
        {
            size_t nSel = aGenerator() % nLen--;
            std::swap(*(aBegin + nSel), *(aBegin + nLen));
            pSamples[nIdx] = *(aBegin + nLen);
        }
    }
};

template <class RandItr, class Compare> class Binner
{
    using ValueType = typename std::iterator_traits<RandItr>::value_type;

    const size_t mnTreeArraySize;
    const size_t mnDividers;
    constexpr static size_t mnMaxStaticSize = 1024 * 50;
    uint8_t maLabels[mnMaxStaticSize];
    ValueType maDividers[nMaxTreeArraySize];
    std::unique_ptr<uint8_t[]> pLabels;
    size_t maSepBinEnds[nMaxTreeArraySize * nMaxTreeArraySize];
    bool mbThreaded;

public:
    size_t maBinEnds[nMaxTreeArraySize];

    Binner(const ValueType* pSamples, size_t nSamples, size_t nBins, bool bThreaded)
        : mnTreeArraySize(lcl_round_down_pow2(nBins))
        , mnDividers(mnTreeArraySize - 1)
        , mbThreaded(bThreaded)
    {
        assert((nSamples % mnTreeArraySize) == 0);
        assert(mnTreeArraySize <= nMaxTreeArraySize);
        std::fill(maBinEnds, maBinEnds + mnTreeArraySize, 0);
        std::fill(maSepBinEnds, maSepBinEnds + mnTreeArraySize * mnTreeArraySize, 0);
        fillTreeArray(1, pSamples, pSamples + nSamples);
    }

    void fillTreeArray(size_t nPos, const ValueType* pLow, const ValueType* pHigh)
    {
        assert(pLow <= pHigh);
        const ValueType* pMid = pLow + (pHigh - pLow) / 2;
        maDividers[nPos] = *pMid;

        if (2 * nPos < mnDividers) // So that 2*nPos < mnTreeArraySize
        {
            fillTreeArray(2 * nPos, pLow, pMid);
            fillTreeArray(2 * nPos + 1, pMid + 1, pHigh);
        }
    }

    constexpr inline size_t findBin(const ValueType& rVal, Compare& aComp)
    {
        size_t nIdx = 1;
        while (nIdx <= mnDividers)
            nIdx = ((nIdx << 1) + aComp(maDividers[nIdx], rVal));
        return (nIdx - mnTreeArraySize);
    }

    void label(const RandItr aBegin, const RandItr aEnd, Compare& aComp)
    {
        ProfileZone aZoneSetup("\tlabel():setup");
        size_t nLen = static_cast<std::size_t>(aEnd - aBegin);
        if (nLen > mnMaxStaticSize)
            pLabels = std::make_unique<uint8_t[]>(nLen);
        uint8_t* pLabelsRaw = (nLen > mnMaxStaticSize) ? pLabels.get() : maLabels;
        aZoneSetup.stop();
        ProfileZone aZoneFindBins("\tFindBins()");
        if (mbThreaded)
        {
            ParallelRunner aPRunner;
            const size_t nBins = mnTreeArraySize;
            for (size_t nTIdx = 0; nTIdx < nBins; ++nTIdx)
            {
                aPRunner.enqueue([this, nTIdx, nBins, nLen, aBegin, pLabelsRaw, &aComp] {
                    ProfileZone aZoneIn("\t\tFindBinsThreaded()");
                    size_t nBinEndsStartIdx = nTIdx * mnTreeArraySize;
                    size_t* pBinEnds = maSepBinEnds + nBinEndsStartIdx;
                    size_t aBinEndsF[nMaxTreeArraySize] = { 0 };
                    for (size_t nIdx = nTIdx; nIdx < nLen; nIdx += nBins)
                    {
                        size_t nBinIdx = findBin(*(aBegin + nIdx), aComp);
                        pLabelsRaw[nIdx] = static_cast<uint8_t>(nBinIdx);
                        ++aBinEndsF[nBinIdx];
                    }

                    for (size_t nIdx = 0; nIdx < mnTreeArraySize; ++nIdx)
                        pBinEnds[nIdx] = aBinEndsF[nIdx];
                });
            }

            aPRunner.wait();

            // Populate maBinEnds from maSepBinEnds
            for (size_t nTIdx = 0; nTIdx < mnTreeArraySize; ++nTIdx)
            {
                for (size_t nSepIdx = 0; nSepIdx < mnTreeArraySize; ++nSepIdx)
                    maBinEnds[nTIdx] += maSepBinEnds[nSepIdx * mnTreeArraySize + nTIdx];
            }
        }
        else
        {
            uint8_t* pLabel = pLabelsRaw;
            for (RandItr aItr = aBegin; aItr != aEnd; ++aItr)
            {
                size_t nBinIdx = findBin(*aItr, aComp);
                *pLabel++ = nBinIdx;
                ++maBinEnds[nBinIdx];
            }
        }

        aZoneFindBins.stop();

        size_t nSum = 0;
        // Store each bin's starting position in maBinEnds array for now.
        for (size_t nIdx = 0; nIdx < mnTreeArraySize; ++nIdx)
        {
            size_t nSize = maBinEnds[nIdx];
            maBinEnds[nIdx] = nSum;
            nSum += nSize;
        }

        // Now maBinEnds has end positions of each bin.
    }

    void bin(const RandItr aBegin, const RandItr aEnd, ValueType* pOut)
    {
        ProfileZone aZone("\tbin()");
        const size_t nLen = static_cast<std::size_t>(aEnd - aBegin);
        uint8_t* pLabelsRaw = (nLen > mnMaxStaticSize) ? pLabels.get() : maLabels;
        size_t nIdx;
        for (nIdx = 0; nIdx < nLen; ++nIdx)
        {
            pOut[maBinEnds[pLabelsRaw[nIdx]]++] = *(aBegin + nIdx);
        }
    }
};

template <class RandItr, class Compare = std::less<>>
void s3sort(const RandItr aBegin, const RandItr aEnd, Compare aComp = Compare(),
            bool bThreaded = true)
{
    static size_t nThreadCount = nThreadCountGlobal;

    constexpr size_t nBaseCaseSize = 1024;
    const std::size_t nLen = static_cast<std::size_t>(aEnd - aBegin);
    if (nLen < nBaseCaseSize)
    {
        std::sort(aBegin, aEnd, aComp);
        return;
    }

    using ValueType = typename std::iterator_traits<RandItr>::value_type;
    auto pOut = std::make_unique<ValueType[]>(nLen);

    const size_t nBins = lcl_round_down_pow2(nThreadCount);
    const size_t nOverSamplingFactor = std::max(1.0, std::sqrt(static_cast<double>(nLen) / 64));
    const size_t nSamples = nOverSamplingFactor * nBins;
    auto aSamples = std::make_unique<ValueType[]>(nSamples);
    ProfileZone aZoneSampleAnsSort("SampleAndSort");
    // Select samples and sort them
    Sampler<RandItr>::sample(aBegin, aEnd, aSamples.get(), nSamples, nBins);
    std::sort(aSamples.get(), aSamples.get() + nSamples, aComp);
    aZoneSampleAnsSort.stop();

    if (!aComp(aSamples[0], aSamples[nSamples - 1]))
    {
        // All samples are equal, fallback to standard sort.
        std::sort(aBegin, aEnd, aComp);
        return;
    }

    ProfileZone aZoneBinner("Binner");
    // Create and populate bins using pOut from input iterators.
    Binner<RandItr, Compare> aBinner(aSamples.get(), nSamples, nBins, bThreaded);
    aBinner.label(aBegin, aEnd, aComp);
    aBinner.bin(aBegin, aEnd, pOut.get());
    aZoneBinner.stop();

    ProfileZone aZoneSortBins("SortBins");
    ValueType* pOutRaw = pOut.get();
    if (bThreaded)
    {
        ParallelRunner aPRunner;
        // Sort the bins separately.
        for (size_t nBinIdx = 0, nBinStart = 0; nBinIdx < nBins; ++nBinIdx)
        {
            size_t nBinEnd = aBinner.maBinEnds[nBinIdx];
            aPRunner.enqueue([pOutRaw, nBinStart, nBinEnd, &aComp] {
                std::sort(pOutRaw + nBinStart, pOutRaw + nBinEnd, aComp);
            });

            nBinStart = nBinEnd;
        }

        aPRunner.wait();
    }
    else
    {
        for (size_t nBinIdx = 0, nBinStart = 0; nBinIdx < nBins; ++nBinIdx)
        {
            auto nBinEnd = aBinner.maBinEnds[nBinIdx];
            std::sort(pOutRaw + nBinStart, pOutRaw + nBinEnd, aComp);
            nBinStart = nBinEnd;
        }
    }

    aZoneSortBins.stop();

    // Move the sorted array to the array specified by input iterators.
    std::move(pOutRaw, pOutRaw + nLen, aBegin);
}

} // anonymous namespace

template <class RandItr, class Compare = std::less<>>
void parallelSort(const RandItr aBegin, const RandItr aEnd, Compare aComp = Compare())
{
    assert(aBegin <= aEnd);
    s3sort(aBegin, aEnd, aComp);
}

} // namespace comphelper

#endif // INCLUDED_COMPHELPER_PARALLELSORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
