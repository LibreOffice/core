/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <string_view>
#include <utility>

#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

namespace test1
{
OUString foo1();
OUString& foo2();
void f1()
{
    // expected-error@+1 {{view pointing into temporary i.e. dangling [loplugin:stringviewdangle]}}
    std::u16string_view v = foo1();
    // expected-error@+1 {{view pointing into temporary i.e. dangling [loplugin:stringviewdangle]}}
    v = foo1();

    // no warning expected
    std::u16string_view v2 = foo2();
    v2 = foo2();
}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
