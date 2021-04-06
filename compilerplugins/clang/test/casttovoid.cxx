/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#undef NDEBUG
#include <cassert>

#define CAST_N3 (void) n3
#define ASSERT_N4 assert(n4 == 0)
#define ASSERT(x) assert(x)
#define USE(x) x

int f1(int n1, int n2, int n3, int n4, int n5) {
    (void) n1; // expected-error {{unnecessary cast to void [loplugin:casttovoid]}}
    int const & r1 = n1; // expected-note {{first consumption is here [loplugin:casttovoid]}}
    (void) n2; // expected-error {{unnecessary cast to void [loplugin:casttovoid]}}
    int const & r2 = {n2}; // expected-note {{first consumption is here [loplugin:casttovoid]}}
    (void) n3; // expected-error {{unnecessary cast to void [loplugin:casttovoid]}}
    int const & r3{n3}; // expected-note {{first consumption is here [loplugin:casttovoid]}}
    (void) n4; // expected-error {{unnecessary cast to void [loplugin:casttovoid]}}
    int const & r4(n4); // expected-note {{first consumption is here [loplugin:casttovoid]}}
    (void) n5; // expected-error {{unnecessary cast to void [loplugin:casttovoid]}}
    int const & r5 = (n5); // expected-note {{first consumption is here [loplugin:casttovoid]}}
    return r1 + r2 + r3 + r4 + r5;
}

int const & f2(int const & n) {
    (void) n; // expected-error {{unnecessary cast to void [loplugin:casttovoid]}}
    return n; // expected-note {{first consumption is here [loplugin:casttovoid]}}
}

int const & f3(int const & n) {
    (void) n; // expected-error {{unnecessary cast to void [loplugin:casttovoid]}}
    return (n); // expected-note {{first consumption is here [loplugin:casttovoid]}}
}

int const & f4(int const & n) {
    (void) n; // expected-error {{unnecessary cast to void [loplugin:casttovoid]}}
    return {n}; // expected-note {{first consumption is here [loplugin:casttovoid]}}
}

int const & f5(int const & n) {
    (void) n; // expected-error {{unnecessary cast to void [loplugin:casttovoid]}}
    return {(n)}; // expected-note {{first consumption is here [loplugin:casttovoid]}}
}

struct S1 {
    S1(int n1, int n2):
        n1_(n1), // expected-note {{first consumption is here [loplugin:casttovoid]}}
        n2_{n2} // expected-note {{first consumption is here [loplugin:casttovoid]}}
    {
        (void) n1; // expected-error {{unnecessary cast to void [loplugin:casttovoid]}}
        (void) n2; // expected-error {{unnecessary cast to void [loplugin:casttovoid]}}
    }
    int const n1_;
    int const n2_;
};

struct S2 { int n; };

int fS2_1(S2 s) {
    (void) s; // expected-error {{unnecessary cast to void [loplugin:casttovoid]}}
    return s.n; // expected-note {{first consumption is here [loplugin:casttovoid]}}
}

int const & fS2_2(S2 const & s) {
    (void) s; // expected-error {{unnecessary cast to void [loplugin:casttovoid]}}
    return s.n; // expected-note {{first consumption is here [loplugin:casttovoid]}}
}

// Don't trigger assert in CastToVoid::VisitReturnStmt:
int n = [] { return 0; }();

int f() {
    int n1 = n;
    int n2 = [](int const & n) -> int const & {
        (void) n; // expected-error {{unnecessary cast to void [loplugin:casttovoid]}}
        return n; // expected-note {{first consumption is here [loplugin:casttovoid]}}
    }(n1);
    return n2;
}

int main() {
    int n1 = 0;
    (void) n1; // expected-error {{unnecessary cast to void [loplugin:casttovoid]}}
    (void const) n1; // expected-error {{unnecessary cast to void [loplugin:casttovoid]}}
    (void volatile) n1; // expected-error {{unnecessary cast to void [loplugin:casttovoid]}}
    (void const volatile) n1; // expected-error {{unnecessary cast to void [loplugin:casttovoid]}}
    (void) (n1); // expected-error {{unnecessary cast to void [loplugin:casttovoid]}}
    (void) ((n1)); // expected-error {{unnecessary cast to void [loplugin:casttovoid]}}
    (void(n1)); // expected-error {{unnecessary cast to void [loplugin:casttovoid]}}
    static_cast<void>(n1); // expected-error {{unnecessary cast to void [loplugin:casttovoid]}}
    int n2 = 0;
    assert(n2 == 0);
    (void) n2; // expected-error {{unnecessary cast to void [loplugin:casttovoid]}}
    int n3 = 0;
    CAST_N3;
    int n4 = 0;
    ASSERT_N4;
    (void) n4;
    int n5 = 0;
    assert(n5 == 0);
    (void) n5;
    int n6 = 0;
    ASSERT(n6 == 0);
    (void) n6;
    int n7 = 0;
    assert(USE(n7) == 0);
    (void) n7;
    int n8 = 0;
    ASSERT(USE(USE(n8 == 0)));
    (void) n8;
    return n1 // expected-note 8 {{first consumption is here [loplugin:casttovoid]}}
        + n2 // expected-note {{first consumption is here [loplugin:casttovoid]}}
        + n3;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
