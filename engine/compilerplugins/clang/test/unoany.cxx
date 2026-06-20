/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include "cpo/uno/Any.hxx"

#pragma clang diagnostic ignored "-Wunknown-warning-option" // for Clang < 13
#pragma clang diagnostic ignored "-Wunused-but-set-variable"

cpo::uno::Any getAny() { return cpo::uno::Any(true); }

int main()
{
    cpo::uno::Any a;
    cpo::uno::Any b;
    a = b;
    a = getAny();
    a = cpo::uno::Any(true); // expected-error {{unnecessary copy, rather use <<= operator directly with the 'Any' constructor argument [loplugin:unoany]}}
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
