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
#include <map>

namespace drawinglayer::primitive2d
{
class Primitive2DContainer
{
};
}

struct Foo
{
    drawinglayer::primitive2d::Primitive2DContainer maMine;

    // expected-error@+2 {{rather use move && param3 [loplugin:moveparam]}}
    Foo(drawinglayer::primitive2d::Primitive2DContainer const& rContainer)
        : maMine(rContainer)
    {
    }

    // no warning expected
    Foo(drawinglayer::primitive2d::Primitive2DContainer&& rContainer)
        : maMine(rContainer)
    {
    }

    void foo1(const drawinglayer::primitive2d::Primitive2DContainer& rContainer)
    {
        // expected-error@+1 {{rather use move && param1 [loplugin:moveparam]}}
        maMine = rContainer;
    }
};

namespace test2
{
typedef std::map<int, int> Map2Map;

struct Foo
{
    Map2Map maMine;

    // expected-error@+2 {{rather use move && param3 [loplugin:moveparam]}}
    Foo(Map2Map const& rContainer)
        : maMine(rContainer)
    {
    }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
