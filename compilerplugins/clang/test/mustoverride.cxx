/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <sal/types.h>

struct Base
{
    // expected-note@+1 {{declared here}}
    SAL_LOPLUGIN_ANNOTATE("mustoverride") virtual void mustOverride();

    SAL_LOPLUGIN_ANNOTATE("mustoverride") virtual void mustOverride2();
};

// expected-error@+1 {{'Derived' does not override 'mustOverride' from 'Base'}}
struct Derived : Base
{
    void mustOverride2() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
