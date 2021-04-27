/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "config_clang.h"
#include <vector>

namespace test1
{
void foo1(std::vector<int> x)
{
    std::vector<int> y;
    // expected-error@+1 {{can std::move value instead of copy [loplugin:optmove]}}
    y = x;
}
}

namespace test2
{
void foo(std::vector<int> x)
{
    // expected-error@+1 {{can std::move value instead of copy [loplugin:optmove]}}
    std::vector<int> y = x;
}
}

namespace test3
{
void foo1(std::vector<int> x)
{
    std::vector<int> y, z;
    y = x;
    // expected-error@+1 {{can std::move value instead of copy [loplugin:optmove]}}
    z = x;
}
}

namespace test4
{
void foo1(std::vector<int> x)
{
    std::vector<int> y;
    // no warning expected, don't even try to follow loop
    for (int i = 0; i < 10; i++)
        y = x;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
