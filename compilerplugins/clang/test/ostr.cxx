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

#define M(arg) f(arg, arg)

struct S
{
    OUString s;
};

void f(OUString const&);

void f(OUString const&, OUString const&);

void f()
{
    // expected-error-re@+1 {{use a _ustr user-defined string literal instead of constructing an instance of '{{(rtl::)?}}OUString' from an ordinary string literal [loplugin:ostr]}}
    OUString s1 = "foo";
    (void)s1;
    // expected-error-re@+1 {{use a _ustr user-defined string literal instead of constructing an instance of '{{(rtl::)?}}OUString' from an ordinary string literal [loplugin:ostr]}}
    OUString s2 = (("foo"));
    (void)s2;
    // expected-error-re@+1 {{use a _ustr user-defined string literal instead of constructing an instance of '{{(rtl::)?}}OUString' from an ordinary string literal [loplugin:ostr]}}
    OUString s3("foo");
    (void)s3;
    // expected-error-re@+1 {{use a _ustr user-defined string literal instead of constructing an instance of '{{(rtl::)?}}OUString' from an ordinary string literal [loplugin:ostr]}}
    OUString s4((("foo")));
    (void)s4;
    // expected-error-re@+1 {{use a _ustr user-defined string literal instead of constructing an instance of '{{(rtl::)?}}OUString' from an ordinary string literal [loplugin:ostr]}}
    f(OUString("foo"));
    // expected-error-re@+1 {{use a _ustr user-defined string literal instead of constructing an instance of '{{(rtl::)?}}OUString' from an ordinary string literal [loplugin:ostr]}}
    f(((OUString((("foo"))))));
    // expected-error-re@+1 {{use a _ustr user-defined string literal instead of constructing an instance of '{{(rtl::)?}}OUString' from an ordinary string literal [loplugin:ostr]}}
    f(OUString("foo", rtl::libreoffice_internal::Dummy()));
    // expected-error-re@+1 {{use a _ustr user-defined string literal instead of constructing an instance of '{{(rtl::)?}}OUString' from an ordinary string literal [loplugin:ostr]}}
    f(((OUString((("foo")), rtl::libreoffice_internal::Dummy()))));
    // expected-error-re@+1 {{use a _ustr user-defined string literal instead of constructing an instance of '{{(rtl::)?}}OUString' from an ordinary string literal [loplugin:ostr]}}
    f("foo");
    // expected-error-re@+1 {{use a _ustr user-defined string literal instead of constructing an instance of '{{(rtl::)?}}OUString' from an ordinary string literal [loplugin:ostr]}}
    f((("foo")));

    // Only generate one warning here, not two, for a macro argument used twice in the macro's
    // expansion:
    // expected-error-re@+1 {{use a _ustr user-defined string literal instead of constructing an instance of '{{(rtl::)?}}OUString' from an ordinary string literal [loplugin:ostr]}}
    M("foo");

    // expected-note@+1 {{intermediary variable l1 declared here [loplugin:ostr]}}
    constexpr OStringLiteral l1("foo");
    // expected-error@+1 {{directly use a _ostr user-defined string literal instead of introducing the intermediary 'OStringLiteral' variable l1 [loplugin:ostr]}}
    (void)l1;
    // expected-error@+1 {{use 'OString', created from a _ostr user-defined string literal, instead of 'OStringLiteral' for the variable l2 [loplugin:ostr]}}
    constexpr OStringLiteral l2("foo");
    (void)l2;
    (void)l2;
    // expected-note@+1 {{intermediary variable l3 declared here [loplugin:ostr]}}
    OUStringLiteral l3(u"foo");
    // expected-error@+1 {{directly use a _ustr user-defined string literal instead of introducing the intermediary 'OUStringLiteral' variable l3 [loplugin:ostr]}}
    (void)l3;
    // expected-error@+1 {{use 'OUString', created from a _ustr user-defined string literal, instead of 'OUStringLiteral' for the variable l4 [loplugin:ostr]}}
    OUStringLiteral l4(u"foo");
    (void)l4;
    (void)l4;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
