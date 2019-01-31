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

// A better replacement for assert(false) to indicate a place in the code that should not be
// reachable.  This should improve on the sometimes poor false-positive warnings emitted by
// compilers when they cannot detect that some condition flagged by assert(false) cannot occur,
// either because assert is reduced to a no-op by NDEBUG or because assert is not marked as noreturn
// in the MSVC headers.  This is inspired by LLVM's LLVM_BUILTIN_UNREACHABLE
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
