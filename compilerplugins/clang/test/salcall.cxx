/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>

#define VOID void

class Class1
{
    SAL_CALL Class1() {} // expected-error {{SAL_CALL unnecessary here [loplugin:salcall]}}
    SAL_CALL ~Class1() {} // expected-error {{SAL_CALL unnecessary here [loplugin:salcall]}}
    SAL_CALL operator int() // expected-error {{SAL_CALL unnecessary here [loplugin:salcall]}}
    {
        return 0;
    }

    void SAL_CALL method1(); // expected-error {{SAL_CALL unnecessary here [loplugin:salcall]}}
    VOID method2() {}
    // no SAL_CALL for above method2, even though "SAL_CALL" appears between definition of VOID and
    // the declaration's name, "method2"
};
void SAL_CALL Class1::method1()
{ // expected-error@-1 {{SAL_CALL unnecessary here [loplugin:salcall]}}
}

class Class2
{
    void method1(); // expected-note {{SAL_CALL inconsistency [loplugin:salcall]}}
};
void SAL_CALL Class2::method1() {} // expected-error {{SAL_CALL inconsistency [loplugin:salcall]}}

// comment this out because it seems to generate a warning in some internal buffer of clang that I can't annotate
#if 0
// no warning, this appears to be legal
class Class3
{
    void SAL_CALL method1(); // expected-error {{SAL_CALL unnecessary here [loplugin:salcall]}}
};
void Class3::method1() {}
#endif

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

#define M1(m) VOID m
class Class9
{
    Class9(); // expected-note {{SAL_CALL inconsistency [loplugin:salcall]}}
    M1(method1)(); // expected-note {{SAL_CALL inconsistency [loplugin:salcall]}}
    void method2(); // expected-note {{SAL_CALL inconsistency [loplugin:salcall]}}
};
#define MC(num)                                                                                    \
    Class##num::Class##num() {}
SAL_CALL MC(9) // expected-error {{SAL_CALL inconsistency [loplugin:salcall]}}
    ; // to appease clang-format
void SAL_CALL Class9::method1() // expected-error {{SAL_CALL inconsistency [loplugin:salcall]}}
{
}
#define M2(T) T SAL_CALL
M2(void) Class9::method2() {} // expected-error {{SAL_CALL inconsistency [loplugin:salcall]}}

void SAL_CALL f0() {} // expected-error {{SAL_CALL unnecessary here [loplugin:salcall]}}

void SAL_CALL f1() {}

void SAL_CALL f2() {}

void SAL_CALL f3() {}

void SAL_CALL f4() {}

typedef void SAL_CALL (*Ptr)();

void takePtr(Ptr);

void usePtr()
{
    f0();
    takePtr(f1);
    takePtr(&f2);
    Ptr p = f3;
    takePtr(p);
    p = f4;
    takePtr(p);
}

#if 0 // see TODO in SalCall::isSalCallFunction
class Class10
{
    void method1();
};
#define M3(T, SAL_CALL) T SAL_CALL::
M3(void, Class10) method1() {} // false "SAL_CALL inconsistency"
#endif

#if 0 //TODO
template<typename> struct S {
    virtual ~S();
    virtual void f();
};
template<typename T> S<T>::~S() {}
template<typename T> void S<T>::f() {}
struct S2: S<int> {
    ~S2();
    void f() {}
};
int main() {
    S2 s2;
    s2->f();
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
