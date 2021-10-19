/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <sal/config.h>
#include "swdllapi.h"
#include <o3tl/strong_int.hxx>
#include <iostream>

typedef o3tl::strong_int<sal_Int32, struct Tag_SwNodeOffset> SwNodeOffset;

/* Just to make it easier to write arithmetic with these types */
template <typename T>
typename std::enable_if<std::is_signed<T>::value, SwNodeOffset>::type operator+(SwNodeOffset a, T n)
{
    return a + SwNodeOffset(n);
}

/* Just to make it easier to write arithmetic with these types */
template <typename T>
typename std::enable_if<std::is_signed<T>::value, SwNodeOffset>::type operator-(SwNodeOffset a, T n)
{
    return a - SwNodeOffset(n);
}

inline SwNodeOffset abs(const SwNodeOffset& a) { return a > SwNodeOffset(0) ? a : -a; }
inline SwNodeOffset min(const SwNodeOffset& a, const SwNodeOffset& b) { return a > b ? a : b; }

constexpr SwNodeOffset NODE_OFFSET_MAX(SAL_MAX_INT32);

SW_DLLPUBLIC std::ostream& operator<<(std::ostream& s, const SwNodeOffset& index);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
