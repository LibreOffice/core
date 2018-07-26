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

struct Foo
{
    OUString m_field; // expected-note {{field here [loplugin:stringloop]}}
    void func1()
    {
        for (int i = 0; i < 10; ++i)
            m_field += "xxx";
        // expected-error@-1 {{appending to OUString in loop, rather use OUStringBuffer [loplugin:stringloop]}}
        // no warning expected
        m_field += "xxx";
    }
};

void func2()
{
    OUString s; // expected-note {{var here [loplugin:stringloop]}}
    for (int i = 0; i < 10; ++i)
        s += "xxx"; // expected-error {{appending to OUString in loop, rather use OUStringBuffer [loplugin:stringloop]}}

    // no warning expected
    OUString s2;
    s2 += "xxx";
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
