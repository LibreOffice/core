/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>

struct S {
    OUString mv1;
    OUString mv2;

    // make sure we ignore cases where the passed in parameter is std::move'd
    S(OUString v1, OUString v2)
      : mv1(std::move(v1)), mv2((std::move(v2))) {}
};


void f()
{
    S* s;
    OUString v1, v2;
    s = new S(v1, v2);
}

struct S2 { S2(int); };

S2 f2() {
    static int n;
    return n;
}

// expected-no-diagnostics

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
