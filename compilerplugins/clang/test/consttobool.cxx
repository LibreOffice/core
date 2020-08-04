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

#include <sal/types.h>

#pragma clang diagnostic ignored "-Wnull-conversion"

enum E
{
    E0,
    E1,
    E2
};

int const c1 = 1;
constexpr int c2 = 2;

struct S
{
    S()
    // expected-error-re@+1 {{implicit conversion of constant {{nullptr|0}} of type 'nullptr_t' to 'bool'; use 'false' instead [loplugin:consttobool]}}
    : b(nullptr)
    {
    }
    bool b;
};

int main()
{
    bool b;
    // expected-error@+1 {{implicit conversion of constant 0 of type 'int' to 'bool'; use 'false' instead [loplugin:consttobool]}}
    b = 0;
    // expected-error@+1 {{implicit conversion of constant 1 of type 'sal_Bool' (aka 'unsigned char') to 'bool'; use 'true' instead [loplugin:consttobool]}}
    b = sal_True;
    // expected-error@+1 {{implicit conversion of constant 1.000000e+00 of type 'double' to 'bool'; use 'true' instead [loplugin:consttobool]}}
    b = 1.0;
    // expected-error@+1 {{implicit conversion of constant 2 of type 'E' to 'bool'; use 'true' instead [loplugin:consttobool]}}
    b = E2;
    // expected-error@+1 {{implicit conversion of constant 97 of type 'char' to 'bool'; use 'true' instead [loplugin:consttobool]}}
    b = 'a';
    // expected-error@+1 {{implicit conversion of constant 1 of type 'int' to 'bool'; use 'true' instead [loplugin:consttobool]}}
    b = c1;
    // expected-error@+1 {{implicit conversion of constant 2 of type 'int' to 'bool'; use 'true' instead [loplugin:consttobool]}}
    b = c2;
    // expected-error@+1 {{implicit conversion of constant 3 of type 'int' to 'bool'; use 'true' instead [loplugin:consttobool]}}
    b = (c1 | c2);

#if !defined NDEBUG
    assert(b); // no warnings from within assert macro itself
    assert(b && "msg"); // no warnings for `&& "msg"`
    if (b)
    {
        assert(!"msg"); // no warnings for `!"msg"`
    }
    // expected-error@+1 {{implicit conversion of constant &"msg"[0] of type 'const char *' to 'bool'; use 'true' instead [loplugin:consttobool]}}
    assert("msg");
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
