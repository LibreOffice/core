/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_O3TL_CLAMP_HXX
#define INCLUDED_O3TL_CLAMP_HXX

#include <sal/config.h>

#include <algorithm>
#include <cassert>

// C++17 std::clamp

namespace o3tl
{
#if defined __cpp_lib_clamp

using std::clamp;

#else

template <typename T> constexpr const T& clamp(const T& v, const T& lo, const T& hi)
{
    assert(!(hi < lo));
    return v < lo ? lo : (hi < v ? hi : v);
}

#endif
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
