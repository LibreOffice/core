/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <map>
#include <rtl/ustring.hxx>

namespace test1
{
int foo(int);

void main()
{
    int x = 1;
    foo(x = 2); // expected-error {{buried assignment, very hard to read [loplugin:buriedassign]}}
    int y = x = 1; // no warning expected
    (void)y;
    int z = foo(
        x = 1); // expected-error {{buried assignment, very hard to read [loplugin:buriedassign]}}
    (void)z;
    switch (x = 1)
    { // expected-error@-1 {{buried assignment, very hard to read [loplugin:buriedassign]}}
    }
    std::map<int, int> map1;
    map1[x = 1]
        = 1; // expected-error@-1 {{buried assignment, very hard to read [loplugin:buriedassign]}}
}
}

namespace test2
{
struct MyInt
{
    int x;
    MyInt(int i)
        : x(i)
    {
    }
    MyInt& operator=(MyInt const&) = default;
    MyInt& operator=(int) { return *this; }
    bool operator<(MyInt const& other) const { return x < other.x; }
};

MyInt foo(MyInt);

void main()
{
    MyInt x = 1;
    foo(x = 2); // expected-error {{buried assignment, very hard to read [loplugin:buriedassign]}}
    MyInt y = x = 1; // no warning expected
    (void)y;
    MyInt z = foo(
        x = 1); // expected-error {{buried assignment, very hard to read [loplugin:buriedassign]}}
    (void)z;
    z = x; // no warning expected
    std::map<MyInt, int> map1;
    map1[x = 1]
        = 1; // expected-error@-1 {{buried assignment, very hard to read [loplugin:buriedassign]}}
}
}

namespace test3
{
void main(OUString sUserAutoCorrFile, OUString sExt)
{
    OUString sRet;
    if (sUserAutoCorrFile == "xxx")
        sRet = sUserAutoCorrFile; // no warning expected
    if (sUserAutoCorrFile == "yyy")
        (sRet = sUserAutoCorrFile)
            += sExt; // expected-error@-1 {{buried assignment, very hard to read [loplugin:buriedassign]}}
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
