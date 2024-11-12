/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vector>
#include <rtl/ustring.hxx>
#include <svl/typedwhich.hxx>

struct S
{
    int i1;
    int i2;
};

void f(S const (&a)[2])
{
    // expected-error-re@+1 {{Loop variable passed by value, pass by reference instead, e.g. 'const {{(struct )?}}S&' [loplugin:rangedforcopy]}}
    for (auto i : a)
    {
        (void)i;
    }
    for (auto[i1, i2] : a)
    {
        (void)i1;
        (void)i2;
    }
}

void f(std::vector<bool> const& v)
{
    for (auto b : v)
    {
        (void)b;
    }
}

// no warning expected
class SvxFontItem;
constexpr TypedWhichId<SvxFontItem> EE_CHAR_FONTINFO1(12);
constexpr TypedWhichId<SvxFontItem> EE_CHAR_FONTINFO2(13);
void f2()
{
    for (auto nWhich : { EE_CHAR_FONTINFO1, EE_CHAR_FONTINFO2 })
        (void)nWhich;
}

// no warning expected
void f3()
{
    for (rtl::OUStringChar c : { 'a', 'b' })
        (void)c;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
