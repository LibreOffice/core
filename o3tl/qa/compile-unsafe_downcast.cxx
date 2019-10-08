/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <o3tl/unsafe_downcast.hxx> // expected-note@o3tl/unsafe_downcast.hxx:* 0+ {{}}

struct Base
{
    virtual ~Base();
};

struct Derived : Base
{
};

void f(Base* b, Derived* d)
{
    o3tl::unsafe_downcast<Derived*>(b);
    o3tl::unsafe_downcast<Derived const*>(b);
    o3tl::unsafe_downcast<Derived&>(*b); // expected-error {{}}
    o3tl::unsafe_downcast<Derived*>(d);
    o3tl::unsafe_downcast<Base*>(d); // expected-error {{}}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
