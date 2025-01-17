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

#include <concepts>

// LLVM 12 libc++ only provides a bare-bones <concepts> that lacks most of its C++20 content, so
// replicate here fore now what we need:

#if defined __cpp_lib_concepts

namespace o3tl
{
using std::integral;
using std::signed_integral;
using std::unsigned_integral;
using std::floating_point;
}

#else

#include <type_traits>

namespace o3tl
{
// Taken from the C++20 spec:

template <typename T> concept integral = std::is_integral_v<T>;

template <typename T> concept signed_integral = integral<T>&& std::is_signed_v<T>;

template <typename T> concept unsigned_integral = integral<T> && !signed_integral<T>;

template <typename T> concept floating_point = std::is_floating_point_v<T>;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
