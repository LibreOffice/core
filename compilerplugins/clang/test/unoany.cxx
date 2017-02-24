/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include "com/sun/star/uno/Any.hxx"

css::uno::Any getAny() { return css::uno::Any(true); }

int main()
{
    css::uno::Any a;
    css::uno::Any b;
    a = b;
    a = getAny();
    a = css::uno::makeAny(true); // expected-error {{unnecessary copy, rather use <<= operator [loplugin:unoany]}}
    a = css::uno::Any(true); // expected-error {{unnecessary copy, rather use <<= operator [loplugin:unoany]}}
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
