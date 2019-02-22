/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#if defined _WIN32 //TODO, see corresponding TODO in compilerplugins/clang/writeonlyvars.cxx
// expected-no-diagnostics
#else

#include <vector>
#include <ostream>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

namespace Bar
{
void test()
{
    // check that we DON'T see reads here
    // expected-error@+1 {{write m_bar3 [loplugin:writeonlyvars]}}
    int* m_bar3;
    // expected-error@+1 {{write m_bar3b [loplugin:writeonlyvars]}}
    int* m_bar3b;
    m_bar3 = nullptr;
    m_bar3b = m_bar3 = nullptr;

    // check that we see reads of field when passed to a function pointer
    // check that we see read of a field that is a function pointer
    // expected-error@+2 {{write m_bar4 [loplugin:writeonlyvars]}}
    // expected-error@+1 {{read m_bar4 [loplugin:writeonlyvars]}}
    int m_bar4;
    // expected-error@+1 {{read m_barfunctionpointer [loplugin:writeonlyvars]}}
    void (*m_barfunctionpointer)(int&) = nullptr;
    m_barfunctionpointer(m_bar4);

    // check that we see reads of a field when used in variable init
    // expected-error@+1 {{read m_bar5 [loplugin:writeonlyvars]}}
    int m_bar5 = 1;
    int x = m_bar5;
    (void)x;

    // check that we see reads of a field when used in ranged-for
    // expected-error@+1 {{read m_bar6 [loplugin:writeonlyvars]}}
    std::vector<int> m_bar6;
    for (auto i : m_bar6)
    {
        (void)i;
    }

    // check that we see writes of array fields
    // expected-error@+1 {{write m_bar7 [loplugin:writeonlyvars]}}
    int m_bar7[5];
    m_bar7[3] = 1;

    // check that we see reads when a field is used in an array expression
    // expected-error@+1 {{read m_bar8 [loplugin:writeonlyvars]}}
    int m_bar8 = 1;
    // expected-error@+1 {{read tmp [loplugin:writeonlyvars]}}
    char tmp[5];
    auto x2 = tmp[m_bar8];
    (void)x2;

    // check that we don't see reads when calling operator>>=
    // expected-error@+1 {{write m_bar9 [loplugin:writeonlyvars]}}
    sal_Int32 m_bar9;
    // expected-error@+1 {{read any [loplugin:writeonlyvars]}}
    css::uno::Any any;
    any >>= m_bar9;

    // check that we see don't see writes when calling operator<<=
    // expected-error@+1 {{read m_bar10 [loplugin:writeonlyvars]}}
    sal_Int32 m_bar10;
    // expected-error@+2 {{write any2 [loplugin:writeonlyvars]}}
    // expected-error@+1 {{read any2 [loplugin:writeonlyvars]}}
    css::uno::Any any2;
    any2 <<= m_bar10;
};
};

struct ReadOnly1
{
    ReadOnly1(int&);
};

namespace ReadOnlyAnalysis
{
void method1(int&);

void test()
{
    // check that we see a write when we pass by non-const ref
    // expected-error@+2 {{read m_f2 [loplugin:writeonlyvars]}}
    // expected-error@+1 {{write m_f2 [loplugin:writeonlyvars]}}
    int m_f2;
    method1(m_f2);

    // expected-error@+1 {{write m_f4 [loplugin:writeonlyvars]}}
    std::vector<int> m_f4;
    m_f4.push_back(1);

    // check that we see a write when we pass by non-const ref
    // expected-error@+2 {{read m_f5 [loplugin:writeonlyvars]}}
    // expected-error@+1 {{write m_f5 [loplugin:writeonlyvars]}}
    int m_f5;
    ReadOnly1 a(m_f5);

    // check that we see a write when we pass by non-const ref
    // expected-error@+2 {{read m_f6 [loplugin:writeonlyvars]}}
    // expected-error@+1 {{write m_f6 [loplugin:writeonlyvars]}}
    int m_f6;
    // expected-error@+1 {{write r [loplugin:writeonlyvars]}}
    int& r = m_f6;
    r = 1;
};
};

void ReadOnlyAnalysis3()
{
    // expected-error@+1 {{read m_f1 [loplugin:writeonlyvars]}}
    int m_f1 = 0;

    if (m_f1)
        m_f1 = 1;
};

// Verify the special logic for container fields that only contains mutations that
// add elements.
void ReadOnlyAnalysis4()
{
    // expected-error@+1 {{read m_readonly [loplugin:writeonlyvars]}}
    std::vector<int> m_readonly;
    // expected-error@+1 {{write m_writeonly [loplugin:writeonlyvars]}}
    std::vector<int> m_writeonly;
    // expected-error@+1 {{read m_readonlyCss [loplugin:writeonlyvars]}}
    css::uno::Sequence<sal_Int32> m_readonlyCss;

    // expected-error@+1 {{write x [loplugin:writeonlyvars]}}
    int x = m_readonly[0];
    (void)x;
    *m_readonly.begin() = 1; // TODO?

    m_writeonly.push_back(0);

    x = m_readonlyCss.getArray()[0];
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
