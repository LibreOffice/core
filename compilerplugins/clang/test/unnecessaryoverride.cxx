/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <config_clang.h>

struct Base
{
    virtual ~Base();
    virtual void f();
    void variadic(int, ...);
    void cv() const volatile;
    void ref();
    static void staticFn();
    void defaults(void* = nullptr, int = 0, double = 1, Base const& = {}, char const* = "foo");
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
    void defaults(void* x1, int x2, double x3, Base const& x4, char const* x5)
    {
        Base::defaults(x1, x2, x3, x4, x5); // no warning
    }
};

struct DerivedSame : Base
{
#if CLANG_VERSION >= 30900 // cf. corresponding condition in Plugin::checkIdenticalDefaultArguments
    void
    defaults( // expected-error {{public function just calls public parent [loplugin:unnecessaryoverride]}}
        void* x1 = 0, int x2 = (1 - 1), double x3 = 1.0, Base const& x4 = (Base()),
        char const* x5 = "f"
                         "oo")
    {
        Base::defaults(x1, x2, x3, x4, x5);
    }
#endif
};

struct DerivedSlightlyDifferent : Base
{
    void defaults( // no warning
        void* x1 = nullptr, int x2 = 0, double x3 = 1, Base const& x4 = DerivedSlightlyDifferent(),
        char const* x5 = "foo")
    {
        Base::defaults(x1, x2, x3, x4, x5);
    }
};

struct Base2
{
    void default1(Base const& = SimpleDerived());
    void default2(Base const& = SimpleDerived());
    void default3(Base = Base());
};

struct Derived2 : Base2
{
    void default1(Base const& x = Intermediate1()) { Base2::default1(x); } // no warning
    void
    default2( // expected-error {{public function just calls public parent [loplugin:unnecessaryoverride]}}
        Base const& x = SimpleDerived())
    {
        Base2::default2(x);
    }
    void
    default3( // expected-error {{public function just calls public parent [loplugin:unnecessaryoverride]}}
        Base x = Base())
    {
        (Base2::default3(x));
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
