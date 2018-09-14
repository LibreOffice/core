/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "unreffun.hxx"

template <typename> struct S
{
    friend void f();
};

void f() {}

void g(); // expected-error {{Unreferenced function declaration [loplugin:unreffun]}}

void h() // expected-error {{Unreferenced externally visible function definition [loplugin:unreffun]}}
{
}

void i() {}

extern void j(); // expected-error {{Unreferenced function declaration [loplugin:unreffun]}}

extern void
k() // expected-error {{Unreferenced externally visible function definition [loplugin:unreffun]}}
{
}

extern void l(); // expected-note {{first declaration is here [loplugin:unreffun]}}
void l() // expected-error {{Unreferenced externally visible function definition [loplugin:unreffun]}}
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
