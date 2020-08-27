/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <rtl/ustring.hxx>

void f(OUString s, bool b)
{
    // expected-error@+2 {{replace 2nd operand of conditional expression with `rtl::OUStringLiteral` [loplugin:conditionalstring]}}
    // expected-note@+1 {{conditional expression is here [loplugin:conditionalstring]}}
    s += (b ? OUString("a") : throw 0);
    // expected-error@+2 {{replace 2nd operand of conditional expression with `rtl::OUStringLiteral` [loplugin:conditionalstring]}}
    // expected-note@+1 {{conditional expression is here [loplugin:conditionalstring]}}
    s += (b ? OUString("a") : OUStringLiteral(u"b"));
    // expected-error@+1 {{replace both 2nd and 3rd operands of conditional expression with `rtl::OUStringLiteral` [loplugin:conditionalstring]}}
    b = (b ? ("x") : (OUString(("y")))) == s;
    // expected-error@+1 {{replace both 2nd and 3rd operands of conditional expression with `rtl::OUStringLiteral` [loplugin:conditionalstring]}}
    b = operator==(s, b ? OUString("x") : OUString("y"));
    // expected-error@+1 {{replace both 2nd and 3rd operands of conditional expression with `rtl::OUStringLiteral` [loplugin:conditionalstring]}}
    s.operator+=(b ? OUString("x") : OUString("y"));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
