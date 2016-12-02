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
    OUString mv;

    // request from vmiklos: make sure we ignore cases where the passed in parameter is std::move'd
    S(OUString v)
      : mv(std::move(v)) {}
};


void f() // expected-error {{Unreferenced externally visible function definition [loplugin:unreffun]}}
{
    S* s;
    OUString v;
    s = new S(v);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
