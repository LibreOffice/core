/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cassert>
#include <cstdlib>
#include <limits>
#include <memory>

#include <o3tl/runtimetooustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <sal/backtrace.hxx>

#include "backtrace.h"
#include "backtraceasstring.hxx"

namespace {

struct FreeGuard {
    FreeGuard(char ** theBuffer): buffer(theBuffer) {}

    ~FreeGuard() { std::free(buffer); }

    char ** buffer;
};

}

OUString osl::detail::backtraceAsString(sal_uInt32 maxDepth) {
    assert(maxDepth != 0);
    auto const maxInt = static_cast<unsigned int>(
        std::numeric_limits<int>::max());
    if (maxDepth > maxInt) {
        maxDepth = static_cast<sal_uInt32>(maxInt);
    }
    auto b1 = std::unique_ptr<void *[]>(new void *[maxDepth]);
    int n = backtrace(b1.get(), static_cast<int>(maxDepth));
    FreeGuard b2(backtrace_symbols(b1.get(), n));
    b1.reset();
    if (b2.buffer == nullptr) {
        return OUString();
    }
    OUStringBuffer b3;
    for (int i = 0; i != n; ++i) {
        if (i != 0) {
            b3.append("\n");
        }
        b3.append(o3tl::runtimeToOUString(b2.buffer[i]));
    }
    return b3.makeStringAndClear();
}

std::unique_ptr<BacktraceState> sal_backtrace_get(sal_uInt32 maxDepth)
{
    assert(maxDepth != 0);
    auto const maxInt = static_cast<unsigned int>(
        std::numeric_limits<int>::max());
    if (maxDepth > maxInt) {
        maxDepth = static_cast<sal_uInt32>(maxInt);
    }
    auto b1 = new void *[maxDepth];
    int n = backtrace(b1, static_cast<int>(maxDepth));
    return std::unique_ptr<BacktraceState>(new BacktraceState{ b1, n });
}

OUString sal_backtrace_to_string(BacktraceState* backtraceState)
{
    FreeGuard b2(backtrace_symbols(backtraceState->buffer, backtraceState->nDepth));
    if (b2.buffer == nullptr) {
        return OUString();
    }
    OUStringBuffer b3;
    for (int i = 0; i != backtraceState->nDepth; ++i) {
        if (i != 0) {
            b3.append("\n");
        }
        b3.append(o3tl::runtimeToOUString(b2.buffer[i]));
    }
    return b3.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
