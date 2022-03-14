/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

struct S1
{
    // expected-note@+1 {{previous declaration is here [loplugin:trivialdestructor]}}
    ~S1();
};

// expected-error@+1 {{no need for explicit destructor decl [loplugin:trivialdestructor]}}
S1::~S1() {}

struct S2
{
    // expected-note@+1 {{previous declaration is here [loplugin:trivialdestructor]}}
    ~S2();
};

// expected-error@+1 {{no need for explicit destructor decl [loplugin:trivialdestructor]}}
S2::~S2() = default;

struct S3
{
    ~S3() = delete;
};

struct S4
{
    union {
        int i;
        float f;
    };
    // expected-error@+1 {{no need for explicit destructor decl [loplugin:trivialdestructor]}}
    ~S4() {}
};

struct Nontrivial
{
    ~Nontrivial();
};

struct S5
{
    union {
        int i;
        Nontrivial n;
    };
    ~S5() {}
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
