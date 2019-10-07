/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cstring>

void f(struct Incomplete* p1, struct S* p2);

struct S
{
    S();
};

void f(struct Incomplete* p1, struct S* p2)
{
    S s;
    // expected-error@+1 {{writing to an object of non-trivial type 'S'; use assignment instead [loplugin:classmemaccess]}}
    std::memset(&s, 0, sizeof s);
    // expected-error@+1 {{writing to an object of non-trivial type 'S'; use assignment instead [loplugin:classmemaccess]}}
    std::memset(static_cast<void*>(&s), 0, sizeof s);
    auto const disableWarning = static_cast<void*>(&s);
    std::memset(disableWarning, 0, sizeof s);
    S a[1][1];
    // expected-error@+1 {{writing to an object of non-trivial type 'S'; use assignment instead [loplugin:classmemaccess]}}
    std::memset(a, 0, sizeof a);
    std::memset(p1, 0, 10); // conservatively assume Incomplete may be trivial
    // expected-error@+1 {{writing to an object of non-trivial type 'S'; use assignment instead [loplugin:classmemaccess]}}
    std::memset(p2, 0, 10);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
