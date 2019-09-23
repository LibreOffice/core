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

namespace test1
{
void f(OUString s1)
{
    OUString s2 = s1;
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s2 += "xxx";
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s2 += "xxx";
}
}

namespace test2
{
void f(OUString s3)
{
    s3 += "xxx";
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s3 += "xxx";
}
}

namespace test3
{
void f(OString s4)
{
    s4 += "xxx";
    // expected-error@+1 {{simplify by merging with the preceding assignment [loplugin:stringadd]}}
    s4 += "xxx";
}
}

// no warning expected
namespace test4
{
void f()
{
    OUString sRet = "xxx";
#if OSL_DEBUG_LEVEL > 0
    sRet += ";";
#endif
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
