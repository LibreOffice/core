/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_clang.h>
#include <sal/config.h>
#include <rtl/ustring.hxx>

struct S1
{
    int x, y;
};

S1 const& f1(int a)
{
    static S1 s1[]{
        // expected-error@-1 {{var should be static const, or allowlisted [loplugin:staticvar]}}
        { 1, 1 }
    };
    // no warning expected
    const S1 s2[]{ { a, 1 } };
    (void)s2;
    return s1[0];
}

struct S2
{
    OUString x;
};

S2 const& f2()
{
    static S2 const s1[]{
        // expected-error@-1 {{static const var requires runtime initialization? [loplugin:staticvar]}}
        { "xxx" }
    };
    return s1[0];
}

// no warning expected
S2 const& f3()
{
    static S2 s1[]{ { "xxx" } };
    return s1[0];
}

// no warning expected
struct S4
{
    OUStringLiteral const cName;
    bool const bCanBeVisible;
};
S4 const& f4()
{
    static const S4 s1[] = {
        { OUStringLiteral(u"/DocColor"), false },
    };
    return s1[0];
}

struct S5
{
    bool const bCanBeVisible;
};
void f5(bool b)
{
    const S5 s1[] = {
        { b },
    };
    (void)s1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
