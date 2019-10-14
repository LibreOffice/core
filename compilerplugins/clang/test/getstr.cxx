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

#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>

using S = OString;

void f(std::ostream& st, OString const& s1, OStringBuffer const& s2, OUString const& s3,
       OUStringBuffer const& s4, S const& s5, OString* p1, OStringBuffer* p2, OUString* p3,
       OUStringBuffer* p4, S* p5, char const* (OString::*pf)() const)
{
    st << s1.getStr() // expected-error {{directly use object of type 'rtl::OString' in a call of 'operator <<', instead of calling 'getStr' first [loplugin:getstr]}}
       << s2.getStr()
       << s3.getStr() // expected-error {{suspicious use of 'getStr' on an object of type 'rtl::OUString'; the result is implicitly cast to a void pointer in a call of 'operator <<' [loplugin:getstr]}}
       << s4.getStr() // expected-error {{suspicious use of 'getStr' on an object of type 'rtl::OUStringBuffer'; the result is implicitly cast to a void pointer in a call of 'operator <<' [loplugin:getstr]}}
       << s5.getStr() // expected-error {{directly use object of type 'S' (aka 'rtl::OString') in a call of 'operator <<', instead of calling 'getStr' first [loplugin:getstr]}}
       << p1->getStr() // expected-error {{directly use object of type 'rtl::OString' in a call of 'operator <<', instead of calling 'getStr' first [loplugin:getstr]}}
       << p2->getStr()
       << p3->getStr() // expected-error {{suspicious use of 'getStr' on an object of type 'rtl::OUString'; the result is implicitly cast to a void pointer in a call of 'operator <<' [loplugin:getstr]}}
       << p4->getStr() // expected-error {{suspicious use of 'getStr' on an object of type 'rtl::OUStringBuffer'; the result is implicitly cast to a void pointer in a call of 'operator <<' [loplugin:getstr]}}
       << p5->getStr() // expected-error {{directly use object of type 'rtl::OString' in a call of 'operator <<', instead of calling 'getStr' first [loplugin:getstr]}}
       << (s1.*pf)();
    SAL_INFO( // expected-error 1+ {{directly use object of type 'rtl::OString' in a call of 'operator <<', instead of calling 'getStr' first [loplugin:getstr]}}
        "test", s1.getStr());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
