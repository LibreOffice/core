/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

int f1(int x)
{
    // expected-error@+1 {{nested if should be collapsed into one statement 9 9 [loplugin:collapseif]}}
    if (x == 1)
        if (x == 2)
            return 1;

    // expected-error@+1 {{nested if should be collapsed into one statement 9 9 [loplugin:collapseif]}}
    if (x == 1)
    {
        if (x == 2)
            return 1;
    }

    // no warning expected
    if (x == 1)
    {
        // comment here prevents warning
        if (x == 2)
            return 1;
    }

    // no warning expected
    if (x == 1)
    {
        if (x == 2)
            return 1;
    }
    else
        return 3;

    // no warning expected
    if (x == 1)
    {
        if (x == 2)
            return 1;
        else
            return 3;
    }

    return 2;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
