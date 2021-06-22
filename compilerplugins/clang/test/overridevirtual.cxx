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
    // expected-note@+1 {{overridden declaration is here [loplugin:overridevirtual]}}
    virtual ~S1();
};

struct S2 : S1
{
    // expected-error@+1 {{overriding virtual function declaration not marked 'override' [loplugin:overridevirtual]}}
    ~S2();
};

template <typename> struct T1
{
    virtual ~T1();
};

template <typename T> struct T2 : T1<T>
{
    ~T2();
};

template <typename> struct U1
{
    // expected-note@+1 {{overridden declaration is here [loplugin:overridevirtual]}}
    virtual ~U1();
};

template <typename T> struct U2 : U1<int>
{
    // expected-error@+1 {{overriding virtual function declaration not marked 'override' [loplugin:overridevirtual]}}
    ~U2();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
