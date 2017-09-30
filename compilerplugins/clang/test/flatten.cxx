/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <exception>

extern int foo();
extern int bar();
class Class {};

void top1() {
    if (foo() == 1) { // expected-note {{if condition here [loplugin:flatten]}}
        Class aClass;
        (void)aClass;
    } else {
        throw std::exception(); // expected-error {{unconditional throw in else branch, rather invert the condition, throw early, and flatten the normal case [loplugin:flatten]}}
    }
}

void top2() {
    if (foo() == 2) {
        throw std::exception(); // expected-error {{unconditional throw in then branch, just flatten the else [loplugin:flatten]}}
    } else {
        Class aClass;
        (void)aClass;
    }
}

void top3() {
    // no warning expected
    if (foo() == 2) {
        throw std::exception();
    } else {
        Class aClass;
        (void)aClass;
    }
    int x = 1;
    (void)x;
}

void top4() {
    // no warning expected
    if (foo() == 2) {
        Class aClass;
        (void)aClass;
    } else {
        throw std::exception();
    }
    int x = 1;
    (void)x;
}

void top5() {
#if 1
    if (foo() == 2) {
        if (foo() == 3) { // expected-note {{if condition here [loplugin:flatten]}}
            bar();
        } else {
            throw std::exception(); // expected-error {{unconditional throw in else branch, rather invert the condition, throw early, and flatten the normal case [loplugin:flatten]}}
        }
    } else
#endif
        throw std::exception(); // no warning expected
}

int main() {
    // no warning expected
    if (bar() == 3) {
        throw std::exception();
    } else {
        throw std::exception();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
