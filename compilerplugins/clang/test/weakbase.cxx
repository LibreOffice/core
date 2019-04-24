/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

namespace tools
{
struct WeakBase
{
    virtual ~WeakBase();
};
}

struct Foo1 : public tools::WeakBase
{
    virtual ~Foo1();
};

struct Foo2 : public tools::WeakBase
{
    virtual ~Foo2();
};

// expected-error@+1 {{multiple copies of WeakBase, through inheritance paths Bar->Foo1->WeakBase, Bar->Foo2->WeakBase [loplugin:weakbase]}}
struct Bar : public Foo1, public Foo2
{
    virtual ~Bar();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
