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

template <sal_Unicode C> OUString f(sal_Unicode c, int n)
{
    OUString s0(C);
    OUString s1(c);
    // expected-note-re@+1 {{literal '{{(rtl::)?}}OUString' variable defined here [loplugin:elidestringvar]}}
    OUString s2('a');
    // expected-note-re@+1 {{literal '{{(rtl::)?}}OUString' variable defined here [loplugin:elidestringvar]}}
    OUString s3(u'a');
    static constexpr OUStringLiteral s4lit(u"a");
    // expected-note-re@+1 {{literal '{{(rtl::)?}}OUString' variable defined here [loplugin:elidestringvar]}}
    OUString s4 = s4lit;
    switch (n)
    {
        case 0:
            return s0;
        case 1:
            return s1;
        case 2:
            // expected-error-re@+1 {{replace single use of literal '{{(rtl::)?}}OUString' variable with a literal [loplugin:elidestringvar]}}
            return s2;
        case 3:
            // expected-error-re@+1 {{replace single use of literal '{{(rtl::)?}}OUString' variable with a literal [loplugin:elidestringvar]}}
            return s3;
        default:
            // expected-error-re@+1 {{replace single use of literal '{{(rtl::)?}}OUString' variable with a literal [loplugin:elidestringvar]}}
            return s4;
    }
}

// Trigger clang-cl to actually parse f under MSVC template instantiation model:
template OUString f<'a'>(sal_Unicode, int);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
