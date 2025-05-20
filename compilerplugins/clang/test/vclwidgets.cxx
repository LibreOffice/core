/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <atomic>

#include <vcl/vclptr.hxx>
#include <vcl/vclreferencebase.hxx>

struct Widget : public VclReferenceBase
{
    VclPtr<Widget> mpParent;

    void widget1()
    {
        // test that we ignore assignments from a member field
        Widget* p = mpParent;
        (void)p;
        // test against false+
        p = (true) ? mpParent.get() : nullptr;
    }

    ~Widget() override
    {
        disposeOnce();
    }

    void dispose() override
    {
        mpParent.reset();
        VclReferenceBase::dispose();
    }
};

VclPtr<Widget> f()
{
    return nullptr;
}

Widget* g()
{
    return nullptr;
}

// test the variable init detection
void bar()
{
    Widget* p = f(); // expected-error {{assigning a returned-by-value VclPtr<T> to a T* variable is dodgy, should be assigned to a VclPtr. If you know that the RHS does not return a newly created T, then add a '.get()' to the RHS [loplugin:vclwidgets]}}
    (void)p;
    Widget* q = g();
    (void)q;
    Widget* r = nullptr;
    (void)r;
}

// test the assignment detection
void bar2()
{
    Widget* p;
    p = nullptr;
    p = f(); // expected-error {{assigning a returned-by-value VclPtr<T> to a T* variable is dodgy, should be assigned to a VclPtr. If you know that the RHS does not return a newly created T, then add a '.get()' to the RHS [loplugin:vclwidgets]}}
    (void)p;
    Widget* q;
    q = g();
    (void)q;
}


// test against false+

template<class T>
T * get() { return nullptr; }

void bar3()
{
    Widget* p;
    p = get<Widget>();
    (void)p;
}

void bar4() {
    VclPtr<Widget> p1;
    //TODO: one error should be enough here?
    // expected-error@+2 {{calling delete on instance of VclReferenceBase subclass, must rather call disposeAndClear() [loplugin:vclwidgets]}}
    // expected-error@+1 {{calling delete on instance of VclPtr, must rather call disposeAndClear() [loplugin:vclwidgets]}}
    delete p1;
    std::atomic<int *> p2;
    // No false positive here:
    delete p2;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
