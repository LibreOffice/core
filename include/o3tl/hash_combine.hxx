/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <cstddef>
#include <functional>
#include <type_traits>
#include <o3tl/concepts.hxx>

namespace o3tl
{
template <typename T, type_32_bit N>
inline void hash_combine(N& nSeed, T const* pValue, size_t nCount)
{
    static_assert(sizeof(nSeed) == 4);
    for (size_t i = 0; i < nCount; ++i)
    {
        nSeed ^= std::hash<T>{}(*pValue) + 0x9E3779B9u + (nSeed << 6) + (nSeed >> 2);
        ++pValue;
    }
}

template <typename T, type_32_bit N> inline void hash_combine(N& nSeed, T const& nValue)
{
    static_assert(sizeof(nSeed) == 4);
    nSeed ^= std::hash<T>{}(nValue) + 0x9E3779B9u + (nSeed << 6) + (nSeed >> 2);
}

template <typename T, type_64_bit N>
inline void hash_combine(N& nSeed, T const* pValue, size_t nCount)
{
    static_assert(sizeof(nSeed) == 8);
    for (size_t i = 0; i < nCount; ++i)
    {
        nSeed ^= std::hash<T>{}(*pValue) + 0x9E3779B97F4A7C15llu + (nSeed << 12) + (nSeed >> 4);
        ++pValue;
    }
}

template <typename T, type_64_bit N> inline void hash_combine(N& nSeed, T const& nValue)
{
    static_assert(sizeof(nSeed) == 8);
    nSeed ^= std::hash<T>{}(nValue) + 0x9E3779B97F4A7C15llu + (nSeed << 12) + (nSeed >> 4);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
