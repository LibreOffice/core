/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <atomic>
#include <initializer_list>

#include <sal/types.h>

template <typename T> struct Sequence
{
    Sequence(std::initializer_list<T>);
};

template <typename T> struct Wrap1
{
    T element;
};

template <typename T> struct Wrap2
{
    Wrap2(T const& e)
        : element(e)
    {
    }
    T element;
};

bool g();

void f()
{
    // expected-error@+1 {{implicit conversion (IntegralCast) from 'bool' to 'int' [loplugin:implicitboolconversion]}}
    int i = false;
    // expected-error@+1 {{implicit conversion (IntegralCast) from 'bool' to 'int' [loplugin:implicitboolconversion]}}
    i = true;
    (void)i;
    std::atomic<bool> b = false;
    (void)b;
    //TODO: Emit only one diagnostic here:
    // expected-error@+2 {{implicit conversion (ConstructorConversion) from 'bool' to 'std::atomic<int>' [loplugin:implicitboolconversion]}}
    // expected-error-re@+1 {{implicit conversion (IntegralCast) from 'bool' to {{.+}} [loplugin:implicitboolconversion]}}
    std::atomic<int> a = false;
    (void)a;
    bool b2 = true;
    b2 &= g();
    (void)b2;
    Sequence<sal_Bool> s1{ false };
    (void)s1;
    Sequence<Sequence<sal_Bool>> s2{ { false } };
    (void)s2;
    // expected-error@+1 {{implicit conversion (IntegralCast) from 'bool' to 'const int' [loplugin:implicitboolconversion]}}
    Sequence<sal_Int32> s3{ false };
    (void)s3;
    // expected-error@+1 {{implicit conversion (IntegralCast) from 'bool' to 'const int' [loplugin:implicitboolconversion]}}
    Sequence<Sequence<sal_Int32>> s4{ { false } };
    (void)s4;
    Wrap1<sal_Bool> w1{ false };
    (void)w1;
    Sequence<Wrap1<sal_Bool>> s5{ { false } };
    (void)s5;
    Wrap2<sal_Bool> w2{ false };
    (void)w2;
    Sequence<Wrap2<sal_Bool>> s6{ { false } };
    (void)s6;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
