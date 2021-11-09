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

OUString const INCLUDED = "xxx";

inline void f()
{
    // expected-error@+1 {{rather declare this using OUStringConstExpr/OUStringLiteral/OStringLiteral/char[] [loplugin:stringstatic]}}
    static OUString const XXX = "xxx";
    (void)XXX;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
