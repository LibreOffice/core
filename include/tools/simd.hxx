/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

namespace simd
{
template <typename T, unsigned int N> inline bool isAligned(const T* pointer)
{
    return 0 == (uintptr_t(pointer) % N);
}

template <typename T> inline T roundDown(T value, unsigned int multiple)
{
    return value & ~(multiple - 1);
}

} // end namespace simd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
