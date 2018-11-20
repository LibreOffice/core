/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include <memory>

#include "rtl/ustring.hxx"
#include "tools/color.hxx"

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
    // expected-error@-2 {{redundant functional cast from 'rtl::OUString' to 'rtl::OUString' [loplugin:redundantfcast]}}
    OUString s2;
    s2 = OUString(
        s1); // expected-error@-1 {{redundant functional cast from 'rtl::OUString' to 'rtl::OUString' [loplugin:redundantfcast]}}

    Color col1;
    Color col2 = Color(
        col1); // expected-error@-1 {{redundant functional cast from 'Color' to 'Color' [loplugin:redundantfcast]}}
    (void)col2;

    Foo foo(1);
    func1(Foo(
        foo)); // expected-error@-1 {{redundant functional cast from 'Foo' to 'Foo' [loplugin:redundantfcast]}} expected-error@-1 {{redundant functional cast from 'Foo' to 'Foo' [loplugin:redundantfcast]}}

    const tools::Polygon aPolygon;
    ImplWritePolyPolygonRecord(tools::PolyPolygon(tools::Polygon(
        aPolygon))); // expected-error@-1 {{redundant functional cast from 'const tools::Polygon' to 'tools::Polygon' [loplugin:redundantfcast]}} expected-error@-1 {{redundant functional cast from 'const tools::Polygon' to 'tools::Polygon' [loplugin:redundantfcast]}}
}

class Class1
{
    Foo foo;
    Foo func2()
    {
        return Foo(
            foo); // expected-error@-1 {{redundant functional cast from 'Foo' to 'Foo' [loplugin:redundantfcast]}} expected-error@-1 {{redundant functional cast from 'Foo' to 'Foo' [loplugin:redundantfcast]}}
    }
};
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
