/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

namespace test1
{
class Mapping
{
    char* m_pMapping;

    // expected-error@+1 {{move constructor can be noexcept [loplugin:noexceptmove]}}
    Mapping(Mapping&& other)
        : m_pMapping(other.m_pMapping)
    {
        other.m_pMapping = nullptr;
    }

    // expected-error@+1 {{move operator= can be noexcept [loplugin:noexceptmove]}}
    Mapping& operator=(Mapping&& other)
    {
        m_pMapping = other.m_pMapping;
        other.m_pMapping = nullptr;
        return *this;
    }
};
};

// No warning expected, because calling throwing function.
namespace test2
{
void foo() noexcept(false);

class Bar
{
    Bar(Bar&&) { foo(); }
};
};

// no warning expected, because calling throwing constructor
namespace test3
{
struct Foo
{
    Foo() noexcept(false);
};
class Bar
{
    Bar(Bar&&) { Foo aFoo; }
};

class Bar2
{
    Foo m_foo;

    Bar2(Bar2&&) {}
};
};

// No warning expected, because calling throwing destructor.
namespace test4
{
struct Foo
{
    ~Foo() noexcept(false);
};

class Bar
{
    Bar(Bar&&) { Foo aFoo; }
};
};

// Check for calls to defaulted constructors.
namespace test5
{
struct Foo
{
    Foo() = default; // non-throwing
};
class Bar
{
    Bar(Bar&&) // expected-error {{move constructor can be noexcept [loplugin:noexceptmove]}}
    {
        Foo aFoo;
        (void)aFoo;
    }
};
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
