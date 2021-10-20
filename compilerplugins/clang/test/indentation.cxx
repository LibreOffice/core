/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config_clang.h"

int foo();
int foo2(int);

#define SOME_MACRO(x) foo2(x)

void top1(int x) {
    {
        foo(); // expected-note {{measured against this one [loplugin:indentation]}}
         foo(); // expected-error {{statement mis-aligned compared to neighbours  [loplugin:indentation]}}
    }
    {
        foo(); // expected-note {{measured against this one [loplugin:indentation]}}
         SOME_MACRO(1); // expected-error {{statement mis-aligned compared to neighbours SOME_MACRO [loplugin:indentation]}}
    }
    // no warning expected
    {
        foo(); foo();
    }
    // no warning expected
    /*xxx*/ foo();


// disable this for now, ends up touching some very large switch statements in sw/ and sc/
    switch (x)
    {
        case 1: foo(); break; // 1expected-note {{measured against this one [loplugin:indentation]}}
         case 2: foo(); break; // 1expected-error {{statement mis-aligned compared to neighbours [loplugin:indentation]}}
    };


    if (x)
    foo(); // expected-error {{if body should be indented [loplugin:indentation]}}

    if (x)
    {
        foo();
    }

    if (x)
    { // expected-note {{start brace here [loplugin:indentation]}}
        foo();
     } // expected-error {{start and end brace not aligned [loplugin:indentation]}}

    if (x) // expected-note {{statement beginning here [loplugin:indentation]}}
     { // expected-error {{start brace not aligned with beginning of parent statement [loplugin:indentation]}}
        foo();
     }

    if (x)
        ;
    else
    foo(); // expected-error {{else body should be indented [loplugin:indentation]}}

    if (x)
        ;
    else
    {
        foo();
    }

    if (x)
        ;
     else  // expected-error {{if and else not aligned [loplugin:indentation]}}
        foo();

    if (x)
    {
    } else
    {
        foo();
    }

#if 0
    if (x)
        foo();
    else
#endif
        foo();
}

void attr() {
    [[maybe_unused]] int i = foo();
    foo();
}

#if CLANG_VERSION >= 100000
void attr_bad() {
    [[maybe_unused]] int i = foo(); // expected-note {{measured against this one [loplugin:indentation]}}
     foo(); // expected-error {{statement mis-aligned compared to neighbours  [loplugin:indentation]}}
}
#endif

void xxx();
void test5(bool x)
{
    if (x)
    {
    xxx(); // expected-error {{body inside brace not indented [loplugin:indentation]}}
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
