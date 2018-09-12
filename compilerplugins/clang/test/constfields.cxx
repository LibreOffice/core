/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vector>
#include <ostream>
#include <com/sun/star/uno/Any.hxx>

struct Test0
{
    void method1() {}
};

// checking for calling non-const method
struct Test1
// expected-error@-1 {{notconst m_field1 [loplugin:constfields]}}
{
    Test0* m_field1;

    void method1()
    {
        if (m_field1)
            m_field1->method1();
    }
};

// checking for assigning to field
struct Test2
// expected-error@-1 {{notconst m_field1 [loplugin:constfields]}}
{
    Test0* m_field1;

    Test2()
        : m_field1(nullptr)
    {
    }

    void method1() { m_field1 = nullptr; }
};
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
