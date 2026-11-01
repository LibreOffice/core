/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <vcl/throwawaymemory.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <cstdint>
#include <cstring>
#include <vector>

#if defined __cpp_lib_memory_resource

namespace
{
class ThrowawayMemoryTest : public CppUnit::TestFixture
{
    void testBlocksAreAlignedAndSeparate();
    void testContainerKeepsItsContentsWhileGrowing();

    CPPUNIT_TEST_SUITE(ThrowawayMemoryTest);
    CPPUNIT_TEST(testBlocksAreAlignedAndSeparate);
    CPPUNIT_TEST(testContainerKeepsItsContentsWhileGrowing);
    CPPUNIT_TEST_SUITE_END();
};

void ThrowawayMemoryTest::testBlocksAreAlignedAndSeparate()
{
    ThrowawayMemory aMemory;
    struct Block
    {
        unsigned char* p;
        std::size_t nSize;
    };
    std::vector<Block> aBlocks;
    // Several megabytes in blocks of mixed sizes and alignments, well past the first block taken
    // from the system.
    for (std::size_t i = 0; i < 20000; ++i)
    {
        const std::size_t nSize = 1 + (i * 37) % 400;
        const std::size_t nAlignment = std::size_t(1) << (i % 5);
        auto* p = static_cast<unsigned char*>(aMemory.allocate(nSize, nAlignment));
        CPPUNIT_ASSERT_EQUAL(std::uintptr_t(0), reinterpret_cast<std::uintptr_t>(p) % nAlignment);
        std::memset(p, static_cast<int>(i % 251), nSize);
        aBlocks.push_back({ p, nSize });
    }
    for (std::size_t i = 0; i < aBlocks.size(); ++i)
    {
        for (std::size_t j = 0; j < aBlocks[i].nSize; ++j)
            CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(i % 251), aBlocks[i].p[j]);
    }
}

void ThrowawayMemoryTest::testContainerKeepsItsContentsWhileGrowing()
{
    ThrowawayMemory aMemory;
    std::pmr::vector<int> aValues(&aMemory);
    for (int i = 0; i < 100000; ++i)
        aValues.push_back(i);
    for (int i = 0; i < 100000; ++i)
        CPPUNIT_ASSERT_EQUAL(i, aValues[i]);
}

CPPUNIT_TEST_SUITE_REGISTRATION(ThrowawayMemoryTest);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
