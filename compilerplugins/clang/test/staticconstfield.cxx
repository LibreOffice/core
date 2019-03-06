/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_clang.h>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>
#include <vector>

class Class1
{
    OUString const
        m_field1; // expected-error {{const field can be static [loplugin:staticconstfield]}}
    Class1()
        : m_field1("xxxx")
    // expected-note@-1 {{init here [loplugin:staticconstfield]}}
    {
        (void)m_field1;
    }
};

class Class2
{
    OString const
        m_field2; // expected-error {{const field can be static [loplugin:staticconstfield]}}
    Class2()
        : m_field2("yyyy")
    // expected-note@-1 {{init here [loplugin:staticconstfield]}}
    {
        (void)m_field2;
    }
};

// no warning expected
class Class4
{
    OUString m_field3;
    Class4()
        : m_field3("zzzz")
    {
        (void)m_field3;
    }
};

class Class5
{
    enum class Enum
    {
        ONE
    };
    float const
        m_fielda1; // expected-error {{const field can be static [loplugin:staticconstfield]}}
    int const m_fielda2; // expected-error {{const field can be static [loplugin:staticconstfield]}}
    bool const
        m_fielda3; // expected-error {{const field can be static [loplugin:staticconstfield]}}
    Enum const
        m_fielda4; // expected-error {{const field can be static [loplugin:staticconstfield]}}
    Class5()
        : m_fielda1(1.0)
        // expected-note@-1 {{init here [loplugin:staticconstfield]}}
        , m_fielda2(1)
        // expected-note@-1 {{init here [loplugin:staticconstfield]}}
        , m_fielda3(true)
        // expected-note@-1 {{init here [loplugin:staticconstfield]}}
        , m_fielda4(Enum::ONE)
    // expected-note@-1 {{init here [loplugin:staticconstfield]}}
    {
        (void)m_fielda1;
        (void)m_fielda2;
        (void)m_fielda3;
        (void)m_fielda4;
    }
};

// no warning expected
class Class6
{
    enum class Enum
    {
        ONE
    };
    float m_fieldb1;
    int m_fieldb2;
    bool m_fieldb3;
    Enum m_fieldb4;
    Class6()
        : m_fieldb1(1.0)
        , m_fieldb2(1)
        , m_fieldb3(true)
        , m_fieldb4(Enum::ONE)
    {
        (void)m_fieldb1;
        (void)m_fieldb2;
        (void)m_fieldb3;
        (void)m_fieldb4;
    }
};

// no warning expected, checking for assigning to const field from multiple constructors
class Class7
{
    bool const m_field7;
    Class7()
        : m_field7(true)
    {
        (void)m_field7;
    }
    Class7(bool b)
        : m_field7(b)
    {
        (void)m_field7;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
