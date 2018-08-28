/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"
#include "rtl/string.hxx"
#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"

void func2(OUString s1, OUString s2)
{
    OUStringBuffer buf;
    buf.append(s1 + s2);
    // expected-error@-1 {{appending added result of OUString to OUStringBuffer, rather do .append(x).append(y) [loplugin:stringbuffer]}}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
