/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include "rtl/ustring.hxx"
#include "tools/color.hxx"

#include <functional>
#include <initializer_list>
#include <memory>

void method1(OUString const&); // expected-note {{in call to method here [loplugin:redundantfcast]}}

struct Foo
{
    Foo(int) {}
};

void func1(Foo const& f); // expected-note {{in call to method here [loplugin:redundantfcast]}}

namespace tools
{
struct Polygon
{
    Polygon() = default;
};
struct PolyPolygon
{
    PolyPolygon(
        Polygon const&); // expected-note {{in call to method here [loplugin:redundantfcast]}}
};
}

void ImplWritePolyPolygonRecord(const tools::PolyPolygon& rPolyPoly);

int main()
{
    OUString s;
    (void)OUString(
        s); // expected-error@-1 {{redundant functional cast from 'rtl::OUString' to 'rtl::OUString' [loplugin:redundantfcast]}}
    using T1 = OUString;
    (void)T1(
        s); // expected-error@-1 {{redundant functional cast from 'rtl::OUString' to 'T1' (aka 'rtl::OUString') [loplugin:redundantfcast]}}
    using T2 = OUString const;
    (void)T2(
        s); // expected-error@-1 {{redundant functional cast from 'rtl::OUString' to 'T2' (aka 'const rtl::OUString') [loplugin:redundantfcast]}}

    (void)std::unique_ptr<int>(std::unique_ptr<int>(
        new int{})); // expected-error@-1 {{redundant functional cast from 'std::unique_ptr<int>' to 'std::unique_ptr<int>' [loplugin:redundantfcast]}}

    OUString s1;
    method1(OUString(
        s1)); // expected-error@-1 {{redundant functional cast from 'rtl::OUString' to 'rtl::OUString' [loplugin:redundantfcast]}}
    OUString s2;
    s2 = OUString(
        s1); // expected-error@-1 {{redundant functional cast from 'rtl::OUString' to 'rtl::OUString' [loplugin:redundantfcast]}}

    Color col1;
    Color col2 = Color(
        col1); // expected-error@-1 {{redundant functional cast from 'Color' to 'Color' [loplugin:redundantfcast]}}
    (void)col2;

    Foo foo(1);
    func1(Foo(
        foo)); // expected-error@-1 {{redundant functional cast from 'Foo' to 'Foo' [loplugin:redundantfcast]}}

    const tools::Polygon aPolygon;
    ImplWritePolyPolygonRecord(tools::PolyPolygon(tools::Polygon(
        aPolygon))); // expected-error@-1 {{redundant functional cast from 'const tools::Polygon' to 'tools::Polygon' [loplugin:redundantfcast]}}
}

class Class1
{
    Foo foo;
    Foo func2()
    {
        return Foo(
            foo); // expected-error@-1 {{redundant functional cast from 'Foo' to 'Foo' [loplugin:redundantfcast]}}
    }
};

// casting of lambdas
namespace test5
{
void f1(std::function<void()> x);
void f2()
{
    // expected-error-re@+1 {{redundant functional cast {{.+}} [loplugin:redundantfcast]}}
    f1(std::function([&]() {}));
}
};
namespace test6
{
void f1(std::function<void(int)>);
void f1(std::function<void(long)>);
void f2()
{
    f1(std::function<void(long)>([&](int) {})); // should not warn here
}
}

namespace test7
{
// expected-note@+1 6 {{in call to method here [loplugin:redundantfcast]}}
void f1(std::initializer_list<int> const&);
// expected-note@+1 6 {{in call to method here [loplugin:redundantfcast]}}
template <typename T> void f2(std::initializer_list<T> const&);
// expected-note@+1 4 {{in call to method here [loplugin:redundantfcast]}}
template <typename T> void f3(T const&);
// expected-note@+1 4 {{in call to method here [loplugin:redundantfcast]}}
template <typename... T> void f4(T const&...);
void f5(int, ...);
void g(std::initializer_list<int> il)
{
    f1(il);
    f2(il);
    f3(il);
    f4(il);
    f5(0, il);
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f1(std::initializer_list<int>(il));
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f2(std::initializer_list<int>(il));
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f3(std::initializer_list<int>(il));
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f4(std::initializer_list<int>(il));
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f5(0, std::initializer_list<int>(il));
    f1({});
    f1(std::initializer_list<int>{}); // should warn, but not modelled as CXXFunctionalCastExpr
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f1(std::initializer_list<int>({}));
    // f2({}); //error
    f2(std::initializer_list<int>{}); // should warn, but not modelled as CXXFunctionalCastExpr
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f2(std::initializer_list<int>({}));
    // f3({}); //error
    f3(std::initializer_list<int>{}); // (not modelled as CXXFunctionalCastExpr anyway)
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f3(std::initializer_list<int>({})); // arguably rather subtle, remove "("...")"
    // f4({}); //error
    f4(std::initializer_list<int>{}); // (not modelled as CXXFunctionalCastExpr anyway)
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f4(std::initializer_list<int>({})); // arguably rather subtle, remove "("...")"
    // f5(0, {}); //error
    f5(0, std::initializer_list<int>{}); // (not modelled as CXXFunctionalCastExpr anyway)
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f5(0, std::initializer_list<int>({})); // arguably rather subtle, remove "("...")"
    f1({ 1 });
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f1(std::initializer_list<int>{ 1 });
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f1(std::initializer_list<int>({ 1 }));
    f2({ 1 });
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f2(std::initializer_list<int>{ 1 });
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f2(std::initializer_list<int>({ 1 }));
    // f3({1}); //error
    f3(std::initializer_list<int>{ 1 });
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f3(std::initializer_list<int>({ 1 })); // arguably rather subtle, remove "("...")"
    // f4({1}); //error
    f4(std::initializer_list<int>{ 1 });
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f4(std::initializer_list<int>({ 1 })); // arguably rather subtle, remove "("...")"
    // f5(0, {1}); //error
    f5(0, std::initializer_list<int>{ 1 });
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f5(0, std::initializer_list<int>({ 1 })); // arguably rather subtle, remove "("...")"
    f1({ 1, 2, 3 });
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f1(std::initializer_list<int>{ 1, 2, 3 });
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f1(std::initializer_list<int>({ 1, 2, 3 }));
    f2({ 1, 2, 3 });
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f2(std::initializer_list<int>{ 1, 2, 3 });
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f2(std::initializer_list<int>({ 1, 2, 3 }));
    // f3({1, 2, 3}); //error
    f3(std::initializer_list<int>{ 1, 2, 3 });
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f3(std::initializer_list<int>({ 1, 2, 3 })); // arguably rather subtle, remove "("...")"
    // f4({1, 2, 3}); //error
    f4(std::initializer_list<int>{ 1, 2, 3 });
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f4(std::initializer_list<int>({ 1, 2, 3 })); // arguably rather subtle, remove "("...")"
    // f5(0, {1, 2, 3}); //error
    f5(0, std::initializer_list<int>{ 1, 2, 3 });
    // expected-error@+1 {{redundant functional cast from 'std::initializer_list<int>' to 'std::initializer_list<int>' [loplugin:redundantfcast]}}
    f5(0, std::initializer_list<int>({ 1, 2, 3 })); // arguably rather subtle, remove "("...")"
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
