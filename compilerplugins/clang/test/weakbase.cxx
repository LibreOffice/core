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

// expected-error@+1 {{found multiple copies of tools::WeakBase, through inheritance paths Bar->Foo1->WeakBase, Bar->Foo2->WeakBase [loplugin:weakbase]}}
struct Bar : public Foo1, public Foo2
{
    virtual ~Bar();
};

namespace cppu
{
class OWeakObject
{
};
}

namespace test2
{
class Foo1 : public cppu::OWeakObject
{
};
class Foo2 : public cppu::OWeakObject
{
};
// expected-error@+1 {{found multiple copies of cppu::OWeakObject, through inheritance paths Foo3->Foo1->OWeakObject, Foo3->Foo2->OWeakObject [loplugin:weakbase]}}
class Foo3 : public Foo1, public Foo2
{
};
}

namespace test3
{
class Foo1 : public virtual cppu::OWeakObject
{
};
class Foo2 : public virtual cppu::OWeakObject
{
};
// no warning expected
class Foo3 : public Foo1, public Foo2
{
};
}

namespace test4
{
class Foo1 : public cppu::OWeakObject
{
};
class Foo2 : public virtual cppu::OWeakObject
{
};
// expected-error@+1 {{found one virtual base and one or more normal bases of cppu::OWeakObject, through inheritance paths Foo3->Foo1->OWeakObject, Foo3->Foo2->OWeakObject [loplugin:weakbase]}}
class Foo3 : public Foo1, public Foo2
{
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
