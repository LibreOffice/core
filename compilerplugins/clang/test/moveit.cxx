/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config_clang.h"
#include <memory>

// expected-note@+3 {{type declared here [loplugin:moveit]}}
// expected-note@+2 {{type declared here [loplugin:moveit]}}
// expected-note@+1 {{type declared here [loplugin:moveit]}}
struct Movable
{
    std::shared_ptr<int> x;

    void method1();
};

namespace test1a
{
struct F
{
    // expected-note@+1 {{passing to this param [loplugin:moveit]}}
    void call_by_value(Movable);
    void foo()
    {
        // expected-note@+1 {{local var declared here [loplugin:moveit]}}
        Movable m;
        // expected-error@+1 {{can std::move this var into this param [loplugin:moveit]}}
        call_by_value(m);
    }
};
}

namespace test1b
{
struct F
{
    // expected-note@+1 {{passing to this param [loplugin:moveit]}}
    F(Movable);
    void foo()
    {
        // expected-note@+1 {{local var declared here [loplugin:moveit]}}
        Movable m;
        // expected-error@+1 {{can std::move this var into this param [loplugin:moveit]}}
        F a(m);
        (void)a;
    }
};
}

namespace test2
{
struct F
{
    // expected-note@+1 {{passing to this param [loplugin:moveit]}}
    F(Movable);
    void foo()
    {
        // expected-note@+1 {{local var declared here [loplugin:moveit]}}
        Movable m;
        // expected-error@+1 {{can std::move this var into this param [loplugin:moveit]}}
        F a(m);
        (void)a;
    }
};
}

// No error expected, because referencing after call
namespace test3
{
struct F
{
    F(Movable);
    void foo()
    {
        Movable m;
        F a(m);
        m.method1();
    }
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
