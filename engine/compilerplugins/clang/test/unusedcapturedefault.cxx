/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

auto f1()
{
    // expected-error@+1 {{unused capture-default [loplugin:unusedcapturedefault]}}
    return [=] { return 0; };
}

auto f2()
{
    // expected-error@+1 {{unused capture-default [loplugin:unusedcapturedefault]}}
    return [=](auto x) { return x; };
}

template <typename T> auto f3(T x)
{
    return [=]() { return x; };
}

#define MACRO1(x) [&] { return x; }

auto f4() {
    int n = 0;
    return MACRO1(n)();
}

auto f5() { return MACRO1(0)(); }

#define MACRO2(x) (x)

auto f6() {
    // expected-error@+1 {{unused capture-default [loplugin:unusedcapturedefault]}}
    return MACRO2([=] { return 0; })();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
