/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vector>

#pragma clang diagnostic ignored "-Wunknown-warning-option" // for Clang < 13
#pragma clang diagnostic ignored "-Wunused-but-set-variable"

void func1()
{
    std::vector<int> v1;
    int n = 0;

    for (int i = 0; i < 10; ++i) // expected-error {{loop variable not used [loplugin:unusedindex]}}
        n += 1;
    for (int i = 0; i < 10; ++i)
        n += i;

    for (int i = 0; i < 10; ++i) // expected-error {{loop variable not used [loplugin:unusedindex]}}
    {
        for (int j = 0; j < 10; ++j)
        {
            n += j;
        }
    }
    for (int i = 0; i < 10; ++i)
    {
        for (int j = 0; j < 10; ++j)
        {
            n += j;
            n += i;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
