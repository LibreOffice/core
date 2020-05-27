/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

namespace o3tl
{
template <typename T, typename N, std::enable_if_t<(sizeof(N) == 4), bool> = false>
inline void hash_combine(N& nSeed, T const& nValue)
{
    static_assert(sizeof(nSeed) == 4);
    nSeed ^= std::hash<T>{}(nValue) + 0x9E3779B9u + (nSeed << 6) + (nSeed >> 2);
}

template <typename T, typename N, std::enable_if_t<(sizeof(N) == 8), bool> = false>
inline void hash_combine(N& nSeed, T const& nValue)
{
    static_assert(sizeof(nSeed) == 8);
    nSeed ^= std::hash<T>{}(nValue) + 0x9E3779B97F4A7C15llu + (nSeed << 12) + (nSeed >> 4);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
