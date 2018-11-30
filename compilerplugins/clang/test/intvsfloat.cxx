/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

static const float PI = 3.4;
struct Class1
{
    float getFloat() const { return 1.5; }
    int getInt() const { return 1; }
    static constexpr float PI = 3.4;
    static constexpr float E() { return 3.4; }
};

void func1(Class1 const& class1)
{
    // expected-error@+1 {{comparing integer to float constant, can never be true [loplugin:intvsfloat]}}
    if (1 == PI)
        return;
    // expected-error@+1 {{comparing integer to float constant, can never be true [loplugin:intvsfloat]}}
    if (1 == class1.PI)
        return;
    // expected-error@+1 {{comparing integer to float constant, can never be true [loplugin:intvsfloat]}}
    if (true == class1.PI)
        return;
    if (1 == class1.getInt()) // no warning expected
        return;
    // expected-error@+1 {{comparing integer to float constant, can never be true [loplugin:intvsfloat]}}
    if (1 == class1.E())
        return;
    // expected-error@+1 {{comparing integer to float constant, can never be true [loplugin:intvsfloat]}}
    if (true == class1.E())
        return;
    if (1 == class1.getFloat()) // no warning expected
        return;
}

void func2(Class1 const& class1)
{
    // expected-error@+1 {{assigning constant float value to int truncates data [loplugin:intvsfloat]}}
    int i0 = PI;
    (void)i0;
    // expected-error@+1 {{assigning constant float value to int truncates data [loplugin:intvsfloat]}}
    int i1 = class1.PI;
    (void)i1;
    // expected-error@+1 {{assigning constant float value to int truncates data [loplugin:intvsfloat]}}
    int i2 = class1.E();
    (void)i2;
    int i3 = class1.getFloat(); // no warning expected
    (void)i3;
    int i4 = class1.getInt(); // no warning expected
    (void)i4;
    // expected-error@+1 {{assigning constant float value to int truncates data [loplugin:intvsfloat]}}
    bool b1 = class1.E();
    (void)b1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
