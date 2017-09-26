/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>
#include <fstream>

void foo();

int main()
{
    try {
        foo();
    } catch(int const &) {  // expected-error {{unnecessary catch and throw [loplugin:unnecessarycatchthrow]}}
        throw;
    }
    try {
        foo();
    } catch(int const & ex) {  // expected-error {{unnecessary catch and throw [loplugin:unnecessarycatchthrow]}}
        throw ex;
    }
    try {
        foo();
    } catch(int const &) {
        std::cout << "test";
        throw;
    }

}

void test1()
{
    // cannot remove catch/throw where the throw is of a non-final class
    struct B {};
    struct D: B {};
    try {
        throw D();
    } catch (B & b) {
        throw b; // must not be removed
    }
};

void test2()
{
    struct F final {};
    try {
        throw F();
    } catch (F const & f) { // expected-error {{unnecessary catch and throw [loplugin:unnecessarycatchthrow]}}
        throw f;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
