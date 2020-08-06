/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

struct S {
    void f1();
    void f2() const;
    void f3();
    void f3() const;
};

int && nix();
int const && cix();
int nir();
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wignored-qualifiers"
int const cir();
#pragma clang diagnostic pop

S && nsx();
S const && csx();
S nsr();
S const csr();

void testArithmeticTypedefs();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
