/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tools/solar.h>

int main()
{
    sal_uIntPtr x = 1;
    sal_uInt32 y = x;
    y = x;
    (void)y;
}

void main2()
{
    int x = 1;
    int y = 1;
    long tmp = x + y;
    // expected-error@-1 {{rather replace type of decl 'long' with 'int' [loplugin:convertlong]}}
    (void)tmp;
    tmp = x + y;

    sal_uLong tmp1 = x + y;
    // expected-error-re@-1 {{rather replace type of decl 'sal_uLong' (aka 'unsigned {{.+}}') with 'int' [loplugin:convertlong]}}
    (void)tmp1;

    int tmp2 = (sal_uLong)1;
    tmp2 = (long)1;

    sal_uIntPtr tmp3 = x + y;
    // expected-error-re@-1 {{rather replace type of decl 'sal_uIntPtr' (aka 'unsigned {{.+}}') with 'int' [loplugin:convertlong]}}
    (void)tmp3;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
