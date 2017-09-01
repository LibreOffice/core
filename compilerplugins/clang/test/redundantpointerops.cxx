/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

struct Struct1 {
    int x;
};

void function1(Struct1& s)
{
    (&s)->x = 1; // expected-error {{'&' followed by '->', rather use '.' [loplugin:redundantpointerops]}}
};

struct Struct2 {
    int x;
    Struct2* operator&() { return this; }
};

void function2(Struct2 s)
{
    (&s)->x = 1; // expected-error {{'&' followed by '->', rather use '.' [loplugin:redundantpointerops]}}
};

void function3(Struct1& s)
{
    (*(&s)).x = 1; // expected-error {{'&' followed by '*', rather use '.' [loplugin:redundantpointerops]}}
};

//void function4(Struct1* s)
//{
//    (*s).x = 1; // xxexpected-error {{'*' followed by '.', rather use '->' [loplugin:redundantpointerops]}}
//};
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
