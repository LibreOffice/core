/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sal/config.h"

// no warning expected
namespace test1
{
class Foo
{
    ~Foo() { f(); }
    void f();
};
}

namespace test2
{
class Foo
{
    ~Foo() { f(); }
    // expected-error@-1 {{calling virtual method from destructor, either make the virtual method final, or make this class final [loplugin:fragiledestructor]}}
    virtual void f();
    // expected-note@-1 {{callee method here [loplugin:fragiledestructor]}}
};
}

// no warning expected
namespace test3
{
class Foo final
{
    ~Foo() { f(); }
    virtual void f();
};
}

namespace test4
{
struct Bar
{
    virtual ~Bar();
    virtual void f();
    // expected-note@-1 {{callee method here [loplugin:fragiledestructor]}}
};
class Foo : public Bar
{
    ~Foo() { f(); }
    // expected-error@-1 {{calling virtual method from destructor, either make the virtual method final, or make this class final [loplugin:fragiledestructor]}}
};
}

// no warning expected
namespace test5
{
struct Bar
{
    virtual ~Bar();
    virtual void f();
};
class Foo : public Bar
{
    ~Foo() { f(); }
    virtual void f() final override;
};
}

// no warning expected
namespace test6
{
struct Bar
{
    virtual ~Bar();
    virtual void f();
};
class Foo : public Bar
{
    ~Foo() { Foo::f(); }
    virtual void f() override;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
