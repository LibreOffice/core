/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>

struct Foo
// expected-error@-1 {{read m_foo1 [loplugin:unusedfields]}}
{
    int m_foo1;
};

struct Bar
// expected-error@-1 {{read m_bar2 [loplugin:unusedfields]}}
{
    int m_bar1;
    int m_bar2 = 1;
    Bar(Foo const & foo) : m_bar1(foo.m_foo1) {}

    int bar() { return m_bar2; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
