/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <rtl/ref.hxx>
#include <tools/gen.hxx>

namespace test1
{
struct Foo
{
    void acquire();
    void release();
};
class Foo1
{
    std::unique_ptr<int> m_pbar1;
    rtl::Reference<Foo> m_pbar2;
    Foo1()
        : m_pbar1(nullptr)
        // expected-error@-1 {{no need to explicitly init an instance of 'std::unique_ptr<int>' with nullptr, just use default constructor [loplugin:simplifyconstruct]}}
        , m_pbar2(nullptr)
    // expected-error@-1 {{no need to explicitly init an instance of 'rtl::Reference<Foo>' with nullptr, just use default constructor [loplugin:simplifyconstruct]}}
    {
    }
};
}

// no warning expected when using std::unique_ptr constructor with a custom deleter
namespace test2
{
struct ITypeLib
{
};
struct IUnknown
{
    void Release();
};
void func2()
{
    std::unique_ptr<IUnknown, void (*)(IUnknown * p)> aITypeLibGuard(nullptr, [](IUnknown* p) {
        if (p)
            p->Release();
    });
}
}

namespace test3
{
struct Foo
{
    void acquire();
    void release();
};
void f(Foo* f)
{
    // expected-error@+1 {{simplify construction, just use 'Foo a(...);' [loplugin:simplifyconstruct]}}
    rtl::Reference<Foo> x = rtl::Reference(f);
}
}

// no warning expected
namespace test4
{
struct Foo
{
    void acquire();
    void release();
};
void f(Foo* f) { auto x = rtl::Reference(f); }
}

namespace test5
{
void f()
{
    // expected-error@+1 {{simplify construction, just use 'Foo a(...);' [loplugin:simplifyconstruct]}}
    tools::Rectangle x = tools::Rectangle(10, 10, 10, 10);
    (void)x;
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
