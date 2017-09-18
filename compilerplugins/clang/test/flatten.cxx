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

int main() {
    if (foo() == 1) { // expected-note {{if condition here [loplugin:flatten]}}
    } else {
        throw std::exception(); // expected-error {{unconditional throw in else branch, rather invert the condition, throw early, and flatten the normal case [loplugin:flatten]}}
    }

    // no warning expected
    if (bar() == 3) {
        throw std::exception();
    } else {
        throw std::exception();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
