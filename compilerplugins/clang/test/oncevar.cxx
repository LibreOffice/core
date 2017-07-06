/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>

/*int foo() { return 1; }*/

void call_value(int);
void call_const_ref(int const &);
void call_ref(int &);
void call_value(OUString);
void call_const_ref(OUString const &);
void call_ref(OUString &);

template<typename T> void f() {
    int i = sizeof (T) + 1; // expected-error {{var used only once, should be inlined or declared const [loplugin:oncevar]}}
    call_value(i); // expected-note {{used here [loplugin:oncevar]}}
}

int main() {
/* TODO
    int i;
    int x = 2;
    if ( (i = foo()) == 0 ) {
        x = 1;
    }
*/


    int i1 = 2; // expected-error {{var used only once, should be inlined or declared const [loplugin:oncevar]}}
    call_value(i1); // expected-note {{used here [loplugin:oncevar]}}
    int i2 = 2; // expected-error {{var used only once, should be inlined or declared const [loplugin:oncevar]}}
    call_const_ref(i2); // expected-note {{used here [loplugin:oncevar]}}

    // don't expect warnings here
    int i3;
    call_ref(i3);
    int const i4 = 2;
    call_value(i4);

    OUString s1("xxx"); // expected-error {{var used only once, should be inlined or declared const [loplugin:oncevar]}}
    call_value(s1); // expected-note {{used here [loplugin:oncevar]}}
    OUString s2("xxx"); // expected-error {{var used only once, should be inlined or declared const [loplugin:oncevar]}}
    call_const_ref(s2); // expected-note {{used here [loplugin:oncevar]}}

    // don't expect warnings here
    OUString s3;
    call_ref(s3);
    OUString const s4("xxx");
    call_value(s4);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
