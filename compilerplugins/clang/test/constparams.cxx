/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>

struct Class1
{
    int const * m_f1;
    Class1(int * f1) : m_f1(f1) {} // expected-error {{this parameter can be const Class1::Class1 [loplugin:constparams]}}
};

struct Class2
{
    int * m_f2;
    Class2(int * f2) : m_f2(f2) {}
};
struct Class3
{
    int * m_f2;
    Class3(void * f2) : m_f2(static_cast<int*>(f2)) {}
};

int const * f1(int *); // expected-note {{canonical parameter declaration here [loplugin:constparams]}}
int const * f2(int *);
int const * f3(int *);
void g() {
    int const * (*p1)(int *);
    int n = 0;
    f1(&n);
    p1 = f2;
    typedef void (*P2)();
    P2 p2;
    p2 = (P2) (f3);
}
int const * f1(int * p) { // expected-error {{this parameter can be const f1 [loplugin:constparams]}}
    return p;
}
void f4(std::string * p) {
    *p = std::string("xxx");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
