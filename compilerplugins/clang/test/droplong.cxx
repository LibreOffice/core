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
    int x = 1;
    int y = 1;
    long tmp = x + y; // expected-error {{rather replace 'long' with 'int' [loplugin:droplong]}}
    (void)tmp;
    tmp = x + y; // expected-error {{rather replace 'long' with 'int' [loplugin:droplong]}}

    sal_uLong tmp1 = x + y; // expected-error-re {{rather replace 'sal_uLong' (aka 'unsigned {{.+}}') with 'int' [loplugin:droplong]}}
    (void)tmp1;

    int tmp2 = (sal_uLong)1; // expected-error-re {{sal_uLong cast from 'sal_uLong' (aka 'unsigned {{.+}}') [loplugin:droplong]}}
    tmp2 = (long)1; // expected-error {{long cast from 'long' [loplugin:droplong]}}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
