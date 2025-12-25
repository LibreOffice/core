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
#include <vcl/vclenum.hxx>

namespace test1
{
sal_UCS4 GetChar();
void f()
{
    // expected-error@+1 {{loosing information assigning a sal_UCS4 value to a 'sal_Unicode' (aka 'char16_t') type [loplugin:narrow]}}
    sal_Unicode cChar = GetChar();
    (void)cChar;
    // no error expected
    sal_UCS4 cChar2 = GetChar();
    (void)cChar2;
    // expected-error@+1 {{loosing information assigning a sal_UCS4 value to a 'sal_Unicode' (aka 'char16_t') type [loplugin:narrow]}}
    cChar = GetChar();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
