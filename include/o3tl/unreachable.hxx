/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_O3TL_UNREACHABLE_HXX
#define INCLUDED_O3TL_UNREACHABLE_HXX

#include <sal/config.h>

#include <cassert>
#include <utility>

// An approximation of C++23 std::unreachable
// (<http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p0627r6.pdf> "Function to mark
// unreachable code").

#if defined __cpp_lib_unreachable

#define O3TL_UNREACHABLE (::std::unreachable())

#else

// This fallback implementation is inspired by LLVM's LLVM_BUILTIN_UNREACHABLE
// (llvm/include/llvm/Support/Compiler.h).

#if defined _MSC_VER
#define O3TL_UNREACHABLE_detail __assume(false)
#else // assuming Clang or GCC with support for:
#define O3TL_UNREACHABLE_detail __builtin_unreachable()
#endif

#define O3TL_UNREACHABLE                                                                           \
    do                                                                                             \
    {                                                                                              \
        assert(false);                                                                             \
        O3TL_UNREACHABLE_detail;                                                                   \
    } while (false)

#endif

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
