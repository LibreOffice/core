/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "tools/solar.h"

namespace test1
{
class Foo
{
    void bar(sal_uIntPtr x); // expected-note {{declaration site here [loplugin:typedefparam]}}
    sal_uIntPtr bar(); // expected-note {{declaration site here [loplugin:typedefparam]}}
};

void Foo::bar(sal_uLong)
// expected-error@-1 {{function param 0 at definition site does not match function param at declaration site, 'sal_uLong' (aka 'unsigned long') vs 'sal_uIntPtr' (aka 'unsigned long') [loplugin:typedefparam]}}
{
}

sal_uLong Foo::bar()
// expected-error@-1 {{function return type at definition site does not match function param at declaration site, 'sal_uLong' (aka 'unsigned long') vs 'sal_uIntPtr' (aka 'unsigned long') [loplugin:typedefparam]}}
{
    return 1;
}
};

// Carve out an exception for the "typedef struct S {...} T" idiom we use in the UNO code
namespace test2
{
typedef struct Foo
{
    int x;
} FooT;

void bar(struct Foo*);

void bar(FooT*){
    // no warning expected
};
};
namespace test3
{
typedef struct Foo
{
    int x;
} FooT;

void bar(Foo*);

void bar(FooT*){
    // no warning expected
};
};

// check method overrides
namespace test4
{
struct Struct1
{
    virtual sal_uIntPtr foo1();
    // expected-note@-1 {{super-class method here [loplugin:typedefparam]}}
    virtual void foo2(sal_uIntPtr);
    // expected-note@-1 {{super-class method here [loplugin:typedefparam]}}
    virtual ~Struct1();
};
struct Struct2 : public Struct1
{
    virtual sal_uLong foo1() override;
    // expected-error@-1 {{method return type does not match overridden method 'sal_uLong' (aka 'unsigned long') vs 'sal_uIntPtr' (aka 'unsigned long') [loplugin:typedefparam]}}
    virtual void foo2(sal_uLong) override;
    // expected-error@-1 {{method param 0 does not match overridden method param 'sal_uLong' (aka 'unsigned long') vs 'sal_uIntPtr' (aka 'unsigned long') [loplugin:typedefparam]}}
};
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
