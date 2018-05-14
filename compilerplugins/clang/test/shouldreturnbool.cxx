/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

int foo1(char ch)
{
    // expected-error@-2 {{only returning one or zero is an indication you want to return bool [loplugin:shouldreturnbool]}}
    if (ch == 'x')
        return 1;
    return 0;
}

long foo2()
{
    // expected-error@-2 {{only returning one or zero is an indication you want to return bool [loplugin:shouldreturnbool]}}
    return 1;
}

enum Enum1
{
    NONE
};

Enum1 foo3() { return Enum1::NONE; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
