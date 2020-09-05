/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>

void test1()
{
    int i = 2; // expected-error {{can reduce scope of var [loplugin:reducevarscope]}}
    {
        i = 3; // expected-note {{used here [loplugin:reducevarscope]}}
    }
    int j = 2; // expected-error {{can reduce scope of var [loplugin:reducevarscope]}}
    {
        j = 3; // expected-note {{used here [loplugin:reducevarscope]}}
        {
            j = 4; // expected-note {{used here [loplugin:reducevarscope]}}
        }
    }
}

// negative test - seen inside a loop
void test2()
{
    int i = 2;
    for (int j = 1; j < 10; ++j)
    {
        i = 3;
    }
}

// negative test - initial assignment from non-constant
void test3()
{
    int j = 1;
    int i = j;
    {
        i = 3;
    }
}

// negative test
void test4()
{
    int i = 2;
    {
        i = 3;
    }
    i = 4;
}

// negative test
void test5()
{
    int i = 2;
    i = 3;
}

// negative test - seen in 2 child blocks
void test6()
{
    int i;
    {
        i = 3;
    }
    {
        i = 3;
    }
}

// TODO negative test - storing pointer to OUString data
// void test7()
// {
// OUString s;
// const sal_Unicode* p = nullptr;
// {
// p = s.getStr();
// }
// auto p2 = p;
// (void)p2;
// }

// negative test - passing var into lambda
void test8()
{
    int i;
    auto l1 = [&]() { i = 1; };
    (void)l1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
