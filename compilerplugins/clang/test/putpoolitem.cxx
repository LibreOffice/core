/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>

class SfxPoolItem
{
public:
    virtual ~SfxPoolItem();
};
class SfxPoolItemSubclass : public SfxPoolItem
{
};
class SfxItemSet
{
public:
    void Put(SfxPoolItem&);
};

void foo(SfxItemSet* pSet)
{
    std::unique_ptr<SfxPoolItemSubclass> foo;
    SfxItemSet aSet;
    // expected-error@+1 {{could use std::move? [loplugin:putpoolitem]}}
    aSet.Put(*foo);

    // expected-error@+1 {{could use std::move? [loplugin:putpoolitem]}}
    pSet->Put(*foo);
}

class Foo2
{
    std::unique_ptr<SfxPoolItemSubclass> m_foo;
    void foo()
    {
        SfxItemSet aSet;
        // no warning expected
        aSet.Put(*m_foo);
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
