/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>
#include <rtl/string.hxx>

class Class1
{
    OUString const m_field1; // expected-note {{field here [loplugin:conststringfield]}}
    Class1()
        : m_field1("xxxx")
    // expected-error@-1 {{string field can be static const [loplugin:conststringfield]}}
    {
    }
};

class Class2
{
    OString const m_field1; // expected-note {{field here [loplugin:conststringfield]}}
    Class2()
        : m_field1("xxxx")
    // expected-error@-1 {{string field can be static const [loplugin:conststringfield]}}
    {
    }
};

// no warning expected
class Class4
{
    OUString m_field1;
    Class4()
        : m_field1("xxxx")
    {
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
