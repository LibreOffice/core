/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

int f(bool b1, bool b2) {
    if (b1 || b2) { // no warning
#if 0
        if (b1)
#endif
        { // expected-error {{block directly inside block [loplugin:blockblock]}}
            { // expected-note {{inner block here [loplugin:blockblock]}}
                return 0;
            }
        }
    }
    return 1;
}

int main() { // expected-error {{block directly inside block [loplugin:blockblock]}}
    { // expected-note {{inner block here [loplugin:blockblock]}}
        int x = 1;
        (void)x;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
