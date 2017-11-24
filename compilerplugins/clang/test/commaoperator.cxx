/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

bool f();

struct S { ~S(); };

int main() {
    f(), f(), f(); // expected-error {{comma operator hides code [loplugin:commaoperator]}}
    (f(), f());
    for (
        f(), f();
        f(), f(); // expected-error {{comma operator hides code [loplugin:commaoperator]}}
        f(), f())
        f(), f(); // expected-error {{comma operator hides code [loplugin:commaoperator]}}
    S s;
    (s = S(), s = S(), s = S());
    for (s = S(), f(); f(); s = S(), f()) {}
    while (s = S(), f()) {}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
