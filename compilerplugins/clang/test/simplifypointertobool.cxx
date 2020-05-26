/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>

void foo();

bool test1(std::unique_ptr<int> p2)
{
    // expected-error@+1 {{simplify, drop the get() [loplugin:simplifypointertobool]}}
    if (p2.get())
        foo();
    // expected-error@+1 {{simplify, drop the get() and wrap the expression in a functional cast to bool [loplugin:simplifypointertobool]}}
    bool b1 = p2.get();
    //TODO:
    bool b2 = ( // deliberately spread across multiple lines
        p2.get());
    return b1 && b2;
}

void test2(std::shared_ptr<int> p)
{
    // expected-error@+1 {{simplify, drop the get() [loplugin:simplifypointertobool]}}
    if (p.get())
        foo();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
