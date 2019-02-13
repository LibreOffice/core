/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <ostream>

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

#define FOO "foo"

void f(std::ostream& s1)
{
    static char const foo[] = "foo";
    s1 << "foo"
       << "foo";
    // expected-error@-1 {{replace '<<' between string literals with juxtaposition}}
    s1 << "foo" << FOO;
    // expected-error@-1 {{replace '<<' between string literals with juxtaposition}}
    s1 << "foo" << foo;
    s1 << "foo" << OString("foo");
    // expected-error@-1 {{replace '<<' between string literals with juxtaposition}}
    s1 << "foo" << OString(FOO);
    // expected-error@-1 {{replace '<<' between string literals with juxtaposition}}
    s1 << "foo" << OString(foo);
    s1 << "foo" << OUString("foo");
    // expected-error@-1 {{replace '<<' between string literals with juxtaposition}}
    s1 << "foo" << OUString(FOO);
    // expected-error@-1 {{replace '<<' between string literals with juxtaposition}}
    s1 << "foo" << OUString(foo);
    s1 << "foo" << OUStringLiteral("foo"); //TODO: warn too, OUStringLiteral wrapped in OUString
    s1 << "foo" << OUStringLiteral(FOO); //TODO: warn too, OUStringLiteral wrapped in OUString
    s1 << "foo" << OUStringLiteral(foo);
    OString s2;
    s2 = "foo" + OString("foo");
    // expected-error@-1 {{replace '+' between string literals with juxtaposition}}
    s2 = "foo" + OString(FOO);
    // expected-error@-1 {{replace '+' between string literals with juxtaposition}}
    s2 = "foo" + OString(foo);
    s2 = "foo" + OStringLiteral("foo");
    // expected-error@-1 {{replace '+' between string literals with juxtaposition}}
    s2 = "foo" + OStringLiteral(FOO);
    // expected-error@-1 {{replace '+' between string literals with juxtaposition}}
    s2 = "foo" + OStringLiteral(foo);
    OUString s3;
    s3 = "foo" + OUString("foo");
    // expected-error@-1 {{replace '+' between string literals with juxtaposition}}
    s3 = "foo" + OUString(FOO);
    // expected-error@-1 {{replace '+' between string literals with juxtaposition}}
    s3 = "foo" + OUString(foo);
    s3 = "foo" + OUStringLiteral("foo");
    // expected-error@-1 {{replace '+' between string literals with juxtaposition}}
    s3 = "foo" + OUStringLiteral(FOO);
    // expected-error@-1 {{replace '+' between string literals with juxtaposition}}
    s3 = "foo" + OUStringLiteral(foo);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
