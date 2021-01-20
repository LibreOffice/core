/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"
#include "tools/color.hxx"

int main()
{
    constexpr Color col1;
    Color col2;
    col2 = sal_Int32(
        col1); // expected-error@-1 {{redundant double conversion [loplugin:doubleconvert]}}
    (void)col2;
    // expected-error@+1 {{redundant double conversion [loplugin:doubleconvert]}}
    Color col3 = sal_Int32(Color());
    (void)col3;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
