/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

struct S1 { virtual ~S1(); };
struct S2 final: S1 {};
struct S3: S1 {};

void f(S1 * s1, S2 * s2) {
    (void) dynamic_cast<S2 *>(s1);
    (void) dynamic_cast<S1 *>(s2);
    (void) dynamic_cast<S2 *>(s2);
    (void) dynamic_cast<S3 *>(s2); // expected-error {{dynamic_cast from 'S2 *' to 'S3 *' always fails [loplugin:faileddyncast]}}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
