/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtl/ustring.hxx>

#pragma clang diagnostic ignored "-Wunknown-warning-option" // for Clang < 13
#pragma clang diagnostic ignored "-Wunused-but-set-variable"

struct S {
    OUString mv1;
    OUString mv2;

    // make sure we ignore cases where the passed in parameter is std::move'd
    S(OUString v1, OUString v2)
      : mv1(std::move(v1)), mv2((std::move(v2))) {}

    // expected-error-re@+1 {{passing '{{(rtl::)?}}OUString' by value, rather pass by const lvalue reference [loplugin:passparamsbyref]}}
    S(OUString v1)
        : mv1(v1) {}

    // expected-error-re@+1 {{passing '{{(rtl::)?}}OUString' by value, rather pass by const lvalue reference [loplugin:passparamsbyref]}}
    void foo(OUString v1) { (void) v1; }

    // no warning expected
    void foo2(OUString v1) { mv1 = std::move(v1); }

    void takeByNonConstRef(OUString&);

    // no warning expected
    void foo3(OUString v)
    {
        takeByNonConstRef(v);
    }
};

namespace test2
{
    struct TestObject { OUString s[64]; void nonConstMethod(); };

    // no warning expected
    void f1(TestObject to)
    {
        to.nonConstMethod();
    }
}

void f()
{
    S* s;
    OUString v1, v2;
    s = new S(v1, v2);
}


// check that we don't warn when the param is modified
OUString trim_string(OUString aString)
{
    aString += "xxx";
    return aString;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
