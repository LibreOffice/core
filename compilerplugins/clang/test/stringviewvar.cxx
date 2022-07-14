/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#undef NDEBUG

#include "sal/config.h"
#include <string_view>
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"
#include "sal/types.h"

void f1(std::string_view sv)
{
    // expected-error-re@+1 {{replace var of type '{{(rtl::)?}}OString' with 'std::string_view' [loplugin:stringviewvar]}}
    OString s1(sv);
    (void)s1;
}

void f2(const OString s1)
{
    // no warning expected
    OString s2(s1);
    (void)s2;
}

std::string_view f3a();
void f3()
{
    // expected-error-re@+1 {{replace var of type '{{(rtl::)?}}OString' with 'std::string_view' [loplugin:stringviewvar]}}
    OString s1 = OString(f3a());
    (void)s1;
}

void f4a(const OString&);
void f4(std::string_view sv)
{
    // no warning expected
    OString s1(sv);
    f4a(s1);
}

void f5(std::string_view sv)
{
    // expected-error-re@+1 {{replace var of type '{{(rtl::)?}}OString' with 'std::string_view' [loplugin:stringviewvar]}}
    OString s1(sv);
    if (s1 == "xxxx")
        f5(sv);
}

void f6(std::u16string_view sv)
{
    // expected-error-re@+1 {{replace var of type '{{(rtl::)?}}OUString' with 'std::u16string_view' [loplugin:stringviewvar]}}
    OUString s6;
    s6 = sv;
    (void)s6;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
