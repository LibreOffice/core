/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>

void call_view(std::u16string_view) {}
struct ConstructWithView
{
    ConstructWithView(std::u16string_view) {}
};

namespace test1
{
void f1(std::u16string_view s1)
{
    // no warning expected
    call_view(s1);
}
void f2(OUString s1)
{
    // expected-error@+1 {{rather than copy, pass with a view using substr() [loplugin:stringview]}}
    call_view(s1.copy(1, 2));
    // expected-error@+1 {{rather than copy, pass with a view using substr() [loplugin:stringview]}}
    ConstructWithView(s1.copy(1, 2));
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
