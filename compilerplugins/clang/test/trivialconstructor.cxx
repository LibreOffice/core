/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

struct S1
{
    // expected-error@+1 {{no need for explicit constructor decl [loplugin:trivialconstructor]}}
    S1() {}
};

struct S2
{
    S2() {}
    S2(int) {}
};

struct S3
{
    S3() {}
    template <typename T> S3(T);
};

template <typename> struct S4
{
#if !defined _MSC_VER
// expected-error@+2 {{no need for explicit constructor decl [loplugin:trivialconstructor]}}
#endif
    S4() {}
};

template <typename> struct S5
{
    S5() {}
    S5(int);
};

template <typename> struct S6
{
    S6() {}
    template <typename T> S6(T);
};

struct S7
{
    S7(int = 0) {}
};

struct S8
{
    template <typename T> S8(T = 0) {}
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
