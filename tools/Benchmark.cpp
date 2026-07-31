/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Benchmark various bits of cool code.
 */

#include <config.h>

#include <chrono>

#include <common/Png.hpp>
#include <kit/Delta.hpp>

using Pixmap = std::vector<char>;

static std::vector<Pixmap> pixmaps;

class DeltaTests {
public:
    static void rleBitmap(Pixmap &pix)
    {
        TileLocation loc = { 0, 0, 0, 0, CanonicalViewId::None, 0 };
        DeltaGenerator::DeltaData rleData(
            1 /*wid*/, reinterpret_cast<unsigned char *>(pix.data()),
            0, 0, 256, 256, loc, 256, 256);
    }

    static void timeRLE(const char *description)
    {
        std::cout << "Benchmark " << description << "\n";

        int deltas = 0;
        const auto start = std::chrono::steady_clock::now();

        // check we have enough work:
        int maxIters = (50000 + pixmaps.size() - 1) / pixmaps.size();
        for (int it = 0; it < maxIters; ++it)
        {
//          std::cerr << "iter " << it << "\n";
            for (Pixmap &pix : pixmaps)
            {
                deltas++;
                rleBitmap(pix);
            }
        }

        const auto end = std::chrono::steady_clock::now();

        std::cout << "took: " <<
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms - ";

        assert(deltas && "div by zero otherwise");

        std::cout << "time/rle: " <<
            (1.0*std::chrono::duration_cast<std::chrono::microseconds>(end - start).count())/deltas << "us\n";
    }
};

int main (int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        uint32_t height, width, rowBytes;
        pixmaps.push_back(Png::loadPng(argv[i], height, width, rowBytes));
//        std::cout << "Loaded: " << argv[i] << " " << width << "x" << height << "\n";
    }

    DeltaTests::timeRLE("CPU");

    simd::init();

    DeltaTests::timeRLE("SIMD");

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
