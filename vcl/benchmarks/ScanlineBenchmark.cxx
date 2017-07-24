/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/cpuid.hxx>

#include "ScanlineTools.hxx"

// SCOPED NANO TIMER
#include <functional>
#include <chrono>
#include <iostream>

struct ScopedNanoTimer
{
    std::string msName;
    std::chrono::high_resolution_clock::time_point maStart;

    ScopedNanoTimer(std::string sName)
        : msName(sName)
        , maStart(std::chrono::high_resolution_clock::now())
    {}

    ~ScopedNanoTimer(void)
    {
        auto aEnd = std::chrono::high_resolution_clock::now();
        sal_Int64 aDurationInMilliSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(aEnd - maStart).count();
        if (aDurationInMilliSeconds > 10)
            std::cout << msName << " time : " << aDurationInMilliSeconds << " ms" << std::endl;
    }
};

namespace vcl
{

namespace benchmark
{

void benchmarkConvertScanlineRGBAtoBGRA()
{
    sal_uInt8 R = 1;
    sal_uInt8 G = 100;
    sal_uInt8 B = 57;
    sal_uInt8 A = 255;

    sal_uInt8 aPixelSource[]   = { R, G, B, A };

    std::vector<sal_uInt8> aSource(2000 * 4);
    std::vector<sal_uInt8> aDestination(2000 * 4);

    for (size_t i = 0; i < aSource.size(); ++i)
    {
        aSource[i] = aPixelSource[i % 4];
    }

    const int nNumberOfLoops = 100000;

    {
        ScopedNanoTimer aTimer("loop");
        for (int c = 0; c < nNumberOfLoops; c++)
        {
            vcl::scanline::swapABCDtoCBAD(aSource.data(), aDestination.data(), aSource.size());
        }
    }
    if (cpuid::hasSSE2())
    {
        ScopedNanoTimer aTimer("sse2");
        for (int c = 0; c < nNumberOfLoops; c++)
        {
            vcl::scanline::swapABCDtoCBAD_SSE2(aSource.data(), aDestination.data(), aSource.size());
        }
    }
    if (cpuid::hasSSSE3())
    {
        ScopedNanoTimer aTimer("ssse3");
        for (int c = 0; c < nNumberOfLoops; c++)
        {
            vcl::scanline::swapABCDtoCBAD_SSSE3(aSource.data(), aDestination.data(), aSource.size());
        }
    }
    if (cpuid::hasAVX2())
    {
        ScopedNanoTimer aTimer("avx2");
        for (int c = 0; c < nNumberOfLoops; c++)
        {
            vcl::scanline::swapABCDtoCBAD_AVX2(aSource.data(), aDestination.data(), aSource.size());
        }
    }
}

} // anonymous namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
