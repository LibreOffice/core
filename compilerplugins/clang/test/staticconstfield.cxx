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
    OUString const m_field1; // expected-note {{field here [loplugin:staticconstfield]}}
    Class1()
        : m_field1("xxxx")
    // expected-error@-1 {{field can be static const [loplugin:staticconstfield]}}
    {
        (void)m_field1;
    }
};

class Class2
{
    OString const m_field1; // expected-note {{field here [loplugin:staticconstfield]}}
    Class2()
        : m_field1("xxxx")
    // expected-error@-1 {{field can be static const [loplugin:staticconstfield]}}
    {
        (void)m_field1;
    }
};

// no warning expected
class Class4
{
    OUString m_field1;
    Class4()
        : m_field1("xxxx")
    {
        (void)m_field1;
    }
};

class Class5
{
    enum class Enum
    {
        ONE
    };
    float const m_field1; // expected-note {{field here [loplugin:staticconstfield]}}
    int const m_field2; // expected-note {{field here [loplugin:staticconstfield]}}
    bool const m_field3; // expected-note {{field here [loplugin:staticconstfield]}}
    Enum const m_field4; // expected-note {{field here [loplugin:staticconstfield]}}
    Class5()
        : m_field1(1.0)
        // expected-error@-1 {{field can be static const [loplugin:staticconstfield]}}
        , m_field2(1)
        // expected-error@-1 {{field can be static const [loplugin:staticconstfield]}}
        , m_field3(true)
        // expected-error@-1 {{field can be static const [loplugin:staticconstfield]}}
        , m_field4(Enum::ONE)
    // expected-error@-1 {{field can be static const [loplugin:staticconstfield]}}
    {
        (void)m_field1;
        (void)m_field2;
        (void)m_field3;
        (void)m_field4;
    }
};

// no warning expected
class Class6
{
    enum class Enum
    {
        ONE
    };
    float m_field1;
    int m_field2;
    bool m_field3;
    Enum m_field4;
    Class6()
        : m_field1(1.0)
        , m_field2(1)
        , m_field3(true)
        , m_field4(Enum::ONE)
    {
        (void)m_field1;
        (void)m_field2;
        (void)m_field3;
        (void)m_field4;
    }
};

// no warning expected, checking for assigning to const field from multiple constructors
class Class7
{
    bool const m_field1;
    Class7()
        : m_field1(true)
    {
        (void)m_field1;
    }
    Class7(bool b)
        : m_field1(b)
    {
        (void)m_field1;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
