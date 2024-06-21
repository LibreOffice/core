/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cassert>
#include <cstddef>

#include <bridges/emscriptencxxabi/cxxabi.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>

OUString emscriptencxxabi::toUnoName(char const* name)
{
    assert(name != nullptr);
    OUStringBuffer b;
    bool scoped = *name == 'N';
    if (scoped)
    {
        ++name;
    }
    for (;;)
    {
        assert(*name >= '0' && *name <= '9');
        std::size_t n = *name++ - '0';
        while (*name >= '0' && *name <= '9')
        {
            n = 10 * n + (*name++ - '0');
        }
        b.appendAscii(name, n);
        name += n;
        if (!scoped)
        {
            assert(*name == 0);
            break;
        }
        if (*name == 'E')
        {
            assert(name[1] == 0);
            break;
        }
        b.append('.');
    }
    return b.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
