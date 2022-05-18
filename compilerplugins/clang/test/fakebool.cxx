/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <sal/types.h>

namespace {

struct S {
    sal_Bool b; // expected-error {{FieldDecl, use "bool" instead of 'sal_Bool' (aka 'unsigned char') [loplugin:fakebool]}}
};

struct S2 {
    sal_Bool & b_;
    // The following should arguably not warn, but currently does (and does find cases that actually
    // can be cleaned up; if it ever produces false warnings for cases that cannot, we need to fix
    // it):
    S2(sal_Bool & b): // expected-error {{ParmVarDecl, use "bool" instead of 'sal_Bool' (aka 'unsigned char') [loplugin:fakebool]}}
        b_(b) {}
};

}

struct S3 {
    sal_Bool b_;
    void f() { S2 s(b_); }
};

namespace {

struct S4 {
    sal_Bool b;
};

}

void f() {
    sal_Bool b;
    (void) &b;
    (void) &S4::b;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
