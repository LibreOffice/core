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

// Determine whether std::ostream operator << for sal_Unicode* (aka char16_t*) is deleted (see
// <http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1423r3.html> "char8_t backward
// compatibility remediation", as implemented now by
// <https://gcc.gnu.org/git/?p=gcc.git;a=commit;h=0c5b35933e5b150df0ab487efb2f11ef5685f713>
// "libstdc++: P1423R3 char8_t remediation (2/4)" for -std=c++2a; TODO: the checks here and the
// relevant code in loplugin:getstr should eventually be removed once support for the deleted
// operators is widespread):
#if __cplusplus > 201703L && (defined __GLIBCXX__ || defined _MSC_VER)
#define HAVE_DELETED_OPERATORS true
#else
#define HAVE_DELETED_OPERATORS false
#endif

using S = OString;

void f(std::ostream& st, OString const& s1, OStringBuffer const& s2,
       OUString const& s3[[maybe_unused]], OUStringBuffer const& s4[[maybe_unused]], S const& s5,
       OString* p1, OStringBuffer* p2, OUString* p3[[maybe_unused]],
       OUStringBuffer* p4[[maybe_unused]], S* p5, char const* (OString::*pf)() const)
{
    st << s1.getStr() // expected-error {{directly use object of type 'rtl::OString' in a call of 'operator <<', instead of calling 'getStr' first [loplugin:getstr]}}
       << s2.getStr()
#if !HAVE_DELETED_OPERATORS
       << s3.getStr() // expected-error {{suspicious use of 'getStr' on an object of type 'rtl::OUString'; the result is implicitly cast to a void pointer in a call of 'operator <<' [loplugin:getstr]}}
       << s4.getStr() // expected-error {{suspicious use of 'getStr' on an object of type 'rtl::OUStringBuffer'; the result is implicitly cast to a void pointer in a call of 'operator <<' [loplugin:getstr]}}
#endif
       << s5.getStr() // expected-error {{directly use object of type 'S' (aka 'rtl::OString') in a call of 'operator <<', instead of calling 'getStr' first [loplugin:getstr]}}
       << p1->getStr() // expected-error {{directly use object of type 'rtl::OString' in a call of 'operator <<', instead of calling 'getStr' first [loplugin:getstr]}}
       << p2->getStr()
#if !HAVE_DELETED_OPERATORS
       << p3->getStr() // expected-error {{suspicious use of 'getStr' on an object of type 'rtl::OUString'; the result is implicitly cast to a void pointer in a call of 'operator <<' [loplugin:getstr]}}
       << p4->getStr() // expected-error {{suspicious use of 'getStr' on an object of type 'rtl::OUStringBuffer'; the result is implicitly cast to a void pointer in a call of 'operator <<' [loplugin:getstr]}}
#endif
       << p5->getStr() // expected-error {{directly use object of type 'rtl::OString' in a call of 'operator <<', instead of calling 'getStr' first [loplugin:getstr]}}
       << (s1.*pf)();
    SAL_INFO( // expected-error 1+ {{directly use object of type 'rtl::OString' in a call of 'operator <<', instead of calling 'getStr' first [loplugin:getstr]}}
        "test", s1.getStr());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
