/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>

// expected-error@+1 {{rather declare this as constexpr [loplugin:stringstatic]}}
static const OUString TEST1 = "xxx";
static constexpr OUString TEST2 = u"xxx"_ustr;

void test2()
{
    (void)TEST2;
    // expected-error@+1 {{rather declare this as constexpr [loplugin:stringstatic]}}
    static const OUString XXX = "xxx";
    static constexpr OUString XXX2 = u"xxx"_ustr;
    (void)XXX;
    (void)XXX2;
    // expected-error@+1 {{rather declare this as constexpr [loplugin:stringstatic]}}
    static const OUString A1[1] = { u"xxx"_ustr };
    static constexpr OUString A2[1] = { u"xxx"_ustr };
    (void)A1;
    (void)A2;
}
