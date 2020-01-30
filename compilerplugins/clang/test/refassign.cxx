/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <vector>

int& f1();
int& f2();

void test1()
{
    int& v1 = f1();
    v1 = f2(); // expected-error {{assigning a 'int &' to a var of type 'int &' probably does not do what you think [loplugin:refassign]}}
    f1()
        = f2(); // expected-error {{assigning a 'int &' to a var of type 'int &' probably does not do what you think [loplugin:refassign]}}

    // no warning expected
    int x = 1;
    x = f1();
    std::vector<int> v;
    v[0] = f1();
    v1 = std::min(1, 2);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
