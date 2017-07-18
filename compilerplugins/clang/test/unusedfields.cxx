/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vector>
#include <ostream>

struct Foo
// expected-error@-1 {{read m_foo1 [loplugin:unusedfields]}}
{
    int m_foo1;
};

struct Bar
// expected-error@-1 {{read m_bar2 [loplugin:unusedfields]}}
// expected-error@-2 {{read m_bar4 [loplugin:unusedfields]}}
// expected-error@-3 {{read m_bar5 [loplugin:unusedfields]}}
// expected-error@-4 {{read m_bar6 [loplugin:unusedfields]}}
// expected-error@-5 {{read m_barfunctionpointer [loplugin:unusedfields]}}
// expected-error@-6 {{read m_bar8 [loplugin:unusedfields]}}
// expected-error@-7 {{write m_bar1 [loplugin:unusedfields]}}
// expected-error@-8 {{write m_bar2 [loplugin:unusedfields]}}
// expected-error@-9 {{write m_bar3 [loplugin:unusedfields]}}
// expected-error@-10 {{write m_bar3b [loplugin:unusedfields]}}
// expected-error@-11 {{write m_bar4 [loplugin:unusedfields]}}
// expected-error@-12 {{write m_bar7 [loplugin:unusedfields]}}
// expected-error@-13 {{write m_barfunctionpointer [loplugin:unusedfields]}}
{
    int  m_bar1;
    int  m_bar2 = 1;
    int* m_bar3;
    int* m_bar3b;
    int  m_bar4;
    void (*m_barfunctionpointer)(int&);
    int  m_bar5;
    std::vector<int> m_bar6;
    int m_bar7[5];
    int m_bar8;
    int m_barstream;

    // check that we see reads of fields like m_foo1 when referred to via constructor initializer
    Bar(Foo const & foo) : m_bar1(foo.m_foo1) {}

    // check that we don't see reads when inside copy/move constructor
    Bar(Bar const & other) { m_bar3 = other.m_bar3; }

    // check that we don't see reads when inside copy/move assignment operator
    Bar& operator=(Bar const & other) { m_bar3 = other.m_bar3; return *this; }

    // check that we DONT see reads here
    int bar2() { return m_bar2; }

    // check that we DONT see reads here
    void bar3()
    {
        m_bar3 = nullptr;
        m_bar3b = m_bar3 = nullptr;
    }

    // check that we see reads of field when passed to a function pointer
    // check that we see read of a field that is a function pointer
    void bar4() { m_barfunctionpointer(m_bar4); }

    // check that we see reads of a field when used in variable init
    void bar5() { int x = m_bar5; (void) x; }

    // check that we see reads of a field when used in ranged-for
    void bar6() { for (auto i : m_bar6) { (void)i; } }

    // check that we see don't see reads of array fields
    void bar7() { m_bar7[3] = 1; }

    // check that we see reads when a field is used in an array expression
    char bar8()
    {
        char tmp[5];
        return tmp[m_bar8];
    }
};

// check that we __dont__ see a read of m_barstream
std::ostream& operator<<(std::ostream& s, Bar const & bar)
{
    s << bar.m_barstream;
    return s;
};

struct ReadOnly1 { ReadOnly1(int&); };

struct ReadOnlyAnalysis
// expected-error@-1 {{read m_f2 [loplugin:unusedfields]}}
// expected-error@-2 {{read m_f3 [loplugin:unusedfields]}}
// expected-error@-3 {{read m_f4 [loplugin:unusedfields]}}
// expected-error@-4 {{read m_f5 [loplugin:unusedfields]}}
// expected-error@-5 {{write m_f2 [loplugin:unusedfields]}}
// expected-error@-6 {{write m_f3 [loplugin:unusedfields]}}
// expected-error@-7 {{write m_f4 [loplugin:unusedfields]}}
// expected-error@-8 {{write m_f5 [loplugin:unusedfields]}}
{
    int m_f1;
    int m_f2;
    int m_f3;
    std::vector<int> m_f4;
    int m_f5;

    // check that we dont see a write of m_f1
    ReadOnlyAnalysis() : m_f1(0) {}

    void method1(int&);

    // check that we see a write when we pass by non-const ref
    void method2() { method1(m_f2); }

    int& method3() { return m_f3; }

    void method4() { m_f4.push_back(1); }

    // check that we see a write when we pass by non-const ref
    void method5() { ReadOnly1 a(m_f5); }
};

struct ReadOnlyAnalysis2
// expected-error@-1 {{write m_r2f1 [loplugin:unusedfields]}}
{
    int m_r2f1;
};

ReadOnlyAnalysis2 global { 1 };

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
