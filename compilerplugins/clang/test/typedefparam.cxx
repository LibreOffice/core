/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "tools/solar.h"

class Foo
{
    void bar(sal_uIntPtr x); // expected-note {{declaration site here [loplugin:typedefparam]}}
    sal_uIntPtr bar(); // expected-note {{declaration site here [loplugin:typedefparam]}}
};

void Foo::bar(sal_uLong)
// expected-error@-1 {{function param 0 at definition site does not match function param at declaration site, 'sal_uLong' (aka 'unsigned long') vs 'sal_uIntPtr' (aka 'unsigned long') [loplugin:typedefparam]}}
{
}

sal_uLong Foo::bar()
// expected-error@-1 {{function return type at definition site does not match function param at declaration site, 'sal_uLong' (aka 'unsigned long') vs 'sal_uIntPtr' (aka 'unsigned long') [loplugin:typedefparam]}}
{
    return 1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
