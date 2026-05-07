/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <iterator>
#include <vector>

#include <rtl/ustring.hxx>

void f13(bool b)
{
    OUString aAttributeName;

    if (b)
    {
        // expected-error-re@+1 {{change type of constant character array ('const char{{ ?}}[9]') to OUStringLiteral [loplugin:stringliteralassign]}}
        aAttributeName = "DimColor";
    }
    else
    {
        // expected-error-re@+1 {{change type of constant character array ('const char{{ ?}}[11]') to OUStringLiteral [loplugin:stringliteralassign]}}
        aAttributeName = "Visibility";
    }
    (void)aAttributeName;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
