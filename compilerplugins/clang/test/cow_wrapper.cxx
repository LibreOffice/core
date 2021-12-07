/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config_clang.h"
#include "o3tl/cow_wrapper.hxx"
#include <utility>

struct ImplBitmapPalette
{
    void foo() const;
};

struct BitmapPalette
{
    void foo1()
    {
        // expected-error@+1 {{calling const method on o3tl::cow_wrapper impl class via non-const pointer, rather use std::as_const to prevent triggering an unnecessary copy [loplugin:cow_wrapper]}}
        mpImpl->foo();
    }
    void foo2() const
    {
        // no error expected
        mpImpl->foo();
    }
    void foo3()
    {
        // expected-error@+1 {{calling const method on o3tl::cow_wrapper impl class via non-const pointer, rather use std::as_const to prevent triggering an unnecessary copy [loplugin:cow_wrapper]}}
        (*mpImpl).foo();
    }
    void foo4()
    {
        // expected-error@+1 {{calling const method on o3tl::cow_wrapper impl class via non-const pointer, rather use std::as_const to prevent triggering an unnecessary copy [loplugin:cow_wrapper]}}
        std::as_const(*mpImpl).foo();
    }
    o3tl::cow_wrapper<ImplBitmapPalette> mpImpl;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
