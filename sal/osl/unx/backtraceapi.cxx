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
#include <memory>

#include <o3tl/runtimetooustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>

#include "backtrace.h"
#include "backtraceasstring.hxx"

namespace {

struct FreeGuard {
    FreeGuard(char ** theBuffer): buffer(theBuffer) {}

    ~FreeGuard() { std::free(buffer); }

    char ** buffer;
};

}

OUString osl::detail::backtraceAsString(int maxNoStackFramesToDisplay)
{
    assert(maxNoStackFramesToDisplay >= 0);
    if (maxNoStackFramesToDisplay == 0) {
        return OUString();
    }
    auto b1 = std::unique_ptr<void *[]>(new void *[maxNoStackFramesToDisplay]);
    int n = backtrace(b1.get(), maxNoStackFramesToDisplay);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
