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
    sal_uInt32 y = x; // expected-error-re {{cast from 'sal_uIntPtr' (aka 'unsigned {{.+}}') to 'sal_uInt32' (aka 'unsigned {{.+}}') [loplugin:convertuintptr]}}
    y = x; // expected-error-re {{cast from 'sal_uIntPtr' (aka 'unsigned {{.+}}') to 'sal_uInt32' (aka 'unsigned {{.+}}') [loplugin:convertuintptr]}}
    (void)y;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
