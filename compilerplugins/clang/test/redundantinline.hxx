/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

struct S1 {
    inline S1();
    inline ~S1();
};

S1::S1() = default;

struct S2 {
    inline S2() = default; // expected-error {{[loplugin:redundantinline]}}
    inline ~S2() = default; // expected-error {{[loplugin:redundantinline]}}
};

struct S3 {
    inline S3();
    inline ~S3();

    inline void f1();

    static inline void f2();

    inline void operator +();

    inline operator int();

    friend inline void f3();
};

S3::S3() {}

S3::~S3() { f1(); }

void S3::f1() { (void)this; }

void S3::f2() {}

void S3::operator +() {}

void f3() {}

S3::operator int() { return 0; }

struct S4 {
    inline S4() {} // expected-error {{function definition redundantly declared 'inline' [loplugin:redundantinline]}}
    inline ~S4() { f1(); } // expected-error {{function definition redundantly declared 'inline' [loplugin:redundantinline]}}

    inline void f1() { (void)this; } // expected-error {{function definition redundantly declared 'inline' [loplugin:redundantinline]}}

    static inline void f2() {} // expected-error {{function definition redundantly declared 'inline' [loplugin:redundantinline]}}

    inline void operator +() {} // expected-error {{function definition redundantly declared 'inline' [loplugin:redundantinline]}}

    inline operator int() { return 0; } // expected-error {{function definition redundantly declared 'inline' [loplugin:redundantinline]}}

    friend inline void f4() {} // expected-error {{function definition redundantly declared 'inline' [loplugin:redundantinline]}}

    static constexpr int f5() { return 0; }

    static constexpr inline int f6() { return 0; } // expected-error {{function definition redundantly declared 'inline' [loplugin:redundantinline]}}
};

constexpr int f5() { return 0; }

constexpr inline int f6() { return 0; } // expected-error {{function definition redundantly declared 'inline' [loplugin:redundantinline]}}

static inline int f7() { return 0; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
