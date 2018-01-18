/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

int main()
{
    int x = 1;

    if (x == 1) // expected-error {{empty if body [loplugin:emptyif]}}
        ;

    if (x == 1)
    {
    }
    // expected-error@-3 {{empty if body [loplugin:emptyif]}}

    if (x == 1)
    {
    }
    else
    {
    }
    // expected-error@-2 {{empty else body [loplugin:emptyif]}}

    if (x == 1)
    {
    }
    else
        ; // expected-error {{empty else body [loplugin:emptyif]}}

    if (x == 1)
    {
    }
    else
    {
        x = 2;
    }

    // no warning expected
    if (x == 1)
    {
        x = 3;
    }
    if (x == 1)
        x = 3;
    if (x == 1)
    {
        //
    }
    if (x == 1)
    {
        /* */
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
