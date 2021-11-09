/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>

#include "stringstatic.hxx"

// expected-error@+1 {{rather declare this using OUStringConstExpr/OUStringLiteral/OStringLiteral/char[] [loplugin:stringstatic]}}
static const OUString TEST1 = "xxx";

void f(rtl_uString const*);

void test2()
{
    // expected-error@+1 {{rather declare this using OUStringConstExpr/OUStringLiteral/OStringLiteral/char[] [loplugin:stringstatic]}}
    static const OUString XXX = "xxx";
    // expected-error@+1 {{rather declare this using OUStringConstExpr/OUStringLiteral/OStringLiteral/char[] [loplugin:stringstatic]}}
    static const OUString XXX2 = "xxx";
    (void)XXX;
    (void)XXX2;
    static const OUString DATA = "xxx";
    f(DATA.pData);
}
