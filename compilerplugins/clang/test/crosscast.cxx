/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

struct Base
{
    virtual ~Base();
};

struct Derived : Base
{
};

struct Virtual1 : virtual Base
{
};

struct Virtual2 : virtual Base
{
};

struct Virtual2a : Virtual2
{
};

struct Other
{
    virtual ~Other();
};

struct[[clang::annotate("loplugin:crosscast")]] Ok { virtual ~Ok(); };

Base* up(Derived* p) { return dynamic_cast<Base*>(p); }

Base& up(Derived& p) { return dynamic_cast<Base&>(p); }

Derived* down(Base* p) { return dynamic_cast<Derived*>(p); }

Derived& down(Base& p) { return dynamic_cast<Derived&>(p); }

Base* self(Base* p) { return dynamic_cast<Base*>(p); }

Base& self(Base& p) { return dynamic_cast<Base&>(p); }

Virtual1* crossVirtual(Virtual2a* p) { return dynamic_cast<Virtual1*>(p); }

Virtual1& crossVirtual(Virtual2a& p) { return dynamic_cast<Virtual1&>(p); }

Base* crossFromOther(Other* p)
{
    // expected-error@+1 {{suspicious dynamic cross cast from 'Other *' to 'Base *' [loplugin:crosscast]}}
    return dynamic_cast<Base*>(p);
}

Base& crossFromOther(Other& p)
{
    // expected-error@+1 {{suspicious dynamic cross cast from 'Other' to 'Base &' [loplugin:crosscast]}}
    return dynamic_cast<Base&>(p);
}

Other* crossToOther(Base* p)
{
    // expected-error@+1 {{suspicious dynamic cross cast from 'Base *' to 'Other *' [loplugin:crosscast]}}
    return dynamic_cast<Other*>(p);
}

Other& crossToOther(Base& p)
{
    // expected-error@+1 {{suspicious dynamic cross cast from 'Base' to 'Other &' [loplugin:crosscast]}}
    return dynamic_cast<Other&>(p);
}

Base* crossFromOk(Ok* p) { return dynamic_cast<Base*>(p); }

Base& crossFromOk(Ok& p) { return dynamic_cast<Base&>(p); }

Ok* crossToOk(Base* p) { return dynamic_cast<Ok*>(p); }

Ok& crossToOk(Base& p) { return dynamic_cast<Ok&>(p); }

void* mostDerived(Base* p) { return dynamic_cast<void*>(p); }

template <typename T> T generic1(Base* p) { return dynamic_cast<T>(p); }

template <typename T> T* generic2(Base* p) { return dynamic_cast<T*>(p); }

template <typename T> Derived* generic3(T p) { return dynamic_cast<Derived*>(p); }

template <typename T> Derived* generic4(T* p) { return dynamic_cast<Derived*>(p); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
