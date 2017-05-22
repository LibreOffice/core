/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

#include "rtl/ustring.hxx"

int main() {
    OUString s;
    (void) OUString(s); // expected-error {{redundant copy construction from 'rtl::OUString' to 'rtl::OUString' [loplugin:stringcopy]}} expected-error {{redundant functional cast from/to 'rtl::OUString' [loplugin:redundantcast]}}
    using T1 = OUString;
    (void) T1(s); // expected-error {{redundant copy construction from 'rtl::OUString' to 'T1' (aka 'rtl::OUString') [loplugin:stringcopy]}}
    using T2 = OUString const;
    (void) T2(s); // expected-error {{redundant copy construction from 'rtl::OUString' to 'T2' (aka 'const rtl::OUString') [loplugin:stringcopy]}}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
