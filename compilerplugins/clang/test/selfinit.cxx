/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <typeinfo>

extern int i;
int i = i;
// expected-error@-1 {{referencing a variable during its own initialization is error-prone and thus suspicious [loplugin:selfinit]}}
// expected-note@-2 {{variable declared here [loplugin:selfinit]}}

int j = [](int n) { return j + n; }(0);
// expected-error@-1 {{referencing a variable during its own initialization is error-prone and thus suspicious [loplugin:selfinit]}}
// expected-note@-2 {{variable declared here [loplugin:selfinit]}}

int k = sizeof k;

int f(std::type_info const&);

int l = f(typeid(l));

bool m = noexcept(m);

template <typename T> int g();

int n = g<decltype(n)>();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
