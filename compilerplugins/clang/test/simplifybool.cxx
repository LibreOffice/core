/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

void f1(int a, int b)
{
    if (!(a < b))
    { // expected-error@-1 {{logical negation of comparison operator, can be simplified by inverting operator [loplugin:simplifybool]}}
        a = b;
    }
};

// Consitently either warn about all or none of the below occurrences of "!!":

enum E1
{
    E1_1 = 1
};

enum E2
{
    E2_1 = 1
};
E2 operator&(E2 e1, E2 e2);
bool operator!(E2 e);

enum class E3
{
    E1 = 1
};
struct W
{
    operator bool();
};
W operator&(E3 e1, E3 e2);

bool f0(int n) { return !!(n & 1); }

bool f1(E1 e) { return !!(e & E1_1); }

bool f2(E2 e) { return !!(e & E2_1); }

bool f3(E3 e) { return !!(e & E3::E1); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
