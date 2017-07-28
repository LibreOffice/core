/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

struct Class1
{
    int const * m_f1;
    Class1(int * f1) : m_f1(f1) {} // expected-error {{this parameter can be const [loplugin:constparams]}}
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
