/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <typeinfo>

struct Base
{
    virtual ~Base();
};

struct Derived : Base
{
};

void good(Base* p)
{
    (void)(typeid(*p) == typeid(Derived));
    (void)(typeid(Derived) == typeid(*p));
    (void)(typeid(*p) != typeid(Derived));
    (void)(typeid(Derived) != typeid(*p));
}

void bad(Base* p)
{
    // expected-error@+1 {{comparison of type info of mixed pointer and non-pointer types 'Base *' and 'Derived' can never succeed [loplugin:typeidcomparison]}}
    (void)(typeid(p) == typeid(Derived));
    // expected-error@+1 {{comparison of type info of mixed pointer and non-pointer types 'Derived' and 'Base *' can never succeed [loplugin:typeidcomparison]}}
    (void)(typeid(Derived) == typeid(p));
    // expected-error@+1 {{comparison of type info of mixed pointer and non-pointer types 'Base *' and 'Derived' can never succeed [loplugin:typeidcomparison]}}
    (void)(typeid(p) != typeid(Derived));
    // expected-error@+1 {{comparison of type info of mixed pointer and non-pointer types 'Derived' and 'Base *' can never succeed [loplugin:typeidcomparison]}}
    (void)(typeid(Derived) != typeid(p));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
