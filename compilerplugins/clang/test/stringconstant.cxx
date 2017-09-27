/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include <cstring>

#include "com/sun/star/uno/Reference.hxx"
#include "rtl/strbuf.hxx"

extern void foo(OUString const &);

struct Foo {
    Foo(OUString const &, int) {}
    Foo(OUString const &) {}
};

int main() {
    char const s1[] = "foo";
    char const * const s2 = "foo";

    OStringBuffer sb;

    sb.append(OString()); // expected-error {{in call of 'rtl::OStringBuffer::append', replace empty 'OString' constructor with empty string literal [loplugin:stringconstant]}}
    sb.append(OString("foo")); // expected-error {{in call of 'rtl::OStringBuffer::append', elide explicit 'OString' constructor [loplugin:stringconstant]}}
    sb.append(OString(s1)); // expected-error {{in call of 'rtl::OStringBuffer::append', elide explicit 'OString' constructor [loplugin:stringconstant]}}
    sb.append(OString(s2)); // expected-error {{in call of 'rtl::OStringBuffer::append', elide explicit 'OString' constructor, and turn the non-array string constant into an array [loplugin:stringconstant]}}
    sb.append(OString("foo", std::strlen("foo"))); // expected-error {{in call of 'rtl::OStringBuffer::append', elide explicit 'OString' constructor [loplugin:stringconstant]}}
    sb.append(OString(s1, std::strlen(s1))); // expected-error {{in call of 'rtl::OStringBuffer::append', elide explicit 'OString' constructor [loplugin:stringconstant]}}
    sb.append(OString(s2, std::strlen(s2))); // expected-error {{in call of 'rtl::OStringBuffer::append', elide explicit 'OString' constructor, and turn the non-array string constant into an array [loplugin:stringconstant]}}
    sb.append("foo");
    sb.append(s1);
    sb.append(s2); // expected-error {{in call of 'rtl::OStringBuffer::append' with non-array string constant argument, turn the non-array string constant into an array [loplugin:stringconstant]}}
    sb.append("foo", std::strlen("foo")); // expected-error {{rewrite call of 'rtl::OStringBuffer::append' with string constant and matching length arguments as call of 'rtl::OStringBuffer::append' [loplugin:stringconstant]}}
    sb.append(s1, 3/*std::strlen(s1)*/); // expected-error {{rewrite call of 'rtl::OStringBuffer::append' with string constant and matching length arguments as call of 'rtl::OStringBuffer::append' [loplugin:stringconstant]}}
    sb.append(s2, 3/*std::strlen(s2)*/); // expected-error {{rewrite call of 'rtl::OStringBuffer::append' with string constant and matching length arguments as call of 'rtl::OStringBuffer::append', and turn the non-array string constant into an array [loplugin:stringconstant]}}

    sb.insert(0, OString()); // expected-error {{in call of 'rtl::OStringBuffer::insert', replace empty 'OString' constructor with empty string literal [loplugin:stringconstant]}}
    sb.insert(0, OString("foo")); // expected-error {{in call of 'rtl::OStringBuffer::insert', elide explicit 'OString' constructor [loplugin:stringconstant]}}
    sb.insert(0, OString(s1)); // expected-error {{in call of 'rtl::OStringBuffer::insert', elide explicit 'OString' constructor [loplugin:stringconstant]}}
    sb.insert(0, OString(s2)); // expected-error {{in call of 'rtl::OStringBuffer::insert', elide explicit 'OString' constructor, and turn the non-array string constant into an array [loplugin:stringconstant]}}
    sb.insert(0, OString("foo", std::strlen("foo"))); // expected-error {{in call of 'rtl::OStringBuffer::insert', elide explicit 'OString' constructor [loplugin:stringconstant]}}
    sb.insert(0, OString(s1, std::strlen(s1))); // expected-error {{in call of 'rtl::OStringBuffer::insert', elide explicit 'OString' constructor [loplugin:stringconstant]}}
    sb.insert(0, OString(s2, std::strlen(s2))); // expected-error {{in call of 'rtl::OStringBuffer::insert', elide explicit 'OString' constructor, and turn the non-array string constant into an array [loplugin:stringconstant]}}
    sb.insert(0, "foo");
    sb.insert(0, s1);
    sb.insert(0, s2); // expected-error {{in call of 'rtl::OStringBuffer::insert' with non-array string constant argument, turn the non-array string constant into an array [loplugin:stringconstant]}}
    sb.insert(0, "foo", std::strlen("foo")); // expected-error {{rewrite call of 'rtl::OStringBuffer::insert' with string constant and matching length arguments as call of 'rtl::OStringBuffer::insert' [loplugin:stringconstant]}}
    sb.insert(0, s1, 3/*std::strlen(s1)*/); // expected-error {{rewrite call of 'rtl::OStringBuffer::insert' with string constant and matching length arguments as call of 'rtl::OStringBuffer::insert' [loplugin:stringconstant]}}
    sb.insert(0, s2, 3/*std::strlen(s2)*/); // expected-error {{rewrite call of 'rtl::OStringBuffer::insert' with string constant and matching length arguments as call of 'rtl::OStringBuffer::insert', and turn the non-array string constant into an array [loplugin:stringconstant]}}

    foo(OUString("xxx")); // expected-error {{in call of 'foo', replace 'OUString' constructed from a string literal directly with the string literal [loplugin:stringconstant}}
    Foo aFoo(OUString("xxx"), 1); // expected-error {{in call of 'Foo::Foo', replace 'OUString' constructed from a string literal directly with the string literal}}
    (void)aFoo;
    Foo aFoo2(OUString("xxx")); // expected-error {{in call of 'Foo::Foo', replace 'OUString' constructed from a string literal directly with the string literal}}
    (void)aFoo2;

    (void) OUString("xxx", 3, RTL_TEXTENCODING_ASCII_US); // expected-error {{simplify construction of 'OUString' with string constant argument [loplugin:stringconstant]}}
    (void) OUString("xxx", 3, RTL_TEXTENCODING_ISO_8859_1); // expected-error {{simplify construction of 'OUString' with string constant argument (but beware, the given textencoding 12 is not RTL_TEXTENCODING_ASCII_US) [loplugin:stringconstant]}}
    (void) OUString("x\xA0x", 3, RTL_TEXTENCODING_ISO_8859_1);

    (void) OUString("xxx", 2, RTL_TEXTENCODING_ASCII_US); // expected-error {{suspicious 'rtl::OUString' constructor with literal of length 3 and non-matching length argument 2 [loplugin:stringconstant]}}

    (void) OUString(u8"xxx", 3, RTL_TEXTENCODING_ASCII_US); // expected-error {{simplify construction of 'OUString' with string constant argument [loplugin:stringconstant]}}
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
