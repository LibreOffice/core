/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

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

#if 0
    if (x)
        foo();
    else
#endif
        foo();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
