/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <externvar.hxx>

int v1;
int v2; // expected-error {{variable with external linkage not declared in an include file [loplugin:externvar]}} expected-note {{should either have internal linkage or be declared in an include file [loplugin:externvar]}}
static int v3;
int const v4 = 0;

extern int v5; // expected-note {{previously declared here [loplugin:externvar]}}
int v5; // expected-error {{variable with external linkage not declared in an include file [loplugin:externvar]}} expected-note {{should either have internal linkage or be declared in an include file [loplugin:externvar]}}

extern "C" int v6; // expected-note {{previously declared here [loplugin:externvar]}}
int v6; // expected-error {{variable with external linkage not declared in an include file [loplugin:externvar]}} expected-note {{should either have internal linkage or be declared in an include file [loplugin:externvar]}}
extern "C" { int v7; } // expected-error {{variable with external linkage not declared in an include file [loplugin:externvar]}} expected-note {{should either have internal linkage or be declared in an include file [loplugin:externvar]}}

namespace {

int u1;
static int u2;
extern "C" int u3; // expected-note {{previously declared here [loplugin:externvar]}}
int u3; // expected-error {{variable with external linkage not declared in an include file [loplugin:externvar]}} expected-note {{should either have internal linkage or be declared in an include file [loplugin:externvar]}}
extern "C" { int u4; }

}

namespace N {

int v1;
int v2; // expected-error {{variable with external linkage not declared in an include file [loplugin:externvar]}} expected-note {{should either have internal linkage or be declared in an include file [loplugin:externvar]}}
static int v3;

}

struct S {
    static int f() {
        static int s = 0;
        return s;
    }

    static int m;
};

int S::m = 0;

int f(int a) {
    static int s = 0;
    ++s;
    int b = a + s + v1 + v2 + v3 + v4 + v5 + v6 + v7 + u1 + u2 + u3 + u4 + N::v1
        + N::v2 + N::v3 + S::f();
    return b;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
