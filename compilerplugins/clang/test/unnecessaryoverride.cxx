/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

struct Base
{
    virtual ~Base();
    virtual void f();
    void variadic(int, ...);
    void cv() const volatile;
    void ref();
    static void staticFn();
};

struct SimpleDerived : Base
{
    void
    f() override // expected-error {{public virtual function just calls public parent [loplugin:unnecessaryoverride]}}
    {
        Base::f();
    }
};

struct Intermediate1 : Base
{
};

struct MultiFunctionIntermediate2 : Base
{
    void f() override;
};

struct MultiFunctionDerived : Intermediate1, MultiFunctionIntermediate2
{
    void f() override { Intermediate1::f(); } // no warning
};

struct MultiClassIntermediate2 : Base
{
};

struct MultiClassDerived : Intermediate1, MultiClassIntermediate2
{
    void f() override { Intermediate1::f(); } // no warning
};

struct DerivedDifferent : Base
{
    void variadic(int x) { Base::variadic(x); } // no warning
    void cv() { Base::cv(); } // no warning
    void ref() && { Base::ref(); } // no warning
    void staticFn() { Base::staticFn(); } // no warning
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
