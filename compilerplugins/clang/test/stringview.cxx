/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>

void call_view(std::u16string_view) {}
void call_view(std::string_view) {}
struct ConstructWithView
{
    ConstructWithView(std::u16string_view) {}
    ConstructWithView(std::string_view) {}
};

namespace test1
{
void f1(std::u16string_view s1)
{
    // no warning expected
    call_view(s1);
}
void f1(std::string_view s1)
{
    // no warning expected
    call_view(s1);
}
void f1(OUString s1)
{
    // expected-error@+1 {{rather than copy, pass with a view using subView() [loplugin:stringview]}}
    call_view(s1.copy(1, 2));
    // expected-error@+1 {{rather than copy, pass with a view using subView() [loplugin:stringview]}}
    call_view(s1.copy(1));
    // expected-error@+1 {{rather than copy, pass with a view using subView() [loplugin:stringview]}}
    ConstructWithView(s1.copy(1, 2));
    // expected-error@+1 {{rather than copy, pass with a view using subView() [loplugin:stringview]}}
    ConstructWithView(s1.copy(1));
}
void f1(OString s1)
{
    // expected-error@+1 {{rather than copy, pass with a view using subView() [loplugin:stringview]}}
    call_view(s1.copy(1, 2));
    // expected-error@+1 {{rather than copy, pass with a view using subView() [loplugin:stringview]}}
    call_view(s1.copy(1));
    // expected-error@+1 {{rather than copy, pass with a view using subView() [loplugin:stringview]}}
    ConstructWithView(s1.copy(1, 2));
    // expected-error@+1 {{rather than copy, pass with a view using subView() [loplugin:stringview]}}
    ConstructWithView(s1.copy(1));
}
}

namespace test2
{
void f3(OUString s1)
{
    // expected-error@+1 {{rather than copy, pass with a view using subView() [loplugin:stringview]}}
    OUString s2 = s1.copy(1, 2) + "xxx";
    // expected-error@+1 {{rather than copy, pass with a view using subView() [loplugin:stringview]}}
    s2 = s1.copy(1) + "xxx";
    // expected-error@+1 {{rather than copy, pass with a view using subView() [loplugin:stringview]}}
    s2 = "xxx" + s1.copy(1);
    // expected-error@+1 {{rather than copy, pass with a view using subView() [loplugin:stringview]}}
    s2 += s1.copy(1);
}
void f3(OString s1)
{
    // expected-error@+1 {{rather than copy, pass with a view using subView() [loplugin:stringview]}}
    OString s2 = s1.copy(1, 2) + "xxx";
    (void)s2;
    // expected-error@+1 {{rather than copy, pass with a view using subView() [loplugin:stringview]}}
    OString s3 = s1.copy(1) + "xxx";
    (void)s3;
}
}

namespace test3
{
void f4(OUString s1, OUString s2)
{
    // expected-error@+1 {{rather than copy, pass with a view using subView() [loplugin:stringview]}}
    sal_Unicode x = s2.copy(1, 2)[12];
    (void)x;
    // expected-error@+1 {{rather than copy, pass with a view using subView() [loplugin:stringview]}}
    if (s2.copy(1, 2) < s1)
        ;
}
}

void f5()
{
    // expected-error@+1 {{instead of an empty 'rtl::OString', pass an empty 'std::string_view' [loplugin:stringview]}}
    call_view(OString());
    // expected-error@+1 {{instead of an empty 'rtl::OUString', pass an empty 'std::u16string_view' [loplugin:stringview]}}
    call_view(OUString());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
