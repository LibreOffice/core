/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <memory>
#include <o3tl/deleter.hxx>
#include <o3tl/sorted_vector.hxx>

struct S1
{
    friend void test1();

private:
    S1() {}
};

void test1()
{
    // expected-error@+1 {{rather use make_shared than constructing from 'int *' [loplugin:makeshared]}}
    std::shared_ptr<int> x(new int);
    // expected-error@+1 {{rather use make_shared [loplugin:makeshared]}}
    x.reset(new int);
    // expected-error@+1 {{rather use make_shared than constructing from 'int *' [loplugin:makeshared]}}
    x = std::shared_ptr<int>(new int);

    // no warning expected
    std::shared_ptr<int> y(new int, o3tl::default_delete<int>());
    y.reset(new int, o3tl::default_delete<int>());
    // no warning expected, no public constructor
    std::shared_ptr<S1> z(new S1);
    z.reset(new S1);

    // no warning expected - this constructor takes an initializer-list, which make_shared does not support
    auto a = std::shared_ptr<o3tl::sorted_vector<int>>(new o3tl::sorted_vector<int>({ 1, 2 }));
};

void test2()
{
    // expected-error-re@+1 {{rather use make_shared than constructing from {{.*}}'unique_ptr<int>'{{.*}} [loplugin:makeshared]}}
    std::shared_ptr<int> x = std::make_unique<int>(1);
    // expected-error-re@+1 {{rather use make_shared than constructing from {{.*}}'unique_ptr<int>'{{.*}} [loplugin:makeshared]}}
    x = std::make_unique<int>(1);
    (void)x;

    // expected-error-re@+1 {{rather use make_shared than constructing from {{.*}}'unique_ptr<int>'{{.*}} [loplugin:makeshared]}}
    std::shared_ptr<int> y(std::make_unique<int>(1));
    (void)y;

    std::unique_ptr<int> u1;
    // expected-error-re@+1 {{rather use make_shared than constructing from {{.+}} (aka 'std{{.*}}::unique_ptr<int{{.*}}>') [loplugin:makeshared]}}
    std::shared_ptr<int> z = std::move(u1);
    // expected-error-re@+1 {{rather use make_shared than constructing from {{.+}} (aka 'std{{.*}}::unique_ptr<int{{.*}}>') [loplugin:makeshared]}}
    z = std::move(u1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
