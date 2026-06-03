/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <array>
#include <atomic>
#include <initializer_list>

#include <com/sun/star/uno/Sequence.hxx>
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

void h(bool);

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
    // expected-error@+1 {{implicit conversion (IntegralCast) from 'bool' to 'const int' [loplugin:implicitboolconversion]}}
    Sequence<int> s3{ false };
    (void)s3;
    // expected-error@+1 {{implicit conversion (IntegralCast) from 'bool' to 'const int' [loplugin:implicitboolconversion]}}
    Sequence<Sequence<int>> s4{ { false } };
    (void)s4;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
