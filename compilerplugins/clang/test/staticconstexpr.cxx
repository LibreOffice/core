/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

namespace test1
{
void f()
{
    // expected-error@+1 {{function-local constexpr vars should be declared static [loplugin:staticconstexpr]}}
    constexpr sal_Int64 powers[] = { 1, 10, 100 };
    (void)powers;

    // no warning expected, simple type
    constexpr sal_Int64 powers2 = 1.0;
    (void)powers2;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
