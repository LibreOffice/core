/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>

class Class1
{
    void SAL_CALL method1(); // xxexpected-error {{SAL_CALL unnecessary here [loplugin:salcall]}}
};
void SAL_CALL Class1::method1() {
} // xxexpected-error {{SAL_CALL unnecessary here [loplugin:salcall]}}

class Class2
{
    void method1(); // expected-error {{SAL_CALL inconsistency [loplugin:salcall]}}
};
void SAL_CALL Class2::method1() {} // expected-note {{SAL_CALL inconsistency [loplugin:salcall]}}

// no warning, this appears to be legal
class Class3
{
    void SAL_CALL method1();
};
void Class3::method1() {}

// no warning, normal case for reference
class Class4
{
    void method1();
};
void Class4::method1() {}

class Class5_1
{
    virtual void method1(); // expected-note {{SAL_CALL inconsistency [loplugin:salcall]}}
    virtual ~Class5_1();
};
class Class5_2
{
    virtual void SAL_CALL method1();
    virtual ~Class5_2();
};
class Class5_3 : public Class5_1, public Class5_2
{
    virtual void SAL_CALL
    method1() override; // expected-error {{SAL_CALL inconsistency [loplugin:salcall]}}
    virtual ~Class5_3();
};

class Class6_1
{
    virtual void SAL_CALL method1();
    virtual ~Class6_1();
};
class Class6_2
{
    virtual void SAL_CALL method1();
    virtual ~Class6_2();
};
class Class6_3 : public Class6_1, public Class6_2
{
    virtual void SAL_CALL method1() override;
    virtual ~Class6_3();
};

class Class7_1
{
    virtual void method1();
    virtual ~Class7_1();
};
class Class7_2
{
    virtual void method1();
    virtual ~Class7_2();
};
class Class7_3 : public Class7_1, public Class7_2
{
    virtual void method1() override;
    virtual ~Class7_3();
};

class Class8_1
{
    virtual void method2();
    virtual ~Class8_1();
};
class Class8_2
{
    virtual void SAL_CALL method2(); // expected-note {{SAL_CALL inconsistency [loplugin:salcall]}}
    virtual ~Class8_2();
};
class Class8_3 : public Class8_1, public Class8_2
{
    virtual void method2() override; // expected-error {{SAL_CALL inconsistency [loplugin:salcall]}}
    virtual ~Class8_3();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
